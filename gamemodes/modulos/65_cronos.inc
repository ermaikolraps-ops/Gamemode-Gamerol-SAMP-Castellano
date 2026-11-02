/* =====================================================================
 *  65_cronos.inc -- CRONOS GAMEROL V1 (14/09/2026)
 *  ---------------------------------------------------------------------
 *  Encargo expreso de Vlad (email "Gamerol 2026 -- Sistema CRONOS
 *  GAMEROL V1"): un nucleo PEQUEÑO y extensible de desafios contrarreloj
 *  que Vlad pueda sembrar por San Andreas sin tocar codigo. Cada crono es
 *  solo: punto de salida + punto de meta + nombre + ranking persistente.
 *  Sin checkpoints intermedios, sin ruta marcada, cualquier vehiculo.
 *
 *  REUTILIZADO (investigado antes de escribir una linea, tal como pide
 *  el encargo) en vez de inventar sistemas paralelos:
 *    - Textdraws titulo+subtitulo Pricedown en Y=380/402, mismo patron
 *      exacto que /puente (22_dropeo.inc, Dropeo_CrearTextdrawsPuente) y
 *      /tratamiento (64_hospitalinterior.inc) -- cuenta atras, "¡YA!" y
 *      los 4 mensajes finales viven todos ahi.
 *    - Freeze de controles durante la cuenta atras: TogglePlayerControllable,
 *      identico a como /puente congela al jugador durante sus 4s.
 *    - Reparacion gratis del premio: Arreglarcoche_Reparar(vehicleid,
 *      indice) (53_arreglarcoche.inc), extraida ese mismo dia de
 *      CMD:arreglarcoche para poder llamarla sin cobrar.
 *    - Combustible al 100%: asignacion directa a veCombustible +
 *      Veh_Guardar, mismo patron que /repostar y la creacion de
 *      vehiculos nuevos (13_vehiculos.inc) -- no existe un stock
 *      "SetCombustible" generico en el proyecto, este es el patron real.
 *    - Ranking con record en memoria + "solo se guarda si mejora",
 *      mismo esqueleto que Pesca_ComprobarRecord (51_pesca.inc):
 *      comparar ANTES de escribir, para que el record mundial nunca se
 *      pise a si mismo. Aqui se compara siempre contra la base de datos
 *      (una consulta agregada por llegada) en vez de un cache en
 *      memoria, porque Cronos puede tener muchos jugadores distintos por
 *      pista (a diferencia de pesca, que solo guarda 10 filas fijas).
 *    - Dialogo TABLIST_HEADERS de /vercronos y ranking MSGBOX, mismo
 *      patron que Comida_MostrarMenu (42_comida.inc) y CMD:rankingpesca
 *      (51_pesca.inc).
 *    - Identidad de jugador: Jugador[playerid][jID] (cuentas.id), igual
 *      que todo el resto del proyecto.
 *
 *  DECISIONES DE DISEÑO explicitas (para que quede constancia, item 19
 *  del encargo -- "distinguir estrictamente HECHO/IMPLEMENTADO..."):
 *    - Deteccion de llegada a meta: POLLING de distancia en un timer
 *      propio (100ms mientras corres), NUNCA OnPlayerEnterCheckpoint.
 *      Investigado antes: ese callback solo lo escuchan hoy
 *      VendedorArmas y MariaVendedor, y el checkpoint nativo de SA-MP es
 *      UNICO por jugador -- enganchar Cronos ahi habria significado
 *      filtrar con un flag propio de todas formas. El polling evita
 *      tocar ese dispatcher compartido por completo.
 *    - Cambio de vehiculo/salida del vehiculo: NO se usa
 *      OnPlayerStateChange. El mismo timer de 100ms ya comprueba en cada
 *      tick que GetPlayerVehicleID(playerid) siga siendo el vehiculo con
 *      el que empezo -- bajarse cuenta como "cambio" automaticamente
 *      (GetPlayerVehicleID pasa a 0), sin necesitar un segundo hook.
 *    - Atomicidad del premio de record mundial: SA-MP/open.mp ejecuta
 *      todo el codigo del gamemode en un unico hilo de logica -- dos
 *      llegadas "simultaneas" siempre se procesan como dos callbacks
 *      ordenados, nunca en paralelo de verdad. Comparar-y-decidir dentro
 *      del mismo callback (antes de que corra cualquier otro codigo del
 *      juego) es suficiente para que el premio nunca se de dos veces; el
 *      UPDATE de guardado ademas lleva su propio IF(...) en SQL como
 *      segunda red de seguridad barata (item 16 del encargo).
 *    - Todas las cronos viven en MUNDO_NORMAL, interior 0 (el mapa
 *      exterior real) -- el encargo nunca pide interiores propios y
 *      todos los ejemplos (Chiliad, Angel Pine, Ruta 69...) son
 *      ubicaciones exteriores reales.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  DEFINICION DE UNA CRONO -- cargadas en memoria al arrancar
//  (Cronos_Iniciar) y anadidas en caliente por /crearcrono. `activo`
//  solo pasa a true tras /crearmeta (item 2/3: no disponible
//  publicamente hasta tener meta valida). `eliminado` es solo en
//  memoria: lo pone /borrarcrono tras el DELETE real en SQL, para no
//  tener que compactar el array.
// ---------------------------------------------------------------------
enum E_CRONO
{
    crId,
    crNombre[48],
    Float:crSalidaX,
    Float:crSalidaY,
    Float:crSalidaZ,
    Float:crSalidaAngulo,
    Float:crMetaX,
    Float:crMetaY,
    Float:crMetaZ,
    bool:crActivo,
    bool:crEliminado,
    STREAMER_TAG_PICKUP:crPickup,
    STREAMER_TAG_3D_TEXT_LABEL:crEtiqueta
}
new g_Cronos[CRONOS_MAX][E_CRONO];
new g_CronosTotal = 0;

// Crono que cada admin tiene actualmente "en edicion" tras /crearcrono y
// antes de /crearmeta -- mismo concepto que g_ToyEditando (41_toys.inc):
// un slot de estado por jugador, -1 = ninguno.
new g_CronosEditando[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  ESTADO DE UN INTENTO EN CURSO, por jugador.
// ---------------------------------------------------------------------
enum { CR_NINGUNO, CR_CUENTA_ATRAS, CR_CORRIENDO }
new g_CronosEstado[MAX_PLAYERS];
new g_CronosIndice[MAX_PLAYERS];       // que crono esta corriendo (indice en g_Cronos)
new g_CronosVehiculo[MAX_PLAYERS];     // vehicleid con el que empezo -- cambiarlo cancela
new g_CronosInicio[MAX_PLAYERS];       // GetTickCount() en el instante exacto del "¡YA!"
new g_CronosRestante[MAX_PLAYERS];     // segundos restantes de la cuenta atras mostrados
new g_CronosTimerCuenta[MAX_PLAYERS];      // timer 1000ms de la cuenta atras, -1 = ninguno
new g_CronosTimerCorriendo[MAX_PLAYERS];   // timer 100ms mientras corre, -1 = ninguno

// Textdraws titulo+subtitulo (cuenta atras / "¡YA!" / mensajes finales),
// mismo patron visual que /puente -- y el cronometro en vivo aparte,
// pequeno arriba del todo (item 5 del encargo).
enum { CRT_TITULO, CRT_SUBTITULO, CRT_TOTAL }
static PlayerText:g_CronosTD[MAX_PLAYERS][CRT_TOTAL];
static PlayerText:g_CronosTDCron[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  TEXTDRAWS
// ---------------------------------------------------------------------
stock Cronos_CrearTextdraws(playerid)
{
    g_CronosTD[playerid][CRT_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 380.0, "LLEGA AL PUNTO ROJO");
    PlayerTextDrawAlignment(playerid, g_CronosTD[playerid][CRT_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_CronosTD[playerid][CRT_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_CronosTD[playerid][CRT_TITULO], 0.42, 1.8);
    PlayerTextDrawColour(playerid, g_CronosTD[playerid][CRT_TITULO], 0x4FD1C5FF);
    PlayerTextDrawSetOutline(playerid, g_CronosTD[playerid][CRT_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_CronosTD[playerid][CRT_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_CronosTD[playerid][CRT_TITULO], true);

    g_CronosTD[playerid][CRT_SUBTITULO] = CreatePlayerTextDraw(playerid, 320.0, 402.0, "5");
    PlayerTextDrawAlignment(playerid, g_CronosTD[playerid][CRT_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_CronosTD[playerid][CRT_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_CronosTD[playerid][CRT_SUBTITULO], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_CronosTD[playerid][CRT_SUBTITULO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_CronosTD[playerid][CRT_SUBTITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_CronosTD[playerid][CRT_SUBTITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_CronosTD[playerid][CRT_SUBTITULO], true);

    // Cronometro en vivo: pequeno, arriba del todo, centrado (item 5 del
    // encargo, literal). Misma familia visual que /puente, pero es un
    // textdraw aparte porque vive en pantalla TODO el intento, no solo
    // como aviso puntual.
    g_CronosTDCron[playerid] = CreatePlayerTextDraw(playerid, 320.0, 8.0, "CRONO 00:00.000");
    PlayerTextDrawAlignment(playerid, g_CronosTDCron[playerid], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_CronosTDCron[playerid], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_CronosTDCron[playerid], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_CronosTDCron[playerid], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_CronosTDCron[playerid], 1);
    PlayerTextDrawSetShadow(playerid, g_CronosTDCron[playerid], 0);
    PlayerTextDrawSetProportional(playerid, g_CronosTDCron[playerid], true);
    return 1;
}

forward Cronos_OcultarAvisoSuperior(playerid);
public Cronos_OcultarAvisoSuperior(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    PlayerTextDrawHide(playerid, g_CronosTD[playerid][CRT_TITULO]);
    PlayerTextDrawHide(playerid, g_CronosTD[playerid][CRT_SUBTITULO]);
    return 1;
}

// MM:SS.mmm -- formato pedido literal (item 5 del encargo).
stock Cronos_FormatearTiempo(ms, destino[], tam)
{
    new minutos = ms / 60000;
    new segundos = (ms / 1000) % 60;
    new milis = ms % 1000;
    format(destino, tam, "%02d:%02d.%03d", minutos, segundos, milis);
    return 1;
}

// ---------------------------------------------------------------------
//  CICLO DE VIDA DEL JUGADOR
// ---------------------------------------------------------------------
stock Cronos_OnConectar(playerid)
{
    g_CronosEditando[playerid]       = -1;
    g_CronosEstado[playerid]         = CR_NINGUNO;
    g_CronosIndice[playerid]         = -1;
    g_CronosVehiculo[playerid]       = INVALID_VEHICLE_ID;
    g_CronosInicio[playerid]         = 0;
    g_CronosRestante[playerid]       = 0;
    g_CronosTimerCuenta[playerid]    = -1;
    g_CronosTimerCorriendo[playerid] = -1;

    Cronos_CrearTextdraws(playerid);
    return 1;
}

// Para desde cualquier situacion: cancelacion, timeout o fin normal --
// mata los dos timers si estan vivos, oculta todo, quita el checkpoint.
// NO decide mensajes ni si hay que soltar los controles: eso lo hace
// quien llama, porque cada caso (desconexion, muerte, cancelacion en
// vivo, fin de intento) necesita algo distinto.
stock Cronos_LimpiarEstado(playerid)
{
    if (g_CronosTimerCuenta[playerid] != -1)
    {
        KillTimer(g_CronosTimerCuenta[playerid]);
        g_CronosTimerCuenta[playerid] = -1;
    }
    if (g_CronosTimerCorriendo[playerid] != -1)
    {
        KillTimer(g_CronosTimerCorriendo[playerid]);
        g_CronosTimerCorriendo[playerid] = -1;
    }
    PlayerTextDrawHide(playerid, g_CronosTD[playerid][CRT_TITULO]);
    PlayerTextDrawHide(playerid, g_CronosTD[playerid][CRT_SUBTITULO]);
    PlayerTextDrawHide(playerid, g_CronosTDCron[playerid]);
    DisablePlayerCheckpoint(playerid);
    g_CronosEstado[playerid] = CR_NINGUNO;
    return 1;
}

stock Cronos_OnDesconectar(playerid)
{
    Cronos_LimpiarEstado(playerid);
    return 1;
}

// Cancelacion EN VIVO (cambio de vehiculo, sales de la zona valida...) --
// a diferencia de morir/desconectar, aqui SI hay que devolver el control
// si estaba congelado por la cuenta atras, y SI hay que avisar.
stock Cronos_Cancelar(playerid, const mensaje[])
{
    new bool:estabaCongelado = (g_CronosEstado[playerid] == CR_CUENTA_ATRAS);
    Cronos_LimpiarEstado(playerid);
    if (estabaCongelado) TogglePlayerControllable(playerid, true);
    if (mensaje[0] != EOS) Error(playerid, mensaje);
    return 1;
}

// ---------------------------------------------------------------------
//  MARCADORES DE SALIDA -- pickup GTA III 1248 + texto flotante AZUL fijo
//  (item 2 del encargo, literal: "CRONO GAMEROL / /crono para
//  participar" -- el nombre concreto de la pista NO va en el letrero,
//  se descubre con /vercronos, igual que /comer no rotula cada plato en
//  el letrero de la calle).
// ---------------------------------------------------------------------
stock Cronos_CrearMarcador(indice)
{
    g_Cronos[indice][crPickup] = CreateDynamicPickup(CRONOS_PICKUP_MODELO, 23,
        g_Cronos[indice][crSalidaX], g_Cronos[indice][crSalidaY], g_Cronos[indice][crSalidaZ], 0, 0, -1, 100.0);

    g_Cronos[indice][crEtiqueta] = CreateDynamic3DTextLabel(
        "CRONO GAMEROL\n{E8B33A}/crono para participar",
        COLOR_AZUL, g_Cronos[indice][crSalidaX], g_Cronos[indice][crSalidaY], g_Cronos[indice][crSalidaZ] + 0.7, 14.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);
    return 1;
}

stock Cronos_DestruirMarcador(indice)
{
    DestroyDynamicPickup(g_Cronos[indice][crPickup]);
    DestroyDynamic3DTextLabel(g_Cronos[indice][crEtiqueta]);
    return 1;
}

stock Cronos_IndiceMasCercano(playerid)
{
    for (new i = 0; i < g_CronosTotal; i++)
    {
        if (g_Cronos[i][crEliminado]) continue;
        if (EnPunto(playerid, CRONOS_RADIO_SALIDA, g_Cronos[i][crSalidaX], g_Cronos[i][crSalidaY], g_Cronos[i][crSalidaZ]))
            return i;
    }
    return -1;
}

stock Cronos_BuscarPorNombre(const nombre[])
{
    for (new i = 0; i < g_CronosTotal; i++)
    {
        if (g_Cronos[i][crEliminado]) continue;
        if (!strcmp(g_Cronos[i][crNombre], nombre, true)) return i;
    }
    return -1;
}

// La i-esima crono ACTIVA (no eliminada) en el mismo orden en que
// /vercronos las lista -- recalculado sobre la marcha en vez de guardar
// un mapeo por jugador: el orden es siempre el mismo (solo se anaden
// cronos nuevas al final, nunca se reordenan ni se borran una vez
// activas), asi que es seguro repetir el mismo filtro en el dialogo.
stock Cronos_IndicePorPosicionActiva(posicion)
{
    new contador = 0;
    for (new i = 0; i < g_CronosTotal; i++)
    {
        if (g_Cronos[i][crEliminado] || !g_Cronos[i][crActivo]) continue;
        if (contador == posicion) return i;
        contador++;
    }
    return -1;
}

stock Cronos_ContarActivos()
{
    new contador = 0;
    for (new i = 0; i < g_CronosTotal; i++)
    {
        if (g_Cronos[i][crEliminado] || !g_Cronos[i][crActivo]) continue;
        contador++;
    }
    return contador;
}

stock Cronos_ObtenerParaGPS(posicion, &Float:x, &Float:y, &Float:z, nombre[], len)
{
    new indice = Cronos_IndicePorPosicionActiva(posicion);
    if (indice == -1) return 0;
    x = g_Cronos[indice][crSalidaX];
    y = g_Cronos[indice][crSalidaY];
    z = g_Cronos[indice][crSalidaZ];
    format(nombre, len, "%s", g_Cronos[indice][crNombre]);
    return 1;
}

// ---------------------------------------------------------------------
//  ARRANQUE -- carga todas las cronos guardadas y planta sus marcadores.
// ---------------------------------------------------------------------
stock Cronos_Iniciar()
{
    g_CronosTotal = 0;
    mysql_pquery(g_BD, "SELECT * FROM cronos", "Cronos_AlCargar");
    return 1;
}

forward Cronos_AlCargar();
public Cronos_AlCargar()
{
    new filas;
    cache_get_row_count(filas);

    for (new i = 0; i < filas && i < CRONOS_MAX; i++)
    {
        new indice = g_CronosTotal++;
        cache_get_value_name_int(i, "id", g_Cronos[indice][crId]);
        cache_get_value_name(i, "nombre", g_Cronos[indice][crNombre], 48);
        cache_get_value_name_float(i, "salida_x", g_Cronos[indice][crSalidaX]);
        cache_get_value_name_float(i, "salida_y", g_Cronos[indice][crSalidaY]);
        cache_get_value_name_float(i, "salida_z", g_Cronos[indice][crSalidaZ]);
        cache_get_value_name_float(i, "salida_angulo", g_Cronos[indice][crSalidaAngulo]);
        cache_get_value_name_float(i, "meta_x", g_Cronos[indice][crMetaX]);
        cache_get_value_name_float(i, "meta_y", g_Cronos[indice][crMetaY]);
        cache_get_value_name_float(i, "meta_z", g_Cronos[indice][crMetaZ]);

        new activo;
        cache_get_value_name_int(i, "activo", activo);
        g_Cronos[indice][crActivo]    = bool:activo;
        g_Cronos[indice][crEliminado] = false;

        Cronos_CrearMarcador(indice);
    }
    printf("  [CRONOS] %d cronos cargadas desde la base de datos.", filas);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMINISTRACION -- /crearcrono, /crearmeta, /borrarcrono. Nivel unico
//  CRONOS_ADMIN_NIVEL (01_configuracion.inc), igual de simple que el
//  resto de comandos de creacion de contenido del proyecto (/creardropeo,
//  /crearcancela...).
// ---------------------------------------------------------------------
CMD:crearcrono(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (SinPermiso(playerid, CRONOS_ADMIN_NIVEL)) return 1;

    new nombre[48];
    if (sscanf(params, "s[48]", nombre) || !strlen(nombre))
    {
        Error(playerid, "Uso: /crearcrono [nombre]");
        return 1;
    }
    if (Cronos_BuscarPorNombre(nombre) != -1)
    {
        Error(playerid, "Ya existe una crono con ese nombre.");
        return 1;
    }
    if (g_CronosTotal >= CRONOS_MAX)
    {
        Error(playerid, "Limite de cronos en memoria alcanzado.");
        return 1;
    }
    if (g_CronosEditando[playerid] != -1)
    {
        Error(playerid, "Ya tienes una crono en edicion. Usa /crearmeta o /borrarcrono primero.");
        return 1;
    }

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    new indice = g_CronosTotal++;
    g_Cronos[indice][crId] = 0; // lo rellena Cronos_AlCrear en cuanto vuelva el INSERT
    format(g_Cronos[indice][crNombre], 48, "%s", nombre);
    g_Cronos[indice][crSalidaX]      = x;
    g_Cronos[indice][crSalidaY]      = y;
    g_Cronos[indice][crSalidaZ]      = z;
    g_Cronos[indice][crSalidaAngulo] = a;
    g_Cronos[indice][crMetaX]        = 0.0;
    g_Cronos[indice][crMetaY]        = 0.0;
    g_Cronos[indice][crMetaZ]        = 0.0;
    g_Cronos[indice][crActivo]       = false;
    g_Cronos[indice][crEliminado]    = false;

    new nombreAdmin[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombreAdmin, sizeof nombreAdmin);

    new consulta[256];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO cronos (nombre, salida_x, salida_y, salida_z, salida_angulo, creado_por) VALUES ('%e', %f, %f, %f, %f, '%e')",
        nombre, x, y, z, a, nombreAdmin);
    mysql_pquery(g_BD, consulta, "Cronos_AlCrear", "i", indice);

    Cronos_CrearMarcador(indice);
    g_CronosEditando[playerid] = indice;

    Exito(playerid, "Crono creada. Ve al punto de meta y usa /crearmeta para activarla.");
    return 1;
}

forward Cronos_AlCrear(indice);
public Cronos_AlCrear(indice)
{
    if (indice >= 0 && indice < CRONOS_MAX) g_Cronos[indice][crId] = cache_insert_id();
    return 1;
}

CMD:crearmeta(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    if (SinPermiso(playerid, CRONOS_ADMIN_NIVEL)) return 1;

    new indice = g_CronosEditando[playerid];
    if (indice == -1)
    {
        Error(playerid, "No tienes ninguna crono en edicion. Usa /crearcrono primero.");
        return 1;
    }

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    g_Cronos[indice][crMetaX] = x;
    g_Cronos[indice][crMetaY] = y;
    g_Cronos[indice][crMetaZ] = z;
    g_Cronos[indice][crActivo] = true;

    // Por nombre (UNIQUE), no por id -- el id real puede tardar unos ms
    // en volver de Cronos_AlCrear tras /crearcrono; el nombre ya se
    // conoce en el momento, sin depender de ese viaje de ida y vuelta.
    new consulta[220];
    mysql_format(g_BD, consulta, sizeof consulta,
        "UPDATE cronos SET meta_x = %f, meta_y = %f, meta_z = %f, activo = 1 WHERE nombre = '%e'",
        x, y, z, g_Cronos[indice][crNombre]);
    mysql_pquery(g_BD, consulta);

    g_CronosEditando[playerid] = -1;

    format(g_Buffer, sizeof g_Buffer, "Crono \"%s\" activada. Ya se puede jugar con /crono.", g_Cronos[indice][crNombre]);
    Exito(playerid, g_Buffer);
    return 1;
}

// Solo borra la crono que el propio admin tiene EN EDICION ahora mismo
// (item 14: "creada por error", antes de /crearmeta) -- a proposito no
// admite borrar una crono ya publicada por nombre/proximidad, para que
// un fallo de escritura nunca pueda tirar abajo una pista ya en uso.
CMD:borrarcrono(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    if (SinPermiso(playerid, CRONOS_ADMIN_NIVEL)) return 1;

    new indice = g_CronosEditando[playerid];
    if (indice == -1)
    {
        Error(playerid, "Solo puedes borrar la crono que tienes en edicion (tras /crearcrono, antes de /crearmeta).");
        return 1;
    }

    new consulta[150];
    mysql_format(g_BD, consulta, sizeof consulta, "DELETE FROM cronos WHERE nombre = '%e'", g_Cronos[indice][crNombre]);
    mysql_pquery(g_BD, consulta);

    Cronos_DestruirMarcador(indice);
    g_Cronos[indice][crEliminado] = true;
    g_CronosEditando[playerid] = -1;

    Exito(playerid, "Crono borrada.");
    return 1;
}

// ---------------------------------------------------------------------
//  /crono -- arranca un intento. Chat minimo (item 15): todo el feedback
//  vive en los textdraws, nunca en el chat salvo error.
// ---------------------------------------------------------------------
CMD:crono(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (g_CronosEstado[playerid] != CR_NINGUNO)
    {
        Error(playerid, "Ya estas en una crono.");
        return 1;
    }
    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante de un vehiculo.");
        return 1;
    }

    new indice = Cronos_IndiceMasCercano(playerid);
    if (indice == -1)
    {
        Error(playerid, "Tienes que estar junto a un punto de salida de Cronos.");
        return 1;
    }
    if (!g_Cronos[indice][crActivo])
    {
        Error(playerid, "Esta crono todavia no esta lista para jugarse.");
        return 1;
    }

    g_CronosEstado[playerid]   = CR_CUENTA_ATRAS;
    g_CronosIndice[playerid]   = indice;
    g_CronosVehiculo[playerid] = GetPlayerVehicleID(playerid);
    g_CronosRestante[playerid] = CRONOS_CUENTA_ATRAS_SEGUNDOS;

    TogglePlayerControllable(playerid, false);

    PlayerTextDrawSetString(playerid, g_CronosTD[playerid][CRT_TITULO], "LLEGA AL PUNTO ROJO");
    new texto[4];
    format(texto, sizeof texto, "%d", g_CronosRestante[playerid]);
    PlayerTextDrawSetString(playerid, g_CronosTD[playerid][CRT_SUBTITULO], texto);
    PlayerTextDrawShow(playerid, g_CronosTD[playerid][CRT_TITULO]);
    PlayerTextDrawShow(playerid, g_CronosTD[playerid][CRT_SUBTITULO]);

    g_CronosTimerCuenta[playerid] = SetTimerEx("Cronos_TicCuentaAtras", 1000, true, "i", playerid);
    return 1;
}

// Timer de 1000ms, mismo patron que Dropeo_PuenteTic (22_dropeo.inc):
// decrementa y comprueba. Al llegar a 0 dispara TODO lo que el encargo
// pide "al finalizar la cuenta atras" a la vez: "¡YA!", arranque exacto
// del cronometro, checkpoint de meta y liberar al jugador (item 4).
forward Cronos_TicCuentaAtras(playerid);
public Cronos_TicCuentaAtras(playerid)
{
    if (!IsPlayerConnected(playerid) || g_CronosEstado[playerid] != CR_CUENTA_ATRAS) return 0;

    g_CronosRestante[playerid]--;
    if (g_CronosRestante[playerid] > 0)
    {
        new texto[4];
        format(texto, sizeof texto, "%d", g_CronosRestante[playerid]);
        PlayerTextDrawSetString(playerid, g_CronosTD[playerid][CRT_SUBTITULO], texto);
        return 1;
    }

    KillTimer(g_CronosTimerCuenta[playerid]);
    g_CronosTimerCuenta[playerid] = -1;

    PlayerTextDrawSetString(playerid, g_CronosTD[playerid][CRT_SUBTITULO], "¡YA!");
    SetTimerEx("Cronos_OcultarAvisoSuperior", CRONOS_YA_DURACION_MS, false, "d", playerid);

    TogglePlayerControllable(playerid, true);
    new indice = g_CronosIndice[playerid];
    SetPlayerCheckpoint(playerid, g_Cronos[indice][crMetaX], g_Cronos[indice][crMetaY], g_Cronos[indice][crMetaZ], CRONOS_RADIO_META);

    g_CronosEstado[playerid] = CR_CORRIENDO;
    g_CronosInicio[playerid] = GetTickCount();

    PlayerTextDrawSetString(playerid, g_CronosTDCron[playerid], "CRONO 00:00.000");
    PlayerTextDrawShow(playerid, g_CronosTDCron[playerid]);
    g_CronosTimerCorriendo[playerid] = SetTimerEx("Cronos_TicCorriendo", CRONOS_TICK_CORRIENDO_MS, true, "i", playerid);
    return 1;
}

// Timer de 100ms mientras corres: refresca el cronometro, vigila
// vehiculo/zona validos, el limite de 5 minutos, y detecta la llegada a
// meta por distancia (nunca por OnPlayerEnterCheckpoint, ver cabecera).
forward Cronos_TicCorriendo(playerid);
public Cronos_TicCorriendo(playerid)
{
    if (!IsPlayerConnected(playerid) || g_CronosEstado[playerid] != CR_CORRIENDO) return 0;

    if (GetPlayerVehicleID(playerid) != g_CronosVehiculo[playerid] || GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Cronos_Cancelar(playerid, "Cambiaste de vehiculo o te bajaste. Crono cancelada.");
        return 0;
    }
    if (GetPlayerInterior(playerid) != 0 || GetPlayerVirtualWorld(playerid) != MUNDO_NORMAL)
    {
        Cronos_Cancelar(playerid, "Saliste de la zona valida. Crono cancelada.");
        return 0;
    }

    new elapsed = GetTickCount() - g_CronosInicio[playerid];
    if (elapsed >= CRONOS_LIMITE_MS)
    {
        Cronos_Timeout(playerid);
        return 0;
    }

    new texto[24], tiempoTexto[16];
    Cronos_FormatearTiempo(elapsed, tiempoTexto, sizeof tiempoTexto);
    format(texto, sizeof texto, "CRONO %s", tiempoTexto);
    PlayerTextDrawSetString(playerid, g_CronosTDCron[playerid], texto);

    new indice = g_CronosIndice[playerid];
    if (EnPunto(playerid, CRONOS_RADIO_META, g_Cronos[indice][crMetaX], g_Cronos[indice][crMetaY], g_Cronos[indice][crMetaZ]))
    {
        Cronos_Finalizar(playerid, elapsed);
        return 0;
    }
    return 1;
}

stock Cronos_Timeout(playerid)
{
    Cronos_LimpiarEstado(playerid);
    PlayerTextDrawSetString(playerid, g_CronosTD[playerid][CRT_TITULO], "TIEMPO AGOTADO");
    PlayerTextDrawShow(playerid, g_CronosTD[playerid][CRT_TITULO]);
    SetTimerEx("Cronos_OcultarAvisoSuperior", CRONOS_AVISO_FINAL_MS, false, "d", playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  FIN DE INTENTO -- llegada valida a meta. Consulta agregada UNICA
//  (record global de la crono + mejor marca propia, en la misma fila)
//  para decidir con una sola ida y vuelta si es record mundial, mejora
//  personal, primera marca de la crono, o ninguna mejora.
// ---------------------------------------------------------------------
stock Cronos_Finalizar(playerid, elapsedMs)
{
    if (elapsedMs <= 0) elapsedMs = 1; // defensivo (item 16: "tiempo invalido/cero"), no deberia poder pasar

    new indice = g_CronosIndice[playerid];
    Cronos_LimpiarEstado(playerid);

    new modeloVeh = GetVehicleModel(g_CronosVehiculo[playerid]);
    new cuentaId  = Jugador[playerid][jID];

    new consulta[280];
    mysql_format(g_BD, consulta, sizeof consulta,
        "SELECT COALESCE(MIN(tiempo_ms), -1) AS record_global, \
                COALESCE(MAX(CASE WHEN cuenta_id = %d THEN tiempo_ms END), -1) AS mi_mejor \
         FROM cronos_ranking WHERE crono_id = %d",
        cuentaId, g_Cronos[indice][crId]);
    mysql_pquery(g_BD, consulta, "Cronos_AlComprobarRecord", "ddddd", playerid, indice, elapsedMs, modeloVeh, cuentaId);
    return 1;
}

forward Cronos_AlComprobarRecord(playerid, indice, elapsedMs, modeloVeh, cuentaId);
public Cronos_AlComprobarRecord(playerid, indice, elapsedMs, modeloVeh, cuentaId)
{
    new recordGlobalAntes, miMejorAntes;
    cache_get_value_name_int(0, "record_global", recordGlobalAntes);
    cache_get_value_name_int(0, "mi_mejor", miMejorAntes);

    new bool:esPrimero         = (recordGlobalAntes == -1);
    new bool:esMundial         = (!esPrimero && elapsedMs < recordGlobalAntes);
    new bool:primeraVezJugador = (miMejorAntes == -1);
    new bool:mejora            = (!esPrimero && !esMundial && (primeraVezJugador || elapsedMs < miMejorAntes));

    // Guardar SOLO si hay mejora real -- el propio UPDATE ademas lleva
    // su IF(...) como segunda red de seguridad (item 16 del encargo).
    if (esPrimero || esMundial || mejora)
    {
        new nombre[MAX_PLAYER_NAME + 1];
        if (IsPlayerConnected(playerid)) NombreRP(playerid, nombre, sizeof nombre);
        else nombre[0] = EOS;

        // La plantilla sola (antes de sustituir valores, con los 4
        // IF(...) del upsert) ya mide 532 caracteres -- un buffer de 340
        // (bug real encontrado 14/09/2026 tras la primera prueba de
        // Vlad: mysql_format fallaba en silencio, "Query was empty", y
        // NINGUN tiempo se guardaba nunca) se queda corto. 700 deja
        // margen de sobra para crono_id/cuenta_id/tiempo_ms/modelo y un
        // nombre de hasta MAX_PLAYER_NAME escapado.
        new consultaGuardar[700];
        mysql_format(g_BD, consultaGuardar, sizeof consultaGuardar,
            "INSERT INTO cronos_ranking (crono_id, cuenta_id, nombre, tiempo_ms, vehiculo_modelo) \
             VALUES (%d, %d, '%e', %d, %d) \
             ON DUPLICATE KEY UPDATE \
                tiempo_ms = IF(VALUES(tiempo_ms) < tiempo_ms, VALUES(tiempo_ms), tiempo_ms), \
                nombre = IF(VALUES(tiempo_ms) < tiempo_ms, VALUES(nombre), nombre), \
                vehiculo_modelo = IF(VALUES(tiempo_ms) < tiempo_ms, VALUES(vehiculo_modelo), vehiculo_modelo), \
                fecha = IF(VALUES(tiempo_ms) < tiempo_ms, VALUES(fecha), fecha)",
            g_Cronos[indice][crId], cuentaId, nombre, elapsedMs, modeloVeh);
        mysql_pquery(g_BD, consultaGuardar);
    }

    if (!IsPlayerConnected(playerid)) return 1; // se fue mientras se comprobaba -- el tiempo ya quedo guardado arriba

    if (esMundial)
    {
        Cronos_PremiarRecordMundial(playerid, indice, elapsedMs);
        Cronos_MostrarResultado(playerid, "RECORD MUNDIAL", elapsedMs, 1);
        return 1;
    }
    if (esPrimero)
    {
        Cronos_MostrarResultado(playerid, "CRONO FINALIZADA", elapsedMs, 1);
        return 1;
    }

    // Mejora personal (real o primera marca de ESTE jugador en una crono
    // que ya tenia gente) o ninguna mejora: en los dos casos hace falta
    // el puesto real, consulta aparte. Se excluye siempre la propia
    // fila (cuenta_id != %d) para que no importe si el UPDATE de arriba
    // ya se aplico o no todavia.
    new tiempoParaPuesto = mejora ? elapsedMs : miMejorAntes;
    new esRecordPersonal = (mejora && !primeraVezJugador) ? 1 : 0;

    new consultaPuesto[220];
    mysql_format(g_BD, consultaPuesto, sizeof consultaPuesto,
        "SELECT COUNT(*) + 1 AS puesto FROM cronos_ranking WHERE crono_id = %d AND cuenta_id != %d AND tiempo_ms < %d",
        g_Cronos[indice][crId], cuentaId, tiempoParaPuesto);
    mysql_pquery(g_BD, consultaPuesto, "Cronos_AlSaberPuesto", "ddd", playerid, tiempoParaPuesto, esRecordPersonal);
    return 1;
}

forward Cronos_AlSaberPuesto(playerid, tiempoMostrado, esRecordPersonal);
public Cronos_AlSaberPuesto(playerid, tiempoMostrado, esRecordPersonal)
{
    if (!IsPlayerConnected(playerid)) return 1;

    new puesto = 1;
    cache_get_value_name_int(0, "puesto", puesto);

    Cronos_MostrarResultado(playerid, esRecordPersonal ? "NUEVO RECORD PERSONAL" : "CRONO FINALIZADA", tiempoMostrado, puesto);
    return 1;
}

stock Cronos_MostrarResultado(playerid, const titulo[], tiempoMs, puesto)
{
    new tiempoTexto[16];
    Cronos_FormatearTiempo(tiempoMs, tiempoTexto, sizeof tiempoTexto);

    new subtitulo[40];
    format(subtitulo, sizeof subtitulo, "PUESTO #%d  %s", puesto, tiempoTexto);

    PlayerTextDrawSetString(playerid, g_CronosTD[playerid][CRT_TITULO], titulo);
    PlayerTextDrawSetString(playerid, g_CronosTD[playerid][CRT_SUBTITULO], subtitulo);
    PlayerTextDrawShow(playerid, g_CronosTD[playerid][CRT_TITULO]);
    PlayerTextDrawShow(playerid, g_CronosTD[playerid][CRT_SUBTITULO]);
    SetTimerEx("Cronos_OcultarAvisoSuperior", CRONOS_AVISO_FINAL_MS, false, "d", playerid);
    return 1;
}

// Premio de RECORD MUNDIAL real (item 11 del encargo): dinero,
// materiales, marihuana con las funciones/patrones economicos ya
// existentes (Item_Buscar + Inv_DarItem, igual que Federico), reparacion
// GRATIS con la funcion extraida de /arreglarcoche (53_arreglarcoche.inc)
// y combustible al 100% con el mismo patron de asignacion directa que
// usa el resto del proyecto (13_vehiculos.inc).
stock Cronos_PremiarRecordMundial(playerid, indice, tiempoMs)
{
    DarDinero(playerid, CRONOS_PREMIO_DINERO);
    Inv_DarItem(playerid, Item_Buscar("Materiales"), CRONOS_PREMIO_MATERIALES, 0);
    Inv_DarItem(playerid, Item_Buscar("Marihuana"), CRONOS_PREMIO_MARIHUANA, 0);

    new vIndice = Veh_BuscarPorObjeto(g_CronosVehiculo[playerid]);
    if (VehiculoValido(vIndice))
    {
        Arreglarcoche_Reparar(g_CronosVehiculo[playerid], vIndice);
        g_Vehiculos[vIndice][veCombustible] = 100.0;
        Veh_Guardar(vIndice);
    }

    new nombre[MAX_PLAYER_NAME + 1], tiempoTexto[16];
    NombreRP(playerid, nombre, sizeof nombre);
    Cronos_FormatearTiempo(tiempoMs, tiempoTexto, sizeof tiempoTexto);
    format(g_Buffer, sizeof g_Buffer, "CRONOS_RECORD_MUNDIAL jugador=%s crono_id=%d tiempo=%s timestamp=%d",
        nombre, g_Cronos[indice][crId], tiempoTexto, gettime());
    EscribirLog("cronos", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  /vercronos -- mismo patron de dialogo que /comer (42_comida.inc):
//  TABLIST_HEADERS con la lista de cronos activas; al elegir una, MSGBOX
//  con su top 10 (mismo patron que CMD:rankingpesca, 51_pesca.inc).
// ---------------------------------------------------------------------
CMD:vercronos(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new lista[512], linea[64], contador = 0;
    format(lista, sizeof lista, "Crono\n");
    for (new i = 0; i < g_CronosTotal; i++)
    {
        if (g_Cronos[i][crEliminado] || !g_Cronos[i][crActivo]) continue;
        format(linea, sizeof linea, "%s\n", g_Cronos[i][crNombre]);
        strcat(lista, linea, sizeof lista);
        contador++;
    }

    if (contador == 0)
    {
        Error(playerid, "Todavia no hay ninguna crono activa.");
        return 1;
    }

    ShowPlayerDialog(playerid, DIALOGO_CRONOS_LISTA, DIALOG_STYLE_TABLIST_HEADERS, "{E8B33A}Cronos Gamerol", lista, "Ver ranking", "Cerrar");
    return 1;
}

stock Cronos_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext

    if (dialogid == DIALOGO_CRONOS_LISTA)
    {
        if (!response) return 1;

        new indice = Cronos_IndicePorPosicionActiva(listitem);
        if (indice == -1) return 1;

        Jugador[playerid][jTemporal] = indice; // valor auxiliar entre dialogos (02_datos.inc), mismo patron que otros modulos

        new consulta[200];
        mysql_format(g_BD, consulta, sizeof consulta,
            "SELECT nombre, tiempo_ms FROM cronos_ranking WHERE crono_id = %d ORDER BY tiempo_ms ASC LIMIT 10",
            g_Cronos[indice][crId]);
        mysql_pquery(g_BD, consulta, "Cronos_AlMostrarRanking", "i", playerid);
        return 1;
    }

    if (dialogid == DIALOGO_CRONOS_RANKING)
    {
        if (response)
        {
            new vacio[2];
            vacio[0] = 1;
            vacio[1] = EOS;
            return cmd_vercronos(playerid, vacio);
        }
        return 1;
    }

    return 0;
}

forward Cronos_AlMostrarRanking(playerid);
public Cronos_AlMostrarRanking(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);

    new lista[512];
    if (filas == 0)
    {
        format(lista, sizeof lista, "Todavia nadie ha completado esta crono.");
    }
    else
    {
        new linea[64], nombre[MAX_PLAYER_NAME + 1], tiempoMs, tiempoTexto[16];
        lista[0] = EOS;
        for (new i = 0; i < filas; i++)
        {
            cache_get_value_name(i, "nombre", nombre, sizeof nombre);
            cache_get_value_name_int(i, "tiempo_ms", tiempoMs);
            Cronos_FormatearTiempo(tiempoMs, tiempoTexto, sizeof tiempoTexto);
            format(linea, sizeof linea, "%d. %s  %s\n", i + 1, nombre, tiempoTexto);
            strcat(lista, linea, sizeof lista);
        }
    }

    new indice = Jugador[playerid][jTemporal];
    new titulo[64];
    if (indice >= 0 && indice < CRONOS_MAX) format(titulo, sizeof titulo, "{E8B33A}%s", g_Cronos[indice][crNombre]);
    else format(titulo, sizeof titulo, "{E8B33A}Crono");

    ShowPlayerDialog(playerid, DIALOGO_CRONOS_RANKING, DIALOG_STYLE_MSGBOX, titulo, lista, "Volver", "Cerrar");
    return 1;
}
