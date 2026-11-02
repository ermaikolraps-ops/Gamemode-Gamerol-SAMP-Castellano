/* =====================================================================
 *  22_dropeo.inc
 *  ---------------------------------------------------------------------
 *  Coches libres por la calle + robo de coches ajenos dejados abiertos.
 *  Rediseno 2026-08-12.
 *
 *  COCHES LIBRES (VEH_DROPEO, propietario = 0)
 *  --------------------------------------------------------------------
 *  Repartidos por todo el mapa (posiciones reales extraidas del main.scm
 *  original de San Andreas). Estan SIEMPRE abiertos pero con el motor
 *  APAGADO: cualquiera puede subirse, pero para arrancarlo hay que
 *  comprarlo primero con /comprarcoche estando al volante (2.000$ a
 *  12.000$ segun el modelo) y luego usar /motor. Los papeles duran
 *  DROPEO_DIAS_PROPIEDAD dias; se renuevan en el taller de la NFS a
 *  partir del sexto dia (DROPEO_RENOVAR_DIAS_ANTES), con /renovarpapeles
 *  o el menu /taller. Si caducan, el coche se teletransporta a su ultimo
 *  sitio aparcado (/park) y vuelve a estar libre y apagado.
 *
 *  ROBO DE COCHES AJENOS (VEH_PERSONAL o VEH_DROPEO con dueno)
 *  --------------------------------------------------------------------
 *  Si el dueno deja su coche ABIERTO por error, cualquiera puede
 *  subirse y usar /puente para arrancarlo sin llave (30% de exito, 4s
 *  de espera). Una vez arrancado, llevalo al taller de la NFS y usa
 *  /dropearcoche (o el menu /taller): el coche vuelve al pool de libres
 *  (propietario = 0, matricula y papeles borrados) y tu te llevas
 *  DROPEO_PREMIO_DROPEO. /dropearcoche NUNCA renueva tus propios
 *  papeles  eso es /renovarpapeles, son cosas distintas.
 *
 *  Comandos jugador:
 *      /comprarcoche       al volante de un coche libre, lo compras
 *      /park                al volante de un coche tuyo, fija su sitio
 *                          de aparcamiento (13_vehiculos.inc)
 *      /taller              menu del taller NFS: renovar papeles o
 *                          dropear un ajeno, segun en que coche estes
 *      /renovarpapeles      en el taller NFS: renueva TUS papeles (solo
 *                          a partir del sexto dia)
 *      /dropearcoche        en el taller NFS: vende un coche AJENO que
 *                          hayas puenteado (NO renueva tus propios papeles)
 *      /puente             arranca sin llave un coche abierto ajeno
 *
 *  Comandos admin (nivel ADMIN_ADMIN):
 *      /creardropeo <modelo>          anade un coche libre en tu sitio
 *      /adminpark                     fija la posicion actual de un libre
 *      /borrardropeo                  borra el coche libre mas cercano (<2m)
 *      /listadropeo                   cuenta libres y con dueno del sistema
 *      /tpdropeo <id_bd>               teleport a un coche por su id de BD
 *      /limpiardropeos <N>             deja solo N libres sin dueno
 * ===================================================================== */

// ---------------------------------------------------------------------
//  ESTADO EN MEMORIA
// ---------------------------------------------------------------------
static bool:g_DropeoPuenteando[MAX_PLAYERS];        // Esta en el freeze de 4s
static      g_DropeoPuenteTimer[MAX_PLAYERS];       // Id del SetTimerEx en curso
static      g_DropeoPuenteIndice[MAX_PLAYERS];      // Indice del veh que puentea
static      g_DropeoIntentos[MAX_PLAYERS];          // Fallos acumulados
static      g_DropeoCooldown[MAX_PLAYERS];          // gettime() hasta el que no puede

// Cuenta atras visual de /puente (coherencia audiovisual, 08/09/2026) --
// timer PROPIO e independiente del de arriba (DropeoPuenteFin, que decide
// exito/fallo): este solo actualiza el textdraw en pantalla, nunca toca
// la mecanica ni la duracion real.
static      g_DropeoPuenteTicTimer[MAX_PLAYERS];    // Id del timer de 1s de la cuenta atras
static      g_DropeoPuenteRestante[MAX_PLAYERS];    // Segundos restantes mostrados
enum { DPT_TITULO, DPT_SUBTITULO, DPT_TOTAL }
static PlayerText:g_DropeoPuenteTD[MAX_PLAYERS][DPT_TOTAL];

// Un coche puenteado con exito se puede arrancar sin llave hasta que
// alguien lo vuelve a cerrar/guardar. Flag de sesion, no se persiste.
static bool:g_DropeoArrancado[MAX_VEHICULOS];
static bool:g_DropeoOcupado[MAX_VEHICULOS];    // Alguien lo esta puenteando ahora

// Accesor para modulos incluidos DESPUES que este (43_federico.inc, al
// crear el Coche de la Droga) -- g_DropeoArrancado es static, no se ve
// desde otro archivo, pero una funcion si.
stock Dropeo_MarcarApagado(indice)
{
    g_DropeoArrancado[indice] = false;
    return 1;
}

// Cuenta cuanta gente hay dentro (conductor + pasajeros) del coche, y la
// "generacion" de la secuencia de parpadeo en curso (ver mas abajo,
// Dropeo_AlSubir / Dropeo_CartelParpadeo). Declaradas aqui arriba porque
// Dropeo_ActualizarCartel ya las necesita.
static g_DropeoOcupantes[MAX_VEHICULOS];
static g_DropeoParpadeoGen[MAX_VEHICULOS];

// Para que otros modulos (26_nfs.inc) puedan dar por vendido un coche
// libre sin tocar el array directamente.
stock Dropeo_MarcarArrancable(indice)
{
    g_DropeoArrancado[indice] = true;
    return 1;
}

