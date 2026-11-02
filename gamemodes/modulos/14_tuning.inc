/* =====================================================================
 *  14_tuning.inc
 *  ---------------------------------------------------------------------
 *  Tuning persistente. Portado del "sistema de tuning persistente" del
 *  catalogo de scripts antiguos (My old PAWNO SCRIPTS.txt), pero sin su
 *  tienda propia: aqui no hace falta, porque GTA:SA ya trae talleres
 *  nativos (Transfender, Wheel Arch Angels, Loco Low Co...) que abren
 *  solos su menu de piezas al entrar en el interior con un vehiculo
 *  compatible. El juego llama a OnVehicleMod/OnVehiclePaintjob cuando
 *  el jugador elige algo ahi dentro; este modulo solo escucha esas dos
 *  llamadas y guarda lo elegido, para que siga puesto la proxima vez
 *  que el coche aparezca (reinicio del servidor, /guardarcoche, etc.).
 *
 *  Se persiste el tuning de CUALQUIER vehiculo, dropeo incluido -- en
 *  este servidor casi toda la flota son coches libres (VEH_DROPEO), asi
 *  que excluirlos dejaba el sistema practicamente inutil. Efecto
 *  colateral aceptado: si tuneas un coche libre y luego cambia de dueno
 *  (o vuelve a quedar libre al caducar los papeles, ver 22_dropeo.inc),
 *  el tuning se queda puesto para quien lo use despues -- no se borra
 *  hasta que el vehiculo se destruye de verdad (Veh_Borrar).
 *
 *  No hay forma de LEER la pintura de un vehiculo (no existe un
 *  GetVehiclePaintjob nativo), asi que se recuerda en memoria
 *  (g_TuningPintura, indexado igual que g_Vehiculos) desde el momento
 *  en que OnVehiclePaintjob la reporta.
 * ===================================================================== */

static g_TuningPintura[MAX_VEHICULOS];

stock Tuning_Iniciar()
{
    for (new i = 0; i < MAX_VEHICULOS; i++) g_TuningPintura[i] = -1;
    return 1;
}

// ---------------------------------------------------------------------
//  APLICAR -- se llama desde Veh_Aparecer (13_vehiculos.inc) cada vez
//  que el coche nace, tanto al cargar el servidor como al comprarse uno
//  nuevo. Async: si no hay fila guardada, no pasa nada.
// ---------------------------------------------------------------------
public Tuning_Aplicar(indice)
{
    if (!VehiculoValido(indice) || g_Vehiculos[indice][veID] == 0) return 0;

    new consulta[96];
    format(consulta, sizeof consulta, "SELECT * FROM vehiculo_tuning WHERE veh_id = %d", g_Vehiculos[indice][veID]);
    mysql_pquery(g_BD, consulta, "AlCargarTuning", "d", indice);
    return 1;
}

forward AlCargarTuning(indice);
public AlCargarTuning(indice)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0) return 0;
    if (!VehiculoValido(indice)) return 0;

    new vehicleid = g_Vehiculos[indice][veObjeto];
    if (vehicleid == INVALID_VEHICLE_ID) return 0;

    new campo[16], valor;
    for (new slot = 0; slot < TUNING_SLOTS; slot++)
    {
        format(campo, sizeof campo, "comp_%d", slot);
        cache_get_value_name_int(0, campo, valor);
        if (valor > 0) AddVehicleComponent(vehicleid, valor);
    }

    new pintura;
    cache_get_value_name_int(0, "pintura", pintura);
    g_TuningPintura[indice] = pintura;
    if (pintura >= 0) ChangeVehiclePaintjob(vehicleid, pintura);
    return 1;
}

