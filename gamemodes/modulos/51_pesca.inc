/* =====================================================================
 *  51_pesca.inc
 *  ---------------------------------------------------------------------
 *  GAMEROL AVENTURAS -- DLC PESCA (V1 completa), encargo de Vlad del
 *  06/09/2026. Circuito: comprar cana+barbacoa -> /pescar en uno de los
 *  puntos configurables -> capturar pez con especie+peso individual ->
 *  vender en un puesto de venta o cocinar en tu barbacoa -> comer.
 *  Ademas: estadisticas, records (por especie y mundial), rangos
 *  cosmeticos de pescador.
 *
 *  NO ES UN SISTEMA PARALELO -- reutiliza:
 *    - Inventario real (10_inventario.inc): cana, barbacoa, los 10 peces
 *      crudos y los 30 platos cocinados son filas normales de g_Items.
 *      El peso INDIVIDUAL de cada captura se guarda en SLOT_DATOS (igual
 *      patron que la municion guardada en la ranura de un arma) -- por
 *      eso las 10 especies tienen itMaxPila=1: cada pez ocupa su propia
 *      ranura, nunca se funde con otro de la misma especie aunque
 *      pesaran lo mismo.
 *    - Tienda (27_tienda.inc): cana y barbacoa se compran con /comprar,
 *      como cualquier otro articulo del catalogo.
 *    - Coordenadas configurables en caliente (mismo patron que
 *      /setbanco y /setempeños, Config_Leer/Config_Escribir sobre
 *      `servidor_config`): /setpesca [1-6] y /setventapescado [1-2],
 *      ambos en 16_administracion.inc.
 *    - Sorteo ponderado (mismo patron que Botin_TirarTramo y
 *      Pawnshop_ElegirObjetoAleatorio en 45_maletero.inc): acumular peso
 *      hasta pasarse de un numero aleatorio.
 *    - Menu de venta con lista+confirmacion (mismo patron que
 *      Empenos_MostrarMenu en 50_pawnshop.inc).
 *    - GameText grande estilo Pricedown para hitos importantes (mismo
 *      patron EXACTO que "Te estas muriendo de hambre" en
 *      44_hambre_sed.inc y el cartel de bienvenida en 47_bienvenida.inc):
 *      dos PlayerTextDraw por jugador, con temporizador para ocultarlos.
 *
 *  PERSISTENCIA (tablas nuevas, ver sql/migraciones/038_pesca.sql):
 *    - cuentas.pesca_capturas / cuentas.pesca_comidos -- se cargan y
 *      guardan igual que hambre/sed (05_basedatos.inc).
 *    - pesca_records (10 filas fijas, una por especie): record de peso,
 *      cuenta y nombre del dueno. El "record mundial" NO es una fila
 *      aparte -- es simplemente el maximo de estas 10 (si el pez mas
 *      grande de la historia fue una Dorada, esa Dorada es TAMBIEN el
 *      record de su propia especie, siempre).
 *    - pesca_barbacoas (una fila por barbacoa desplegada, PK cuenta_id):
 *      posicion, y el estado de cocinado con marcas de tiempo ABSOLUTAS
 *      (gettime(), no un contador relativo) -- mismo patron que
 *      `federico_estado.next_rotation_at` o `vehiculos.papeles_expiran`,
 *      para que sobreviva a un reinicio del servidor sin duplicar ni
 *      perder la comida.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  LAS 10 ESPECIES -- tabla centralizada para poder rebalancear rapido.
//  pePeso es el peso relativo DENTRO del 60% de exito (PESCA_PROBABILIDAD_EXITO,
//  01_configuracion.inc) -- las tres primeras (comunes) sencillamente
//  aparecen mas, los grandes (Atun, Pez espada) son mas raros pero NO al
//  nivel de los objetos de 100.000$ de los maleteros: encargo expreso de
//  Vlad, "variedad antes que loteria brutal".
// ---------------------------------------------------------------------
enum E_PESCA_ESPECIE
{
    peNombre[24],
    pePesoMinG,     // gramos
    pePesoMaxG,     // gramos
    pePrecioKg,     // $ por kilo
    pePeso          // peso relativo del sorteo de especie
}
new const g_PescaEspecies[][E_PESCA_ESPECIE] = {
    {"Sardina",     300,   800,   25, 2500},   // referencia de Vlad: ~25$/kg
    {"Boqueron",    200,   600,   20, 2500},
    {"Caballa",     400,  1200,   35, 2000},
    {"Trucha",      500,  2000,   60, 1500},
    {"Dorada",      800,  3000,   90, 1000},
    {"Lubina",     1000,  3500,  110,  800},
    {"Salmon",     1500,  5000,  140,  500},
    {"Pez globo",   500,  2000,  180,  350},
    {"Pez espada", 3000,  8000,  200,  200},
    {"Atun",       4000, 10000,  220,  150}
};
#define PESCA_TOTAL_ESPECIES  (sizeof g_PescaEspecies)
// Suma de pePeso de arriba: 2500+2500+2000+1500+1000+800+500+350+200+150.
// Calculado a mano (igual que PAWNSHOP_PESO_TOTAL en 45_maletero.inc):
// si se retoca una fila, actualizar este numero.
#define PESCA_ESPECIE_PESO_TOTAL 11500

#define PESCA_TOTAL_RECETAS 3
new const g_PescaRecetas[PESCA_TOTAL_RECETAS][16] = {"al limon", "al ajillo", "al carbon"};

// Resueltos UNA vez en Pesca_Iniciar() via Item_Buscar -- nunca IDs
// escritos a mano (mismo criterio que todo el resto del proyecto: el id
// real de un objeto es su posicion en g_Items, y eso puede desplazarse).
new g_PescaItemCana;
new g_PescaItemBarbacoa;
new g_PescaItemCrudo[PESCA_TOTAL_ESPECIES];
new g_PescaItemCocinado[PESCA_TOTAL_ESPECIES][PESCA_TOTAL_RECETAS];

stock Pesca_ResolverItems()
{
    g_PescaItemCana      = Item_Buscar("Cana de pescar");
    g_PescaItemBarbacoa  = Item_Buscar("Barbacoa");

    new nombreCompleto[48];
    for (new e = 0; e < PESCA_TOTAL_ESPECIES; e++)
    {
        g_PescaItemCrudo[e] = Item_Buscar(g_PescaEspecies[e][peNombre]);

        for (new r = 0; r < PESCA_TOTAL_RECETAS; r++)
        {
            format(nombreCompleto, sizeof nombreCompleto, "%s %s", g_PescaEspecies[e][peNombre], g_PescaRecetas[r]);
            g_PescaItemCocinado[e][r] = Item_Buscar(nombreCompleto);
        }
    }
    return 1;
}

// true si `item` es uno de los 10 peces crudos. Recorrido lineal sobre
// una tabla de 10 -- mismo coste que Pawnshop_IndicePorItem.
stock bool:Pesca_EsPezCrudo(item)
{
    for (new e = 0; e < PESCA_TOTAL_ESPECIES; e++)
    {
        if (g_PescaItemCrudo[e] == item) return true;
    }
    return false;
}
stock Pesca_EspecieDeCrudo(item)
{
    for (new e = 0; e < PESCA_TOTAL_ESPECIES; e++)
    {
        if (g_PescaItemCrudo[e] == item) return e;
    }
    return -1;
}

// true si `item` es uno de los 30 platos cocinados -- usado desde
// 10_inventario.inc (Inv_Usar) para desviar el consumo generico de
// ITEM_CONSUMIBLE hacia Pesca_ComerPlato().
stock bool:Pesca_EsPlatoCocinado(item)
{
    for (new e = 0; e < PESCA_TOTAL_ESPECIES; e++)
    {
        for (new r = 0; r < PESCA_TOTAL_RECETAS; r++)
        {
            if (g_PescaItemCocinado[e][r] == item) return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------
//  PUNTOS DE PESCA -- 6 posiciones configurables, NUNCA hardcodeadas en
//  logica (los #define PESCA1_X.. de 01_configuracion.inc son solo el
//  valor de PARTIDA que carga Config_Leer la primera vez). Los 6 puntos
//  estan activos (los ultimos 3, con las coordenadas de Angel Pine desde
//  el 07/09/2026) -- un punto solo queda inactivo si su X e Y llegan a
//  0,0 (p.ej. tras un /setpesca sin terminar), y entonces no crea
//  etiqueta ni permite /pescar hasta que un Gestion+ lo marque de nuevo.
//
//  PESCA_TOTAL_PUNTOS/VENTAS y los arrays g_PescaPunto*/g_PescaVenta*
//  estan declarados en 05_basedatos.inc, no aqui: /setpesca y
//  /setventapescado viven en 16_administracion.inc, que se incluye
//  ANTES que este modulo, y Pawn exige que una variable global este
//  declarada textualmente antes de su primer uso (a diferencia de las
//  funciones, que si admiten forward implicito).
// ---------------------------------------------------------------------
new STREAMER_TAG_3D_TEXT_LABEL:g_PescaPuntoEtiqueta[PESCA_TOTAL_PUNTOS] = {STREAMER_TAG_3D_TEXT_LABEL:-1, ...};

