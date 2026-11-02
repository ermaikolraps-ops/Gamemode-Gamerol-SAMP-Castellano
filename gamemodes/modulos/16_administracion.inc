/* =====================================================================
 *  16_administracion.inc
 *  ---------------------------------------------------------------------
 *  Equipo administrativo de seis niveles.
 *
 *      6  Fundador            todo, y solo se obtiene por consola RCON
 *      5  Gestion             nombra admins hasta nivel 4
 *      4  Administrador Jefe  economia, vehiculos, trabajos y facciones
 *      3  Administrador       baneos, carcel y sanciones graves
 *      2  Moderador           expulsiones, silencios y congelaciones
 *      1  Soporte             atiende reportes y se mueve por el mapa
 *
 *  REGLA DE ORO: solo puedes dar un rango POR DEBAJO del tuyo, y nunca
 *  puedes sancionar a alguien de rango igual o superior al tuyo.
 *
 *  ------------------ COMO CONSEGUIR EL PRIMER FUNDADOR ------------------
 *  1. Entra al juego y pulsa T para abrir el chat.
 *  2. Escribe:  /rcon login <la contrasena de config.json>
 *  3. Escribe:  /fundador
 *  A partir de ahi ya puedes repartir rangos con /daradmin.
 *  -----------------------------------------------------------------------
 * ===================================================================== */

#define MAX_REPORTES            25

// Tipos de sancion para la tabla `sanciones`.
#define SANCION_AVISO           1
#define SANCION_KICK            2
#define SANCION_MUTE            3
#define SANCION_CARCEL          4
#define SANCION_BAN             5

enum E_REPORTE
{
    bool:reUsado,
    reJugador,
    reNombre[MAX_PLAYER_NAME + 1],
    reTexto[144],
    reSegundos,
    reAtiende
}
static g_Reportes[MAX_REPORTES][E_REPORTE];

// ---------------------------------------------------------------------
//  UTILIDADES
// ---------------------------------------------------------------------
public Admin_NombreNivel(nivel, destino[], tam)
{
    static const nombres[][] =
    {
        "Jugador",
        "Soporte",
        "Moderador",
        "Administrador",
        "Administrador Jefe",
        "Gestion",
        "Fundador"
    };

    if (nivel < 0 || nivel > ADMIN_FUNDADOR) nivel = 0;
    format(destino, tam, "%s", nombres[nivel]);
    return 1;
}

// Mensaje visible solo para el equipo administrativo.
public Admin_Aviso(const texto[])
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jAdmin] < ADMIN_SOPORTE) continue;
        SendClientMessage(i, COLOR_ADMIN_ACCION, texto);
    }
    return 1;
}

// Registra la accion y avisa al equipo.
stock Admin_Registrar(playerid, const accion[], const objetivo[], const detalles[])
{
    new admin[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, admin, sizeof admin);
    BD_RegistrarLog(admin, Jugador[playerid][jAdmin], accion, objetivo, detalles);
    return 1;
}

// Impide actuar sobre alguien de rango igual o superior.
stock bool:Admin_NoPuedeCon(playerid, objetivo)
{
    if (Jugador[objetivo][jAdmin] < Jugador[playerid][jAdmin]) return false;
    if (playerid == objetivo) return false;

    Error(playerid, "No puedes usar ese comando con alguien de tu mismo rango o superior.");
    return true;
}

// ---------------------------------------------------------------------
//  PRIMER FUNDADOR
// ---------------------------------------------------------------------
CMD:fundador(playerid, params[])
{
    if (NoLogueado(playerid)) return 1;

    if (!IsPlayerAdmin(playerid))
    {
        Error(playerid, "Este comando necesita que antes inicies sesion en la consola RCON.");
        Info(playerid, "Escribe: /rcon login <contrasena de config.json>");
        return 1;
    }

    if (Jugador[playerid][jAdmin] >= ADMIN_FUNDADOR)
    {
        Error(playerid, "Ya eres Fundador.");
        return 1;
    }

    Jugador[playerid][jAdmin] = ADMIN_FUNDADOR;
    BD_GuardarCuenta(playerid);

    new nombre[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, nombre, sizeof nombre);

    Exito(playerid, "Ahora eres Fundador del servidor. Escribe /admin para ver todos tus comandos.");
    Admin_Registrar(playerid, "FUNDADOR", nombre, "Autoascenso por RCON");

    format(g_Buffer, sizeof g_Buffer, "%s se ha nombrado Fundador via RCON", nombre);
    EscribirLog("administracion", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  PANEL Y LISTA DE COMANDOS
// ---------------------------------------------------------------------
CMD:admin(playerid, params[])
{
    if (NoLogueado(playerid)) return 1;
    if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

    Admin_MostrarComandos(playerid);
    return 1;
}

public Admin_MostrarComandos(playerid)
{
    new nivel = Jugador[playerid][jAdmin], rango[32];
    Admin_NombreNivel(nivel, rango, sizeof rango);

    new cuerpo[3600], titulo[80];
    format(titulo, sizeof titulo, "{E8973A}%s   ~   nivel %d de 6", rango, nivel);

    format(cuerpo, sizeof cuerpo,
        "{E8B33A}NIVEL 1  ~  SOPORTE\n\
        {FFFFFF}/panel{B4B5B7}             menu de moderacion por dialogos\n\
        {FFFFFF}/a{B4B5B7} texto           chat interno del equipo\n\
        {FFFFFF}/reportes{B4B5B7}          reportes pendientes\n\
        {FFFFFF}/atender{B4B5B7} id        te teletransporta al reporte\n\
        {FFFFFF}/ir{B4B5B7} id             te llevas hasta un jugador\n\
        {FFFFFF}/goto{B4B5B7} lugar        ls, grove... (o solo /ls, /grove)\n\
        {FFFFFF}/staff{B4B5B7}             on/off modo servicio\n\
        {FFFFFF}/revisar{B4B5B7} id        ficha completa de un jugador\n\
        {FFFFFF}/admins{B4B5B7}            equipo conectado\n\
        {FFFFFF}/pos{B4B5B7}               tus coordenadas exactas\n\n");

    if (nivel >= ADMIN_MODERADOR)
    {
        strcat(cuerpo,
            "{E8B33A}NIVEL 2  ~  MODERADOR\n\
            {FFFFFF}/traer{B4B5B7} id           traes a alguien hasta ti\n\
            {FFFFFF}/expulsar_jugador{B4B5B7} id motivo\n\
            {FFFFFF}/avisar{B4B5B7} id motivo   aviso formal (3 = baneo)\n\
            {FFFFFF}/mutear{B4B5B7} id min motivo\n\
            {FFFFFF}/desmutear{B4B5B7} id\n\
            {FFFFFF}/congelar{B4B5B7} id  ~  {FFFFFF}/descongelar{B4B5B7} id\n\
            {FFFFFF}/espectar{B4B5B7} id  ~  {FFFFFF}/noespectar{B4B5B7}\n\n", sizeof cuerpo);
    }

    if (nivel >= ADMIN_ADMIN)
    {
        strcat(cuerpo,
            "{E8B33A}NIVEL 3  ~  ADMINISTRADOR\n\
            {FFFFFF}/banear{B4B5B7} id dias motivo   (0 dias = permanente)\n\
            {FFFFFF}/desbanear{B4B5B7} nombre\n\
            {FFFFFF}/carcel{B4B5B7} id min motivo  ~  {FFFFFF}/soltar{B4B5B7} id\n\
            {FFFFFF}/desarmar{B4B5B7} id  ~  {FFFFFF}/curar{B4B5B7} id\n\
            {FFFFFF}/reparar{B4B5B7}                arregla tu vehiculo\n\
            {FFFFFF}/limpiarsuelo{B4B5B7}           borra los objetos tirados\n\
            {E8B33A}-- vehiculos --\n\
            {FFFFFF}/cambiarcolor{B4B5B7} c1 c2     repinta, y se queda asi\n\
            {FFFFFF}/adminpark{B4B5B7}              fija donde reaparece\n\
            {FFFFFF}/adminlock{B4B5B7}              entra saltandote el candado\n\
            {FFFFFF}/creardropeo{B4B5B7} modelo  ~  {FFFFFF}/borrardropeo{B4B5B7}\n\
            {FFFFFF}/cambiarprecio{B4B5B7} precio | auto\n\n", sizeof cuerpo);
    }

    if (nivel >= ADMIN_JEFE)
    {
        strcat(cuerpo,
            "{E8B33A}NIVEL 4  ~  ADMINISTRADOR JEFE\n\
            {FFFFFF}/darobjeto{B4B5B7} id objeto cantidad\n\
            {FFFFFF}/darskin{B4B5B7} id piel\n\
            {FFFFFF}/darjob{B4B5B7} id trabajo rango  ~  {FFFFFF}/quitarjob{B4B5B7} id trabajo\n\
            {FFFFFF}/darfaccion{B4B5B7} id faccion rango  ~  {FFFFFF}/quitarfaccion{B4B5B7} id\n\
            {FFFFFF}/crearvehiculo{B4B5B7} modelo tipo tipoid rango\n\
            {FFFFFF}/borrarvehiculo{B4B5B7}         el de al lado, para siempre\n\
            {FFFFFF}/crearcancela{B4B5B7} jugador modelo profundidad  ~  {FFFFFF}/movercancela{B4B5B7} jugador\n\
            {FFFFFF}/cancelaspendientes{B4B5B7}     quien esta esperando\n\n", sizeof cuerpo);
    }

    if (nivel >= ADMIN_GESTION)
    {
        strcat(cuerpo,
            "{E8B33A}NIVEL 5  ~  GESTION\n\
            {FFFFFF}/daradmin{B4B5B7} id nivel     invita a alguien al equipo\n\
            {FFFFFF}/darnivel{B4B5B7} id nivel     nivel de personaje\n\
            {FFFFFF}/liderfaccion{B4B5B7} id       nombra lider de faccion\n\
            {FFFFFF}/anuncio{B4B5B7} texto         mensaje a todo el servidor\n\
            {FFFFFF}/setbanco{B4B5B7}\n\
            {FFFFFF}/marcarcoord{B4B5B7} clave nombre  ~  {FFFFFF}/borrarcoord{B4B5B7} clave\n\
            {FFFFFF}/logs{B4B5B7}                  ultimas acciones del equipo\n\n", sizeof cuerpo);
    }

    if (nivel >= ADMIN_FUNDADOR)
    {
        strcat(cuerpo,
            "{E8B33A}NIVEL 6  ~  FUNDADOR\n\
            {FFFFFF}/daradmin{B4B5B7} id nivel     hasta nivel 5\n\
            {FFFFFF}/guardartodo{B4B5B7}           fuerza el guardado global\n\
            {FFFFFF}/reiniciar{B4B5B7}             reinicia el modo de juego\n", sizeof cuerpo);
    }

    ShowPlayerDialog(playerid, DIALOGO_ADMIN, DIALOG_STYLE_MSGBOX, titulo, cuerpo, "Cerrar", "");
    return 1;
}

CMD:admins(playerid, params[])
{
    if (NoLogueado(playerid)) return 1;

    new nombre[MAX_PLAYER_NAME + 1], rango[32], conectados = 0;

    Linea(playerid, COLOR_MARCA, "Equipo administrativo conectado:");

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jAdmin] < ADMIN_SOPORTE) continue;
        if (Jugador[i][jAdminOculto] && Jugador[playerid][jAdmin] < Jugador[i][jAdmin]) continue;

        NombreRP(i, nombre, sizeof nombre);
        Admin_NombreNivel(Jugador[i][jAdmin], rango, sizeof rango);

        format(g_Buffer, sizeof g_Buffer, "  {E8973A}%s{B4B5B7} - %s (ID %d)", rango, nombre, i);
        SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
        conectados++;
    }

    if (conectados == 0)
    {
        Linea(playerid, COLOR_GRIS, "  No hay ningun administrador conectado ahora mismo.");
        Linea(playerid, COLOR_GRIS, "  Puedes dejar un reporte igualmente con /reportar.");
    }
    return 1;
}

