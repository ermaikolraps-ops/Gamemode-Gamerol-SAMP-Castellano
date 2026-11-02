/* =====================================================================
 *  27_tienda.inc
 *  ---------------------------------------------------------------------
 *  Gasolinera de 24h. Un mostrador publico, sin trabajo ni faccion detras
 *  (cualquiera puede comprar), con cuatro cosas de picoteo y el bidon de
 *  gasolina como producto estrella para no depender del surtidor.
 *
 *  INTERIOR: el 24/7 de toda la vida, interior 17 (uno de los 6 modelos
 *  de 24/7 que trae GTA:SA, documentado en open.mp/docs/scripting/
 *  resources/interiorids). Si algun dia quieres otra variante, ahi hay
 *  otras cinco (interiores 4, 6, 10, 16 y 18) con sus propias coordenadas.
 *
 *  DOS SUCURSALES desde el 07/09/2026 (Tienda Angel Pine, negocio
 *  APARTE con su propia caja fuerte, NEGOCIO_SLOT_TIENDA_ANGELPINE):
 *  comparten el MISMO interior nativo y el MISMO catalogo, pero cada una
 *  tiene su propio mundo virtual (g_TiendaSucursales[i][tsMundo]) para no
 *  repetir el fallo de Cluckin' Bell 2 del mismo dia (un solo /salir
 *  compartido volviendo siempre a la primera puerta).
 * ===================================================================== */

// Puerta desde la calle (coordenadas marcadas por el usuario con
// /marcarcoord postienda).
#define TIENDA_EXT_X            1929.88
#define TIENDA_EXT_Y            -1776.19
#define TIENDA_EXT_Z            13.5469
#define TIENDA_EXT_A            92.0095

// Punto de llegada dentro del interior. 24/7 #1 del catalogo de open.mp.
// Compartido por las dos sucursales -- lo que las separa es el mundo.
#define TIENDA_INTERIOR         17
#define TIENDA_INT_X            -25.7220
#define TIENDA_INT_Y            -187.8216
#define TIENDA_INT_Z            1003.5469
#define TIENDA_INT_A            0.0

// Icono "i" (1239) de /comprar -- pedido expreso de Vlad 13/09/2026,
// movido de TIENDA_INT_X/Y/Z (punto de llegada) a esta coordenada,
// marcada en /panel > Coordenadas. Compartida por las dos sucursales
// (mismo mueble, mismo interior reutilizado) -- Vlad la marco una vez en
// cada una por separado (mundos distintos) y las dos cayeron a menos de
// medio metro, asi que un solo punto vale para ambas.
#define TIENDA_ICONO_X          -21.3561
#define TIENDA_ICONO_Y          -184.9575
#define TIENDA_ICONO_Z          1003.55

// Icono de tienda ("Diner", 17 -- el radar no trae un icono de 24/7
// generico, este es el mas parecido) siempre visible en el minimapa,
// sin limite de distancia. Peticion de Vlad, 05/09/2026, mismo criterio
// que Banco/Transfender/Comida. Sprite real en TIENDA_ICON_SPRITE
// (01_configuracion.inc, compartido con TIENDA_ANGELPINE_ICON_SLOT para
// que las dos sucursales nunca puedan quedar con iconos distintos).
#define TIENDA_ICON_SLOT        39

// ---------------------------------------------------------------------
//  SUCURSALES -- 0 = original (Gasolinera Los Santos), 1 = Angel Pine.
//  Coordenadas de la Angel Pine en TIENDA_ANGELPINE_* (01_configuracion.inc).
// ---------------------------------------------------------------------
#define TIENDA_TOTAL_SUCURSALES 2

enum E_TIENDA_SUCURSAL
{
    tsNombreCartel[24],
    Float:tsExtX, Float:tsExtY, Float:tsExtZ, Float:tsExtA,
    tsMundo,
    tsNegocioSlot
}
static const g_TiendaSucursales[TIENDA_TOTAL_SUCURSALES][E_TIENDA_SUCURSAL] =
{
    {"GASOLINERA 24H",    TIENDA_EXT_X,           TIENDA_EXT_Y,           TIENDA_EXT_Z,           TIENDA_EXT_A,
        MUNDO_TIENDA,           NEGOCIO_SLOT_TIENDA},
    {"TIENDA ANGEL PINE", TIENDA_ANGELPINE_EXT_X, TIENDA_ANGELPINE_EXT_Y, TIENDA_ANGELPINE_EXT_Z, TIENDA_ANGELPINE_EXT_A,
        MUNDO_TIENDA_ANGELPINE, NEGOCIO_SLOT_TIENDA_ANGELPINE}
};