// ---------------------------------------------------------------------
//  HOOKS DE CICLO DE VIDA
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
//  CUENTA ATRAS VISUAL DE /PUENTE -- posicion inferior (Y=380/402),
//  reutilizada de 44_hambre_sed.inc ("Te estas muriendo de hambre!") tal
//  como pidio Vlad, 12/09/2026: antes vivia arriba, copiando la posicion
//  de /arreglarcoche/Federico. Fuente, tamano de titulo y colores
//  tambien alineados con esa referencia (57_roboexpress.inc hizo el
//  mismo ajuste antes, para su propio aviso). Solo presentacion: nunca
//  decide exito/fallo, eso lo sigue haciendo DropeoPuenteFin exactamente
//  igual que antes.
// ---------------------------------------------------------------------
stock Dropeo_CrearTextdrawsPuente(playerid)
{
    g_DropeoPuenteTD[playerid][DPT_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 380.0, "PUENTEANDO...");
    PlayerTextDrawAlignment(playerid, g_DropeoPuenteTD[playerid][DPT_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_DropeoPuenteTD[playerid][DPT_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_DropeoPuenteTD[playerid][DPT_TITULO], 0.42, 1.8);
    PlayerTextDrawColour(playerid, g_DropeoPuenteTD[playerid][DPT_TITULO], 0x4FD1C5FF);
    PlayerTextDrawSetOutline(playerid, g_DropeoPuenteTD[playerid][DPT_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_DropeoPuenteTD[playerid][DPT_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_DropeoPuenteTD[playerid][DPT_TITULO], true);

    g_DropeoPuenteTD[playerid][DPT_SUBTITULO] = CreatePlayerTextDraw(playerid, 320.0, 402.0, "4");
    PlayerTextDrawAlignment(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO], true);
    return 1;
}

stock Dropeo_PuenteOcultarAviso(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    PlayerTextDrawHide(playerid, g_DropeoPuenteTD[playerid][DPT_TITULO]);
    PlayerTextDrawHide(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO]);
    return 1;
}

// Timer PROPIO de 1000ms, repetitivo, independiente del SetTimerEx de
// DROPEO_PUENTE_FREEZE_MS que decide el resultado. Solo actualiza el
// numero en pantalla; se autodestruye al llegar a "1" (el ultimo cambio
// de textdraw a "0" no hace falta, DropeoPuenteFin oculta el aviso justo
// cuando la mecanica real termina, en el mismo instante).
forward Dropeo_PuenteTic(playerid);
public  Dropeo_PuenteTic(playerid)
{
    if (!IsPlayerConnected(playerid) || !g_DropeoPuenteando[playerid])
    {
        if (g_DropeoPuenteTicTimer[playerid] != -1)
        {
            KillTimer(g_DropeoPuenteTicTimer[playerid]);
            g_DropeoPuenteTicTimer[playerid] = -1;
        }
        Dropeo_PuenteOcultarAviso(playerid);
        return 0;
    }

    g_DropeoPuenteRestante[playerid]--;
    if (g_DropeoPuenteRestante[playerid] <= 0)
    {
        KillTimer(g_DropeoPuenteTicTimer[playerid]);
        g_DropeoPuenteTicTimer[playerid] = -1;
        return 1;   // se queda mostrando "1" hasta que DropeoPuenteFin oculte el aviso
    }

    new texto[4];
    format(texto, sizeof texto, "%d", g_DropeoPuenteRestante[playerid]);
    PlayerTextDrawSetString(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO], texto);
    return 1;
}

forward Dropeo_OnConectar(playerid);
public  Dropeo_OnConectar(playerid)
{
    g_DropeoPuenteando[playerid] = false;
    g_DropeoPuenteTimer[playerid] = -1;
    g_DropeoPuenteIndice[playerid] = -1;
    g_DropeoIntentos[playerid] = 0;
    g_DropeoCooldown[playerid] = 0;
    g_DropeoPuenteTicTimer[playerid] = -1;
    g_DropeoPuenteRestante[playerid] = 0;
    Dropeo_CrearTextdrawsPuente(playerid);
    return 1;
}

forward Dropeo_OnDesconectar(playerid);
public  Dropeo_OnDesconectar(playerid)
{
    if (g_DropeoPuenteando[playerid] && g_DropeoPuenteTimer[playerid] != -1)
    {
        KillTimer(g_DropeoPuenteTimer[playerid]);
        g_DropeoPuenteTimer[playerid] = -1;
    }
    if (g_DropeoPuenteTicTimer[playerid] != -1)
    {
        KillTimer(g_DropeoPuenteTicTimer[playerid]);
        g_DropeoPuenteTicTimer[playerid] = -1;
    }
    g_DropeoPuenteando[playerid] = false;
    if (g_DropeoPuenteIndice[playerid] != -1)
    {
        new i = g_DropeoPuenteIndice[playerid];
        if (i >= 0 && i < MAX_VEHICULOS) g_DropeoOcupado[i] = false;
        g_DropeoPuenteIndice[playerid] = -1;
    }
    return 1;
}

forward Dropeo_OnSpawn(playerid);
public  Dropeo_OnSpawn(playerid)
{
    #pragma unused playerid
    return 1;
}

stock Dropeo_Iniciar()
{
    SetTimer("DropeoTicHora", 3600000, true);
    Dropeo_CrearBlipParquin();
    Dropeo_CrearBlipTaller();
    return 1;
}

// Blip fijo (coche rojo) sobre el parquin comunitario lleno de coches
// libres. Visible siempre, para todo el mundo, no solo de cerca.
stock Dropeo_CrearBlipParquin()
{
    return 1;
}

// Blip fijo (herramienta/taller) sobre el taller de la NFS. Icono
// segun la numeracion estandar de open.mp; si no encaja con el sprite
// esperado, es cuestion de cambiar DROPEO_ICONO_TALLER.
stock Dropeo_CrearBlipTaller()
{
    return 1;
}

// ---------------------------------------------------------------------
//  PRECIO DE UN COCHE LIBRE SEGUN EL MODELO
//  Escala el precio de catalogo (13_vehiculos.inc) a un rango 2000-12000.
//  Se usa al crear coches nuevos (/creardropeo) y al reciclar uno
//  dropeado de vuelta al pool de libres.
// ---------------------------------------------------------------------
// Los libres siempre nacen con el motor apagado (hay que comprarlos con
// /comprarcoche para arrancarlos). Se llama cada vez que un coche VUELVE
// a ser libre (caducidad o dropear uno ajeno) para dejarlo apagado otra vez.
stock Dropeo_ForzarMotorApagado(indice)
{
    if (!VehiculoValido(indice)) return 0;

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(g_Vehiculos[indice][veObjeto], motor, luces, alarma, puertas, capo, maletero, objetivo);
    SetVehicleParamsEx(g_Vehiculos[indice][veObjeto], VEHICLE_PARAMS_OFF, luces, alarma, VEHICLE_PARAMS_OFF, capo, maletero, objetivo);
    return 1;
}

stock Dropeo_PrecioParaLibre(modelo)
{
    new base = Conce_PrecioModelo(modelo);
    if (base <= 0) return 5000;
    if (base > 240000) base = 240000;

    new precio = 2000 + (base * 10000) / 240000;
    if (precio < 2000) precio = 2000;
    if (precio > 12000) precio = 12000;
    return (precio / 100) * 100;
}

// ---------------------------------------------------------------------
//  HELPERS DE ACCESO
// ---------------------------------------------------------------------

// Puenteable = esta abierto y no es tuyo (personal ajeno, o dropeo con
// dueno ajeno). Los libres sin dueno NUNCA se puentean: ya estan abiertos.
stock bool:Dropeo_EsPuenteable(playerid, indice)
{
    if (!VehiculoValido(indice)) return false;

    // Coches de Jugadores (59_federicojugadores.inc, 08/09/2026): un
    // vehiculo buscado por Federico ahora mismo (o escondido en su
    // ventana de 48h tras entregarse) solo se puede robar via
    // /forzarcerradura -- nunca por el /puente generico, gane o no gane
    // aqui por estar sin cerrar. Unico punto de entrada de /puente y del
    // menu /taller, cubre los dos caminos a la vez.
    if (FedJugadores_EstaProtegido(g_Vehiculos[indice][veID])) return false;

    new tipo = g_Vehiculos[indice][veTipo];

    if (tipo == VEH_DROPEO)
    {
        if (g_Vehiculos[indice][vePropietario] == 0)
        {
            // Cualquier coche libre se puede puentear (05/09/2026,
            // peticion expresa de Vlad): no hace falta comprarlo con
            // /comprarcoche solo para cogerlo un rato -- no hay dueno al
            // que "robarle" nada, es simplemente arrancarlo. Esto ya
            // cubria de antes, como caso particular, los coches que
            // busca Federico (43_federico.inc) -- ahora es la regla
            // general, no una excepcion aparte.
            return true;
        }
        if (g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID]) return false;
        if (g_Vehiculos[indice][veCerrado]) return false;
        return true;
    }

    if (tipo == VEH_PERSONAL)
    {
        if (g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID]) return false;
        if (g_Vehiculos[indice][veCerrado]) return false;
        return true;
    }
    return false;
}

// Dropeable en el taller = tiene dueno y no eres tu. Los libres sin
// dueno no se dropean (no hay nada que "robar" ahi): se compran.
stock bool:Dropeo_EsDropeable(playerid, indice)
{
    if (!VehiculoValido(indice)) return false;

    // Coches de Jugadores: mismo motivo que en Dropeo_EsPuenteable de
    // arriba -- dropearlo aqui se saltaria por completo /forzarcerradura
    // y el candado de Federico. Cubre /dropearcoche y el menu /taller.
    if (FedJugadores_EstaProtegido(g_Vehiculos[indice][veID])) return false;

    new tipo = g_Vehiculos[indice][veTipo];
    if (tipo != VEH_PERSONAL && tipo != VEH_DROPEO) return false;
    if (g_Vehiculos[indice][vePropietario] == 0) return false;
    if (g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID]) return false;
    return true;
}

// El motor solo arranca si eres el dueno (coche comprado con
// /comprarcoche), o si ya lo puenteaste con exito. Los libres sin dueno
// NO arrancan hasta que alguien los compra.
public bool:Dropeo_PuedeArrancar(playerid, indice)
{
    if (!VehiculoValido(indice)) return true;

    new tipo = g_Vehiculos[indice][veTipo];
    if (tipo == VEH_TRABAJO || tipo == VEH_FACCION || tipo == VEH_SERVIDOR) return true;

    if (tipo == VEH_PERSONAL)
    {
        if (g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID]) return true;
        return g_DropeoArrancado[indice];
    }

    if (tipo == VEH_DROPEO)
    {
        if (g_Vehiculos[indice][vePropietario] == 0) return false;
        if (g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID]) return true;
        return g_DropeoArrancado[indice];
    }

    return true;
}

