/* =====================================================================
 *  24_casas.inc
 *  ---------------------------------------------------------------------
 *  Casas (v1): catalogo fijo, propiedad, entrada/salida y un almacen
 *  (bahul) por casa.
 *
 *  Igual que los vehiculos: se compra con dinero de mano + banco, queda
 *  a nombre de la cuenta (`casas.propietario`), y se puede cerrar con
 *  llave. La separacion entre casas que comparten el mismo interior
 *  nativo de GTA:SA se resuelve con un mundo virtual propio por casa
 *  (columna `mundo_virtual`), el mismo truco que usan los vehiculos
 *  personales con su interior reservado.
 *
 *  No hay alquiler, mercado entre jugadores ni muebles todavia: eso
 *  queda para una version futura.
 * ===================================================================== */

#define MAX_CASAS               600
#define CASA_SLOTS_ALMACEN      30
#define CASA_RADIO_PUERTA       3.0
#define CASA_REVENTA_PORCENTAJE 60      // % del precio que se devuelve al vender
#define CASA_MUNDO_BASE         6000    // mundo_virtual = CASA_MUNDO_BASE + id de la casa

// /borrarcasa (08/09/2026): busca la puerta exterior mas cercana al punto
// que hay CASA_BORRAR_DISTANCIA metros por delante del admin (misma
// formula que 10_inventario.inc para "dejar algo delante del personaje"),
// dentro de CASA_BORRAR_RADIO. No hace falta estar pegado a la puerta.
#define CASA_BORRAR_DISTANCIA   4.0
#define CASA_BORRAR_RADIO       6.0

// Historial del pickup de venta: 1273 (casita verde nativa) -> se quito
// el 05/09/2026 a favor de un icono rojo/verde plano (radar_propertyR/G,
// modelo 19329) -> ese icono se quito a su vez el 07/09/2026, peticion
// expresa de Vlad de volver al 1273 de siempre. El generico 1239 se
// probo tambien de paso en su momento (una "i" de informacion flotando)
// y no encajaba.
#define CASA_LLAVE_PICKUP       1318    // Puerta desde dentro

// ---------------------------------------------------------------------
//  INTERIORES DISPONIBLES AL CREAR UNA CASA (/crearcasa)
//  Punto de llegada dentro del interior nativo de GTA:SA. La puerta de
//  la calle la pone el admin con su propia posicion en el momento de
//  crear la casa; esto solo fija el "dentro".
// ---------------------------------------------------------------------
enum E_INTERIOR_CASA
{
    icNombre[32],
    icInteriorID,
    Float:icX, Float:icY, Float:icZ
};
static const g_InterioresCasa[][E_INTERIOR_CASA] =
{
    {"The Johnson house",          3,  2496.0549, -1695.1749, 1014.7422},
    {"Angel Pine trailer",         2,     1.1853,    -3.2387,  999.4284},
    {"Abandoned AC tower",        10,   419.8936,  2537.1155,   10.0000},
    {"Wardrobe / Changing room",  14,   256.9047,   -41.6537, 1002.0234},
    {"The Camel's Toe safehouse",  1,  2216.1282, -1076.3052, 1050.4844},
    {"Verdant Bluffs safehouse",   8,  2365.1089, -1133.0795, 1050.8750},
    {"Willowfield safehouse",     11,  2282.9766, -1140.2861, 1050.8984},
    {"Safehouse 1",                5,  2233.6919, -1112.8107, 1050.8828},
    {"Safehouse 3",                9,  2319.1272, -1023.9562, 1050.2109},
    {"Safehouse 4",               10,  2261.0977, -1137.8833, 1050.6328},
    {"Unused safe house",         12,  2323.7063, -1147.6509, 1050.7101},
    {"Random House",               2,  2236.6997, -1078.9478, 1049.0234},
    {"Burglary house 1",           3,   234.6087,  1187.8195, 1080.2578},
    {"Burglary house 2",           2,   225.5707,  1240.0643, 1082.1406},
    {"Burglary house 3",           1,   224.2880,  1289.1907, 1082.1406},
    {"Burglary house 4",           5,   239.2819,  1114.1991, 1080.9922},
    {"Burglary house 5",          15,   295.1391,  1473.3719, 1080.2578},
    {"Burglary house 6",           2,   446.6260,  1397.7380, 1084.3047},
    {"Burglary house 7",           5,   227.7559,  1114.3844, 1080.9922},
    {"Burglary house 8",           4,   261.1165,  1287.2197, 1080.2578},
    {"Burglary house 9",          10,    24.3769,  1341.1829, 1084.3750},
    {"Burglary house 10",          4,   221.6766,  1142.4962, 1082.6094},
    {"Burglary house 11",          4,  -262.1759,  1456.6158, 1084.3672},
    {"Burglary house 12",          5,    22.8610,  1404.9165, 1084.4297},
    {"Burglary house 13",          5,   140.3679,  1367.8837, 1083.8621},
    {"Burglary house 14",          6,   234.2826,  1065.2290, 1084.2101},
    {"Burglary house 15",          6,   -68.5145,  1353.8485, 1080.2109},
    {"Burglary house 16",         15,  -285.2511,  1471.1970, 1084.3750},
    {"Burglary house 17",          8,   -42.5267,  1408.2300, 1084.4297},
    {"Burglary house 18",          9,    84.9244,  1324.2983, 1083.8594}
};
#define TOTAL_INTERIORES_CASA (sizeof (g_InterioresCasa))

enum E_CASA
{
    caID,
    caNombre[49],
    caInteriorID,
    caMundoVirtual,
    Float:caExtX, Float:caExtY, Float:caExtZ, Float:caExtA,
    Float:caIntX, Float:caIntY, Float:caIntZ, Float:caIntA,
    caPrecio,
    caPropietario,
    caPropietarioNombre[MAX_PLAYER_NAME + 1],
    bool:caCerrada,
    STREAMER_TAG_PICKUP:caPickupInt,
    STREAMER_TAG_3D_TEXT_LABEL:caEtiquetaExt,
    STREAMER_TAG_3D_TEXT_LABEL:caEtiquetaInt,
    STREAMER_TAG_PICKUP:caPickupExt           // casita verde nativa (1273), de vuelta el 07/09/2026
};
new g_Casas[MAX_CASAS][E_CASA];
new g_TotalCasas = 0;

// Bahul de cada casa. item = 0 quiere decir ranura vacia.
enum { CA_ITEM, CA_CANTIDAD, CA_DATOS, CA_CAMPOS };
new g_CasaAlmacen[MAX_CASAS][CASA_SLOTS_ALMACEN][CA_CAMPOS];