CMD:a(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/a [mensaje para el equipo administrativo]");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], rango[32], mensaje[224];
    GetPlayerName(playerid, nombre, sizeof nombre);
    SubrayadoAEspacio(nombre);
    Admin_NombreNivel(Jugador[playerid][jAdmin], rango, sizeof rango);
    LimpiarTexto(params);

    format(mensaje, sizeof mensaje, "[STAFF] %s %s: %s", rango, nombre, params);

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jAdmin] < ADMIN_SOPORTE) continue;
        SendClientMessage(i, COLOR_ADMIN_CHAT, mensaje);
    }
    return 1;
}

CMD:pos(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    format(g_Buffer, sizeof g_Buffer, "%.4f, %.4f, %.4f   angulo %.4f", x, y, z, a);
    Info(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "Interior %d   Mundo virtual %d", GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));
    Info(playerid, g_Buffer);

    printf("[POS] %.4f, %.4f, %.4f  angulo %.4f  int %d  mundo %d",
        x, y, z, a, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));
    return 1;
}

// ---------------------------------------------------------------------
//  MODO SERVICIO
//  Reutiliza jAdminOculto: en /admins te ocultas de quien tenga menos
//  nivel que tu (regla que ya existia), y mientras estas en servicio la
//  vida y el chaleco se mantienen al maximo (Admin_Segundo, mas abajo).
//  No es invisibilidad de verdad: SA-MP no permite ocultar el modelo del
//  jugador a otros. Es un blindaje de salud, no un "modo fantasma".
// ---------------------------------------------------------------------
CMD:staff(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

    Jugador[playerid][jAdminOculto] = !Jugador[playerid][jAdminOculto];

    if (Jugador[playerid][jAdminOculto])
    {
        SetPlayerHealth(playerid, 100.0);
        SetPlayerArmour(playerid, 100.0);
        Exito(playerid, "Modo servicio activado. Vida y chaleco al maximo mientras dure.");
        Info(playerid, "En /admins solo te ven los de tu nivel o superior.");
    }
    else
    {
        Exito(playerid, "Modo servicio desactivado.");
    }
    return 1;
}

// ---------------------------------------------------------------------
//  REPORTES
// ---------------------------------------------------------------------
CMD:reportar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/reportar [lo que quieres contarle al equipo]");
        return 1;
    }

    // Evita duplicados del mismo jugador.
    for (new i = 0; i < MAX_REPORTES; i++)
    {
        if (g_Reportes[i][reUsado] && g_Reportes[i][reJugador] == playerid)
        {
            Error(playerid, "Ya tienes un reporte abierto. Espera a que lo atiendan.");
            return 1;
        }
    }

    new hueco = -1;
    for (new i = 0; i < MAX_REPORTES; i++)
    {
        if (!g_Reportes[i][reUsado]) { hueco = i; break; }
    }

    if (hueco == -1)
    {
        Error(playerid, "La cola de reportes esta llena. Intentalo en un minuto.");
        return 1;
    }

    LimpiarTexto(params);

    g_Reportes[hueco][reUsado]    = true;
    g_Reportes[hueco][reJugador]  = playerid;
    g_Reportes[hueco][reSegundos] = 0;
    g_Reportes[hueco][reAtiende]  = INVALID_PLAYER_ID;
    GetPlayerName(playerid, g_Reportes[hueco][reNombre], MAX_PLAYER_NAME + 1);
    format(g_Reportes[hueco][reTexto], 144, "%s", params);

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);

    format(g_Buffer, sizeof g_Buffer, "[REPORTE #%d] %s (ID %d): %s", hueco + 1, nombre, playerid, params);
    Admin_Aviso(g_Buffer);

    Exito(playerid, "Reporte enviado. Un administrador te atendera en cuanto pueda.");
    Info(playerid, "Sigue roleando mientras tanto, no te quedes parado.");
    return 1;
}
CMD:reporte(playerid, params[]) return cmd_reportar(playerid, params);

CMD:reportes(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

    new abiertos = 0, tiempo[32], nombre[MAX_PLAYER_NAME + 1];

    Linea(playerid, COLOR_MARCA, "Reportes abiertos:");

    for (new i = 0; i < MAX_REPORTES; i++)
    {
        if (!g_Reportes[i][reUsado]) continue;

        FormatearTiempo(g_Reportes[i][reSegundos], tiempo, sizeof tiempo);
        format(nombre, sizeof nombre, "%s", g_Reportes[i][reNombre]);
        SubrayadoAEspacio(nombre);

        format(g_Buffer, sizeof g_Buffer, "  {E8973A}#%d{B4B5B7} %s (ID %d) - hace %s%s",
            i + 1, nombre, g_Reportes[i][reJugador], tiempo,
            (g_Reportes[i][reAtiende] != INVALID_PLAYER_ID) ? (" {53C168}[atendiendo]") : (""));
        SendClientMessage(playerid, COLOR_GRIS, g_Buffer);

        format(g_Buffer, sizeof g_Buffer, "      {FFFFFF}%s", g_Reportes[i][reTexto]);
        SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
        abiertos++;
    }

    if (abiertos == 0) Linea(playerid, COLOR_GRIS, "  No hay ningun reporte pendiente.");
    else Linea(playerid, COLOR_GRIS, "  Usa /atender [numero] para ir con el jugador.");
    return 1;
}

CMD:atender(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

    new numero;
    if (sscanf(params, "d", numero))
    {
        Uso(playerid, "/atender [numero de reporte]");
        return 1;
    }

    new indice = numero - 1;
    if (indice < 0 || indice >= MAX_REPORTES || !g_Reportes[indice][reUsado])
    {
        Error(playerid, "Ese reporte no existe.");
        return 1;
    }

    new objetivo = g_Reportes[indice][reJugador];
    if (!IsPlayerConnected(objetivo) || !Jugador[objetivo][jLogueado])
    {
        Error(playerid, "El jugador ya no esta conectado. Reporte cerrado.");
        g_Reportes[indice][reUsado] = false;
        return 1;
    }

    g_Reportes[indice][reAtiende] = playerid;

    new Float:x, Float:y, Float:z;
    GetPlayerPos(objetivo, x, y, z);
    SetPlayerPos(playerid, x + 1.5, y + 1.5, z);
    SetPlayerInterior(playerid, GetPlayerInterior(objetivo));
    SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(objetivo));

    new admin[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], rango[32];
    NombreRP(playerid, admin, sizeof admin);
    NombreRP(objetivo, suyo, sizeof suyo);
    Admin_NombreNivel(Jugador[playerid][jAdmin], rango, sizeof rango);

    format(g_Buffer, sizeof g_Buffer, "%s %s esta atendiendo tu reporte.", rango, admin);
    SendClientMessage(objetivo, COLOR_ADMIN_ACCION, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "%s esta atendiendo el reporte #%d de %s.", admin, numero, suyo);
    Admin_Aviso(g_Buffer);

    g_Reportes[indice][reUsado] = false;
    return 1;
}

// g_Reportes es "static" (privado de este archivo). Estas dos funciones
// son la unica puerta de entrada para que 23_panel.inc pueda listar los
// reportes abiertos sin tener que duplicar el array.
stock Reportes_ConstruirLista(destino[], tam)
{
    new linea[192], nombre[MAX_PLAYER_NAME + 1], tiempo[32], abiertos = 0;

    format(destino, tam, "{E8B33A}Jugador\tHace\tMensaje\n");

    for (new i = 0; i < MAX_REPORTES; i++)
    {
        if (!g_Reportes[i][reUsado]) continue;

        format(nombre, sizeof nombre, "%s", g_Reportes[i][reNombre]);
        SubrayadoAEspacio(nombre);
        FormatearTiempo(g_Reportes[i][reSegundos], tiempo, sizeof tiempo);

        format(linea, sizeof linea, "{FFFFFF}%s (ID %d)\t{B4B5B7}%s\t%s\n",
            nombre, g_Reportes[i][reJugador], tiempo, g_Reportes[i][reTexto]);

        if (strlen(destino) + strlen(linea) >= tam - 8) break;
        strcat(destino, linea, tam);
        abiertos++;
    }
    return abiertos;
}

// Traduce la fila que devuelve el dialogo (orden de aparicion en la
// lista) al numero de reporte que espera /atender. Devuelve 0 si la
// fila ya no corresponde a ningun reporte abierto.
stock Reportes_NumeroPorFila(fila)
{
    new indice = 0;
    for (new i = 0; i < MAX_REPORTES; i++)
    {
        if (!g_Reportes[i][reUsado]) continue;
        if (indice == fila) return i + 1;
        indice++;
    }
    return 0;
}

// ---------------------------------------------------------------------
//  MOVIMIENTO Y CONSULTA
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
//  COORDENADAS GUARDADAS  (/goto, /marcarcoord, /borrarcoord)
//  Se guardan en la tabla `coordenadas` y se cargan en memoria al
//  arrancar. La seccion "Coordenadas" del panel (23_panel.inc) lee y
//  escribe este mismo array, que por eso NO es "static".
// ---------------------------------------------------------------------
#define MAX_COORDS 60

enum E_GOTO
{
    goID,
    goClave[24],
    goNombre[48],
    Float:goX, Float:goY, Float:goZ, Float:goA,
    goInterior,
    goMundo
};
new g_Coords[MAX_COORDS][E_GOTO];
new g_TotalCoords = 0;

stock Coord_CargarTodas()
{
    mysql_pquery(g_BD, "SELECT * FROM coordenadas ORDER BY nombre ASC", "AlCargarCoordenadas");
    return 1;
}

forward AlCargarCoordenadas();
public AlCargarCoordenadas()
{
    new filas;
    cache_get_row_count(filas);

    g_TotalCoords = 0;

    for (new i = 0; i < filas && i < MAX_COORDS; i++)
    {
        new indice = g_TotalCoords;
        cache_get_value_name_int(i, "id", g_Coords[indice][goID]);
        cache_get_value_name(i, "clave", g_Coords[indice][goClave], 24);
        cache_get_value_name(i, "nombre", g_Coords[indice][goNombre], 48);
        cache_get_value_name_float(i, "x", g_Coords[indice][goX]);
        cache_get_value_name_float(i, "y", g_Coords[indice][goY]);
        cache_get_value_name_float(i, "z", g_Coords[indice][goZ]);
        cache_get_value_name_float(i, "a", g_Coords[indice][goA]);
        cache_get_value_name_int(i, "interior", g_Coords[indice][goInterior]);
        cache_get_value_name_int(i, "mundo", g_Coords[indice][goMundo]);
        g_TotalCoords++;
    }

    printf("  [COORDENADAS] %d puntos cargados.", g_TotalCoords);
    return 1;
}

stock Coord_BuscarPorClave(const clave[])
{
    for (new i = 0; i < g_TotalCoords; i++)
    {
        if (strcmp(g_Coords[i][goClave], clave, true) == 0) return i;
    }
    return -1;
}

stock Coord_Teletransportar(playerid, indice)
{
    if (indice < 0 || indice >= g_TotalCoords) return 0;

    new Float:x = g_Coords[indice][goX], Float:y = g_Coords[indice][goY], Float:z = g_Coords[indice][goZ];

    if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
    {
        new vehiculo = GetPlayerVehicleID(playerid);
        SetVehiclePos(vehiculo, x, y, z);
        SetVehicleZAngle(vehiculo, g_Coords[indice][goA]);
        LinkVehicleToInterior(vehiculo, g_Coords[indice][goInterior]);
        SetVehicleVirtualWorld(vehiculo, g_Coords[indice][goMundo]);
    }
    else
    {
        SetPlayerPos(playerid, x, y, z);
        SetPlayerFacingAngle(playerid, g_Coords[indice][goA]);
    }

    SetPlayerInterior(playerid, g_Coords[indice][goInterior]);
    SetPlayerVirtualWorld(playerid, g_Coords[indice][goMundo]);
    SetCameraBehindPlayer(playerid);

    format(g_Buffer, sizeof g_Buffer, "Te has teletransportado a %s.", g_Coords[indice][goNombre]);
    Info(playerid, g_Buffer);
    return 1;
}

