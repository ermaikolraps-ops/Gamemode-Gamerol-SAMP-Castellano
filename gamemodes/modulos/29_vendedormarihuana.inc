/* =====================================================================
 *  29_vendedormarihuana.inc
 *  ---------------------------------------------------------------------
 *  Trabajo de Vendedor de Marihuana (ranura 3). Reparto a clientes NPC
 *  por toda la ciudad -- no tiene nada que ver con el cultivo libre ni
 *  con la venta entre jugadores de 28_marihuana.inc, que siguen igual.
 *
 *  Flujo:
 *    1. /aceptarjob en el punto de contratacion (MVEND_JOB_*, config en
 *       01_configuracion.inc): te haces Vendedor de Marihuana, igual que
 *       cualquier otro trabajo (reusa el /trabajo generico de
 *       11_trabajos.inc, no hay logica de contratacion propia aqui).
 *    2. /vendedor en la cabina telefonica (MVEND_CABINA_*): activa el
 *       modo vendedor. Cada 5 minutos, si sigues activo y no tienes
 *       ningun encargo pendiente, te llega un cliente nuevo.
 *    3. /aceptarventa: aceptas el encargo. Se genera un cliente -- un
 *       NPC de verdad (STREAMER_TAG_ACTOR), visible solo para ti -- en
 *       uno de los 10 puntos de la ciudad, con checkpoint y GPS. Tienes
 *       90 segundos para llegar desde que aceptas.
 *    4. Entrega automatica al pisar el checkpoint (MariaVendedor_OnEnterCheckpoint,
 *       enganchado en OnPlayerEnterCheckpoint de 99_callbacks.inc), o
 *       /entregarventa a mano si prefieres escribirlo.
 *
 *  El cliente se ancla a un unico jugador con el parametro playerid de
 *  CreateDynamicActor: nadie mas lo ve, y no hace falta gestionar varias
 *  entregas simultaneas chocando en el mismo punto.
 *
 *  Reputacion: contador en memoria, no persistido en base de datos (se
 *  reinicia al reconectar). A partir de cierto numero de entregas
 *  completadas en la sesion, los clientes pueden pedir mas cantidad.
 * ===================================================================== */

// Icono de mapa temporal (GPS del cliente). El slot 90 lo usa el taller
// de dropeo (22_dropeo.inc); este es el siguiente libre.
#define MVEND_ICON_SLOT          91

// Skin generico de civil para el cliente. Puramente cosmetico -- cambialo
// por el que prefieras (id 0-311) si quieres algo mas concreto.
#define MVEND_CLIENTE_SKIN       7

// Tramos de reputacion: mas entregas completadas en la sesion, mas
// grande puede ser la siguiente peticion.
#define MVEND_REP_TRAMO_1        5
#define MVEND_REP_TRAMO_2        15
#define MVEND_GRAMOS_MAX_REP1    120
#define MVEND_GRAMOS_MAX_REP2    150

enum E_MVEND_PUNTO
{
    Float:mpX, Float:mpY, Float:mpZ, Float:mpA,
    mpZona[32]
}

// 8 puntos de entrega, puestos a mano desde /panel > Coordenadas
// (claves "posvent1".."posvent8"). Nombres de zona cortos a proposito:
// los mensajes de chat con clientes 0.3.7 (allow_037_clients) pueden
// perderse sin avisar si se pasan de largo, asi que aqui no sobra nada.
static const g_MVendPuntos[][E_MVEND_PUNTO] =
{
    {872.396, -1350.33, 13.6964, 176.943, "el punto de entrega 1"},
    {204.577, -1657.23, 13.1235,  88.021, "el punto de entrega 2"},
    {555.374, -1765.12,  5.657,  88.021, "el punto de entrega 3"},
    {866.009, -1793.34, 13.7176,  88.021, "el punto de entrega 4"},
    {923.386,  -1364.9, 13.2679,  88.021, "el punto de entrega 5"},
    {520.246, -1278.25, 16.0673,  88.021, "el punto de entrega 6"},
    {1605.64, -1057.42, 23.7946,  88.021, "el punto de entrega 7"},
    {1966.85, -1157.51, 20.8428,  88.021, "el punto de entrega 8"}
};
#define MVEND_TOTAL_PUNTOS (sizeof g_MVendPuntos)