// Avisa al dueno original de que le dropearon el coche, si esta conectado.
stock Dropeo_NotificarDueno(indice, const nombreDropero[])
{
    if (!VehiculoValido(indice)) return 0;

    new duenoID = g_Vehiculos[indice][vePropietario];
    if (duenoID == 0) return 0;

    new nombreCoche[48];
    Veh_NombreModelo(g_Vehiculos[indice][veModelo], nombreCoche, sizeof nombreCoche);

    for (new p = 0; p < MAX_PLAYERS; p++)
    {
        if (!IsPlayerConnected(p)) continue;
        if (!Jugador[p][jLogueado]) continue;
        if (Jugador[p][jID] != duenoID) continue;

        format(g_Buffer, sizeof g_Buffer,
            "Te han DROPEADO el %s (%s). Ya no es tuyo  se lo llevo %s.",
            nombreCoche, g_Vehiculos[indice][veMatricula], nombreDropero);
        Aviso(p, g_Buffer);
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------
//  CARTEL SOBRE CADA COCHE LIBRE / CON PAPELES
//  Verde = libre. Rojo = con propietario. Azul = precio. Negro = el
//  resto del texto. Desaparece mientras alguien lo conduce.
// ---------------------------------------------------------------------
stock Dropeo_CrearCartel(indice)
{
    if (!VehiculoValido(indice)) return 0;

    new Float:x, Float:y, Float:z;
    GetVehiclePos(g_Vehiculos[indice][veObjeto], x, y, z);

    g_Vehiculos[indice][veEtiqueta] = CreateDynamic3DTextLabel(" ", 0xFFFFFFFF,
        x, y, z + 1.8, 12.0, INVALID_PLAYER_ID, g_Vehiculos[indice][veObjeto], 0, 0, 0, -1, 30.0);

    Dropeo_ActualizarCartel(indice);
    return 1;
}

stock Dropeo_ActualizarCartel(indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (g_Vehiculos[indice][veEtiqueta] == STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID) return 0;

    new nombre[32], texto[224];
    Veh_NombreModelo(g_Vehiculos[indice][veModelo], nombre, sizeof nombre);

    new estado[24];
    format(estado, sizeof estado, g_Vehiculos[indice][veCerrado] ? "{FF3232}[CERRADO]" : "{32FF32}[ABIERTO]");

    if (g_Vehiculos[indice][vePropietario] == 0)
    {
        // Los coches NFS no llevan precio de catalogo: se negocian con
        // /nfsvendercoche, no tiene sentido mostrar el vePrecio base.
        if (g_Vehiculos[indice][veEsNFS])
        {
            format(texto, sizeof texto, "{32FF32}LIBRE (NFS)\n%s\n{3399FF}%s",
                estado, nombre);
        }
        else
        {
            new precio[24];
            FormatearDinero(g_Vehiculos[indice][vePrecio], precio, sizeof precio);
            format(texto, sizeof texto, "{32FF32}LIBRE\n{FFFFFF}%s\n%s\n{3399FF}%s",
                precio, estado, nombre);
        }
    }
    else
    {
        new dueno[MAX_PLAYER_NAME + 1];
        format(dueno, sizeof dueno, "%s", g_Vehiculos[indice][vePropietarioNombre]);
        SubrayadoAEspacio(dueno);

        new dias = 0;
        if (g_Vehiculos[indice][veDropeoExpira] > 0)
        {
            dias = (g_Vehiculos[indice][veDropeoExpira] - gettime()) / 86400;
            if (dias < 0) dias = 0;
        }

        format(texto, sizeof texto, "{FFFF00}Propietario: %s\n{FFFFFF}Quedan %d dia(s)\n%s\n{3399FF}%s",
            dueno, dias, estado, nombre);
    }

    // Linea extra, siempre al final del todo, para no chocar con
    // ninguna de las lineas de arriba -- solo mientras esta publicado
    // en el Mercado Libre (34_mercado.inc, Mercado_Congelar/Descongelar).
    if (g_Vehiculos[indice][veEnVenta])
    {
        strcat(texto, "\n{E8B33A}A LA VENTA - Mercado Libre", sizeof texto);
    }

    // Rastreador GPS (13_vehiculos.inc): igual que "A LA VENTA", linea
    // extra siempre al final del todo, esta vez la ultima de todas.
    if (g_Vehiculos[indice][veTieneRastreador])
    {
        strcat(texto, "\n{FF3232}Localizador ON", sizeof texto);
    }

    UpdateDynamic3DTextLabelText(g_Vehiculos[indice][veEtiqueta], 0xFFFFFFFF, texto);

    // A prueba de fallos: si hay alguien dentro, el cartel se queda
    // oculto pase lo que pase tras un refresco de texto (algunas
    // actualizaciones del streamer pueden reactivar la visibilidad).
    // Dropeo_RefrescarCartelForzado() lo vuelve a mostrar aposta cuando
    // hace falta (por ejemplo, /lock desde dentro).
    if (g_DropeoOcupantes[indice] > 0)
    {
        Streamer_ToggleItemStatic(STREAMER_TYPE_3D_TEXT_LABEL, _:g_Vehiculos[indice][veEtiqueta], 0);
    }
    return 1;
}

// Para /lock (13_vehiculos.inc, otro archivo): refresca el cartel YA
// MISMO aunque alguien vaya dentro, y si es el caso reinicia la cuenta
// atras de 2s + parpadeo para que vuelva a desaparecer solo.
stock Dropeo_RefrescarCartelForzado(indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (g_Vehiculos[indice][veTipo] != VEH_DROPEO) return 0;
    if (g_Vehiculos[indice][veEtiqueta] == STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID) return 0;

    Dropeo_ActualizarCartel(indice);

    if (g_DropeoOcupantes[indice] > 0)
    {
        Streamer_ToggleItemStatic(STREAMER_TYPE_3D_TEXT_LABEL, _:g_Vehiculos[indice][veEtiqueta], 1);
        g_DropeoParpadeoGen[indice]++;
        SetTimerEx("Dropeo_CartelParpadeo", 2000, false, "ddd", indice, g_DropeoParpadeoGen[indice], 0);
    }
    return 1;
}

// Cuantos coches libres (VEH_DROPEO sin dueno) hay ahora mismo en el
// mapa. Usado por el contador en pantalla de 30_gps.inc.
stock Dropeo_ContarLibres()
{
    new total = 0;
    for (new i = 0; i < MAX_VEHICULOS; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] != 0) continue;
        total++;
    }
    return total;
}

// Oculta/muestra el cartel mientras alguien va al volante. Se llama
// desde OnPlayerStateChange en 99_callbacks.inc.
// Cuenta cuanta gente hay dentro (conductor + pasajeros) para saber
// cuando ocultar/mostrar el cartel: con varios ocupantes, el ultimo en
// bajarse es quien lo vuelve a mostrar, no el primero.
//
// Ya no se oculta al instante al subirte: se queda 2 segundos visible,
// parpadea dos veces y entonces desaparece. g_DropeoParpadeoGen invalida
// la secuencia pendiente si te bajas antes de que termine (o si vuelve a
// subir alguien mientras tanto).
stock Dropeo_AlSubir(indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (g_Vehiculos[indice][veTipo] != VEH_DROPEO) return 0;
    if (g_Vehiculos[indice][veEtiqueta] == STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID) return 0;

    g_DropeoOcupantes[indice]++;
    if (g_DropeoOcupantes[indice] == 1)
    {
        g_DropeoParpadeoGen[indice]++;
        SetTimerEx("Dropeo_CartelParpadeo", 2000, false, "ddd", indice, g_DropeoParpadeoGen[indice], 0);
    }
    return 1;
}

stock Dropeo_AlBajar(indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (g_Vehiculos[indice][veTipo] != VEH_DROPEO) return 0;
    if (g_Vehiculos[indice][veEtiqueta] == STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID) return 0;

    if (g_DropeoOcupantes[indice] > 0) g_DropeoOcupantes[indice]--;

    if (g_DropeoOcupantes[indice] == 0)
    {
        g_DropeoParpadeoGen[indice]++; // invalida cualquier parpadeo pendiente
        Dropeo_ActualizarCartel(indice);
        Streamer_ToggleItemStatic(STREAMER_TYPE_3D_TEXT_LABEL, _:g_Vehiculos[indice][veEtiqueta], 1);
    }
    return 1;
}