CMD:goto(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

    new clave[24];
    if (sscanf(params, "s[24]", clave))
    {
        new lista[512];
        format(lista, sizeof lista, "Lugares disponibles:");
        for (new i = 0; i < g_TotalCoords; i++)
        {
            format(g_Buffer, sizeof g_Buffer, " %s (%s)", g_Coords[i][goClave], g_Coords[i][goNombre]);
            strcat(lista, g_Buffer, sizeof lista);
        }
        Info(playerid, lista);
        Uso(playerid, "/goto [lugar]");
        return 1;
    }

    new indice = Coord_BuscarPorClave(clave);
    if (indice == -1)
    {
        Error(playerid, "Ese lugar no existe. Escribe /goto para ver la lista.");
        return 1;
    }

    Coord_Teletransportar(playerid, indice);
    return 1;
}
CMD:ls(playerid, params[])
{
    #pragma unused params
    new clave[4] = "ls";
    return cmd_goto(playerid, clave);
}
CMD:grove(playerid, params[])
{
    #pragma unused params
    new clave[6] = "grove";
    return cmd_goto(playerid, clave);
}

CMD:marcarcoord(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new clave[24], nombre[48];
    if (sscanf(params, "s[24]s[48]", clave, nombre))
    {
        Uso(playerid, "/marcarcoord [clave sin espacios] [nombre para mostrar]");
        return 1;
    }

    if (g_TotalCoords >= MAX_COORDS)
    {
        Error(playerid, "Se ha alcanzado el limite de coordenadas guardadas.");
        return 1;
    }

    if (Coord_BuscarPorClave(clave) != -1)
    {
        Error(playerid, "Ya existe una coordenada con esa clave.");
        return 1;
    }

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);
    new interior = GetPlayerInterior(playerid), mundo = GetPlayerVirtualWorld(playerid);

    new admin[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);
    LimpiarTexto(nombre);

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "INSERT INTO coordenadas (clave, nombre, x, y, z, a, interior, mundo, creado_por) \
         VALUES ('%e', '%e', %f, %f, %f, %f, %d, %d, '%e')",
        clave, nombre, x, y, z, a, interior, mundo, admin);
    mysql_pquery(g_BD, g_Buffer, "AlMarcarCoordenada", "d", playerid);

    new indice = g_TotalCoords;
    g_Coords[indice][goID] = 0;
    format(g_Coords[indice][goClave], 24, "%s", clave);
    format(g_Coords[indice][goNombre], 48, "%s", nombre);
    g_Coords[indice][goX] = x;
    g_Coords[indice][goY] = y;
    g_Coords[indice][goZ] = z;
    g_Coords[indice][goA] = a;
    g_Coords[indice][goInterior] = interior;
    g_Coords[indice][goMundo] = mundo;
    g_TotalCoords++;

    Exito(playerid, "Coordenada guardada.");
    return 1;
}

forward AlMarcarCoordenada(playerid);
public AlMarcarCoordenada(playerid)
{
    if (g_TotalCoords == 0) return 0;
    g_Coords[g_TotalCoords - 1][goID] = cache_insert_id();
    #pragma unused playerid
    return 1;
}

CMD:borrarcoord(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new clave[24];
    if (sscanf(params, "s[24]", clave))
    {
        Uso(playerid, "/borrarcoord [clave]");
        return 1;
    }

    new indice = Coord_BuscarPorClave(clave);
    if (indice == -1)
    {
        Error(playerid, "No existe ninguna coordenada con esa clave.");
        return 1;
    }

    if (g_Coords[indice][goID] > 0)
    {
        format(g_Buffer, sizeof g_Buffer, "DELETE FROM coordenadas WHERE id = %d", g_Coords[indice][goID]);
        mysql_pquery(g_BD, g_Buffer);
    }

    // Compacta el hueco moviendo la ultima al sitio borrado.
    g_Coords[indice] = g_Coords[g_TotalCoords - 1];
    g_TotalCoords--;

    Exito(playerid, "Coordenada borrada.");
    return 1;
}

CMD:ir(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/ir [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    new Float:x, Float:y, Float:z;
    GetPlayerPos(objetivo, x, y, z);

    if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
    {
        new vehiculo = GetPlayerVehicleID(playerid);
        SetVehiclePos(vehiculo, x + 2.0, y + 2.0, z);
        LinkVehicleToInterior(vehiculo, GetPlayerInterior(objetivo));
        SetVehicleVirtualWorld(vehiculo, GetPlayerVirtualWorld(objetivo));
    }
    else SetPlayerPos(playerid, x + 1.5, y + 1.5, z);

    SetPlayerInterior(playerid, GetPlayerInterior(objetivo));
    SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(objetivo));

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(objetivo, nombre, sizeof nombre);
    format(g_Buffer, sizeof g_Buffer, "Te has teletransportado hasta %s.", nombre);
    Info(playerid, g_Buffer);
    return 1;
}

CMD:traer(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/traer [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;
    if (Admin_NoPuedeCon(playerid, objetivo)) return 1;

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    SetPlayerPos(objetivo, x + 1.5, y + 1.5, z);
    SetPlayerInterior(objetivo, GetPlayerInterior(playerid));
    SetPlayerVirtualWorld(objetivo, GetPlayerVirtualWorld(playerid));

    new admin[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);
    format(g_Buffer, sizeof g_Buffer, "%s te ha traido a su posicion.", admin);
    SendClientMessage(objetivo, COLOR_ADMIN_ACCION, g_Buffer);

    Exito(playerid, "Jugador traido.");
    return 1;
}

CMD:revisar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/revisar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    new nombre[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1], ip[46];
    NombreRP(objetivo, nombre, sizeof nombre);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);
    GetPlayerIp(objetivo, ip, sizeof ip);

    new mano[24], banco[24], tiempo[48], rangoAdmin[32];
    FormatearDinero(Jugador[objetivo][jDinero], mano, sizeof mano);
    FormatearDinero(Jugador[objetivo][jBanco], banco, sizeof banco);
    FormatearMinutos(Jugador[objetivo][jMinutosTotales], tiempo, sizeof tiempo);
    Admin_NombreNivel(Jugador[objetivo][jAdmin], rangoAdmin, sizeof rangoAdmin);

    new trabajo1[32], trabajo2[32], faccion[48];
    Trabajo_Nombre(Jugador[objetivo][jTrabajo][0], trabajo1, sizeof trabajo1);
    Trabajo_Nombre(Jugador[objetivo][jTrabajo][1], trabajo2, sizeof trabajo2);
    Faccion_Nombre(Jugador[objetivo][jFaccion], faccion, sizeof faccion);

    new Float:vida, Float:armadura;
    GetPlayerHealth(objetivo, vida);
    GetPlayerArmour(objetivo, armadura);

    new cuerpo[1400];
    format(cuerpo, sizeof cuerpo,
        "{E8B33A}%s{B4B5B7}   cuenta {FFFFFF}%s{B4B5B7}   id de BD {FFFFFF}%d{B4B5B7}   ID {FFFFFF}%d\n\n\
        {B4B5B7}Nivel        {FFFFFF}%d{B4B5B7}      Respeto {FFFFFF}%d{B4B5B7}      Tiempo {FFFFFF}%s\n\
        {B4B5B7}Efectivo     {3D9970}%s{B4B5B7}      Banco {3D9970}%s\n\
        {B4B5B7}Vida         {FFFFFF}%.0f{B4B5B7}      Chaleco {FFFFFF}%.0f\n\
        {B4B5B7}Trabajos     {FFFFFF}%s{B4B5B7} / {FFFFFF}%s\n\
        {B4B5B7}Faccion      {FFFFFF}%s{B4B5B7} (rango %d)\n\
        {B4B5B7}Vehiculos    {FFFFFF}%d\n\
        {B4B5B7}Peso         {FFFFFF}%.1f{B4B5B7} de %.1f kg\n\n\
        {B4B5B7}Rango staff  {E8973A}%s\n\
        {B4B5B7}Avisos       {FFFFFF}%d{B4B5B7} de 3\n\
        {B4B5B7}Silenciado   {FFFFFF}%d s{B4B5B7}      Carcel {FFFFFF}%d s\n\
        {B4B5B7}IP           {FFFFFF}%s\n\
        {B4B5B7}Posicion     {FFFFFF}%.1f, %.1f, %.1f{B4B5B7}  int %d  mundo %d",
        nombre, cuenta, Jugador[objetivo][jID], objetivo,
        Jugador[objetivo][jNivel], Jugador[objetivo][jRespeto], tiempo,
        mano, banco, vida, armadura,
        trabajo1, trabajo2,
        faccion, Jugador[objetivo][jFaccionRango],
        Veh_ContarDeJugador(objetivo),
        Inv_PesoActual(objetivo), Inv_PesoMaximo(objetivo),
        rangoAdmin, Jugador[objetivo][jAvisos],
        Jugador[objetivo][jMuteSegundos], Jugador[objetivo][jCarcelSegundos],
        ip,
        Jugador[objetivo][jPosX], Jugador[objetivo][jPosY], Jugador[objetivo][jPosZ],
        GetPlayerInterior(objetivo), GetPlayerVirtualWorld(objetivo));

    ShowPlayerDialog(playerid, DIALOGO_ADMIN_FICHA, DIALOG_STYLE_MSGBOX,
        "{E8973A}Ficha administrativa", cuerpo, "Cerrar", "");
    return 1;
}