stock Pesca_CargarConfiguracion()
{
    g_PescaPuntoX[0] = Float:Config_Leer("pesca1_x", PESCA1_X);
    g_PescaPuntoY[0] = Float:Config_Leer("pesca1_y", PESCA1_Y);
    g_PescaPuntoZ[0] = Float:Config_Leer("pesca1_z", PESCA1_Z);
    g_PescaPuntoA[0] = Float:Config_Leer("pesca1_a", PESCA1_A);

    g_PescaPuntoX[1] = Float:Config_Leer("pesca2_x", PESCA2_X);
    g_PescaPuntoY[1] = Float:Config_Leer("pesca2_y", PESCA2_Y);
    g_PescaPuntoZ[1] = Float:Config_Leer("pesca2_z", PESCA2_Z);
    g_PescaPuntoA[1] = Float:Config_Leer("pesca2_a", PESCA2_A);

    g_PescaPuntoX[2] = Float:Config_Leer("pesca3_x", PESCA3_X);
    g_PescaPuntoY[2] = Float:Config_Leer("pesca3_y", PESCA3_Y);
    g_PescaPuntoZ[2] = Float:Config_Leer("pesca3_z", PESCA3_Z);
    g_PescaPuntoA[2] = Float:Config_Leer("pesca3_a", PESCA3_A);

    g_PescaPuntoX[3] = Float:Config_Leer("pesca4_x", PESCA4_X);
    g_PescaPuntoY[3] = Float:Config_Leer("pesca4_y", PESCA4_Y);
    g_PescaPuntoZ[3] = Float:Config_Leer("pesca4_z", PESCA4_Z);
    g_PescaPuntoA[3] = Float:Config_Leer("pesca4_a", PESCA4_A);

    g_PescaPuntoX[4] = Float:Config_Leer("pesca5_x", PESCA5_X);
    g_PescaPuntoY[4] = Float:Config_Leer("pesca5_y", PESCA5_Y);
    g_PescaPuntoZ[4] = Float:Config_Leer("pesca5_z", PESCA5_Z);
    g_PescaPuntoA[4] = Float:Config_Leer("pesca5_a", PESCA5_A);

    g_PescaPuntoX[5] = Float:Config_Leer("pesca6_x", PESCA6_X);
    g_PescaPuntoY[5] = Float:Config_Leer("pesca6_y", PESCA6_Y);
    g_PescaPuntoZ[5] = Float:Config_Leer("pesca6_z", PESCA6_Z);
    g_PescaPuntoA[5] = Float:Config_Leer("pesca6_a", PESCA6_A);

    for (new i = 0; i < PESCA_TOTAL_PUNTOS; i++)
    {
        g_PescaPuntoActivo[i] = (g_PescaPuntoX[i] != 0.0 || g_PescaPuntoY[i] != 0.0);
    }

    g_PescaVentaX[0] = Float:Config_Leer("ventapesca1_x", VENTAPESCA1_X);
    g_PescaVentaY[0] = Float:Config_Leer("ventapesca1_y", VENTAPESCA1_Y);
    g_PescaVentaZ[0] = Float:Config_Leer("ventapesca1_z", VENTAPESCA1_Z);

    g_PescaVentaX[1] = Float:Config_Leer("ventapesca2_x", VENTAPESCA2_X);
    g_PescaVentaY[1] = Float:Config_Leer("ventapesca2_y", VENTAPESCA2_Y);
    g_PescaVentaZ[1] = Float:Config_Leer("ventapesca2_z", VENTAPESCA2_Z);

    g_PescaVentaX[2] = Float:Config_Leer("ventapesca3_x", VENTAPESCA3_X);
    g_PescaVentaY[2] = Float:Config_Leer("ventapesca3_y", VENTAPESCA3_Y);
    g_PescaVentaZ[2] = Float:Config_Leer("ventapesca3_z", VENTAPESCA3_Z);

    print("  [PESCA] Configuracion de puntos cargada.");
    return 1;
}

// Recrea la etiqueta de UN punto de pesca (llamada al arrancar y desde
// /setpesca) -- los 3D text label no se pueden mover, solo destruir y
// volver a crear (mismo motivo que Empenos_CrearEtiquetaInterior).
stock Pesca_CrearEtiquetaPunto(indice)
{
    if (_:g_PescaPuntoEtiqueta[indice] != -1)
    {
        DestroyDynamic3DTextLabel(g_PescaPuntoEtiqueta[indice]);
        g_PescaPuntoEtiqueta[indice] = STREAMER_TAG_3D_TEXT_LABEL:-1;
    }
    if (!g_PescaPuntoActivo[indice]) return 0;

    // Texto pequeno, filosofia Transfender -- no un cartel gigante,
    // solo la indicacion minima (encargo expreso de Vlad).
    g_PescaPuntoEtiqueta[indice] = CreateDynamic3DTextLabel(
        "{4FD1C5}Punto de pesca\n{FFFFFF}/pescar", 0x4FD1C5FF,
        g_PescaPuntoX[indice], g_PescaPuntoY[indice], g_PescaPuntoZ[indice] + 0.6, 10.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 20.0);
    return 1;
}

stock Pesca_CrearMarcadorVenta(indice)
{
    g_PescaVentaPickup[indice] = CreateDynamicPickup(VENTAPESCA_PICKUP_MODELO, 23,
        g_PescaVentaX[indice], g_PescaVentaY[indice], g_PescaVentaZ[indice], 0, 0, -1, 100.0);

    g_PescaVentaEtiqueta[indice] = CreateDynamic3DTextLabel(
        "{4FD1C5}Compramos tu pescado\n{FFFFFF}/venderpescado", 0x4FD1C5FF,
        g_PescaVentaX[indice], g_PescaVentaY[indice], g_PescaVentaZ[indice] + 0.7, 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 25.0);
    return 1;
}

// Nombre de cada puesto de venta -- usado por /gps (30_gps.inc,
// GPS_MarcarVenta), que resuelve estos 3 puestos por indice en vez de un
// array "static const" porque su posicion es una variable en tiempo
// real (corregible con /setventapescado).
stock Pesca_VentaNombre(indice, destino[], tam)
{
    if (indice == 0)      format(destino, tam, "Tierra Robada Bait Shop");
    else if (indice == 1) format(destino, tam, "Angel Pine Bait Shop");
    // El 3er puesto (07/09/2026, junto a los puntos de pesca de Los
    // Santos) se quedo con este nombre generico de sobra desde que se
    // añadio -- corregido de paso al reorganizar /gps.
    else if (indice == 2) format(destino, tam, "Los Santos Bait Shop");
    else                  format(destino, tam, "Puesto de venta de pescado");
    return 1;
}

stock Pesca_Iniciar()
{
    Pesca_ResolverItems();
    Pesca_CargarConfiguracion();

    for (new i = 0; i < PESCA_TOTAL_PUNTOS; i++) Pesca_CrearEtiquetaPunto(i);
    for (new i = 0; i < PESCA_TOTAL_VENTAS; i++) Pesca_CrearMarcadorVenta(i);

    Pesca_CargarRecords();
    Pesca_CargarBarbacoas();

    print("  [PESCA] Gamerol Aventuras: DLC Pesca listo.");
    return 1;
}

// ---------------------------------------------------------------------
//  GAMETEXT GRANDE -- MISMA implementacion visual, literalmente copiada
//  del patron de 44_hambre_sed.inc / 47_bienvenida.inc (dos
//  PlayerTextDraw, fuente Pricedown, se ocultan solos con un timer).
//  Encargo EXPRESO de Vlad: capturas importantes, ascensos, records y
//  record mundial usan esta MISMA identidad visual, nunca una nueva.
// ---------------------------------------------------------------------
enum { PESCA_PTD_TITULO, PESCA_PTD_SUBTITULO, PESCA_PTD_TOTAL }
static PlayerText:g_PescaAvisoTD[MAX_PLAYERS][PESCA_PTD_TOTAL];
static bool:g_PescaAvisoVisible[MAX_PLAYERS];

stock Pesca_CrearTextdrawsAviso(playerid)
{
    g_PescaAvisoTD[playerid][PESCA_PTD_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 300.0, "-");
    PlayerTextDrawAlignment(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO], 0.48, 2.0);
    PlayerTextDrawColour(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO], 0x4FD1C5FF);
    PlayerTextDrawSetOutline(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO], true);

    g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO] = CreatePlayerTextDraw(playerid, 320.0, 325.0, "-");
    PlayerTextDrawAlignment(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO], 0.22, 1.2);
    PlayerTextDrawColour(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO], true);

    g_PescaAvisoVisible[playerid] = false;
    return 1;
}

// color: 0xRRGGBBAA para el titulo. duracionMs: cuanto queda en pantalla.
stock Pesca_MostrarAviso(playerid, const titulo[], const subtitulo[], color, duracionMs)
{
    if (!IsPlayerConnected(playerid)) return 0;

    PlayerTextDrawSetString(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO], titulo);
    PlayerTextDrawColour(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO], color);
    PlayerTextDrawSetString(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO], subtitulo);

    PlayerTextDrawShow(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO]);
    PlayerTextDrawShow(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO]);
    g_PescaAvisoVisible[playerid] = true;

    SetTimerEx("Pesca_OcultarAviso", duracionMs, false, "d", playerid);
    return 1;
}

forward Pesca_OcultarAviso(playerid);
public Pesca_OcultarAviso(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    if (!g_PescaAvisoVisible[playerid]) return 0;

    PlayerTextDrawHide(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_TITULO]);
    PlayerTextDrawHide(playerid, g_PescaAvisoTD[playerid][PESCA_PTD_SUBTITULO]);
    g_PescaAvisoVisible[playerid] = false;
    return 1;
}

// Emite el aviso grande a TODOS los conectados -- solo para el record
// mundial, el unico hito global del DLC.
stock Pesca_MostrarAvisoGlobal(const titulo[], const subtitulo[])
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || IsPlayerNPC(i) || !Jugador[i][jLogueado]) continue;
        Pesca_MostrarAviso(i, titulo, subtitulo, 0xFFD700FF, 6000);
        PlayerPlaySound(i, 1057, 0.0, 0.0, 0.0); // ping fuerte -- mismo sonido que usa 16_administracion.inc para avisos de peso
    }
    return 1;
}

