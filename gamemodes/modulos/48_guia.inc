/* =====================================================================
 *  48_guia.inc
 *  ---------------------------------------------------------------------
 *  Guia ingame, encargo de Vlad del 05/09/2026. Mismo mecanismo que el
 *  icono de bienvenida (47_bienvenida.inc): un objeto en el mundo (bolsa
 *  de dinero, modelo 1274 -- la misma del banco), en la coordenada que
 *  Vlad marco con el panel (tabla `coordenadas`, id 50).
 *
 *  La diferencia es que aqui no se da nada -- se abre un dialogo con
 *  paginas (Anterior/Siguiente) explicando como generar dinero en el
 *  servidor. Repartido en 4 paginas a proposito: si sale todo de golpe
 *  en un solo muro de texto, el jugador cierra la ventana sin leer nada.
 *
 *      1. Nomina + trabajos libres
 *      2. Los coches de Federico + mercado de vehiculos
 *      3. Forzar maleteros + hacking (Panel del Jugador)
 *      4. Enlaces web
 *
 *  Se dispara al ENTRAR en el radio (deteccion por flanco, no por
 *  cooldown de tiempo): si te quedas parado encima no se reabre en
 *  bucle, y si te vas y vuelves, se abre de nuevo sin esperar nada.
 * ===================================================================== */

static STREAMER_TAG_PICKUP:g_GuiaPickup;
static STREAMER_TAG_3D_TEXT_LABEL:g_GuiaEtiqueta;

static bool:g_GuiaDentro[MAX_PLAYERS];
static g_GuiaPaginaActual[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  ARRANQUE
// ---------------------------------------------------------------------
stock Guia_Iniciar()
{
    g_GuiaPickup = CreateDynamicPickup(GUIA_ICONO_MODELO, 23,
        GUIA_X, GUIA_Y, GUIA_Z, 0, 0, -1, 100.0);

    g_GuiaEtiqueta = CreateDynamic3DTextLabel(
        "{53C168}Guia de Gamerol\n{FFFFFF}Acercate para ver como ganar dinero",
        0x53C168FF, GUIA_X, GUIA_Y, GUIA_Z + 0.7, 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    return 1;
}

stock Guia_OnConnect(playerid)
{
    g_GuiaDentro[playerid] = false;
    g_GuiaPaginaActual[playerid] = 0;
    return 1;
}

// ---------------------------------------------------------------------
//  TICK -- llamado cada segundo real desde TicSegundo (99_callbacks.inc).
//  Deteccion por flanco: solo dispara al entrar, no cada segundo que
//  pasas dentro del radio.
// ---------------------------------------------------------------------
stock Guia_Segundo()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (!IsPlayerConnected(playerid)) continue;
        if (!Jugador[playerid][jLogueado]) continue;

        new bool:dentro = (GetPlayerInterior(playerid) == 0 && GetPlayerVirtualWorld(playerid) == 0
            && EnPunto(playerid, GUIA_RADIO, GUIA_X, GUIA_Y, GUIA_Z));

        if (dentro && !g_GuiaDentro[playerid])
        {
            g_GuiaDentro[playerid] = true;
            Guia_MostrarPagina(playerid, 1);
        }
        else if (!dentro && g_GuiaDentro[playerid])
        {
            g_GuiaDentro[playerid] = false;
        }
    }
    return 1;
}

