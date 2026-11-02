/* =====================================================================
 *  32_negocios.inc
 *  ---------------------------------------------------------------------
 *  10 ranuras fijas de negocio, igual que trabajos y facciones: el
 *  catalogo (nombre, posicion, precio de fabrica) vive aqui en codigo,
 *  el estado mutable (propietario, caja fuerte) se guarda en la tabla
 *  `negocios` (sql/migraciones/015_negocios.sql). El PRECIO real de
 *  compra puede llevar ademas un override puesto por un fundador con
 *  /cambiarprecionegocio (migracion 047, columna precio_override,
 *  08/09/2026) -- ver g_NegocioPrecio / Negocio_Precio() mas abajo, el
 *  precio de este catalogo pasa a ser solo el valor por defecto.
 *
 *  Rellenas: 0 (Gasolinera
 *  Los Santos), 1 (DGT), 2 (Bonos del Estado, los dos ultimos dentro
 *  del banco), 3-5 (franquicias de comida, 42_comida.inc), 6 (Motel
 *  Angel Pine, 07/09/2026, 54_motel.inc) y 7 (Tienda Angel Pine,
 *  07/09/2026, segunda sucursal de 27_tienda.inc); las 2 ranuras
 *  restantes son huecos libres para mas adelante.
 *
 *  Un jugador solo puede tener UN negocio a la vez.
 *
 *    /comprarnegocio   en el icono del dolar, si esta libre
 *    /soltarnegocio    lo dejas libre ahi mismo, recibes 50% de lo pagado
 *    /vendernegocio    oferta a otro jugador (igual que /vendermaria)
 *    /aceptarnegocio   cierra el trato de /vendernegocio
 *    /cajafuerte       en el icono, solo el dueno: mirar saldo / sacar
 *    /vercajafuerte    solo el dueno, desde cualquier sitio
 *    /cambiarprecionegocio  admin fundador, en el icono (08/09/2026)
 *
 *  Ingresos automaticos en tiempo real, cada uno mientras el negocio
 *  tenga dueno (si no, el dinero no va a ningun lado, igual que antes de
 *  que existiera cada negocio):
 *    - Gasolinera Los Santos: 60% de cada venta de 27_tienda.inc.
 *    - Bonos del Estado: el 40% restante de esa misma venta.
 *    - DGT: 100% de cada /comprarcoche (antes se perdia entero).
 *    - Bonos del Estado: 10% de PAYDAY_SUBVENCION en cada nomina de
 *      cualquier jugador (aparte, no le resta nada al jugador).
 *    - Bonos del Estado: lo que sobra de /nfsvendercoche despues del
 *      fijo del vendedor NFS (tambien se perdia entero antes).
 * ===================================================================== */

#define MAX_NEGOCIOS             10
// NEGOCIO_SLOT_TIENDA y NEGOCIO_SLOT_DGT se movieron a
// 01_configuracion.inc el 07/09/2026: 13_vehiculos.inc (modulo 13, antes
// que este) necesita usarlos para los petrolpumps nuevos y
// /arreglarcoche, y un #define (a diferencia de una funcion) tiene que
// estar definido ANTES textualmente, no admite forward.
#define NEGOCIO_SLOT_BONOS       2        // "Bonos del Estado"
#define NEGOCIO_SLOT_BURGERSHOT  3        // "Burger Shot" (42_comida.inc)
#define NEGOCIO_SLOT_CLUCKINBELL 4        // "Cluckin' Bell" (42_comida.inc)
#define NEGOCIO_SLOT_PIZZA       5        // "Well Stacked Pizza" (42_comida.inc)
#define NEGOCIO_SLOT_MOTEL       6        // "Motel Angel Pine" (54_motel.inc)
#define NEGOCIO_RADIO_ACCION     4.0      // distancia para comprar/soltar/caja fuerte
#define NEGOCIO_PICKUP_MODELO    1274     // "bigdollar", el mismo signo del dolar que ya usa el banco
// NEGOCIO_PORCENTAJE_* viven en 01_configuracion.inc: el texto de
// /ayuda (09_cuenta.inc) los necesita y ese archivo se incluye antes.

enum E_NEGOCIO_DEF
{
    neNombre[32],
    Float:neX, Float:neY, Float:neZ, Float:neA,
    neInterior, neMundo,
    nePrecio,
    bool:neActivo,        // false = ranura libre, todavia sin definir
    // Compra en la calle, caja fuerte dentro -- decision expresa de Vlad
    // el 03/09/2026 para las franquicias de comida (slots 3/4/5). Para
    // el resto de negocios (compra y caja fuerte en el mismo punto),
    // neCompraExterior queda en false y los cuatro campos siguientes sin
    // usar.
    bool:neCompraExterior,
    Float:neCompraX, Float:neCompraY, Float:neCompraZ, Float:neCompraA
}