// paso 0,2,4 = ocultar · paso 1,3 = mostrar (dos parpadeos completos) ·
// tras el paso 4 se queda oculto para siempre, hasta el proximo AlBajar.
forward Dropeo_CartelParpadeo(indice, generacion, paso);
public Dropeo_CartelParpadeo(indice, generacion, paso)
{
    if (!VehiculoValido(indice)) return 0;
    if (generacion != g_DropeoParpadeoGen[indice]) return 0;
    if (g_DropeoOcupantes[indice] == 0) return 0;
    if (g_Vehiculos[indice][veEtiqueta] == STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID) return 0;

    new bool:mostrar = ((paso % 2) == 1);
    Streamer_ToggleItemStatic(STREAMER_TYPE_3D_TEXT_LABEL, _:g_Vehiculos[indice][veEtiqueta], mostrar ? 1 : 0);

    if (paso < 4)
    {
        SetTimerEx("Dropeo_CartelParpadeo", 200, false, "ddd", indice, generacion, paso + 1);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  CADUCIDAD  (revision cada hora)
// ---------------------------------------------------------------------
forward DropeoTicHora();
public  DropeoTicHora()
{
    new ahora = gettime();

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] == 0) continue;
        if (g_Vehiculos[i][veDropeoExpira] == 0 || g_Vehiculos[i][veDropeoExpira] > ahora) continue;

        // Coches de Jugadores (08/09/2026): si esta retirado por
        // Federico (buscado ahora o escondido en su ventana de 48h), NO
        // se le quita el propietario aqui aunque los papeles ya hayan
        // caducado -- la caducidad SIGUE corriendo (no se toca
        // veDropeoExpira), pero la desposesion se salta a proposito
        // mientras esta protegido. Encargo explicito: "caducidad de
        // papeles != perdida de propiedad durante la retirada de
        // Federico". Sin este corte, este mismo barrido (que nunca
        // comprueba si el objeto fisico existe) desposeeria en silencio
        // un vehiculo que Federico tiene escondido/destruido ahora mismo.
        if (FedJugadores_EstaProtegido(g_Vehiculos[i][veID])) continue;

        // Caduco: vuelve al pool de libres, y el coche vuelve fisicamente
        // a su ultimo sitio aparcado (veX/veY/veZ/veA  el que fijo el
        // dueno con /park, o el de creacion si nunca lo movio).
        new duenoID = g_Vehiculos[i][vePropietario];
        g_Vehiculos[i][vePropietario] = 0;
        g_Vehiculos[i][vePropietarioNombre][0] = EOS;
        g_Vehiculos[i][veDropeoExpira] = 0;
        g_Vehiculos[i][veCerrado] = false;
        // Solo se recalcula si nadie le ha puesto precio a mano con
        // /cambiarprecio (vePrecioManual). Ver migracion 030.
        if (!g_Vehiculos[i][vePrecioManual])
        {
            g_Vehiculos[i][vePrecio] = Dropeo_PrecioParaLibre(g_Vehiculos[i][veModelo]);
        }

        SetVehiclePos(g_Vehiculos[i][veObjeto], g_Vehiculos[i][veX], g_Vehiculos[i][veY], g_Vehiculos[i][veZ]);
        SetVehicleZAngle(g_Vehiculos[i][veObjeto], g_Vehiculos[i][veA]);
        LinkVehicleToInterior(g_Vehiculos[i][veObjeto], g_Vehiculos[i][veInterior]);
        SetVehicleVirtualWorld(g_Vehiculos[i][veObjeto], g_Vehiculos[i][veMundo]);

        Veh_ActualizarCierreTodos(i);
        Dropeo_ForzarMotorApagado(i);
        Veh_Guardar(i);
        Dropeo_ActualizarCartel(i);

        for (new p = 0; p < MAX_PLAYERS; p++)
        {
            if (!IsPlayerConnected(p) || !Jugador[p][jLogueado]) continue;
            if (Jugador[p][jID] != duenoID) continue;
            Aviso(p, "Los papeles de tu coche de la calle han caducado. Ha vuelto a su ultimo sitio aparcado y ya es libre otra vez.");
        }

        format(g_Buffer, sizeof g_Buffer, "CADUCAR  matricula=%s  cuenta=%d", g_Vehiculos[i][veMatricula], duenoID);
        EscribirLog("dropeo", g_Buffer);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  /COMPRARCOCHE
// ---------------------------------------------------------------------
CMD:comprarcoche(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del coche que quieres comprar.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_DROPEO)
    {
        Error(playerid, "Este no es un coche libre del sistema de dropeo.");
        return 1;
    }

    if (g_Vehiculos[indice][veEsNFS])
    {
        Error(playerid, "Contacta con un agente NFS para comprar este vehiculo (mediante ROL).");
        return 1;
    }

    if (g_Vehiculos[indice][vePropietario] != 0)
    {
        if (g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID])
            Error(playerid, "Este coche ya es tuyo.");
        else
            Error(playerid, "Este coche ya tiene dueno.");
        return 1;
    }

    // Los modelos que Federico este buscando esta ronda no se pueden
    // comprar -- el unico destino posible es entregarselos a el
    // (43_federico.inc). Puentealo con /puente y usa /entregarcoche.
    if (Federico_SlotDeModelo(g_Vehiculos[indice][veModelo]) != -1)
    {
        Error(playerid, "Federico busca este coche esta ronda -- no se puede comprar, solo entregarselo con /entregarcoche.");
        return 1;
    }

    new precio = g_Vehiculos[indice][vePrecio];
    if (!TieneDinero(playerid, precio))
    {
        new falta[24];
        FormatearDinero(precio, falta, sizeof falta);
        format(g_Buffer, sizeof g_Buffer, "Necesitas %s en efectivo para comprarlo.", falta);
        Error(playerid, g_Buffer);
        return 1;
    }

    DarDinero(playerid, -precio);
    Negocio_IngresoComprarCoche(precio);   // 100% a la caja fuerte de la DGT, si tiene dueno

    new nombre[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);
    GetPlayerName(playerid, cuenta, sizeof cuenta);

    g_Vehiculos[indice][vePropietario] = Jugador[playerid][jID];
    format(g_Vehiculos[indice][vePropietarioNombre], MAX_PLAYER_NAME + 1, "%s", cuenta);
    g_Vehiculos[indice][veDropeoExpira] = gettime() + (DROPEO_DIAS_PROPIEDAD * 86400);
    g_Vehiculos[indice][veAvisado24h] = false;
    g_DropeoArrancado[indice] = true;

    // Encargo de Vlad, 06/09/2026: comprar un coche libre no debe
    // regalar de paso el botin del sorteo de maleteros (45_maletero.inc)
    // -- si le habia tocado algo, se pierde al comprarlo.
    Maletero_VaciarTodo(indice);

    Veh_Guardar(indice);
    Dropeo_ActualizarCartel(indice);

    new precioTxt[24];
    FormatearDinero(precio, precioTxt, sizeof precioTxt);
    format(g_Buffer, sizeof g_Buffer, "Comprado por %s.", precioTxt);
    Exito(playerid, g_Buffer);

    // Aviso de legalizacion: "(DGT)" en azul, el aviso en blanco, y el
    // recordatorio del taller en rojo.
    format(g_Buffer, sizeof g_Buffer,
        PREFIJO "{5A9BD5}(DGT) {FFFFFF}Vehiculo comprado - Papeles duran %d dias {FF0000}(/Taller)",
        DROPEO_DIAS_PROPIEDAD);
    SendClientMessage(playerid, 0xFFFFFFFF, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "* %s paga en mano por un coche de la calle y se lo queda.", nombre);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "COMPRAR_LIBRE  actor=%d  matricula=%s  precio=%d",
        Jugador[playerid][jID], g_Vehiculos[indice][veMatricula], precio);
    EscribirLog("dropeo", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  /ABANDONARCOCHE
//  Vende tu vehiculo (de catalogo o comprado en la calle) por un precio
//  simbolico, sin tener que llevarlo al taller: se queda ahi mismo,
//  libre para quien lo quiera, y esa posicion pasa a ser su nuevo /park
//  (mismo mecanismo que Veh_Aparcar). Distinto de /vendervehiculo, que
//  paga precio real de catalogo pero exige estar en el taller clandestino.
// ---------------------------------------------------------------------
CMD:abandonarcoche(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del vehiculo que quieres abandonar.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (indice == -1
        || (g_Vehiculos[indice][veTipo] != VEH_PERSONAL && g_Vehiculos[indice][veTipo] != VEH_DROPEO)
        || g_Vehiculos[indice][vePropietario] != Jugador[playerid][jID])
    {
        Error(playerid, "Este vehiculo no es tuyo.");
        return 1;
    }

    // Coches de Jugadores (08/09/2026): no dejar que el propio dueno se
    // escape de la mecanica abandonando el coche mientras Federico lo
    // esta buscando -- este comando no pasa por Dropeo_EsPuenteable ni
    // Dropeo_EsDropeable, necesita su propio corte.
    if (FedJugadores_EstaProtegido(g_Vehiculos[indice][veID]))
    {
        Error(playerid, "Este vehiculo esta siendo buscado por Federico ahora mismo, no puedes abandonarlo.");
        return 1;
    }

    g_Vehiculos[indice][veTipo] = VEH_DROPEO;
    g_Vehiculos[indice][vePropietario] = 0;
    g_Vehiculos[indice][vePropietarioNombre][0] = EOS;
    g_Vehiculos[indice][veDropeoExpira] = 0;
    g_Vehiculos[indice][veEsNFS] = false;
    g_Vehiculos[indice][veCerrado] = false;

    Veh_Aparcar(indice);   // guarda la posicion actual como su nuevo /park y lo persiste

    if (g_Vehiculos[indice][veEtiqueta] == STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID)
        Dropeo_CrearCartel(indice);
    else
        Dropeo_ActualizarCartel(indice);

    Veh_ActualizarCierreTodos(indice);

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);
    RemovePlayerFromVehicle(playerid);

    DarDinero(playerid, ABANDONO_PRECIO);

    new dinero[24];
    FormatearDinero(ABANDONO_PRECIO, dinero, sizeof dinero);
    format(g_Buffer, sizeof g_Buffer,
        PREFIJO "{5A9BD5}(DGT) {FFFFFF}Vehiculo abandonado, queda libre (%s).", dinero);
    SendClientMessage(playerid, 0xFFFFFFFF, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "* %s se baja del coche, coge sus cosas y lo deja ahi tirado.", nombre);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "ABANDONAR  actor=%d  matricula=%s",
        Jugador[playerid][jID], g_Vehiculos[indice][veMatricula]);
    EscribirLog("dropeo", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  /PUENTE
// ---------------------------------------------------------------------
CMD:puente(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaHerido(playerid))   return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante para puentear.");
        return 1;
    }

    if (g_DropeoPuenteando[playerid])
    {
        Error(playerid, "Ya estas puenteando un coche. Espera.");
        return 1;
    }

    new ahora = gettime();
    if (g_DropeoCooldown[playerid] > ahora)
    {
        new espera = g_DropeoCooldown[playerid] - ahora;
        format(g_Buffer, sizeof g_Buffer,
            "Has forzado demasiadas cerraduras. Espera %d min %d s.",
            espera / 60, espera % 60);
        Error(playerid, g_Buffer);
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (indice == -1)
    {
        Error(playerid, "Este vehiculo no pertenece al sistema.");
        return 1;
    }

    if (!Dropeo_EsPuenteable(playerid, indice))
    {
        Error(playerid, "Este vehiculo no se puede puentear.");
        return 1;
    }

    if (g_DropeoArrancado[indice])
    {
        Error(playerid, "Este vehiculo ya esta arrancado.");
        return 1;
    }

    if (g_DropeoOcupado[indice])
    {
        Error(playerid, "Alguien lo esta puenteando ahora mismo.");
        return 1;
    }

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(g_Vehiculos[indice][veObjeto], motor, luces, alarma, puertas, capo, maletero, objetivo);
    if (motor == VEHICLE_PARAMS_ON)
    {
        Error(playerid, "El motor ya esta arrancado.");
        return 1;
    }

    g_DropeoPuenteando[playerid]   = true;
    g_DropeoPuenteIndice[playerid] = indice;
    g_DropeoOcupado[indice]        = true;

    TogglePlayerControllable(playerid, false);

    // El aviso de chat "puenteando... aguanta 4 segundos" se quito el
    // 08/09/2026 -- la cuenta atras visual de aqui abajo (PlayerTextDraw
    // con el numero de segundos restantes) ya cubre esa misma info.
    g_DropeoPuenteTimer[playerid] =
        SetTimerEx("DropeoPuenteFin", DROPEO_PUENTE_FREEZE_MS, false, "i", playerid);

    // Cuenta atras visual (coherencia audiovisual, 08/09/2026) -- la
    // duracion REAL sigue siendo DROPEO_PUENTE_FREEZE_MS, sin tocar; el
    // numero inicial se calcula a partir de esa constante, nunca
    // hardcodeado, para que se adapte solo si algun dia cambia.
    PlayerPlaySound(playerid, 1133, 0.0, 0.0, 0.0);   // identidad audiovisual: mismo sonido que /arreglarcoche, UNA sola vez
    g_DropeoPuenteRestante[playerid] = DROPEO_PUENTE_FREEZE_MS / 1000;
    new textoInicial[4];
    format(textoInicial, sizeof textoInicial, "%d", g_DropeoPuenteRestante[playerid]);
    PlayerTextDrawSetString(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO], textoInicial);
    PlayerTextDrawShow(playerid, g_DropeoPuenteTD[playerid][DPT_TITULO]);
    PlayerTextDrawShow(playerid, g_DropeoPuenteTD[playerid][DPT_SUBTITULO]);
    g_DropeoPuenteTicTimer[playerid] = SetTimerEx("Dropeo_PuenteTic", 1000, true, "i", playerid);
    return 1;
}