// ---------------------------------------------------------------------
//  RANGOS DE PESCADOR -- solo por total de capturas (cuentas.pesca_capturas).
//  Cosmeticos: no dan ninguna ventaja, ver 01_configuracion.inc.
// ---------------------------------------------------------------------
stock Pesca_RangoDe(capturas)
{
    new rango = 0;
    for (new i = 0; i < PESCA_TOTAL_RANGOS; i++)
    {
        if (capturas >= g_PescaRangoUmbral[i]) rango = i;
    }
    return rango;
}
stock Pesca_NombreRango(rango, destino[], tam)
{
    if (rango < 0 || rango >= PESCA_TOTAL_RANGOS) rango = 0;
    format(destino, tam, "%s", g_PescaRangoNombre[rango]);
    return 1;
}

// ---------------------------------------------------------------------
//  RECORDS -- 10 filas fijas en memoria, una por especie. El record
//  mundial es simplemente el maximo de las 10 (ver cabecera del archivo).
// ---------------------------------------------------------------------
enum E_PESCA_RECORD
{
    prPesoGramos,
    prCuentaId,
    prNombre[MAX_PLAYER_NAME + 1]
}
new g_PescaRecord[PESCA_TOTAL_ESPECIES][E_PESCA_RECORD];

stock Pesca_CargarRecords()
{
    for (new e = 0; e < PESCA_TOTAL_ESPECIES; e++)
    {
        g_PescaRecord[e][prPesoGramos] = 0;
        g_PescaRecord[e][prCuentaId]   = 0;
        g_PescaRecord[e][prNombre][0]  = EOS;
    }
    mysql_pquery(g_BD, "SELECT * FROM pesca_records", "AlCargarPescaRecords");
    return 1;
}

forward AlCargarPescaRecords();
public AlCargarPescaRecords()
{
    new filas, especie;
    cache_get_row_count(filas);

    for (new i = 0; i < filas; i++)
    {
        cache_get_value_name_int(i, "especie_id", especie);
        if (especie < 0 || especie >= PESCA_TOTAL_ESPECIES) continue;

        cache_get_value_name_int(i, "peso_gramos", g_PescaRecord[especie][prPesoGramos]);
        cache_get_value_name_int(i, "cuenta_id",   g_PescaRecord[especie][prCuentaId]);
        cache_get_value_name(i, "nombre", g_PescaRecord[especie][prNombre], MAX_PLAYER_NAME + 1);
    }
    printf("  [PESCA] %d records cargados desde la base de datos.", filas);
    return 1;
}

stock Pesca_GuardarRecord(especie)
{
    new consulta[220];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO pesca_records (especie_id, peso_gramos, cuenta_id, nombre) VALUES (%d, %d, %d, '%e') \
         ON DUPLICATE KEY UPDATE peso_gramos = VALUES(peso_gramos), cuenta_id = VALUES(cuenta_id), nombre = VALUES(nombre)",
        especie, g_PescaRecord[especie][prPesoGramos], g_PescaRecord[especie][prCuentaId], g_PescaRecord[especie][prNombre]);
    mysql_pquery(g_BD, consulta);
    return 1;
}

// Devuelve el maximo actual entre las 10 especies -- el "record mundial"
// vigente ANTES de aplicar una captura nueva.
stock Pesca_RecordMundialActual()
{
    new maximo = 0;
    for (new e = 0; e < PESCA_TOTAL_ESPECIES; e++)
    {
        if (g_PescaRecord[e][prPesoGramos] > maximo) maximo = g_PescaRecord[e][prPesoGramos];
    }
    return maximo;
}