// ---------------------------------------------------------------------
//  PAGINAS
// ---------------------------------------------------------------------
stock Guia_MostrarPagina(playerid, pagina)
{
    g_GuiaPaginaActual[playerid] = pagina;

    new titulo[48];
    new cuerpo[1800];
    new boton1[16];
    new boton2[16];

    switch (pagina)
    {
        case 1:
        {
            format(titulo, sizeof titulo, "{53C168}GAMEROL {FFFFFF}- Pagina 1/4");
            format(boton1, sizeof boton1, "Siguiente");
            format(boton2, sizeof boton2, "Cerrar");
            format(cuerpo, sizeof cuerpo,
                "{53C168}GAMEROL\n\n\
                {FFFFFF}Recuerda visitar la web para llevar tu partida al siguiente nivel.\n\
                {E05A5A}Gamerol/panel\n\n\n\
                {53C168}¿COMO CONSEGUIR DINERO?\n\n\n\
                {53C168}NOMINA\n\n\
                {FFFFFF}Todo jugador recibe un pago cada 20 minutos jugados.\n\
                La cantidad depende de tu trabajo o faccion.\n\n\n\
                {53C168}TRABAJOS LIBRES\n\n\
                {53C168}$$ {FFFFFF}Vendedor de armas\n\
                Gana dinero realizando ventas entre jugadores.\n\n\
                {53C168}$$ {FFFFFF}NFS\n\
                Compra, mueve y vende vehiculos a otros jugadores.\n\n\
                {53C168}$$$ {FFFFFF}Vendedor de marihuana\n\
                Vende marihuana a NPC.\n\
                ¡Altas ganancias!\n\n\
                {53C168}$$ {FFFFFF}Medicos\n\
                Cura a otros jugadores con /tratar y cobra el precio acordado.\n\n\
                {53C168}$$ {FFFFFF}Facciones\n\
                La Policia Nacional obtiene una payday x4.\n\n\n\
                {53C168}OTRAS FORMAS DE GANAR DINERO\n\n\
                {53C168}$$ {FFFFFF}Atracos\n\
                Atraca comercios desprotegidos con /atracar.\n\
                Ojo, se avisa a la Policia.");
        }
        case 2:
        {
            format(titulo, sizeof titulo, "{53C168}GAMEROL {FFFFFF}- Pagina 2/4");
            format(boton1, sizeof boton1, "Siguiente");
            format(boton2, sizeof boton2, "Atras");
            format(cuerpo, sizeof cuerpo,
                "{E05A5A}$$$$$ LOS VEHICULOS DE FEDERICO\n\n\
                {FFFFFF}Entrega los vehiculos que Federico esta buscando.\n\n\
                Consulta la lista usando:\n\
                {E05A5A}/verlista\n\n\
                {FFFFFF}Las recompensas van desde:\n\
                {53C168}10.000$ hasta 300.000$\n\n\
                {FFFFFF}El pago depende de la rareza y dificultad del vehiculo.\n\n\
                {E05A5A}$$$$$$ ROBO EXPRESS\n\
                {FFFFFF}De vez en cuando Federico necesita un vehiculo muy concreto YA.\n\
                Si ves el aviso, usa:\n\
                {E05A5A}/aceptarmision\n\
                {53C168}¡Paga 1.000.000$!\n\n\
                Mas informacion:\n\
                {E05A5A}Gamerol/federico\n\n\n\
                {53C168}MERCADO DE VEHICULOS\n\n\
                {FFFFFF}Consulta el stock de vehiculos disponibles en:\n\
                {E05A5A}Gamerol/coches");
        }
        case 3:
        {
            format(titulo, sizeof titulo, "{53C168}GAMEROL {FFFFFF}- Pagina 3/4");
            format(boton1, sizeof boton1, "Siguiente");
            format(boton2, sizeof boton2, "Atras");
            format(cuerpo, sizeof cuerpo,
                "{E05A5A}FORZAR MALETEROS\n\n\
                {FFFFFF}Hay cientos de coches repartidos por el mapa.\n\n\
                Usa:\n\
                {E05A5A}/forzarmaletero\n\n\
                {FFFFFF}para intentar abrir el maletero de un vehiculo y conseguir el loot de su interior.\n\n\
                El loot puede venderse en la Casa de Empeños.\n\
                Consulta su ubicacion usando:\n\
                {E05A5A}/gps\n\n\n\
                {E05A5A}HACKING\n\n\
                {FFFFFF}Tambien puedes conseguir informacion sobre maleteros con objetos de alto valor desde el Panel del Jugador.\n\n\
                Entra en:\n\
                {E05A5A}Gamerol/panel\n\n\
                {FFFFFF}Inicia sesion con tu cuenta del juego y busca:\n\
                {53C168}HACKEAR\n\n\
                {FFFFFF}Lo encontraras en la parte inferior derecha del panel.");
        }
        case 4:
        {
            format(titulo, sizeof titulo, "{53C168}GAMEROL {FFFFFF}- Pagina 4/4");
            format(boton1, sizeof boton1, "Cerrar");
            format(boton2, sizeof boton2, "Atras");
            format(cuerpo, sizeof cuerpo,
                "{53C168}ENLACES WEB\n\n\
                {FFFFFF}Toda la web de Gamerol, en un mismo sitio:\n\n\
                {53C168}Panel del jugador\n\
                {E05A5A}Gamerol/panel\n\n\
                {53C168}Los coches de Federico\n\
                {E05A5A}Gamerol/federico\n\n\
                {53C168}Mercado de vehiculos\n\
                {E05A5A}Gamerol/coches\n\n\
                {53C168}Pagina principal\n\
                {E05A5A}Gamerol");
        }
        default: return 0;
    }

    ShowPlayerDialog(playerid, DIALOGO_GUIA, DIALOG_STYLE_MSGBOX, titulo, cuerpo, boton1, boton2);
    return 1;
}

// ---------------------------------------------------------------------
//  RESPUESTA AL DIALOGO -- boton1 siempre "avanza" (o "Cerrar" en la
//  pagina 4), boton2 siempre "retrocede" (o "Cerrar" en la pagina 1).
// ---------------------------------------------------------------------
stock Guia_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused listitem, inputtext
    if (dialogid != DIALOGO_GUIA) return 0;

    switch (g_GuiaPaginaActual[playerid])
    {
        case 1: if (response) Guia_MostrarPagina(playerid, 2);
        case 2: if (response) Guia_MostrarPagina(playerid, 3); else Guia_MostrarPagina(playerid, 1);
        case 3: if (response) Guia_MostrarPagina(playerid, 4); else Guia_MostrarPagina(playerid, 2);
        case 4: if (!response) Guia_MostrarPagina(playerid, 3);
    }
    return 1;
}
