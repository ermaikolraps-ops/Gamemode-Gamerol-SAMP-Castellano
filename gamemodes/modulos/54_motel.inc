/* =====================================================================
 *  54_motel.inc
 *  ---------------------------------------------------------------------
 *  Motel Angel Pine, encargo de Vlad del 07/09/2026. El negocio en si
 *  (compra, caja fuerte, ingreso pasivo) es una ranura mas de
 *  32_negocios.inc (NEGOCIO_SLOT_MOTEL) -- este modulo SOLO cubre las
 *  dos habitaciones de alquiler por tiempo real:
 *
 *    /motel    en el cartel de una habitacion libre: cobra
 *              MOTEL_ALQUILER_PRECIO y la deja alquilada 24h reales.
 *    /entrar   en el cartel de tu habitacion alquilada, te mete dentro.
 *    /salir    desde dentro, te devuelve EXACTAMENTE al cartel de esa
 *              misma habitacion (nunca al de la otra).
 *
 *  Las dos habitaciones comparten el mismo interior nativo de GTA:SA
 *  ("Random House", interior 2, ver g_InterioresCasa en 24_casas.inc),
 *  pero cada una tiene su PROPIO mundo virtual (MOTEL_MUNDO_BASE +
 *  indice) -- exactamente el mismo truco que usan las casas
 *  (CASA_MUNDO_BASE) y que se aplico el 07/09/2026 para separar las dos
 *  sucursales de Cluckin' Bell despues de que /salir devolviera siempre
 *  a la puerta de la PRIMERA sucursal. Sin mundo propio por habitacion
 *  se repetiria ese mismo fallo aqui.
 *
 *  Persistencia: una fila fija por habitacion (0 y 1) en
 *  `motel_habitaciones` (sql/migraciones/039_motel.sql), con el vencimiento
 *  como marca de tiempo ABSOLUTA (gettime()), igual que
 *  `pesca_barbacoas.cocina_hasta` o `vehiculos.dropeo_expira` -- para que
 *  sobreviva a un reinicio sin ambiguedad. Motel_Segundo() (llamado desde
 *  TicSegundo, 99_callbacks.inc) libera sola la habitacion en cuanto pasan
 *  las 24h, sin que nadie tenga que estar conectado para que ocurra.
 * ===================================================================== */

#define MOTEL_TOTAL_HABITACIONES 2
#define MOTEL_MUNDO_BASE         7000   // mundo = MOTEL_MUNDO_BASE + indice (0 o 1)

enum E_MOTEL_HABITACION
{
    Float:mhExtX, Float:mhExtY, Float:mhExtZ, Float:mhExtA,
    mhOcupada,                              // 0/1, no bool: se lee tal cual de la cache de mysql
    mhCuentaId,                             // cuentas.id del inquilino, 0 = libre
    mhCuentaNombre[MAX_PLAYER_NAME + 1],
    mhExpira,                               // gettime() absoluto, 0 = libre
    STREAMER_TAG_3D_TEXT_LABEL:mhEtiqueta
}
new g_MotelHab[MOTEL_TOTAL_HABITACIONES][E_MOTEL_HABITACION];

// ---------------------------------------------------------------------
//  ARRANQUE
// ---------------------------------------------------------------------
stock Motel_Iniciar()
{
    g_MotelHab[0][mhExtX] = MOTEL_HAB1_X; g_MotelHab[0][mhExtY] = MOTEL_HAB1_Y;
    g_MotelHab[0][mhExtZ] = MOTEL_HAB1_Z; g_MotelHab[0][mhExtA] = MOTEL_HAB1_A;
    g_MotelHab[1][mhExtX] = MOTEL_HAB2_X; g_MotelHab[1][mhExtY] = MOTEL_HAB2_Y;
    g_MotelHab[1][mhExtZ] = MOTEL_HAB2_Z; g_MotelHab[1][mhExtA] = MOTEL_HAB2_A;

    for (new i = 0; i < MOTEL_TOTAL_HABITACIONES; i++)
    {
        g_MotelHab[i][mhOcupada] = 0;
        g_MotelHab[i][mhCuentaId] = 0;
        g_MotelHab[i][mhCuentaNombre][0] = EOS;
        g_MotelHab[i][mhExpira] = 0;

        // Texto real (libre/alquilada) lo pone Motel_ActualizarEtiqueta
        // en cuanto responde AlCargarMotelHabitaciones -- mismo patron
        // que Negocio_Iniciar() (etiqueta en blanco) + AlCargarNegocios().
        g_MotelHab[i][mhEtiqueta] = CreateDynamic3DTextLabel(" ", 0xFFFFFFFF,
            g_MotelHab[i][mhExtX], g_MotelHab[i][mhExtY], g_MotelHab[i][mhExtZ] + 0.7, 10.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 25.0);
    }

    print("  [MOTEL] Habitaciones preparadas.");
    return 1;
}