// ---------------------------------------------------------------------
//  GUARDAR -- vuelca TODAS las piezas actuales del coche (no solo la
//  que se acaba de cambiar) mas la pintura recordada en memoria. Un
//  UPSERT sencillo: el volumen de escrituras no justifica nada mas fino.
// ---------------------------------------------------------------------
stock Tuning_Guardar(indice)
{
    if (!VehiculoValido(indice) || g_Vehiculos[indice][veID] == 0) return 0;

    new vehicleid = g_Vehiculos[indice][veObjeto];
    if (vehicleid == INVALID_VEHICLE_ID) return 0;

    new comp[TUNING_SLOTS];
    for (new slot = 0; slot < TUNING_SLOTS; slot++) comp[slot] = GetVehicleComponentInSlot(vehicleid, CARMODTYPE:slot);

    new consulta[850];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO vehiculo_tuning \
            (veh_id, comp_0,comp_1,comp_2,comp_3,comp_4,comp_5,comp_6,comp_7,comp_8,comp_9,comp_10,comp_11,comp_12,comp_13,comp_14,comp_15, pintura) \
         VALUES (%d, %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, %d) \
         ON DUPLICATE KEY UPDATE \
            comp_0=VALUES(comp_0),   comp_1=VALUES(comp_1),   comp_2=VALUES(comp_2),   comp_3=VALUES(comp_3), \
            comp_4=VALUES(comp_4),   comp_5=VALUES(comp_5),   comp_6=VALUES(comp_6),   comp_7=VALUES(comp_7), \
            comp_8=VALUES(comp_8),   comp_9=VALUES(comp_9),   comp_10=VALUES(comp_10), comp_11=VALUES(comp_11), \
            comp_12=VALUES(comp_12), comp_13=VALUES(comp_13), comp_14=VALUES(comp_14), comp_15=VALUES(comp_15), \
            pintura=VALUES(pintura)",
        g_Vehiculos[indice][veID],
        comp[0], comp[1], comp[2], comp[3], comp[4], comp[5], comp[6], comp[7],
        comp[8], comp[9], comp[10], comp[11], comp[12], comp[13], comp[14], comp[15],
        g_TuningPintura[indice]);

    mysql_pquery(g_BD, consulta);
    return 1;
}

// ---------------------------------------------------------------------
//  GANCHOS DE OnVehicleMod / OnVehiclePaintjob (99_callbacks.inc)
// ---------------------------------------------------------------------
stock Tuning_AlModificar(vehicleid)
{
    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (!VehiculoValido(indice) || g_Vehiculos[indice][veID] == 0) return 0;

    Tuning_Guardar(indice);
    return 1;
}

stock Tuning_AlPintar(vehicleid, paintjobid)
{
    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (!VehiculoValido(indice) || g_Vehiculos[indice][veID] == 0) return 0;

    g_TuningPintura[indice] = paintjobid;
    Tuning_Guardar(indice);
    return 1;
}

// ---------------------------------------------------------------------
//  BORRADO -- llamado desde Veh_Borrar (13_vehiculos.inc) antes de que
//  se vacie la ficha del vehiculo, mientras veID todavia es valido.
// ---------------------------------------------------------------------
public Tuning_AlBorrarVehiculo(indice)
{
    if (!VehiculoValido(indice)) return 0;

    if (g_Vehiculos[indice][veID] > 0)
    {
        new consulta[64];
        format(consulta, sizeof consulta, "DELETE FROM vehiculo_tuning WHERE veh_id = %d", g_Vehiculos[indice][veID]);
        mysql_pquery(g_BD, consulta);
    }

    g_TuningPintura[indice] = -1;
    return 1;
}

// ---------------------------------------------------------------------
//  MARCADORES DE TRANSFENDER (04/09/2026) -- mismo patron que
//  Federico_CrearMarcador (43_federico.inc): objeto plano (19329) con
//  texto como textura + etiqueta 3D, sin arriesgar coordenadas
//  inventadas de memoria. Los talleres nativos de GTA:SA no tienen
//  coordenadas en este proyecto porque el juego los abre solo (ver
//  cabecera del archivo); para marcarlos hace falta que alguien vaya
//  fisicamente y los apunte con /marcarcoord, igual que se hizo con
//  Federico y con las puertas de las franquicias de comida (ver
//  42_comida.inc).
//
//  A proposito de alcance (encargo de Vlad, 04/09/2026): SOLO
//  Transfender. Wheel Arch Angels y Loco Low Co quedan fuera, no es un
//  olvido -- si algun dia se piden, es anadir su propio prefijo de
//  clave, esto no los cubre.
//
//  Cada fila de la tabla PROPIA `transfender_marcadores` saca su
//  marcador -- no hay limite fijo de 1, para cubrir las varias copias
//  reales que GTA:SA tiene de Transfender por el mapa segun se vayan
//  marcando con /marcartransfender.
//
//  Tabla propia desde el 12/09/2026 (migracion 049) -- antes vivia como
//  filas sueltas en la tabla generica `coordenadas` (clave 'transfender1'
//  etc.), y el texto flotante desaparecio DOS VECES porque algo (una
//  limpieza manual de esa tabla compartida, lo mas probable) se llevaba
//  por delante esas filas junto con marcadores de prueba sin relacion.
//  Encargo expreso de Vlad: que esto no vuelva a pasar "pase lo que
//  pase" -- al vivir en su propia tabla, ninguna limpieza de
//  `coordenadas` puede tocarla. /marcarcoord y /borrarcoord siguen
//  existiendo igual que siempre, para todo lo demas.
// ---------------------------------------------------------------------
#define MAX_MARCADORES_TRANSFENDER 8