forward DropeoPuenteFin(playerid);
public  DropeoPuenteFin(playerid)
{
    if (!IsPlayerConnected(playerid) || !g_DropeoPuenteando[playerid]) return 0;

    g_DropeoPuenteando[playerid] = false;
    g_DropeoPuenteTimer[playerid] = -1;

    if (g_DropeoPuenteTicTimer[playerid] != -1)
    {
        KillTimer(g_DropeoPuenteTicTimer[playerid]);
        g_DropeoPuenteTicTimer[playerid] = -1;
    }
    Dropeo_PuenteOcultarAviso(playerid);

    TogglePlayerControllable(playerid, true);

    new indice = g_DropeoPuenteIndice[playerid];
    g_DropeoPuenteIndice[playerid] = -1;

    if (!VehiculoValido(indice))
    {
        Error(playerid, "El vehiculo ya no esta disponible.");
        return 0;
    }
    g_DropeoOcupado[indice] = false;

    if (GetPlayerVehicleID(playerid) != g_Vehiculos[indice][veObjeto]
        || GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Te bajaste del coche antes de terminar el puente.");
        return 0;
    }

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(g_Vehiculos[indice][veObjeto], motor, luces, alarma, puertas, capo, maletero, objetivo);

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);

    if (random(100) < DROPEO_PUENTE_EXITO)
    {
        g_DropeoArrancado[indice] = true;
        Teletipo_RegistrarPuenteExito(); // 62_teletipo.inc -- solo cuenta para el aviso de racha, no cambia nada de aqui
        SetVehicleParamsEx(g_Vehiculos[indice][veObjeto],
            VEHICLE_PARAMS_ON, luces, alarma, VEHICLE_PARAMS_OFF, capo, maletero, objetivo);
        Veh_ActualizarCierreTodos(indice);

        // Alarma (13_vehiculos.inc): si el vehiculo la tiene comprada,
        // esto sobreescribe el "alarma" de arriba con la nativa sonando.
        Alarma_Disparar(indice);

        GameTextForPlayer(playerid, "~g~PUENTE EXITOSO", 3000, 3);

        format(g_Buffer, sizeof g_Buffer,
            "* %s fuerza el contacto y consigue arrancar el vehiculo.", nombre);
        MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, g_Buffer);

        // Si Federico busca este modelo, el destino es Federico -- nada
        // de taller NFS, /dropearcoche ni aviso de robo (no hay victima:
        // el coche no tiene dueno, vePropietario ya es 0). El checkpoint
        // que puso Federico_AlEntrarComoConductor al subir se pisa aqui
        // (el puente tarda unos segundos y este handler corre despues),
        // asi que hay que volver a ponerlo.
        if (g_Vehiculos[indice][vePropietario] == 0 && Federico_SlotDeModelo(g_Vehiculos[indice][veModelo]) != -1)
        {
            SetPlayerCheckpoint(playerid, FEDERICO_X, FEDERICO_Y, FEDERICO_Z, FEDERICO_RADIO_ENTREGA);
            Info(playerid, "Este vehiculo lo busca Federico. Llevalo hasta el y usa /entregarcoche.");
        }
        else if (g_Vehiculos[indice][vePropietario] == 0)
        {
            // Coche libre normal (05/09/2026, peticion expresa de Vlad):
            // arrancarlo NO pasa por el taller de la NFS ni se trata como
            // un robo real -- no hay victima, es solo cogerlo un rato.
            // Sin esto, cualquier coche libre puenteable caia en la rama
            // de "ajeno" de abajo. SI hay un aviso aparte (AVISO_PUENTE_LIBRE,
            // 01_configuracion.inc), 60% de probabilidad, SOLO en el
            // puente positivo -- un intento fallido no avisa a nadie
            // (a diferencia de /forzarmaletero, que tira la alerta
            // siempre, gane o pierda).
            // Aviso de chat "Motor arrancado." retirado el 08/09/2026: ya
            // esta el GameText "PUENTE EXITOSO" de arriba y el /me
            // automatico de mas abajo, de sobra para el mismo hecho.

            new Float:avx, Float:avy, Float:avz;
            GetPlayerPos(playerid, avx, avy, avz);
            Policia_CrearAviso(AVISO_PUENTE_LIBRE, playerid, avx, avy, avz, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));
        }
        else
        {
            SetPlayerCheckpoint(playerid, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z, 5.0);
            Info(playerid, "Vehiculo ajeno. Llevalo al taller de la NFS y usa /dropearcoche.");

            // Aviso a la policia (37_policia.inc). Puentear un coche ajeno
            // con exito ES el robo: es aqui y no en /dropearcoche, que ya
            // es venderlo.
            new Float:avx, Float:avy, Float:avz;
            GetPlayerPos(playerid, avx, avy, avz);
            Policia_CrearAviso(AVISO_ROBO_COCHE, playerid, avx, avy, avz, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));
        }

        g_DropeoIntentos[playerid] = 0;
    }
    else
    {
        GameTextForPlayer(playerid, "~r~FALLO AL PUENTEAR", 2500, 3);
        // El mensaje de chat "No conseguiste puentearlo." se quito el
        // 10/09/2026 (peticion expresa de Vlad): el GameText de arriba ya
        // lo deja claro a pantalla completa, era redundante -- mismo
        // criterio ya aplicado al mensaje de exito ("Motor arrancado.")
        // el 08/09/2026, ver mas abajo.

        format(g_Buffer, sizeof g_Buffer,
            "* %s manipula los cables del contacto sin exito.", nombre);
        MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, g_Buffer);

        g_DropeoIntentos[playerid]++;
        if (g_DropeoIntentos[playerid] >= DROPEO_PUENTE_FALLOS)
        {
            g_DropeoIntentos[playerid] = 0;
            g_DropeoCooldown[playerid] = gettime() + DROPEO_PUENTE_ESPERA;
            format(g_Buffer, sizeof g_Buffer,
                "Has fallado %d veces seguidas. Espera %d minutos antes de intentarlo otra vez.",
                DROPEO_PUENTE_FALLOS, DROPEO_PUENTE_ESPERA / 60);
            Aviso(playerid, g_Buffer);
        }
    }
    return 1;
}

// ---------------------------------------------------------------------
//  ACCIONES DEL TALLER  (compartidas entre /dropearcoche, /renovarpapeles
//  y el menu /taller  una unica logica, varias puertas de entrada)
// ---------------------------------------------------------------------

// Renueva TUS papeles (VEH_DROPEO comprado con /comprarcoche). Solo a
// partir del sexto dia (cuando quedan DROPEO_RENOVAR_DIAS_ANTES dias o
// menos): antes de eso todavia te sobra tiempo.
stock Dropeo_RenovarPapeles(playerid, indice, precio = NFS_RENOVAR_PRECIO)
{
    if (!VehiculoValido(indice) || g_Vehiculos[indice][veTipo] != VEH_DROPEO
        || g_Vehiculos[indice][vePropietario] != Jugador[playerid][jID])
    {
        Error(playerid, "Ese coche no es tuyo.");
        return 0;
    }

    new restante = g_Vehiculos[indice][veDropeoExpira] - gettime();
    if (restante > DROPEO_RENOVAR_DIAS_ANTES * 86400)
    {
        new diasRestantes = restante / 86400;
        format(g_Buffer, sizeof g_Buffer,
            "Todavia no puedes renovar. Te quedan %d dias de papeles; podras renovar a partir del dia %d.",
            diasRestantes, DROPEO_DIAS_PROPIEDAD - DROPEO_RENOVAR_DIAS_ANTES);
        Error(playerid, g_Buffer);
        return 0;
    }

    if (!TieneDinero(playerid, precio))
    {
        format(g_Buffer, sizeof g_Buffer, "Necesitas $%d para renovar los papeles.", precio);
        Error(playerid, g_Buffer);
        return 0;
    }

    DarDinero(playerid, -precio);
    g_Vehiculos[indice][veDropeoExpira] = gettime() + (DROPEO_DIAS_PROPIEDAD * 86400);
    g_Vehiculos[indice][veAvisado24h] = false;
    Veh_Guardar(indice);
    Dropeo_ActualizarCartel(indice);

    format(g_Buffer, sizeof g_Buffer, "Papeles renovados por $%d. Te quedan %d dias.",
        precio, DROPEO_DIAS_PROPIEDAD);
    Exito(playerid, g_Buffer);
    return 1;
}