// Las coordenadas reales salen de la tabla `coordenadas` (marcadas por
// el usuario con /marcarcoord: "posnegocio1", "posnegocio2",
// "posnegocio3"). Las 7 ranuras restantes estan vacias a proposito --
// para anadir un negocio nuevo, solo hay que rellenar una de estas filas.
//
// DGT y Bonos del Estado usan interior 3 / mundo 500 (MUNDO_BANCO) a
// mano porque esta tabla es "static const" (constante de compilacion) y
// no puede leer la variable g_BancoInterior en caliente. Si algun dia
// se mueve el banco a OTRO interior con /setbanco, hay que actualizar
// el "3" de estas dos filas a mano tambien.
static const g_NegociosDef[MAX_NEGOCIOS][E_NEGOCIO_DEF] =
{
    {"Gasolinera Los Santos", 1929.24, -1784.26, 13.5469, 91.0947,  0,   0, 120000, true, false, 0.0, 0.0, 0.0, 0.0},
    {"DGT",                    359.14,   163.063, 1008.38, 93.3976, 3, 500, 150000, true, false, 0.0, 0.0, 0.0, 0.0},
    {"Bonos del Estado",      358.701,   166.327, 1008.38, 91.8309, 3, 500, 200000, true, false, 0.0, 0.0, 0.0, 0.0},
    // Franquicias de comida rapida (42_comida.inc). SUPERSEDIDO el
    // 03/09/2026: ya NO hay dos puntos (compra fuera / caja fuerte
    // dentro) -- Vlad marco un icono UNICO en la calle para
    // /comprarnegocio y /cajafuerte a la vez (COMIDA_ICONO_*, ver
    // 01_configuracion.inc), igual que Gasolinera/DGT/Bonos. neActivo en
    // interior 0 / mundo 0 (exterior). El mostrador de dentro
    // (COMIDA_INT_*) sigue existiendo solo para /comer.
    {"Burger Shot",        COMIDA_ICONO_BURGERSHOT_X,  COMIDA_ICONO_BURGERSHOT_Y,  COMIDA_ICONO_BURGERSHOT_Z,  COMIDA_ICONO_BURGERSHOT_A,  0, 0, 200000, true,
        false, 0.0, 0.0, 0.0, 0.0},
    {"Cluckin' Bell",      COMIDA_ICONO_CLUCKINBELL_X, COMIDA_ICONO_CLUCKINBELL_Y, COMIDA_ICONO_CLUCKINBELL_Z, COMIDA_ICONO_CLUCKINBELL_A, 0, 0, 175000, true,
        false, 0.0, 0.0, 0.0, 0.0},
    {"Well Stacked Pizza", COMIDA_ICONO_PIZZA_X,       COMIDA_ICONO_PIZZA_Y,       COMIDA_ICONO_PIZZA_Z,       COMIDA_ICONO_PIZZA_A,       0, 0, 150000, true,
        false, 0.0, 0.0, 0.0, 0.0},
    // Motel Angel Pine -- 07/09/2026. Mismo punto para /comprarnegocio y
    // /cajafuerte, igual que Gasolinera/DGT/Bonos (neCompraExterior
    // false). Las dos habitaciones de alquiler (/motel) son un sistema
    // aparte, ver 54_motel.inc -- esta ranura solo cubre el negocio: caja
    // fuerte + ingreso pasivo (Motel_IngresoDiario) + el cobro de cada
    // /motel (Negocio_Ingresar, mismo patron que /repostar con la
    // Gasolinera Los Santos).
    {"Motel Angel Pine",   MOTEL_COMPRAR_X,            MOTEL_COMPRAR_Y,            MOTEL_COMPRAR_Z,            MOTEL_COMPRAR_A,            0, 0, MOTEL_PRECIO, true,
        false, 0.0, 0.0, 0.0, 0.0},
    // Tienda Angel Pine -- 07/09/2026. Segunda sucursal de la gasolinera
    // (27_tienda.inc, mismo interior/catalogo, mundo propio), pero negocio
    // APARTE con su propia caja fuerte -- ver Negocio_IngresoTienda, mas
    // abajo, que reparte cada venta 60/40 con Bonos del Estado igual que
    // la original.
    {"Tienda Angel Pine",  TIENDA_ANGELPINE_COMPRAR_X, TIENDA_ANGELPINE_COMPRAR_Y, TIENDA_ANGELPINE_COMPRAR_Z, TIENDA_ANGELPINE_COMPRAR_A, 0, 0, TIENDA_ANGELPINE_PRECIO, true,
        false, 0.0, 0.0, 0.0, 0.0},
    {"",                          0.0,      0.0,     0.0,     0.0, 0,   0,      0, false, false, 0.0, 0.0, 0.0, 0.0},
    {"",                          0.0,      0.0,     0.0,     0.0, 0,   0,      0, false, false, 0.0, 0.0, 0.0, 0.0}
};

// Estado mutable, cargado de la tabla `negocios` al arrancar.
new g_NegocioPropietario[MAX_NEGOCIOS];                          // cuentas.id, 0 = libre
new g_NegocioPropietarioNombre[MAX_NEGOCIOS][MAX_PLAYER_NAME + 1];
new g_NegocioCajaFuerte[MAX_NEGOCIOS];