// ---------------------------------------------------------------------
//  SANCIONES
// ---------------------------------------------------------------------
CMD:expulsar_jugador(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;

    new objetivoTexto[24], motivo[128];
    if (sscanf(params, "s[24]s[128]", objetivoTexto, motivo))
    {
        Uso(playerid, "/expulsar_jugador [id o nombre] [motivo]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;
    if (Admin_NoPuedeCon(playerid, objetivo)) return 1;

    new admin[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);
    NombreRP(objetivo, suyo, sizeof suyo);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);
    LimpiarTexto(motivo);

    format(g_Buffer, sizeof g_Buffer, "%s ha expulsado a %s. Motivo: %s", admin, suyo, motivo);
    MensajeGlobal(COLOR_ADMIN_ACCION, g_Buffer);

    BD_RegistrarSancion(Jugador[objetivo][jID], cuenta, SANCION_KICK, admin, motivo, 0);
    Admin_Registrar(playerid, "KICK", cuenta, motivo);

    BD_GuardarCuenta(objetivo);
    SetTimerEx("ExpulsarRetardado", 500, false, "d", objetivo);
    return 1;
}
CMD:kick(playerid, params[]) return cmd_expulsar_jugador(playerid, params);

CMD:avisar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;

    new objetivoTexto[24], motivo[128];
    if (sscanf(params, "s[24]s[128]", objetivoTexto, motivo))
    {
        Uso(playerid, "/avisar [id o nombre] [motivo]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;
    if (Admin_NoPuedeCon(playerid, objetivo)) return 1;

    Jugador[objetivo][jAvisos]++;

    new admin[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);
    NombreRP(objetivo, suyo, sizeof suyo);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);
    LimpiarTexto(motivo);

    format(g_Buffer, sizeof g_Buffer, "%s ha recibido un aviso de %s (%d de 3). Motivo: %s",
        suyo, admin, Jugador[objetivo][jAvisos], motivo);
    MensajeGlobal(COLOR_ADMIN_ACCION, g_Buffer);

    BD_RegistrarSancion(Jugador[objetivo][jID], cuenta, SANCION_AVISO, admin, motivo, 0);
    Admin_Registrar(playerid, "AVISO", cuenta, motivo);

    if (Jugador[objetivo][jAvisos] >= 3)
    {
        new consulta[400];
        mysql_format(g_BD, consulta, sizeof consulta,
            "UPDATE cuentas SET baneado = 1, ban_motivo = '%e', ban_admin = '%e', \
                                ban_expira = DATE_ADD(NOW(), INTERVAL 7 DAY), avisos = 0 WHERE id = %d",
            "Acumulacion de 3 avisos", admin, Jugador[objetivo][jID]);
        mysql_pquery(g_BD, consulta);

        format(g_Buffer, sizeof g_Buffer, "%s acumula 3 avisos y queda baneado 7 dias.", suyo);
        MensajeGlobal(COLOR_ROJO_FUERTE, g_Buffer);

        BD_RegistrarSancion(Jugador[objetivo][jID], cuenta, SANCION_BAN, admin, "Acumulacion de 3 avisos", 604800);
        SetTimerEx("ExpulsarRetardado", 800, false, "d", objetivo);
        return 1;
    }

    BD_GuardarCuenta(objetivo);
    return 1;
}

CMD:mutear(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;

    new objetivoTexto[24], minutos, motivo[128];
    if (sscanf(params, "s[24]ds[128]", objetivoTexto, minutos, motivo))
    {
        Uso(playerid, "/mutear [id o nombre] [minutos] [motivo]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;
    if (Admin_NoPuedeCon(playerid, objetivo)) return 1;

    if (minutos < 1 || minutos > 1440)
    {
        Error(playerid, "El silencio va de 1 a 1440 minutos.");
        return 1;
    }

    Jugador[objetivo][jMuteSegundos] = minutos * 60;

    new admin[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);
    NombreRP(objetivo, suyo, sizeof suyo);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);
    LimpiarTexto(motivo);

    format(g_Buffer, sizeof g_Buffer, "%s ha silenciado a %s durante %d minutos. Motivo: %s", admin, suyo, minutos, motivo);
    MensajeGlobal(COLOR_ADMIN_ACCION, g_Buffer);

    BD_RegistrarSancion(Jugador[objetivo][jID], cuenta, SANCION_MUTE, admin, motivo, minutos * 60);
    Admin_Registrar(playerid, "MUTE", cuenta, motivo);
    BD_GuardarCuenta(objetivo);
    return 1;
}

CMD:desmutear(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/desmutear [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    Jugador[objetivo][jMuteSegundos] = 0;
    BD_GuardarCuenta(objetivo);

    Exito(objetivo, "Ya puedes volver a hablar.");
    Exito(playerid, "Jugador desmuteado.");
    return 1;
}

CMD:congelar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/congelar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;
    if (Admin_NoPuedeCon(playerid, objetivo)) return 1;

    Jugador[objetivo][jCongelado] = true;
    TogglePlayerControllable(objetivo, false);

    new admin[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);
    format(g_Buffer, sizeof g_Buffer, "%s te ha congelado. No te desconectes.", admin);
    SendClientMessage(objetivo, COLOR_ADMIN_ACCION, g_Buffer);

    Exito(playerid, "Jugador congelado.");
    return 1;
}

CMD:descongelar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/descongelar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    Jugador[objetivo][jCongelado] = false;
    TogglePlayerControllable(objetivo, true);

    Exito(objetivo, "Ya puedes moverte.");
    Exito(playerid, "Jugador descongelado.");
    return 1;
}

CMD:banear(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new objetivoTexto[24], dias, motivo[128];
    if (sscanf(params, "s[24]ds[128]", objetivoTexto, dias, motivo))
    {
        Uso(playerid, "/banear [id o nombre] [dias, 0 = permanente] [motivo]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;
    if (Admin_NoPuedeCon(playerid, objetivo)) return 1;

    if (dias < 0 || dias > 3650)
    {
        Error(playerid, "Los dias tienen que estar entre 0 y 3650.");
        return 1;
    }

    new admin[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);
    NombreRP(objetivo, suyo, sizeof suyo);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);
    LimpiarTexto(motivo);

    new consulta[512];
    if (dias == 0)
    {
        mysql_format(g_BD, consulta, sizeof consulta,
            "UPDATE cuentas SET baneado = 1, ban_motivo = '%e', ban_admin = '%e', ban_expira = NULL WHERE id = %d",
            motivo, admin, Jugador[objetivo][jID]);
    }
    else
    {
        mysql_format(g_BD, consulta, sizeof consulta,
            "UPDATE cuentas SET baneado = 1, ban_motivo = '%e', ban_admin = '%e', \
                                ban_expira = DATE_ADD(NOW(), INTERVAL %d DAY) WHERE id = %d",
            motivo, admin, dias, Jugador[objetivo][jID]);
    }
    mysql_pquery(g_BD, consulta);

    if (dias == 0) format(g_Buffer, sizeof g_Buffer, "%s ha baneado permanentemente a %s. Motivo: %s", admin, suyo, motivo);
    else           format(g_Buffer, sizeof g_Buffer, "%s ha baneado a %s durante %d dias. Motivo: %s", admin, suyo, dias, motivo);

    MensajeGlobal(COLOR_ROJO_FUERTE, g_Buffer);

    BD_RegistrarSancion(Jugador[objetivo][jID], cuenta, SANCION_BAN, admin, motivo, dias * 86400);
    Admin_Registrar(playerid, "BAN", cuenta, motivo);

    BD_GuardarCuenta(objetivo);
    SetTimerEx("ExpulsarRetardado", 800, false, "d", objetivo);
    return 1;
}

CMD:desbanear(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new cuenta[MAX_PLAYER_NAME + 1];
    if (sscanf(params, "s[25]", cuenta))
    {
        Uso(playerid, "/desbanear [nombre completo de la cuenta]");
        return 1;
    }

    new consulta[300];
    mysql_format(g_BD, consulta, sizeof consulta,
        "UPDATE cuentas SET baneado = 0, ban_motivo = '', ban_expira = NULL, avisos = 0 WHERE nombre = '%e'", cuenta);
    mysql_pquery(g_BD, consulta);

    format(g_Buffer, sizeof g_Buffer, "Cuenta %s desbaneada (si existia).", cuenta);
    Exito(playerid, g_Buffer);

    Admin_Registrar(playerid, "DESBAN", cuenta, "");
    return 1;
}

// ---------------------------------------------------------------------
//  GESTION DE CUENTAS DESCONECTADAS (23_panel.inc, seccion "Cuentas")
//  Version por id/nombre de estos mismos ajustes, para cuando el jugador
//  no esta en el servidor. Si esta conectado en el momento de banear, se
//  le expulsa igual que hace /banear.
// ---------------------------------------------------------------------
stock Cuenta_DarDineroOffline(playerid, cuentaid, const nombreCuenta[], cantidad)
{
    new consulta[200];
    mysql_format(g_BD, consulta, sizeof consulta,
        "UPDATE cuentas SET dinero = dinero + %d WHERE id = %d", cantidad, cuentaid);
    mysql_pquery(g_BD, consulta);

    new admin[MAX_PLAYER_NAME + 1], dineroTxt[24];
    NombreRP(playerid, admin, sizeof admin);
    FormatearDinero((cantidad < 0) ? (-cantidad) : (cantidad), dineroTxt, sizeof dineroTxt);

    format(g_Buffer, sizeof g_Buffer, "Le has %s %s a la cuenta %s (esta desconectada).",
        (cantidad < 0) ? ("quitado") : ("dado"), dineroTxt, nombreCuenta);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "%d (offline)", cantidad);
    Admin_Registrar(playerid, "DINERO", nombreCuenta, g_Buffer);
}

stock Cuenta_CambiarNivelOffline(playerid, cuentaid, const nombreCuenta[], nivel)
{
    new consulta[160];
    mysql_format(g_BD, consulta, sizeof consulta,
        "UPDATE cuentas SET nivel = %d WHERE id = %d", nivel, cuentaid);
    mysql_pquery(g_BD, consulta);

    format(g_Buffer, sizeof g_Buffer, "Nivel de la cuenta %s puesto en %d (esta desconectada).", nombreCuenta, nivel);
    Exito(playerid, g_Buffer);
    Admin_Registrar(playerid, "NIVEL", nombreCuenta, "offline");
}

stock Cuenta_BanearOffline(playerid, cuentaid, const nombreCuenta[], dias, const motivo[])
{
    new admin[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);

    new consulta[512];
    if (dias == 0)
    {
        mysql_format(g_BD, consulta, sizeof consulta,
            "UPDATE cuentas SET baneado = 1, ban_motivo = '%e', ban_admin = '%e', ban_expira = NULL WHERE id = %d",
            motivo, admin, cuentaid);
    }
    else
    {
        mysql_format(g_BD, consulta, sizeof consulta,
            "UPDATE cuentas SET baneado = 1, ban_motivo = '%e', ban_admin = '%e', \
                                ban_expira = DATE_ADD(NOW(), INTERVAL %d DAY) WHERE id = %d",
            motivo, admin, dias, cuentaid);
    }
    mysql_pquery(g_BD, consulta);

    if (dias == 0) format(g_Buffer, sizeof g_Buffer, "%s ha baneado permanentemente la cuenta %s. Motivo: %s", admin, nombreCuenta, motivo);
    else           format(g_Buffer, sizeof g_Buffer, "%s ha baneado la cuenta %s durante %d dias. Motivo: %s", admin, nombreCuenta, dias, motivo);
    MensajeGlobal(COLOR_ROJO_FUERTE, g_Buffer);

    BD_RegistrarSancion(cuentaid, nombreCuenta, SANCION_BAN, admin, motivo, dias * 86400);
    Admin_Registrar(playerid, "BAN", nombreCuenta, motivo);

    // Si la cuenta esta conectada ahora mismo, la expulsamos igual que /banear.
    new objetivo = BuscarJugador(nombreCuenta);
    if (objetivo != INVALID_PLAYER_ID && objetivo != JUGADOR_AMBIGUO)
    {
        SetTimerEx("ExpulsarRetardado", 800, false, "d", objetivo);
    }
}

CMD:carcel(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new objetivoTexto[24], minutos, motivo[128];
    if (sscanf(params, "s[24]ds[128]", objetivoTexto, minutos, motivo))
    {
        Uso(playerid, "/carcel [id o nombre] [minutos] [motivo]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;
    if (Admin_NoPuedeCon(playerid, objetivo)) return 1;

    if (minutos < 1 || minutos > 720)
    {
        Error(playerid, "La carcel va de 1 a 720 minutos.");
        return 1;
    }

    // Si estaba caido, se resuelve primero (descongela, quita la
    // invulnerabilidad y limpia el aviso de Medicos) para que la carcel
    // no se quede peleando contra el estado de herido.
    if (Jugador[objetivo][jHerido]) Medicos_ResolverAdmin(objetivo);

    Jugador[objetivo][jCarcelSegundos] = minutos * 60;

    SetPlayerPos(objetivo, CARCEL_X, CARCEL_Y, CARCEL_Z);
    SetPlayerInterior(objetivo, CARCEL_INTERIOR);
    SetPlayerVirtualWorld(objetivo, 1000 + objetivo);

    new admin[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);
    NombreRP(objetivo, suyo, sizeof suyo);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);
    LimpiarTexto(motivo);

    format(g_Buffer, sizeof g_Buffer, "%s ha encarcelado a %s durante %d minutos. Motivo: %s", admin, suyo, minutos, motivo);
    MensajeGlobal(COLOR_ADMIN_ACCION, g_Buffer);

    BD_RegistrarSancion(Jugador[objetivo][jID], cuenta, SANCION_CARCEL, admin, motivo, minutos * 60);
    Admin_Registrar(playerid, "CARCEL", cuenta, motivo);
    BD_GuardarCuenta(objetivo);
    return 1;
}

CMD:soltar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/soltar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    Admin_SacarDeCarcel(objetivo);
    Exito(playerid, "Jugador liberado.");
    return 1;
}

stock Admin_SacarDeCarcel(playerid)
{
    Jugador[playerid][jCarcelSegundos] = 0;

    SetPlayerPos(playerid, HOSPITAL_X, HOSPITAL_Y, HOSPITAL_Z);
    SetPlayerFacingAngle(playerid, HOSPITAL_A);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
    SetCameraBehindPlayer(playerid);

    Exito(playerid, "Has salido de la carcel administrativa.");
    BD_GuardarCuenta(playerid);
    return 1;
}

CMD:desarmar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/desarmar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;
    if (Admin_NoPuedeCon(playerid, objetivo)) return 1;

    ResetPlayerWeapons(objetivo);
    Exito(playerid, "Jugador desarmado.");
    Aviso(objetivo, "Un administrador te ha retirado las armas de las manos.");
    return 1;
}

