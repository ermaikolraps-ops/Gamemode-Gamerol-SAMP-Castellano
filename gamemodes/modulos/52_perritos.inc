/* =====================================================================
 *  52_perritos.inc
 *  ---------------------------------------------------------------------
 *  Puesto de Perritos Calientes, encargo de Vlad del 07/09/2026.
 *  Coordenada marcada en /panel (tabla `coordenadas`, id 67).
 *
 *  Sigue EXACTAMENTE el patron de /comer (42_comida.inc): dialogo con
 *  el menu, cobra, sube vida (tope 100), reproduce la animacion real
 *  del juego (libreria FOOD) y suma hambre/sed. La UNICA diferencia a
 *  proposito, decision de Vlad: este puesto NO es una franquicia
 *  comprable -- no hay negocio, no hay dueno, no hay caja fuerte. Lo
 *  cobrado se destruye, igual que cualquier compra en la tienda antes
 *  de que existieran los negocios.
 *
 *  Precios y vida: NO los dio Vlad, propuestos por Claudia siguiendo la
 *  misma proporcion que el resto del menu de comida rapida (g_ComidaMenu,
 *  42_comida.inc) -- avisar si se quieren otros numeros, es un solo
 *  array a tocar (g_PerritoMenu, aqui abajo).
 * ===================================================================== */

enum E_PRODUCTO_PERRITO
{
    ppNombre[32],
    ppPrecio,
    Float:ppVida,
    Float:ppHambre,
    Float:ppSed
}

#define PERRITO_TOTAL_PRODUCTOS 3
static const g_PerritoMenu[PERRITO_TOTAL_PRODUCTOS][E_PRODUCTO_PERRITO] =
{
    {"Perrito Caliente + Bebida", 180, 20.0, 40.0, 40.0},
    {"Perrito de Tofu + Bebida",  170, 18.0, 30.0, 40.0},
    {"Perrito Halal + Bebida",    170, 18.0, 30.0, 40.0}
};

static STREAMER_TAG_3D_TEXT_LABEL:g_PerritoEtiqueta;
static g_PerritoProductoServido[MAX_PLAYERS];

stock Perrito_Iniciar()
{
    g_PerritoEtiqueta = CreateDynamic3DTextLabel(
        "{4FD1C5}Perritos Calientes\n{FFFFFF}Usa {E8B33A}/perritocaliente",
        0x4FD1C5FF, PERRITO_X, PERRITO_Y, PERRITO_Z + 0.9, 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 25.0);
    return 1;
}

// Icono de minimapa quitado el 11/09/2026 -- peticion expresa de Vlad
// de limpiar todos los iconos del mapa (la gente usa /gps). Se deja el
// stock vacio para no tocar el call site en OnPlayerConnect.
stock Perrito_OnConectar(playerid)
{
    #pragma unused playerid
    return 1;
}

CMD:perritocaliente(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (!EnPunto(playerid, PERRITO_RADIO, PERRITO_X, PERRITO_Y, PERRITO_Z))
    {
        Error(playerid, "Tienes que estar en el puesto de Perritos Calientes.");
        return 1;
    }

    new lista[400], linea[64], precio[24];
    format(lista, sizeof lista, "Producto\tPrecio\n");

    for (new p = 0; p < PERRITO_TOTAL_PRODUCTOS; p++)
    {
        FormatearDinero(g_PerritoMenu[p][ppPrecio], precio, sizeof precio);
        format(linea, sizeof linea, "%s\t%s\n", g_PerritoMenu[p][ppNombre], precio);
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_PERRITO, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8B33A}Perritos Calientes", lista, "Pedir", "Cerrar");
    return 1;
}

stock Perrito_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext

    if (dialogid != DIALOGO_PERRITO) return 0;
    if (!response) return 1;
    if (listitem < 0 || listitem >= PERRITO_TOTAL_PRODUCTOS) return 1;

    // Revalida que sigue en el puesto -- pudo alejarse con el dialogo abierto.
    if (!EnPunto(playerid, PERRITO_RADIO, PERRITO_X, PERRITO_Y, PERRITO_Z))
    {
        Error(playerid, "Ya no estas en el puesto de Perritos Calientes.");
        return 1;
    }

    new precio = g_PerritoMenu[listitem][ppPrecio];
    if (!TieneDinero(playerid, precio))
    {
        Error(playerid, "No te llega el dinero para eso.");
        return 1;
    }

    // Puesto publico, sin dueno: el cobro se destruye, a proposito
    // (decision de Vlad, 07/09/2026) -- a diferencia de /comer, que
    // ingresa el 100% a la caja fuerte de la franquicia.
    DarDinero(playerid, -precio);

    new Float:vida;
    GetPlayerHealth(playerid, vida);
    vida += g_PerritoMenu[listitem][ppVida];
    if (vida > 100.0) vida = 100.0;
    SetPlayerHealth(playerid, vida);

    // La animacion se retrasa un poco -- mismo bug y misma solucion que
    // Comida_AnimarRetardado (42_comida.inc): aplicarla en el mismo tick
    // que se cierra el dialogo no se veia (07/09/2026).
    g_PerritoProductoServido[playerid] = listitem;
    SetTimerEx("Perrito_AnimarRetardado", COMIDA_ANIMACION_RETRASO_MS, false, "d", playerid);

    new hsTexto[64];
    HambreSed_Sumar(playerid, g_PerritoMenu[listitem][ppHambre], g_PerritoMenu[listitem][ppSed], hsTexto, sizeof hsTexto);

    format(g_Buffer, sizeof g_Buffer, "Has pedido: %s.%s", g_PerritoMenu[listitem][ppNombre], hsTexto);
    Exito(playerid, g_Buffer);
    return 1;
}

forward Perrito_AnimarRetardado(playerid);
public Perrito_AnimarRetardado(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    if (!EnPunto(playerid, PERRITO_RADIO, PERRITO_X, PERRITO_Y, PERRITO_Z)) return 0;

    Animar(playerid, "FOOD", "EAT_Burger", 4.1, false, false, false, false, 0);

    new listitem = g_PerritoProductoServido[playerid];
    if (listitem < 0 || listitem >= PERRITO_TOTAL_PRODUCTOS) return 1;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[128];
    NombreRP(playerid, nombre, sizeof nombre);
    format(mensaje, sizeof mensaje, "* %s come %s.", nombre, g_PerritoMenu[listitem][ppNombre]);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
    return 1;
}