// Dropea (vende) un coche AJENO que hayas puenteado con /puente: le
// quita los papeles al dueno y vuelve al pool de libres. Esto es lo
// unico que hace /dropearcoche  nada de renovar papeles propios.
stock Dropeo_DropearAjeno(playerid, indice)
{
    if (!Dropeo_EsDropeable(playerid, indice))
    {
        Error(playerid, "No puedes hacer nada con este coche aqui.");
        return 0;
    }

    new nombreDropero[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombreDropero, sizeof nombreDropero);
    Dropeo_NotificarDueno(indice, nombreDropero);

    g_Vehiculos[indice][veTipo]              = VEH_DROPEO;
    g_Vehiculos[indice][vePropietario]       = 0;
    g_Vehiculos[indice][vePropietarioNombre][0] = EOS;
    g_Vehiculos[indice][veDropeoExpira]      = 0;
    g_Vehiculos[indice][veCerrado]           = false;
    // Igual que en la caducidad: respeta el precio puesto a mano.
    if (!g_Vehiculos[indice][vePrecioManual])
    {
        g_Vehiculos[indice][vePrecio]        = Dropeo_PrecioParaLibre(g_Vehiculos[indice][veModelo]);
    }
    // false, NO true: Dropeo_ForzarMotorApagado (linea de abajo) apaga
    // el motor de verdad, y desde el 12/08/2026 TODOS los vehiculos
    // nacen/vuelven apagados (Veh_Aparecer, 13_vehiculos.inc) -- hay que
    // volver a puentearlo. Estaba puesto a "true" por error: el coche
    // quedaba marcado como ya arrancado sin estarlo de verdad, y tanto
    // /motor como /puente lo rechazaban sin que hubiera forma de
    // arrancarlo. Bug real, encontrado el 03/09/2026 al intentar
    // puentear un coche de Federico que ya habia pasado por aqui.
    g_DropeoArrancado[indice]                = false;
    Veh_ActualizarCierreTodos(indice);
    Dropeo_ForzarMotorApagado(indice);
    Veh_Guardar(indice);

    if (g_Vehiculos[indice][veEtiqueta] == STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID)
    {
        Dropeo_CrearCartel(indice);
    }
    else
    {
        Dropeo_ActualizarCartel(indice);
    }

    DarDinero(playerid, DROPEO_PREMIO_DROPEO);

    format(g_Buffer, sizeof g_Buffer,
        "* %s dropea el vehiculo en el taller de la NFS.", nombreDropero);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, g_Buffer);

    new dineroTxt[24];
    FormatearDinero(DROPEO_PREMIO_DROPEO, dineroTxt, sizeof dineroTxt);
    format(g_Buffer, sizeof g_Buffer, "Has dropeado el coche. Te llevas %s.", dineroTxt);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer,
        "DROPEAR  actor=%s(%d)  matricula=%s  modelo=%d",
        nombreDropero, Jugador[playerid][jID],
        g_Vehiculos[indice][veMatricula], g_Vehiculos[indice][veModelo]);
    EscribirLog("dropeo", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  /DROPEARCOCHE  (taller de la NFS)
//  SOLO para coches AJENOS que hayas puenteado con /puente: les quita
//  los papeles al dueno y vuelven al pool de libres. Para renovar TUS
//  propios papeles usa /renovarpapeles o el menu /taller  esto NO es
//  lo mismo, dropear es quitarle el coche a otro.
// ---------------------------------------------------------------------
CMD:dropearcoche(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaHerido(playerid))   return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del coche.");
        return 1;
    }

    if (GetPlayerDistanceFromPoint(playerid, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z) > NFS_TALLER_RADIO)
    {
        Error(playerid, "Tienes que estar en el taller de la NFS.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_DROPEO && g_Vehiculos[indice][veTipo] != VEH_PERSONAL)
    {
        Error(playerid, "Este vehiculo no es del sistema de dropeo.");
        return 1;
    }

    if (g_Vehiculos[indice][veTipo] == VEH_DROPEO && g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID])
    {
        Error(playerid, "Este coche es tuyo. Para renovar los papeles usa /renovarpapeles o el menu /taller.");
        return 1;
    }

    if (g_Vehiculos[indice][veTipo] == VEH_DROPEO && g_Vehiculos[indice][vePropietario] == 0)
    {
        if (Federico_SlotDeModelo(g_Vehiculos[indice][veModelo]) != -1)
            Error(playerid, "Federico busca este coche. Usa /entregarcoche junto a el, no se dropea.");
        else
            Error(playerid, "Este coche todavia es libre. Compralo con /comprarcoche.");
        return 1;
    }

    Dropeo_DropearAjeno(playerid, indice);
    return 1;
}

// ---------------------------------------------------------------------
//  /RENOVARPAPELES  (taller de la NFS)
//  Renueva TUS papeles (el coche que compraste con /comprarcoche). Solo
//  a partir del sexto dia. Para lo mismo sin acordarte del comando, usa
//  el menu /taller.
// ---------------------------------------------------------------------
CMD:renovarpapeles(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaHerido(playerid))   return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del coche.");
        return 1;
    }

    if (GetPlayerDistanceFromPoint(playerid, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z) > NFS_TALLER_RADIO)
    {
        Error(playerid, "Tienes que estar en el taller de la NFS.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_DROPEO || g_Vehiculos[indice][vePropietario] != Jugador[playerid][jID])
    {
        Error(playerid, "Este coche no es tuyo.");
        return 1;
    }

    Dropeo_RenovarPapeles(playerid, indice, NFS_RENOVAR_PRECIO);
    return 1;
}

// ---------------------------------------------------------------------
//  /TALLER  ·  Menu del taller de la NFS
//  ---------------------------------------------------------------------
//  Una unica puerta de entrada: segun el coche en el que estes sentado,
//  ofrece renovar tus papeles o dropear uno ajeno puenteado. Reutiliza
//  Dropeo_RenovarPapeles() y Dropeo_DropearAjeno(), las mismas funciones
//  que usan /renovarpapeles y /dropearcoche.
// ---------------------------------------------------------------------
#define TALLER_OPC_RENOVAR  0
#define TALLER_OPC_DROPEAR  1

static g_TallerOpcion[MAX_PLAYERS][2];
static g_TallerOpcionNum[MAX_PLAYERS];
static g_TallerVehiculo[MAX_PLAYERS];
static g_TallerPrecioRenovar[MAX_PLAYERS];

CMD:taller(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaHerido(playerid))   return 1;

    if (GetPlayerDistanceFromPoint(playerid, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z) > NFS_TALLER_RADIO)
    {
        Error(playerid, "No estas en el taller de la {FF0000}NFS{E05A5A}.");
        SetPlayerCheckpoint(playerid, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z, 5.0);   // mismo radio que GPS_RADIO_CHECKPOINT (30_gps.inc)
        return 1;
    }

    new precioRenovar = NFS_RENOVAR_PRECIO;

    new indice = -1;
    if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
    {
        indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    }

    new lista[300];
    lista[0] = EOS;
    g_TallerOpcionNum[playerid] = 0;
    g_TallerPrecioRenovar[playerid] = precioRenovar;

    if (indice != -1 && g_Vehiculos[indice][veTipo] == VEH_DROPEO && g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID])
    {
        new restante = g_Vehiculos[indice][veDropeoExpira] - gettime();
        new dias = restante / 86400;

        if (restante <= DROPEO_RENOVAR_DIAS_ANTES * 86400)
        {
            format(g_Buffer, sizeof g_Buffer, "Renovar papeles ($%d)  %d dia(s) restantes\n", precioRenovar, dias);
        }
        else
        {
            format(g_Buffer, sizeof g_Buffer, "{888888}Renovar papeles (aun no  quedan %d dias)\n", dias);
        }
        strcat(lista, g_Buffer, sizeof lista);
        g_TallerOpcion[playerid][g_TallerOpcionNum[playerid]] = TALLER_OPC_RENOVAR;
        g_TallerOpcionNum[playerid]++;
    }

    if (indice != -1 && Dropeo_EsDropeable(playerid, indice))
    {
        strcat(lista, "Dropear (vender) este coche ajeno\n", sizeof lista);
        g_TallerOpcion[playerid][g_TallerOpcionNum[playerid]] = TALLER_OPC_DROPEAR;
        g_TallerOpcionNum[playerid]++;
    }

    if (g_TallerOpcionNum[playerid] == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_TALLER, DIALOG_STYLE_MSGBOX, "{E8973A}Taller de la NFS",
            "{B4B5B7}Aqui no hay nada que hacer ahora mismo.\n\n\
            {FFFFFF}Sube a un coche tuyo{B4B5B7} (comprado con /comprarcoche) para renovar sus papeles,\n\
            o a uno {FFFFFF}ajeno que hayas puenteado{B4B5B7} con /puente para dropearlo.",
            "Cerrar", "");
        return 1;
    }

    g_TallerVehiculo[playerid] = indice;
    ShowPlayerDialog(playerid, DIALOGO_TALLER, DIALOG_STYLE_LIST, "{E8973A}Taller de la NFS", lista, "Elegir", "Cerrar");
    return 1;
}