CMD:curar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/curar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (Jugador[objetivo][jHerido])
    {
        // Estaba caido/congelado: lo levanta donde esta, sin trasladarlo
        // ni cobrarle nada (ver Medicos_ResolverAdmin, 36_medicos.inc).
        Medicos_ResolverAdmin(objetivo);
    }
    else
    {
        SetPlayerHealth(objetivo, 100.0);
    }

    Exito(playerid, "Jugador curado.");
    Exito(objetivo, "Un administrador te ha curado.");
    return 1;
}

CMD:reparar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante.");
        return 1;
    }

    new vehiculo = GetPlayerVehicleID(playerid);
    RepairVehicle(vehiculo);

    new indice = Veh_BuscarPorObjeto(vehiculo);
    if (indice != -1)
    {
        Veh_Guardar(indice);
    }

    Exito(playerid, "Vehiculo reparado.");
    return 1;
}

CMD:limpiarsuelo(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new borrados = 0;
    for (new i = 0; i < MAX_OBJETOS_SUELO; i++)
    {
        if (Suelo_Borrar(i)) borrados++;
    }

    format(g_Buffer, sizeof g_Buffer, "Se han retirado %d objetos del suelo.", borrados);
    Exito(playerid, g_Buffer);
    Admin_Registrar(playerid, "LIMPIAR", "", "Objetos del suelo");
    return 1;
}

// ---------------------------------------------------------------------
//  ESPECTAR
// ---------------------------------------------------------------------
CMD:espectar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/espectar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (objetivo == playerid)
    {
        Error(playerid, "No puedes espectarte a ti mismo.");
        return 1;
    }

    if (Jugador[playerid][jSpecObjetivo] == INVALID_PLAYER_ID)
    {
        GuardarPosicionActual(playerid);
        Jugador[playerid][jSpecPos][0] = Jugador[playerid][jPosX];
        Jugador[playerid][jSpecPos][1] = Jugador[playerid][jPosY];
        Jugador[playerid][jSpecPos][2] = Jugador[playerid][jPosZ];
        Jugador[playerid][jSpecPos][3] = Jugador[playerid][jPosA];
        Jugador[playerid][jSpecInterior] = GetPlayerInterior(playerid);
        Jugador[playerid][jSpecMundo]    = GetPlayerVirtualWorld(playerid);
    }

    Jugador[playerid][jSpecObjetivo] = objetivo;

    TogglePlayerSpectating(playerid, true);
    SetPlayerInterior(playerid, GetPlayerInterior(objetivo));
    SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(objetivo));

    if (GetPlayerState(objetivo) == PLAYER_STATE_DRIVER || GetPlayerState(objetivo) == PLAYER_STATE_PASSENGER)
    {
        PlayerSpectateVehicle(playerid, GetPlayerVehicleID(objetivo));
    }
    else PlayerSpectatePlayer(playerid, objetivo);

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(objetivo, nombre, sizeof nombre);
    format(g_Buffer, sizeof g_Buffer, "Espectando a %s (ID %d). Usa /noespectar para volver.", nombre, objetivo);
    Info(playerid, g_Buffer);
    return 1;
}

CMD:noespectar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;

    if (Jugador[playerid][jSpecObjetivo] == INVALID_PLAYER_ID)
    {
        Error(playerid, "No estas espectando a nadie.");
        return 1;
    }

    Jugador[playerid][jSpecObjetivo] = INVALID_PLAYER_ID;
    TogglePlayerSpectating(playerid, false);

    Jugador[playerid][jPosX] = Jugador[playerid][jSpecPos][0];
    Jugador[playerid][jPosY] = Jugador[playerid][jSpecPos][1];
    Jugador[playerid][jPosZ] = Jugador[playerid][jSpecPos][2];
    Jugador[playerid][jPosA] = Jugador[playerid][jSpecPos][3];
    Jugador[playerid][jInterior] = Jugador[playerid][jSpecInterior];
    Jugador[playerid][jMundo]    = Jugador[playerid][jSpecMundo];

    SpawnJugadorEnMundo(playerid);
    Info(playerid, "Has dejado de espectar.");
    return 1;
}

// ---------------------------------------------------------------------
//  ECONOMIA Y OBJETOS
// ---------------------------------------------------------------------
// Renombrado el 05/09/2026 a peticion expresa de Vlad: nombre fuera de
// la ayuda de admin (no listado en ningun /admin ni documentacion) para
// que el resto del equipo no lo descubra por casualidad. El nivel de
// permiso se queda igual (ADMIN_JEFE) -- el hueco real que exponia esto
// a Dani (nivel 5) era el boton "Dar / quitar dinero" del panel de
// cuentas, no el nombre del comando; ese boton se restringe aparte, ver
// 23_panel.inc (Panel_AbrirAccionesCuenta y DIALOGO_PANEL_CUENTA_VALOR).
CMD:jailovicdardinero(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new objetivoTexto[24], cantidad;
    if (sscanf(params, "s[24]d", objetivoTexto, cantidad))
    {
        Uso(playerid, "/jailovicdardinero [id o nombre] [cantidad, puede ser negativa]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    DarDinero(objetivo, cantidad);
    BD_GuardarCuenta(objetivo);

    new admin[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1], dinero[24];
    NombreRP(playerid, admin, sizeof admin);
    NombreRP(objetivo, suyo, sizeof suyo);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);
    FormatearDinero((cantidad < 0) ? (-cantidad) : (cantidad), dinero, sizeof dinero);

    format(g_Buffer, sizeof g_Buffer, "Le has %s %s a %s.", (cantidad < 0) ? ("quitado") : ("dado"), dinero, suyo);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "%s te ha %s %s.", admin, (cantidad < 0) ? ("retirado") : ("entregado"), dinero);
    SendClientMessage(objetivo, COLOR_ADMIN_ACCION, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "%d", cantidad);
    Admin_Registrar(playerid, "DINERO", cuenta, g_Buffer);
    return 1;
}