enum E_MVEND_JUGADOR
{
    bool:mvActivo,                 // modo vendedor encendido (telefono)
    mvEstado,                      // 0 nada, 1 oferta pendiente, 2 en camino
    mvPunto,                       // indice en g_MVendPuntos de la entrega actual
    mvGramos,
    mvPrecio,
    mvProximaOferta,               // gettime() a partir de aqui puede salir oferta nueva
    mvExpira,                      // gettime() limite para llegar (estado 2)
    mvVentasCompletadas,           // reputacion en memoria, no persistida
    STREAMER_TAG_ACTOR:mvActor
}
new g_MVend[MAX_PLAYERS][E_MVEND_JUGADOR];

static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaCabina;

// ---------------------------------------------------------------------
//  ARRANQUE
// ---------------------------------------------------------------------
stock MariaVendedor_Iniciar()
{
    g_EtiquetaCabina = CreateDynamic3DTextLabel(
        "CABINA DE VENTAS\n{FFFFFF}Escribe {53C168}/vendedor{FFFFFF} para activar o desactivar el telefono",
        0x53C168FF, MVEND_CABINA_X, MVEND_CABINA_Y, MVEND_CABINA_Z + 0.7, 10.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 25.0);


    print("  [VENDEDOR MARIHUANA] Sistema listo.");
    return 1;
}

public MariaVendedor_OnConnect(playerid)
{
    new vacio[E_MVEND_JUGADOR];
    g_MVend[playerid] = vacio;
    g_MVend[playerid][mvActor] = STREAMER_TAG_ACTOR:-1;
    return 1;
}

public MariaVendedor_OnDisconnect(playerid)
{
    MariaVendedor_Limpiar(playerid);
    return 1;
}

// Se llama desde OnPlayerDeath (99_callbacks.inc). Si tenia un encargo en
// marcha, se cancela -- el cliente no espera a nadie herido.
public MariaVendedor_OnDeath(playerid)
{
    if (g_MVend[playerid][mvEstado] == 0) return 1;

    MariaVendedor_Limpiar(playerid);
    g_MVend[playerid][mvEstado] = 0;
    g_MVend[playerid][mvProximaOferta] = gettime() + MVEND_OFERTA_SIGUIENTE;

    Aviso(playerid, "El cliente se ha esfumado mientras estabas herido. El encargo se ha cancelado.");
    return 1;
}

// Se llama desde Trabajo_Quitar (11_trabajos.inc) al dejar el trabajo o
// cambiarlo por otro.
public MariaVendedor_TrabajoRetirado(playerid)
{
    MariaVendedor_Limpiar(playerid);

    new vacio[E_MVEND_JUGADOR];
    g_MVend[playerid] = vacio;
    g_MVend[playerid][mvActor] = STREAMER_TAG_ACTOR:-1;
    return 1;
}

// Se llama desde Trabajo_Dialogo (11_trabajos.inc) justo al firmar.
public MariaVendedor_TrasContratar(playerid)
{
    Info(playerid, "Ve a la cabina telefonica y escribe /vendedor para empezar a recibir clientes.");
    return 1;
}

