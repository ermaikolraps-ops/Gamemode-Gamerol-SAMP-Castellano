/* =====================================================================
 *  42_comida.inc
 *  ---------------------------------------------------------------------
 *  Las tres cadenas de comida rapida de San Andreas -- Burger Shot,
 *  Cluckin' Bell y Well Stacked Pizza Co. -- como negocios comprables
 *  (ranuras 3/4/5 de 32_negocios.inc, ver su cabecera y la de COMIDA
 *  RAPIDA en 01_configuracion.inc para el porque del modelo de
 *  franquicia). Este archivo solo pone encima lo propio de comida:
 *
 *    /comer            dentro de cualquier sucursal, abre el menu de
 *                       esa cadena (3 productos). Compra: sube vida
 *                       (tope 100), reproduce la animacion de comer de
 *                       verdad del juego (libreria FOOD) y el 100% del
 *                       precio va a la caja fuerte de la franquicia si
 *                       tiene dueno -- si no tiene, se pierde, igual que
 *                       el resto de negocios sin comprar.
 *
 *  Comprar la franquicia, la caja fuerte, /soltarnegocio,
 *  /vendernegocio... todo eso ya es Negocio_Comprar() y compania de
 *  32_negocios.inc, generico: no hay nada que escribir aparte, solo
 *  rellenar las 3 filas de la tabla (ya hecho).
 *
 *  PUERTAS EXTERIORES: primera sucursal de cada cadena anadida el
 *  03/09/2026 (coordenadas COMIDA_LS_* de 01_configuracion.inc, marcadas
 *  por Vlad con /marcarcoord). Quedan ~27 mas por marcar de las ~30 del
 *  mapa original -- anadir una es una fila nueva en Comida_Iniciar(),
 *  nada mas. El icono de /comprarnegocio de cada franquicia vive fuera,
 *  cerca de su sucursal marcada (ver 32_negocios.inc, neCompraExterior);
 *  la caja fuerte se queda dentro del interior compartido.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  MENU -- 3 productos fijos por cadena. Precios en la misma banda que
//  el catalogo de 27_tienda.inc (picoteo "de estraperlo").
// ---------------------------------------------------------------------
enum E_PRODUCTO_COMIDA
{
    pcNombre[32],
    pcPrecio,
    Float:pcVida,       // vida que restaura, antes del tope de 100
    Float:pcHambre,     // hambre que recupera (44_hambre_sed.inc)
    Float:pcSed         // sed que recupera -- >0 solo en los "menu" (llevan bebida)
}

static const g_ComidaMenu[COMIDA_TOTAL_CADENAS][COMIDA_PRODUCTOS_POR_CADENA][E_PRODUCTO_COMIDA] =
{
    // Burger Shot
    {
        {"Burger Shot Clasica", 120, 15.0, 30.0,  0.0},
        {"Menu Doble Shot",     220, 30.0, 45.0, 20.0},
        {"Batido Shot",          90, 10.0,  0.0, 20.0},
        {"Te Helado",           100,  5.0,  0.0, 50.0},
        {"Gaseosa",              90,  5.0,  0.0, 45.0}
    },
    // Cluckin' Bell
    {
        {"Cubo de Alitas",      150, 20.0, 35.0,  0.0},
        {"Menu Cluck",          200, 25.0, 40.0, 20.0},
        {"Nuggets Familiares",  250, 30.0, 45.0,  0.0},
        {"Te Helado",           100,  5.0,  0.0, 50.0},
        {"Gaseosa",              90,  5.0,  0.0, 45.0}
    },
    // Well Stacked Pizza Co.
    {
        {"Porcion Suelta",       80, 10.0, 20.0,  0.0},
        {"Pizza Familiar",      300, 35.0, 55.0,  0.0},
        {"Pizza Vegetariana",   180, 20.0, 35.0,  0.0},
        {"Te Helado",           100,  5.0,  0.0, 50.0},
        {"Gaseosa",              90,  5.0,  0.0, 45.0}
    }
};

// Nombre de cadena y animacion real del juego (libreria "FOOD",
// confirmada contra open.mp/docs/scripting/resources/animations) --
// EAT_Burger ya la usa 10_inventario.inc para los consumibles normales,
// aqui cada cadena tiene la suya propia.
static const g_ComidaNombreCadena[COMIDA_TOTAL_CADENAS][32] =
{
    "Burger Shot", "Cluckin' Bell", "Well Stacked Pizza"
};
static const g_ComidaAnimacion[COMIDA_TOTAL_CADENAS][16] =
{
    "EAT_Burger", "EAT_Chicken", "EAT_Pizza"
};
static const g_ComidaSlotNegocio[COMIDA_TOTAL_CADENAS] =
{
    NEGOCIO_SLOT_BURGERSHOT, NEGOCIO_SLOT_CLUCKINBELL, NEGOCIO_SLOT_PIZZA
};
static const g_ComidaInterior[COMIDA_TOTAL_CADENAS] =
{
    COMIDA_INTERIOR_BURGERSHOT, COMIDA_INTERIOR_CLUCKINBELL, COMIDA_INTERIOR_PIZZA
};
// g_ComidaMundo (un mundo por CADENA) se retiro el 07/09/2026 -- cada
// sucursal tiene ahora su propio mundo (scMundo, mas abajo), no uno
// compartido por toda la cadena. Ver el aviso junto a Comida_SucursalDe.

// g_ComidaIconoSprite se quito el 11/09/2026 junto con el resto de
// iconos de minimapa del gamemode -- peticion expresa de Vlad, la gente
// usa /gps.
#define COMIDA_ICON_SLOT_BASE   40

// Que cadena tiene abierta cada jugador en el dialogo del menu.
static g_ComidaCadenaAbierta[MAX_PLAYERS];

// Bug real encontrado el 07/09/2026: aplicar ApplyAnimation en el MISMO
// tick en que se procesa la respuesta de un dialogo (OnDialogResponse)
// no se ve -- el cliente todavia esta cerrando visualmente el cuadro de
// dialogo y descarta la animacion. Se retrasa un poco (mismo patron que
// Medicos_AplicarAnimacionCaida en 36_medicos.inc, que ya retrasa una
// animacion 1s por el mismo tipo de motivo: el cliente necesita un
// instante libre de UI para aplicarla de verdad).
#define COMIDA_ANIMACION_RETRASO_MS 300
static g_ComidaProductoServido[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  SUCURSALES EXTERIORES -- una por cadena desde el 03/09/2026, el resto
//  todavia por marcar.
// ---------------------------------------------------------------------
enum E_SUCURSAL_COMIDA
{
    scCadena,
    Float:scX, Float:scY, Float:scZ, Float:scA,
    bool:scActiva,
    scMundo,    // PROPIO de esta sucursal, NO compartido con las demas
                // de la misma cadena -- arreglado 07/09/2026, ver
                // MUNDO_COMIDA_SUCURSAL_BASE en 01_configuracion.inc.
    Float:scIconoX, Float:scIconoY, Float:scIconoZ
                // Icono "i" de /comer dentro del interior (13/09/2026,
                // pedido expreso de Vlad) -- distinto del punto de
                // llegada (COMIDA_INT_*): es solo donde flota el
                // letrero/pickup.
}
new g_SucursalesComida[MAX_SUCURSALES_COMIDA][E_SUCURSAL_COMIDA];
new g_TotalSucursalesComida = 0;

new STREAMER_TAG_3D_TEXT_LABEL:g_SucursalEtiqueta[MAX_SUCURSALES_COMIDA]
    = {STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID, ...};
new STREAMER_TAG_PICKUP:g_SucursalIconoPickup[MAX_SUCURSALES_COMIDA]
    = {STREAMER_TAG_PICKUP:INVALID_STREAMER_ID, ...};
new STREAMER_TAG_3D_TEXT_LABEL:g_SucursalIconoEtiqueta[MAX_SUCURSALES_COMIDA]
    = {STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID, ...};

// Rellena aqui cuando lleguen las coordenadas reales, o carga desde una
// tabla si algun dia se prefiere admin en caliente (/creartienda ya usa
// ese patron en 27_tienda.inc si hace falta copiarlo). De momento, en
// codigo, que es donde vive el resto del mapa de negocios/gasolineras:
//
//   Comida_AnadirSucursal(COMIDA_CADENA_BURGERSHOT, X, Y, Z, A);
//
// Uno por linea, tantas como sucursales tenga esa cadena.
stock Comida_Iniciar()
{
    // Primera sucursal real de cada cadena, marcada por Vlad con
    // /marcarcoord el 03/09/2026 (tabla `coordenadas`). Coordenadas
    // centralizadas en 01_configuracion.inc (COMIDA_LS_*). Icono de
    // /comer (COMIDA_ICONOCARTA_*) anadido el 13/09/2026, pedido expreso
    // de Vlad, coordenada marcada aparte en /panel.
    Comida_AnadirSucursal(COMIDA_CADENA_BURGERSHOT,  COMIDA_LS_BURGERSHOT_X,  COMIDA_LS_BURGERSHOT_Y,  COMIDA_LS_BURGERSHOT_Z,  COMIDA_LS_BURGERSHOT_A,
                           COMIDA_ICONOCARTA_BURGERSHOT_X, COMIDA_ICONOCARTA_BURGERSHOT_Y, COMIDA_ICONOCARTA_BURGERSHOT_Z);
    Comida_AnadirSucursal(COMIDA_CADENA_CLUCKINBELL, COMIDA_LS_CLUCKINBELL_X, COMIDA_LS_CLUCKINBELL_Y, COMIDA_LS_CLUCKINBELL_Z, COMIDA_LS_CLUCKINBELL_A,
                           COMIDA_ICONOCARTA_CLUCKINBELL_X, COMIDA_ICONOCARTA_CLUCKINBELL_Y, COMIDA_ICONOCARTA_CLUCKINBELL_Z);
    Comida_AnadirSucursal(COMIDA_CADENA_PIZZA,       COMIDA_LS_PIZZA_X,       COMIDA_LS_PIZZA_Y,       COMIDA_LS_PIZZA_Z,       COMIDA_LS_PIZZA_A,
                           COMIDA_ICONOCARTA_PIZZA_X, COMIDA_ICONOCARTA_PIZZA_Y, COMIDA_ICONOCARTA_PIZZA_Z);

    // Segunda sucursal de Cluckin' Bell, "Clucking Bell 2" (07/09/2026,
    // coordenada id 66) -- mismo dueno y caja fuerte que la primera,
    // solo una puerta mas. Icono de minimapa y etiqueta salen solos del
    // bucle de mas abajo, cero codigo nuevo.
    Comida_AnadirSucursal(COMIDA_CADENA_CLUCKINBELL, COMIDA_CB2_X, COMIDA_CB2_Y, COMIDA_CB2_Z, COMIDA_CB2_A,
                           COMIDA_ICONOCARTA_CB2_X, COMIDA_ICONOCARTA_CB2_Y, COMIDA_ICONOCARTA_CB2_Z);

    for (new i = 0; i < g_TotalSucursalesComida; i++)
    {
        if (!g_SucursalesComida[i][scActiva]) continue;

        new cadena = g_SucursalesComida[i][scCadena];
        new texto[64];
        format(texto, sizeof texto, "%s\n{FFFFFF}Escribe {E8B33A}/entrar", g_ComidaNombreCadena[cadena]);

        g_SucursalEtiqueta[i] = CreateDynamic3DTextLabel(texto, COLOR_MARCA,
            g_SucursalesComida[i][scX], g_SucursalesComida[i][scY], g_SucursalesComida[i][scZ] + 0.7, 10.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 25.0);

        // Icono "i" (1239) dentro del interior, escalado a la sucursal
        // (su propio mundo + el interior de su cadena) -- pedido expreso
        // de Vlad 13/09/2026.
        g_SucursalIconoPickup[i] = CreateDynamicPickup(1239, 23,
            g_SucursalesComida[i][scIconoX], g_SucursalesComida[i][scIconoY], g_SucursalesComida[i][scIconoZ],
            g_SucursalesComida[i][scMundo], g_ComidaInterior[cadena], -1, 100.0);

        g_SucursalIconoEtiqueta[i] = CreateDynamic3DTextLabel(
            "{FFFFFF}Escribe {E8B33A}/comer{FFFFFF} para ver la carta",
            COLOR_BLANCO, g_SucursalesComida[i][scIconoX], g_SucursalesComida[i][scIconoY], g_SucursalesComida[i][scIconoZ] + 0.7, 8.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, g_SucursalesComida[i][scMundo], g_ComidaInterior[cadena], -1, 25.0);
    }

    printf("  [COMIDA] %d sucursales activas de comida rapida.", g_TotalSucursalesComida);
    return 1;
}

stock Comida_AnadirSucursal(cadena, Float:x, Float:y, Float:z, Float:a, Float:iconoX = 0.0, Float:iconoY = 0.0, Float:iconoZ = 0.0)
{
    if (g_TotalSucursalesComida >= MAX_SUCURSALES_COMIDA) return -1;

    new indice = g_TotalSucursalesComida++;
    g_SucursalesComida[indice][scCadena] = cadena;
    g_SucursalesComida[indice][scX] = x;
    g_SucursalesComida[indice][scY] = y;
    g_SucursalesComida[indice][scZ] = z;
    g_SucursalesComida[indice][scA] = a;
    g_SucursalesComida[indice][scActiva] = true;
    g_SucursalesComida[indice][scMundo] = MUNDO_COMIDA_SUCURSAL_BASE + indice;
    g_SucursalesComida[indice][scIconoX] = iconoX;
    g_SucursalesComida[indice][scIconoY] = iconoY;
    g_SucursalesComida[indice][scIconoZ] = iconoZ;
    return indice;
}

// Iconos de minimapa quitados el 11/09/2026 -- peticion expresa de Vlad
// de limpiar todos los iconos del mapa (la gente usa /gps). Se deja el
// stock vacio para no tocar el call site en OnPlayerConnect.
stock Comida_OnConectar(playerid)
{
    #pragma unused playerid
    return 1;
}

// ---------------------------------------------------------------------
//  ENTRAR Y SALIR -- mismo patron que Tienda_IntentarEntrar/Salir
//  (27_tienda.inc): /entrar y /salir viven en 15_banco.inc y prueban
//  cada sistema con interior fijo por turno.
// ---------------------------------------------------------------------
public bool:Comida_IntentarEntrar(playerid)
{
    if (GetPlayerInterior(playerid) != 0) return false;

    for (new i = 0; i < g_TotalSucursalesComida; i++)
    {
        if (!g_SucursalesComida[i][scActiva]) continue;
        if (!EnPunto(playerid, COMIDA_RADIO_PUERTA, g_SucursalesComida[i][scX], g_SucursalesComida[i][scY], g_SucursalesComida[i][scZ])) continue;

        new cadena = g_SucursalesComida[i][scCadena];
        SetPlayerPos(playerid, Comida_InteriorCoord(cadena, 0), Comida_InteriorCoord(cadena, 1), Comida_InteriorCoord(cadena, 2));
        SetPlayerFacingAngle(playerid, Comida_InteriorCoord(cadena, 3));
        SetPlayerInterior(playerid, g_ComidaInterior[cadena]);
        // Mundo PROPIO de esta sucursal (no el de la cadena entera) --
        // arreglado 07/09/2026, ver el comentario de scMundo mas arriba.
        SetPlayerVirtualWorld(playerid, g_SucursalesComida[i][scMundo]);
        SetCameraBehindPlayer(playerid);

        Info(playerid, "Escribe /comer para ver la carta.");
        return true;
    }
    return false;
}

// Sucursal EXACTA en la que esta ahora mismo el jugador (por interior +
// su mundo propio), o -1 si no esta en ninguna. Arreglado 07/09/2026 --
// antes solo se sabia la CADENA (Comida_CadenaDe), suficiente para
// pintar el menu pero no para saber por que puerta salir.
stock Comida_SucursalDe(playerid)
{
    new interior = GetPlayerInterior(playerid);
    new mundo = GetPlayerVirtualWorld(playerid);

    for (new i = 0; i < g_TotalSucursalesComida; i++)
    {
        if (!g_SucursalesComida[i][scActiva]) continue;
        if (interior != g_ComidaInterior[g_SucursalesComida[i][scCadena]]) continue;
        if (mundo != g_SucursalesComida[i][scMundo]) continue;
        return i;
    }
    return -1;
}

public bool:Comida_IntentarSalir(playerid)
{
    new sucursal = Comida_SucursalDe(playerid);
    if (sucursal == -1) return false;

    // Sale por la MISMA puerta por la que entro -- arreglado
    // 07/09/2026 (antes salia siempre por la primera sucursal activa de
    // la cadena, aunque hubiera entrado por otra).
    SetPlayerPos(playerid, g_SucursalesComida[sucursal][scX], g_SucursalesComida[sucursal][scY], g_SucursalesComida[sucursal][scZ]);
    SetPlayerFacingAngle(playerid, g_SucursalesComida[sucursal][scA]);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
    SetCameraBehindPlayer(playerid);
    return true;
}

// Pequeno truco para no repetir 4 lineas de "segun la cadena, X o Y o
// Z": ndice 0=x,1=y,2=z,3=angulo de la posicion del mostrador dentro
// del interior de esa cadena.
stock Float:Comida_InteriorCoord(cadena, indice)
{
    switch (cadena)
    {
        case COMIDA_CADENA_BURGERSHOT:
        {
            switch (indice)
            {
                case 0: return COMIDA_INT_BURGERSHOT_X;
                case 1: return COMIDA_INT_BURGERSHOT_Y;
                case 2: return COMIDA_INT_BURGERSHOT_Z;
                case 3: return COMIDA_INT_BURGERSHOT_A;
            }
        }
        case COMIDA_CADENA_CLUCKINBELL:
        {
            switch (indice)
            {
                case 0: return COMIDA_INT_CLUCKINBELL_X;
                case 1: return COMIDA_INT_CLUCKINBELL_Y;
                case 2: return COMIDA_INT_CLUCKINBELL_Z;
                case 3: return COMIDA_INT_CLUCKINBELL_A;
            }
        }
        case COMIDA_CADENA_PIZZA:
        {
            switch (indice)
            {
                case 0: return COMIDA_INT_PIZZA_X;
                case 1: return COMIDA_INT_PIZZA_Y;
                case 2: return COMIDA_INT_PIZZA_Z;
                case 3: return COMIDA_INT_PIZZA_A;
            }
        }
    }
    return 0.0;
}

// Cadena en la que esta AHORA MISMO el jugador (dentro del interior), o
// -1 si no esta en ninguna. Reescrita 07/09/2026 sobre
// Comida_SucursalDe -- ya no compara contra g_ComidaMundo[cadena]
// (compartido), sino contra el mundo PROPIO de cada sucursal real.
stock Comida_CadenaDe(playerid)
{
    new sucursal = Comida_SucursalDe(playerid);
    if (sucursal == -1) return -1;
    return g_SucursalesComida[sucursal][scCadena];
}

// ---------------------------------------------------------------------
//  ADMIN · /entrarcomida -- SOLO mientras no haya puertas exteriores
//  reales marcadas. Teleporta directo al mostrador de la cadena para
//  poder probar /comer, la caja fuerte, etc. sin esperar a manana. En
//  cuanto Comida_AnadirSucursal tenga filas de verdad, /entrar hace
//  esto mismo sin hacer falta admin ni comando aparte -- este se puede
//  retirar entonces si se prefiere, no rompe nada dejarlo.
// ---------------------------------------------------------------------
CMD:entrarcomida(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new cadena;
    if (sscanf(params, "d", cadena) || cadena < 0 || cadena >= COMIDA_TOTAL_CADENAS)
    {
        Uso(playerid, "/entrarcomida <0=Burger Shot, 1=Cluckin' Bell, 2=Pizza>");
        return 1;
    }

    // Mundo de la PRIMERA sucursal activa de esa cadena -- arreglado
    // 07/09/2026, ya no existe un mundo unico por cadena (g_ComidaMundo),
    // cada sucursal tiene el suyo propio (ver scMundo).
    new mundoPrueba = MUNDO_NORMAL;
    for (new i = 0; i < g_TotalSucursalesComida; i++)
    {
        if (g_SucursalesComida[i][scActiva] && g_SucursalesComida[i][scCadena] == cadena)
        {
            mundoPrueba = g_SucursalesComida[i][scMundo];
            break;
        }
    }

    SetPlayerPos(playerid, Comida_InteriorCoord(cadena, 0), Comida_InteriorCoord(cadena, 1), Comida_InteriorCoord(cadena, 2));
    SetPlayerFacingAngle(playerid, Comida_InteriorCoord(cadena, 3));
    SetPlayerInterior(playerid, g_ComidaInterior[cadena]);
    SetPlayerVirtualWorld(playerid, mundoPrueba);
    SetCameraBehindPlayer(playerid);

    format(g_Buffer, sizeof g_Buffer, "Dentro de %s (modo prueba). Escribe /comer.", g_ComidaNombreCadena[cadena]);
    Info(playerid, g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  /comer
// ---------------------------------------------------------------------
CMD:comer(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new cadena = Comida_CadenaDe(playerid);
    if (cadena == -1)
    {
        Error(playerid, "Aqui no hay nada que comer.");
        return 1;
    }

    g_ComidaCadenaAbierta[playerid] = cadena;
    Comida_MostrarMenu(playerid, cadena);
    return 1;
}

stock Comida_MostrarMenu(playerid, cadena)
{
    new lista[256], linea[64], precio[24];
    format(lista, sizeof lista, "Producto\tPrecio\n");

    for (new p = 0; p < COMIDA_PRODUCTOS_POR_CADENA; p++)
    {
        FormatearDinero(g_ComidaMenu[cadena][p][pcPrecio], precio, sizeof precio);
        format(linea, sizeof linea, "%s\t%s\n", g_ComidaMenu[cadena][p][pcNombre], precio);
        strcat(lista, linea, sizeof lista);
    }

    format(g_Buffer, sizeof g_Buffer, "{E8B33A}%s", g_ComidaNombreCadena[cadena]);
    ShowPlayerDialog(playerid, DIALOGO_COMIDA_MENU, DIALOG_STYLE_TABLIST_HEADERS, g_Buffer, lista, "Pedir", "Cerrar");
    return 1;
}

stock Comida_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext

    if (dialogid != DIALOGO_COMIDA_MENU) return 0;
    if (!response) return 1;

    new cadena = g_ComidaCadenaAbierta[playerid];
    if (cadena < 0 || cadena >= COMIDA_TOTAL_CADENAS) return 1;

    // Revalida que sigue dentro -- pudo salir con el dialogo abierto.
    if (Comida_CadenaDe(playerid) != cadena)
    {
        Error(playerid, "Ya no estas dentro de ese local.");
        return 1;
    }

    if (listitem < 0 || listitem >= COMIDA_PRODUCTOS_POR_CADENA) return 1;

    new precio = g_ComidaMenu[cadena][listitem][pcPrecio];
    if (!TieneDinero(playerid, precio))
    {
        Error(playerid, "No te llega el dinero para eso.");
        Comida_MostrarMenu(playerid, cadena);
        return 1;
    }

    DarDinero(playerid, -precio);

    new Float:vida;
    GetPlayerHealth(playerid, vida);
    vida += g_ComidaMenu[cadena][listitem][pcVida];
    if (vida > 100.0) vida = 100.0;
    SetPlayerHealth(playerid, vida);

    // La animacion se retrasa un poco -- ver Comida_AnimarRetardado mas
    // abajo, con la explicacion completa (bug real encontrado el
    // 07/09/2026: aplicarla en el mismo tick que se cierra el dialogo
    // no se veia).
    SetTimerEx("Comida_AnimarRetardado", COMIDA_ANIMACION_RETRASO_MS, false, "dd", playerid, cadena);
    g_ComidaProductoServido[playerid] = listitem;

    // El 100% del precio, a la caja fuerte de la franquicia -- no pasa
    // nada si no tiene dueno todavia, Negocio_Ingresar() ya lo ignora.
    Negocio_Ingresar(g_ComidaSlotNegocio[cadena], precio);

    // Hambre y sed (44_hambre_sed.inc): el pedido se consume en el acto
    // (no pasa por inventario), asi que se suma aqui directamente.
    new hsTexto[64];
    HambreSed_Sumar(playerid, g_ComidaMenu[cadena][listitem][pcHambre], g_ComidaMenu[cadena][listitem][pcSed], hsTexto, sizeof hsTexto);

    format(g_Buffer, sizeof g_Buffer, "Has pedido: %s.%s", g_ComidaMenu[cadena][listitem][pcNombre], hsTexto);
    Exito(playerid, g_Buffer);

    // El menu NO se cierra solo tras pedir (08/09/2026, peticion expresa
    // de Vlad) -- se reabre para que el jugador pueda pedir mas de una
    // cosa seguida; lo cierra el mismo con el boton "Cerrar" del dialogo
    // (dialogid == DIALOGO_COMIDA_MENU con response == 0, mas arriba).
    Comida_MostrarMenu(playerid, cadena);
    return 1;
}

forward Comida_AnimarRetardado(playerid, cadena);
public Comida_AnimarRetardado(playerid, cadena)
{
    if (!IsPlayerConnected(playerid)) return 0;
    // Revalida que sigue dentro de esa cadena -- pudo salir en los 300ms.
    if (Comida_CadenaDe(playerid) != cadena) return 0;

    Animar(playerid, "FOOD", g_ComidaAnimacion[cadena], 4.1, false, false, false, false, 0);

    new listitem = g_ComidaProductoServido[playerid];
    if (listitem < 0 || listitem >= COMIDA_PRODUCTOS_POR_CADENA) return 1;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[128];
    NombreRP(playerid, nombre, sizeof nombre);
    format(mensaje, sizeof mensaje, "* %s come %s.", nombre, g_ComidaMenu[cadena][listitem][pcNombre]);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
    return 1;
}

// ---------------------------------------------------------------------
//  INGRESO PASIVO -- una vez al dia real de uptime del servidor (ver
//  g_SegundosServidor % COMIDA_SEGUNDOS_DIA en TicSegundo, 99_callbacks.inc),
//  SUMADO al 100% de cada venta de /comer que ya entra por
//  Negocio_Ingresar mas arriba. Cada franquicia cobra por su cuenta,
//  solo si tiene dueno -- igual que el resto de negocios.
// ---------------------------------------------------------------------
stock Comida_IngresoDiario()
{
    for (new c = 0; c < COMIDA_TOTAL_CADENAS; c++)
    {
        Negocio_Ingresar(g_ComidaSlotNegocio[c], COMIDA_INGRESO_DIARIO);
    }
    return 1;
}