// Precio EFECTIVO de cada ranura (08/09/2026, /cambiarprecionegocio) --
// arranca igual que g_NegociosDef[slot][nePrecio] (el catalogo de codigo,
// que sigue siendo "static const", no se puede tocar en caliente) y
// AlCargarNegocios lo pisa si la fila trae un precio_override en la
// tabla `negocios`. Todo el codigo que necesite el precio REAL de compra
// de una ranura tiene que leer esto, nunca el nePrecio del catalogo
// directamente -- ver Negocio_Precio() mas abajo.
new g_NegocioPrecio[MAX_NEGOCIOS];
new STREAMER_TAG_PICKUP:g_NegocioPickup[MAX_NEGOCIOS];
new STREAMER_TAG_3D_TEXT_LABEL:g_NegocioEtiqueta[MAX_NEGOCIOS] = {STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID, ...};

// Segundo icono/etiqueta, SOLO para las ranuras con neCompraExterior true
// (hoy, las tres franquicias de comida): el pickup de /comprarnegocio en
// la calle, aparte del que ya existe en g_NegocioPickup para la caja
// fuerte dentro del interior.
new STREAMER_TAG_PICKUP:g_NegocioPickupCompra[MAX_NEGOCIOS];
new STREAMER_TAG_3D_TEXT_LABEL:g_NegocioEtiquetaCompra[MAX_NEGOCIOS] = {STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID, ...};

// Oferta pendiente de /vendernegocio, guardada en quien la RECIBE (mismo
// patron que /vendermaria en 28_marihuana.inc). Slot -1 = sin oferta.
static g_NegocioOfertaDe[MAX_PLAYERS];
static g_NegocioOfertaSlot[MAX_PLAYERS];
static g_NegocioOfertaPrecio[MAX_PLAYERS];