// /casas por paginas, solo libres (12/09/2026, peticion expresa de Vlad
// tras subir MAX_CASAS a 600 -- ANTES listaba TODAS en un solo dialogo,
// que ni cabria ni tendria sentido mostrar casas ya compradas). La lista
// de libres se recalcula una vez al abrir /casas y se guarda por
// jugador para que "Pagina siguiente/anterior" no tenga que volver a
// recorrer g_TotalCasas en cada click, y para que la pagina no cambie de
// contenido a media navegacion si alguien compra una casa mientras tanto.
#define CASA_PAGINA_TAMANO 20
static g_CasaPaginaActual[MAX_PLAYERS];
static g_CasaListaLibres[MAX_PLAYERS][MAX_CASAS];
static g_CasaListaLibresTotal[MAX_PLAYERS];

// Estado de dialogo por jugador.
static g_CasaSeleccionada[MAX_PLAYERS];   // indice en g_Casas, flujo compra/venta/ficha
static g_CasaInvSlot[MAX_PLAYERS];        // ranura del inventario elegida al guardar
static g_CasaAlmacenSlot[MAX_PLAYERS];    // ranura del bahul elegida al sacar
static g_CasaCantidadMax[MAX_PLAYERS];
static g_CasaArmarioCorte[MAX_PLAYERS];   // en el dialogo de /armario, fila donde empiezan las del bahul

// Estado del flujo /crearcasa (admin).
static Float:g_CasaCrearExt[MAX_PLAYERS][4];  // x, y, z, angulo, capturados al escribir el comando
static g_CasaCrearInterior[MAX_PLAYERS];      // indice en g_InterioresCasa
static g_CasaCrearNombre[MAX_PLAYERS][49];

// ---------------------------------------------------------------------
//  CARGA AL ARRANCAR
// ---------------------------------------------------------------------
stock Casa_CargarTodas()
{
    mysql_pquery(g_BD, "SELECT * FROM casas ORDER BY id ASC", "AlCargarCasas");
    return 1;
}

public AlCargarCasas()
{
    new filas;
    cache_get_row_count(filas);

    g_TotalCasas = 0;

    for (new i = 0; i < filas && i < MAX_CASAS; i++)
    {
        new indice = g_TotalCasas;

        cache_get_value_name_int(i, "id",            g_Casas[indice][caID]);
        cache_get_value_name(i,     "nombre",         g_Casas[indice][caNombre], 49);
        cache_get_value_name_int(i, "interior_id",    g_Casas[indice][caInteriorID]);
        cache_get_value_name_int(i, "mundo_virtual",  g_Casas[indice][caMundoVirtual]);
        cache_get_value_name_float(i, "ext_x",        g_Casas[indice][caExtX]);
        cache_get_value_name_float(i, "ext_y",        g_Casas[indice][caExtY]);
        cache_get_value_name_float(i, "ext_z",        g_Casas[indice][caExtZ]);
        cache_get_value_name_float(i, "ext_a",        g_Casas[indice][caExtA]);
        cache_get_value_name_float(i, "int_x",        g_Casas[indice][caIntX]);
        cache_get_value_name_float(i, "int_y",        g_Casas[indice][caIntY]);
        cache_get_value_name_float(i, "int_z",        g_Casas[indice][caIntZ]);
        cache_get_value_name_float(i, "int_a",        g_Casas[indice][caIntA]);
        cache_get_value_name_int(i, "precio",         g_Casas[indice][caPrecio]);
        cache_get_value_name_int(i, "propietario",    g_Casas[indice][caPropietario]);
        cache_get_value_name(i,     "propietario_nombre", g_Casas[indice][caPropietarioNombre], MAX_PLAYER_NAME + 1);

        new temporal;
        cache_get_value_name_int(i, "cerrada", temporal);
        g_Casas[indice][caCerrada] = bool:(temporal != 0);

        Casa_CrearMarcadores(indice);
        g_TotalCasas++;
    }

    printf("  [CASAS] %d casas cargadas desde la base de datos.", g_TotalCasas);

    mysql_pquery(g_BD, "SELECT * FROM casas_almacen", "AlCargarAlmacenCasas");
    return 1;
}

public AlCargarAlmacenCasas()
{
    new filas;
    cache_get_row_count(filas);

    new cargados = 0, casaID, slot;

    for (new i = 0; i < filas; i++)
    {
        cache_get_value_name_int(i, "casa_id", casaID);
        cache_get_value_name_int(i, "slot", slot);

        new indice = Casa_IndicePorID(casaID);
        if (indice == -1 || slot < 0 || slot >= CASA_SLOTS_ALMACEN) continue;

        cache_get_value_name_int(i, "item",     g_CasaAlmacen[indice][slot][CA_ITEM]);
        cache_get_value_name_int(i, "cantidad", g_CasaAlmacen[indice][slot][CA_CANTIDAD]);
        cache_get_value_name_int(i, "datos",    g_CasaAlmacen[indice][slot][CA_DATOS]);
        cargados++;
    }

    printf("  [CASAS] %d objetos cargados en los bahules.", cargados);
    return 1;
}

stock Casa_IndicePorID(id)
{
    for (new i = 0; i < g_TotalCasas; i++)
    {
        if (g_Casas[i][caID] == id) return i;
    }
    return -1;
}

// ---------------------------------------------------------------------
//  MARCADORES
// ---------------------------------------------------------------------
stock Casa_CrearMarcadores(indice)
{
    // Vuelta a la casita verde nativa (1273) -- peticion expresa de Vlad
    // el 07/09/2026: fuera el icono rojo/verde plano (04/09-07/09,
    // modelo 19329 con textura radar_propertyR/G) y de vuelta el pickup
    // original de siempre. Nativo de GTA:SA, sin logica de color: se ve
    // igual este libre u ocupada (el estado real lo dice el cartel de
    // texto, Casa_ActualizarEtiquetaExterior).
    g_Casas[indice][caPickupExt] = CreateDynamicPickup(1273, 23,
        g_Casas[indice][caExtX], g_Casas[indice][caExtY], g_Casas[indice][caExtZ], 0, 0, -1, 100.0);

    g_Casas[indice][caEtiquetaExt] = CreateDynamic3DTextLabel(" ", COLOR_MARCA,
        g_Casas[indice][caExtX], g_Casas[indice][caExtY], g_Casas[indice][caExtZ] + 0.7, 10.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, -1, -1, 25.0);

    g_Casas[indice][caPickupInt] = CreateDynamicPickup(CASA_LLAVE_PICKUP, 23,
        g_Casas[indice][caIntX], g_Casas[indice][caIntY], g_Casas[indice][caIntZ],
        g_Casas[indice][caMundoVirtual], g_Casas[indice][caInteriorID], -1, 100.0);

    g_Casas[indice][caEtiquetaInt] = CreateDynamic3DTextLabel(
        "{FFFFFF}Escribe {E8B33A}/salircasa{FFFFFF} para volver a la calle",
        COLOR_BLANCO, g_Casas[indice][caIntX], g_Casas[indice][caIntY], g_Casas[indice][caIntZ] + 0.7, 8.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, g_Casas[indice][caMundoVirtual], g_Casas[indice][caInteriorID], -1, 25.0);

    Casa_ActualizarEtiquetaExterior(indice);
    return 1;
}