enum E_TIENDA_ITEM
{
    tiNombre[32],
    tiPrecio,
    tiCantidad          // unidades del item que se entregan por esa compra
};

// Solo lo justo de una gasolinera: el bidon y un par de vicios de
// mostrador. Nada de armas ni municion aqui -- eso es cosa del Vendedor
// de Armas. Comida y bebida se quito entera el 05/09/2026 (peticion de
// Vlad): para eso estan las franquicias de comida rapida (42_comida.inc).
// Precios de estraperlo: caros aposta, es la comodidad de no moverte lo
// que se paga.
static const g_TiendaCatalogo[][E_TIENDA_ITEM] =
{
    {"Bidon de gasolina",   1200,   1},
    {"Semilla de marihuana", 5000, 10},

    // A peticion de Vlad (04/09/2026): el resto de objetos personales se
    // deja para mas adelante.
    {"Chaleco antibalas",   1000,   1},
    {"Botiquin",            1000,   1},

    // A peticion de Vlad (05/09/2026): Portatil Hacker, primera version
    // en esta misma tienda -- se puede mover a otra mas adelante sin
    // que eso haga falta resolver ahora (ver master prompt, seccion 2).
    {"Portatil Hacker",    50000,   1},

    // Gamerol Aventuras: DLC Pesca (51_pesca.inc, 06/09/2026). Precios en
    // PESCA_PRECIO_CANA/PESCA_PRECIO_BARBACOA (01_configuracion.inc) --
    // aqui van como literales porque g_TiendaCatalogo es "static const"
    // y no admite una expresion de otro modulo, pero los dos valen 10.000.
    {"Cana de pescar",     10000,   1},
    {"Barbacoa",           10000,   1}
};
#define TOTAL_TIENDA_CATALOGO (sizeof (g_TiendaCatalogo))

// Accesor para otros modulos (50_pawnshop.inc: calcular el coste real de
// producir marihuana a partir del precio actual de la semilla) --
// g_TiendaCatalogo es "static", nadie de fuera puede leerlo directo.
// Devuelve false si el nombre no existe en el catalogo.
stock bool:Tienda_BuscarPorNombre(const nombre[], &precio, &cantidad)
{
    for (new i = 0; i < TOTAL_TIENDA_CATALOGO; i++)
    {
        if (strcmp(g_TiendaCatalogo[i][tiNombre], nombre, true) == 0)
        {
            precio = g_TiendaCatalogo[i][tiPrecio];
            cantidad = g_TiendaCatalogo[i][tiCantidad];
            return true;
        }
    }
    precio = 0;
    cantidad = 0;
    return false;
}

static STREAMER_TAG_PICKUP:g_PickupTiendaFuera[TIENDA_TOTAL_SUCURSALES];
static STREAMER_TAG_PICKUP:g_PickupTiendaDentro[TIENDA_TOTAL_SUCURSALES];
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaTiendaFuera[TIENDA_TOTAL_SUCURSALES];
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaTiendaDentro[TIENDA_TOTAL_SUCURSALES];