// Icono de llave inglesa en el minimapa, siempre visible (peticion de
// Vlad, 05/09/2026) -- un slot de icono por cada Transfender cargado.
#define TUNING_ICON_SLOT_BASE      80

static STREAMER_TAG_OBJECT:g_TransfenderMarcadorObj[MAX_MARCADORES_TRANSFENDER];
static STREAMER_TAG_3D_TEXT_LABEL:g_TransfenderMarcadorEtq[MAX_MARCADORES_TRANSFENDER];
static Float:g_TransfenderMarcadorPos[MAX_MARCADORES_TRANSFENDER][3];
static g_TransfenderTotalMarcadores = 0;

stock Tuning_CrearMarcadores()
{
    mysql_pquery(g_BD, "SELECT nombre, x, y, z FROM transfender_marcadores ORDER BY id ASC", "AlCargarTransfender");
    return 1;
}

// Crea el objeto+etiqueta de un marcador ya sea al arrancar (AlCargarTransfender)
// o al momento con /marcartransfender -- un solo sitio que sabe como se ve un
// Transfender, para no duplicar el bloque de creacion en dos sitios.
stock Tuning_CrearUnMarcador(slot, const nombre[48], Float:x, Float:y, Float:z)
{
    g_TransfenderMarcadorPos[slot][0] = x;
    g_TransfenderMarcadorPos[slot][1] = y;
    g_TransfenderMarcadorPos[slot][2] = z;

    g_TransfenderMarcadorObj[slot] = CreateDynamicObject(19329, x, y, z + 1.2, 0.0, 0.0, 0.0, 0, 0, -1, 100.0);
    SetDynamicObjectMaterialText(g_TransfenderMarcadorObj[slot], 0, "TRANSFENDER",
        OBJECT_MATERIAL_SIZE_128x32, "Impact", 20, 1, 0xFFE8B33A, 0x00000000, 1);

    format(g_Buffer, sizeof g_Buffer, "%s\n{FFFFFF}Transfender -- entra con tu vehiculo", nombre);
    g_TransfenderMarcadorEtq[slot] = CreateDynamic3DTextLabel(g_Buffer, COLOR_MARCA, x, y, z + 0.7, 14.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);
    return 1;
}

forward AlCargarTransfender();
public AlCargarTransfender()
{
    new filas;
    cache_get_row_count(filas);

    g_TransfenderTotalMarcadores = 0;

    for (new i = 0; i < filas && i < MAX_MARCADORES_TRANSFENDER; i++)
    {
        new Float:x, Float:y, Float:z;
        new nombre[48];
        cache_get_value_name_float(i, "x", x);
        cache_get_value_name_float(i, "y", y);
        cache_get_value_name_float(i, "z", z);
        cache_get_value_name(i, "nombre", nombre, 48);

        Tuning_CrearUnMarcador(g_TransfenderTotalMarcadores, nombre, x, y, z);
        g_TransfenderTotalMarcadores++;
    }

    printf("  [TRANSFENDER] %d marcadores preparados (tabla propia transfender_marcadores).", g_TransfenderTotalMarcadores);
    return 1;
}

// ---------------------------------------------------------------------
//  /marcartransfender y /borrartransfender -- mismo nivel que
//  /marcarcoord (ADMIN_GESTION). Guardan en la tabla PROPIA (ver arriba)
//  y crean/destruyen el marcador AL INSTANTE, sin esperar a un reinicio.
// ---------------------------------------------------------------------
CMD:marcartransfender(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new nombre[48];
    if (sscanf(params, "s[48]", nombre))
    {
        Uso(playerid, "/marcartransfender [nombre para mostrar]");
        return 1;
    }

    if (g_TransfenderTotalMarcadores >= MAX_MARCADORES_TRANSFENDER)
    {
        Error(playerid, "Ya hay el maximo de Transfender marcados.");
        return 1;
    }

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    LimpiarTexto(nombre);

    new admin[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "INSERT INTO transfender_marcadores (nombre, x, y, z, creado_por) VALUES ('%e', %f, %f, %f, '%e')",
        nombre, x, y, z, admin);
    mysql_pquery(g_BD, g_Buffer);

    Tuning_CrearUnMarcador(g_TransfenderTotalMarcadores, nombre, x, y, z);
    g_TransfenderTotalMarcadores++;

    Exito(playerid, "Transfender marcado -- guardado para siempre, no hace falta reiniciar.");
    return 1;
}