stock Casa_ActualizarEtiquetaExterior(indice)
{
    new texto[144], precio[24];

    if (g_Casas[indice][caPropietario] == 0)
    {
        FormatearDinero(g_Casas[indice][caPrecio], precio, sizeof precio);
        format(texto, sizeof texto, "%s\n{53C168}Libre - %s\n{E8B33A}/comprarcasa",
            g_Casas[indice][caNombre], precio);
    }
    else
    {
        format(texto, sizeof texto, "%s\n{FF1111}OCUPADO",
            g_Casas[indice][caNombre]);
    }

    UpdateDynamic3DTextLabelText(g_Casas[indice][caEtiquetaExt], COLOR_MARCA, texto);
    return 1;
}

// ---------------------------------------------------------------------
//  LOCALIZAR AL JUGADOR RESPECTO A LAS CASAS
// ---------------------------------------------------------------------
stock Casa_EnPuertaExterior(playerid)
{
    if (GetPlayerInterior(playerid) != 0) return -1;

    for (new i = 0; i < g_TotalCasas; i++)
    {
        if (EnPunto(playerid, CASA_RADIO_PUERTA, g_Casas[i][caExtX], g_Casas[i][caExtY], g_Casas[i][caExtZ])) return i;
    }
    return -1;
}

// Casa en la que esta AHORA MISMO (dentro del interior).
stock Casa_DondeEstoy(playerid)
{
    new mundo = GetPlayerVirtualWorld(playerid), interior = GetPlayerInterior(playerid);

    for (new i = 0; i < g_TotalCasas; i++)
    {
        if (g_Casas[i][caMundoVirtual] == mundo && g_Casas[i][caInteriorID] == interior) return i;
    }
    return -1;
}

stock bool:Casa_EsPropietario(playerid, indice)
{
    return (g_Casas[indice][caPropietario] != 0 && g_Casas[indice][caPropietario] == Jugador[playerid][jID]);
}

// Cuantas casas del catalogo siguen sin dueno. Usado por el contador en
// pantalla de 30_gps.inc, junto al de coches libres.
stock Casa_ContarLibres()
{
    new total = 0;
    for (new i = 0; i < g_TotalCasas; i++)
    {
        if (g_Casas[i][caPropietario] == 0) total++;
    }
    return total;
}

// ---------------------------------------------------------------------
//  CATALOGO  (/casas)
// ---------------------------------------------------------------------
CMD:casas(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    g_CasaListaLibresTotal[playerid] = 0;
    for (new i = 0; i < g_TotalCasas; i++)
    {
        if (g_Casas[i][caPropietario] != 0) continue;
        g_CasaListaLibres[playerid][g_CasaListaLibresTotal[playerid]] = i;
        g_CasaListaLibresTotal[playerid]++;
    }

    Casa_MostrarPagina(playerid, 0);
    return 1;
}

// Redibuja el dialogo de /casas con la pagina pedida, sobre el
// "congelado" de libres que ya se calculo al abrir el comando (ver
// arriba). Filas especiales "Pagina anterior/siguiente" se insertan
// como filas normales del DIALOG_STYLE_LIST -- el hueco que ocupan se
// descuenta al traducir listitem de vuelta a un indice de casa en
// OnDialogResponse (case DIALOGO_CASAS).
stock Casa_MostrarPagina(playerid, pagina)
{
    g_CasaPaginaActual[playerid] = pagina;

    new total = g_CasaListaLibresTotal[playerid];
    new inicio = pagina * CASA_PAGINA_TAMANO;
    new fin = inicio + CASA_PAGINA_TAMANO;
    if (fin > total) fin = total;

    new lista[2200], linea[128], precio[24];
    format(lista, sizeof lista, "{E8B33A}Casa\tPrecio\n");

    if (total == 0)
    {
        strcat(lista, "{B4B5B7}No quedan casas libres ahora mismo.\t\n", sizeof lista);
    }
    if (pagina > 0)
    {
        strcat(lista, "{5A9BD5}<< Pagina anterior\t\n", sizeof lista);
    }

    for (new i = inicio; i < fin; i++)
    {
        new indice = g_CasaListaLibres[playerid][i];
        FormatearDinero(g_Casas[indice][caPrecio], precio, sizeof precio);
        format(linea, sizeof linea, "{FFFFFF}%s\t%s\n", g_Casas[indice][caNombre], precio);
        strcat(lista, linea, sizeof lista);
    }

    if (fin < total)
    {
        strcat(lista, "{5A9BD5}Pagina siguiente >>\t\n", sizeof lista);
    }

    new totalPaginas = (total > 0) ? (((total - 1) / CASA_PAGINA_TAMANO) + 1) : 1;
    new titulo[64];
    format(titulo, sizeof titulo, "{E8973A}Casas libres (pagina %d/%d)", pagina + 1, totalPaginas);

    ShowPlayerDialog(playerid, DIALOGO_CASAS, DIALOG_STYLE_TABLIST_HEADERS,
        titulo, lista, "Elegir", "Cerrar");
    return 1;
}

