/* =====================================================================
 *  49_guiavehiculos.inc
 *  ---------------------------------------------------------------------
 *  Guia de vehiculos, encargo de Vlad del 05/09/2026. Copia exacta del
 *  mecanismo y el estilo de la guia general (48_guia.inc) -- mismo
 *  dialogo, misma paginacion Anterior/Siguiente, mismos colores -- solo
 *  cambia el objeto en el mundo (modelo 19130) y el contenido, esta vez
 *  centrado en el sistema de vehiculos: comprar, papeles, rareza, NFS,
 *  robo, Federico y maleteros.
 *
 *  Nota de fidelidad (encargo explicito: "no inventar comandos ni
 *  precios"): el texto original pedia "/vendercoche ID PRECIO" como
 *  venta generica entre jugadores, pero ese comando no existe en el
 *  gamemode -- la unica venta de un vehiculo a otro jugador es
 *  /nfsvendercoche (26_nfs.inc), y solo funciona con coches ya marcados
 *  NFS por un miembro de ese trabajo. La pagina de PAPELES se ajusto
 *  para no prometer algo que el servidor no hace; el comando real quedo
 *  puesto en la pagina de NFS, que es donde de verdad se usa.
 *
 *  8 paginas: 1) Comprar  2) Papeles  3) Rareza  4) NFS  5) Robo
 *             6) Federico  7) Maleteros  8) Cierre
 * ===================================================================== */

static STREAMER_TAG_PICKUP:g_GuiaVehPickup;
static STREAMER_TAG_3D_TEXT_LABEL:g_GuiaVehEtiqueta;

static bool:g_GuiaVehDentro[MAX_PLAYERS];
static g_GuiaVehPaginaActual[MAX_PLAYERS];

#define GUIAVEH_TOTAL_PAGINAS 8

// ---------------------------------------------------------------------
//  ARRANQUE
// ---------------------------------------------------------------------
stock GuiaVeh_Iniciar()
{
    g_GuiaVehPickup = CreateDynamicPickup(GUIAVEH_ICONO_MODELO, 23,
        GUIAVEH_X, GUIAVEH_Y, GUIAVEH_Z, 0, 0, -1, 100.0);

    g_GuiaVehEtiqueta = CreateDynamic3DTextLabel(
        "{53C168}Sistema de vehiculos de Gamerol\n{FFFFFF}Acercate para ver la guia",
        0x53C168FF, GUIAVEH_X, GUIAVEH_Y, GUIAVEH_Z + 0.7, 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    return 1;
}

stock GuiaVeh_OnConnect(playerid)
{
    g_GuiaVehDentro[playerid] = false;
    g_GuiaVehPaginaActual[playerid] = 0;
    return 1;
}

// ---------------------------------------------------------------------
//  TICK -- llamado cada segundo real desde TicSegundo (99_callbacks.inc).
//  Deteccion por flanco, igual que 48_guia.inc.
// ---------------------------------------------------------------------
stock GuiaVeh_Segundo()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (!IsPlayerConnected(playerid)) continue;
        if (!Jugador[playerid][jLogueado]) continue;

        new bool:dentro = (GetPlayerInterior(playerid) == 0 && GetPlayerVirtualWorld(playerid) == 0
            && EnPunto(playerid, GUIAVEH_RADIO, GUIAVEH_X, GUIAVEH_Y, GUIAVEH_Z));

        if (dentro && !g_GuiaVehDentro[playerid])
        {
            g_GuiaVehDentro[playerid] = true;
            GuiaVeh_MostrarPagina(playerid, 1);
        }
        else if (!dentro && g_GuiaVehDentro[playerid])
        {
            g_GuiaVehDentro[playerid] = false;
        }
    }
    return 1;
}