CMD:borrartransfender(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;
    #pragma unused params

    if (g_TransfenderTotalMarcadores <= 0)
    {
        Error(playerid, "No hay ningun Transfender marcado.");
        return 1;
    }

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    new cercano = -1;
    new Float:mejorDist = 999999.0;
    for (new i = 0; i < g_TransfenderTotalMarcadores; i++)
    {
        new Float:dist = VectorSize(px - g_TransfenderMarcadorPos[i][0], py - g_TransfenderMarcadorPos[i][1], pz - g_TransfenderMarcadorPos[i][2]);
        if (dist < mejorDist) { mejorDist = dist; cercano = i; }
    }

    if (cercano == -1 || mejorDist > 15.0)
    {
        Error(playerid, "No hay ningun Transfender marcado cerca de ti (15m).");
        return 1;
    }

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "DELETE FROM transfender_marcadores WHERE x = %f AND y = %f AND z = %f LIMIT 1",
        g_TransfenderMarcadorPos[cercano][0], g_TransfenderMarcadorPos[cercano][1], g_TransfenderMarcadorPos[cercano][2]);
    mysql_pquery(g_BD, g_Buffer);

    DestroyDynamicObject(g_TransfenderMarcadorObj[cercano]);
    DestroyDynamic3DTextLabel(g_TransfenderMarcadorEtq[cercano]);

    // Compacta el hueco moviendo el ultimo a la posicion borrada -- mismo
    // patron que /borrarcoord (16_administracion.inc).
    g_TransfenderTotalMarcadores--;
    if (cercano != g_TransfenderTotalMarcadores)
    {
        g_TransfenderMarcadorObj[cercano] = g_TransfenderMarcadorObj[g_TransfenderTotalMarcadores];
        g_TransfenderMarcadorEtq[cercano] = g_TransfenderMarcadorEtq[g_TransfenderTotalMarcadores];
        g_TransfenderMarcadorPos[cercano][0] = g_TransfenderMarcadorPos[g_TransfenderTotalMarcadores][0];
        g_TransfenderMarcadorPos[cercano][1] = g_TransfenderMarcadorPos[g_TransfenderTotalMarcadores][1];
        g_TransfenderMarcadorPos[cercano][2] = g_TransfenderMarcadorPos[g_TransfenderTotalMarcadores][2];
    }

    Exito(playerid, "Transfender borrado.");
    return 1;
}

// Coloca el icono de llave inglesa (Mod garage, id 27) de cada
// Transfender cargado en el minimapa del jugador, sin limite de
// distancia -- se llama en OnPlayerConnect, antes incluso de iniciar
// sesion, igual que el resto de _OnConectar de este archivo.
// Iconos de minimapa quitados el 11/09/2026 -- peticion expresa de Vlad
// de limpiar todos los iconos del mapa (la gente usa /gps). Se deja el
// stock vacio para no tocar el call site en OnPlayerConnect.
stock Tuning_OnConectar(playerid)
{
    #pragma unused playerid
    return 1;
}

// Accesor para 30_gps.inc: el primer Transfender marcado con
// /marcarcoord ("transfender1", etc.) -- g_TransfenderMarcadorPos es
// "static" (privado de este archivo), y su numero es dinamico (0-8,
// segun lo que haya en /panel), asi que no puede vivir como #define fijo
// en la lista de destinos de siempre. Devuelve false si Vlad todavia no
// ha marcado ninguno.
stock bool:Tuning_PrimerTransfenderPos(&Float:x, &Float:y, &Float:z)
{
    if (g_TransfenderTotalMarcadores <= 0) return false;

    x = g_TransfenderMarcadorPos[0][0];
    y = g_TransfenderMarcadorPos[0][1];
    z = g_TransfenderMarcadorPos[0][2];
    return true;
}