// Que ranura eligio el jugador, entre el comando y la respuesta al
// dialogo (compra, caja fuerte).
static g_NegocioSeleccionado[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  ARRANQUE Y CARGA
// ---------------------------------------------------------------------
stock Negocio_Iniciar()
{
    for (new i = 0; i < MAX_NEGOCIOS; i++)
    {
        if (!g_NegociosDef[i][neActivo]) continue;

        // Valor por defecto -- AlCargarNegocios lo sobreescribe si la
        // fila de la base de datos trae un precio_override.
        g_NegocioPrecio[i] = g_NegociosDef[i][nePrecio];

        g_NegocioPickup[i] = CreateDynamicPickup(NEGOCIO_PICKUP_MODELO, 23,
            g_NegociosDef[i][neX], g_NegociosDef[i][neY], g_NegociosDef[i][neZ],
            g_NegociosDef[i][neMundo], g_NegociosDef[i][neInterior], -1, 100.0);

        g_NegocioEtiqueta[i] = CreateDynamic3DTextLabel(" ", 0xFFFFFFFF,
            g_NegociosDef[i][neX], g_NegociosDef[i][neY], g_NegociosDef[i][neZ] + 1.0, 12.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, g_NegociosDef[i][neMundo], g_NegociosDef[i][neInterior], -1, 30.0);

        // Icono de compra aparte, en la calle -- solo si esta ranura lo pide.
        if (g_NegociosDef[i][neCompraExterior])
        {
            g_NegocioPickupCompra[i] = CreateDynamicPickup(NEGOCIO_PICKUP_MODELO, 23,
                g_NegociosDef[i][neCompraX], g_NegociosDef[i][neCompraY], g_NegociosDef[i][neCompraZ],
                0, 0, -1, 100.0);

            g_NegocioEtiquetaCompra[i] = CreateDynamic3DTextLabel(" ", 0xFFFFFFFF,
                g_NegociosDef[i][neCompraX], g_NegociosDef[i][neCompraY], g_NegociosDef[i][neCompraZ] + 1.0, 12.0,
                INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);
        }
    }

    print("  [NEGOCIOS] Sistema listo.");
    return 1;
}

stock Negocio_CargarTodas()
{
    // COALESCE a 0 -- mismo patron que vehiculos.dropeo_expira/federico_until
    // (13_vehiculos.inc) para columnas NULL: se resuelve en SQL, no hace
    // falta comprobar NULL aparte en Pawn.
    mysql_pquery(g_BD, "SELECT *, COALESCE(precio_override, 0) AS precio_efectiva FROM negocios ORDER BY slot ASC", "AlCargarNegocios");
    return 1;
}

forward AlCargarNegocios();
public AlCargarNegocios()
{
    new filas;
    cache_get_row_count(filas);

    for (new i = 0; i < filas; i++)
    {
        new slot;
        cache_get_value_name_int(i, "slot", slot);
        if (slot < 0 || slot >= MAX_NEGOCIOS) continue;

        cache_get_value_name_int(i, "propietario", g_NegocioPropietario[slot]);
        cache_get_value_name(i, "propietario_nombre", g_NegocioPropietarioNombre[slot], MAX_PLAYER_NAME + 1);
        cache_get_value_name_int(i, "caja_fuerte", g_NegocioCajaFuerte[slot]);

        // 0 quiere decir que nadie ha usado /cambiarprecionegocio en esta
        // ranura todavia -- se queda el precio de catalogo puesto por
        // Negocio_Iniciar().
        new precioOverride;
        cache_get_value_name_int(i, "precio_efectiva", precioOverride);
        if (precioOverride > 0) g_NegocioPrecio[slot] = precioOverride;

        Negocio_ActualizarEtiqueta(slot);
    }

    printf("  [NEGOCIOS] %d ranuras cargadas desde la base de datos.", filas);
    return 1;
}

public Negocio_OnConectar(playerid)
{
    g_NegocioOfertaSlot[playerid] = -1;
    return 1;
}

// ---------------------------------------------------------------------
//  CONSULTAS
// ---------------------------------------------------------------------
stock Negocio_SlotCercano(playerid)
{
    for (new i = 0; i < MAX_NEGOCIOS; i++)
    {
        if (!g_NegociosDef[i][neActivo]) continue;

        // El interior y el mundo tienen que coincidir tambien: varios
        // interiores de GTA:SA comparten las mismas coordenadas en bruto
        // (es como el banco distingue su mostrador de cualquier otro
        // interior parecido), no basta con la distancia.
        if (GetPlayerInterior(playerid) != g_NegociosDef[i][neInterior]) continue;
        if (GetPlayerVirtualWorld(playerid) != g_NegociosDef[i][neMundo]) continue;
        if (EnPunto(playerid, NEGOCIO_RADIO_ACCION, g_NegociosDef[i][neX], g_NegociosDef[i][neY], g_NegociosDef[i][neZ])) return i;
    }
    return -1;
}

// Igual que Negocio_SlotCercano, pero para /comprarnegocio: si la ranura
// tiene neCompraExterior, comprueba el punto de la calle (interior 0,
// mundo 0) en vez del punto de siempre. Para el resto de negocios
// (neCompraExterior false) el resultado es identico a Negocio_SlotCercano
// -- cero cambio de comportamiento para Gasolinera/DGT/Bonos.
stock Negocio_SlotCercanoCompra(playerid)
{
    for (new i = 0; i < MAX_NEGOCIOS; i++)
    {
        if (!g_NegociosDef[i][neActivo]) continue;

        if (g_NegociosDef[i][neCompraExterior])
        {
            if (GetPlayerInterior(playerid) != 0) continue;
            if (GetPlayerVirtualWorld(playerid) != 0) continue;
            if (EnPunto(playerid, NEGOCIO_RADIO_ACCION, g_NegociosDef[i][neCompraX], g_NegociosDef[i][neCompraY], g_NegociosDef[i][neCompraZ])) return i;
        }
        else
        {
            if (GetPlayerInterior(playerid) != g_NegociosDef[i][neInterior]) continue;
            if (GetPlayerVirtualWorld(playerid) != g_NegociosDef[i][neMundo]) continue;
            if (EnPunto(playerid, NEGOCIO_RADIO_ACCION, g_NegociosDef[i][neX], g_NegociosDef[i][neY], g_NegociosDef[i][neZ])) return i;
        }
    }
    return -1;
}

// Precio REAL de compra de una ranura ahora mismo -- el de catalogo,
// salvo que un fundador lo haya cambiado con /cambiarprecionegocio.
// Todo el codigo que cobre o muestre el precio de comprar un negocio
// tiene que pasar por aqui, nunca por g_NegociosDef[slot][nePrecio]
// directo (ese es solo el valor de fabrica, de solo lectura).
stock Negocio_Precio(slot)
{
    if (slot < 0 || slot >= MAX_NEGOCIOS) return 0;
    return g_NegocioPrecio[slot];
}

stock Negocio_DeJugador(playerid)
{
    for (new i = 0; i < MAX_NEGOCIOS; i++)
    {
        if (!g_NegociosDef[i][neActivo]) continue;
        if (g_NegocioPropietario[i] != 0 && g_NegocioPropietario[i] == Jugador[playerid][jID]) return i;
    }
    return -1;
}

// Cuantos negocios (de los activos) siguen sin dueno. Usado por el
// contador en pantalla de 30_gps.inc, junto al de coches y casas.
stock Negocio_ContarLibres()
{
    new total = 0;
    for (new i = 0; i < MAX_NEGOCIOS; i++)
    {
        if (!g_NegociosDef[i][neActivo]) continue;
        if (g_NegocioPropietario[i] == 0) total++;
    }
    return total;
}

stock Negocio_ActualizarEtiqueta(slot)
{
    if (slot < 0 || slot >= MAX_NEGOCIOS || !g_NegociosDef[slot][neActivo]) return 0;
    if (g_NegocioEtiqueta[slot] == STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID) return 0;

    new texto[160];

    if (g_NegocioPropietario[slot] == 0)
    {
        new precio[24];
        FormatearDinero(Negocio_Precio(slot), precio, sizeof precio);
        format(texto, sizeof texto, "{E8B33A}%s\n{32FF32}LIBRE\n{FFFFFF}%s\n{B4B5B7}/comprarnegocio",
            g_NegociosDef[slot][neNombre], precio);
    }
    else
    {
        new dueno[MAX_PLAYER_NAME + 1];
        format(dueno, sizeof dueno, "%s", g_NegocioPropietarioNombre[slot]);
        SubrayadoAEspacio(dueno);

        format(texto, sizeof texto, "{E8B33A}%s\n{FFFF00}Propietario: %s",
            g_NegociosDef[slot][neNombre], dueno);
    }

    UpdateDynamic3DTextLabelText(g_NegocioEtiqueta[slot], 0xFFFFFFFF, texto);

    // Mismo texto en el cartel de la calle, si esta ranura tiene uno.
    if (g_NegociosDef[slot][neCompraExterior] && g_NegocioEtiquetaCompra[slot] != STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID)
    {
        UpdateDynamic3DTextLabelText(g_NegocioEtiquetaCompra[slot], 0xFFFFFFFF, texto);
    }
    return 1;
}

// Llamada desde 27_tienda.inc en cada venta con exito. Si la gasolinera
// (ranura NEGOCIO_SLOT_TIENDA) tiene dueno, el 60% del precio pagado va
// a su caja fuerte al instante -- si esta libre, el dinero no va a
// ningun lado, igual que antes de que existiera este sistema.
// Suma a la caja fuerte de un negocio (si tiene dueno) y lo persiste.
// Comun a los cuatro ingresos automaticos de mas abajo.
stock Negocio_Ingresar(slot, cantidad)
{
    if (g_NegocioPropietario[slot] == 0) return 0;
    if (cantidad <= 0) return 0;

    g_NegocioCajaFuerte[slot] += cantidad;

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer, "UPDATE negocios SET caja_fuerte = %d WHERE slot = %d",
        g_NegocioCajaFuerte[slot], slot);
    mysql_pquery(g_BD, g_Buffer);
    return 1;
}