// Elegir una casa del catalogo NO la compra: solo marca su puerta en el
// mapa. Comprar exige estar fisicamente alli (/comprarcasa). Desde el
// 12/09/2026 /casas solo lista libres (ver CMD:casas), asi que esta
// funcion ya nunca recibe una casa con dueno -- se quitaron las ramas
// "Tuya"/"Ocupada", que se habian quedado muertas.
stock Casa_MarcarEnMapa(playerid, indice)
{
    SetPlayerCheckpoint(playerid, g_Casas[indice][caExtX], g_Casas[indice][caExtY], g_Casas[indice][caExtZ], 4.0);

    format(g_Buffer, sizeof g_Buffer, "Te he marcado la puerta de '%s' en el mapa.", g_Casas[indice][caNombre]);
    Info(playerid, g_Buffer);

    new precio[24];
    FormatearDinero(g_Casas[indice][caPrecio], precio, sizeof precio);
    format(g_Buffer, sizeof g_Buffer, "Precio: %s. Cuando llegues a la puerta, escribe /comprarcasa.", precio);
    Info(playerid, g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  COMPRA Y VENTA
// ---------------------------------------------------------------------
stock Casa_Comprar(playerid, indice)
{
    if (indice < 0 || indice >= g_TotalCasas) return 0;
    if (g_Casas[indice][caPropietario] != 0)
    {
        Error(playerid, "Esa casa ya tiene dueno.");
        return 0;
    }

    new precio = g_Casas[indice][caPrecio];
    new total = Jugador[playerid][jDinero] + Jugador[playerid][jBanco];
    if (total < precio)
    {
        new falta[24];
        FormatearDinero(precio - total, falta, sizeof falta);
        format(g_Buffer, sizeof g_Buffer, "Te faltan %s entre el efectivo y el banco.", falta);
        Error(playerid, g_Buffer);
        return 0;
    }

    new enMano = Jugador[playerid][jDinero];
    new pagoMano = (enMano >= precio) ? (precio) : (enMano);
    new pagoBanco = precio - pagoMano;

    DarDinero(playerid, -pagoMano);
    Jugador[playerid][jBanco] -= pagoBanco;

    if (pagoBanco > 0)
    {
        Banco_Movimiento(Jugador[playerid][jID], 2, -pagoBanco, Jugador[playerid][jBanco],
            "Inmobiliaria Malaga", "Compra de vivienda");
    }

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);
    GetPlayerName(playerid, g_Casas[indice][caPropietarioNombre], MAX_PLAYER_NAME + 1);
    g_Casas[indice][caPropietario] = Jugador[playerid][jID];
    g_Casas[indice][caCerrada] = true;

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "UPDATE casas SET propietario = %d, propietario_nombre = '%e', cerrada = 1, fecha_compra = NOW() WHERE id = %d",
        Jugador[playerid][jID], g_Casas[indice][caPropietarioNombre], g_Casas[indice][caID]);
    mysql_pquery(g_BD, g_Buffer);

    BD_GuardarCuenta(playerid);
    Casa_ActualizarEtiquetaExterior(indice);

    new dinero[24];
    FormatearDinero(precio, dinero, sizeof dinero);

    SendClientMessage(playerid, COLOR_MARCA, " ");
    format(g_Buffer, sizeof g_Buffer, PREFIJO "Has comprado {FFFFFF}%s{B4B5B7} por {3D9970}%s{B4B5B7}. Esta cerrada con llave.",
        g_Casas[indice][caNombre], dinero);
    SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
    SendClientMessage(playerid, COLOR_MARCA, " ");

    SetPlayerPos(playerid, g_Casas[indice][caIntX], g_Casas[indice][caIntY], g_Casas[indice][caIntZ]);
    SetPlayerFacingAngle(playerid, g_Casas[indice][caIntA]);
    SetPlayerInterior(playerid, g_Casas[indice][caInteriorID]);
    SetPlayerVirtualWorld(playerid, g_Casas[indice][caMundoVirtual]);
    SetCameraBehindPlayer(playerid);
    Info(playerid, "Escribe /cerrarcasa, /guardar y /sacar. /salircasa para volver a la calle.");

    format(g_Buffer, sizeof g_Buffer, "%s compro la casa '%s' por %d", nombre, g_Casas[indice][caNombre], precio);
    EscribirLog("economia", g_Buffer);
    return 1;
}

stock Casa_Vender(playerid, indice)
{
    if (indice < 0 || indice >= g_TotalCasas) return 0;
    if (!Casa_EsPropietario(playerid, indice))
    {
        Error(playerid, "Esa casa no es tuya.");
        return 0;
    }

    for (new s = 0; s < CASA_SLOTS_ALMACEN; s++)
    {
        if (g_CasaAlmacen[indice][s][CA_ITEM] != 0)
        {
            Error(playerid, "Vacia el bahul de la casa antes de venderla (entra y usa /sacar).");
            return 0;
        }
    }

    new reventa = (g_Casas[indice][caPrecio] * CASA_REVENTA_PORCENTAJE) / 100;
    Jugador[playerid][jBanco] += reventa;
    Banco_Movimiento(Jugador[playerid][jID], 1, reventa, Jugador[playerid][jBanco],
        "Inmobiliaria Malaga", "Venta de vivienda");

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);

    format(g_Buffer, sizeof g_Buffer, "%s vendio la casa '%s' por %d", nombre, g_Casas[indice][caNombre], reventa);
    EscribirLog("economia", g_Buffer);

    g_Casas[indice][caPropietario] = 0;
    g_Casas[indice][caPropietarioNombre][0] = EOS;
    g_Casas[indice][caCerrada] = true;

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "UPDATE casas SET propietario = 0, propietario_nombre = '', cerrada = 1 WHERE id = %d", g_Casas[indice][caID]);
    mysql_pquery(g_BD, g_Buffer);

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer, "DELETE FROM casas_almacen WHERE casa_id = %d", g_Casas[indice][caID]);
    mysql_pquery(g_BD, g_Buffer);

    BD_GuardarCuenta(playerid);
    Casa_ActualizarEtiquetaExterior(indice);

    new dinero[24];
    FormatearDinero(reventa, dinero, sizeof dinero);
    format(g_Buffer, sizeof g_Buffer, "Casa vendida. Se han ingresado %s en tu banco.", dinero);
    Exito(playerid, g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  CREAR CASA  (admin, /crearcasa)
//  Ponte donde quieras la puerta de la calle y escribe el comando: la
//  puerta sale ahi. El interior lo eliges de la lista curada de arriba.
// ---------------------------------------------------------------------
CMD:crearcasa(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    if (g_TotalCasas >= MAX_CASAS)
    {
        Error(playerid, "Se ha alcanzado el limite de casas del servidor.");
        return 1;
    }

    GetPlayerPos(playerid, g_CasaCrearExt[playerid][0], g_CasaCrearExt[playerid][1], g_CasaCrearExt[playerid][2]);
    GetPlayerFacingAngle(playerid, g_CasaCrearExt[playerid][3]);

    new lista[1400], linea[48];
    lista[0] = EOS;

    for (new i = 0; i < TOTAL_INTERIORES_CASA; i++)
    {
        format(linea, sizeof linea, "%s\n", g_InterioresCasa[i][icNombre]);
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_CASA_CREAR_INTERIOR, DIALOG_STYLE_LIST,
        "{E8973A}Elige el interior", lista, "Elegir", "Cancelar");
    return 1;
}

stock Casa_Crear(playerid, const nombre[], interiorIdx, Float:extX, Float:extY, Float:extZ, Float:extA, precio)
{
    if (interiorIdx < 0 || interiorIdx >= TOTAL_INTERIORES_CASA) return -1;
    if (g_TotalCasas >= MAX_CASAS) return -1;

    new indice = g_TotalCasas;

    g_Casas[indice][caID] = 0;
    format(g_Casas[indice][caNombre], 49, "%s", nombre);
    g_Casas[indice][caInteriorID]   = g_InterioresCasa[interiorIdx][icInteriorID];
    g_Casas[indice][caMundoVirtual] = 0;
    g_Casas[indice][caExtX] = extX;
    g_Casas[indice][caExtY] = extY;
    g_Casas[indice][caExtZ] = extZ;
    g_Casas[indice][caExtA] = extA;
    g_Casas[indice][caIntX] = g_InterioresCasa[interiorIdx][icX];
    g_Casas[indice][caIntY] = g_InterioresCasa[interiorIdx][icY];
    g_Casas[indice][caIntZ] = g_InterioresCasa[interiorIdx][icZ];
    g_Casas[indice][caIntA] = 0.0;
    g_Casas[indice][caPrecio] = precio;
    g_Casas[indice][caPropietario] = 0;
    g_Casas[indice][caPropietarioNombre][0] = EOS;
    g_Casas[indice][caCerrada] = true;

    g_TotalCasas++;

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "INSERT INTO casas (nombre, interior_id, mundo_virtual, ext_x, ext_y, ext_z, ext_a, \
                            int_x, int_y, int_z, int_a, precio) \
         VALUES ('%e', %d, 0, %f, %f, %f, %f, %f, %f, %f, %f, %d)",
        g_Casas[indice][caNombre], g_Casas[indice][caInteriorID],
        extX, extY, extZ, extA,
        g_Casas[indice][caIntX], g_Casas[indice][caIntY], g_Casas[indice][caIntZ], g_Casas[indice][caIntA],
        precio);

    mysql_pquery(g_BD, g_Buffer, "AlCrearCasa", "dd", playerid, indice);
    return indice;
}