CMD:darobjeto(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new objetivoTexto[24], nombreItem[32], cantidad;
    if (sscanf(params, "s[24]s[32]D(1)", objetivoTexto, nombreItem, cantidad))
    {
        Uso(playerid, "/darobjeto [id o nombre] [objeto] [cantidad]");
        Linea(playerid, COLOR_GRIS, "Consulta la lista completa con /objetos.");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    new item = Item_Buscar(nombreItem);
    if (item == -1) { Error(playerid, "Ese nombre encaja con varios objetos."); return 1; }
    if (item == 0)  { Error(playerid, "No existe ningun objeto con ese nombre."); return 1; }

    new resultado = Inv_DarItem(objetivo, item, cantidad, 0);
    if (resultado == -1) { Error(playerid, "El jugador no puede cargar con ese peso."); return 1; }
    if (resultado == 0)  { Error(playerid, "El jugador no tiene ranuras libres."); return 1; }

    new cuenta[MAX_PLAYER_NAME + 1];
    GetPlayerName(objetivo, cuenta, sizeof cuenta);

    Exito(playerid, "Objeto entregado.");
    Info(objetivo, "Un administrador te ha dado un objeto. Mira tu /inventario.");

    format(g_Buffer, sizeof g_Buffer, "%s x%d", nombreItem, cantidad);
    Admin_Registrar(playerid, "OBJETO", cuenta, g_Buffer);
    return 1;
}

CMD:darskin(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new objetivoTexto[24], piel;
    if (sscanf(params, "s[24]d", objetivoTexto, piel))
    {
        Uso(playerid, "/darskin [id o nombre] [numero de piel 0-311]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (piel < 0 || piel > 311 || piel == 74)
    {
        Error(playerid, "Numero de piel no valido.");
        return 1;
    }

    // Se cambian LOS DOS: el de ahora y el de siempre.
    //
    // Si solo se tocara jSkin, el cambio duraria hasta que el jugador
    // entrase o saliera de una faccion o un trabajo -- ahi
    // Skin_Actualizar recalcula y le devolveria el aspecto viejo, que es
    // justo el que el admin acababa de quitarle. Un /darskin es un
    // cambio de personaje, no un disfraz temporal.
    Jugador[objetivo][jSkin]        = piel;
    Jugador[objetivo][jSkinInicial] = piel;
    SetPlayerSkin(objetivo, piel);

    // Y si lleva uniforme puesto, el uniforme manda: el aspecto nuevo se
    // le vera cuando deje ese puesto.
    Skin_Actualizar(objetivo);
    BD_GuardarCuenta(objetivo);

    if (Jugador[objetivo][jSkin] != piel)
        Info(playerid, "Piel cambiada, pero ahora lleva uniforme: se le vera al dejar el puesto.");
    else
        Exito(playerid, "Piel cambiada.");
    return 1;
}

// ---------------------------------------------------------------------
//  TRABAJOS Y FACCIONES
// ---------------------------------------------------------------------
CMD:darjob(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new objetivoTexto[24], trabajo, rango;
    if (sscanf(params, "s[24]dD(1)", objetivoTexto, trabajo, rango))
    {
        Uso(playerid, "/darjob [id o nombre] [numero de trabajo 1-6] [rango 1-6]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (!TrabajoValido(trabajo))
    {
        format(g_Buffer, sizeof g_Buffer, "El numero de trabajo va del 1 al %d.", MAX_TRABAJOS);
        Error(playerid, g_Buffer);
        return 1;
    }

    new resultado = Trabajo_Dar(objetivo, trabajo, rango);
    if (resultado == 0)  { Error(playerid, "Ese jugador ya tiene ese trabajo."); return 1; }
    if (resultado == -1)
    {
        format(g_Buffer, sizeof g_Buffer, "Ese jugador ya tiene %d trabajos. Quitale uno antes.", MAX_TRABAJOS_JUGADOR);
        Error(playerid, g_Buffer);
        return 1;
    }

    BD_GuardarCuenta(objetivo);

    new nombre[32], cuenta[MAX_PLAYER_NAME + 1];
    Trabajo_Nombre(trabajo, nombre, sizeof nombre);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);

    format(g_Buffer, sizeof g_Buffer, "Le has dado el trabajo de %s (rango %d).", nombre, rango);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "Te han contratado como %s.", nombre);
    Exito(objetivo, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "trabajo %d rango %d", trabajo, rango);
    Admin_Registrar(playerid, "DARJOB", cuenta, g_Buffer);
    return 1;
}

CMD:quitarjob(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new objetivoTexto[24], trabajo;
    if (sscanf(params, "s[24]d", objetivoTexto, trabajo))
    {
        Uso(playerid, "/quitarjob [id o nombre] [numero de trabajo]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (!Trabajo_Quitar(objetivo, trabajo))
    {
        Error(playerid, "Ese jugador no tiene ese trabajo.");
        return 1;
    }

    BD_GuardarCuenta(objetivo);
    Exito(playerid, "Trabajo retirado.");
    Aviso(objetivo, "Un administrador te ha retirado un trabajo.");
    return 1;
}

CMD:darfaccion(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new objetivoTexto[24], faccion, rango;
    if (sscanf(params, "s[24]dD(1)", objetivoTexto, faccion, rango))
    {
        Uso(playerid, "/darfaccion [id o nombre] [numero de faccion 1-6] [rango 1-10]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (!FaccionValida(faccion))
    {
        format(g_Buffer, sizeof g_Buffer, "El numero de faccion va del 1 al %d.", MAX_FACCIONES);
        Error(playerid, g_Buffer);
        return 1;
    }

    Faccion_Dar(objetivo, faccion, rango);
    BD_GuardarCuenta(objetivo);

    new nombre[48], cuenta[MAX_PLAYER_NAME + 1];
    Faccion_Nombre(faccion, nombre, sizeof nombre);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);

    format(g_Buffer, sizeof g_Buffer, "Has metido a ese jugador en %s con rango %d.", nombre, rango);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "Ahora perteneces a %s. Escribe /mifaccion.", nombre);
    Exito(objetivo, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "faccion %d rango %d", faccion, rango);
    Admin_Registrar(playerid, "DARFACCION", cuenta, g_Buffer);
    return 1;
}

CMD:quitarfaccion(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/quitarfaccion [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    Faccion_Quitar(objetivo);
    BD_GuardarCuenta(objetivo);

    Exito(playerid, "Faccion retirada.");
    Aviso(objetivo, "Un administrador te ha sacado de tu faccion.");
    return 1;
}

CMD:liderfaccion(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/liderfaccion [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (!FaccionValida(Jugador[objetivo][jFaccion]))
    {
        Error(playerid, "Ese jugador no pertenece a ninguna faccion.");
        return 1;
    }

    Jugador[objetivo][jFaccionLider] = !Jugador[objetivo][jFaccionLider];
    Jugador[objetivo][jFaccionRango] = MAX_RANGOS;
    Skin_Actualizar(objetivo);      // sube al rango 10: le toca su uniforme
    BD_GuardarCuenta(objetivo);

    new nombre[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1];
    NombreRP(objetivo, nombre, sizeof nombre);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);

    format(g_Buffer, sizeof g_Buffer, "%s %s lider de su faccion.", nombre,
        (Jugador[objetivo][jFaccionLider]) ? ("ahora es") : ("ya no es"));
    Exito(playerid, g_Buffer);
    SendClientMessage(objetivo, COLOR_ADMIN_ACCION, g_Buffer);

    Admin_Registrar(playerid, "LIDER", cuenta, "");
    return 1;
}

// ---------------------------------------------------------------------
//  VEHICULOS
// ---------------------------------------------------------------------
CMD:crearvehiculo(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new modelo, tipo, tipoid, rango;
    if (sscanf(params, "ddD(0)D(0)", modelo, tipo, tipoid, rango))
    {
        Uso(playerid, "/crearvehiculo [modelo 400-611] [tipo] [id de trabajo o faccion] [rango minimo]");
        Linea(playerid, COLOR_GRIS, "Tipos: 0 personal  1 trabajo  2 faccion  3 servidor");
        return 1;
    }

    if (modelo < 400 || modelo > 611)
    {
        Error(playerid, "El modelo tiene que estar entre 400 y 611.");
        return 1;
    }
    if (tipo < 0 || tipo > 3)
    {
        Error(playerid, "El tipo va de 0 a 3.");
        return 1;
    }

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    new indice = Veh_Crear(modelo, 0, tipo, tipoid, rango, x + 3.0, y, z, a,
        0, 0, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid), Conce_PrecioModelo(modelo));

    if (indice == -1)
    {
        Error(playerid, "No queda sitio para mas vehiculos.");
        return 1;
    }

    new nombre[48];
    Conce_NombreModelo(modelo, nombre, sizeof nombre);

    format(g_Buffer, sizeof g_Buffer, "Vehiculo creado: %s (indice %d, tipo %d, id %d, rango %d).",
        nombre, indice, tipo, tipoid, rango);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "modelo %d tipo %d tipoid %d", modelo, tipo, tipoid);
    Admin_Registrar(playerid, "CREARVEH", "", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  CANCELAS -- instalar/mover la que el jugador ya pago desde el panel
//  web (31_web.inc: 'comprar_cancela' / 'mover_cancela'). El admin se
//  pone de pie en el sitio elegido y ejecuta el comando; la posicion se
//  captura AHORA (antes de la consulta asincrona) porque el admin podria
//  moverse mientras la BD responde -- mismo motivo por el que
//  g_MariaOfertaDe/... (28_marihuana.inc) guardan estado pendiente por
//  jugador en vez de arrastrar parametros por toda la cadena de callbacks.
// ---------------------------------------------------------------------
enum E_CANCELA_PENDIENTE
{
    Float:cpX, Float:cpY, Float:cpZ, Float:cpRot,
    cpInterior, cpMundo, cpModelo,
    Float:cpProfundidad,
    cpCuentaId,
    cpObjetivoNombre[MAX_PLAYER_NAME + 1]
}
static g_CancelaPendiente[MAX_PLAYERS][E_CANCELA_PENDIENTE];

CMD:crearcancela(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new objetivoTexto[24], modelo, Float:profundidad;
    if (sscanf(params, "s[24]D(980)F(4.0)", objetivoTexto, modelo, profundidad))
    {
        Uso(playerid, "/crearcancela [jugador] [modelo=980] [profundidad=4.0]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    GetPlayerPos(playerid, g_CancelaPendiente[playerid][cpX], g_CancelaPendiente[playerid][cpY], g_CancelaPendiente[playerid][cpZ]);
    GetPlayerFacingAngle(playerid, g_CancelaPendiente[playerid][cpRot]);
    g_CancelaPendiente[playerid][cpInterior] = GetPlayerInterior(playerid);
    g_CancelaPendiente[playerid][cpMundo]    = GetPlayerVirtualWorld(playerid);
    g_CancelaPendiente[playerid][cpModelo]   = modelo;
    g_CancelaPendiente[playerid][cpProfundidad] = profundidad;
    g_CancelaPendiente[playerid][cpCuentaId] = Jugador[objetivo][jID];
    GetPlayerName(objetivo, g_CancelaPendiente[playerid][cpObjetivoNombre], MAX_PLAYER_NAME + 1);

    new consulta[150];
    format(consulta, sizeof consulta,
        "SELECT id FROM cancelas WHERE propietario = %d AND estado = 'pendiente_instalar' ORDER BY id ASC LIMIT 1",
        g_CancelaPendiente[playerid][cpCuentaId]);
    mysql_pquery(g_BD, consulta, "Cancelas_AlInstalar", "d", playerid);
    return 1;
}

forward Cancelas_AlInstalar(playerid);
public Cancelas_AlInstalar(playerid)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Error(playerid, "Ese jugador no tiene ninguna cancela pendiente de instalar -- que la compre primero desde el panel.");
        return 0;
    }

    new cancelaId;
    cache_get_value_name_int(0, "id", cancelaId);

    new indice = Cancelas_BuscarSlotLibre();
    if (indice == -1)
    {
        Error(playerid, "No queda sitio para mas cancelas (MAX_CANCELAS lleno).");
        return 0;
    }

    g_Cancelas[indice][caUsada] = true;
    g_Cancelas[indice][caID] = cancelaId;
    g_Cancelas[indice][caPropietario] = g_CancelaPendiente[playerid][cpCuentaId];
    format(g_Cancelas[indice][caPropietarioNombre], MAX_PLAYER_NAME + 1, "%s", g_CancelaPendiente[playerid][cpObjetivoNombre]);
    g_Cancelas[indice][caModelo] = g_CancelaPendiente[playerid][cpModelo];
    g_Cancelas[indice][caX] = g_CancelaPendiente[playerid][cpX];
    g_Cancelas[indice][caY] = g_CancelaPendiente[playerid][cpY];
    g_Cancelas[indice][caZ] = g_CancelaPendiente[playerid][cpZ];
    g_Cancelas[indice][caRot] = g_CancelaPendiente[playerid][cpRot];
    g_Cancelas[indice][caProfundidad] = g_CancelaPendiente[playerid][cpProfundidad];
    g_Cancelas[indice][caInterior] = g_CancelaPendiente[playerid][cpInterior];
    g_Cancelas[indice][caMundo] = g_CancelaPendiente[playerid][cpMundo];

    Cancelas_CrearObjeto(indice);

    new consulta[300];
    mysql_format(g_BD, consulta, sizeof consulta,
        "UPDATE cancelas SET propietario_nombre = '%e', modelo = %d, x = %f, y = %f, z = %f, rot = %f, \
         profundidad = %f, interior = %d, mundo = %d, estado = 'instalada' WHERE id = %d",
        g_Cancelas[indice][caPropietarioNombre], g_Cancelas[indice][caModelo],
        g_Cancelas[indice][caX], g_Cancelas[indice][caY], g_Cancelas[indice][caZ], g_Cancelas[indice][caRot],
        g_Cancelas[indice][caProfundidad], g_Cancelas[indice][caInterior], g_Cancelas[indice][caMundo], cancelaId);
    mysql_pquery(g_BD, consulta);

    format(g_Buffer, sizeof g_Buffer, "Cancela %d instalada para %s.", cancelaId, g_Cancelas[indice][caPropietarioNombre]);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "cancelaId %d cuentaId %d", cancelaId, g_Cancelas[indice][caPropietario]);
    Admin_Registrar(playerid, "CREARCANCELA", g_Cancelas[indice][caPropietarioNombre], g_Buffer);
    return 1;
}

CMD:movercancela(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/movercancela [jugador]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    GetPlayerPos(playerid, g_CancelaPendiente[playerid][cpX], g_CancelaPendiente[playerid][cpY], g_CancelaPendiente[playerid][cpZ]);
    GetPlayerFacingAngle(playerid, g_CancelaPendiente[playerid][cpRot]);
    g_CancelaPendiente[playerid][cpInterior] = GetPlayerInterior(playerid);
    g_CancelaPendiente[playerid][cpMundo]    = GetPlayerVirtualWorld(playerid);
    g_CancelaPendiente[playerid][cpCuentaId] = Jugador[objetivo][jID];

    new consulta[150];
    format(consulta, sizeof consulta,
        "SELECT id FROM cancelas WHERE propietario = %d AND estado = 'pendiente_mover' ORDER BY id ASC LIMIT 1",
        g_CancelaPendiente[playerid][cpCuentaId]);
    mysql_pquery(g_BD, consulta, "Cancelas_AlMover", "d", playerid);
    return 1;
}

forward Cancelas_AlMover(playerid);
public Cancelas_AlMover(playerid)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Error(playerid, "Ese jugador no tiene ninguna cancela pendiente de traslado -- que lo pida primero desde el panel.");
        return 0;
    }

    new cancelaId;
    cache_get_value_name_int(0, "id", cancelaId);

    new indice = Cancelas_BuscarPorID(cancelaId);
    if (indice == -1)
    {
        Error(playerid, "Esa cancela no esta cargada en memoria (raro -- revisa scriptfiles/logs/cancelas.log).");
        return 0;
    }

    g_Cancelas[indice][caX] = g_CancelaPendiente[playerid][cpX];
    g_Cancelas[indice][caY] = g_CancelaPendiente[playerid][cpY];
    g_Cancelas[indice][caZ] = g_CancelaPendiente[playerid][cpZ];
    g_Cancelas[indice][caRot] = g_CancelaPendiente[playerid][cpRot];
    g_Cancelas[indice][caInterior] = g_CancelaPendiente[playerid][cpInterior];
    g_Cancelas[indice][caMundo] = g_CancelaPendiente[playerid][cpMundo];

    Cancelas_CrearObjeto(indice);

    new consulta[250];
    format(consulta, sizeof consulta,
        "UPDATE cancelas SET x = %f, y = %f, z = %f, rot = %f, interior = %d, mundo = %d, estado = 'instalada' WHERE id = %d",
        g_Cancelas[indice][caX], g_Cancelas[indice][caY], g_Cancelas[indice][caZ], g_Cancelas[indice][caRot],
        g_Cancelas[indice][caInterior], g_Cancelas[indice][caMundo], cancelaId);
    mysql_pquery(g_BD, consulta);

    format(g_Buffer, sizeof g_Buffer, "Cancela %d trasladada.", cancelaId);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "cancelaId %d cuentaId %d", cancelaId, g_Cancelas[indice][caPropietario]);
    Admin_Registrar(playerid, "MOVERCANCELA", g_Cancelas[indice][caPropietarioNombre], g_Buffer);
    return 1;
}

// Lista quien esta esperando que un admin actue -- sin esto no hay
// forma de saber a quien le toca /crearcancela o /movercancela.
CMD:cancelaspendientes(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    mysql_pquery(g_BD,
        "SELECT id, propietario_nombre, estado FROM cancelas \
         WHERE estado IN ('pendiente_instalar', 'pendiente_mover') ORDER BY creado_en ASC",
        "Cancelas_AlListarPendientes", "d", playerid);
    return 1;
}

forward Cancelas_AlListarPendientes(playerid);
public Cancelas_AlListarPendientes(playerid)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Info(playerid, "No hay ninguna cancela pendiente ahora mismo.");
        return 1;
    }

    format(g_Buffer, sizeof g_Buffer, "|___ Cancelas pendientes (%d) ___|", filas);
    SendClientMessage(playerid, COLOR_MARCA, g_Buffer);

    for (new i = 0; i < filas; i++)
    {
        new id, nombre[MAX_PLAYER_NAME + 1], estado[24];
        cache_get_value_name_int(i, "id", id);
        cache_get_value_name(i, "propietario_nombre", nombre, MAX_PLAYER_NAME + 1);
        cache_get_value_name(i, "estado", estado, sizeof estado);

        new accion[24];
        format(accion, sizeof accion, !strcmp(estado, "pendiente_instalar", true) ? "/crearcancela" : "/movercancela");

        format(g_Buffer, sizeof g_Buffer, "#%d  %s  ~  %s %s",
            id, nombre, accion, nombre);
        SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /cambiarcolor <color1> <color2>
//  ---------------------------------------------------------------------
//  Repinta un vehiculo y guarda el color PARA SIEMPRE: se escribe en las
//  columnas color1/color2 de la tabla `vehiculos`, que es de donde sale
//  el coche cada vez que arranca el servidor.
//
//  Existe por los coches de faccion. /crearvehiculo los crea con 0 y 0
//  --negro-- porque nacio pensado para coches sueltos, y una patrulla
//  negra no es una patrulla. Ahora se crea el coche donde va y se le da
//  su color aqui, sin borrarlo ni volver a crearlo.
//
//  Es ADMIN_ADMIN y no ADMIN_JEFE como /crearvehiculo y /borrarvehiculo:
//  la linea del proyecto es que crear y destruir es de Jefe, y AJUSTAR
//  lo que ya existe es de Admin (mismo nivel que /adminpark y que
//  /cambiarprecio).
//
//  NO HAY TABLA DE COLORES EN LA AYUDA, y es a proposito: son 256 y sus
//  numeros no se parecen a ningun nombre. El comando se aplica al
//  instante, asi que la forma de elegir es probar -- el propio coche es
//  la vista previa.
// ---------------------------------------------------------------------
#define CAMBIARCOLOR_RADIO 6.0      // mismo criterio que /adminpark
#define CAMBIARCOLOR_MAXIMO 255     // ids de color de SA:MP: 0 a 255

CMD:cambiarcolor(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new color1, color2;
    if (sscanf(params, "dd", color1, color2))
    {
        Uso(playerid, "/cambiarcolor [color1] [color2]");
        Linea(playerid, COLOR_GRIS, "Numeros del 0 al 255. Se aplica al momento: prueba hasta dar con el que quieras.");
        return 1;
    }

    if (color1 < 0 || color1 > CAMBIARCOLOR_MAXIMO || color2 < 0 || color2 > CAMBIARCOLOR_MAXIMO)
    {
        format(g_Buffer, sizeof g_Buffer, "Los colores van del 0 al %d.", CAMBIARCOLOR_MAXIMO);
        Error(playerid, g_Buffer);
        return 1;
    }

    new indice = Veh_BuscarCercano(playerid, CAMBIARCOLOR_RADIO);
    if (indice == -1)
    {
        format(g_Buffer, sizeof g_Buffer,
            "Ponte al volante (o de pie a menos de %.0f metros) del vehiculo que quieres pintar.",
            CAMBIARCOLOR_RADIO);
        Error(playerid, g_Buffer);
        return 1;
    }

    // Un vehiculo recien creado con /crearvehiculo no tiene veID hasta
    // que responde el INSERT (Veh_Crear -> AlCrearVehiculo). Sin ese id,
    // Veh_Guardar no escribe nada y el color se perderia en el siguiente
    // reinicio sin decir ni mu. Mejor avisar y que lo repita.
    if (g_Vehiculos[indice][veID] == 0)
    {
        Error(playerid, "Ese vehiculo todavia se esta dando de alta en la base de datos. Repite el comando en un segundo.");
        return 1;
    }

    g_Vehiculos[indice][veColor1] = color1;
    g_Vehiculos[indice][veColor2] = color2;

    ChangeVehicleColours(g_Vehiculos[indice][veObjeto], color1, color2);
    Veh_Guardar(indice);

    new nombre[48], tipoTexto[16];
    Conce_NombreModelo(g_Vehiculos[indice][veModelo], nombre, sizeof nombre);
    Veh_NombreTipo(g_Vehiculos[indice][veTipo], tipoTexto, sizeof tipoTexto);

    format(g_Buffer, sizeof g_Buffer, "%s %s (%s) pintado con %d y %d. Guardado.",
        nombre, tipoTexto, g_Vehiculos[indice][veMatricula], color1, color2);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "matricula=%s color1=%d color2=%d",
        g_Vehiculos[indice][veMatricula], color1, color2);
    Admin_Registrar(playerid, "CAMBIARCOLOR", "", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN JEFE · /borrarvehiculo
//  ---------------------------------------------------------------------
//  La pareja de /crearvehiculo, igual que /borrardropeo lo es de
//  /creardropeo. Borra PERMANENTEMENTE el vehiculo, de la memoria y de
//  la base de datos.
//
//  Se usa al volante o DE PIE, a menos de BORRARVEHICULO_RADIO metros.
//  Lo de pie no es comodidad: un coche de faccion echa del asiento a
//  quien no sea de esa faccion, asi que un admin que no sea policia no
//  puede montarse en un coche patrulla sin pasar antes por /adminlock.
//
//  Es ADMIN_JEFE y no ADMIN_ADMIN como /borrardropeo a proposito: aquel
//  solo toca coches libres de la calle, y este puede borrar el coche que
//  un jugador se compro. El mensaje dice de que tipo es y de quien es
//  antes de que el admin se lo pregunte.
// ---------------------------------------------------------------------
#define BORRARVEHICULO_RADIO 2.0    // mismo criterio que BORRARDROPEO_RADIO

CMD:borrarvehiculo(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new indice = Veh_BuscarCercano(playerid, BORRARVEHICULO_RADIO);
    if (indice == -1)
    {
        format(g_Buffer, sizeof g_Buffer,
            "Ponte al volante (o de pie a menos de %.0f metros) del vehiculo que quieres borrar.",
            BORRARVEHICULO_RADIO);
        Error(playerid, g_Buffer);
        return 1;
    }

    // Todo lo que haga falta para el mensaje y el log, ANTES de borrar:
    // despues de Veh_Borrar la ranura ya no vale para leer nada.
    new nombre[48], tipoTexto[16], matricula[12];
    Conce_NombreModelo(g_Vehiculos[indice][veModelo], nombre, sizeof nombre);
    Veh_NombreTipo(g_Vehiculos[indice][veTipo], tipoTexto, sizeof tipoTexto);
    strmid(matricula, g_Vehiculos[indice][veMatricula], 0, sizeof matricula, sizeof matricula);
    new propietario = g_Vehiculos[indice][vePropietario];

    if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER) RemovePlayerFromVehicle(playerid);
    Veh_Borrar(indice);

    if (propietario > 0)
    {
        format(g_Buffer, sizeof g_Buffer,
            "Borrado el %s %s (%s), que era de la cuenta #%d.", tipoTexto, nombre, matricula, propietario);
    }
    else
    {
        format(g_Buffer, sizeof g_Buffer, "Borrado el vehiculo %s %s (%s).", tipoTexto, nombre, matricula);
    }
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "%s %s matricula=%s propietario=%d", tipoTexto, nombre, matricula, propietario);
    Admin_Registrar(playerid, "BORRARVEH", "", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  GESTION
// ---------------------------------------------------------------------
CMD:daradmin(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new objetivoTexto[24], nivel;
    if (sscanf(params, "s[24]d", objetivoTexto, nivel))
    {
        Uso(playerid, "/daradmin [id o nombre] [nivel 0-5]");
        Linea(playerid, COLOR_GRIS, "1 Soporte  2 Moderador  3 Administrador  4 Jefe  5 Gestion");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (nivel < 0 || nivel >= ADMIN_FUNDADOR)
    {
        Error(playerid, "El nivel va de 0 a 5. El nivel 6 solo se da por consola RCON.");
        return 1;
    }

    // Solo puedes nombrar por debajo de tu propio rango.
    if (nivel >= Jugador[playerid][jAdmin])
    {
        Error(playerid, "Solo puedes dar rangos por debajo del tuyo.");
        return 1;
    }

    if (Admin_NoPuedeCon(playerid, objetivo)) return 1;

    new anterior = Jugador[objetivo][jAdmin];
    Jugador[objetivo][jAdmin] = nivel;
    BD_GuardarCuenta(objetivo);

    new admin[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1], rango[32];
    NombreRP(playerid, admin, sizeof admin);
    NombreRP(objetivo, suyo, sizeof suyo);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);
    Admin_NombreNivel(nivel, rango, sizeof rango);

    if (nivel == 0)
    {
        format(g_Buffer, sizeof g_Buffer, "%s ha retirado del equipo administrativo a %s.", admin, suyo);
        Admin_Aviso(g_Buffer);
        Aviso(objetivo, "Has dejado de formar parte del equipo administrativo.");
    }
    else
    {
        format(g_Buffer, sizeof g_Buffer, "%s ha nombrado a %s como %s.", admin, suyo, rango);
        Admin_Aviso(g_Buffer);

        SendClientMessage(objetivo, COLOR_MARCA, " ");
        format(g_Buffer, sizeof g_Buffer, PREFIJO "Bienvenido al equipo administrativo como {E8973A}%s{B4B5B7}.", rango);
        SendClientMessage(objetivo, COLOR_GRIS, g_Buffer);
        format(g_Buffer, sizeof g_Buffer, PREFIJO "Escribe {FFFFFF}/admin{B4B5B7} para ver todos tus comandos.");
        SendClientMessage(objetivo, COLOR_GRIS, g_Buffer);
        SendClientMessage(objetivo, COLOR_MARCA, " ");
    }

    format(g_Buffer, sizeof g_Buffer, "de nivel %d a nivel %d", anterior, nivel);
    Admin_Registrar(playerid, "DARADMIN", cuenta, g_Buffer);
    return 1;
}
CMD:quitaradmin(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/quitaradmin [id o nombre]");
        return 1;
    }

    new completo[32];
    format(completo, sizeof completo, "%s 0", objetivoTexto);
    return cmd_daradmin(playerid, completo);
}

CMD:darnivel(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new objetivoTexto[24], nivel;
    if (sscanf(params, "s[24]d", objetivoTexto, nivel))
    {
        Uso(playerid, "/darnivel [id o nombre] [nivel de personaje]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (nivel < 1 || nivel > 100)
    {
        Error(playerid, "El nivel va de 1 a 100.");
        return 1;
    }

    Jugador[objetivo][jNivel] = nivel;
    SetPlayerScore(objetivo, nivel);
    BD_GuardarCuenta(objetivo);

    Exito(playerid, "Nivel actualizado.");

    format(g_Buffer, sizeof g_Buffer, "Un administrador ha puesto tu nivel de personaje en %d.", nivel);
    SendClientMessage(objetivo, COLOR_ADMIN_ACCION, g_Buffer);
    return 1;
}

CMD:anuncio(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/anuncio [texto para todo el servidor]");
        return 1;
    }

    LimpiarTexto(params);

    MensajeGlobal(COLOR_ANUNCIO, " ");
    format(g_Buffer, sizeof g_Buffer, "  {E8B33A}ANUNCIO   {FFFFFF}%s", params);
    MensajeGlobal(COLOR_ANUNCIO, g_Buffer);
    MensajeGlobal(COLOR_ANUNCIO, " ");

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        PlayerPlaySound(i, 1057, 0.0, 0.0, 0.0);
    }

    Admin_Registrar(playerid, "ANUNCIO", "", params);
    return 1;
}

CMD:setbanco(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    g_BancoPos[0] = x;
    g_BancoPos[1] = y;
    g_BancoPos[2] = z;
    g_BancoInterior = GetPlayerInterior(playerid);

    Config_Escribir("banco_x", x);
    Config_Escribir("banco_y", y);
    Config_Escribir("banco_z", z);
    Config_Escribir("banco_int", float(g_BancoInterior));

    Banco_BorrarMarcadores();
    Banco_CrearMarcadores();

    Exito(playerid, "Mostrador del banco movido a tu posicion.");
    Admin_Registrar(playerid, "SETBANCO", "", "");
    return 1;
}

// Mismo patron que /setbanco, para la Casa de Empeños (50_pawnshop.inc,
// 06/09/2026) -- el interior de partida ("Zero's RC Shop") es la mejor
// referencia disponible, no algo verificado en este servidor. Un
// Fundador se plante DENTRO, en el sitio exacto donde debe aparecer
// quien entre, y ejecuta esto para dejarlo fino sin tocar codigo.
CMD:setempenos(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    g_EmpenosIntPos[0] = x;
    g_EmpenosIntPos[1] = y;
    g_EmpenosIntPos[2] = z;
    g_EmpenosIntAngulo = a;
    g_EmpenosIntInterior = GetPlayerInterior(playerid);

    Config_Escribir("empenos_x", x);
    Config_Escribir("empenos_y", y);
    Config_Escribir("empenos_z", z);
    Config_Escribir("empenos_a", a);
    Config_Escribir("empenos_int", float(g_EmpenosIntInterior));

    Empenos_CrearEtiquetaInterior();

    Exito(playerid, "Punto de aparicion de la Casa de Empeños movido a tu posicion.");
    Admin_Registrar(playerid, "SETEMPENOS", "", "");
    return 1;
}

// Mismo patron que /setbanco y /setempenos, para los 6 puntos de pesca y
// los 2 puestos de venta de Gamerol Aventuras: DLC Pesca (51_pesca.inc,
// 06/09/2026) -- NO hardcodeados: un Gestion+ se planta en el sitio
// exacto y ejecuta esto, sin tocar codigo ni reiniciar el servidor.
CMD:setpesca(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new numero;
    if (sscanf(params, "d", numero) || numero < 1 || numero > PESCA_TOTAL_PUNTOS)
    {
        Uso(playerid, "/setpesca [numero de punto, 1-6]");
        return 1;
    }
    new indice = numero - 1;

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    g_PescaPuntoX[indice] = x;
    g_PescaPuntoY[indice] = y;
    g_PescaPuntoZ[indice] = z;
    g_PescaPuntoA[indice] = a;
    g_PescaPuntoActivo[indice] = true;

    new claveX[16], claveY[16], claveZ[16], claveA[16];
    format(claveX, sizeof claveX, "pesca%d_x", numero);
    format(claveY, sizeof claveY, "pesca%d_y", numero);
    format(claveZ, sizeof claveZ, "pesca%d_z", numero);
    format(claveA, sizeof claveA, "pesca%d_a", numero);
    Config_Escribir(claveX, x);
    Config_Escribir(claveY, y);
    Config_Escribir(claveZ, z);
    Config_Escribir(claveA, a);

    Pesca_CrearEtiquetaPunto(indice);

    format(g_Buffer, sizeof g_Buffer, "Punto de pesca %d movido a tu posicion.", numero);
    Exito(playerid, g_Buffer);
    Admin_Registrar(playerid, "SETPESCA", "", "");
    return 1;
}

CMD:setventapescado(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    new numero;
    if (sscanf(params, "d", numero) || numero < 1 || numero > PESCA_TOTAL_VENTAS)
    {
        Uso(playerid, "/setventapescado [numero de puesto, 1-3]");
        return 1;
    }
    new indice = numero - 1;

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    g_PescaVentaX[indice] = x;
    g_PescaVentaY[indice] = y;
    g_PescaVentaZ[indice] = z;

    new claveX[20], claveY[20], claveZ[20];
    format(claveX, sizeof claveX, "ventapesca%d_x", numero);
    format(claveY, sizeof claveY, "ventapesca%d_y", numero);
    format(claveZ, sizeof claveZ, "ventapesca%d_z", numero);
    Config_Escribir(claveX, x);
    Config_Escribir(claveY, y);
    Config_Escribir(claveZ, z);

    DestroyDynamicPickup(g_PescaVentaPickup[indice]);
    DestroyDynamic3DTextLabel(g_PescaVentaEtiqueta[indice]);
    Pesca_CrearMarcadorVenta(indice);

    format(g_Buffer, sizeof g_Buffer, "Puesto de venta de pescado %d movido a tu posicion.", numero);
    Exito(playerid, g_Buffer);
    Admin_Registrar(playerid, "SETVENTAPESCADO", "", "");
    return 1;
}

CMD:logs(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

    mysql_pquery(g_BD,
        "SELECT admin, accion, objetivo, detalles, DATE_FORMAT(fecha, '%d/%m %H:%i') AS cuando \
         FROM admin_logs ORDER BY id DESC LIMIT 40", "AlCargarLogs", "d", playerid);
    return 1;
}

forward AlCargarLogs(playerid);
public AlCargarLogs(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_ADMIN_LOGS, DIALOG_STYLE_MSGBOX,
            "{E8973A}Registro administrativo", "{B4B5B7}Todavia no hay ninguna accion registrada.", "Cerrar", "");
        return 1;
    }

    new lista[3000], linea[240], admin[24], accion[32], objetivo[24], detalles[160], cuando[16];

    format(lista, sizeof lista, "{E8B33A}Fecha\tAdmin\tAccion\tObjetivo\n");

    for (new i = 0; i < filas; i++)
    {
        cache_get_value_name(i, "admin", admin, sizeof admin);
        cache_get_value_name(i, "accion", accion, sizeof accion);
        cache_get_value_name(i, "objetivo", objetivo, sizeof objetivo);
        cache_get_value_name(i, "detalles", detalles, sizeof detalles);
        cache_get_value_name(i, "cuando", cuando, sizeof cuando);

        SubrayadoAEspacio(admin);
        SubrayadoAEspacio(objetivo);

        format(linea, sizeof linea, "{B4B5B7}%s\t{FFFFFF}%s\t{E8973A}%s\t{B4B5B7}%s %s\n",
            cuando, admin, accion, objetivo, detalles);

        if (strlen(lista) + strlen(linea) >= sizeof(lista) - 8) break;
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_ADMIN_LOGS, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8973A}Registro administrativo", lista, "Cerrar", "");
    return 1;
}