// Llamada desde 27_tienda.inc en cada venta con exito, en CUALQUIERA de
// las dos sucursales. El 100% del precio se reparte entero entre la caja
// fuerte de ESA sucursal (slotTienda -- NEGOCIO_SLOT_TIENDA o
// NEGOCIO_SLOT_TIENDA_ANGELPINE, 07/09/2026) y Bonos del Estado: 60%/40%,
// cada uno solo si tiene dueno, independientemente del otro.
stock Negocio_IngresoTienda(precio, slotTienda)
{
    new ingresoTienda = (precio * NEGOCIO_PORCENTAJE_CAJA) / 100;
    new ingresoEstado = precio - ingresoTienda;

    Negocio_Ingresar(slotTienda, ingresoTienda);
    Negocio_Ingresar(NEGOCIO_SLOT_BONOS, ingresoEstado);
    return 1;
}

// Llamada desde 22_dropeo.inc en cada /comprarcoche con exito. Antes
// ese dinero se perdia entero; ahora el 100% va a la caja fuerte de la
// DGT si tiene dueno.
stock Negocio_IngresoComprarCoche(precio)
{
    Negocio_Ingresar(NEGOCIO_SLOT_DGT, precio);
    return 1;
}

// Llamada desde Payday_Entregar (17_payday.inc) para CADA jugador que
// cobra nomina. Aparte del sueldo del jugador -- no le resta nada --,
// Bonos del Estado se lleva un %10 de PAYDAY_SUBVENCION si tiene dueno.
stock Negocio_IngresoNomina()
{
    new ingreso = (PAYDAY_SUBVENCION * NEGOCIO_PORCENTAJE_NOMINA) / 100;
    Negocio_Ingresar(NEGOCIO_SLOT_BONOS, ingreso);
    return 1;
}

// Llamada desde Payday_Entregar (17_payday.inc) con el "impuesto de
// vehiculos" que SI se le resta al jugador (NEGOCIO_IMPUESTO_VEHICULO
// por cada coche a su nombre). Va integro a Bonos del Estado.
stock Negocio_IngresoImpuestoVehiculos(cantidad)
{
    Negocio_Ingresar(NEGOCIO_SLOT_BONOS, cantidad);
    return 1;
}

// Llamada desde /aceptarnfs (26_nfs.inc) con lo que sobra del precio de
// venta despues del fijo del vendedor NFS (NFS_VENTA_PAGO_NFS). Antes
// ese sobrante se perdia entero; ahora va a Bonos del Estado si tiene
// dueno, sin tocar lo que ya cobra el vendedor.
stock Negocio_IngresoNFS(remanente)
{
    Negocio_Ingresar(NEGOCIO_SLOT_BONOS, remanente);
    return 1;
}

// Llamada desde 37_policia.inc cada vez que alguien paga una multa.
// El 100% va a Bonos del Estado si tiene dueño.
//
// Por que a Bonos y no a la caja de la Policia: una multa es dinero
// publico, no del agente que la pone. Asi ademas la recaudacion la
// disfruta OTRO jugador (el dueño de Bonos) en vez de desaparecer, que
// era justo lo que pedia ECONOMIA.md -- un sumidero que no sea un
// agujero negro.
// Posicion del mostrador de un negocio. Existe porque g_NegociosDef es
// `static` y no se ve desde otros modulos: 37_policia.inc la necesita
// para saber si estas en la DGT al sacarte el carnet.
stock Negocio_Posicion(slot, &Float:x, &Float:y, &Float:z, &interior, &mundo)
{
    if (slot < 0 || slot >= MAX_NEGOCIOS) return 0;
    x = g_NegociosDef[slot][neX];
    y = g_NegociosDef[slot][neY];
    z = g_NegociosDef[slot][neZ];
    interior = g_NegociosDef[slot][neInterior];
    mundo = g_NegociosDef[slot][neMundo];
    return 1;
}