// Icono de minimapa quitado el 11/09/2026 -- peticion expresa de Vlad
// de limpiar todos los iconos del mapa (la gente usa /gps). Se deja el
// stock vacio para no tocar el call site en OnPlayerConnect.
stock Motel_OnConectar(playerid)
{
    #pragma unused playerid
    return 1;
}

stock Motel_CargarHabitaciones()
{
    mysql_pquery(g_BD, "SELECT * FROM motel_habitaciones ORDER BY habitacion ASC", "AlCargarMotelHabitaciones");
    return 1;
}

forward AlCargarMotelHabitaciones();
public AlCargarMotelHabitaciones()
{
    new filas;
    cache_get_row_count(filas);

    new ahora = gettime();

    for (new i = 0; i < filas && i < MOTEL_TOTAL_HABITACIONES; i++)
    {
        new cuentaId, expira;
        cache_get_value_name_int(i, "cuenta_id", cuentaId);
        cache_get_value_name_int(i, "expira", expira);

        // Ya caducada (incluso si el servidor estuvo apagado mas de 24h):
        // se trata como libre desde el primer segundo, y se persiste asi
        // para no volver a leer datos viejos la proxima vez.
        if (cuentaId != 0 && expira > ahora)
        {
            g_MotelHab[i][mhOcupada] = 1;
            g_MotelHab[i][mhCuentaId] = cuentaId;
            cache_get_value_name(i, "cuenta_nombre", g_MotelHab[i][mhCuentaNombre], MAX_PLAYER_NAME + 1);
            g_MotelHab[i][mhExpira] = expira;
        }
        else
        {
            g_MotelHab[i][mhOcupada] = 0;
            g_MotelHab[i][mhCuentaId] = 0;
            g_MotelHab[i][mhCuentaNombre][0] = EOS;
            g_MotelHab[i][mhExpira] = 0;
            Motel_Guardar(i);
        }

        Motel_ActualizarEtiqueta(i);
    }

    printf("  [MOTEL] %d habitaciones cargadas desde la base de datos.", filas);
    return 1;
}

stock Motel_Guardar(indice)
{
    new consulta[256];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO motel_habitaciones (habitacion, cuenta_id, cuenta_nombre, expira) \
         VALUES (%d, %d, '%e', %d) \
         ON DUPLICATE KEY UPDATE cuenta_id=VALUES(cuenta_id), cuenta_nombre=VALUES(cuenta_nombre), expira=VALUES(expira)",
        indice, g_MotelHab[indice][mhCuentaId], g_MotelHab[indice][mhCuentaNombre], g_MotelHab[indice][mhExpira]);
    mysql_pquery(g_BD, consulta);
    return 1;
}

// OCUPADA en rojo -- peticion expresa de Vlad, 07/09/2026: solo dos
// habitaciones, no se pueden alquilar a mas gente, tiene que quedar claro
// a simple vista que ya tiene inquilino (antes seguia diciendo "/Entrar"
// aunque fuera de otro jugador, ambiguo para quien pasaba por al lado).
stock Motel_ActualizarEtiqueta(indice)
{
    new texto[96];
    if (g_MotelHab[indice][mhOcupada])
    {
        format(texto, sizeof texto, "{4FD1C5}Habitacion Motel\n{FF0000}OCUPADA");
    }
    else
    {
        format(texto, sizeof texto, "{4FD1C5}Habitacion Motel\n{FFFFFF}/Motel");
    }
    UpdateDynamic3DTextLabelText(g_MotelHab[indice][mhEtiqueta], 0xFFFFFFFF, texto);
    return 1;
}

// ---------------------------------------------------------------------
//  ¿QUE HABITACION HAY EN ESTE PUNTO / EN QUE HABITACION ESTOY DENTRO?
// ---------------------------------------------------------------------
stock Motel_HabitacionEnPunto(playerid)
{
    // Interior Y mundo, no solo interior -- varios interiores de GTA:SA
    // comparten las mismas coordenadas en bruto (mismo motivo que
    // Negocio_SlotCercano, 32_negocios.inc).
    if (GetPlayerInterior(playerid) != 0) return -1;
    if (GetPlayerVirtualWorld(playerid) != MUNDO_NORMAL) return -1;

    for (new i = 0; i < MOTEL_TOTAL_HABITACIONES; i++)
    {
        if (EnPunto(playerid, MOTEL_HAB_RADIO, g_MotelHab[i][mhExtX], g_MotelHab[i][mhExtY], g_MotelHab[i][mhExtZ])) return i;
    }
    return -1;
}

stock Motel_HabitacionDondeEstoy(playerid)
{
    if (GetPlayerInterior(playerid) != MOTEL_INTERIOR_ID) return -1;

    new mundo = GetPlayerVirtualWorld(playerid);
    for (new i = 0; i < MOTEL_TOTAL_HABITACIONES; i++)
    {
        if (mundo == MOTEL_MUNDO_BASE + i) return i;
    }
    return -1;
}