// ---------------------------------------------------------------------
//  MARCADORES -- una vez por sucursal.
// ---------------------------------------------------------------------
stock Tienda_Iniciar()
{
    for (new i = 0; i < TIENDA_TOTAL_SUCURSALES; i++)
    {
        g_PickupTiendaFuera[i] = CreateDynamicPickup(1239, 23,
            g_TiendaSucursales[i][tsExtX], g_TiendaSucursales[i][tsExtY], g_TiendaSucursales[i][tsExtZ], 0, 0, -1, 100.0);

        new etiqueta[96];
        format(etiqueta, sizeof etiqueta, "%s\n{FFFFFF}Escribe {E8B33A}/entrar{FFFFFF} para pasar dentro",
            g_TiendaSucursales[i][tsNombreCartel]);
        g_EtiquetaTiendaFuera[i] = CreateDynamic3DTextLabel(etiqueta,
            COLOR_MARCA, g_TiendaSucursales[i][tsExtX], g_TiendaSucursales[i][tsExtY], g_TiendaSucursales[i][tsExtZ] + 0.7, 14.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

        g_PickupTiendaDentro[i] = CreateDynamicPickup(1239, 23, TIENDA_ICONO_X, TIENDA_ICONO_Y, TIENDA_ICONO_Z,
            g_TiendaSucursales[i][tsMundo], TIENDA_INTERIOR, -1, 100.0);

        g_EtiquetaTiendaDentro[i] = CreateDynamic3DTextLabel(
            "{FFFFFF}Escribe {E8B33A}/comprar{FFFFFF} para ver el genero\n{FFFFFF}o {E8B33A}/salir{FFFFFF} para irte",
            COLOR_BLANCO, TIENDA_ICONO_X, TIENDA_ICONO_Y, TIENDA_ICONO_Z + 0.7, 8.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, g_TiendaSucursales[i][tsMundo], TIENDA_INTERIOR, -1, 25.0);
    }

    print("  [TIENDA] Gasolinera y Tienda Angel Pine listas.");
    return 1;
}

// Icono de tienda siempre visible en el minimapa, una vez por sucursal.
// Se llama en OnPlayerConnect (99_callbacks.inc), igual que el resto de
// _OnConectar de icono fijo del gamemode.
// Iconos de minimapa quitados el 11/09/2026 -- peticion expresa de Vlad
// de limpiar todos los iconos del mapa (la gente usa /gps). Se deja el
// stock vacio para no tocar el call site en OnPlayerConnect.
stock Tienda_OnConectar(playerid)
{
    #pragma unused playerid
    return 1;
}

// ---------------------------------------------------------------------
//  ¿EN QUE SUCURSAL ESTOY (DENTRO)?  -- por interior + mundo, mismo
//  patron que Comida_SucursalDe/Casa_DondeEstoy.
// ---------------------------------------------------------------------
stock Tienda_SucursalDe(playerid)
{
    if (GetPlayerInterior(playerid) != TIENDA_INTERIOR) return -1;

    new mundo = GetPlayerVirtualWorld(playerid);
    for (new i = 0; i < TIENDA_TOTAL_SUCURSALES; i++)
    {
        if (g_TiendaSucursales[i][tsMundo] == mundo) return i;
    }
    return -1;
}

// ---------------------------------------------------------------------
//  ENTRAR Y SALIR
//  Los comandos /entrar y /salir en si viven en 15_banco.inc (son el
//  punto de entrada generico a cualquier interior fijo del servidor);
//  estas dos funciones son las que llama desde alli cuando el punto del
//  banco no coincide, para no duplicar el comando.
// ---------------------------------------------------------------------
stock bool:Tienda_IntentarEntrar(playerid)
{
    if (GetPlayerInterior(playerid) != 0) return false;

    for (new i = 0; i < TIENDA_TOTAL_SUCURSALES; i++)
    {
        if (!EnPunto(playerid, 2.5, g_TiendaSucursales[i][tsExtX], g_TiendaSucursales[i][tsExtY], g_TiendaSucursales[i][tsExtZ])) continue;

        SetPlayerPos(playerid, TIENDA_INT_X, TIENDA_INT_Y, TIENDA_INT_Z);
        SetPlayerFacingAngle(playerid, TIENDA_INT_A);
        SetPlayerInterior(playerid, TIENDA_INTERIOR);
        SetPlayerVirtualWorld(playerid, g_TiendaSucursales[i][tsMundo]);
        SetCameraBehindPlayer(playerid);

        Info(playerid, "Escribe /comprar para ver el genero.");
        return true;
    }
    return false;
}

stock bool:Tienda_IntentarSalir(playerid)
{
    new indice = Tienda_SucursalDe(playerid);
    if (indice == -1) return false;

    SetPlayerPos(playerid, g_TiendaSucursales[indice][tsExtX], g_TiendaSucursales[indice][tsExtY], g_TiendaSucursales[indice][tsExtZ]);
    SetPlayerFacingAngle(playerid, g_TiendaSucursales[indice][tsExtA]);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
    SetCameraBehindPlayer(playerid);
    return true;
}

// ---------------------------------------------------------------------
//  COMPRAR
//  Se puede usar en cualquier punto del interior, no hace falta mostrador.
// ---------------------------------------------------------------------
CMD:comprar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (Tienda_SucursalDe(playerid) == -1)
    {
        Error(playerid, "Tienes que estar dentro de una tienda.");
        return 1;
    }

    Tienda_MostrarCatalogo(playerid);
    return 1;
}