public Dropeo_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext
    if (dialogid != DIALOGO_TALLER) return 0;
    if (!response) return 1;

    if (listitem < 0 || listitem >= g_TallerOpcionNum[playerid]) return 1;

    new indice = g_TallerVehiculo[playerid];
    if (!VehiculoValido(indice))
    {
        Error(playerid, "Ese vehiculo ya no esta disponible.");
        return 1;
    }

    switch (g_TallerOpcion[playerid][listitem])
    {
        case TALLER_OPC_RENOVAR: Dropeo_RenovarPapeles(playerid, indice, g_TallerPrecioRenovar[playerid]);
        case TALLER_OPC_DROPEAR: Dropeo_DropearAjeno(playerid, indice);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /adminpark
//  ---------------------------------------------------------------------
//  Guarda la posicion y el angulo actuales de un vehiculo REGISTRADO
//  como su sitio fijo: donde reaparece al reiniciar el servidor. Para
//  cuando uno se queda mal colocado (metido en una pared, torcido...) y
//  hay que reajustarlo sin borrarlo y crear otro: lo llevas al sitio
//  bueno y guardas con este comando.
//
//  Vale para CUALQUIER tipo, no solo los coches libres del dropeo.
//  Empezo limitado a VEH_DROPEO por el modulo donde vive, pero el
//  problema es identico en los de trabajo y los de faccion: un coche
//  patrulla creado con /crearvehiculo aparece tres metros a la derecha
//  del admin, y colocar una flota entera clavada a la primera no pasa.
//  Ni /park ni /crearvehiculo cubren ese caso -- /park solo toca coches
//  a nombre de un jugador.
//
//  El personal (VEH_PERSONAL) tambien entra: si un admin tiene que
//  recolocar el coche de alguien, es este comando y no otro. Queda en
//  el log de dropeo, con matricula y coordenadas.
// ---------------------------------------------------------------------
#define ADMINPARK_RADIO 6.0

CMD:adminpark(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    // Al volante, o de pie junto al coche por si el motor apagado no
    // deja moverlo desde dentro.
    new indice = Veh_BuscarCercano(playerid, ADMINPARK_RADIO);

    if (indice == -1)
    {
        Error(playerid, "Ponte al volante (o de pie al lado) del vehiculo que quieres fijar.");
        return 1;
    }

    Veh_Aparcar(indice);

    // Decir de QUE coche se trata: con varios juntos en un garaje, un
    // "PARK!" a secas no aclara si has fijado el que creias.
    new nombre[48];
    Conce_NombreModelo(g_Vehiculos[indice][veModelo], nombre, sizeof nombre);

    format(g_Buffer, sizeof g_Buffer, "PARK! %s (%s) fijado aqui.",
        nombre, g_Vehiculos[indice][veMatricula]);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer,
        "ADMINPARK  admin=%d  matricula=%s  tipo=%d  x=%.1f y=%.1f z=%.1f a=%.1f",
        Jugador[playerid][jID], g_Vehiculos[indice][veMatricula], g_Vehiculos[indice][veTipo],
        g_Vehiculos[indice][veX], g_Vehiculos[indice][veY], g_Vehiculos[indice][veZ], g_Vehiculos[indice][veA]);
    EscribirLog("dropeo", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /creardropeo <modelo>
// ---------------------------------------------------------------------
CMD:creardropeo(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new modelo;
    if (sscanf(params, "d", modelo))
    {
        Uso(playerid, "/creardropeo <modelo (400-611)>");
        return 1;
    }

    if (modelo < 400 || modelo > 611)
    {
        Error(playerid, "Modelo fuera de rango (400-611).");
        return 1;
    }

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    new precio = Dropeo_PrecioParaLibre(modelo);
    new indice = Veh_Crear(modelo, 0, VEH_DROPEO, 0, 0, x, y + 3.0, z, a, -1, -1, 0, MUNDO_NORMAL, precio);
    if (indice == -1)
    {
        Error(playerid, "No hay hueco libre en la flota (MAX_VEHICULOS lleno).");
        return 1;
    }

    // Los coches libres nacen siempre abiertos (sin cerradura), pero el
    // motor SI nace apagado como cualquier otro vehiculo (Veh_Aparecer,
    // 13_vehiculos.inc, regla del 12/08/2026) -- hace falta /puente.
    // Antes decia "true" aqui, dejando el coche marcado como ya
    // arrancado sin estarlo de verdad (mismo bug que en
    // Dropeo_DropearAjeno, corregido el mismo dia).
    g_Vehiculos[indice][veCerrado] = false;
    g_DropeoArrancado[indice]      = false;
    Veh_ActualizarCierreTodos(indice);
    Veh_Guardar(indice);
    Dropeo_CrearCartel(indice);

    // Botin automatico tambien para los coches libres nuevos, no solo
    // para el pool que ya existia -- encargo de Vlad, 06/09/2026. Con
    // retraso porque el INSERT de Veh_Crear es asincrono: el veID real
    // (que Maletero_Guardar exige > 0) todavia no ha llegado en este
    // instante, mismo problema y misma solucion que
    // Federico_HistoricoCapturarID (43_federico.inc).
    SetTimerEx("Dropeo_CreardropeoTirarLoot", 500, false, "i", indice);

    format(g_Buffer, sizeof g_Buffer,
        "Coche libre creado. Modelo %d, matricula %s, precio $%d.",
        modelo, g_Vehiculos[indice][veMatricula], precio);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer,
        "CREAR_DROPEO  admin=%d  modelo=%d  matricula=%s  x=%.1f y=%.1f z=%.1f",
        Jugador[playerid][jID], modelo, g_Vehiculos[indice][veMatricula], x, y, z);
    EscribirLog("dropeo", g_Buffer);
    return 1;
}

forward Dropeo_CreardropeoTirarLoot(indice);
public  Dropeo_CreardropeoTirarLoot(indice)
{
    if (!Maletero_EsCandidatoLibre(indice)) return 0; // veID aun 0, o algo lo invalido en estos 500ms
    Maletero_TirarLootAleatorio(indice);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /customprueba
//  ---------------------------------------------------------------------
//  Prueba del primer modelo custom del servidor: el Volkswagen New
//  Beetle de YochiThMaster333, que en el cliente REEMPLAZA al Uranus
//  (modelo 558). El servidor no sabe nada del mod: spawnea un Uranus
//  normal como coche libre. Quien tenga el .dff/.txd instalado ve el
//  Beetle; quien no, ve el Uranus de siempre. Repartir esos archivos es
//  justo el trabajo del launcher.
//
//  Es un /creardropeo con el modelo clavado a 558 y el precio a
//  CUSTOMPRUEBA_PRECIO (el automatico daria 2.700$, no es lo pedido).
// ---------------------------------------------------------------------
#define CUSTOMPRUEBA_MODELO 558     // Uranus: lo que el mod reemplaza en el cliente
#define CUSTOMPRUEBA_PRECIO 5000

CMD:customprueba(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    new indice = Veh_Crear(CUSTOMPRUEBA_MODELO, 0, VEH_DROPEO, 0, 0, x, y + 3.0, z, a, -1, -1, 0, MUNDO_NORMAL, CUSTOMPRUEBA_PRECIO);
    if (indice == -1)
    {
        Error(playerid, "No hay hueco libre en la flota (MAX_VEHICULOS lleno).");
        return 1;
    }

    // Igual que /creardropeo: nace abierto, pero con el motor apagado
    // (hace falta /puente) -- ver el comentario de alli.
    g_Vehiculos[indice][veCerrado] = false;
    g_DropeoArrancado[indice]      = false;
    Veh_ActualizarCierreTodos(indice);
    Veh_Guardar(indice);
    Dropeo_CrearCartel(indice);

    // Precio sellado a mano para que el reciclaje de papeles caducados no
    // lo devuelva al automatico. El veID todavia llega por AlCrearVehiculo,
    // asi que el UPDATE espera a tenerlo (CustomPrueba_SellarPrecio).
    g_Vehiculos[indice][vePrecioManual] = true;
    SetTimerEx("CustomPrueba_SellarPrecio", 500, false, "dd", indice, 0);

    format(g_Buffer, sizeof g_Buffer,
        "Beetle de prueba creado. Modelo %d (Uranus), matricula %s, precio $%d.",
        CUSTOMPRUEBA_MODELO, g_Vehiculos[indice][veMatricula], CUSTOMPRUEBA_PRECIO);
    Exito(playerid, g_Buffer);
    Info(playerid, "Sin el mod instalado en el cliente veras un Uranus normal.");

    format(g_Buffer, sizeof g_Buffer,
        "CUSTOMPRUEBA  admin=%d  modelo=%d  matricula=%s  precio=%d  x=%.1f y=%.1f z=%.1f",
        Jugador[playerid][jID], CUSTOMPRUEBA_MODELO, g_Vehiculos[indice][veMatricula],
        CUSTOMPRUEBA_PRECIO, x, y, z);
    EscribirLog("dropeo", g_Buffer);
    return 1;
}

// El id de base de datos de un vehiculo recien creado llega de forma
// asincrona (AlCrearVehiculo). Reintentamos hasta tenerlo y entonces
// marcamos precio_manual, que es lo que impide que el reciclaje vuelva a
// poner el precio automatico.
#define CUSTOMPRUEBA_SELLAR_INTENTOS 10

forward CustomPrueba_SellarPrecio(indice, intento);
public CustomPrueba_SellarPrecio(indice, intento)
{
    if (!VehiculoValido(indice)) return 0;

    if (g_Vehiculos[indice][veID] == 0)
    {
        if (intento < CUSTOMPRUEBA_SELLAR_INTENTOS)
            SetTimerEx("CustomPrueba_SellarPrecio", 500, false, "dd", indice, intento + 1);
        return 0;
    }

    new consulta[128];
    format(consulta, sizeof consulta,
        "UPDATE vehiculos SET precio = %d, precio_manual = 1 WHERE id = %d",
        CUSTOMPRUEBA_PRECIO, g_Vehiculos[indice][veID]);
    mysql_pquery(g_BD, consulta);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /borrardropeo
//  ---------------------------------------------------------------------
//  Se usa DE PIE, fuera del coche (tambien vale si estas dentro, ya que
//  tu posicion coincide con la del vehiculo). Borra el coche libre del
//  sistema de dropeo mas cercano a ti; si no hay ninguno a menos de
//  BORRARDROPEO_RADIO metros, no borra nada.
// ---------------------------------------------------------------------
#define BORRARDROPEO_RADIO 2.0

CMD:borrardropeo(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new indice = Veh_BuscarCercano(playerid, BORRARDROPEO_RADIO, VEH_DROPEO);

    if (indice == -1)
    {
        format(g_Buffer, sizeof g_Buffer,
            "No hay ningun coche libre del sistema de dropeo a menos de %.0f metros.", BORRARDROPEO_RADIO);
        Error(playerid, g_Buffer);
        return 1;
    }

    new matricula[12];
    strmid(matricula, g_Vehiculos[indice][veMatricula], 0, sizeof matricula, sizeof matricula);

    Veh_Borrar(indice);

    format(g_Buffer, sizeof g_Buffer, "Coche libre %s borrado.", matricula);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "BORRAR_DROPEO  admin=%d  matricula=%s", Jugador[playerid][jID], matricula);
    EscribirLog("dropeo", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /cambiarprecio <precio | auto>
//  ---------------------------------------------------------------------
//  Cambia el precio de un coche libre, de forma permanente. Elige el
//  coche igual que /borrardropeo: el que llevas montado si vas en uno, y
//  si no el mas cercano dentro de CAMBIARPRECIO_RADIO metros.
//
//  El precio ya se guardaba en la base de datos (columna `precio`), pero
//  se RECALCULABA solo desde el catalogo del modelo cada vez que el coche
//  volvia al pool de libres (papeles caducados o alguien lo dropea). Por
//  eso este comando ademas marca `precio_manual` (migracion 030): con esa
//  bandera puesta, esos dos puntos de reciclaje ya no lo tocan.
//
//  "/cambiarprecio auto" quita la bandera y devuelve el coche al precio
//  calculado automaticamente segun su modelo.
// ---------------------------------------------------------------------
#define CAMBIARPRECIO_RADIO 2.0     // mismo criterio que BORRARDROPEO_RADIO
#define CAMBIARPRECIO_MAXIMO 100000000

CMD:cambiarprecio(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/cambiarprecio <precio | auto>");
        return 1;
    }

    // El coche en el que vas montado manda; si no vas en ninguno, el mas
    // cercano. Asi se puede usar tanto sentado dentro como de pie al lado.
    // Ojo: a pie GetPlayerVehicleID devuelve 0, que NO es lo mismo que
    // INVALID_VEHICLE_ID (lo unico contra lo que protege
    // Veh_BuscarPorObjeto), asi que hay que descartarlo aparte.
    new montado = GetPlayerVehicleID(playerid);
    new indice = (montado > 0) ? Veh_BuscarPorObjeto(montado) : -1;

    if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_DROPEO)
    {
        indice = -1;

        new Float:mejor = CAMBIARPRECIO_RADIO, Float:vx, Float:vy, Float:vz, Float:distancia;
        for (new i = 0; i < g_UltimoVehiculo; i++)
        {
            if (!g_Vehiculos[i][veUsado]) continue;
            if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;

            GetVehiclePos(g_Vehiculos[i][veObjeto], vx, vy, vz);
            distancia = GetPlayerDistanceFromPoint(playerid, vx, vy, vz);

            if (distancia < mejor) { mejor = distancia; indice = i; }
        }
    }

    if (indice == -1)
    {
        Formato("No hay ningun coche libre del sistema de dropeo a menos de %.0f metros, ni vas montado en uno.", CAMBIARPRECIO_RADIO);
        Error(playerid, g_Buffer);
        return 1;
    }

    new bool:volverAuto = (strcmp(params, "auto", true) == 0);
    new precio;

    if (volverAuto)
    {
        precio = Dropeo_PrecioParaLibre(g_Vehiculos[indice][veModelo]);
    }
    else
    {
        if (sscanf(params, "d", precio))
        {
            Uso(playerid, "/cambiarprecio <precio | auto>");
            return 1;
        }

        if (precio < 0 || precio > CAMBIARPRECIO_MAXIMO)
        {
            Formato("El precio tiene que estar entre 0 y %d$.", CAMBIARPRECIO_MAXIMO);
            Error(playerid, g_Buffer);
            return 1;
        }
    }

    new anterior = g_Vehiculos[indice][vePrecio];

    g_Vehiculos[indice][vePrecio]       = precio;
    g_Vehiculos[indice][vePrecioManual] = !volverAuto;

    new consulta[128];
    format(consulta, sizeof consulta,
        "UPDATE vehiculos SET precio = %d, precio_manual = %d WHERE id = %d",
        precio, volverAuto ? 0 : 1, g_Vehiculos[indice][veID]);
    mysql_pquery(g_BD, consulta);

    // El cartel sobre el coche muestra el precio cuando no tiene dueno.
    Dropeo_ActualizarCartel(indice);

    new dineroAntes[24], dineroAhora[24];
    FormatearDinero(anterior, dineroAntes, sizeof dineroAntes);
    FormatearDinero(precio, dineroAhora, sizeof dineroAhora);

    if (volverAuto)
    {
        Formato("Coche %s: precio devuelto al automatico, %s (antes %s).", g_Vehiculos[indice][veMatricula], dineroAhora, dineroAntes);
    }
    else
    {
        Formato("Coche %s: precio fijado en %s (antes %s). No se recalculara solo.", g_Vehiculos[indice][veMatricula], dineroAhora, dineroAntes);
    }
    Exito(playerid, g_Buffer);

    if (g_Vehiculos[indice][vePropietario] != 0)
    {
        Aviso(playerid, "Ojo: este coche tiene dueno ahora mismo. El precio nuevo se vera cuando vuelva a estar libre.");
    }

    format(g_Buffer, sizeof g_Buffer, "CAMBIAR_PRECIO  admin=%d  matricula=%s  antes=%d  ahora=%d  manual=%d",
        Jugador[playerid][jID], g_Vehiculos[indice][veMatricula], anterior, precio, volverAuto ? 0 : 1);
    EscribirLog("dropeo", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /listadropeo
// ---------------------------------------------------------------------
CMD:listadropeo(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new libres = 0, conPapeles = 0;
    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] == 0) libres++;
        else conPapeles++;
    }

    format(g_Buffer, sizeof g_Buffer,
        "Sistema de dropeo: {FFFFFF}%d libres{8BC4E8}, {FFFFFF}%d con papeles{8BC4E8}. Maximo pool: %d.",
        libres, conPapeles, MAX_VEHICULOS);
    Info(playerid, g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /tpdropeo <id BD>
// ---------------------------------------------------------------------
CMD:tpdropeo(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new idBD;
    if (sscanf(params, "d", idBD))
    {
        Uso(playerid, "/tpdropeo <id_bd>");
        return 1;
    }

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veID] != idBD) continue;

        SetPlayerPos(playerid,
            g_Vehiculos[i][veX] + 1.5,
            g_Vehiculos[i][veY],
            g_Vehiculos[i][veZ] + 0.5);
        SetPlayerInterior(playerid, g_Vehiculos[i][veInterior]);
        SetPlayerVirtualWorld(playerid, g_Vehiculos[i][veMundo]);

        format(g_Buffer, sizeof g_Buffer, "TP al vehiculo id %d (matricula %s).",
            idBD, g_Vehiculos[i][veMatricula]);
        Exito(playerid, g_Buffer);
        return 1;
    }

    Error(playerid, "No existe ningun vehiculo con ese id de BD.");
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /limpiardropeos <N>
//  ---------------------------------------------------------------------
//  Deja como mucho N coches SIN DUENO borrando los mas viejos. Los que
//  tienen papeles activos no se tocan.
CMD:limpiardropeos(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new limite;
    if (sscanf(params, "d", limite) || limite < 0)
    {
        Uso(playerid, "/limpiardropeos <cuantos_dejar>");
        return 1;
    }

    new indices[MAX_VEHICULOS], n = 0;
    for (new i = 0; i < g_UltimoVehiculo && n < MAX_VEHICULOS; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] != 0) continue;
        indices[n++] = i;
    }

    for (new i = 0; i < n - 1; i++)
    {
        for (new j = 0; j < n - 1 - i; j++)
        {
            if (g_Vehiculos[indices[j]][veID] > g_Vehiculos[indices[j+1]][veID])
            {
                new tmp = indices[j];
                indices[j] = indices[j+1];
                indices[j+1] = tmp;
            }
        }
    }

    new borrados = 0;
    for (new i = 0; i < n && n - borrados > limite; i++)
    {
        Veh_Borrar(indices[i]);
        borrados++;
    }

    format(g_Buffer, sizeof g_Buffer,
        "Limpieza completa. Borrados %d libres, quedan %d.",
        borrados, n - borrados);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer,
        "LIMPIAR_DROPEOS  admin=%d  limite=%d  borrados=%d",
        Jugador[playerid][jID], limite, borrados);
    EscribirLog("dropeo", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  HOOK · limpieza al borrar un vehiculo
// ---------------------------------------------------------------------
public Dropeo_AlBorrarVehiculo(indice)
{
    if (indice < 0 || indice >= MAX_VEHICULOS) return 0;
    g_DropeoOcupado[indice]   = false;
    g_DropeoArrancado[indice] = false;

    if (g_Vehiculos[indice][veEtiqueta] != STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID)
    {
        DestroyDynamic3DTextLabel(g_Vehiculos[indice][veEtiqueta]);
        g_Vehiculos[indice][veEtiqueta] = STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  /FORZAR  (compatibilidad con 20_desguace.inc)
//  Los coches libres ya no necesitan forzarse: nacen siempre abiertos.
//  Se deja la funcion para no romper el /forzar de los chatarreros.
// ---------------------------------------------------------------------
public Dropeo_IntentarForzar(playerid)
{
    Error(playerid, "Los coches libres de la calle ya estan abiertos. No hace falta forzarlos.");
    return 1;
}