// Comprueba y aplica records tras una captura. Todo el feedback (avisos,
// GameText, sonido) vive aqui -- Pesca_TerminarAccion solo llama a esto.
stock Pesca_ComprobarRecord(playerid, especie, pesoGramos)
{
    if (pesoGramos <= g_PescaRecord[especie][prPesoGramos]) return 0;

    new mundialAntes = Pesca_RecordMundialActual();

    g_PescaRecord[especie][prPesoGramos] = pesoGramos;
    g_PescaRecord[especie][prCuentaId]   = Jugador[playerid][jID];
    NombreRP(playerid, g_PescaRecord[especie][prNombre], MAX_PLAYER_NAME + 1);
    Pesca_GuardarRecord(especie);

    new nombreJugador[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombreJugador, sizeof nombreJugador);

    if (pesoGramos > mundialAntes)
    {
        // Record MUNDIAL: global, ping fuerte.
        format(g_Buffer, sizeof g_Buffer, "NUEVO RECORD MUNDIAL");
        new subtitulo[80];
        format(subtitulo, sizeof subtitulo, "%s ha pescado un/a %s de %.2f kg",
            nombreJugador, g_PescaEspecies[especie][peNombre], float(pesoGramos) / 1000.0);
        Pesca_MostrarAvisoGlobal(g_Buffer, subtitulo);

        format(g_Buffer, sizeof g_Buffer,
            "PESCA_RECORD_MUNDIAL jugador=%s especie=%s peso_g=%d timestamp=%d",
            nombreJugador, g_PescaEspecies[especie][peNombre], pesoGramos, gettime());
        EscribirLog("pesca", g_Buffer);
    }
    else
    {
        // Record de especie: personal, ping suave.
        format(g_Buffer, sizeof g_Buffer, "NUEVO RECORD");
        new subtitulo[64];
        format(subtitulo, sizeof subtitulo, "%s: %.2f kg", g_PescaEspecies[especie][peNombre], float(pesoGramos) / 1000.0);
        Pesca_MostrarAviso(playerid, g_Buffer, subtitulo, 0x4FD1C5FF, 5000);
        PlayerPlaySound(playerid, 1052, 0.0, 0.0, 0.0);

        format(g_Buffer, sizeof g_Buffer,
            "PESCA_RECORD_ESPECIE jugador=%s especie=%s peso_g=%d timestamp=%d",
            nombreJugador, g_PescaEspecies[especie][peNombre], pesoGramos, gettime());
        EscribirLog("pesca", g_Buffer);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  /PESCAR -- accion cronometrada (10s), 60% de exito. Mismo patron de
//  "congelar + animar + resolver con timer" que /forzarmaletero
//  (45_maletero.inc), sin cuenta atras en pantalla (aqui no hace falta:
//  no es un forzado que pueda fallar por terceros, es una espera).
// ---------------------------------------------------------------------
static bool:g_PescaPescando[MAX_PLAYERS];
static g_PescaTimer[MAX_PLAYERS];

CMD:pescar(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (g_PescaPescando[playerid])
    {
        Error(playerid, "Ya estas pescando.");
        return 1;
    }
    if (IsPlayerInAnyVehicle(playerid))
    {
        Error(playerid, "Bajate del vehiculo para pescar.");
        return 1;
    }
    if (Inv_TieneItem(playerid, g_PescaItemCana) < 1)
    {
        Error(playerid, "Necesitas una Cana de pescar. Se compra en la tienda por 10.000$.");
        return 1;
    }

    new enPunto = false;
    for (new i = 0; i < PESCA_TOTAL_PUNTOS; i++)
    {
        if (!g_PescaPuntoActivo[i]) continue;
        if (EnPunto(playerid, PESCA_RADIO, g_PescaPuntoX[i], g_PescaPuntoY[i], g_PescaPuntoZ[i]))
        {
            enPunto = true;
            break;
        }
    }
    if (!enPunto)
    {
        Error(playerid, "Aqui no hay ningun punto de pesca. Usa /gps o busca el cartel \"Punto de pesca\".");
        return 1;
    }

    g_PescaPescando[playerid] = true;
    TogglePlayerControllable(playerid, false);

    // Animacion repurpose de una ya existente en el juego -- GTA:SA no
    // trae ninguna animacion de pesca de fabrica. "BASEBALL"/"bat_idle"
    // (sostener algo alargado con las dos manos, balanceo ligero) es la
    // mas cercana sin inventar nada -- SIN VERIFICAR en este servidor
    // todavia, decir si no encaja para cambiarla (mismo aviso que
    // "CAR"/"crckdoor" en 45_maletero.inc).
    Animar(playerid, "BASEBALL", "bat_idle", 4.0, true, false, false, false, 0);

    // Caña de pescar real en la mano mientras dura la accion (modelo
    // 18632, ver 01_configuracion.inc) -- se quita sola al terminar o
    // cancelarse en Pesca_TerminarAccion/Pesca_CancelarAccion.
    SetPlayerAttachedObject(playerid, PESCA_CANA_SLOT, PESCA_CANA_MODELO, PESCA_CANA_BONE,
        PESCA_CANA_OFFSET_X, PESCA_CANA_OFFSET_Y, PESCA_CANA_OFFSET_Z,
        PESCA_CANA_ROT_X, PESCA_CANA_ROT_Y, PESCA_CANA_ROT_Z,
        1.0, 1.0, 1.0);

    Info(playerid, "Echas el sedal...");

    g_PescaTimer[playerid] = SetTimerEx("Pesca_TerminarAccion", PESCA_DURACION_SEGUNDOS * 1000, false, "d", playerid);
    return 1;
}

forward Pesca_TerminarAccion(playerid);
public Pesca_TerminarAccion(playerid)
{
    if (!IsPlayerConnected(playerid) || !g_PescaPescando[playerid]) return 0;

    g_PescaPescando[playerid] = false;
    g_PescaTimer[playerid] = -1;
    TogglePlayerControllable(playerid, true);
    ClearAnimations(playerid);
    if (IsPlayerAttachedObjectSlotUsed(playerid, PESCA_CANA_SLOT)) RemovePlayerAttachedObject(playerid, PESCA_CANA_SLOT);

    if (random(100) >= PESCA_PROBABILIDAD_EXITO)
    {
        Info(playerid, "No ha picado nada. Prueba otra vez.");
        return 1;
    }

    // Sorteo ponderado de especie -- mismo patron que Botin_TirarTramo.
    new tirada = random(PESCA_ESPECIE_PESO_TOTAL), acumulado = 0, especie = 0;
    for (new e = 0; e < PESCA_TOTAL_ESPECIES; e++)
    {
        acumulado += g_PescaEspecies[e][pePeso];
        if (tirada < acumulado) { especie = e; break; }
    }

    new rango = g_PescaEspecies[especie][pePesoMaxG] - g_PescaEspecies[especie][pePesoMinG];
    new pesoGramos = g_PescaEspecies[especie][pePesoMinG] + random(rango + 1);

    new resultado = Inv_DarItem(playerid, g_PescaItemCrudo[especie], 1, pesoGramos);
    if (resultado == -1)
    {
        Error(playerid, "Has pescado algo, pero pesas demasiado para cargarlo. Suelta peso y vuelve a intentarlo.");
        return 1;
    }
    if (resultado == 0)
    {
        Error(playerid, "Has pescado algo, pero no te queda ninguna ranura libre en el inventario.");
        return 1;
    }

    new valorAprox = floatround((float(pesoGramos) / 1000.0) * g_PescaEspecies[especie][pePrecioKg]);
    new valorTexto[24];
    FormatearDinero(valorAprox, valorTexto, sizeof valorTexto);

    format(g_Buffer, sizeof g_Buffer, "Has pescado un/a %s de %.2f kg (valor aproximado %s).",
        g_PescaEspecies[especie][peNombre], float(pesoGramos) / 1000.0, valorTexto);
    Exito(playerid, g_Buffer);

    // Estadisticas y rango -- SOLO por numero de capturas (encargo expreso).
    new capturasAntes = Jugador[playerid][jPescaCapturas];
    Jugador[playerid][jPescaCapturas]++;

    new rangoAntes = Pesca_RangoDe(capturasAntes);
    new rangoAhora = Pesca_RangoDe(Jugador[playerid][jPescaCapturas]);
    if (rangoAhora != rangoAntes)
    {
        new nombreRango[32];
        Pesca_NombreRango(rangoAhora, nombreRango, sizeof nombreRango);
        Pesca_MostrarAviso(playerid, "HAS ASCENDIDO", nombreRango, 0x4FD1C5FF, 5000);
        PlayerPlaySound(playerid, 1052, 0.0, 0.0, 0.0);
    }

    Pesca_ComprobarRecord(playerid, especie, pesoGramos);

    format(g_Buffer, sizeof g_Buffer, "PESCA_CAPTURA jugador=%s especie=%s peso_g=%d valor_aprox=%d timestamp=%d",
        Jugador[playerid][jNombre], g_PescaEspecies[especie][peNombre], pesoGramos, valorAprox, gettime());
    EscribirLog("pesca", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  BARBACOA -- deployable personal (una por cuenta), con cocinado.
//  Estado calculado siempre a partir de marcas de tiempo ABSOLUTAS
//  (gettime()), nunca de un contador relativo -- sobrevive a reinicios
//  sin ambiguedad:
//
//    baCocinandoItem == 0                                -> LIBRE
//    baCocinandoItem != 0 && gettime() <  baCocinaHasta  -> COCINANDO
//    baCocinandoItem != 0 && gettime() >= baCocinaHasta
//        && gettime() < baQuemaHasta                     -> LISTO (recoger)
//    baCocinandoItem != 0 && gettime() >= baQuemaHasta   -> SE QUEMA (Pesca_Segundo lo limpia)
// ---------------------------------------------------------------------
#define MAX_BARBACOAS 100

enum E_PESCA_BARBACOA
{
    bool:baUsado,
    baCuentaId,
    Float:baX, Float:baY, Float:baZ, Float:baA,
    baInterior,
    baMundo,
    STREAMER_TAG_OBJECT:baObjeto,
    STREAMER_TAG_3D_TEXT_LABEL:baEtiqueta,
    baCocinandoItem,     // id de g_Items del plato en curso, 0 = libre
    baCocinaHasta,       // gettime() en el que termina de cocinarse
    baQuemaHasta,        // gettime() en el que se quema si no se recoge
    STREAMER_TAG_OBJECT:baObjetoComida, // representacion visual sobre la parrilla, -1 si no hay
    STREAMER_TAG_OBJECT:baObjetoHumo    // humo mientras cocina, -1 si no hay -- ver Pesca_CrearObjetoComida
}
new g_Barbacoa[MAX_BARBACOAS][E_PESCA_BARBACOA];

stock Pesca_CargarBarbacoas()
{
    mysql_pquery(g_BD, "SELECT * FROM pesca_barbacoas", "AlCargarBarbacoas");
    return 1;
}

forward AlCargarBarbacoas();
public AlCargarBarbacoas()
{
    new filas;
    cache_get_row_count(filas);

    new ahora = gettime();
    new cargadas = 0;

    for (new i = 0; i < filas && i < MAX_BARBACOAS; i++)
    {
        g_Barbacoa[i][baUsado] = true;
        cache_get_value_name_int(i, "cuenta_id", g_Barbacoa[i][baCuentaId]);
        cache_get_value_name_float(i, "x", g_Barbacoa[i][baX]);
        cache_get_value_name_float(i, "y", g_Barbacoa[i][baY]);
        cache_get_value_name_float(i, "z", g_Barbacoa[i][baZ]);
        cache_get_value_name_float(i, "a", g_Barbacoa[i][baA]);
        cache_get_value_name_int(i, "interior", g_Barbacoa[i][baInterior]);
        cache_get_value_name_int(i, "mundo", g_Barbacoa[i][baMundo]);
        cache_get_value_name_int(i, "cocinando_item", g_Barbacoa[i][baCocinandoItem]);
        cache_get_value_name_int(i, "cocina_hasta", g_Barbacoa[i][baCocinaHasta]);
        cache_get_value_name_int(i, "quema_hasta", g_Barbacoa[i][baQuemaHasta]);

        // Si el servidor estuvo apagado mas alla de la ventana de recogida,
        // se resuelve como quemado al cargar -- igual criterio que Federico
        // resolviendo una ronda vencida al arrancar.
        if (g_Barbacoa[i][baCocinandoItem] != 0 && ahora >= g_Barbacoa[i][baQuemaHasta])
        {
            g_Barbacoa[i][baCocinandoItem] = 0;
            g_Barbacoa[i][baCocinaHasta] = 0;
            g_Barbacoa[i][baQuemaHasta] = 0;
            Pesca_GuardarBarbacoa(i);
        }

        // Se hunde PESCA_BARBACOA_HUNDIDO_Z para que no quede flotando --
        // mismo problema y misma solucion que MARIA_HUNDIDO_Z en
        // 28_marihuana.inc (el pivote del modelo no esta en su base).
        g_Barbacoa[i][baObjeto] = CreateDynamicObject(PESCA_BARBACOA_MODELO,
            g_Barbacoa[i][baX], g_Barbacoa[i][baY], g_Barbacoa[i][baZ] - PESCA_BARBACOA_HUNDIDO_Z, 0.0, 0.0, g_Barbacoa[i][baA],
            g_Barbacoa[i][baMundo], g_Barbacoa[i][baInterior], -1, 100.0);

        g_Barbacoa[i][baObjetoComida] = STREAMER_TAG_OBJECT:-1;
        g_Barbacoa[i][baObjetoHumo]   = STREAMER_TAG_OBJECT:-1;
        if (g_Barbacoa[i][baCocinandoItem] != 0)
        {
            Pesca_CrearObjetoComida(i);
        }

        g_Barbacoa[i][baEtiqueta] = STREAMER_TAG_3D_TEXT_LABEL:-1;
        Pesca_ActualizarEtiquetaBarbacoa(i);

        cargadas++;
    }
    printf("  [PESCA] %d barbacoas desplegadas cargadas desde la base de datos.", cargadas);
    return 1;
}

stock Pesca_GuardarBarbacoa(indice)
{
    new consulta[300];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO pesca_barbacoas (cuenta_id, x, y, z, a, interior, mundo, cocinando_item, cocina_hasta, quema_hasta) \
         VALUES (%d, %f, %f, %f, %f, %d, %d, %d, %d, %d) \
         ON DUPLICATE KEY UPDATE x=VALUES(x), y=VALUES(y), z=VALUES(z), a=VALUES(a), interior=VALUES(interior), \
            mundo=VALUES(mundo), cocinando_item=VALUES(cocinando_item), cocina_hasta=VALUES(cocina_hasta), quema_hasta=VALUES(quema_hasta)",
        g_Barbacoa[indice][baCuentaId], g_Barbacoa[indice][baX], g_Barbacoa[indice][baY], g_Barbacoa[indice][baZ], g_Barbacoa[indice][baA],
        g_Barbacoa[indice][baInterior], g_Barbacoa[indice][baMundo],
        g_Barbacoa[indice][baCocinandoItem], g_Barbacoa[indice][baCocinaHasta], g_Barbacoa[indice][baQuemaHasta]);
    mysql_pquery(g_BD, consulta);
    return 1;
}

stock Pesca_IndiceBarbacoaDeCuenta(cuentaId)
{
    for (new i = 0; i < MAX_BARBACOAS; i++)
    {
        if (g_Barbacoa[i][baUsado] && g_Barbacoa[i][baCuentaId] == cuentaId) return i;
    }
    return -1;
}

// Representacion visual sobre la parrilla -- QUITADA el 06/09/2026 a
// peticion de Vlad ("quiero ese objeto raro fuera de ahi"): el modelo
// 2204, que se asumia sin verificar como "bandeja de comida de
// restaurante", salia como un objeto irreconocible flotando encima de
// la barbacoa. Se deja SOLO el humo (ver PESCA_MODELO_HUMO) hasta
// encontrar un modelo que de verdad parezca comida sobre unas brasas.
// #define PESCA_MODELO_COMIDA_PARRILLA 2204

// Humo -- REHECHO el 06/09/2026 tras un fallo grave: la primera version
// usaba CreateExplosion(tipo 10, radio 0.0) cada 6s pensando que un radio
// 0 no hacia daño. NO ES ASI en esta build: mataba al jugador en bucle
// mientras cocinaba. Sustituido por un OBJETO real, sin fisica ni daño
// posible -- particula de humo pura, ya usada en SA-MP para efectos
// ambientales (verificado contra dev.prineside.com). Vive y muere con
// el objeto de comida: aparece al empezar a cocinar, desaparece al
// recoger/quemarse, nunca un tick aparte que pueda repetir el error.
// Probado primero 18673 "cigarette_smoke" (salía como un objeto
// distinto, no el humo esperado); cambiado a peticion de Vlad a **18727
// "smoke50lit"**, humo mas denso.
#define PESCA_MODELO_HUMO 18727

stock Pesca_CrearObjetoComida(indice)
{
    // El objeto de comida (antes modelo 2204) esta desactivado a
    // proposito -- ver el aviso junto a PESCA_MODELO_COMIDA_PARRILLA,
    // arriba. baObjetoComida se queda siempre en -1 mientras tanto.
    if (_:g_Barbacoa[indice][baObjetoHumo] != -1) return 0;

    // Al ras del suelo, a la misma altura que el propio modelo de la
    // barbacoa (baZ - PESCA_BARBACOA_HUNDIDO_Z) -- pedido expreso de
    // Vlad el 07/09/2026, antes flotaba por encima.
    g_Barbacoa[indice][baObjetoHumo] = CreateDynamicObject(PESCA_MODELO_HUMO,
        g_Barbacoa[indice][baX], g_Barbacoa[indice][baY], g_Barbacoa[indice][baZ] - PESCA_BARBACOA_HUNDIDO_Z, 0.0, 0.0, 0.0,
        g_Barbacoa[indice][baMundo], g_Barbacoa[indice][baInterior], -1, 100.0);
    return 1;
}
stock Pesca_QuitarObjetoComida(indice)
{
    if (_:g_Barbacoa[indice][baObjetoHumo] != -1)
    {
        DestroyDynamicObject(g_Barbacoa[indice][baObjetoHumo]);
        g_Barbacoa[indice][baObjetoHumo] = STREAMER_TAG_OBJECT:-1;
    }

    if (_:g_Barbacoa[indice][baObjetoComida] == -1) return 0;
    DestroyDynamicObject(g_Barbacoa[indice][baObjetoComida]);
    g_Barbacoa[indice][baObjetoComida] = STREAMER_TAG_OBJECT:-1;
    return 1;
}

// Nombre del plato en curso -- "Especie al receta", tal como pide el
// encargo para la etiqueta ("Cocinando: Dorada al ajillo").
stock Pesca_NombrePlato(item, destino[], tam)
{
    Item_Nombre(item, destino, tam);
    return 1;
}

stock Pesca_ActualizarEtiquetaBarbacoa(indice)
{
    new nombreDueno[MAX_PLAYER_NAME + 1], texto[128];
    Pesca_NombreCuenta(g_Barbacoa[indice][baCuentaId], nombreDueno, sizeof nombreDueno);

    if (g_Barbacoa[indice][baCocinandoItem] != 0)
    {
        new nombrePlato[40];
        Pesca_NombrePlato(g_Barbacoa[indice][baCocinandoItem], nombrePlato, sizeof nombrePlato);

        if (gettime() >= g_Barbacoa[indice][baCocinaHasta])
        {
            format(texto, sizeof texto, "{E8B33A}Barbacoa de %s\n{3D9970}Listo: %s\n{FFFFFF}/recogercocinado", nombreDueno, nombrePlato);
        }
        else
        {
            // Cuenta atras -- pedido expreso de Vlad el 07/09/2026.
            // Pesca_ActualizarEtiquetaBarbacoa se llama cada segundo
            // mientras cocina (ver Pesca_Segundo), asi que el numero
            // baja solo sin timer aparte.
            new restante = g_Barbacoa[indice][baCocinaHasta] - gettime();
            if (restante < 0) restante = 0;
            format(texto, sizeof texto, "{E8B33A}Barbacoa de %s\n{FFFFFF}Cocinando: %s\n{B4B5B7}%d segundos restantes", nombreDueno, nombrePlato, restante);
        }
    }
    else
    {
        format(texto, sizeof texto, "{E8B33A}Barbacoa de %s\n{FFFFFF}Usa /cocinar", nombreDueno);
    }

    if (_:g_Barbacoa[indice][baEtiqueta] == -1)
    {
        g_Barbacoa[indice][baEtiqueta] = CreateDynamic3DTextLabel(texto, 0xE8B33AFF,
            g_Barbacoa[indice][baX], g_Barbacoa[indice][baY], g_Barbacoa[indice][baZ] + 1.0, 10.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, g_Barbacoa[indice][baInterior], g_Barbacoa[indice][baMundo], -1, 20.0);
    }
    else
    {
        UpdateDynamic3DTextLabelText(g_Barbacoa[indice][baEtiqueta], 0xE8B33AFF, texto);
    }
    return 1;
}

// Nombre de una cuenta por id -- si esta conectada usa su nombre en
// memoria (mas barato), si no cae al que quedo guardado en el record de
// la propia barbacoa la primera vez (mismo criterio: mejor un nombre
// levemente viejo que una consulta SQL sincrona en un texto flotante).
stock Pesca_NombreCuenta(cuentaId, destino[], tam)
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i) && Jugador[i][jLogueado] && Jugador[i][jID] == cuentaId)
        {
            NombreRP(i, destino, tam);
            return 1;
        }
    }
    format(destino, tam, "un pescador");
    return 1;
}