// ---------------------------------------------------------------------
//  LIMPIEZA: destruye el cliente NPC, el checkpoint y el icono de mapa.
//  No toca mvEstado ni mvProximaOferta -- eso lo decide quien la llama.
// ---------------------------------------------------------------------
stock MariaVendedor_Limpiar(playerid)
{
    if (IsValidDynamicActor(g_MVend[playerid][mvActor]))
    {
        DestroyDynamicActor(g_MVend[playerid][mvActor]);
    }
    g_MVend[playerid][mvActor] = STREAMER_TAG_ACTOR:-1;

    DisablePlayerCheckpoint(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  REPUTACION: rango maximo de gramos que puede pedir el proximo cliente.
// ---------------------------------------------------------------------
stock MariaVendedor_RangoMaximo(playerid)
{
    if (g_MVend[playerid][mvVentasCompletadas] >= MVEND_REP_TRAMO_2) return MVEND_GRAMOS_MAX_REP2;
    if (g_MVend[playerid][mvVentasCompletadas] >= MVEND_REP_TRAMO_1) return MVEND_GRAMOS_MAX_REP1;
    return MVEND_GRAMOS_MAX;
}

// ---------------------------------------------------------------------
//  GENERA UNA OFERTA NUEVA (estado 1: pendiente de aceptar)
// ---------------------------------------------------------------------
stock MariaVendedor_GenerarOferta(playerid)
{
    new maximo = MariaVendedor_RangoMaximo(playerid);

    g_MVend[playerid][mvEstado] = 1;
    g_MVend[playerid][mvPunto]  = random(MVEND_TOTAL_PUNTOS);
    g_MVend[playerid][mvGramos] = MVEND_GRAMOS_MIN + random(maximo - MVEND_GRAMOS_MIN + 1);
    g_MVend[playerid][mvPrecio] = g_MVend[playerid][mvGramos] * MVEND_PRECIO_GRAMO;

    // Mensaje corto a proposito -- ver nota en g_MVendPuntos sobre el
    // limite de longitud de los clientes 0.3.7.
    new gramos[16], dinero[24], mensaje[128];
    Maria_FormatearGramos(g_MVend[playerid][mvGramos], gramos, sizeof gramos);
    FormatearDinero(g_MVend[playerid][mvPrecio], dinero, sizeof dinero);

    format(mensaje, sizeof mensaje, "Venta de %s de marihuana por %s. Dirigete a %s y usa /aceptarventa.",
        gramos, dinero, g_MVendPuntos[g_MVend[playerid][mvPunto]][mpZona]);
    Info(playerid, mensaje);
    return 1;
}

// ---------------------------------------------------------------------
//  ENTREGA: usada tanto por /entregarventa como por el checkpoint
//  automatico. "automatico" solo silencia el aviso de distancia (el
//  checkpoint ya garantiza que estas encima).
// ---------------------------------------------------------------------
stock MariaVendedor_Entregar(playerid, bool:automatico)
{
    if (g_MVend[playerid][mvEstado] != 2) return 0;

    new punto = g_MVend[playerid][mvPunto];

    if (!EnPunto(playerid, 3.0, g_MVendPuntos[punto][mpX], g_MVendPuntos[punto][mpY], g_MVendPuntos[punto][mpZ]))
    {
        if (!automatico) Error(playerid, "Todavia no has llegado al punto de entrega.");
        return 0;
    }

    new item = Item_Buscar("Marihuana");
    new gramos = g_MVend[playerid][mvGramos];
    new precio = g_MVend[playerid][mvPrecio];

    if (Inv_TieneItem(playerid, item) < gramos)
    {
        new gramosTexto[16];
        Maria_FormatearGramos(gramos, gramosTexto, sizeof gramosTexto);
        Formato("Te faltan %s. Corre a por mas.", gramosTexto);
        Error(playerid, g_Buffer);
        return 0;
    }

    Inv_QuitarItem(playerid, item, gramos);
    DarDinero(playerid, precio);

    new anteriorMaximo = MariaVendedor_RangoMaximo(playerid);
    g_MVend[playerid][mvVentasCompletadas]++;

    // Contador persistente para el panel web (la reputacion de arriba
    // es solo de la sesion, esto es el historial de toda la cuenta).
    new consultaStat[100];
    format(consultaStat, sizeof consultaStat, "UPDATE cuentas SET maria_ventas_npc = maria_ventas_npc + 1 WHERE id = %d", Jugador[playerid][jID]);
    mysql_pquery(g_BD, consultaStat);

    MariaVendedor_Limpiar(playerid);
    g_MVend[playerid][mvEstado] = 0;
    g_MVend[playerid][mvProximaOferta] = gettime() + MVEND_OFERTA_SIGUIENTE;

    new dinero[24], nombre[MAX_PLAYER_NAME + 1], mensaje[128];
    FormatearDinero(precio, dinero, sizeof dinero);
    format(mensaje, sizeof mensaje, "Entrega hecha. Cobras %s.", dinero);
    Exito(playerid, mensaje);

    NombreRP(playerid, nombre, sizeof nombre);
    format(mensaje, sizeof mensaje, "* %s le pasa una bolsa a un tipo, que se la guarda y se aleja.", nombre);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

    if (MariaVendedor_RangoMaximo(playerid) > anteriorMaximo)
    {
        Exito(playerid, "Tu reputacion crece: los clientes pediran mas cantidad.");
    }
    return 1;
}

// Devuelve 1 si el checkpoint era nuestro (dispatcher de 99_callbacks.inc,
// mismo contrato que VendedorArmas_OnEnterCheckpoint). Se queda en 1
// aunque la entrega falle por falta de genero, para que el checkpoint no
// se borre y pueda reintentarlo si vuelve con mas.
stock MariaVendedor_OnEnterCheckpoint(playerid)
{
    if (g_MVend[playerid][mvEstado] != 2) return 0;
    MariaVendedor_Entregar(playerid, true);
    return 1;
}

// ---------------------------------------------------------------------
//  CHEQUEO POR SEGUNDO (llamado desde TicSegundo, 99_callbacks.inc)
// ---------------------------------------------------------------------
stock MariaVendedor_SegundoGlobal()
{
    new ahora = gettime();

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (!Trabajo_Tiene(i, TRABAJO_VENDEDOR_MARIHUANA)) continue;

        // Se le acabo el tiempo para llegar al cliente.
        if (g_MVend[i][mvEstado] == 2 && ahora > g_MVend[i][mvExpira])
        {
            MariaVendedor_Limpiar(i);
            g_MVend[i][mvEstado] = 0;
            g_MVend[i][mvProximaOferta] = ahora + MVEND_OFERTA_SIGUIENTE;
            Error(i, "Se te ha acabado el tiempo. El cliente se ha cansado de esperar y se ha ido.");
            continue;
        }

        // Toca oferta nueva.
        if (g_MVend[i][mvActivo] && g_MVend[i][mvEstado] == 0 && ahora >= g_MVend[i][mvProximaOferta])
        {
            MariaVendedor_GenerarOferta(i);
        }
    }
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDOS
// ---------------------------------------------------------------------
CMD:vendedor(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (!Trabajo_Tiene(playerid, TRABAJO_VENDEDOR_MARIHUANA))
    {
        Error(playerid, "No eres Vendedor de Marihuana.");
        return 1;
    }

    if (g_MVend[playerid][mvActivo])
    {
        g_MVend[playerid][mvActivo] = false;

        // Se cuelga con un encargo en marcha: se cancela.
        if (g_MVend[playerid][mvEstado] != 0)
        {
            MariaVendedor_Limpiar(playerid);
            g_MVend[playerid][mvEstado] = 0;
        }

        Info(playerid, "Has colgado el telefono. Ya no te llegaran clientes.");
        return 1;
    }

    if (!EnPunto(playerid, MVEND_CABINA_RADIO, MVEND_CABINA_X, MVEND_CABINA_Y, MVEND_CABINA_Z))
    {
        Error(playerid, "Tienes que estar en la cabina telefonica para activar el modo vendedor.");
        return 1;
    }

    g_MVend[playerid][mvActivo] = true;
    g_MVend[playerid][mvProximaOferta] = gettime() + MVEND_OFERTA_PRIMERA;

    Exito(playerid, "Has activado el telefono. En unos segundos te puede llegar el primer cliente.");
    return 1;
}

CMD:aceptarventa(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (!Trabajo_Tiene(playerid, TRABAJO_VENDEDOR_MARIHUANA))
    {
        Error(playerid, "No eres Vendedor de Marihuana.");
        return 1;
    }

    if (g_MVend[playerid][mvEstado] != 1)
    {
        Error(playerid, "No tienes ningun encargo pendiente ahora mismo.");
        return 1;
    }

    new punto = g_MVend[playerid][mvPunto];

    g_MVend[playerid][mvEstado] = 2;
    g_MVend[playerid][mvExpira] = gettime() + MVEND_OFERTA_TIEMPO;

    g_MVend[playerid][mvActor] = CreateDynamicActor(MVEND_CLIENTE_SKIN,
        g_MVendPuntos[punto][mpX], g_MVendPuntos[punto][mpY], g_MVendPuntos[punto][mpZ], g_MVendPuntos[punto][mpA],
        true, 100.0, 0, -1, playerid);

    SetPlayerCheckpoint(playerid, g_MVendPuntos[punto][mpX], g_MVendPuntos[punto][mpY], g_MVendPuntos[punto][mpZ], 3.0);

    new mensaje[128];
    format(mensaje, sizeof mensaje, "Encargo aceptado. Ve a %s -- %d segundos.",
        g_MVendPuntos[punto][mpZona], MVEND_OFERTA_TIEMPO);
    Exito(playerid, mensaje);
    return 1;
}

CMD:entregarventa(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (g_MVend[playerid][mvEstado] != 2)
    {
        Error(playerid, "No tienes ninguna entrega en marcha.");
        return 1;
    }

    MariaVendedor_Entregar(playerid, false);
    return 1;
}

CMD:cancelarventa(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (g_MVend[playerid][mvEstado] == 0)
    {
        Error(playerid, "No tienes ningun encargo que cancelar.");
        return 1;
    }

    MariaVendedor_Limpiar(playerid);
    g_MVend[playerid][mvEstado] = 0;
    g_MVend[playerid][mvProximaOferta] = gettime() + MVEND_OFERTA_SIGUIENTE;

    Info(playerid, "Has cancelado el encargo.");
    return 1;
}