stock Negocio_IngresoMulta(cantidad)
{
    Negocio_Ingresar(NEGOCIO_SLOT_BONOS, cantidad);
    return 1;
}

// ---------------------------------------------------------------------
//  COMPRAR
// ---------------------------------------------------------------------
CMD:comprarnegocio(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new slot = Negocio_SlotCercanoCompra(playerid);
    if (slot == -1)
    {
        Error(playerid, "Aqui no hay ningun negocio.");
        return 1;
    }

    if (g_NegocioPropietario[slot] != 0)
    {
        Error(playerid, "Este negocio ya tiene dueno.");
        return 1;
    }

    if (Negocio_DeJugador(playerid) != -1)
    {
        Error(playerid, "Solo puedes tener un negocio a la vez.");
        return 1;
    }

    if (!TieneDinero(playerid, Negocio_Precio(slot)))
    {
        Error(playerid, "No tienes suficiente dinero para comprar este negocio.");
        return 1;
    }

    g_NegocioSeleccionado[playerid] = slot;

    new precio[24], cuerpo[300];
    FormatearDinero(Negocio_Precio(slot), precio, sizeof precio);
    format(cuerpo, sizeof cuerpo, "{E8B33A}%s\n\n{B4B5B7}Precio: {3D9970}%s\n\n{FFFFFF}Confirmas la compra?",
        g_NegociosDef[slot][neNombre], precio);

    ShowPlayerDialog(playerid, DIALOGO_NEGOCIO_COMPRAR_CONFIRMAR, DIALOG_STYLE_MSGBOX,
        "{53C168}Comprar negocio", cuerpo, "Comprar", "Cancelar");
    return 1;
}

stock Negocio_Comprar(playerid, slot)
{
    if (slot < 0 || slot >= MAX_NEGOCIOS || !g_NegociosDef[slot][neActivo]) return 0;

    // Se revalida todo: pudo cambiar mientras el dialogo estaba abierto.
    if (g_NegocioPropietario[slot] != 0) { Error(playerid, "Este negocio ya tiene dueno."); return 0; }
    if (Negocio_DeJugador(playerid) != -1) { Error(playerid, "Solo puedes tener un negocio a la vez."); return 0; }

    new precio = Negocio_Precio(slot);
    if (!TieneDinero(playerid, precio)) { Error(playerid, "No tienes suficiente dinero para comprar este negocio."); return 0; }

    DarDinero(playerid, -precio);

    g_NegocioPropietario[slot] = Jugador[playerid][jID];
    NombreRP(playerid, g_NegocioPropietarioNombre[slot], MAX_PLAYER_NAME + 1);

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "UPDATE negocios SET propietario = %d, propietario_nombre = '%e', fecha_compra = NOW() WHERE slot = %d",
        g_NegocioPropietario[slot], g_NegocioPropietarioNombre[slot], slot);
    mysql_pquery(g_BD, g_Buffer);

    Negocio_ActualizarEtiqueta(slot);

    new nombreJugador[MAX_PLAYER_NAME + 1], mensaje[160];
    NombreRP(playerid, nombreJugador, sizeof nombreJugador);
    format(mensaje, sizeof mensaje, "* %s firma los papeles y se queda con %s.", nombreJugador, g_NegociosDef[slot][neNombre]);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

    Exito(playerid, "Negocio comprado. Usa /cajafuerte junto al icono del dolar para gestionarlo.");
    return 1;
}

// ---------------------------------------------------------------------
//  SOLTAR
// ---------------------------------------------------------------------
CMD:soltarnegocio(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new slot = Negocio_DeJugador(playerid);
    if (slot == -1)
    {
        Error(playerid, "No tienes ningun negocio.");
        return 1;
    }

    if (GetPlayerInterior(playerid) != g_NegociosDef[slot][neInterior]
        || GetPlayerVirtualWorld(playerid) != g_NegociosDef[slot][neMundo]
        || !EnPunto(playerid, NEGOCIO_RADIO_ACCION, g_NegociosDef[slot][neX], g_NegociosDef[slot][neY], g_NegociosDef[slot][neZ]))
    {
        Error(playerid, "Tienes que estar en tu negocio para soltarlo.");
        return 1;
    }

    new reembolso = (Negocio_Precio(slot) * NEGOCIO_PORCENTAJE_SOLTAR) / 100;
    new cajaPerdida = g_NegocioCajaFuerte[slot];

    DarDinero(playerid, reembolso);

    g_NegocioPropietario[slot] = 0;
    g_NegocioPropietarioNombre[slot][0] = EOS;
    g_NegocioCajaFuerte[slot] = 0;

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "UPDATE negocios SET propietario = 0, propietario_nombre = '', caja_fuerte = 0, fecha_compra = NULL WHERE slot = %d", slot);
    mysql_pquery(g_BD, g_Buffer);

    Negocio_ActualizarEtiqueta(slot);

    new dinero[24], mensaje[192];
    FormatearDinero(reembolso, dinero, sizeof dinero);

    if (cajaPerdida > 0)
    {
        new cajaTexto[24];
        FormatearDinero(cajaPerdida, cajaTexto, sizeof cajaTexto);
        format(mensaje, sizeof mensaje, "Has soltado %s. Te devuelven %s. La caja fuerte tenia %s y se ha perdido.",
            g_NegociosDef[slot][neNombre], dinero, cajaTexto);
    }
    else
    {
        format(mensaje, sizeof mensaje, "Has soltado %s. Te devuelven %s.", g_NegociosDef[slot][neNombre], dinero);
    }
    Exito(playerid, mensaje);
    return 1;
}