CMD:ponerbarbacoa(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (Pesca_IndiceBarbacoaDeCuenta(Jugador[playerid][jID]) != -1)
    {
        Error(playerid, "Ya tienes una barbacoa desplegada. Usa /recogerbarbacoa primero.");
        return 1;
    }
    if (Inv_TieneItem(playerid, g_PescaItemBarbacoa) < 1)
    {
        Error(playerid, "No tienes ninguna Barbacoa. Se compra en la tienda por 10.000$.");
        return 1;
    }
    if (IsPlayerInAnyVehicle(playerid))
    {
        Error(playerid, "Bajate del vehiculo para colocarla.");
        return 1;
    }

    new indice = -1;
    for (new i = 0; i < MAX_BARBACOAS; i++)
    {
        if (!g_Barbacoa[i][baUsado]) { indice = i; break; }
    }
    if (indice == -1)
    {
        Error(playerid, "Hay demasiadas barbacoas desplegadas en el servidor ahora mismo.");
        return 1;
    }

    if (!Inv_QuitarItem(playerid, g_PescaItemBarbacoa, 1))
    {
        Error(playerid, "No se ha podido sacar la barbacoa.");
        return 1;
    }

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);
    x += 1.0 * floatsin(-a, degrees);
    y += 1.0 * floatcos(-a, degrees);

    g_Barbacoa[indice][baUsado]         = true;
    g_Barbacoa[indice][baCuentaId]      = Jugador[playerid][jID];
    g_Barbacoa[indice][baX]             = x;
    g_Barbacoa[indice][baY]             = y;
    g_Barbacoa[indice][baZ]             = z;
    g_Barbacoa[indice][baA]             = a;
    g_Barbacoa[indice][baInterior]      = GetPlayerInterior(playerid);
    g_Barbacoa[indice][baMundo]         = GetPlayerVirtualWorld(playerid);
    g_Barbacoa[indice][baCocinandoItem] = 0;
    g_Barbacoa[indice][baCocinaHasta]   = 0;
    g_Barbacoa[indice][baQuemaHasta]    = 0;
    g_Barbacoa[indice][baObjetoComida]  = STREAMER_TAG_OBJECT:-1;
    g_Barbacoa[indice][baObjetoHumo]    = STREAMER_TAG_OBJECT:-1;
    g_Barbacoa[indice][baEtiqueta]      = STREAMER_TAG_3D_TEXT_LABEL:-1;

    // Se hunde PESCA_BARBACOA_HUNDIDO_Z para que no quede flotando (ver
    // el comentario junto a la constante, 01_configuracion.inc).
    g_Barbacoa[indice][baObjeto] = CreateDynamicObject(PESCA_BARBACOA_MODELO, x, y, z - PESCA_BARBACOA_HUNDIDO_Z, 0.0, 0.0, a,
        g_Barbacoa[indice][baMundo], g_Barbacoa[indice][baInterior], -1, 100.0);

    Pesca_ActualizarEtiquetaBarbacoa(indice);
    Pesca_GuardarBarbacoa(indice);

    Exito(playerid, "Has desplegado tu barbacoa. Usa /cocinar cuando tengas pescado.");
    return 1;
}

CMD:recogerbarbacoa(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new indice = Pesca_IndiceBarbacoaDeCuenta(Jugador[playerid][jID]);
    if (indice == -1) { Error(playerid, "No tienes ninguna barbacoa desplegada."); return 1; }

    if (!EnPunto(playerid, 3.0, g_Barbacoa[indice][baX], g_Barbacoa[indice][baY], g_Barbacoa[indice][baZ]))
    {
        Error(playerid, "Tienes que estar junto a tu barbacoa.");
        return 1;
    }
    if (g_Barbacoa[indice][baCocinandoItem] != 0)
    {
        Error(playerid, "Tu barbacoa esta ocupada. Recoge o espera a que termine antes de guardarla.");
        return 1;
    }

    new resultado = Inv_DarItem(playerid, g_PescaItemBarbacoa, 1, 0);
    if (resultado != 1)
    {
        Error(playerid, "No tienes sitio en el inventario para guardar la barbacoa.");
        return 1;
    }

    DestroyDynamicObject(g_Barbacoa[indice][baObjeto]);
    if (_:g_Barbacoa[indice][baEtiqueta] != -1) DestroyDynamic3DTextLabel(g_Barbacoa[indice][baEtiqueta]);
    Pesca_QuitarObjetoComida(indice);

    new consulta[80];
    format(consulta, sizeof consulta, "DELETE FROM pesca_barbacoas WHERE cuenta_id = %d", g_Barbacoa[indice][baCuentaId]);
    mysql_pquery(g_BD, consulta);

    g_Barbacoa[indice][baUsado] = false;

    Exito(playerid, "Has recogido tu barbacoa.");
    return 1;
}