forward AlCrearCasa(playerid, indice);
public AlCrearCasa(playerid, indice)
{
    if (indice < 0 || indice >= g_TotalCasas) return 0;

    g_Casas[indice][caID]           = cache_insert_id();
    g_Casas[indice][caMundoVirtual] = CASA_MUNDO_BASE + g_Casas[indice][caID];

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer, "UPDATE casas SET mundo_virtual = %d WHERE id = %d",
        g_Casas[indice][caMundoVirtual], g_Casas[indice][caID]);
    mysql_pquery(g_BD, g_Buffer);

    Casa_CrearMarcadores(indice);

    if (IsPlayerConnected(playerid))
    {
        format(g_Buffer, sizeof g_Buffer, "Casa '%s' creada (id %d, precio %d).",
            g_Casas[indice][caNombre], g_Casas[indice][caID], g_Casas[indice][caPrecio]);
        Exito(playerid, g_Buffer);
    }

    printf("[CASAS] Creada '%s' (id %d) por administracion.", g_Casas[indice][caNombre], g_Casas[indice][caID]);
    return 1;
}

// ---------------------------------------------------------------------
//  HERRAMIENTAS DE ADMINISTRACION  (usadas por la seccion Casas del panel)
// ---------------------------------------------------------------------
stock Casa_TransferirPropietario(indice, nuevoPropietarioID, const nuevoPropietarioNombre[])
{
    if (indice < 0 || indice >= g_TotalCasas) return 0;

    g_Casas[indice][caPropietario] = nuevoPropietarioID;
    format(g_Casas[indice][caPropietarioNombre], MAX_PLAYER_NAME + 1, "%s", nuevoPropietarioNombre);
    g_Casas[indice][caCerrada] = true;

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "UPDATE casas SET propietario = %d, propietario_nombre = '%e', cerrada = 1, fecha_compra = NOW() WHERE id = %d",
        nuevoPropietarioID, g_Casas[indice][caPropietarioNombre], g_Casas[indice][caID]);
    mysql_pquery(g_BD, g_Buffer);

    Casa_ActualizarEtiquetaExterior(indice);
    return 1;
}

stock Casa_LiberarPropietario(indice)
{
    if (indice < 0 || indice >= g_TotalCasas) return 0;

    g_Casas[indice][caPropietario] = 0;
    g_Casas[indice][caPropietarioNombre][0] = EOS;
    g_Casas[indice][caCerrada] = true;

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "UPDATE casas SET propietario = 0, propietario_nombre = '', cerrada = 1 WHERE id = %d", g_Casas[indice][caID]);
    mysql_pquery(g_BD, g_Buffer);

    Casa_ActualizarEtiquetaExterior(indice);
    return 1;
}

stock Casa_CambiarPrecio(indice, precio)
{
    if (indice < 0 || indice >= g_TotalCasas) return 0;

    g_Casas[indice][caPrecio] = precio;

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer, "UPDATE casas SET precio = %d WHERE id = %d",
        precio, g_Casas[indice][caID]);
    mysql_pquery(g_BD, g_Buffer);

    Casa_ActualizarEtiquetaExterior(indice);
    return 1;
}