// ---------------------------------------------------------------------
//  PAGINAS
// ---------------------------------------------------------------------
stock GuiaVeh_MostrarPagina(playerid, pagina)
{
    g_GuiaVehPaginaActual[playerid] = pagina;

    new titulo[56];
    new cuerpo[1400];
    new boton1[16];
    new boton2[16];

    format(titulo, sizeof titulo, "{53C168}SISTEMA DE VEHICULOS {FFFFFF}- Pagina %d/%d", pagina, GUIAVEH_TOTAL_PAGINAS);
    format(boton1, sizeof boton1, (pagina == GUIAVEH_TOTAL_PAGINAS) ? ("Cerrar") : ("Siguiente"));
    format(boton2, sizeof boton2, (pagina == 1) ? ("Cerrar") : ("Atras"));

    switch (pagina)
    {
        case 1:
        {
            format(cuerpo, sizeof cuerpo,
                "{53C168}SISTEMA DE VEHICULOS\n\n\
                {FFFFFF}Hay cientos de vehiculos repartidos por todo el mapa.\n\
                Los vehiculos que estan libres pueden ser comprados por cualquier jugador.\n\n\n\
                {53C168}COMPRAR UN COCHE\n\n\
                {FFFFFF}Subete a un vehiculo libre y utiliza:\n\n\
                {E05A5A}/comprarcoche\n\n\
                {FFFFFF}Si esta disponible, pasaras a ser su propietario.");
        }
        case 2:
        {
            format(cuerpo, sizeof cuerpo,
                "{53C168}PAPELES DEL VEHICULO\n\n\
                {FFFFFF}Los papeles de tu vehiculo duran 7 dias.\n\n\
                Puedes renovarlos utilizando:\n\n\
                {E05A5A}/taller\n\n\
                {FFFFFF}en el recinto de la NFS. Consulta su ubicacion con {E05A5A}/gps\n\n\n\
                {FFFFFF}Si los papeles caducan, pierdes la propiedad y el vehiculo vuelve a quedar LIBRE, aparcado en su ultimo sitio.");
        }
        case 3:
        {
            format(cuerpo, sizeof cuerpo,
                "{53C168}RAREZA DE LOS COCHES\n\n\
                {FFFFFF}Los vehiculos estan divididos en cuatro categorias:\n\n\
                {E05A5A}SUPER RARE\n\
                {FFFFFF}RARE\n\
                {FFFFFF}DEL MONTON\n\
                {FFFFFF}CHATARRA\n\n\
                Algunos modelos son muchisimo mas dificiles de encontrar que otros.\n\n\
                Consulta los vehiculos del servidor en:\n\n\
                {E05A5A}Gamerol/coches");
        }
        case 4:
        {
            format(cuerpo, sizeof cuerpo,
                "{53C168}NFS\n\n\
                {FFFFFF}La NFS es un trabajo libre dedicado al mundo del motor.\n\n\
                Sus miembros pueden recoger vehiculos libres y llevarlos hasta su recinto.\n\n\
                Una vez alli, esos coches salen del sistema normal de /comprarcoche y se adquieren a traves de la NFS con:\n\n\
                {E05A5A}/nfsvendercoche\n\n\
                {FFFFFF}La NFS intentara conseguir los mejores vehiculos para venderlos a otros jugadores.");
        }
        case 5:
        {
            format(cuerpo, sizeof cuerpo,
                "{E05A5A}ROBO DE VEHICULOS\n\n\
                {FFFFFF}¡No dejes tu vehiculo personal abierto!\n\n\
                Otro jugador puede intentar hacerle un puente utilizando:\n\n\
                {E05A5A}/puente\n\n\
                {FFFFFF}Si consigue robarlo y liberarlo en el taller de la NFS, perderas los papeles y el coche volvera a quedar libre.");
        }
        case 6:
        {
            format(cuerpo, sizeof cuerpo,
                "{E05A5A}$$$$$ LOS COCHES DE FEDERICO\n\n\
                {FFFFFF}Federico busca determinados vehiculos y paga grandes cantidades de dinero por conseguirlos.\n\n\
                Varias categorias a la vez -- normal, Secreto, Moto del Dia, Chatarra y Personaje Historico -- todas renovandose cada 24 horas.\n\n\
                Consulta todo lo que busca ahora mismo utilizando:\n\n\
                {E05A5A}/verlista\n\n\
                {FFFFFF}Consulta la lista completa y el tiempo restante en:\n\n\
                {E05A5A}Gamerol/federico\n\n\
                {FFFFFF}Entrega en Los Santos o en Angel Pine, utilizando:\n\n\
                {E05A5A}/entregarcoche");
        }
        case 7:
        {
            format(cuerpo, sizeof cuerpo,
                "{53C168}MALETEROS\n\n\
                {FFFFFF}Si eres propietario de un vehiculo y estas junto a el, utiliza:\n\n\
                {E05A5A}/maletero\n\n\
                {FFFFFF}para acceder a tu maletero. Un jugador que NO sea propietario no puede usarlo.\n\n\
                Si quieres intentar acceder al maletero de un vehiculo ajeno, debes utilizar:\n\n\
                {E05A5A}/forzarmaletero\n\n\
                {FFFFFF}Algunos vehiculos repartidos por el mapa pueden contener objetos de gran valor.\n\n\
                Tambien puedes usar el sistema HACKEAR del Panel del Jugador:\n\n\
                {E05A5A}Gamerol/panel");
        }
        case 8:
        {
            format(cuerpo, sizeof cuerpo,
                "{53C168}ENCUENTRA - COMPRA - ROBA - VENDE - ENTREGA\n\n\
                {FFFFFF}Los coches de Gamerol no son solo transporte.\n\n\
                Explora el mapa, encuentra modelos raros, comercia con otros jugadores, trabaja con la NFS, busca los vehiculos de Federico y manten tus papeles al dia.\n\n\
                {E05A5A}Gamerol/coches\n\
                {E05A5A}Gamerol/federico");
        }
        default: return 0;
    }

    ShowPlayerDialog(playerid, DIALOGO_GUIA_VEHICULOS, DIALOG_STYLE_MSGBOX, titulo, cuerpo, boton1, boton2);
    return 1;
}

// ---------------------------------------------------------------------
//  RESPUESTA AL DIALOGO -- boton1 siempre "avanza" (o "Cerrar" en la
//  ultima pagina), boton2 siempre "retrocede" (o "Cerrar" en la 1).
// ---------------------------------------------------------------------
stock GuiaVeh_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused listitem, inputtext
    if (dialogid != DIALOGO_GUIA_VEHICULOS) return 0;

    new pagina = g_GuiaVehPaginaActual[playerid];

    if (pagina == GUIAVEH_TOTAL_PAGINAS)
    {
        if (!response) GuiaVeh_MostrarPagina(playerid, pagina - 1);
        return 1;
    }
    if (pagina == 1)
    {
        if (response) GuiaVeh_MostrarPagina(playerid, pagina + 1);
        return 1;
    }

    if (response) GuiaVeh_MostrarPagina(playerid, pagina + 1);
    else           GuiaVeh_MostrarPagina(playerid, pagina - 1);
    return 1;
}