// ---------------------------------------------------------------------
//  /COCINAR -- menu con SOLO los peces crudos que el jugador tiene de
//  verdad, despues 3 recetas del pez elegido. Selecciona por RANURA
//  (no por especie): cada pez pesa distinto y vive en su propia ranura
//  (itMaxPila=1), asi que hay que consumir la ranura exacta que eligio,
//  nunca "una unidad cualquiera de esa especie" -- Inv_QuitarItem no
//  distingue ranuras, por eso aqui se usa Inv_VaciarSlot directamente
//  (accesor ya publico de 10_inventario.inc, pensado para esto).
// ---------------------------------------------------------------------
static g_PescaCocinarSlot[MAX_PLAYERS][MAX_SLOTS_INVENTARIO]; // slot real por fila de la lista
static g_PescaCocinarTotal[MAX_PLAYERS];
static g_PescaCocinarSlotElegido[MAX_PLAYERS];
static g_PescaCocinarEspecieElegida[MAX_PLAYERS];

CMD:cocinar(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new indice = Pesca_IndiceBarbacoaDeCuenta(Jugador[playerid][jID]);
    if (indice == -1) { Error(playerid, "No tienes ninguna barbacoa desplegada. Usa /ponerbarbacoa."); return 1; }

    if (!EnPunto(playerid, 3.0, g_Barbacoa[indice][baX], g_Barbacoa[indice][baY], g_Barbacoa[indice][baZ]))
    {
        Error(playerid, "Tienes que estar junto a tu barbacoa.");
        return 1;
    }
    if (g_Barbacoa[indice][baCocinandoItem] != 0)
    {
        Error(playerid, "Tu barbacoa ya esta cocinando algo.");
        return 1;
    }

    new total = 0;
    new lista[1024], linea[80], nombre[32];

    format(lista, sizeof lista, "{E8B33A}Pescado\tPeso\n");
    for (new s = 0; s < MAX_SLOTS_INVENTARIO && total < MAX_SLOTS_INVENTARIO; s++)
    {
        new item = Inv_ItemEnSlot(playerid, s);
        if (!Pesca_EsPezCrudo(item)) continue;

        Item_Nombre(item, nombre, sizeof nombre);
        format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%.2f kg\n", nombre, float(Inv_DatosEnSlot(playerid, s)) / 1000.0);
        strcat(lista, linea, sizeof lista);

        g_PescaCocinarSlot[playerid][total] = s;
        total++;
    }

    if (total == 0)
    {
        Info(playerid, "No llevas ningun pescado crudo encima.");
        return 1;
    }

    g_PescaCocinarTotal[playerid] = total;
    ShowPlayerDialog(playerid, DIALOGO_PESCA_COCINAR, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8B33A}Cocinar -- elige el pescado", lista, "Elegir", "Cancelar");
    return 1;
}