// Separado del comando para poder reabrirlo solo tras cada compra (ver
// Tienda_Dialogo) sin obligar a escribir /comprar cada vez.
stock Tienda_MostrarCatalogo(playerid)
{
    new lista[512], linea[64], dinero[24], nombre[40];
    format(lista, sizeof lista, "Articulo\tPrecio\n");

    for (new i = 0; i < TOTAL_TIENDA_CATALOGO; i++)
    {
        FormatearDinero(g_TiendaCatalogo[i][tiPrecio], dinero, sizeof dinero);

        if (g_TiendaCatalogo[i][tiCantidad] > 1)
        {
            format(nombre, sizeof nombre, "%s (x%d)", g_TiendaCatalogo[i][tiNombre], g_TiendaCatalogo[i][tiCantidad]);
        }
        else
        {
            format(nombre, sizeof nombre, "%s", g_TiendaCatalogo[i][tiNombre]);
        }

        format(linea, sizeof linea, "%s\t%s\n", nombre, dinero);
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_TIENDA, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8B33A}Gasolinera 24h", lista, "Comprar", "Cerrar");
    return 1;
}

// ---------------------------------------------------------------------
//  DIALOGOS
// ---------------------------------------------------------------------
stock Tienda_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext

    switch (dialogid)
    {
        case DIALOGO_TIENDA:
        {
            if (!response) return 1;
            if (listitem < 0 || listitem >= TOTAL_TIENDA_CATALOGO) return 1;

            new indiceSucursal = Tienda_SucursalDe(playerid);
            if (indiceSucursal == -1)
            {
                Error(playerid, "Ya no estas dentro de la tienda.");
                return 1;
            }

            new item = Item_Buscar(g_TiendaCatalogo[listitem][tiNombre]);
            if (item == 0)
            {
                Error(playerid, "Ese articulo ya no esta disponible.");
                Tienda_MostrarCatalogo(playerid);
                return 1;
            }

            new precio = g_TiendaCatalogo[listitem][tiPrecio];
            if (!TieneDinero(playerid, precio))
            {
                Error(playerid, "No te llega el dinero para eso.");
                Tienda_MostrarCatalogo(playerid);
                return 1;
            }

            DarDinero(playerid, -precio);

            new cantidad = g_TiendaCatalogo[listitem][tiCantidad];
            if (Inv_DarItem(playerid, item, cantidad, 0) <= 0)
            {
                DarDinero(playerid, precio);
                Error(playerid, "No te cabe en el inventario ahora mismo.");
                Tienda_MostrarCatalogo(playerid);
                return 1;
            }

            // Venta ya confirmada de verdad (no se va a deshacer): el 60%
            // va a la caja fuerte del dueno de ESTA sucursal (cada una su
            // propia ranura de negocio), el 40% restante siempre a Bonos
            // del Estado, igual que antes de que existiera la segunda.
            Negocio_IngresoTienda(precio, g_TiendaSucursales[indiceSucursal][tsNegocioSlot]);

            new mensaje[160];

            // Mensaje especial (texto pedido literal por Vlad, master
            // prompt seccion 3) -- el Portatil Hacker es el unico
            // articulo de la tienda con aviso propio, el resto sigue el
            // generico de siempre.
            if (strcmp(g_TiendaCatalogo[listitem][tiNombre], "Portatil Hacker", true) == 0)
            {
                format(mensaje, sizeof mensaje,
                    "Has comprado el Portatil de Hacker. En esta edicion no esta conectado a ninguna pagina.");
            }
            else if (cantidad > 1)
            {
                format(mensaje, sizeof mensaje, "Has comprado: %s x%d.", g_TiendaCatalogo[listitem][tiNombre], cantidad);
            }
            else
            {
                format(mensaje, sizeof mensaje, "Has comprado: %s.", g_TiendaCatalogo[listitem][tiNombre]);
            }
            Exito(playerid, mensaje);

            // Se reabre solo para poder encadenar compras (pedido expreso:
            // comprar diez semillas no debe obligar a escribir /comprar
            // diez veces). Solo se cierra de verdad con "Cerrar".
            Tienda_MostrarCatalogo(playerid);
            return 1;
        }
    }
    return 0;
}
