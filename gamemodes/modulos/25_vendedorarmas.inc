/* =====================================================================
 *  25_vendedorarmas.inc
 *  ---------------------------------------------------------------------
 *  Trabajo de Vendedor de Armas (ranura 1). Sistema copiado 1:1 de
 *  NG-RP (Next Generation Roleplay), traducido al castellano.
 *
 *  No hay tienda ni NPC: es fabricacion y venta entre jugadores.
 *
 *    1. /pedirmats en el punto de suministro (ARMAS_MATS_PEDIR_*):
 *       pagas 500$ al momento y te sale un checkpoint hacia la fabrica.
 *    2. Llegas a la fabrica (ARMAS_MATS_RECIBIR_*): los materiales se
 *       acreditan solos, como item normal ("Materiales", inventario).
 *    3. /venderarma [jugador] [arma]: gastas materiales y fabricas el
 *       arma. A ti mismo, al instante y a la mano (no pasa por el
 *       inventario). A otro jugador, le haces una oferta y con
 *       /aceptararma la cierra el.
 *
 *  El nivel de habilidad (0-1200+) desbloquea el catalogo, igual que en
 *  NGRP. Las armas de fuego dan +1 de nivel al venderlas; las de cuerpo
 *  a cuerpo no.
 * ===================================================================== */

enum E_ARMAS_JUGADOR
{
    aaSkill,
    aaVentasTotal,
    aaMatsRuns,
    bool:aaStatsCargadas,
    bool:aaEsperandoMats,       // true tras /pedirmats, hasta que llega a la fabrica
    aaUltimaVenta,              // GetTickCount de la ultima venta (cooldown)

    // Oferta de venta pendiente SOBRE este jugador (quien la recibe)
    aaOfertaArma,                // ID de arma GTA, 0 = sin oferta
    aaOfertaMats,
    aaOfertaPrecio,               // Lo que paga el comprador al vendedor
    aaOfertaDe,                  // playerid del vendedor
    bool:aaOfertaXP
}
new gArmas[MAX_PLAYERS][E_ARMAS_JUGADOR];

// ---------------------------------------------------------------------
//  CATALOGO  igual que en NGRP (armsdealer.pwn), traducido.
// ---------------------------------------------------------------------
enum E_ARMA_CATALOGO
{
    acComando[16],       // Lo que se escribe en /venderarma
    acNombre[24],        // Lo que se muestra
    acArma,               // ID de arma de GTA
    acMateriales,
    acSkillMinimo,
    acMunicion,
    bool:acDaXP,
    acPrecio              // Lo que paga el comprador cuando la venta es a otro jugador
}
static const g_CatalogoArmas[][E_ARMA_CATALOGO] =
{
    //  Comando         Nombre                 Arma  Materiales  Skill  Municion  XP     Precio
    {"flores",          "Flores",               14,        100,     0,        0, false,     100},
    {"punos",           "Punos americanos",      1,        100,     0,        0, false,     150},
    {"bate",            "Bate",                  5,        100,     0,        0, false,     150},
    {"baston",          "Baston",               15,        100,     0,        0, false,     150},
    {"pala",            "Pala",                  6,        100,     0,        0, false,     150},
    {"palo",            "Palo",                  2,        100,     0,        0, false,     150},
    {"taco",            "Taco de billar",        7,        100,     0,        0, false,     150},
    {"katana",          "Katana",                8,        100,     0,        0, false,     250},
    {"dildo",           "Dildo",                10,        100,     0,        0, false,     100},
    {"9mm",             "Pistola 9mm",          22,        500,     0,      250, true,     1500},
    {"sdpistol",        "Pistola silenciada",   23,       1000,     0,      250, true,     3000},
    {"escopeta",        "Escopeta",             25,       4000,    50,      100, true,    10000},
    {"mp5",             "MP5",                  29,       2500,   200,      250, true,     7000},
    {"rifle",           "Rifle",                33,       3000,   200,      150, true,     8000},
    {"tec9",            "Tec9",                 32,       3000,   700,      250, true,     8000},
    {"uzi",             "Uzi",                  28,       2500,   700,      250, true,     7000},
    {"deagle",          "Deagle",               24,       5000,  1200,      100, true,    13000},
    {"ak47",            "AK47",                 30,      10000,  1200,      150, true,    25000}
};
#define ARMAS_CATALOGO_TOTAL (sizeof g_CatalogoArmas)

