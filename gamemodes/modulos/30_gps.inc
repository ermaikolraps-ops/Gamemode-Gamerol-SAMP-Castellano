/* =====================================================================
 *  30_gps.inc
 *  ---------------------------------------------------------------------
 *  Sin blips fijos en el minimapa (se quitaron todos a proposito): para
 *  llegar a cualquier punto de interes se usa /gps. Marca un checkpoint
 *  y una ruta hacia el sitio elegido, igual que hacia el punto de
 *  contratacion de un trabajo o cualquier otro destino fijo.
 *
 *  Por ahora solo estos seis destinos; se puede ampliar la lista mas
 *  adelante sin tocar nada mas que este archivo.
 * ===================================================================== */

#define GPS_RADIO_CHECKPOINT     5.0

// Fila de textdraws fijos abajo a la izquierda: [/GPS], contadores...,
// [/Comprarcoche] al final de la fila (mismo patron que [/GPS]: rojo, fijo,
// recordatorio del comando, no un contador).
// El hueco visible tiene dos partes: el ANCHO propio del texto anterior
// (que SI depende de cuantos caracteres tenga) y un HUECO fijo en
// blanco detras (que NO depende del largo -- es el mismo siempre, y es
// el que quedo "perfecto" entre [/GPS] y coches). El primer intento
// mezclaba las dos cosas en un solo numero por caracter, y al aplicarlo
// a un texto mas largo el hueco en blanco crecia tambien -- de ahi que
// "casas" quedara lejísimos. Separados asi:
//   X nueva = X anterior + (ANCHO_CARACTER * caracteres del texto anterior) + HUECO_FIJO
// Para sumar un elemento nuevo detras de "negocios": GPS_HUD_X_NEGOCIOS +
// (GPS_HUD_ANCHO_CARACTER * caracteres de "[X negocios libres]") + GPS_HUD_HUECO_FIJO
#define GPS_HUD_ANCHO_CARACTER   5.0
#define GPS_HUD_HUECO_FIJO       12.0
#define GPS_HUD_X_GPS            8.0
#define GPS_HUD_X_COCHES         (GPS_HUD_X_GPS      + (GPS_HUD_ANCHO_CARACTER * 6.0)  + GPS_HUD_HUECO_FIJO)   // "[/GPS]" = 6 caracteres
#define GPS_HUD_X_CASAS          (GPS_HUD_X_COCHES   + (GPS_HUD_ANCHO_CARACTER * 19.0) + GPS_HUD_HUECO_FIJO)   // "[209 coches libres]" = 19 caracteres
#define GPS_HUD_X_NEGOCIOS       (GPS_HUD_X_CASAS    + (GPS_HUD_ANCHO_CARACTER * 16.0) + GPS_HUD_HUECO_FIJO)   // "[X casas libres]" = 16 caracteres (un pelin mas cerca que el calculo inicial)
#define GPS_HUD_X_COMPRARCOCHE   (GPS_HUD_X_NEGOCIOS + (GPS_HUD_ANCHO_CARACTER * 19.0) + GPS_HUD_HUECO_FIJO)   // "[X negocios libres]" = 19 caracteres

// ---------------------------------------------------------------------
//  CATEGORIAS -- 07/09/2026, reorganizacion completa a peticion de Vlad.
//  /gps ya no es una lista plana: primero eliges categoria, luego el
//  destino dentro de ella ("Volver" regresa a las categorias). Cada
//  categoria tiene su propio color de cabecera; dentro, cada fila
//  conserva el color/etiqueta que ya tenia en la lista plana de antes.
// ---------------------------------------------------------------------
enum
{
    GPSCAT_ROL,
    GPSCAT_ACTIVIDADES,
    GPSCAT_COMIDA,
    GPSCAT_VEHICULOS,
    GPSCAT_FEDERICO,
    GPSCAT_NEGOCIOS,
    GPSCAT_DINERO,
    GPSCAT_TRABAJOS,
    GPSCAT_CRONOS,
    GPSCAT_TOTAL
}