// Solo casas libres: borrar una casa con dueno se hace liberandola antes.
stock Casa_Borrar(indice)
{
    if (indice < 0 || indice >= g_TotalCasas) return 0;
    if (g_Casas[indice][caPropietario] != 0) return 0;

    DestroyDynamicPickup(g_Casas[indice][caPickupInt]);
    DestroyDynamic3DTextLabel(g_Casas[indice][caEtiquetaExt]);
    DestroyDynamic3DTextLabel(g_Casas[indice][caEtiquetaInt]);
    DestroyDynamicPickup(g_Casas[indice][caPickupExt]);

    format(g_Buffer, sizeof g_Buffer, "DELETE FROM casas WHERE id = %d", g_Casas[indice][caID]);
    mysql_pquery(g_BD, g_Buffer);   // casas_almacen cae con el ON DELETE CASCADE

    new ultimo = g_TotalCasas - 1;
    g_Casas[indice] = g_Casas[ultimo];
    for (new s = 0; s < CASA_SLOTS_ALMACEN; s++) g_CasaAlmacen[indice][s] = g_CasaAlmacen[ultimo][s];
    g_TotalCasas--;
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /borrarcasa (08/09/2026) -- borra la casa cuya puerta
//  exterior este mas cerca de donde mira el admin, reutilizando
//  Casa_Borrar() de arriba (la misma que ya usaba el panel web): sigue
//  rechazando casas con dueno, no hace falta duplicar esa comprobacion.
// ---------------------------------------------------------------------
CMD:borrarcasa(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new Float:px, Float:py, Float:pz, Float:angulo;
    GetPlayerPos(playerid, px, py, pz);
    GetPlayerFacingAngle(playerid, angulo);

    new Float:fx = px + (CASA_BORRAR_DISTANCIA * floatsin(-angulo, degrees));
    new Float:fy = py + (CASA_BORRAR_DISTANCIA * floatcos(-angulo, degrees));

    new mejor = -1;
    new Float:mejorDist = CASA_BORRAR_RADIO * CASA_BORRAR_RADIO;

    for (new i = 0; i < g_TotalCasas; i++)
    {
        new Float:dx = g_Casas[i][caExtX] - fx;
        new Float:dy = g_Casas[i][caExtY] - fy;
        new Float:dz = g_Casas[i][caExtZ] - pz;
        new Float:dist = (dx * dx) + (dy * dy) + (dz * dz);
        if (dist < mejorDist)
        {
            mejorDist = dist;
            mejor = i;
        }
    }

    if (mejor == -1)
    {
        Error(playerid, "No hay ninguna casa delante de ti.");
        return 1;
    }

    if (g_Casas[mejor][caPropietario] != 0)
    {
        Error(playerid, "Esa casa tiene dueno. Liberala primero (panel web) antes de borrarla.");
        return 1;
    }

    new nombreCasa[49];
    format(nombreCasa, sizeof nombreCasa, "%s", g_Casas[mejor][caNombre]);
    new idCasa = g_Casas[mejor][caID];

    if (!Casa_Borrar(mejor))
    {
        Error(playerid, "No se ha podido borrar esa casa.");
        return 1;
    }

    format(g_Buffer, sizeof g_Buffer, "Casa '%s' (id %d) borrada.", nombreCasa, idCasa);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "id %d", idCasa);
    Admin_Registrar(playerid, "/borrarcasa", nombreCasa, g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  COMPRAR Y VENDER  (solo en la puerta)
// ---------------------------------------------------------------------
CMD:comprarcasa(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new indice = Casa_EnPuertaExterior(playerid);
    if (indice == -1)
    {
        Error(playerid, "Aqui no hay ninguna puerta de una casa. Escribe /casas para que te la marque en el mapa.");
        return 1;
    }

    if (g_Casas[indice][caPropietario] != 0)
    {
        Error(playerid, "Esta casa ya tiene dueno.");
        return 1;
    }

    g_CasaSeleccionada[playerid] = indice;

    new precio[24], cuerpo[300];
    FormatearDinero(g_Casas[indice][caPrecio], precio, sizeof precio);
    format(cuerpo, sizeof cuerpo, "{E8B33A}%s\n\n{B4B5B7}Precio: {3D9970}%s\n\n{FFFFFF}Confirmas la compra?",
        g_Casas[indice][caNombre], precio);

    ShowPlayerDialog(playerid, DIALOGO_CASA_COMPRAR_CONFIRMAR, DIALOG_STYLE_MSGBOX,
        "{E8973A}Comprar casa", cuerpo, "Comprar", "Cancelar");
    return 1;
}

CMD:vendercasa(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new indice = Casa_EnPuertaExterior(playerid);
    if (indice == -1) indice = Casa_DondeEstoy(playerid);
    if (indice == -1)
    {
        Error(playerid, "Tienes que estar en la puerta o dentro de tu casa.");
        return 1;
    }

    if (!Casa_EsPropietario(playerid, indice))
    {
        Error(playerid, "Esta casa no es tuya.");
        return 1;
    }

    g_CasaSeleccionada[playerid] = indice;

    new precio[24], reventa[24], cuerpo[300];
    FormatearDinero(g_Casas[indice][caPrecio], precio, sizeof precio);
    FormatearDinero((g_Casas[indice][caPrecio] * CASA_REVENTA_PORCENTAJE) / 100, reventa, sizeof reventa);

    format(cuerpo, sizeof cuerpo,
        "{E8B33A}%s\n\n{B4B5B7}Pagaste: {3D9970}%s\n{B4B5B7}Reventa (60%%): {3D9970}%s\n\n\
        {FFFFFF}Esto es definitivo. Si el bahul no esta vacio, no se puede vender.",
        g_Casas[indice][caNombre], precio, reventa);

    ShowPlayerDialog(playerid, DIALOGO_CASA_VENDER_CONFIRMAR, DIALOG_STYLE_MSGBOX,
        "{E8973A}Vender casa", cuerpo, "Vender", "Cancelar");
    return 1;
}

// ---------------------------------------------------------------------
//  ENTRAR Y SALIR
//  No son comandos propios: /entrar y /salir son genericos (15_banco.inc),
//  prueban banco, tienda y por ultimo casa en cadena. Estas dos funciones
//  son el eslabon de casa -- devuelven false en silencio si aqui no hay
//  ninguna puerta/salida de casa (para que se intente el siguiente
//  sitio); si SI la hay, se quedan con la interaccion pase lo que pase
//  (con o sin dueno, cerrada o no) y siempre devuelven true.
// ---------------------------------------------------------------------
stock bool:Casa_IntentarEntrar(playerid)
{
    new indice = Casa_EnPuertaExterior(playerid);
    if (indice == -1) return false;

    if (g_Casas[indice][caPropietario] == 0)
    {
        Error(playerid, "Esta casa no tiene dueno. Escribe /comprarcasa.");
        return true;
    }

    if (g_Casas[indice][caCerrada] && !Casa_EsPropietario(playerid, indice))
    {
        Error(playerid, "Esta casa esta cerrada con llave.");
        return true;
    }

    SetPlayerPos(playerid, g_Casas[indice][caIntX], g_Casas[indice][caIntY], g_Casas[indice][caIntZ]);
    SetPlayerFacingAngle(playerid, g_Casas[indice][caIntA]);
    SetPlayerInterior(playerid, g_Casas[indice][caInteriorID]);
    SetPlayerVirtualWorld(playerid, g_Casas[indice][caMundoVirtual]);
    SetCameraBehindPlayer(playerid);
    return true;
}

stock bool:Casa_IntentarSalir(playerid)
{
    new indice = Casa_DondeEstoy(playerid);
    if (indice == -1) return false;

    SetPlayerPos(playerid, g_Casas[indice][caExtX], g_Casas[indice][caExtY], g_Casas[indice][caExtZ]);
    SetPlayerFacingAngle(playerid, g_Casas[indice][caExtA]);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
    SetCameraBehindPlayer(playerid);
    return true;
}

// ---------------------------------------------------------------------
//  ABRIR Y CERRAR
// ---------------------------------------------------------------------
stock Casa_CambiarCerradura(playerid, bool:cerrar)
{
    new indice = Casa_EnPuertaExterior(playerid);
    if (indice == -1) indice = Casa_DondeEstoy(playerid);
    if (indice == -1) { Error(playerid, "Tienes que estar en la puerta o dentro de tu casa."); return 0; }

    if (!Casa_EsPropietario(playerid, indice))
    {
        Error(playerid, "Esta casa no es tuya.");
        return 0;
    }

    if (g_Casas[indice][caCerrada] == cerrar)
    {
        Info(playerid, cerrar ? "Ya estaba cerrada con llave." : "Ya estaba abierta.");
        return 0;
    }

    g_Casas[indice][caCerrada] = cerrar;

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer, "UPDATE casas SET cerrada = %d WHERE id = %d",
        _:g_Casas[indice][caCerrada], g_Casas[indice][caID]);
    mysql_pquery(g_BD, g_Buffer);

    Casa_ActualizarEtiquetaExterior(indice);

    Exito(playerid, cerrar ? "Casa cerrada con llave." : "Casa abierta.");
    return 1;
}

CMD:abrir(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    Casa_CambiarCerradura(playerid, false);
    return 1;
}

CMD:cerrar(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    Casa_CambiarCerradura(playerid, true);
    return 1;
}

// ---------------------------------------------------------------------
//  BAHUL  (/guardar, /sacar)
// ---------------------------------------------------------------------
stock Casa_AlmacenGuardarFila(indice, slot)
{
    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "INSERT INTO casas_almacen (casa_id, slot, item, cantidad, datos) VALUES (%d, %d, %d, %d, %d) \
         ON DUPLICATE KEY UPDATE item = VALUES(item), cantidad = VALUES(cantidad), datos = VALUES(datos)",
        g_Casas[indice][caID], slot, g_CasaAlmacen[indice][slot][CA_ITEM],
        g_CasaAlmacen[indice][slot][CA_CANTIDAD], g_CasaAlmacen[indice][slot][CA_DATOS]);
    mysql_pquery(g_BD, g_Buffer);
    return 1;
}