static STREAMER_TAG_PICKUP:g_PickupArmasMats;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaArmasMats;
static STREAMER_TAG_PICKUP:g_PickupArmasRecibir;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaArmasRecibir;

// ---------------------------------------------------------------------
//  ARRANQUE
// ---------------------------------------------------------------------
public VendedorArmas_Iniciar()
{
    // Punto 2: pedir materiales.
    g_PickupArmasMats = CreateDynamicPickup(1239, 23,
        ARMAS_MATS_PEDIR_X, ARMAS_MATS_PEDIR_Y, ARMAS_MATS_PEDIR_Z, 0, 0, -1, 100.0);
    g_EtiquetaArmasMats = CreateDynamic3DTextLabel(
        "{E8973A}Suministro de materiales\n{FFFFFF}Escribe {E8B33A}/pedirmats{FFFFFF} para pedir un cargamento",
        0xE8973AFF, ARMAS_MATS_PEDIR_X, ARMAS_MATS_PEDIR_Y, ARMAS_MATS_PEDIR_Z + 0.6, 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    // Punto 3: fabrica, recepcion automatica.
    g_PickupArmasRecibir = CreateDynamicPickup(1239, 23,
        ARMAS_MATS_RECIBIR_X, ARMAS_MATS_RECIBIR_Y, ARMAS_MATS_RECIBIR_Z, 0, 0, -1, 100.0);
    g_EtiquetaArmasRecibir = CreateDynamic3DTextLabel(
        "{E8973A}Fabrica\n{FFFFFF}Trae aqui tu cargamento para recibir los materiales",
        0xE8973AFF, ARMAS_MATS_RECIBIR_X, ARMAS_MATS_RECIBIR_Y, ARMAS_MATS_RECIBIR_Z + 0.6, 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    return 1;
}

public VendedorArmas_OnConnect(playerid)
{
    new vacio[E_ARMAS_JUGADOR];
    gArmas[playerid] = vacio;
    return 1;
}

public VendedorArmas_OnSpawn(playerid)
{
    if (!gArmas[playerid][aaStatsCargadas] && Jugador[playerid][jID] != 0)
    {
        VendedorArmas_CargarStats(playerid);
    }
    return 1;
}

public VendedorArmas_OnDisconnect(playerid)
{
    VendedorArmas_GuardarStats(playerid);
    return 1;
}

public VendedorArmas_TrabajoRetirado(playerid)
{
    gArmas[playerid][aaEsperandoMats] = false;
    DisablePlayerCheckpoint(playerid);
    return 1;
}

// Justo al firmar el contrato: marca el primer viaje de materiales.
public VendedorArmas_TrasContratar(playerid)
{
    SetPlayerCheckpoint(playerid, ARMAS_MATS_PEDIR_X, ARMAS_MATS_PEDIR_Y, ARMAS_MATS_PEDIR_Z, ARMAS_MATS_RADIO_PEDIR);
    SendClientMessage(playerid, COLOR_ROJO_FUERTE, "Se te ha marcado un punto - Viaje de materiales.");
    return 1;
}

// ---------------------------------------------------------------------
//  ESTADISTICAS (armas_stats)
// ---------------------------------------------------------------------
stock VendedorArmas_CargarStats(playerid)
{
    if (Jugador[playerid][jID] == 0) return 0;

    new consulta[160];
    format(consulta, sizeof consulta,
        "SELECT * FROM armas_stats WHERE cuenta_id = %d LIMIT 1", Jugador[playerid][jID]);
    mysql_pquery(g_BD, consulta, "AlCargarStatsArmas", "d", playerid);
    return 1;
}

forward AlCargarStatsArmas(playerid);
public AlCargarStatsArmas(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        new consulta[128];
        format(consulta, sizeof consulta,
            "INSERT IGNORE INTO armas_stats (cuenta_id) VALUES (%d)", Jugador[playerid][jID]);
        mysql_pquery(g_BD, consulta);

        gArmas[playerid][aaSkill]        = 0;
        gArmas[playerid][aaVentasTotal]  = 0;
        gArmas[playerid][aaMatsRuns]     = 0;
        gArmas[playerid][aaStatsCargadas] = true;
        return 1;
    }

    cache_get_value_name_int(0, "skill",           gArmas[playerid][aaSkill]);
    cache_get_value_name_int(0, "ventas_total",    gArmas[playerid][aaVentasTotal]);
    cache_get_value_name_int(0, "materiales_runs", gArmas[playerid][aaMatsRuns]);
    gArmas[playerid][aaStatsCargadas] = true;
    return 1;
}

stock VendedorArmas_GuardarStats(playerid)
{
    if (!gArmas[playerid][aaStatsCargadas]) return 0;
    if (Jugador[playerid][jID] == 0)        return 0;

    new consulta[256];
    mysql_format(g_BD, consulta, sizeof consulta,
        "UPDATE armas_stats SET skill = %d, ventas_total = %d, materiales_runs = %d, ultima_venta = NOW() WHERE cuenta_id = %d",
        gArmas[playerid][aaSkill], gArmas[playerid][aaVentasTotal], gArmas[playerid][aaMatsRuns],
        Jugador[playerid][jID]);
    mysql_pquery(g_BD, consulta);
    return 1;
}

// ---------------------------------------------------------------------
//  MAT RUN
// ---------------------------------------------------------------------
CMD:pedirmats(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (!Trabajo_Tiene(playerid, TRABAJO_VENDEDOR_ARMAS))
    {
        Error(playerid, "No eres Vendedor de Armas.");
        return 1;
    }

    if (!EnPunto(playerid, ARMAS_MATS_RADIO_PEDIR, ARMAS_MATS_PEDIR_X, ARMAS_MATS_PEDIR_Y, ARMAS_MATS_PEDIR_Z))
    {
        Error(playerid, "No estas en el punto de suministro.");
        return 1;
    }

    if (gArmas[playerid][aaEsperandoMats])
    {
        Error(playerid, "Ya tienes un cargamento en camino. Llevalo a la fabrica.");
        return 1;
    }

    if (!TieneDinero(playerid, ARMAS_MATS_PRECIO))
    {
        Formato("No tienes los %d$ del cargamento de materiales.", ARMAS_MATS_PRECIO);
        Error(playerid, g_Buffer);
        return 1;
    }

    DarDinero(playerid, -ARMAS_MATS_PRECIO);
    gArmas[playerid][aaEsperandoMats] = true;

    Formato("Has pagado %d$ por %d materiales. Llevalos a la fabrica.", ARMAS_MATS_PRECIO, ARMAS_MATS_CANTIDAD);
    Exito(playerid, g_Buffer);

    SetPlayerCheckpoint(playerid, ARMAS_MATS_RECIBIR_X, ARMAS_MATS_RECIBIR_Y, ARMAS_MATS_RECIBIR_Z, ARMAS_MATS_RADIO_RECIBIR);
    return 1;
}

// Devuelve 1 si el checkpoint era nuestro (para el dispatcher de 99_callbacks).
stock VendedorArmas_OnEnterCheckpoint(playerid)
{
    if (!gArmas[playerid][aaEsperandoMats]) return 0;
    if (!EnPunto(playerid, ARMAS_MATS_RADIO_RECIBIR, ARMAS_MATS_RECIBIR_X, ARMAS_MATS_RECIBIR_Y, ARMAS_MATS_RECIBIR_Z)) return 0;

    if (Inv_DarItem(playerid, ARMAS_ITEM_MATERIALES, ARMAS_MATS_CANTIDAD, 0) != 1)
    {
        Error(playerid, "No te caben los materiales encima. Haz sitio y vuelve.");
        return 1;
    }

    gArmas[playerid][aaEsperandoMats] = false;
    gArmas[playerid][aaMatsRuns]++;
    DisablePlayerCheckpoint(playerid);

    Formato("La fabrica te ha dado %d materiales por tu cargamento.", ARMAS_MATS_CANTIDAD);
    Exito(playerid, g_Buffer);

    VendedorArmas_GuardarStats(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  VENTA DE ARMAS
// ---------------------------------------------------------------------
stock VendedorArmas_BuscarCatalogo(const comando[])
{
    for (new i = 0; i < ARMAS_CATALOGO_TOTAL; i++)
    {
        if (strcmp(comando, g_CatalogoArmas[i][acComando], true) == 0) return i;
    }
    return -1;
}

stock VendedorArmas_MostrarCatalogo(playerid)
{
    Linea(playerid, COLOR_MARCA, "Catalogo disponible a tu nivel:");
    for (new i = 0; i < ARMAS_CATALOGO_TOTAL; i++)
    {
        if (gArmas[playerid][aaSkill] < g_CatalogoArmas[i][acSkillMinimo]) continue;

        new numero[24], precio[24];
        FormatearNumero(g_CatalogoArmas[i][acMateriales], numero, sizeof numero);
        FormatearDinero(g_CatalogoArmas[i][acPrecio], precio, sizeof precio);
        format(g_Buffer, sizeof g_Buffer, "  {FFFFFF}%s{B4B5B7} - %s materiales - venta a otro jugador: %s ({E8B33A}/venderarma [jugador] %s{B4B5B7})",
            g_CatalogoArmas[i][acNombre], numero, precio, g_CatalogoArmas[i][acComando]);
        Linea(playerid, COLOR_GRIS, g_Buffer);
    }
    Uso(playerid, "/venderarma [jugador] [arma]");
    return 1;
}

CMD:venderarma(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (!Trabajo_Tiene(playerid, TRABAJO_VENDEDOR_ARMAS))
    {
        Error(playerid, "No eres Vendedor de Armas.");
        return 1;
    }

    new id, comando[16];
    if (sscanf(params, "us[16]", id, comando))
    {
        VendedorArmas_MostrarCatalogo(playerid);
        return 1;
    }

    if (!IsPlayerConnected(id))
    {
        Error(playerid, "Ese jugador no esta conectado.");
        return 1;
    }

    if ((GetTickCount() - gArmas[playerid][aaUltimaVenta]) < (ARMAS_VENTA_ESPERA * 1000))
    {
        Error(playerid, "Espera unos segundos antes de vender otra arma.");
        return 1;
    }

    new indice = VendedorArmas_BuscarCatalogo(comando);
    if (indice == -1)
    {
        Error(playerid, "Arma no valida. Escribe /venderarma para ver el catalogo.");
        return 1;
    }

    if (gArmas[playerid][aaSkill] < g_CatalogoArmas[indice][acSkillMinimo])
    {
        Formato("Necesitas nivel %d para fabricar eso.", g_CatalogoArmas[indice][acSkillMinimo]);
        Error(playerid, g_Buffer);
        return 1;
    }

    if (Inv_TieneItem(playerid, ARMAS_ITEM_MATERIALES) < g_CatalogoArmas[indice][acMateriales])
    {
        Error(playerid, "No tienes suficientes materiales.");
        return 1;
    }

    if (id != playerid && !EstaCerca(playerid, id, ARMAS_OFERTA_RADIO))
    {
        Error(playerid, "No estas cerca de ese jugador.");
        return 1;
    }

    if (id == playerid)
    {
        Inv_QuitarItem(playerid, ARMAS_ITEM_MATERIALES, g_CatalogoArmas[indice][acMateriales]);
        GivePlayerWeapon(playerid, g_CatalogoArmas[indice][acArma], g_CatalogoArmas[indice][acMunicion]);

        if (g_CatalogoArmas[indice][acDaXP])
        {
            gArmas[playerid][aaSkill]++;
            gArmas[playerid][aaVentasTotal]++;
        }
        VendedorArmas_GuardarStats(playerid);

        new nombre[MAX_PLAYER_NAME + 1], mensaje[144];
        NombreRP(playerid, nombre, sizeof nombre);
        format(mensaje, sizeof mensaje, "* %s fabrica %s con sus materiales.", nombre, g_CatalogoArmas[indice][acNombre]);
        MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

        Formato("Has fabricado: %s.", g_CatalogoArmas[indice][acNombre]);
        Exito(playerid, g_Buffer);
    }
    else
    {
        gArmas[id][aaOfertaArma]   = g_CatalogoArmas[indice][acArma];
        gArmas[id][aaOfertaMats]   = g_CatalogoArmas[indice][acMateriales];
        gArmas[id][aaOfertaPrecio] = g_CatalogoArmas[indice][acPrecio];
        gArmas[id][aaOfertaDe]     = playerid;
        gArmas[id][aaOfertaXP]     = g_CatalogoArmas[indice][acDaXP];

        new precioTexto[24];
        FormatearDinero(g_CatalogoArmas[indice][acPrecio], precioTexto, sizeof precioTexto);

        new nombreObjetivo[MAX_PLAYER_NAME + 1], mensaje[160];
        NombreRP(id, nombreObjetivo, sizeof nombreObjetivo);
        format(mensaje, sizeof mensaje, "Le has ofrecido %s a %s por %s.", g_CatalogoArmas[indice][acNombre], nombreObjetivo, precioTexto);
        Exito(playerid, mensaje);

        format(mensaje, sizeof mensaje, "Te ofrecen %s por %s. Escribe /aceptararma para comprarla.", g_CatalogoArmas[indice][acNombre], precioTexto);
        Info(id, mensaje);
    }

    gArmas[playerid][aaUltimaVenta] = GetTickCount();
    return 1;
}

CMD:aceptararma(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (gArmas[playerid][aaOfertaArma] == 0)
    {
        Error(playerid, "No tienes ninguna oferta de arma pendiente.");
        return 1;
    }

    new vendedor = gArmas[playerid][aaOfertaDe];

    if (!IsPlayerConnected(vendedor) || !EstaCerca(playerid, vendedor, ARMAS_OFERTA_RADIO))
    {
        Error(playerid, "El vendedor ya no esta cerca. La oferta se ha cancelado.");
        gArmas[playerid][aaOfertaArma] = 0;
        return 1;
    }

    if (Inv_TieneItem(vendedor, ARMAS_ITEM_MATERIALES) < gArmas[playerid][aaOfertaMats])
    {
        Error(playerid, "El vendedor ya no tiene suficientes materiales.");
        gArmas[playerid][aaOfertaArma] = 0;
        return 1;
    }

    new precio = gArmas[playerid][aaOfertaPrecio];
    if (!TieneDinero(playerid, precio))
    {
        Error(playerid, "No tienes dinero suficiente para comprar esa arma.");
        gArmas[playerid][aaOfertaArma] = 0;
        return 1;
    }

    Inv_QuitarItem(vendedor, ARMAS_ITEM_MATERIALES, gArmas[playerid][aaOfertaMats]);
    GivePlayerWeapon(playerid, gArmas[playerid][aaOfertaArma], 0);

    DarDinero(playerid, -precio);
    DarDinero(vendedor, precio);

    if (gArmas[playerid][aaOfertaXP])
    {
        gArmas[vendedor][aaSkill]++;
        gArmas[vendedor][aaVentasTotal]++;
        VendedorArmas_GuardarStats(vendedor);
    }

    new precioTexto[24];
    FormatearDinero(precio, precioTexto, sizeof precioTexto);

    new nombreComprador[MAX_PLAYER_NAME + 1], nombreVendedor[MAX_PLAYER_NAME + 1], mensaje[144];
    NombreRP(playerid, nombreComprador, sizeof nombreComprador);
    NombreRP(vendedor, nombreVendedor, sizeof nombreVendedor);

    format(mensaje, sizeof mensaje, "%s ha aceptado tu arma y te ha pagado %s.", nombreComprador, precioTexto);
    Exito(vendedor, mensaje);

    format(mensaje, sizeof mensaje, "Has aceptado el arma. Pagaste %s.", precioTexto);
    Exito(playerid, mensaje);

    format(g_Buffer, sizeof g_Buffer, "%s compro un arma a %s por %d", nombreComprador, nombreVendedor, precio);
    EscribirLog("economia", g_Buffer);

    // Aviso a la policia (37_policia.inc). El sospechoso es el VENDEDOR:
    // el delito 302 es la venta, no la compra.
    new Float:avx, Float:avy, Float:avz;
    GetPlayerPos(playerid, avx, avy, avz);
    Policia_CrearAviso(AVISO_ARMAS, vendedor, avx, avy, avz, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));

    gArmas[playerid][aaOfertaArma] = 0;
    return 1;
}

// Alias exacto del cartel: "Vendedor de Armas / /aceptarjob".
CMD:aceptarjob(playerid, params[]) return cmd_trabajo(playerid, params);