// ---------------------------------------------------------------------
//  /MOTEL -- alquilar una habitacion libre
// ---------------------------------------------------------------------
CMD:motel(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new indice = Motel_HabitacionEnPunto(playerid);
    if (indice == -1)
    {
        Error(playerid, "Tienes que estar en el cartel de una habitacion del motel.");
        return 1;
    }

    if (g_MotelHab[indice][mhOcupada])
    {
        if (g_MotelHab[indice][mhCuentaId] == Jugador[playerid][jID])
        {
            Info(playerid, "Ya tienes esta habitacion alquilada. Usa /entrar.");
        }
        else
        {
            Error(playerid, "Esta habitacion ya esta alquilada. Prueba la otra.");
        }
        return 1;
    }

    if (!TieneDinero(playerid, MOTEL_ALQUILER_PRECIO))
    {
        Error(playerid, "No te llegan los 1.000$ para alquilar la habitacion.");
        return 1;
    }

    DarDinero(playerid, -MOTEL_ALQUILER_PRECIO);
    Negocio_Ingresar(NEGOCIO_SLOT_MOTEL, MOTEL_ALQUILER_PRECIO);

    g_MotelHab[indice][mhOcupada] = 1;
    g_MotelHab[indice][mhCuentaId] = Jugador[playerid][jID];
    NombreRP(playerid, g_MotelHab[indice][mhCuentaNombre], MAX_PLAYER_NAME + 1);
    g_MotelHab[indice][mhExpira] = gettime() + MOTEL_ALQUILER_SEGUNDOS;
    Motel_Guardar(indice);
    Motel_ActualizarEtiqueta(indice);

    Exito(playerid, "Has alquilado la habitacion. Fuiste cobrado 1.000$, valida 24 horas reales.");
    return 1;
}

// ---------------------------------------------------------------------
//  /ENTRAR y /SALIR -- llamados desde CMD:entrar/CMD:salir, 15_banco.inc
// ---------------------------------------------------------------------
stock bool:Motel_IntentarEntrar(playerid)
{
    new indice = Motel_HabitacionEnPunto(playerid);
    if (indice == -1) return false;

    if (!g_MotelHab[indice][mhOcupada])
    {
        Error(playerid, "Esta habitacion esta libre. Alquilala con /motel.");
        return true;
    }

    if (g_MotelHab[indice][mhCuentaId] != Jugador[playerid][jID])
    {
        Error(playerid, "Esta habitacion esta alquilada por otro jugador.");
        return true;
    }

    SetPlayerPos(playerid, MOTEL_INT_X, MOTEL_INT_Y, MOTEL_INT_Z);
    SetPlayerInterior(playerid, MOTEL_INTERIOR_ID);
    SetPlayerVirtualWorld(playerid, MOTEL_MUNDO_BASE + indice);
    SetCameraBehindPlayer(playerid);
    return true;
}

stock bool:Motel_IntentarSalir(playerid)
{
    new indice = Motel_HabitacionDondeEstoy(playerid);
    if (indice == -1) return false;

    SetPlayerPos(playerid, g_MotelHab[indice][mhExtX], g_MotelHab[indice][mhExtY], g_MotelHab[indice][mhExtZ]);
    SetPlayerFacingAngle(playerid, g_MotelHab[indice][mhExtA]);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
    SetCameraBehindPlayer(playerid);
    return true;
}

// ---------------------------------------------------------------------
//  CADUCIDAD -- una vez por segundo (TicSegundo, 99_callbacks.inc), igual
//  de barato que Veh_AvisoCaducidad_Segundo: solo dos habitaciones.
// ---------------------------------------------------------------------
stock Motel_Segundo()
{
    new ahora = gettime();

    for (new i = 0; i < MOTEL_TOTAL_HABITACIONES; i++)
    {
        if (!g_MotelHab[i][mhOcupada]) continue;
        if (ahora < g_MotelHab[i][mhExpira]) continue;

        new cuentaId = g_MotelHab[i][mhCuentaId];

        g_MotelHab[i][mhOcupada] = 0;
        g_MotelHab[i][mhCuentaId] = 0;
        g_MotelHab[i][mhCuentaNombre][0] = EOS;
        g_MotelHab[i][mhExpira] = 0;
        Motel_Guardar(i);
        Motel_ActualizarEtiqueta(i);

        for (new p = 0; p < MAX_PLAYERS; p++)
        {
            if (IsPlayerConnected(p) && Jugador[p][jLogueado] && Jugador[p][jID] == cuentaId)
            {
                Info(p, "Tu alquiler de la habitacion del motel ha caducado.");
            }
        }
    }
    return 1;
}

// ---------------------------------------------------------------------
//  INGRESO PASIVO -- una vez al dia real de uptime, mismo tick que
//  Comida_IngresoDiario (TicSegundo, 99_callbacks.inc).
// ---------------------------------------------------------------------
stock Motel_IngresoDiario()
{
    Negocio_Ingresar(NEGOCIO_SLOT_MOTEL, MOTEL_INGRESO_DIARIO);
    return 1;
}