CMD:recogercocinado(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new indice = Pesca_IndiceBarbacoaDeCuenta(Jugador[playerid][jID]);
    if (indice == -1) { Error(playerid, "No tienes ninguna barbacoa desplegada."); return 1; }

    if (!EnPunto(playerid, 3.0, g_Barbacoa[indice][baX], g_Barbacoa[indice][baY], g_Barbacoa[indice][baZ]))
    {
        Error(playerid, "Tienes que estar junto a tu barbacoa.");
        return 1;
    }
    if (g_Barbacoa[indice][baCocinandoItem] == 0)
    {
        Error(playerid, "Tu barbacoa no tiene nada cocinandose.");
        return 1;
    }
    if (gettime() < g_Barbacoa[indice][baCocinaHasta])
    {
        Error(playerid, "Todavia no esta listo.");
        return 1;
    }

    new item = g_Barbacoa[indice][baCocinandoItem];
    new resultado = Inv_DarItem(playerid, item, 1, 0);
    if (resultado != 1)
    {
        Error(playerid, "No tienes sitio en el inventario para el plato. Haz hueco y vuelve a intentarlo.");
        return 1;
    }

    g_Barbacoa[indice][baCocinandoItem] = 0;
    g_Barbacoa[indice][baCocinaHasta]   = 0;
    g_Barbacoa[indice][baQuemaHasta]    = 0;
    Pesca_QuitarObjetoComida(indice);
    Pesca_ActualizarEtiquetaBarbacoa(indice);
    Pesca_GuardarBarbacoa(indice);

    new nombre[40];
    Item_Nombre(item, nombre, sizeof nombre);
    format(g_Buffer, sizeof g_Buffer, "Has recogido: %s.", nombre);
    Exito(playerid, g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  COMER UN PLATO COCINADO -- llamado desde Inv_Usar (10_inventario.inc,
//  caso ITEM_CONSUMIBLE) cuando el item es uno de los 30 platos. Vida al
//  100%, escudo +10 con tope 50, acumulativo. El slot lo vacia quien
//  llama (Inv_Usar), igual que hace con cualquier otro consumible.
// ---------------------------------------------------------------------
stock Pesca_ComerPlato(playerid, item)
{
    SetPlayerHealth(playerid, 100.0);

    new Float:armadura;
    GetPlayerArmour(playerid, armadura);
    armadura += PESCA_COMER_ESCUDO;
    if (armadura > PESCA_COMER_ESCUDO_TOPE) armadura = PESCA_COMER_ESCUDO_TOPE;
    SetPlayerArmour(playerid, armadura);

    Jugador[playerid][jPescaComidos]++;

    Animar(playerid, "FOOD", "EAT_Burger", 4.1, false, false, false, false, 0);

    // Hambre y sed (44_hambre_sed.inc) -- añadido el 07/09/2026, encima
    // de la vida/escudo de arriba (esto NO lo tenian los platos de pesca
    // hasta ahora, a diferencia de las 3 franquicias de comida rapida).
    new hsTexto[64];
    HambreSed_Sumar(playerid, PESCA_COMER_HAMBRE, PESCA_COMER_SED, hsTexto, sizeof hsTexto);

    new nombre[40], mensaje[MAX_PLAYER_NAME + 1], texto[220];
    Item_Nombre(item, nombre, sizeof nombre);
    NombreRP(playerid, mensaje, sizeof mensaje);

    format(texto, sizeof texto, "* %s come %s.", mensaje, nombre);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, texto);

    format(texto, sizeof texto, "Has comido %s. Vida al 100%%, escudo %.0f%%.%s", nombre, armadura, hsTexto);
    Exito(playerid, texto);
    return 1;
}

// Estado de /venderpescado, declarado ANTES de Pesca_Dialogo (que ya lo
// usa en el caso DIALOGO_PESCA_VENDER) -- ver CMD:venderpescado mas abajo.
static g_PescaVentaListaSlot[MAX_PLAYERS][MAX_SLOTS_INVENTARIO];
static g_PescaVentaListaTotal[MAX_PLAYERS];
static g_PescaVentaSlotSel[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  DIALOGOS -- cocinar (elegir pez -> elegir receta) y vender.
// ---------------------------------------------------------------------
stock Pesca_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext
    switch (dialogid)
    {
        case DIALOGO_PESCA_COCINAR:
        {
            if (!response) return 1;
            if (listitem < 0 || listitem >= g_PescaCocinarTotal[playerid]) return 1;

            new slot = g_PescaCocinarSlot[playerid][listitem];
            new item = Inv_ItemEnSlot(playerid, slot);
            new especie = Pesca_EspecieDeCrudo(item);
            if (especie == -1) { Error(playerid, "Ese pescado ya no esta disponible."); return 1; }

            g_PescaCocinarSlotElegido[playerid]    = slot;
            g_PescaCocinarEspecieElegida[playerid] = especie;

            format(g_Buffer, sizeof g_Buffer, "%s al limon\n%s al ajillo\n%s al carbon",
                g_PescaEspecies[especie][peNombre], g_PescaEspecies[especie][peNombre], g_PescaEspecies[especie][peNombre]);
            ShowPlayerDialog(playerid, DIALOGO_PESCA_COCINAR_RECETA, DIALOG_STYLE_LIST,
                "{E8B33A}Elige la receta", g_Buffer, "Cocinar", "Atras");
            return 1;
        }

        case DIALOGO_PESCA_COCINAR_RECETA:
        {
            if (!response)
            {
                new vacio[2];
                vacio[0] = EOS;
                return cmd_cocinar(playerid, vacio);
            }
            if (listitem < 0 || listitem >= PESCA_TOTAL_RECETAS) return 1;

            new indice = Pesca_IndiceBarbacoaDeCuenta(Jugador[playerid][jID]);
            if (indice == -1) { Error(playerid, "Ya no tienes barbacoa desplegada."); return 1; }
            if (g_Barbacoa[indice][baCocinandoItem] != 0) { Error(playerid, "Tu barbacoa ya esta cocinando algo."); return 1; }

            new slot = g_PescaCocinarSlotElegido[playerid];
            new especie = g_PescaCocinarEspecieElegida[playerid];

            // Revalidacion final: el pez sigue siendo el mismo y sigue ahi
            // (pudo venderse/tirarse mientras el jugador miraba el menu).
            if (Inv_ItemEnSlot(playerid, slot) != g_PescaItemCrudo[especie])
            {
                Error(playerid, "Ese pescado ya no esta disponible.");
                return 1;
            }

            new item = g_PescaItemCocinado[especie][listitem];
            Inv_VaciarSlot(playerid, slot);

            g_Barbacoa[indice][baCocinandoItem] = item;
            g_Barbacoa[indice][baCocinaHasta]   = gettime() + PESCA_COCINAR_SEGUNDOS;
            g_Barbacoa[indice][baQuemaHasta]    = g_Barbacoa[indice][baCocinaHasta] + PESCA_QUEMAR_SEGUNDOS;

            Pesca_CrearObjetoComida(indice);
            Pesca_ActualizarEtiquetaBarbacoa(indice);
            Pesca_GuardarBarbacoa(indice);

            new nombre[40];
            Item_Nombre(item, nombre, sizeof nombre);
            format(g_Buffer, sizeof g_Buffer, "Has puesto %s en la barbacoa. Estara listo en %d segundos.", nombre, PESCA_COCINAR_SEGUNDOS);
            Exito(playerid, g_Buffer);
            return 1;
        }

        case DIALOGO_PESCA_VENDER:
        {
            if (!response) return 1;

            if (listitem == g_PescaVentaListaTotal[playerid])
            {
                // Ultima fila = "Vender todo".
                Pesca_VenderTodo(playerid);
                return 1;
            }
            if (listitem < 0 || listitem >= g_PescaVentaListaTotal[playerid]) return 1;

            g_PescaVentaSlotSel[playerid] = g_PescaVentaListaSlot[playerid][listitem];

            new slot = g_PescaVentaSlotSel[playerid];
            new item = Inv_ItemEnSlot(playerid, slot);
            if (item == 0) { Error(playerid, "Ese pescado ya no esta disponible."); Pesca_MostrarMenuVenta(playerid); return 1; }

            new especie = Pesca_EspecieDeCrudo(item);
            new pesoGramos = Inv_DatosEnSlot(playerid, slot);
            new valor = floatround((float(pesoGramos) / 1000.0) * g_PescaEspecies[especie][pePrecioKg]);

            new nombre[32], valorTexto[24];
            Item_Nombre(item, nombre, sizeof nombre);
            FormatearDinero(valor, valorTexto, sizeof valorTexto);

            format(g_Buffer, sizeof g_Buffer, "{FFFFFF}Vender:\n\n{E8B33A}%s -- %.2f kg\n\n{FFFFFF}Recibiras:\n{3D9970}%s",
                nombre, float(pesoGramos) / 1000.0, valorTexto);
            ShowPlayerDialog(playerid, DIALOGO_PESCA_VENDER_CONFIRMAR, DIALOG_STYLE_MSGBOX,
                "{E8973A}Compramos tu pescado", g_Buffer, "Confirmar", "Cancelar");
            return 1;
        }

        case DIALOGO_PESCA_VENDER_CONFIRMAR:
        {
            if (!response) return 1;

            new slot = g_PescaVentaSlotSel[playerid];
            new item = Inv_ItemEnSlot(playerid, slot);
            if (!Pesca_EsPezCrudo(item)) { Error(playerid, "Ese pescado ya no esta disponible."); return 1; }

            new especie = Pesca_EspecieDeCrudo(item);
            new pesoGramos = Inv_DatosEnSlot(playerid, slot);
            new valor = floatround((float(pesoGramos) / 1000.0) * g_PescaEspecies[especie][pePrecioKg]);

            Inv_VaciarSlot(playerid, slot);
            DarDinero(playerid, valor);

            new valorTexto[24];
            FormatearDinero(valor, valorTexto, sizeof valorTexto);
            format(g_Buffer, sizeof g_Buffer, "Has vendido tu %s por %s.", g_PescaEspecies[especie][peNombre], valorTexto);
            Exito(playerid, g_Buffer);

            format(g_Buffer, sizeof g_Buffer, "PESCA_VENTA jugador=%s especie=%s peso_g=%d valor=%d timestamp=%d",
                Jugador[playerid][jNombre], g_PescaEspecies[especie][peNombre], pesoGramos, valor, gettime());
            EscribirLog("pesca", g_Buffer);
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------
//  /VENDERPESCADO
// ---------------------------------------------------------------------
CMD:venderpescado(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new enPunto = false;
    for (new i = 0; i < PESCA_TOTAL_VENTAS; i++)
    {
        if (EnPunto(playerid, VENTAPESCA_RADIO, g_PescaVentaX[i], g_PescaVentaY[i], g_PescaVentaZ[i])) { enPunto = true; break; }
    }
    if (!enPunto)
    {
        Error(playerid, "Tienes que estar en un puesto de venta de pescado (Tierra Robada o Angel Pine).");
        return 1;
    }

    Pesca_MostrarMenuVenta(playerid);
    return 1;
}

stock Pesca_MostrarMenuVenta(playerid)
{
    new total = 0;
    new lista[1400], linea[100], nombre[32], valorTexto[24];

    format(lista, sizeof lista, "{E8B33A}Pescado\tPeso\tValor\n");
    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        new item = Inv_ItemEnSlot(playerid, s);
        if (!Pesca_EsPezCrudo(item)) continue;

        new especie = Pesca_EspecieDeCrudo(item);
        new pesoGramos = Inv_DatosEnSlot(playerid, s);
        new valor = floatround((float(pesoGramos) / 1000.0) * g_PescaEspecies[especie][pePrecioKg]);

        Item_Nombre(item, nombre, sizeof nombre);
        FormatearDinero(valor, valorTexto, sizeof valorTexto);
        format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%.2f kg\t{3D9970}%s\n", nombre, float(pesoGramos) / 1000.0, valorTexto);
        strcat(lista, linea, sizeof lista);

        g_PescaVentaListaSlot[playerid][total] = s;
        total++;
    }

    if (total == 0)
    {
        Info(playerid, "No llevas ningun pescado crudo encima.");
        return 1;
    }

    g_PescaVentaListaTotal[playerid] = total;
    if (total > 1) strcat(lista, "{4FD1C5}Vender todo\n", sizeof lista);

    ShowPlayerDialog(playerid, DIALOGO_PESCA_VENDER, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8973A}Compramos tu pescado", lista, "Elegir", "Cerrar");
    return 1;
}

stock Pesca_VenderTodo(playerid)
{
    new total = 0, piezas = 0;

    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        new item = Inv_ItemEnSlot(playerid, s);
        if (!Pesca_EsPezCrudo(item)) continue;

        new especie = Pesca_EspecieDeCrudo(item);
        new pesoGramos = Inv_DatosEnSlot(playerid, s);
        new valor = floatround((float(pesoGramos) / 1000.0) * g_PescaEspecies[especie][pePrecioKg]);

        Inv_VaciarSlot(playerid, s);
        total += valor;
        piezas++;
    }

    if (piezas == 0) { Info(playerid, "No llevas ningun pescado crudo encima."); return 1; }

    DarDinero(playerid, total);

    new valorTexto[24];
    FormatearDinero(total, valorTexto, sizeof valorTexto);
    format(g_Buffer, sizeof g_Buffer, "Has vendido %d piezas por %s.", piezas, valorTexto);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "PESCA_VENTA_TODO jugador=%s piezas=%d total=%d timestamp=%d",
        Jugador[playerid][jNombre], piezas, total, gettime());
    EscribirLog("pesca", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  /PESCAS -- estadisticas y records. Los dos rankings globales (mas
//  capturas, mas comido) piden una consulta a `cuentas`; se muestran los
//  datos propios y de especie al instante y el dialogo se abre en cuanto
//  las dos consultas responden (contador g_PescaStatsEsperando).
// ---------------------------------------------------------------------
static g_PescaStatsEsperando[MAX_PLAYERS];
static g_PescaStatsTopCapturasNombre[MAX_PLAYERS][MAX_PLAYER_NAME + 1];
static g_PescaStatsTopCapturasValor[MAX_PLAYERS];
static g_PescaStatsTopComidosNombre[MAX_PLAYERS][MAX_PLAYER_NAME + 1];
static g_PescaStatsTopComidosValor[MAX_PLAYERS];

CMD:pescas(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    g_PescaStatsEsperando[playerid] = 2;
    g_PescaStatsTopCapturasNombre[playerid][0] = EOS;
    g_PescaStatsTopComidosNombre[playerid][0] = EOS;

    mysql_pquery(g_BD, "SELECT nombre, pesca_capturas FROM cuentas ORDER BY pesca_capturas DESC LIMIT 1",
        "AlConsultarTopPescaCapturas", "d", playerid);
    mysql_pquery(g_BD, "SELECT nombre, pesca_comidos FROM cuentas ORDER BY pesca_comidos DESC LIMIT 1",
        "AlConsultarTopPescaComidos", "d", playerid);
    return 1;
}

forward AlConsultarTopPescaCapturas(playerid);
public AlConsultarTopPescaCapturas(playerid)
{
    if (IsPlayerConnected(playerid))
    {
        new filas;
        cache_get_row_count(filas);
        if (filas > 0)
        {
            cache_get_value_name(0, "nombre", g_PescaStatsTopCapturasNombre[playerid], MAX_PLAYER_NAME + 1);
            cache_get_value_name_int(0, "pesca_capturas", g_PescaStatsTopCapturasValor[playerid]);
        }
        g_PescaStatsEsperando[playerid]--;
        if (g_PescaStatsEsperando[playerid] <= 0) Pesca_MostrarStats(playerid);
    }
    return 1;
}
forward AlConsultarTopPescaComidos(playerid);
public AlConsultarTopPescaComidos(playerid)
{
    if (IsPlayerConnected(playerid))
    {
        new filas;
        cache_get_row_count(filas);
        if (filas > 0)
        {
            cache_get_value_name(0, "nombre", g_PescaStatsTopComidosNombre[playerid], MAX_PLAYER_NAME + 1);
            cache_get_value_name_int(0, "pesca_comidos", g_PescaStatsTopComidosValor[playerid]);
        }
        g_PescaStatsEsperando[playerid]--;
        if (g_PescaStatsEsperando[playerid] <= 0) Pesca_MostrarStats(playerid);
    }
    return 1;
}

stock Pesca_MostrarStats(playerid)
{
    new nombreRango[32];
    Pesca_NombreRango(Pesca_RangoDe(Jugador[playerid][jPescaCapturas]), nombreRango, sizeof nombreRango);

    new cuerpo[1800], linea[100];

    format(cuerpo, sizeof cuerpo,
        "{E8B33A}Tu rango:\t{FFFFFF}%s\n{E8B33A}Tus capturas:\t{FFFFFF}%d\n{E8B33A}Platos comidos:\t{FFFFFF}%d\n\n{E8973A}Records por especie\n",
        nombreRango, Jugador[playerid][jPescaCapturas], Jugador[playerid][jPescaComidos]);

    for (new e = 0; e < PESCA_TOTAL_ESPECIES; e++)
    {
        if (g_PescaRecord[e][prPesoGramos] > 0)
        {
            format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%.2f kg -- %s\n",
                g_PescaEspecies[e][peNombre], float(g_PescaRecord[e][prPesoGramos]) / 1000.0, g_PescaRecord[e][prNombre]);
        }
        else
        {
            format(linea, sizeof linea, "{FFFFFF}%s\t{666666}sin capturar todavia\n", g_PescaEspecies[e][peNombre]);
        }
        strcat(cuerpo, linea, sizeof cuerpo);
    }

    new mundial = Pesca_RecordMundialActual();
    if (mundial > 0)
    {
        new especieMundial = 0;
        for (new e = 0; e < PESCA_TOTAL_ESPECIES; e++) { if (g_PescaRecord[e][prPesoGramos] == mundial) especieMundial = e; }

        format(linea, sizeof linea, "\n{FFD700}Record mundial:\t%.2f kg (%s) -- %s\n",
            float(mundial) / 1000.0, g_PescaEspecies[especieMundial][peNombre], g_PescaRecord[especieMundial][prNombre]);
        strcat(cuerpo, linea, sizeof cuerpo);
    }

    if (!EstaVacio(g_PescaStatsTopCapturasNombre[playerid]))
    {
        format(linea, sizeof linea, "{FFFFFF}Mas capturas del servidor:\t%s (%d)\n",
            g_PescaStatsTopCapturasNombre[playerid], g_PescaStatsTopCapturasValor[playerid]);
        strcat(cuerpo, linea, sizeof cuerpo);
    }
    if (!EstaVacio(g_PescaStatsTopComidosNombre[playerid]))
    {
        format(linea, sizeof linea, "{FFFFFF}Mas pescado comido:\t%s (%d)\n",
            g_PescaStatsTopComidosNombre[playerid], g_PescaStatsTopComidosValor[playerid]);
        strcat(cuerpo, linea, sizeof cuerpo);
    }

    ShowPlayerDialog(playerid, DIALOGO_PESCA_STATS, DIALOG_STYLE_TABLIST_HEADERS,
        "{4FD1C5}Gamerol Aventuras -- Pesca", cuerpo, "Cerrar", "");
    return 1;
}

// ---------------------------------------------------------------------
//  /RANKINGPESCA -- podio de los 3 con mas capturas totales y el record
//  mundial. A diferencia de /pescas (estadisticas PERSONALES), aqui no
//  hay nada del jugador que lo consulta: es la tabla de competicion.
// ---------------------------------------------------------------------
CMD:rankingpesca(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    mysql_pquery(g_BD, "SELECT nombre, pesca_capturas FROM cuentas WHERE pesca_capturas > 0 ORDER BY pesca_capturas DESC LIMIT 3",
        "AlConsultarPodioPesca", "d", playerid);
    return 1;
}

forward AlConsultarPodioPesca(playerid);
public AlConsultarPodioPesca(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);

    new cuerpo[700], linea[120];
    format(cuerpo, sizeof cuerpo, "{E8973A}PODIO -- mas capturas\n");

    static const medallas[3][24] = {"{FFD700}1er lugar", "{C0C0C0}2o lugar", "{CD7F32}3er lugar"};

    if (filas == 0)
    {
        strcat(cuerpo, "{B4B5B7}Todavia nadie ha pescado nada.\n", sizeof cuerpo);
    }
    else
    {
        new nombre[MAX_PLAYER_NAME + 1], capturas;
        for (new i = 0; i < filas; i++)
        {
            cache_get_value_name(i, "nombre", nombre, sizeof nombre);
            cache_get_value_name_int(i, "pesca_capturas", capturas);

            format(linea, sizeof linea, "%s{FFFFFF}: %s (%d capturas)\n", medallas[i], nombre, capturas);
            strcat(cuerpo, linea, sizeof cuerpo);
        }
    }

    new mundial = Pesca_RecordMundialActual();
    strcat(cuerpo, "\n{E8973A}RECORD MUNDIAL\n", sizeof cuerpo);

    if (mundial > 0)
    {
        new especieMundial = 0;
        for (new e = 0; e < PESCA_TOTAL_ESPECIES; e++) { if (g_PescaRecord[e][prPesoGramos] == mundial) especieMundial = e; }

        format(linea, sizeof linea, "{FFFFFF}%.2f kg de %s -- %s\n",
            float(mundial) / 1000.0, g_PescaEspecies[especieMundial][peNombre], g_PescaRecord[especieMundial][prNombre]);
        strcat(cuerpo, linea, sizeof cuerpo);
    }
    else
    {
        strcat(cuerpo, "{B4B5B7}Todavia no hay ningun record.\n", sizeof cuerpo);
    }

    ShowPlayerDialog(playerid, DIALOGO_PESCA_RANKING, DIALOG_STYLE_MSGBOX,
        "{4FD1C5}Ranking de Pesca", cuerpo, "Cerrar", "");
    return 1;
}

// ---------------------------------------------------------------------
//  TICK -- una vez por segundo desde TicSegundo (99_callbacks.inc).
//  Solo se ocupa de la ventana de quemado: cocinar/listo se comprueban
//  al vuelo (gettime()) en cada comando, no hace falta tick para eso.
// ---------------------------------------------------------------------
stock Pesca_Segundo()
{
    new ahora = gettime();
    for (new i = 0; i < MAX_BARBACOAS; i++)
    {
        if (!g_Barbacoa[i][baUsado]) continue;
        if (g_Barbacoa[i][baCocinandoItem] == 0) continue;

        // El humo mientras cocina ya NO se hace aqui -- ver el aviso en
        // Pesca_CrearObjetoComida. (Version anterior: un CreateExplosion
        // tipo 10 cada 6s "sin daño". Era falso en esta build: explotaba
        // de verdad y mataba al jugador en bucle mientras cocinaba.
        // Retirado por completo el 06/09/2026, sustituido por el objeto
        // de humo real de arriba.)
        if (ahora < g_Barbacoa[i][baCocinaHasta])
        {
            // Cuenta atras en la etiqueta -- pedido expreso de Vlad el
            // 07/09/2026. UpdateDynamic3DTextLabelText, no destruir y
            // recrear (ya lo resuelve Pesca_ActualizarEtiquetaBarbacoa).
            Pesca_ActualizarEtiquetaBarbacoa(i);
            continue;
        }

        if (ahora < g_Barbacoa[i][baQuemaHasta]) continue;

        // Se quema: se pierde el plato, se limpia el estado y se avisa
        // al dueno si esta conectado.
        new nombre[40];
        Item_Nombre(g_Barbacoa[i][baCocinandoItem], nombre, sizeof nombre);

        g_Barbacoa[i][baCocinandoItem] = 0;
        g_Barbacoa[i][baCocinaHasta]   = 0;
        g_Barbacoa[i][baQuemaHasta]    = 0;
        Pesca_QuitarObjetoComida(i);
        Pesca_ActualizarEtiquetaBarbacoa(i);
        Pesca_GuardarBarbacoa(i);

        for (new p = 0; p < MAX_PLAYERS; p++)
        {
            if (IsPlayerConnected(p) && Jugador[p][jLogueado] && Jugador[p][jID] == g_Barbacoa[i][baCuentaId])
            {
                format(g_Buffer, sizeof g_Buffer, "Se te ha quemado: %s. No has recogido a tiempo.", nombre);
                Error(p, g_Buffer);
            }
        }
    }
    return 1;
}

// ---------------------------------------------------------------------
//  CICLO DE VIDA DEL JUGADOR
// ---------------------------------------------------------------------
forward Pesca_OnConectar(playerid);
public Pesca_OnConectar(playerid)
{
    g_PescaPescando[playerid] = false;
    g_PescaTimer[playerid] = -1;
    Pesca_CrearTextdrawsAviso(playerid);

    // Iconos de minimapa (puestos de venta y puntos de pesca) quitados
    // el 11/09/2026 -- peticion expresa de Vlad de limpiar todos los
    // iconos del mapa (la gente usa /gps). Se mantiene la etiqueta 3D.
    return 1;
}

forward Pesca_OnDesconectar(playerid);
public Pesca_OnDesconectar(playerid)
{
    Pesca_CancelarAccion(playerid);
    return 1;
}

forward Pesca_OnMuerte(playerid);
public Pesca_OnMuerte(playerid)
{
    Pesca_CancelarAccion(playerid);
    return 1;
}

// Llamada al morir o desconectar a mitad de /pescar. TogglePlayerControllable
// se pone a true igualmente al morir (aunque OnPlayerSpawn ya normaliza el
// control en cada respawn segun jCongelado, no cuesta nada dejarlo limpio
// aqui tambien -- mismo criterio defensivo que el resto del proyecto con
// "no dejar al jugador atascado").
stock Pesca_CancelarAccion(playerid)
{
    if (!g_PescaPescando[playerid]) return 0;

    g_PescaPescando[playerid] = false;
    if (g_PescaTimer[playerid] != -1)
    {
        KillTimer(g_PescaTimer[playerid]);
        g_PescaTimer[playerid] = -1;
    }

    if (IsPlayerConnected(playerid))
    {
        TogglePlayerControllable(playerid, true);
        ClearAnimations(playerid);
        if (IsPlayerAttachedObjectSlotUsed(playerid, PESCA_CANA_SLOT)) RemovePlayerAttachedObject(playerid, PESCA_CANA_SLOT);
    }
    return 1;
}
