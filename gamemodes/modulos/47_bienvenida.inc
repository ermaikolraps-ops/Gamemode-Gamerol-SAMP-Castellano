/* =====================================================================
 *  47_bienvenida.inc
 *  ---------------------------------------------------------------------
 *  Icono de bienvenida en el mundo, encargo de Vlad del 05/09/2026.
 *
 *  Una caja de regalo (modelo 19055, streamer) en la coordenada que Vlad
 *  marco con el panel de admin (tabla `coordenadas`, id 49). Al
 *  acercarse:
 *
 *    - Sale en pantalla, grandes, en la misma fuente Pricedown que el
 *      titulo de /federico: "Bienvenido a Gamerol 2026" y debajo, mas
 *      pequeno, "Gamerol".
 *    - Se ingresan 15.000$ en mano -- pero SOLO la primera vez por
 *      cuenta. El flag vive en `cuentas.bienvenida_reclamada` y se marca
 *      con un UPDATE atomico (mismo patron que /danijailovic en
 *      04_comandos.inc: el WHERE con la condicion en 0 es lo que hace
 *      imposible canjearlo dos veces, sin importar lag ni relog).
 *    - El TEXTO tiene cooldown de 1h, pero SILENCIOSO: si ya se enseño
 *      hace menos de una hora, simplemente no vuelve a salir. Sin aviso,
 *      sin mensaje de "espera" -- el jugador ni se entera de que hay
 *      cooldown, solo deja de ver el cartel un rato.
 * ===================================================================== */

static STREAMER_TAG_PICKUP:g_BienvenidaPickup;
static STREAMER_TAG_3D_TEXT_LABEL:g_BienvenidaEtiqueta;

enum
{
    BTD_TITULO,
    BTD_SUBTITULO,
    BTD_TOTAL
}
static PlayerText:g_BienvenidaTD[MAX_PLAYERS][BTD_TOTAL];
static bool:g_BienvenidaTDVisible[MAX_PLAYERS];

static g_BienvenidaUltimoTexto[MAX_PLAYERS];   // gettime() de la ultima vez que se enseño el texto
static bool:g_BienvenidaEsperandoBD[MAX_PLAYERS]; // evita disparar dos UPDATE mientras responde el primero