// ---------------------------------------------------------------------
//  VENTA JUGADOR A JUGADOR: /vendernegocio + /aceptarnegocio
//  Mismo patron de "oferta pendiente" que /vendermaria en
//  28_marihuana.inc. La caja fuerte NO se vacia al vender -- se vende
//  el negocio "con caja y todo", a diferencia de /soltarnegocio.
// ---------------------------------------------------------------------
CMD:vendernegocio(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new slot = Negocio_DeJugador(playerid);
    if (slot == -1)
    {
        Error(playerid, "No tienes ningun negocio que vender.");
        return 1;
    }

    new objetivoTexto[MAX_PLAYER_NAME + 1], precio;
    if (sscanf(params, "s[24]d", objetivoTexto, precio))
    {
        Uso(playerid, "/vendernegocio [jugador] [precio]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (objetivo == INVALID_PLAYER_ID || objetivo == playerid)
    {
        Error(playerid, "Ese jugador no esta conectado.");
        return 1;
    }

    if (!EstaCerca(playerid, objetivo, ARMAS_OFERTA_RADIO))
    {
        Error(playerid, "Tienes que estar cerca de esa persona para ofrecerle nada.");
        return 1;
    }

    if (precio < 0)
    {
        Error(playerid, "El precio no puede ser negativo.");
        return 1;
    }

    g_NegocioOfertaDe[objetivo] = playerid;
    g_NegocioOfertaSlot[objetivo] = slot;
    g_NegocioOfertaPrecio[objetivo] = precio;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[160], dinero[24];
    NombreRP(playerid, nombre, sizeof nombre);
    FormatearDinero(precio, dinero, sizeof dinero);

    format(mensaje, sizeof mensaje, "%s te ofrece su negocio (%s) por %s. Escribe /aceptarnegocio para cerrar el trato.",
        nombre, g_NegociosDef[slot][neNombre], dinero);
    Info(objetivo, mensaje);

    Exito(playerid, "Oferta enviada. Si la acepta, se cerrara el trato.");
    return 1;
}

CMD:aceptarnegocio(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (g_NegocioOfertaSlot[playerid] == -1)
    {
        Error(playerid, "No tienes ninguna oferta de negocio pendiente.");
        return 1;
    }

    new vendedor = g_NegocioOfertaDe[playerid];
    new slot = g_NegocioOfertaSlot[playerid];
    new precio = g_NegocioOfertaPrecio[playerid];

    if (!IsPlayerConnected(vendedor) || !EstaCerca(playerid, vendedor, ARMAS_OFERTA_RADIO))
    {
        Error(playerid, "El vendedor ya no esta cerca. La oferta se ha cancelado.");
        g_NegocioOfertaSlot[playerid] = -1;
        return 1;
    }

    if (g_NegocioPropietario[slot] != Jugador[vendedor][jID])
    {
        Error(playerid, "El vendedor ya no es el dueno de ese negocio.");
        g_NegocioOfertaSlot[playerid] = -1;
        return 1;
    }

    if (Negocio_DeJugador(playerid) != -1)
    {
        Error(playerid, "Ya tienes un negocio. Solo puedes tener uno.");
        g_NegocioOfertaSlot[playerid] = -1;
        return 1;
    }

    if (!TieneDinero(playerid, precio))
    {
        Error(playerid, "No tienes dinero suficiente para pagar.");
        return 1;
    }

    DarDinero(playerid, -precio);
    DarDinero(vendedor, precio);

    g_NegocioPropietario[slot] = Jugador[playerid][jID];
    NombreRP(playerid, g_NegocioPropietarioNombre[slot], MAX_PLAYER_NAME + 1);

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "UPDATE negocios SET propietario = %d, propietario_nombre = '%e', fecha_compra = NOW() WHERE slot = %d",
        g_NegocioPropietario[slot], g_NegocioPropietarioNombre[slot], slot);
    mysql_pquery(g_BD, g_Buffer);

    Negocio_ActualizarEtiqueta(slot);
    g_NegocioOfertaSlot[playerid] = -1;

    Exito(playerid, "Trato cerrado. El negocio ya es tuyo.");
    Exito(vendedor, "Trato cerrado, te han pagado.");
    return 1;
}

// ---------------------------------------------------------------------
//  CAJA FUERTE
// ---------------------------------------------------------------------
CMD:cajafuerte(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new slot = Negocio_SlotCercano(playerid);
    if (slot == -1)
    {
        Error(playerid, "Aqui no hay ningun negocio.");
        return 1;
    }

    if (g_NegocioPropietario[slot] != Jugador[playerid][jID])
    {
        Error(playerid, "Esta caja fuerte no es tuya.");
        return 1;
    }

    g_NegocioSeleccionado[playerid] = slot;

    ShowPlayerDialog(playerid, DIALOGO_NEGOCIO_CAJAFUERTE, DIALOG_STYLE_LIST, "{53C168}Caja fuerte",
        "{FFFFFF}Mirar saldo\n{FFFFFF}Sacar dinero", "Elegir", "Cerrar");
    return 1;
}

CMD:vercajafuerte(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new slot = Negocio_DeJugador(playerid);
    if (slot == -1)
    {
        Error(playerid, "No tienes ningun negocio.");
        return 1;
    }

    new dinero[24], mensaje[112];
    FormatearDinero(g_NegocioCajaFuerte[slot], dinero, sizeof dinero);
    format(mensaje, sizeof mensaje, "Caja fuerte de %s: %s.", g_NegociosDef[slot][neNombre], dinero);
    Info(playerid, mensaje);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /cambiarprecionegocio (08/09/2026) -- cambia el precio de
//  compra del negocio en el que estas parado ahora mismo (mismo punto
//  que /cajafuerte, Negocio_SlotCercano). Solo fundador: el precio de
//  catalogo esta pensado como un valor de equilibrio del servidor
//  entero, no algo que cualquier admin deba poder tocar sobre la marcha.
//  No hace falta ser el dueno ni que el negocio este libre.
// ---------------------------------------------------------------------
CMD:cambiarprecionegocio(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_FUNDADOR)) return 1;

    new slot = Negocio_SlotCercano(playerid);
    if (slot == -1)
    {
        Error(playerid, "Aqui no hay ningun negocio.");
        return 1;
    }

    new cantidad;
    if (sscanf(params, "d", cantidad) || cantidad <= 0)
    {
        Uso(playerid, "/cambiarprecionegocio <cantidad>");
        return 1;
    }

    g_NegocioPrecio[slot] = cantidad;

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "UPDATE negocios SET precio_override = %d WHERE slot = %d", cantidad, slot);
    mysql_pquery(g_BD, g_Buffer);

    Negocio_ActualizarEtiqueta(slot);

    new precio[24];
    FormatearDinero(cantidad, precio, sizeof precio);
    format(g_Buffer, sizeof g_Buffer, "Precio de %s cambiado a %s.", g_NegociosDef[slot][neNombre], precio);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "slot %d, nuevo precio %d", slot, cantidad);
    Admin_Registrar(playerid, "/cambiarprecionegocio", g_NegociosDef[slot][neNombre], g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  DIALOGOS
// ---------------------------------------------------------------------
stock Negocio_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    switch (dialogid)
    {
        case DIALOGO_NEGOCIO_COMPRAR_CONFIRMAR:
        {
            if (!response) return 1;
            Negocio_Comprar(playerid, g_NegocioSeleccionado[playerid]);
            return 1;
        }

        case DIALOGO_NEGOCIO_CAJAFUERTE:
        {
            if (!response) return 1;

            new slot = g_NegocioSeleccionado[playerid];
            if (slot < 0 || slot >= MAX_NEGOCIOS || g_NegocioPropietario[slot] != Jugador[playerid][jID]) return 1;

            if (listitem == 0)
            {
                new dinero[24], mensaje[96];
                FormatearDinero(g_NegocioCajaFuerte[slot], dinero, sizeof dinero);
                format(mensaje, sizeof mensaje, "Saldo de la caja fuerte: %s.", dinero);
                Info(playerid, mensaje);
                return 1;
            }

            // listitem == 1: sacar dinero.
            if (g_NegocioCajaFuerte[slot] <= 0)
            {
                Error(playerid, "La caja fuerte esta vacia.");
                return 1;
            }

            format(g_Buffer, sizeof g_Buffer, "Cuanto quieres sacar? (1 a %d)", g_NegocioCajaFuerte[slot]);
            ShowPlayerDialog(playerid, DIALOGO_NEGOCIO_CAJAFUERTE_SACAR, DIALOG_STYLE_INPUT,
                "{53C168}Sacar de la caja fuerte", g_Buffer, "Sacar", "Cancelar");
            return 1;
        }

        case DIALOGO_NEGOCIO_CAJAFUERTE_SACAR:
        {
            if (!response) return 1;

            new slot = g_NegocioSeleccionado[playerid];
            if (slot < 0 || slot >= MAX_NEGOCIOS || g_NegocioPropietario[slot] != Jugador[playerid][jID]) return 1;

            if (!EsNumeroEntero(inputtext) || strval(inputtext) < 1 || strval(inputtext) > g_NegocioCajaFuerte[slot])
            {
                Error(playerid, "Cantidad invalida.");
                return 1;
            }

            new cantidad = strval(inputtext);
            g_NegocioCajaFuerte[slot] -= cantidad;
            DarDinero(playerid, cantidad);

            mysql_format(g_BD, g_Buffer, sizeof g_Buffer, "UPDATE negocios SET caja_fuerte = %d WHERE slot = %d",
                g_NegocioCajaFuerte[slot], slot);
            mysql_pquery(g_BD, g_Buffer);

            new dinero[24], mensaje[96];
            FormatearDinero(cantidad, dinero, sizeof dinero);
            format(mensaje, sizeof mensaje, "Has sacado %s en efectivo de la caja fuerte.", dinero);
            Exito(playerid, mensaje);
            return 1;
        }
    }
    return 0;
}