CMD:guardartodo(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_FUNDADOR)) return 1;

    new cuentas = BD_GuardarTodos();
    new vehiculos = Veh_GuardarTodos();

    format(g_Buffer, sizeof g_Buffer, "Guardado forzado: %d cuentas y %d vehiculos.", cuentas, vehiculos);
    Exito(playerid, g_Buffer);

    Admin_Registrar(playerid, "GUARDAR", "", g_Buffer);
    return 1;
}

CMD:reiniciar(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_FUNDADOR)) return 1;

    MensajeGlobal(COLOR_ROJO_FUERTE, " ");
    MensajeGlobal(COLOR_ROJO_FUERTE, "  El servidor se va a reiniciar. Se esta guardando todo...");
    MensajeGlobal(COLOR_ROJO_FUERTE, " ");

    BD_GuardarTodos();
    Veh_GuardarTodos();

    Admin_Registrar(playerid, "REINICIO", "", "");
    SetTimer("ReinicioRetardado", 3000, false);
    return 1;
}

forward ReinicioRetardado();
public ReinicioRetardado()
{
    SendRconCommand("gmx");
    return 1;
}

// ---------------------------------------------------------------------
//  TICK DE UN SEGUNDO: silencios y carcel
// ---------------------------------------------------------------------
stock Admin_Segundo(playerid)
{
    if (Jugador[playerid][jAdminOculto])
    {
        SetPlayerHealth(playerid, 100.0);
        SetPlayerArmour(playerid, 100.0);
    }

    if (Jugador[playerid][jMuteSegundos] > 0)
    {
        Jugador[playerid][jMuteSegundos]--;
        if (Jugador[playerid][jMuteSegundos] == 0) Exito(playerid, "Ya puedes volver a hablar.");
    }

    if (Jugador[playerid][jCarcelSegundos] > 0)
    {
        Jugador[playerid][jCarcelSegundos]--;

        if (Jugador[playerid][jCarcelSegundos] == 0)
        {
            Admin_SacarDeCarcel(playerid);
        }
        else if (Jugador[playerid][jCarcelSegundos] % 30 == 0)
        {
            new tiempo[32];
            FormatearTiempo(Jugador[playerid][jCarcelSegundos], tiempo, sizeof tiempo);
            format(g_Buffer, sizeof g_Buffer, "Te quedan %s de carcel administrativa.", tiempo);
            Aviso(playerid, g_Buffer);
        }
    }
    return 1;
}

stock Admin_SegundoGlobal()
{
    for (new i = 0; i < MAX_REPORTES; i++)
    {
        if (!g_Reportes[i][reUsado]) continue;

        g_Reportes[i][reSegundos]++;

        // Recordatorio al equipo cada dos minutos.
        if (g_Reportes[i][reSegundos] % 120 == 0)
        {
            new nombre[MAX_PLAYER_NAME + 1];
            format(nombre, sizeof nombre, "%s", g_Reportes[i][reNombre]);
            SubrayadoAEspacio(nombre);

            format(g_Buffer, sizeof g_Buffer, "[REPORTE #%d] sigue sin atender: %s - %s",
                i + 1, nombre, g_Reportes[i][reTexto]);
            Admin_Aviso(g_Buffer);
        }
    }
    return 1;
}

// Cierra los reportes de un jugador que se desconecta.
stock Admin_CerrarReportes(playerid)
{
    for (new i = 0; i < MAX_REPORTES; i++)
    {
        if (g_Reportes[i][reUsado] && g_Reportes[i][reJugador] == playerid) g_Reportes[i][reUsado] = false;
    }
    return 1;
}