// ---------------------------------------------------------------------
//  ARRANQUE
// ---------------------------------------------------------------------
stock Bienvenida_Iniciar()
{
    g_BienvenidaPickup = CreateDynamicPickup(BIENVENIDA_ICONO_MODELO, 23,
        BIENVENIDA_X, BIENVENIDA_Y, BIENVENIDA_Z, 0, 0, -1, 100.0);

    g_BienvenidaEtiqueta = CreateDynamic3DTextLabel(
        "{E8B33A}Bienvenido a Gamerol\n{FFFFFF}Acercate para reclamar tu regalo",
        0xE8B33AFF, BIENVENIDA_X, BIENVENIDA_Y, BIENVENIDA_Z + 0.7, 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    return 1;
}

stock Bienvenida_OnConnect(playerid)
{
    g_BienvenidaUltimoTexto[playerid] = 0;
    g_BienvenidaEsperandoBD[playerid] = false;
    g_BienvenidaTDVisible[playerid] = false;
    Bienvenida_CrearTextdraws(playerid);
    return 1;
}

stock Bienvenida_OnDisconnect(playerid)
{
    Bienvenida_OcultarTexto(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  TEXTDRAWS -- mismo font que el titulo de /federico (Pricedown),
//  pedido expreso: "letras grandes, las mismas que usa /federico".
// ---------------------------------------------------------------------
stock Bienvenida_CrearTextdraws(playerid)
{
    g_BienvenidaTD[playerid][BTD_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 130.0, "Bienvenido a Gamerol 2026");
    PlayerTextDrawAlignment(playerid, g_BienvenidaTD[playerid][BTD_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_BienvenidaTD[playerid][BTD_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_BienvenidaTD[playerid][BTD_TITULO], 0.55, 2.4);
    PlayerTextDrawColour(playerid, g_BienvenidaTD[playerid][BTD_TITULO], 0xE8B33AFF);
    PlayerTextDrawSetOutline(playerid, g_BienvenidaTD[playerid][BTD_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_BienvenidaTD[playerid][BTD_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_BienvenidaTD[playerid][BTD_TITULO], true);

    // Mismo font que el titulo (PRICEDOWN), pero mas pequeño -- pedido
    // expreso: "abajo mas pequeño Gamerol".
    g_BienvenidaTD[playerid][BTD_SUBTITULO] = CreatePlayerTextDraw(playerid, 320.0, 158.0, "Gamerol");
    PlayerTextDrawAlignment(playerid, g_BienvenidaTD[playerid][BTD_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_BienvenidaTD[playerid][BTD_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_BienvenidaTD[playerid][BTD_SUBTITULO], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_BienvenidaTD[playerid][BTD_SUBTITULO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_BienvenidaTD[playerid][BTD_SUBTITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_BienvenidaTD[playerid][BTD_SUBTITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_BienvenidaTD[playerid][BTD_SUBTITULO], true);
    return 1;
}

stock Bienvenida_MostrarTexto(playerid)
{
    PlayerTextDrawShow(playerid, g_BienvenidaTD[playerid][BTD_TITULO]);
    PlayerTextDrawShow(playerid, g_BienvenidaTD[playerid][BTD_SUBTITULO]);
    g_BienvenidaTDVisible[playerid] = true;

    SetTimerEx("Bienvenida_OcultarTexto", BIENVENIDA_TEXTO_DURACION_MS, false, "d", playerid);
    return 1;
}

forward Bienvenida_OcultarTexto(playerid);
public Bienvenida_OcultarTexto(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    if (!g_BienvenidaTDVisible[playerid]) return 0;

    PlayerTextDrawHide(playerid, g_BienvenidaTD[playerid][BTD_TITULO]);
    PlayerTextDrawHide(playerid, g_BienvenidaTD[playerid][BTD_SUBTITULO]);
    g_BienvenidaTDVisible[playerid] = false;
    return 1;
}

// ---------------------------------------------------------------------
//  TICK -- llamado cada segundo real desde TicSegundo (99_callbacks.inc).
//  No usa OnPlayerPickUpDynamicPickup a proposito: el pickup es solo el
//  adorno visual (el diamante), la deteccion real es por distancia, asi
//  que funciona igual de bien acercarse a pie que en coche.
// ---------------------------------------------------------------------
stock Bienvenida_Segundo()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (!IsPlayerConnected(playerid)) continue;
        if (!Jugador[playerid][jLogueado]) continue;
        if (GetPlayerInterior(playerid) != 0 || GetPlayerVirtualWorld(playerid) != 0) continue;
        if (!EnPunto(playerid, BIENVENIDA_RADIO, BIENVENIDA_X, BIENVENIDA_Y, BIENVENIDA_Z)) continue;

        // Texto: cooldown silencioso de 1h, sin avisar de nada.
        if (gettime() - g_BienvenidaUltimoTexto[playerid] >= BIENVENIDA_TEXTO_COOLDOWN)
        {
            g_BienvenidaUltimoTexto[playerid] = gettime();
            Bienvenida_MostrarTexto(playerid);
        }

        // Dinero: una unica vez por cuenta, para siempre.
        if (!g_BienvenidaEsperandoBD[playerid])
        {
            g_BienvenidaEsperandoBD[playerid] = true;

            new consulta[150];
            format(consulta, sizeof consulta,
                "UPDATE cuentas SET bienvenida_reclamada = 1 WHERE id = %d AND bienvenida_reclamada = 0",
                Jugador[playerid][jID]);
            mysql_pquery(g_BD, consulta, "AlReclamarBienvenida", "d", playerid);
        }
    }
    return 1;
}

forward AlReclamarBienvenida(playerid);
public AlReclamarBienvenida(playerid)
{
    if (IsPlayerConnected(playerid)) g_BienvenidaEsperandoBD[playerid] = false;
    if (!IsPlayerConnected(playerid)) return 0;

    if (cache_affected_rows() == 0) return 0; // ya reclamado antes, sin avisar

    DarDinero(playerid, BIENVENIDA_RECOMPENSA);
    Formato("El icono de bienvenida te ha dado %d$ en mano. Solo pasa una vez, disfrutalo.", BIENVENIDA_RECOMPENSA);
    Exito(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "BIENVENIDA reclamada: cuenta %d (%s), %d$", Jugador[playerid][jID], Jugador[playerid][jNombre], BIENVENIDA_RECOMPENSA);
    EscribirLog("economia", g_Buffer);
    return 1;
}