stock Casa_AlmacenBorrarFila(indice, slot)
{
    mysql_format(g_BD, g_Buffer, sizeof g_Buffer, "DELETE FROM casas_almacen WHERE casa_id = %d AND slot = %d",
        g_Casas[indice][caID], slot);
    mysql_pquery(g_BD, g_Buffer);
    return 1;
}

stock Casa_AlmacenAgregar(indice, item, cantidad, datos)
{
    for (new s = 0; s < CASA_SLOTS_ALMACEN; s++)
    {
        if (g_CasaAlmacen[indice][s][CA_ITEM] != 0) continue;

        g_CasaAlmacen[indice][s][CA_ITEM]     = item;
        g_CasaAlmacen[indice][s][CA_CANTIDAD] = cantidad;
        g_CasaAlmacen[indice][s][CA_DATOS]    = datos;
        Casa_AlmacenGuardarFila(indice, s);
        return 1;
    }
    return 0;
}

// /armario: una sola lista con tu inventario Y el bahul juntos. Las
// filas de arriba (verde, "Guardar") son lo que llevas encima; las de
// abajo (azul, "Sacar") es lo que ya hay en el bahul. Un click en
// cualquier fila mueve ese objeto al otro lado -- no hace falta elegir
// "guardar" o "sacar" primero. g_CasaArmarioCorte recuerda en que fila
// termina el inventario y empieza el bahul, para el dialogo de respuesta
// (Casa_Dialogo, caso DIALOGO_CASA_ARMARIO, mas abajo).
CMD:armario(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new indice = Casa_DondeEstoy(playerid);
    if (indice == -1 || !Casa_EsPropietario(playerid, indice))
    {
        Error(playerid, "Tienes que estar dentro de tu propia casa.");
        return 1;
    }

    g_CasaSeleccionada[playerid] = indice;

    new lista[1400], linea[112], nombre[32];
    format(lista, sizeof lista, "{E8B33A}Objeto\tCantidad\tClick para\n");

    new filasInventario = 0;
    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        new item = Inv_ItemEnSlot(playerid, s);
        if (!ItemValido(item)) continue;

        Item_Nombre(item, nombre, sizeof nombre);
        format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%d\t{32FF32}Guardar\n", nombre, Inv_CantidadEnSlot(playerid, s));
        strcat(lista, linea, sizeof lista);
        filasInventario++;
    }

    new filasBahul = 0;
    for (new s = 0; s < CASA_SLOTS_ALMACEN; s++)
    {
        new item = g_CasaAlmacen[indice][s][CA_ITEM];
        if (!ItemValido(item)) continue;

        Item_Nombre(item, nombre, sizeof nombre);
        format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%d\t{3399FF}Sacar\n", nombre, g_CasaAlmacen[indice][s][CA_CANTIDAD]);
        strcat(lista, linea, sizeof lista);
        filasBahul++;
    }

    if (filasInventario == 0 && filasBahul == 0)
    {
        Error(playerid, "No llevas nada encima y el bahul esta vacio.");
        return 1;
    }

    g_CasaArmarioCorte[playerid] = filasInventario;

    ShowPlayerDialog(playerid, DIALOGO_CASA_ARMARIO, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8973A}Armario", lista, "Elegir", "Cerrar");
    return 1;
}

// Traduce la fila del dialogo del bahul a la ranura real (salta huecos vacios).
stock Casa_AlmacenSlotDeFila(indice, fila)
{
    new contador = 0;
    for (new s = 0; s < CASA_SLOTS_ALMACEN; s++)
    {
        if (!ItemValido(g_CasaAlmacen[indice][s][CA_ITEM])) continue;
        if (contador == fila) return s;
        contador++;
    }
    return -1;
}