static g_GpsCategoriaSel[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  TEXTDRAW FIJO
//  Recordatorio permanente en pantalla de que /GPS existe -- pedido
//  expresamente para que nadie se quede sin saber que el comando existe.
//  Se muestra una vez, al terminar el login (GPS_MostrarTextdrawFijo,
//  llamado desde Login_TerminarSesion en 06_login.inc), y no se oculta
//  mas: no hay ningun otro elemento fijo en esa esquina con el que pueda
//  chocar.
// ---------------------------------------------------------------------
static Text:g_GpsTextdrawFijo;
static Text:g_ComprarCocheTD;                          // "[/Comprarcoche]", mismo patron que [/GPS]
static Text:g_WebURL_TD;                                // "Gamerol" arriba a la derecha
static PlayerText:g_GpsContadorTD[MAX_PLAYERS];        // contador "[XXX coches libres]", ver mas abajo
static PlayerText:g_CasasLibresTD[MAX_PLAYERS];        // contador "[XXX casas libres]", ver mas abajo
static PlayerText:g_NegociosLibresTD[MAX_PLAYERS];     // contador "[X negocios libres]", ver mas abajo

stock GPS_CrearTextdraw()
{
    // Restilado 13/09/2026 a peticion de Vlad: mismo color y fuente que
    // "PUENTEANDO..." (22_dropeo.inc) -- Pricedown, 0x4FD1C5FF. Antes era
    // Aharoni Bold rojo. Posicion sin tocar: ya quedaba "cerca de la zona
    // inferior izquierda/radar" pedida, y ahora ademas queda sola en su
    // fila (ver GPS_MostrarTextdrawFijo mas abajo, ya no comparte hueco
    // con los contadores ni con [/Comprarcoche]).
    g_GpsTextdrawFijo = TextDrawCreate(GPS_HUD_X_GPS, 425.0, "[/GPS]");
    TextDrawFont(g_GpsTextdrawFijo, TEXT_DRAW_FONT_PRICEDOWN);
    TextDrawLetterSize(g_GpsTextdrawFijo, 0.28, 1.3);
    TextDrawColour(g_GpsTextdrawFijo, 0x4FD1C5FF);
    TextDrawSetShadow(g_GpsTextdrawFijo, 1);
    TextDrawSetOutline(g_GpsTextdrawFijo, 1);
    TextDrawSetProportional(g_GpsTextdrawFijo, true);

    g_ComprarCocheTD = TextDrawCreate(GPS_HUD_X_COMPRARCOCHE, 425.0, "[/Comprarcoche]");
    TextDrawFont(g_ComprarCocheTD, TEXT_DRAW_FONT_AHARONI_BOLD);
    TextDrawLetterSize(g_ComprarCocheTD, 0.315, 1.44);
    TextDrawColour(g_ComprarCocheTD, 0xFF0000FF);
    TextDrawSetShadow(g_ComprarCocheTD, 1);
    TextDrawSetOutline(g_ComprarCocheTD, 1);
    TextDrawSetProportional(g_ComprarCocheTD, true);

    // STEP 14 UI 18/09/2026: la URL "Gamerol" que aparecia
    // sobre el HUD del vehiculo se retira a peticion de Vlad al montar el
    // widget nuevo (13_vehiculos.inc). YA NO se crea el TextDraw ni se
    // muestra. Se conserva la variable g_WebURL_TD y los stocks
    // GPS_MostrarURL/GPS_OcultarURL como no-op por si algun modulo futuro
    // los llama sin querer (rollback: descomentar el bloque de abajo y
    // borrar la linea de #pragma unused).
    #pragma unused g_WebURL_TD
    /*
    g_WebURL_TD = TextDrawCreate(511.0, 397.0, "Gamerol");
    TextDrawFont(g_WebURL_TD, TEXT_DRAW_FONT_BANK_GOTHIC);
    TextDrawLetterSize(g_WebURL_TD, 0.22, 1.0);
    TextDrawColour(g_WebURL_TD, 0x4FD1C5FF);
    TextDrawSetShadow(g_WebURL_TD, 0);
    TextDrawSetOutline(g_WebURL_TD, 1);
    TextDrawSetProportional(g_WebURL_TD, true);
    */
    return 1;
}

stock GPS_MostrarTextdrawFijo(playerid)
{
    TextDrawShowForPlayer(playerid, g_GpsTextdrawFijo);

    // 13/09/2026, peticion expresa de Vlad: quitar del HUD permanente los
    // contadores "[X coches/casas/negocios libres]" y el recordatorio
    // "[/Comprarcoche]" -- limpieza visual, no un cambio de sistema. Los
    // textdraws se siguen creando y GPS_ActualizarContador() los sigue
    // actualizando cada segundo exactamente igual que antes (no cuesta
    // nada mantenerlos vivos aunque nunca se muestren), por si algun dia
    // se quiere reactivar esta fila sin tener que reconstruir nada. La
    // informacion sigue disponible via /gps y los comandos de cada
    // sistema; /comprarcoche sigue funcionando igual, solo desaparece el
    // cartel permanente que lo recordaba.
    // TextDrawShowForPlayer(playerid, g_ComprarCocheTD);
    // PlayerTextDrawShow(playerid, g_GpsContadorTD[playerid]);
    // PlayerTextDrawShow(playerid, g_CasasLibresTD[playerid]);
    // PlayerTextDrawShow(playerid, g_NegociosLibresTD[playerid]);

    HambreSed_Mostrar(playerid); // ahora bajo el dinero, arriba a la derecha (44_hambre_sed.inc)
    return 1;
}

// Usado por Federico_MostrarUI/OcultarUI (43_federico.inc): en vez de
// alargar el fondo de /federico para tapar esta fila a la fuerza (se
// probo y quedaba forzado), se oculta la fila entera mientras la
// pantalla de Federico esta abierta y se repone al cerrarla.
stock GPS_OcultarTextdrawFijo(playerid)
{
    TextDrawHideForPlayer(playerid, g_GpsTextdrawFijo);
    TextDrawHideForPlayer(playerid, g_ComprarCocheTD);
    PlayerTextDrawHide(playerid, g_GpsContadorTD[playerid]);
    PlayerTextDrawHide(playerid, g_CasasLibresTD[playerid]);
    PlayerTextDrawHide(playerid, g_NegociosLibresTD[playerid]);
    HambreSed_Ocultar(playerid);
    return 1;
}

// STEP 14 UI 18/09/2026: stocks conservados pero NO-OP. La URL ya no se
// crea (ver Iniciar arriba); pintarla o esconderla es un no-op seguro
// aunque el TextDraw handle sea INVALID_TEXT_DRAW. Rollback: rehacer el
// TextDraw en Iniciar y volver a llamar Show/Hide desde 13_vehiculos.inc.
stock GPS_MostrarURL(playerid)
{
    #pragma unused playerid
    return 1;
}

stock GPS_OcultarURL(playerid)
{
    #pragma unused playerid
    return 1;
}

// ---------------------------------------------------------------------
//  CONTADORES DE COCHES Y CASAS LIBRES
//  Al lado del [/GPS], mismo font/negrita, en verde. Son por-jugador (no
//  textdraws globales) porque asi PlayerTextDrawSetString los actualiza
//  en caliente sin tener que ocultarlos y volver a mostrarlos -- mismo
//  mecanismo que ya usa el velocimetro (Veh_HUD_Segundo, 13_vehiculos.inc).
// ---------------------------------------------------------------------
stock GPS_CrearTextdrawJugador(playerid)
{
    g_GpsContadorTD[playerid] = CreatePlayerTextDraw(playerid, GPS_HUD_X_COCHES, 425.0, "[0 coches libres]");
    PlayerTextDrawFont(playerid, g_GpsContadorTD[playerid], TEXT_DRAW_FONT_AHARONI_BOLD);
    PlayerTextDrawLetterSize(playerid, g_GpsContadorTD[playerid], 0.315, 1.44);
    PlayerTextDrawColour(playerid, g_GpsContadorTD[playerid], 0x32FF32FF);
    PlayerTextDrawSetShadow(playerid, g_GpsContadorTD[playerid], 1);
    PlayerTextDrawSetOutline(playerid, g_GpsContadorTD[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_GpsContadorTD[playerid], true);

    g_CasasLibresTD[playerid] = CreatePlayerTextDraw(playerid, GPS_HUD_X_CASAS, 425.0, "[0 casas libres]");
    PlayerTextDrawFont(playerid, g_CasasLibresTD[playerid], TEXT_DRAW_FONT_AHARONI_BOLD);
    PlayerTextDrawLetterSize(playerid, g_CasasLibresTD[playerid], 0.315, 1.44);
    PlayerTextDrawColour(playerid, g_CasasLibresTD[playerid], 0x32FF32FF);
    PlayerTextDrawSetShadow(playerid, g_CasasLibresTD[playerid], 1);
    PlayerTextDrawSetOutline(playerid, g_CasasLibresTD[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_CasasLibresTD[playerid], true);

    g_NegociosLibresTD[playerid] = CreatePlayerTextDraw(playerid, GPS_HUD_X_NEGOCIOS, 425.0, "[0 negocios libres]");
    PlayerTextDrawFont(playerid, g_NegociosLibresTD[playerid], TEXT_DRAW_FONT_AHARONI_BOLD);
    PlayerTextDrawLetterSize(playerid, g_NegociosLibresTD[playerid], 0.315, 1.44);
    PlayerTextDrawColour(playerid, g_NegociosLibresTD[playerid], 0x32FF32FF);
    PlayerTextDrawSetShadow(playerid, g_NegociosLibresTD[playerid], 1);
    PlayerTextDrawSetOutline(playerid, g_NegociosLibresTD[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_NegociosLibresTD[playerid], true);
    return 1;
}

// Llamado cada segundo desde TicSegundo (99_callbacks.inc): un solo
// conteo de cada cosa, repartido al texto de todos los conectados.
stock GPS_ActualizarContador()
{
    new totalCoches = Dropeo_ContarLibres();
    new textoCoches[32];
    format(textoCoches, sizeof textoCoches, "[%d coches libres]", totalCoches);

    new totalCasas = Casa_ContarLibres();
    new textoCasas[32];
    format(textoCasas, sizeof textoCasas, "[%d casas libres]", totalCasas);

    new totalNegocios = Negocio_ContarLibres();
    new textoNegocios[32];
    format(textoNegocios, sizeof textoNegocios, "[%d negocios libres]", totalNegocios);

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        PlayerTextDrawSetString(i, g_GpsContadorTD[i], textoCoches);
        PlayerTextDrawSetString(i, g_CasasLibresTD[i], textoCasas);
        PlayerTextDrawSetString(i, g_NegociosLibresTD[i], textoNegocios);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDO
// ---------------------------------------------------------------------
CMD:gps(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    GPS_MostrarCategorias(playerid);
    return 1;
}

stock GPS_MostrarCategorias(playerid)
{
    ShowPlayerDialog(playerid, DIALOGO_GPS, DIALOG_STYLE_LIST, "{E8B33A}GPS -- elige una categoria",
        "{4FD1C5}Rol\n\
        {53C168}Actividades\n\
        {FF8C00}Comida\n\
        {5A9BD5}Vehiculos\n\
        {8BC4E8}Entregar Coches \"Federico\"\n\
        {32FF32}Negocios\n\
        {FF0000}GANAR DINERO $\n\
        {E8973A}Trabajos\n\
        {E05A5A}Cronos",
        "Elegir", "Cerrar");
    return 1;
}

// ---------------------------------------------------------------------
//  DESTINOS POR CATEGORIA
//  Marca la categoria elegida (g_GpsCategoriaSel) y muestra su lista.
//  Buffer de 1024: la categoria mas larga (GANAR DINERO, 12 filas con
//  descripciones) en bruto con colores de por medio ronda los 700-800.
// ---------------------------------------------------------------------
stock GPS_MostrarDestinos(playerid, categoria)
{
    g_GpsCategoriaSel[playerid] = categoria;

    new titulo[64];
    new lista[1536];   // GANAR DINERO $ crecio a 15 filas (QoL 08/09/2026) -- 1024 se quedaba justo

    switch (categoria)
    {
        case GPSCAT_ROL:
        {
            format(titulo, sizeof titulo, "{4FD1C5}GPS > Rol");
            format(lista, sizeof lista,
                "Banco\n\
                Banco {8BC4E8}(Angel Pine)\n\
                Tienda LS {8BC4E8}(Cosas varias)\n\
                Tienda Angel Pine {8BC4E8}(Cosas varias)\n\
                {32FF32}(NEGOCIO){FFFFFF} Motel Angel Pine");
        }
        case GPSCAT_ACTIVIDADES:
        {
            format(titulo, sizeof titulo, "{53C168}GPS > Actividades");
            format(lista, sizeof lista,
                "{4FD1C5}(Pescar){FFFFFF} Zona Los Santos\n\
                {4FD1C5}(Pescar){FFFFFF} Zona Angel Pine\n\
                {4FD1C5}(Venta Pescado){FFFFFF} Los Santos\n\
                {4FD1C5}(Venta Pescado){FFFFFF} Angel Pine\n\
                {4FD1C5}(Venta Pescado){FFFFFF} Tierra Robada");
        }
        case GPSCAT_COMIDA:
        {
            format(titulo, sizeof titulo, "{FF8C00}GPS > Comida");
            format(lista, sizeof lista,
                "{FF0000}(Comida){FFFFFF} Burger Shot {8BC4E8}(LS)\n\
                {FF0000}(Comida){FFFFFF} Cluckin' Bell {8BC4E8}(LS)\n\
                {FF0000}(Comida){FFFFFF} Well Stacked Pizza {8BC4E8}(LS)\n\
                {FF0000}(Comida){FFFFFF} Cluckin' Bell {8BC4E8}(Angel Pine)\n\
                {FF0000}(Comida){FFFFFF} Dogs {8BC4E8}(Angel Pine)");
        }
        case GPSCAT_VEHICULOS:
        {
            format(titulo, sizeof titulo, "{5A9BD5}GPS > Vehiculos");
            format(lista, sizeof lista,
                "Gasolinera {8BC4E8}(LS)\n\
                Gasolinera {8BC4E8}(Angel Pine)\n\
                Transfender {8BC4E8}(LS)\n\
                Taller NFS {8BC4E8}(LS)\n\
                Mecanico {8BC4E8}(Grove Street)\n\
                Mecanico {8BC4E8}(Angel Pine)\n\
                Mecanico {8BC4E8}(Policia Nacional)\n\
                Mecanico {8BC4E8}(NFS)\n\
                Gasolinera Carretera LS-SF");
        }
        case GPSCAT_FEDERICO:
        {
            format(titulo, sizeof titulo, "{8BC4E8}GPS > Entregar Coches \"Federico\"");
            format(lista, sizeof lista,
                "Punto de entrega {8BC4E8}(LS)\n\
                Punto de entrega {8BC4E8}(Angel Pine)\n\
                {FF0000}Usa /verlista para ver la lista completa\n\
                {FF0000}Lista completa y hackear para verlo TODO: el juego/federico - el juego/coches");
        }
        case GPSCAT_NEGOCIOS:
        {
            // "Todos los negocios" (peticion de Vlad): cada ranura activa
            // de 32_negocios.inc, mismo orden que su propio catalogo.
            format(titulo, sizeof titulo, "{32FF32}GPS > Negocios");
            format(lista, sizeof lista,
                "{32FF32}(NEGOCIO){FFFFFF} Gasolinera Los Santos\n\
                {32FF32}(NEGOCIO){FFFFFF} Tienda Angel Pine\n\
                {32FF32}(NEGOCIO){FFFFFF} DGT\n\
                {32FF32}(NEGOCIO){FFFFFF} Bonos del Estado\n\
                {32FF32}(NEGOCIO){FFFFFF} Burger Shot\n\
                {32FF32}(NEGOCIO){FFFFFF} Cluckin' Bell\n\
                {32FF32}(NEGOCIO){FFFFFF} Well Stacked Pizza\n\
                {32FF32}(NEGOCIO){FFFFFF} Motel Angel Pine");
        }
        case GPSCAT_DINERO:
        {
            format(titulo, sizeof titulo, "{FF0000}GPS > GANAR DINERO $");
            // Nombres entre parentesis en rojo (peticion expresa de Vlad
            // 07/09/2026), para que resalten mas dentro de la lista.
            format(lista, sizeof lista,
                "{FF0000}(/Entregarcoche){FFFFFF} LS (Entrega coches por dinero)\n\
                {FF0000}(/Entregarcoche){FFFFFF} AngelPine (Entrega coches por dinero)\n\
                {FF0000}(Casa de empeños){FFFFFF} /Vender (Lo que robas de maleteros)\n\
                {FF0000}(Zona de Pesca){FFFFFF} Los Santos /Pescar\n\
                {FF0000}(Zona de pesca){FFFFFF} Angel Pine /Pescar\n\
                {FF0000}(Venta de pescado){FFFFFF} Los Santos /Venderpescado (Vendes tu pesca)\n\
                {FF0000}(Venta de pescado){FFFFFF} Angel Pine (Vendes tu pesca)\n\
                {FF0000}(Venta de pescado){FFFFFF} Tierra Robada (Vendes tu pesca)\n\
                {FF0000}(Vendedor de marihuana){FFFFFF} Los Santos /Aceptartrabajo (Planta Marihuana)\n\
                {FF0000}(Vendedor de armas){FFFFFF} Los Santos /Aceptartrabajo (Craftea armas)\n\
                {FF0000}(NFS){FFFFFF} Los Santos /Aceptartrabajo (Vende coches a jugadores)\n\
                {FF0000}(Medicos){FFFFFF} Los Santos /Aceptartrabajo (Cura a jugadores)\n\
                {FF0000}(Faccion){FFFFFF} Policia Nacional (Haz que se cumpla la ley y el codigo penal)\n\
                {FF0000}(Atracos){FFFFFF} /Atracar en comercios desprotegidos (busca el cartel rojo)\n\
                {FF0000}(Robo Express){FFFFFF} /Aceptarmision cuando aparezca el aviso ($1.000.000)");
        }
        case GPSCAT_TRABAJOS:
        {
            format(titulo, sizeof titulo, "{E8973A}GPS > Trabajos");
            format(lista, sizeof lista,
                "Vendedor de Armas {8BC4E8}(Aceptar trabajo)\n\
                Vendedor de Armas {8BC4E8}(Viaje de materiales)\n\
                Vendedor de Marihuana {8BC4E8}(Aceptar trabajo)\n\
                Recinto NFS\n\
                Paramedico {8BC4E8}(Aceptar trabajo)");
        }
        case GPSCAT_CRONOS:
        {
            new total = Cronos_ContarActivos();
            if (total == 0)
            {
                Error(playerid, "Todavia no hay ninguna crono activa.");
                return 1;
            }

            format(titulo, sizeof titulo, "{E05A5A}CRONOS -- elige uno para marcarlo");
            lista[0] = EOS;

            new linea[64], nombre[48], Float:x, Float:y, Float:z;
            #pragma unused x, y, z
            for (new i = 0; i < total; i++)
            {
                if (!Cronos_ObtenerParaGPS(i, x, y, z, nombre, sizeof nombre)) continue;
                format(linea, sizeof linea, "%s\n", nombre);
                strcat(lista, linea, sizeof lista);
            }
        }
    }

    ShowPlayerDialog(playerid, DIALOGO_GPS_DESTINO, DIALOG_STYLE_LIST, titulo, lista, "Ir", "Volver");
    return 1;
}

stock GPS_MarcarCheckpoint(playerid, Float:x, Float:y, Float:z, const nombre[])
{
    #pragma unused nombre
    SetPlayerCheckpoint(playerid, x, y, z, GPS_RADIO_CHECKPOINT);

    // Texto exclusivo del GPS (pedido expreso de Vlad, 14/09/2026): tag
    // "(GPS)" en azul en vez del prefijo generico "[Gamerol 2026]" de
    // Exito() -- mismo patron que "(Gamerol IA)" en 61_duda.inc.
    format(g_Buffer, sizeof g_Buffer, "%s(GPS){FFFFFF} Marcado en mapa.", HEX_AZUL);
    SendClientMessage(playerid, COLOR_BLANCO, g_Buffer);
    return 1;
}

// Puestos de venta de pescado (51_pesca.inc): posicion en tiempo real
// (corregible con /setventapescado), no un #define -- se resuelven aqui
// por indice (0 Tierra Robada, 1 Angel Pine, 2 Los Santos) en vez de
// vivir en un array "static const".
stock GPS_MarcarVenta(playerid, indice)
{
    new nombre[32];
    Pesca_VentaNombre(indice, nombre, sizeof nombre);
    GPS_MarcarCheckpoint(playerid, g_PescaVentaX[indice], g_PescaVentaY[indice], g_PescaVentaZ[indice], nombre);
    return 1;
}

// ---------------------------------------------------------------------
//  DIÁLOGO
// ---------------------------------------------------------------------
stock GPS_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext

    if (dialogid == DIALOGO_GPS)
    {
        if (!response) return 1;
        if (listitem < 0 || listitem >= GPSCAT_TOTAL) return 1;

        GPS_MostrarDestinos(playerid, listitem);
        return 1;
    }

    if (dialogid == DIALOGO_GPS_DESTINO)
    {
        if (!response)
        {
            GPS_MostrarCategorias(playerid);
            return 1;
        }

        switch (g_GpsCategoriaSel[playerid])
        {
            case GPSCAT_ROL: switch (listitem)
            {
                case 0: GPS_MarcarCheckpoint(playerid, BANCO_ENTRADA_X, BANCO_ENTRADA_Y, BANCO_ENTRADA_Z, "el Banco");
                case 1: GPS_MarcarCheckpoint(playerid, ANGELPINE_BANCO_X, ANGELPINE_BANCO_Y, ANGELPINE_BANCO_Z, "el Banco de Angel Pine");
                case 2: GPS_MarcarCheckpoint(playerid, TIENDA_EXT_X, TIENDA_EXT_Y, TIENDA_EXT_Z, "la Tienda de Los Santos");
                case 3: GPS_MarcarCheckpoint(playerid, TIENDA_ANGELPINE_EXT_X, TIENDA_ANGELPINE_EXT_Y, TIENDA_ANGELPINE_EXT_Z, "la Tienda de Angel Pine");
                case 4: GPS_MarcarCheckpoint(playerid, MOTEL_COMPRAR_X, MOTEL_COMPRAR_Y, MOTEL_COMPRAR_Z, "el Motel Angel Pine");
            }
            case GPSCAT_ACTIVIDADES: switch (listitem)
            {
                case 0: GPS_MarcarCheckpoint(playerid, PESCAGPS_LS_X, PESCAGPS_LS_Y, PESCAGPS_LS_Z, "la zona de pesca de Los Santos");
                case 1: GPS_MarcarCheckpoint(playerid, PESCAGPS_ANGELPINE_X, PESCAGPS_ANGELPINE_Y, PESCAGPS_ANGELPINE_Z, "la zona de pesca de Angel Pine");
                case 2: GPS_MarcarVenta(playerid, 2);   // Los Santos Bait Shop
                case 3: GPS_MarcarVenta(playerid, 1);   // Angel Pine Bait Shop
                case 4: GPS_MarcarVenta(playerid, 0);   // Tierra Robada Bait Shop
            }
            case GPSCAT_COMIDA: switch (listitem)
            {
                case 0: GPS_MarcarCheckpoint(playerid, COMIDA_LS_BURGERSHOT_X, COMIDA_LS_BURGERSHOT_Y, COMIDA_LS_BURGERSHOT_Z, "Burger Shot");
                case 1: GPS_MarcarCheckpoint(playerid, COMIDA_LS_CLUCKINBELL_X, COMIDA_LS_CLUCKINBELL_Y, COMIDA_LS_CLUCKINBELL_Z, "Cluckin' Bell (LS)");
                case 2: GPS_MarcarCheckpoint(playerid, COMIDA_LS_PIZZA_X, COMIDA_LS_PIZZA_Y, COMIDA_LS_PIZZA_Z, "Well Stacked Pizza");
                case 3: GPS_MarcarCheckpoint(playerid, COMIDA_CB2_X, COMIDA_CB2_Y, COMIDA_CB2_Z, "Cluckin' Bell (Angel Pine)");
                case 4: GPS_MarcarCheckpoint(playerid, PERRITO_X, PERRITO_Y, PERRITO_Z, "Dogs Angel Pine");
            }
            case GPSCAT_VEHICULOS: switch (listitem)
            {
                case 0: GPS_MarcarCheckpoint(playerid, GASOLINERA_LEGION_X, GASOLINERA_LEGION_Y, GASOLINERA_LEGION_Z, "la Gasolinera de Los Santos");
                case 1: GPS_MarcarCheckpoint(playerid, PETROLPUMP2_X, PETROLPUMP2_Y, PETROLPUMP2_Z, "la Gasolinera de Angel Pine");
                case 2:
                {
                    new Float:x, Float:y, Float:z;
                    if (Tuning_PrimerTransfenderPos(x, y, z))
                    {
                        GPS_MarcarCheckpoint(playerid, x, y, z, "Transfender");
                    }
                    else
                    {
                        Error(playerid, "Todavia no hay ningun Transfender marcado en /panel.");
                    }
                }
                case 3: GPS_MarcarCheckpoint(playerid, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z, "el Taller de la NFS");
                // Indices de g_Arreglarcoche* (01_configuracion.inc):
                // 0=Angel Pine, 1=Grove Street, 2=NFS, 3=Policia Nacional.
                // El "Taller NFS" de arriba (renovar papeles/vender) es un
                // punto distinto del Mecanico de NFS, aunque esten cerca.
                case 4: GPS_MarcarCheckpoint(playerid, g_ArreglarcocheX[1], g_ArreglarcocheY[1], g_ArreglarcocheZ[1], "el Mecanico de Grove Street");
                case 5: GPS_MarcarCheckpoint(playerid, g_ArreglarcocheX[0], g_ArreglarcocheY[0], g_ArreglarcocheZ[0], "el Mecanico de Angel Pine");
                case 6: GPS_MarcarCheckpoint(playerid, g_ArreglarcocheX[3], g_ArreglarcocheY[3], g_ArreglarcocheZ[3], "el Mecanico de Policia Nacional");
                case 7: GPS_MarcarCheckpoint(playerid, g_ArreglarcocheX[2], g_ArreglarcocheY[2], g_ArreglarcocheZ[2], "el Mecanico de NFS");
                case 8: GPS_MarcarCheckpoint(playerid, PETROLPUMP_CARRETERA_LSSF_X, PETROLPUMP_CARRETERA_LSSF_Y, PETROLPUMP_CARRETERA_LSSF_Z, "la Gasolinera Carretera LS-SF");
            }
            case GPSCAT_FEDERICO: switch (listitem)
            {
                case 0: GPS_MarcarCheckpoint(playerid, FEDERICO_X, FEDERICO_Y, FEDERICO_Z, "el punto de entrega de Federico (LS)");
                case 1: GPS_MarcarCheckpoint(playerid, FEDERICO_ANGELPINE_X, FEDERICO_ANGELPINE_Y, FEDERICO_ANGELPINE_Z, "el punto de entrega de Federico (Angel Pine)");
                // Filas 2 y 3 son solo texto informativo, no destinos --
                // se vuelve a mostrar la misma lista sin marcar nada.
                default: GPS_MostrarDestinos(playerid, GPSCAT_FEDERICO);
            }
            case GPSCAT_NEGOCIOS: switch (listitem)
            {
                case 0: GPS_MarcarCheckpoint(playerid, 1929.24, -1784.26, 13.5469, "la Gasolinera Los Santos");
                case 1: GPS_MarcarCheckpoint(playerid, TIENDA_ANGELPINE_COMPRAR_X, TIENDA_ANGELPINE_COMPRAR_Y, TIENDA_ANGELPINE_COMPRAR_Z, "la Tienda Angel Pine");
                case 2: GPS_MarcarCheckpoint(playerid, BANCO_ENTRADA_X, BANCO_ENTRADA_Y, BANCO_ENTRADA_Z, "la DGT");
                case 3: GPS_MarcarCheckpoint(playerid, BANCO_ENTRADA_X, BANCO_ENTRADA_Y, BANCO_ENTRADA_Z, "Bonos del Estado");
                case 4: GPS_MarcarCheckpoint(playerid, COMIDA_LS_BURGERSHOT_X, COMIDA_LS_BURGERSHOT_Y, COMIDA_LS_BURGERSHOT_Z, "Burger Shot");
                case 5: GPS_MarcarCheckpoint(playerid, COMIDA_LS_CLUCKINBELL_X, COMIDA_LS_CLUCKINBELL_Y, COMIDA_LS_CLUCKINBELL_Z, "Cluckin' Bell");
                case 6: GPS_MarcarCheckpoint(playerid, COMIDA_LS_PIZZA_X, COMIDA_LS_PIZZA_Y, COMIDA_LS_PIZZA_Z, "Well Stacked Pizza");
                case 7: GPS_MarcarCheckpoint(playerid, MOTEL_COMPRAR_X, MOTEL_COMPRAR_Y, MOTEL_COMPRAR_Z, "el Motel Angel Pine");
            }
            case GPSCAT_DINERO: switch (listitem)
            {
                case 0: GPS_MarcarCheckpoint(playerid, FEDERICO_X, FEDERICO_Y, FEDERICO_Z, "Federico (LS)");
                case 1: GPS_MarcarCheckpoint(playerid, FEDERICO_ANGELPINE_X, FEDERICO_ANGELPINE_Y, FEDERICO_ANGELPINE_Z, "Federico (Angel Pine)");
                case 2: GPS_MarcarCheckpoint(playerid, EMPENOS_EXT_X, EMPENOS_EXT_Y, EMPENOS_EXT_Z, "la Casa de Empeños");
                case 3: GPS_MarcarCheckpoint(playerid, PESCAGPS_LS_X, PESCAGPS_LS_Y, PESCAGPS_LS_Z, "la zona de pesca de Los Santos");
                case 4: GPS_MarcarCheckpoint(playerid, PESCAGPS_ANGELPINE_X, PESCAGPS_ANGELPINE_Y, PESCAGPS_ANGELPINE_Z, "la zona de pesca de Angel Pine");
                case 5: GPS_MarcarVenta(playerid, 2);   // Los Santos Bait Shop
                case 6: GPS_MarcarVenta(playerid, 1);   // Angel Pine Bait Shop
                case 7: GPS_MarcarVenta(playerid, 0);   // Tierra Robada Bait Shop -- faltaba (QoL 08/09/2026)
                case 8: GPS_MarcarCheckpoint(playerid, MVEND_JOB_X, MVEND_JOB_Y, MVEND_JOB_Z, "el Vendedor de Marihuana");
                case 9: GPS_MarcarCheckpoint(playerid, ARMAS_JOB_X, ARMAS_JOB_Y, ARMAS_JOB_Z, "el Vendedor de Armas");
                case 10: GPS_MarcarCheckpoint(playerid, NFS_JOB_X, NFS_JOB_Y, NFS_JOB_Z, "la NFS");
                // 13/09/2026: el trabajo de Medicos se contrata ahora DENTRO
                // del interior del hospital (11_trabajos.inc), asi que el GPS
                // lleva a la puerta de la calle (HOSPITAL_ENTRADA_X/Y/Z,
                // 01_configuracion.inc) donde esta el /entrar, no a
                // HOSPITAL_X/Y/Z (el punto de reaparicion tras morir).
                case 11: GPS_MarcarCheckpoint(playerid, HOSPITAL_ENTRADA_X, HOSPITAL_ENTRADA_Y, HOSPITAL_ENTRADA_Z, "Medicos Los Santos");
                case 12: GPS_MarcarCheckpoint(playerid, COMISARIA_EXT_X, COMISARIA_EXT_Y, COMISARIA_EXT_Z, "la Comisaria de Los Santos");
                // Atracos y Robo Express no tienen un punto fijo (locales
                // dinamicos / mision al azar) -- fila informativa, sin
                // checkpoint, mismo patron que las filas 2/3 de GPSCAT_FEDERICO.
                case 13: Info(playerid, "Atraca comercios desprotegidos con /atracar. Busca el cartel rojo con un \"!\" -- la ubicacion no es fija.");
                case 14: Info(playerid, "Cada 30 minutos puede aparecer un aviso de Robo Express. Usa /aceptarmision para aceptarlo -- paga 1.000.000$.");
            }
            case GPSCAT_TRABAJOS: switch (listitem)
            {
                case 0: GPS_MarcarCheckpoint(playerid, ARMAS_JOB_X, ARMAS_JOB_Y, ARMAS_JOB_Z, "el Vendedor de Armas");
                case 1: GPS_MarcarCheckpoint(playerid, ARMAS_MATS_PEDIR_X, ARMAS_MATS_PEDIR_Y, ARMAS_MATS_PEDIR_Z, "el viaje de materiales del Vendedor de Armas");
                case 2: GPS_MarcarCheckpoint(playerid, MVEND_JOB_X, MVEND_JOB_Y, MVEND_JOB_Z, "el Vendedor de Marihuana");
                case 3: GPS_MarcarCheckpoint(playerid, NFS_RECINTO_X, NFS_RECINTO_Y, NFS_RECINTO_Z, "el Recinto NFS");
                case 4: GPS_MarcarCheckpoint(playerid, HOSPITAL_ENTRADA_X, HOSPITAL_ENTRADA_Y, HOSPITAL_ENTRADA_Z, "el Paramedico");
            }
            case GPSCAT_CRONOS:
            {
                new Float:x, Float:y, Float:z, nombre[48];
                if (Cronos_ObtenerParaGPS(listitem, x, y, z, nombre, sizeof nombre))
                    GPS_MarcarCheckpoint(playerid, x, y, z, nombre);
            }
        }
        return 1;
    }

    return 0;
}