// ---------------------------------------------------------------------
//  DIALOGOS
// ---------------------------------------------------------------------
public Casa_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    switch (dialogid)
    {
        case DIALOGO_CASAS:
        {
            if (!response) return 1;

            new pagina = g_CasaPaginaActual[playerid];
            new total = g_CasaListaLibresTotal[playerid];
            new inicio = pagina * CASA_PAGINA_TAMANO;
            new fin = inicio + CASA_PAGINA_TAMANO;
            if (fin > total) fin = total;

            new fila = listitem;
            if (pagina > 0)
            {
                if (fila == 0)
                {
                    Casa_MostrarPagina(playerid, pagina - 1);
                    return 1;
                }
                fila--;
            }

            if (fila >= 0 && fila < (fin - inicio))
            {
                Casa_MarcarEnMapa(playerid, g_CasaListaLibres[playerid][inicio + fila]);
            }
            else if (fin < total)
            {
                Casa_MostrarPagina(playerid, pagina + 1);
            }
            return 1;
        }

        case DIALOGO_CASA_COMPRAR_CONFIRMAR:
        {
            if (!response) return 1;
            Casa_Comprar(playerid, g_CasaSeleccionada[playerid]);
            return 1;
        }

        case DIALOGO_CASA_VENDER_CONFIRMAR:
        {
            if (!response) return 1;
            Casa_Vender(playerid, g_CasaSeleccionada[playerid]);
            return 1;
        }

        case DIALOGO_CASA_ARMARIO:
        {
            if (!response) return 1;

            new indice = g_CasaSeleccionada[playerid];

            // Fila de arriba (inventario) -> guardar. Misma logica de
            // siempre, solo que la fila ya no viene de su propio dialogo.
            if (listitem < g_CasaArmarioCorte[playerid])
            {
                new slot = Inv_SlotDeFila(playerid, listitem);
                if (slot == -1) return 1;

                new cantidad = Inv_CantidadEnSlot(playerid, slot);
                g_CasaInvSlot[playerid] = slot;
                g_CasaCantidadMax[playerid] = cantidad;

                if (cantidad <= 1)
                {
                    new item = Inv_ItemEnSlot(playerid, slot), datos = Inv_DatosEnSlot(playerid, slot);

                    if (!Casa_AlmacenAgregar(indice, item, 1, datos)) { Error(playerid, "El bahul esta lleno."); return 1; }
                    Inv_QuitarItem(playerid, item, 1);
                    Exito(playerid, "Guardado en el bahul.");
                    return 1;
                }

                format(g_Buffer, sizeof g_Buffer, "Cuantas unidades quieres guardar? (1 a %d)", cantidad);
                ShowPlayerDialog(playerid, DIALOGO_CASA_GUARDAR_CANTIDAD, DIALOG_STYLE_INPUT,
                    "{E8973A}Guardar en el bahul", g_Buffer, "Guardar", "Cancelar");
                return 1;
            }

            // Fila de abajo (bahul) -> sacar.
            new filaBahul = listitem - g_CasaArmarioCorte[playerid];
            new slot = Casa_AlmacenSlotDeFila(indice, filaBahul);
            if (slot == -1) return 1;

            g_CasaAlmacenSlot[playerid] = slot;
            g_CasaCantidadMax[playerid] = g_CasaAlmacen[indice][slot][CA_CANTIDAD];

            if (g_CasaCantidadMax[playerid] <= 1)
            {
                Casa_SacarConfirmado(playerid, indice, slot, 1);
                return 1;
            }

            format(g_Buffer, sizeof g_Buffer, "Cuantas unidades quieres sacar? (1 a %d)", g_CasaCantidadMax[playerid]);
            ShowPlayerDialog(playerid, DIALOGO_CASA_SACAR_CANTIDAD, DIALOG_STYLE_INPUT,
                "{E8973A}Sacar del bahul", g_Buffer, "Sacar", "Cancelar");
            return 1;
        }

        case DIALOGO_CASA_GUARDAR_CANTIDAD:
        {
            if (!response) return 1;

            if (!EsNumeroEntero(inputtext) || strval(inputtext) < 1 || strval(inputtext) > g_CasaCantidadMax[playerid])
            {
                Error(playerid, "Cantidad invalida.");
                return 1;
            }

            new cantidad = strval(inputtext);
            new slot = g_CasaInvSlot[playerid];
            new item = Inv_ItemEnSlot(playerid, slot), datos = Inv_DatosEnSlot(playerid, slot);
            new indice = g_CasaSeleccionada[playerid];

            if (!Casa_AlmacenAgregar(indice, item, cantidad, datos)) { Error(playerid, "El bahul esta lleno."); return 1; }

            Inv_QuitarItem(playerid, item, cantidad);
            Exito(playerid, "Guardado en el bahul.");
            return 1;
        }

        case DIALOGO_CASA_SACAR_CANTIDAD:
        {
            if (!response) return 1;

            if (!EsNumeroEntero(inputtext) || strval(inputtext) < 1 || strval(inputtext) > g_CasaCantidadMax[playerid])
            {
                Error(playerid, "Cantidad invalida.");
                return 1;
            }

            Casa_SacarConfirmado(playerid, g_CasaSeleccionada[playerid], g_CasaAlmacenSlot[playerid], strval(inputtext));
            return 1;
        }

        case DIALOGO_CASA_CREAR_INTERIOR:
        {
            if (!response) return 1;
            if (listitem < 0 || listitem >= TOTAL_INTERIORES_CASA) return 1;

            g_CasaCrearInterior[playerid] = listitem;

            format(g_Buffer, sizeof g_Buffer, "Nombre para la casa (sugerencia: %s):", g_InterioresCasa[listitem][icNombre]);
            ShowPlayerDialog(playerid, DIALOGO_CASA_CREAR_NOMBRE, DIALOG_STYLE_INPUT,
                "{E8973A}Crear casa", g_Buffer, "Continuar", "Cancelar");
            return 1;
        }

        case DIALOGO_CASA_CREAR_NOMBRE:
        {
            if (!response) return 1;

            if (EstaVacio(inputtext))
            {
                Error(playerid, "Tienes que escribir un nombre.");
                return 1;
            }

            format(g_CasaCrearNombre[playerid], 49, "%s", inputtext);

            ShowPlayerDialog(playerid, DIALOGO_CASA_CREAR_PRECIO, DIALOG_STYLE_INPUT,
                "{E8973A}Crear casa", "Precio de venta (numero, sin puntos ni simbolos):", "Crear", "Cancelar");
            return 1;
        }

        case DIALOGO_CASA_CREAR_PRECIO:
        {
            if (!response) return 1;

            if (!EsNumeroEntero(inputtext) || strval(inputtext) < 1)
            {
                Error(playerid, "Escribe un precio valido, mayor que 0.");
                return 1;
            }

            Casa_Crear(playerid, g_CasaCrearNombre[playerid], g_CasaCrearInterior[playerid],
                g_CasaCrearExt[playerid][0], g_CasaCrearExt[playerid][1], g_CasaCrearExt[playerid][2], g_CasaCrearExt[playerid][3],
                strval(inputtext));
            return 1;
        }
    }
    return 0;
}

stock Casa_SacarConfirmado(playerid, indice, slot, cantidad)
{
    new item = g_CasaAlmacen[indice][slot][CA_ITEM], datos = g_CasaAlmacen[indice][slot][CA_DATOS];

    new resultado = Inv_DarItem(playerid, item, cantidad, datos);
    if (resultado == 0)  { Error(playerid, "No tienes ranuras libres en el inventario."); return 0; }
    if (resultado == -1) { Error(playerid, "Pesa demasiado para llevarlo encima."); return 0; }

    if (g_CasaAlmacen[indice][slot][CA_CANTIDAD] > cantidad)
    {
        g_CasaAlmacen[indice][slot][CA_CANTIDAD] -= cantidad;
        Casa_AlmacenGuardarFila(indice, slot);
    }
    else
    {
        g_CasaAlmacen[indice][slot][CA_ITEM]     = 0;
        g_CasaAlmacen[indice][slot][CA_CANTIDAD] = 0;
        g_CasaAlmacen[indice][slot][CA_DATOS]    = 0;
        Casa_AlmacenBorrarFila(indice, slot);
    }

    Exito(playerid, "Sacado del bahul.");
    return 1;
}
