/* =====================================================================
 *  06_login.inc
 *  ---------------------------------------------------------------------
 *  Pantalla de entrada cinematográfica, registro e inicio de sesión.
 *
 *  Cómo funciona:
 *    1. El jugador conecta y pasa a modo espectador en un mundo privado.
 *    2. La cámara recorre Los Santos plano a plano.
 *    3. Encima se dibujan los textdraws con el logo y los botones.
 *    4. Los botones son clicables con el ratón; abren diálogos nativos
 *       para escribir la contraseña (Pawn no puede leer el teclado).
 *    5. Las contraseñas se guardan con bcrypt de coste 12. Nunca en claro.
 * ===================================================================== */

// Si pones aquí la URL de un .mp3 sonará de fondo en la pantalla de
// inicio. Déjalo vacío para no reproducir nada.
#define LOGIN_MUSICA            ""

// Punto donde se planta el personaje al crearlo. Coordenada puesta a
// mano desde /panel > Coordenadas (clave "poselegirskin") el 2026-08-19.
// La camara se calcula a partir de esta misma posicion y de CREACION_A:
// se coloca 3.4 unidades por delante de la CARA del personaje (en
// sentido CONTRARIO a hacia donde mira, no en el mismo -- probado en
// directo: con el signo al reves la camara quedaba detras, viendo la
// espalda) y mira hacia el mismo punto donde esta el personaje, asi
// queda justo enfrente, mirandolo de frente.
#define CREACION_X              1231.48
#define CREACION_Y              -1309.5
#define CREACION_Z              13.4362
#define CREACION_A              297.333
#define CREACION_CAM_X          1234.5004
#define CREACION_CAM_Y          -1311.0611
#define CREACION_CAM_Z          14.3362

// ---------------------------------------------------------------------
//  PLANOS DE LA CÁMARA
//  Añade o quita entradas libremente: el bucle se adapta solo.
// ---------------------------------------------------------------------
enum E_PLANO
{
    Float:plCamIni[3],
    Float:plCamFin[3],
    Float:plMiraIni[3],
    Float:plMiraFin[3],
    plZona[48]
}

static const g_Planos[][E_PLANO] =
{
    {{1470.0, -1250.0, 120.0}, {1620.0, -1400.0,  95.0}, {1550.0, -1350.0, 40.0}, {1500.0, -1300.0, 28.0}, "Downtown  ·  Los Santos"},
    {{2470.0, -1740.0,  45.0}, {2540.0, -1640.0,  32.0}, {2495.0, -1685.0, 14.0}, {2500.0, -1665.0, 14.0}, "Grove Street  ·  Ganton"},
    {{ 340.0, -2020.0,  35.0}, { 460.0, -1900.0,  26.0}, { 390.0, -2050.0,  9.0}, { 425.0, -1950.0,  9.0}, "Playa de Santa María"},
    {{1580.0, -2360.0,  65.0}, {1760.0, -2240.0,  42.0}, {1685.0, -2250.0, 15.0}, {1725.0, -2195.0, 15.0}, "Aeropuerto Internacional"},
    {{1230.0, -720.0,  135.0}, {1360.0, -590.0,  112.0}, {1300.0, -650.0,  90.0}, {1335.0, -615.0,  82.0}, "Mulholland  ·  Richman"},
    {{2660.0, -1740.0,  62.0}, {2810.0, -1590.0,  52.0}, {2760.0, -1620.0, 22.0}, {2785.0, -1600.0, 16.0}, "Estadio  ·  Los Santos"}
};

// ---------------------------------------------------------------------
//  ORÍGENES DEL PERSONAJE (sabor de rol, se muestra en /cuenta)
// ---------------------------------------------------------------------
static const g_Origenes[][] =
{
    "Málaga, España",
    "Madrid, España",
    "Sevilla, España",
    "Barcelona, España",
    "Buenos Aires, Argentina",
    "Ciudad de México, México",
    "Bogotá, Colombia",
    "Santiago, Chile",
    "La Habana, Cuba",
    "Los Santos, San Andreas"
};

// ---------------------------------------------------------------------
//  PIELES SELECCIONABLES
//  Sólo civiles: las de policía, médico, bombero y militar quedan
//  reservadas para las facciones.
// ---------------------------------------------------------------------
static const g_SkinsHombre[] =
{
    1, 2, 3, 4, 5, 6, 7, 8, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    27, 28, 29, 30, 32, 33, 34, 35, 36, 37, 43, 44, 45, 46, 47, 48, 49, 50, 51,
    52, 57, 58, 59, 60, 61, 62, 66, 67, 68, 70, 71, 72, 73, 78, 79, 80, 81, 82,
    83, 84, 86, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
    108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 120, 121, 122, 123, 124,
    125, 126, 127, 128, 132, 133, 134, 135, 136, 137, 142, 143, 144, 146, 147,
    153, 154, 155, 156, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
    170, 171, 173, 174, 175, 176, 177, 179, 180, 181, 182, 183, 184, 185, 186,
    187, 188, 189, 200, 202, 203, 204, 206, 209, 210, 212, 213, 217, 220, 221,
    222, 223, 227, 228, 229, 230, 234, 235, 236, 239, 240, 241, 242, 247, 248,
    249, 250, 252, 253, 254, 255, 258, 259, 260, 261, 262, 264, 268, 269, 270,
    271, 272, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299
};

static const g_SkinsMujer[] =
{
    9, 10, 11, 12, 13, 31, 38, 39, 40, 41, 53, 54, 55, 56, 63, 64, 69, 75, 76,
    77, 85, 87, 88, 89, 90, 91, 92, 93, 129, 130, 131, 138, 139, 140, 141, 145,
    148, 150, 151, 152, 157, 169, 172, 178, 190, 191, 192, 193, 194, 195, 196,
    197, 198, 199, 201, 205, 207, 211, 214, 215, 216, 218, 219, 224, 225, 226,
    231, 232, 233, 237, 238, 243, 244, 245, 246, 251, 256, 257, 263
};

// ---------------------------------------------------------------------
//  TEXTDRAWS
// ---------------------------------------------------------------------
enum
{
    TD_BARRA_SUP, TD_BARRA_INF, TD_LOGO, TD_SUB, TD_LINEA,
    TD_PANEL, TD_PIE,
    TD_BOTON1_CAJA, TD_BOTON2_CAJA, TD_BOTON3_CAJA,
    TD_BOTON2_TXT,  TD_BOTON3_TXT,
    TD_TOTAL
};

enum
{
    PTD_BIENVENIDA, PTD_NOMBRE, PTD_INFO, PTD_ZONA, PTD_JUGADORES, PTD_BOTON1_TXT,
    PTD_TOTAL
};

// Creación de personaje
enum
{
    CTD_PANEL, CTD_TITULO, CTD_AYUDA,
    CTD_IZQ, CTD_DER, CTD_SEXO, CTD_ACEPTAR,
    CTD_IZQ_TXT, CTD_DER_TXT, CTD_SEXO_TXT, CTD_ACEPTAR_TXT,
    CTD_TOTAL
};

static Text:g_TD[TD_TOTAL];
static Text:g_CTD[CTD_TOTAL];
static PlayerText:g_PTD[MAX_PLAYERS][PTD_TOTAL];
static PlayerText:g_PCTD[MAX_PLAYERS];          // Texto "Piel 12 de 200"

static g_TimerPlano[MAX_PLAYERS];
static g_PlanoActual[MAX_PLAYERS];
static g_SkinIndice[MAX_PLAYERS];
static bool:g_EnCreacion[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  CREACIÓN DE LOS TEXTDRAWS GLOBALES
// ---------------------------------------------------------------------
stock Login_CrearTextdraws()
{
    // --- Bandas cinematográficas ---------------------------------------
    g_TD[TD_BARRA_SUP] = TextDrawCreate(-10.0, -4.0, "_");
    TextDrawUseBox(g_TD[TD_BARRA_SUP], true);
    TextDrawBoxColour(g_TD[TD_BARRA_SUP], 0x000000E6);
    TextDrawTextSize(g_TD[TD_BARRA_SUP], 660.0, 0.0);
    TextDrawLetterSize(g_TD[TD_BARRA_SUP], 0.0, 6.4);
    TextDrawSetShadow(g_TD[TD_BARRA_SUP], 0);
    TextDrawSetOutline(g_TD[TD_BARRA_SUP], 0);

    g_TD[TD_BARRA_INF] = TextDrawCreate(-10.0, 396.0, "_");
    TextDrawUseBox(g_TD[TD_BARRA_INF], true);
    TextDrawBoxColour(g_TD[TD_BARRA_INF], 0x000000E6);
    TextDrawTextSize(g_TD[TD_BARRA_INF], 660.0, 0.0);
    TextDrawLetterSize(g_TD[TD_BARRA_INF], 0.0, 6.0);
    TextDrawSetShadow(g_TD[TD_BARRA_INF], 0);
    TextDrawSetOutline(g_TD[TD_BARRA_INF], 0);

    // --- Logotipo -------------------------------------------------------
    g_TD[TD_LOGO] = TextDrawCreate(320.0, 96.0, "Gamerol 2026");
    TextDrawAlignment(g_TD[TD_LOGO], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_TD[TD_LOGO], TEXT_DRAW_FONT_PRICEDOWN);
    TextDrawLetterSize(g_TD[TD_LOGO], 1.05, 3.75);
    TextDrawColour(g_TD[TD_LOGO], 0xE8B33AFF);
    TextDrawSetOutline(g_TD[TD_LOGO], 2);
    TextDrawBackgroundColour(g_TD[TD_LOGO], 0x000000FF);
    TextDrawSetProportional(g_TD[TD_LOGO], true);

    g_TD[TD_SUB] = TextDrawCreate(320.0, 144.0, "Encuentralos todos!");
    TextDrawAlignment(g_TD[TD_SUB], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_TD[TD_SUB], TEXT_DRAW_FONT_BANK_GOTHIC);
    TextDrawLetterSize(g_TD[TD_SUB], 0.21, 1.05);
    TextDrawColour(g_TD[TD_SUB], 0xE7E7E7FF);
    TextDrawSetOutline(g_TD[TD_SUB], 0);
    TextDrawSetShadow(g_TD[TD_SUB], 1);
    TextDrawBackgroundColour(g_TD[TD_SUB], 0x000000AA);
    TextDrawSetProportional(g_TD[TD_SUB], true);

    g_TD[TD_LINEA] = TextDrawCreate(240.0, 160.0, "_");
    TextDrawUseBox(g_TD[TD_LINEA], true);
    TextDrawBoxColour(g_TD[TD_LINEA], 0xE8B33AC8);
    TextDrawTextSize(g_TD[TD_LINEA], 400.0, 0.0);
    TextDrawLetterSize(g_TD[TD_LINEA], 0.0, 0.10);
    TextDrawSetShadow(g_TD[TD_LINEA], 0);
    TextDrawSetOutline(g_TD[TD_LINEA], 0);

    // --- Panel central --------------------------------------------------
    g_TD[TD_PANEL] = TextDrawCreate(196.0, 236.0, "_");
    TextDrawUseBox(g_TD[TD_PANEL], true);
    TextDrawBoxColour(g_TD[TD_PANEL], 0x0A0A0AC8);
    TextDrawTextSize(g_TD[TD_PANEL], 444.0, 0.0);
    TextDrawLetterSize(g_TD[TD_PANEL], 0.0, 16.6);
    TextDrawSetShadow(g_TD[TD_PANEL], 0);
    TextDrawSetOutline(g_TD[TD_PANEL], 0);

    // --- Pie de pantalla ------------------------------------------------
    g_TD[TD_PIE] = TextDrawCreate(320.0, 420.0, "SAMP 2026");
    TextDrawAlignment(g_TD[TD_PIE], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_TD[TD_PIE], TEXT_DRAW_FONT_AHARONI_BOLD);
    TextDrawLetterSize(g_TD[TD_PIE], 0.20, 0.95);
    TextDrawColour(g_TD[TD_PIE], 0x9A9A9AFF);
    TextDrawSetOutline(g_TD[TD_PIE], 0);
    TextDrawSetShadow(g_TD[TD_PIE], 0);
    TextDrawSetProportional(g_TD[TD_PIE], true);

    // --- Botones (caja clicable + texto centrado encima) ----------------
    Login_CrearBoton(TD_BOTON1_CAJA, 302.0, 0xE8B33A28);
    Login_CrearBoton(TD_BOTON2_CAJA, 326.0, 0x1E1E1EC8);
    Login_CrearBoton(TD_BOTON3_CAJA, 350.0, 0x1E1E1EC8);

    Login_CrearEtiquetaBoton(TD_BOTON2_TXT, 330.0, "NORMAS DEL SERVIDOR", 0xC9C9C9FF);
    Login_CrearEtiquetaBoton(TD_BOTON3_TXT, 354.0, "SALIR DEL SERVIDOR",  0xB07070FF);

    Creacion_CrearTextdraws();

    print("  [LOGIN] Pantalla de entrada preparada.");
    return 1;
}

// Caja rectangular clicable, de x=214 a x=426.
static stock Login_CrearBoton(indice, Float:y, color)
{
    g_TD[indice] = TextDrawCreate(214.0, y, "_");
    TextDrawUseBox(g_TD[indice], true);
    TextDrawBoxColour(g_TD[indice], color);
    TextDrawTextSize(g_TD[indice], 426.0, 20.0);
    TextDrawLetterSize(g_TD[indice], 0.0, 1.75);
    TextDrawSetShadow(g_TD[indice], 0);
    TextDrawSetOutline(g_TD[indice], 0);
    TextDrawSetSelectable(g_TD[indice], true);
    return 1;
}

static stock Login_CrearEtiquetaBoton(indice, Float:y, const texto[], color)
{
    g_TD[indice] = TextDrawCreate(320.0, y, texto);
    TextDrawAlignment(g_TD[indice], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_TD[indice], TEXT_DRAW_FONT_BANK_GOTHIC);
    TextDrawLetterSize(g_TD[indice], 0.23, 1.10);
    TextDrawColour(g_TD[indice], color);
    TextDrawSetOutline(g_TD[indice], 0);
    TextDrawSetShadow(g_TD[indice], 1);
    TextDrawBackgroundColour(g_TD[indice], 0x000000AA);
    TextDrawSetProportional(g_TD[indice], true);
    return 1;
}

// ---------------------------------------------------------------------
//  TEXTDRAWS POR JUGADOR
// ---------------------------------------------------------------------
stock Login_CrearTextdrawsJugador(playerid)
{
    g_PTD[playerid][PTD_BIENVENIDA] = CreatePlayerTextDraw(playerid, 320.0, 244.0, "_");
    PlayerTextDrawAlignment(playerid, g_PTD[playerid][PTD_BIENVENIDA], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_PTD[playerid][PTD_BIENVENIDA], TEXT_DRAW_FONT_BANK_GOTHIC);
    PlayerTextDrawLetterSize(playerid, g_PTD[playerid][PTD_BIENVENIDA], 0.19, 0.95);
    PlayerTextDrawColour(playerid, g_PTD[playerid][PTD_BIENVENIDA], 0xE8B33AFF);
    PlayerTextDrawSetOutline(playerid, g_PTD[playerid][PTD_BIENVENIDA], 0);
    PlayerTextDrawSetShadow(playerid, g_PTD[playerid][PTD_BIENVENIDA], 1);
    PlayerTextDrawBackgroundColour(playerid, g_PTD[playerid][PTD_BIENVENIDA], 0x000000AA);
    PlayerTextDrawSetProportional(playerid, g_PTD[playerid][PTD_BIENVENIDA], true);

    g_PTD[playerid][PTD_NOMBRE] = CreatePlayerTextDraw(playerid, 320.0, 258.0, "_");
    PlayerTextDrawAlignment(playerid, g_PTD[playerid][PTD_NOMBRE], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_PTD[playerid][PTD_NOMBRE], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_PTD[playerid][PTD_NOMBRE], 0.42, 1.75);
    PlayerTextDrawColour(playerid, g_PTD[playerid][PTD_NOMBRE], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_PTD[playerid][PTD_NOMBRE], 1);
    PlayerTextDrawBackgroundColour(playerid, g_PTD[playerid][PTD_NOMBRE], 0x000000FF);
    PlayerTextDrawSetProportional(playerid, g_PTD[playerid][PTD_NOMBRE], true);

    g_PTD[playerid][PTD_INFO] = CreatePlayerTextDraw(playerid, 320.0, 282.0, "_");
    PlayerTextDrawAlignment(playerid, g_PTD[playerid][PTD_INFO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_PTD[playerid][PTD_INFO], TEXT_DRAW_FONT_AHARONI_BOLD);
    PlayerTextDrawLetterSize(playerid, g_PTD[playerid][PTD_INFO], 0.20, 0.98);
    PlayerTextDrawColour(playerid, g_PTD[playerid][PTD_INFO], 0xA8A8A8FF);
    PlayerTextDrawSetOutline(playerid, g_PTD[playerid][PTD_INFO], 0);
    PlayerTextDrawSetShadow(playerid, g_PTD[playerid][PTD_INFO], 1);
    PlayerTextDrawBackgroundColour(playerid, g_PTD[playerid][PTD_INFO], 0x000000AA);
    PlayerTextDrawSetProportional(playerid, g_PTD[playerid][PTD_INFO], true);

    g_PTD[playerid][PTD_BOTON1_TXT] = CreatePlayerTextDraw(playerid, 320.0, 306.0, "_");
    PlayerTextDrawAlignment(playerid, g_PTD[playerid][PTD_BOTON1_TXT], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_PTD[playerid][PTD_BOTON1_TXT], TEXT_DRAW_FONT_BANK_GOTHIC);
    PlayerTextDrawLetterSize(playerid, g_PTD[playerid][PTD_BOTON1_TXT], 0.25, 1.15);
    PlayerTextDrawColour(playerid, g_PTD[playerid][PTD_BOTON1_TXT], 0xE8B33AFF);
    PlayerTextDrawSetOutline(playerid, g_PTD[playerid][PTD_BOTON1_TXT], 0);
    PlayerTextDrawSetShadow(playerid, g_PTD[playerid][PTD_BOTON1_TXT], 1);
    PlayerTextDrawBackgroundColour(playerid, g_PTD[playerid][PTD_BOTON1_TXT], 0x000000AA);
    PlayerTextDrawSetProportional(playerid, g_PTD[playerid][PTD_BOTON1_TXT], true);

    g_PTD[playerid][PTD_ZONA] = CreatePlayerTextDraw(playerid, 18.0, 402.0, "_");
    PlayerTextDrawFont(playerid, g_PTD[playerid][PTD_ZONA], TEXT_DRAW_FONT_BANK_GOTHIC);
    PlayerTextDrawLetterSize(playerid, g_PTD[playerid][PTD_ZONA], 0.20, 1.00);
    PlayerTextDrawColour(playerid, g_PTD[playerid][PTD_ZONA], 0xE8B33AFF);
    PlayerTextDrawSetOutline(playerid, g_PTD[playerid][PTD_ZONA], 0);
    PlayerTextDrawSetShadow(playerid, g_PTD[playerid][PTD_ZONA], 1);
    PlayerTextDrawBackgroundColour(playerid, g_PTD[playerid][PTD_ZONA], 0x000000AA);
    PlayerTextDrawSetProportional(playerid, g_PTD[playerid][PTD_ZONA], true);

    g_PTD[playerid][PTD_JUGADORES] = CreatePlayerTextDraw(playerid, 622.0, 402.0, "_");
    PlayerTextDrawAlignment(playerid, g_PTD[playerid][PTD_JUGADORES], TEXT_DRAW_ALIGN_RIGHT);
    PlayerTextDrawFont(playerid, g_PTD[playerid][PTD_JUGADORES], TEXT_DRAW_FONT_BANK_GOTHIC);
    PlayerTextDrawLetterSize(playerid, g_PTD[playerid][PTD_JUGADORES], 0.20, 1.00);
    PlayerTextDrawColour(playerid, g_PTD[playerid][PTD_JUGADORES], 0xC9C9C9FF);
    PlayerTextDrawSetOutline(playerid, g_PTD[playerid][PTD_JUGADORES], 0);
    PlayerTextDrawSetShadow(playerid, g_PTD[playerid][PTD_JUGADORES], 1);
    PlayerTextDrawBackgroundColour(playerid, g_PTD[playerid][PTD_JUGADORES], 0x000000AA);
    PlayerTextDrawSetProportional(playerid, g_PTD[playerid][PTD_JUGADORES], true);

    g_PCTD[playerid] = CreatePlayerTextDraw(playerid, 320.0, 330.0, "_");
    PlayerTextDrawAlignment(playerid, g_PCTD[playerid], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_PCTD[playerid], TEXT_DRAW_FONT_AHARONI_BOLD);
    PlayerTextDrawLetterSize(playerid, g_PCTD[playerid], 0.22, 1.05);
    PlayerTextDrawColour(playerid, g_PCTD[playerid], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_PCTD[playerid], 0);
    PlayerTextDrawSetShadow(playerid, g_PCTD[playerid], 1);
    PlayerTextDrawBackgroundColour(playerid, g_PCTD[playerid], 0x000000AA);
    PlayerTextDrawSetProportional(playerid, g_PCTD[playerid], true);
    return 1;
}

// ---------------------------------------------------------------------
//  ENTRADA DEL JUGADOR
// ---------------------------------------------------------------------
// Deja los identificadores en blanco antes de crear nada. Sin esto, un
// jugador recien conectado tendria ceros en el array y al "borrar" sus
// textdraws destruiria los del modulo de al lado.
stock Login_ReiniciarTextdraws(playerid)
{
    for (new i = 0; i < PTD_TOTAL; i++) g_PTD[playerid][i] = INVALID_PLAYER_TEXT_DRAW;
    g_PCTD[playerid] = INVALID_PLAYER_TEXT_DRAW;
    g_EnCreacion[playerid] = false;
    return 1;
}

// Lo consulta OnPlayerSpawn: durante la creacion del personaje el
// jugador aparece en el mundo aunque todavia no tenga cuenta.
stock bool:Login_EnCreacion(playerid)
{
    return g_EnCreacion[playerid];
}

// Score de adorno para quien todavia no ha entrado a su cuenta. Sin
// esto el listado de jugadores los ensena a todos con un 0. Se les da un
// numero bajo al azar dentro del margen de 01_configuracion.inc, y en
// cuanto se loguean AplicarDatosJugador lo sustituye por su nivel real.
stock Login_ScoreProvisional(playerid)
{
    // random(n) devuelve de 0 a n-1, asi que el margen lleva un +1 para
    // que el maximo configurado tambien pueda salir.
    new margen = SCORE_SIN_LOGUEAR_MAX - SCORE_SIN_LOGUEAR_MIN;
    if (margen < 0) margen = 0;

    SetPlayerScore(playerid, SCORE_SIN_LOGUEAR_MIN + random(margen + 1));
    return 1;
}

stock Login_Preparar(playerid)
{
    g_PlanoActual[playerid] = random(sizeof(g_Planos));
    g_TimerPlano[playerid]  = 0;
    g_EnCreacion[playerid]  = false;

    SetPlayerVirtualWorld(playerid, MUNDO_LOGIN);
    SetPlayerInterior(playerid, 0);
    TogglePlayerSpectating(playerid, true);
    SetPlayerWeather(playerid, 2);
    SetPlayerTime(playerid, 19, 30);
    Login_ScoreProvisional(playerid);

    Login_CrearTextdrawsJugador(playerid);
    Login_SiguientePlano(playerid);

    g_TimerPlano[playerid] = SetTimerEx("Login_SiguientePlano", TIEMPO_CAMARA_LOGIN, true, "d", playerid);

    if (strlen(LOGIN_MUSICA) > 0) PlayAudioStreamForPlayer(playerid, LOGIN_MUSICA);
    return 1;
}

forward Login_SiguientePlano(playerid);
public Login_SiguientePlano(playerid)
{
    if (!IsPlayerConnected(playerid) || Jugador[playerid][jLogueado]) return 0;

    new p = g_PlanoActual[playerid];

    InterpolateCameraPos(playerid,
        g_Planos[p][plCamIni][0], g_Planos[p][plCamIni][1], g_Planos[p][plCamIni][2],
        g_Planos[p][plCamFin][0], g_Planos[p][plCamFin][1], g_Planos[p][plCamFin][2],
        TIEMPO_CAMARA_LOGIN, CAMERA_MOVE);

    InterpolateCameraLookAt(playerid,
        g_Planos[p][plMiraIni][0], g_Planos[p][plMiraIni][1], g_Planos[p][plMiraIni][2],
        g_Planos[p][plMiraFin][0], g_Planos[p][plMiraFin][1], g_Planos[p][plMiraFin][2],
        TIEMPO_CAMARA_LOGIN, CAMERA_MOVE);

    PlayerTextDrawSetString(playerid, g_PTD[playerid][PTD_ZONA], g_Planos[p][plZona]);

    g_PlanoActual[playerid] = (p + 1) % sizeof(g_Planos);
    return 1;
}

// ---------------------------------------------------------------------
//  MOSTRAR Y OCULTAR LA PANTALLA
// ---------------------------------------------------------------------
public Login_MostrarPantalla(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);

    for (new i = 0; i < TD_TOTAL; i++) TextDrawShowForPlayer(playerid, g_TD[i]);

    PlayerTextDrawSetString(playerid, g_PTD[playerid][PTD_NOMBRE], nombre);

    if (Jugador[playerid][jRegistrado])
    {
        PlayerTextDrawSetString(playerid, g_PTD[playerid][PTD_BIENVENIDA], "BIENVENIDO DE NUEVO A");
        PlayerTextDrawSetString(playerid, g_PTD[playerid][PTD_BOTON1_TXT], "INICIAR SESION");

        new horas = Jugador[playerid][jMinutosTotales] / 60;
        format(g_Buffer, sizeof g_Buffer, "Nivel %d   ~   %d hora%s de juego   ~   %d jugador%s en linea",
            Jugador[playerid][jNivel], horas, (horas == 1) ? ("") : ("s"),
            g_Conectados, (g_Conectados == 1) ? ("") : ("es"));
        PlayerTextDrawSetString(playerid, g_PTD[playerid][PTD_INFO], g_Buffer);
    }
    else
    {
        PlayerTextDrawSetString(playerid, g_PTD[playerid][PTD_BIENVENIDA], "TU HISTORIA EMPIEZA AQUI");
        PlayerTextDrawSetString(playerid, g_PTD[playerid][PTD_BOTON1_TXT], "CREAR CUENTA NUEVA");
        PlayerTextDrawSetString(playerid, g_PTD[playerid][PTD_INFO], "Esta cuenta todavia no existe. Registrate para empezar a jugar.");
    }

    format(g_Buffer, sizeof g_Buffer, "%d / %d", g_Conectados, MAX_PLAYERS);
    PlayerTextDrawSetString(playerid, g_PTD[playerid][PTD_JUGADORES], g_Buffer);

    PlayerTextDrawShow(playerid, g_PTD[playerid][PTD_BIENVENIDA]);
    PlayerTextDrawShow(playerid, g_PTD[playerid][PTD_NOMBRE]);
    PlayerTextDrawShow(playerid, g_PTD[playerid][PTD_INFO]);
    PlayerTextDrawShow(playerid, g_PTD[playerid][PTD_BOTON1_TXT]);
    PlayerTextDrawShow(playerid, g_PTD[playerid][PTD_ZONA]);
    PlayerTextDrawShow(playerid, g_PTD[playerid][PTD_JUGADORES]);

    SelectTextDraw(playerid, 0xE8B33AC8);

    SendClientMessage(playerid, COLOR_MARCA, " ");
    SendClientMessage(playerid, COLOR_MARCA, "  " SERVIDOR_NOMBRE);
    SendClientMessage(playerid, COLOR_GRIS,  "  Usa el raton para elegir una opcion en la pantalla.");
    SendClientMessage(playerid, COLOR_MARCA, " ");
    return 1;
}

public Login_OcultarPantalla(playerid)
{
    for (new i = 0; i < TD_TOTAL; i++) TextDrawHideForPlayer(playerid, g_TD[i]);
    for (new i = 0; i < PTD_TOTAL; i++) PlayerTextDrawHide(playerid, g_PTD[playerid][i]);

    CancelSelectTextDraw(playerid);

    if (g_TimerPlano[playerid] != 0)
    {
        KillTimer(g_TimerPlano[playerid]);
        g_TimerPlano[playerid] = 0;
    }

    if (strlen(LOGIN_MUSICA) > 0) StopAudioStreamForPlayer(playerid);
    return 1;
}

stock Login_BorrarTextdrawsJugador(playerid)
{
    for (new i = 0; i < PTD_TOTAL; i++)
    {
        if (g_PTD[playerid][i] != INVALID_PLAYER_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, g_PTD[playerid][i]);
            g_PTD[playerid][i] = INVALID_PLAYER_TEXT_DRAW;
        }
    }
    if (g_PCTD[playerid] != INVALID_PLAYER_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, g_PCTD[playerid]);
        g_PCTD[playerid] = INVALID_PLAYER_TEXT_DRAW;
    }
    if (g_TimerPlano[playerid] != 0)
    {
        KillTimer(g_TimerPlano[playerid]);
        g_TimerPlano[playerid] = 0;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  CLICS EN LA PANTALLA DE ENTRADA
// ---------------------------------------------------------------------
stock Login_ClickTextDraw(playerid, Text:clickedid)
{
    // El jugador ha pulsado ESC: le devolvemos el cursor, no puede salirse.
    if (clickedid == INVALID_TEXT_DRAW)
    {
        if (!Jugador[playerid][jLogueado]) SelectTextDraw(playerid, 0xE8B33AC8);
        return 1;
    }

    if (g_EnCreacion[playerid]) return Creacion_ClickTextDraw(playerid, clickedid);

    if (Jugador[playerid][jLogueado]) return 0;

    if (clickedid == g_TD[TD_BOTON1_CAJA])
    {
        if (Jugador[playerid][jRegistrado]) Login_PedirPassword(playerid);
        else                                Login_PedirPasswordNueva(playerid);
        return 1;
    }

    if (clickedid == g_TD[TD_BOTON2_CAJA])
    {
        Login_MostrarNormas(playerid);
        return 1;
    }

    if (clickedid == g_TD[TD_BOTON3_CAJA])
    {
        Kick(playerid);
        return 1;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  DIÁLOGOS DE INICIO DE SESIÓN
// ---------------------------------------------------------------------
stock Login_PedirPassword(playerid)
{
    new cuerpo[512];
    format(cuerpo, sizeof cuerpo,
        "{FFFFFF}Bienvenido de nuevo a {E8B33A}" SERVIDOR_NOMBRE "{FFFFFF}.\n\n\
        Escribe la contrasena de tu cuenta para continuar.\n\n\
        {B4B5B7}Intentos restantes: {FFFFFF}%d{B4B5B7} de %d.",
        LOGIN_INTENTOS_MAX - Jugador[playerid][jIntentosLogin], LOGIN_INTENTOS_MAX);

    ShowPlayerDialog(playerid, DIALOGO_LOGIN, DIALOG_STYLE_PASSWORD,
        "{E8B33A}Iniciar sesion", cuerpo, "Entrar", "Salir");
    return 1;
}

stock Login_PedirPasswordNueva(playerid)
{
    new cuerpo[512];
    format(cuerpo, sizeof cuerpo,
        "{FFFFFF}Vas a crear la cuenta {E8B33A}%s{FFFFFF}.\n\n\
        Elige una contrasena de entre %d y %d caracteres.\n\n\
        {B4B5B7}Se guarda cifrada: ni los administradores pueden leerla.\n\
        No uses la misma que en otros sitios.",
        Jugador[playerid][jNombre], PASSWORD_MIN, PASSWORD_MAX);

    ShowPlayerDialog(playerid, DIALOGO_REGISTRO_PASS, DIALOG_STYLE_PASSWORD,
        "{E8B33A}Crear cuenta  ~  1 de 5", cuerpo, "Siguiente", "Cancelar");
    return 1;
}

stock Login_MostrarNormas(playerid)
{
    static const normas[] =
        "{E8B33A}NORMAS BASICAS DE " SERVIDOR_NOMBRE "\n\n\
        {FFFFFF}1.  Interpreta un personaje creible. Esto es un servidor de rol, no un deathmatch.\n\
        {FFFFFF}2.  Prohibido el {E05A5A}DM{FFFFFF} (matar sin motivo de rol) y el {E05A5A}DB{FFFFFF} (atropellar sin motivo).\n\
        {FFFFFF}3.  Prohibido el {E05A5A}Metagaming{FFFFFF}: usar informacion OOC dentro del rol.\n\
        {FFFFFF}4.  Prohibido el {E05A5A}Powergaming{FFFFFF}: acciones imposibles o forzar el rol ajeno.\n\
        {FFFFFF}5.  Si mueres, tu personaje pierde la memoria de la ultima media hora ({E8B33A}NLR{FFFFFF}).\n\
        {FFFFFF}6.  El chat {C2A2DA}/b{FFFFFF} es fuera de personaje. Uselo con moderacion.\n\
        {FFFFFF}7.  Respeta a los demas jugadores y al equipo administrativo.\n\
        {FFFFFF}8.  Prohibido cualquier programa de trampas o modificacion que de ventaja.\n\
        {FFFFFF}9.  Una cuenta por persona. No compartas tu contrasena.\n\
        {FFFFFF}10. Ambientacion: {E8B33A}Los Santos, 1992{FFFFFF}. Nada de tecnologia moderna.\n\n\
        {B4B5B7}El desconocimiento de las normas no exime de su cumplimiento.";

    ShowPlayerDialog(playerid, DIALOGO_NORMAS, DIALOG_STYLE_MSGBOX,
        "{E8B33A}Normas del servidor", normas, "Volver", "");
    return 1;
}

// ---------------------------------------------------------------------
//  VERIFICACIÓN DE CONTRASEÑA
// ---------------------------------------------------------------------
stock Login_ComprobarPassword(playerid, const introducida[])
{
    bcrypt_verify(playerid, "AlVerificarPassword", introducida, Jugador[playerid][jPassword]);
    return 1;
}

forward AlVerificarPassword(playerid, bool:correcta);
public AlVerificarPassword(playerid, bool:correcta)
{
    if (!IsPlayerConnected(playerid)) return 0;

    if (correcta)
    {
        BD_CargarCuenta(playerid);
        return 1;
    }

    Jugador[playerid][jIntentosLogin]++;

    if (Jugador[playerid][jIntentosLogin] >= LOGIN_INTENTOS_MAX)
    {
        new nombre[MAX_PLAYER_NAME + 1];
        GetPlayerName(playerid, nombre, sizeof nombre);
        format(g_Buffer, sizeof g_Buffer, "Demasiados intentos fallidos: %s", nombre);
        EscribirLog("seguridad", g_Buffer);

        Error(playerid, "Demasiados intentos fallidos. Vuelve a conectar.");
        SetTimerEx("ExpulsarRetardado", 700, false, "d", playerid);
        return 1;
    }

    Error(playerid, "Contrasena incorrecta.");
    Login_PedirPassword(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  HASHEO DE CONTRASEÑA
//  jTemporal2 indica qué hacer con el hash una vez calculado:
//     1 = registro nuevo    2 = cambio de contraseña desde /cuenta
// ---------------------------------------------------------------------
forward AlHashearPassword(playerid, hashid);
public AlHashearPassword(playerid, hashid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    bcrypt_get_hash(Jugador[playerid][jPassword], BCRYPT_HASH_LENGTH);

    if (Jugador[playerid][jTemporal2] == 2)
    {
        Jugador[playerid][jTemporal2] = 0;
        BD_GuardarPassword(playerid);
        Exito(playerid, "Contrasena cambiada correctamente.");
        return 1;
    }

    Jugador[playerid][jTemporal2] = 0;

    // Registro: seguimos con los datos del personaje.
    ShowPlayerDialog(playerid, DIALOGO_REGISTRO_EMAIL, DIALOG_STYLE_INPUT,
        "{E8B33A}Crear cuenta  ~  2 de 5",
        "{FFFFFF}Escribe un correo electronico de contacto.\n\n\
        {B4B5B7}Sirve para recuperar la cuenta si pierdes la contrasena.\n\
        Puedes dejarlo vacio y anadirlo mas adelante con {FFFFFF}/cuenta{B4B5B7}.",
        "Siguiente", "Omitir");
    return 1;
}

// ---------------------------------------------------------------------
//  CREACIÓN DE PERSONAJE: SELECTOR DE PIEL
// ---------------------------------------------------------------------
static stock Creacion_CrearTextdraws()
{
    g_CTD[CTD_PANEL] = TextDrawCreate(150.0, 296.0, "_");
    TextDrawUseBox(g_CTD[CTD_PANEL], true);
    TextDrawBoxColour(g_CTD[CTD_PANEL], 0x0A0A0AD2);
    TextDrawTextSize(g_CTD[CTD_PANEL], 490.0, 0.0);
    TextDrawLetterSize(g_CTD[CTD_PANEL], 0.0, 11.4);
    TextDrawSetShadow(g_CTD[CTD_PANEL], 0);
    TextDrawSetOutline(g_CTD[CTD_PANEL], 0);

    g_CTD[CTD_TITULO] = TextDrawCreate(320.0, 302.0, "ELIGE EL ASPECTO DE TU PERSONAJE");
    TextDrawAlignment(g_CTD[CTD_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_CTD[CTD_TITULO], TEXT_DRAW_FONT_BANK_GOTHIC);
    TextDrawLetterSize(g_CTD[CTD_TITULO], 0.24, 1.15);
    TextDrawColour(g_CTD[CTD_TITULO], 0xE8B33AFF);
    TextDrawSetOutline(g_CTD[CTD_TITULO], 0);
    TextDrawSetShadow(g_CTD[CTD_TITULO], 1);
    TextDrawBackgroundColour(g_CTD[CTD_TITULO], 0x000000AA);
    TextDrawSetProportional(g_CTD[CTD_TITULO], true);

    g_CTD[CTD_AYUDA] = TextDrawCreate(320.0, 348.0, "Puedes cambiar de aspecto mas adelante en las tiendas de ropa de la ciudad");
    TextDrawAlignment(g_CTD[CTD_AYUDA], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_CTD[CTD_AYUDA], TEXT_DRAW_FONT_AHARONI_BOLD);
    TextDrawLetterSize(g_CTD[CTD_AYUDA], 0.17, 0.85);
    TextDrawColour(g_CTD[CTD_AYUDA], 0x9A9A9AFF);
    TextDrawSetOutline(g_CTD[CTD_AYUDA], 0);
    TextDrawSetShadow(g_CTD[CTD_AYUDA], 1);
    TextDrawBackgroundColour(g_CTD[CTD_AYUDA], 0x000000AA);
    TextDrawSetProportional(g_CTD[CTD_AYUDA], true);

    Creacion_CrearBoton(CTD_IZQ,     158.0, 210.0, 0x1E1E1EC8);
    Creacion_CrearBoton(CTD_DER,     420.0, 472.0, 0x1E1E1EC8);
    Creacion_CrearBoton(CTD_SEXO,    216.0, 314.0, 0x1E1E1EC8);
    Creacion_CrearBoton(CTD_ACEPTAR, 320.0, 414.0, 0xE8B33A3C);

    Creacion_CrearEtiqueta(CTD_IZQ_TXT,     184.0, "<<",           0xFFFFFFFF);
    Creacion_CrearEtiqueta(CTD_DER_TXT,     446.0, ">>",           0xFFFFFFFF);
    Creacion_CrearEtiqueta(CTD_SEXO_TXT,    265.0, "CAMBIAR SEXO", 0xC9C9C9FF);
    Creacion_CrearEtiqueta(CTD_ACEPTAR_TXT, 367.0, "CONFIRMAR",    0xE8B33AFF);
    return 1;
}

static stock Creacion_CrearBoton(indice, Float:x1, Float:x2, color)
{
    g_CTD[indice] = TextDrawCreate(x1, 320.0, "_");
    TextDrawUseBox(g_CTD[indice], true);
    TextDrawBoxColour(g_CTD[indice], color);
    TextDrawTextSize(g_CTD[indice], x2, 20.0);
    TextDrawLetterSize(g_CTD[indice], 0.0, 1.75);
    TextDrawSetShadow(g_CTD[indice], 0);
    TextDrawSetOutline(g_CTD[indice], 0);
    TextDrawSetSelectable(g_CTD[indice], true);
    return 1;
}

static stock Creacion_CrearEtiqueta(indice, Float:x, const texto[], color)
{
    g_CTD[indice] = TextDrawCreate(x, 324.0, texto);
    TextDrawAlignment(g_CTD[indice], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_CTD[indice], TEXT_DRAW_FONT_BANK_GOTHIC);
    TextDrawLetterSize(g_CTD[indice], 0.23, 1.10);
    TextDrawColour(g_CTD[indice], color);
    TextDrawSetOutline(g_CTD[indice], 0);
    TextDrawSetShadow(g_CTD[indice], 1);
    TextDrawBackgroundColour(g_CTD[indice], 0x000000AA);
    TextDrawSetProportional(g_CTD[indice], true);
    return 1;
}

public Login_IniciarCreacionPersonaje(playerid)
{
    Login_OcultarPantalla(playerid);

    g_EnCreacion[playerid]  = true;
    g_SkinIndice[playerid]  = 0;

    TogglePlayerSpectating(playerid, false);
    SetSpawnInfo(playerid, NO_TEAM, Creacion_SkinActual(playerid),
        CREACION_X, CREACION_Y, CREACION_Z, CREACION_A, WEAPON_FIST, 0, WEAPON_FIST, 0, WEAPON_FIST, 0);
    SpawnPlayer(playerid);

    SetPlayerVirtualWorld(playerid, MUNDO_CREACION);
    SetPlayerInterior(playerid, 0);
    SetPlayerPos(playerid, CREACION_X, CREACION_Y, CREACION_Z);
    SetPlayerFacingAngle(playerid, CREACION_A);
    TogglePlayerControllable(playerid, false);
    SetPlayerWeather(playerid, 2);
    SetPlayerTime(playerid, 14, 0);

    SetPlayerCameraPos(playerid, CREACION_CAM_X, CREACION_CAM_Y, CREACION_CAM_Z);
    SetPlayerCameraLookAt(playerid, CREACION_X, CREACION_Y, CREACION_Z + 0.35);

    for (new i = 0; i < CTD_TOTAL; i++) TextDrawShowForPlayer(playerid, g_CTD[i]);
    Creacion_Actualizar(playerid);
    PlayerTextDrawShow(playerid, g_PCTD[playerid]);

    SelectTextDraw(playerid, 0xE8B33AC8);

    SendClientMessage(playerid, COLOR_MARCA, " ");
    Info(playerid, "Usa las flechas de la pantalla para elegir tu aspecto y pulsa CONFIRMAR.");
    return 1;
}

static stock Creacion_SkinActual(playerid)
{
    if (Jugador[playerid][jSexo] == 2)
    {
        return g_SkinsMujer[g_SkinIndice[playerid] % sizeof(g_SkinsMujer)];
    }
    return g_SkinsHombre[g_SkinIndice[playerid] % sizeof(g_SkinsHombre)];
}

static stock Creacion_TotalSkins(playerid)
{
    return (Jugador[playerid][jSexo] == 2) ? sizeof(g_SkinsMujer) : sizeof(g_SkinsHombre);
}

static stock Creacion_Actualizar(playerid)
{
    new skin = Creacion_SkinActual(playerid);
    Jugador[playerid][jSkin] = skin;
    SetPlayerSkin(playerid, skin);

    format(g_Buffer, sizeof g_Buffer, "%s   ~   Aspecto %d de %d   ~   Piel n%c %d",
        (Jugador[playerid][jSexo] == 2) ? ("Mujer") : ("Hombre"),
        g_SkinIndice[playerid] + 1, Creacion_TotalSkins(playerid), 186, skin);

    PlayerTextDrawSetString(playerid, g_PCTD[playerid], g_Buffer);
    return 1;
}

stock Creacion_ClickTextDraw(playerid, Text:clickedid)
{
    new total = Creacion_TotalSkins(playerid);

    if (clickedid == g_CTD[CTD_IZQ])
    {
        g_SkinIndice[playerid] = (g_SkinIndice[playerid] - 1 + total) % total;
        Creacion_Actualizar(playerid);
        return 1;
    }

    if (clickedid == g_CTD[CTD_DER])
    {
        g_SkinIndice[playerid] = (g_SkinIndice[playerid] + 1) % total;
        Creacion_Actualizar(playerid);
        return 1;
    }

    if (clickedid == g_CTD[CTD_SEXO])
    {
        Jugador[playerid][jSexo] = (Jugador[playerid][jSexo] == 1) ? 2 : 1;
        g_SkinIndice[playerid]   = 0;
        Creacion_Actualizar(playerid);
        return 1;
    }

    if (clickedid == g_CTD[CTD_ACEPTAR])
    {
        Creacion_Finalizar(playerid);
        return 1;
    }
    return 1;
}

static stock Creacion_Finalizar(playerid)
{
    g_EnCreacion[playerid] = false;

    for (new i = 0; i < CTD_TOTAL; i++) TextDrawHideForPlayer(playerid, g_CTD[i]);
    PlayerTextDrawHide(playerid, g_PCTD[playerid]);
    CancelSelectTextDraw(playerid);

    BD_CrearCuenta(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  FIN DEL PROCESO: EL JUGADOR ENTRA AL MUNDO
// ---------------------------------------------------------------------
public Login_TerminarSesion(playerid)
{
    Login_OcultarPantalla(playerid);

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);

    Jugador[playerid][jSpawneado] = true;
    SpawnJugadorEnMundo(playerid);

    // Sin esto, el jugador se queda con la hora/clima de la pantalla de
    // login o de creacion de personaje (SetPlayerTime/SetPlayerWeather,
    // mas arriba en este archivo) para siempre, ignorando el reloj/clima
    // real del mundo (60_climahora.inc, 10/09/2026). TicMinuto lo
    // reafirma cada minuto de todas formas, pero esto lo deja correcto
    // desde el primer segundo en vez de esperar hasta 60s.
    ClimaHora_SincronizarJugador(playerid);

    SendClientMessage(playerid, COLOR_MARCA, " ");
    format(g_Buffer, sizeof g_Buffer, PREFIJO "Bienvenido de nuevo, {FFFFFF}%s{B4B5B7}.", nombre);
    SendClientMessage(playerid, COLOR_GRIS, g_Buffer);

    new tiempo[48];
    FormatearMinutos(Jugador[playerid][jMinutosTotales], tiempo, sizeof tiempo);
    format(g_Buffer, sizeof g_Buffer, PREFIJO "Llevas {FFFFFF}%s{B4B5B7} de juego. Escribe {FFFFFF}/ayuda{B4B5B7} si necesitas algo.", tiempo);
    SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
    SendClientMessage(playerid, COLOR_MARCA, " ");

    GPS_MostrarTextdrawFijo(playerid);

    if (Jugador[playerid][jAdmin] > 0)
    {
        new rango[32];
        Admin_NombreNivel(Jugador[playerid][jAdmin], rango, sizeof rango);
        format(g_Buffer, sizeof g_Buffer, "Has entrado como {FFFFFF}%s{E8973A}. Escribe /admin para ver tus comandos.", rango);
        SendClientMessage(playerid, COLOR_ADMIN_ACCION, g_Buffer);
    }

    format(g_Buffer, sizeof g_Buffer, "%s ha iniciado sesion (id de cuenta %d)", nombre, Jugador[playerid][jID]);
    EscribirLog("conexiones", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  CONTROL DEL TIEMPO MÁXIMO PARA IDENTIFICARSE
// ---------------------------------------------------------------------
stock Login_Segundo(playerid)
{
    if (Jugador[playerid][jLogueado]) return 0;

    Jugador[playerid][jSegundosLogin]++;

    if (Jugador[playerid][jSegundosLogin] == LOGIN_SEGUNDOS_MAX - 30)
    {
        Aviso(playerid, "Te quedan 30 segundos para iniciar sesion antes de ser desconectado.");
    }
    else if (Jugador[playerid][jSegundosLogin] >= LOGIN_SEGUNDOS_MAX)
    {
        Error(playerid, "Has tardado demasiado en iniciar sesion.");
        Kick(playerid);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  RESPUESTAS A LOS DIÁLOGOS DE LOGIN Y REGISTRO
//  Devuelve 1 si ha gestionado el diálogo.
// ---------------------------------------------------------------------
stock Login_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    switch (dialogid)
    {
        // ---------------------------------------------------------- LOGIN
        case DIALOGO_LOGIN:
        {
            if (!response)
            {
                Kick(playerid);
                return 1;
            }
            if (strlen(inputtext) < 1)
            {
                Error(playerid, "Tienes que escribir tu contrasena.");
                Login_PedirPassword(playerid);
                return 1;
            }
            Login_ComprobarPassword(playerid, inputtext);
            return 1;
        }

        // ------------------------------------------------------- REGISTRO
        case DIALOGO_REGISTRO_PASS:
        {
            if (!response)
            {
                Login_MostrarPantalla(playerid);
                return 1;
            }

            new largo = strlen(inputtext);
            if (largo < PASSWORD_MIN || largo > PASSWORD_MAX)
            {
                format(g_Buffer, sizeof g_Buffer, "La contrasena debe tener entre %d y %d caracteres.", PASSWORD_MIN, PASSWORD_MAX);
                Error(playerid, g_Buffer);
                Login_PedirPasswordNueva(playerid);
                return 1;
            }

            format(Jugador[playerid][jTemporalTexto], 64, "%s", inputtext);

            ShowPlayerDialog(playerid, DIALOGO_REGISTRO_CONFIRMAR, DIALOG_STYLE_PASSWORD,
                "{E8B33A}Crear cuenta  ~  1 de 5",
                "{FFFFFF}Repite la contrasena para confirmarla.",
                "Confirmar", "Atras");
            return 1;
        }

        case DIALOGO_REGISTRO_CONFIRMAR:
        {
            if (!response)
            {
                Login_PedirPasswordNueva(playerid);
                return 1;
            }

            if (strcmp(inputtext, Jugador[playerid][jTemporalTexto], false) != 0)
            {
                Error(playerid, "Las contrasenas no coinciden.");
                Login_PedirPasswordNueva(playerid);
                return 1;
            }

            Jugador[playerid][jTemporal2] = 1;
            bcrypt_hash(playerid, "AlHashearPassword", inputtext, COSTE_BCRYPT);

            // Borramos la contrasena en claro de memoria en cuanto se usa.
            format(Jugador[playerid][jTemporalTexto], 64, "");
            Info(playerid, "Cifrando la contrasena, un momento...");
            return 1;
        }

        case DIALOGO_REGISTRO_EMAIL:
        {
            if (response && strlen(inputtext) > 0)
            {
                if (strfind(inputtext, "@") == -1 || strfind(inputtext, ".") == -1 || strlen(inputtext) > 60)
                {
                    Error(playerid, "Ese correo no parece valido.");
                    ShowPlayerDialog(playerid, DIALOGO_REGISTRO_EMAIL, DIALOG_STYLE_INPUT,
                        "{E8B33A}Crear cuenta  ~  2 de 5",
                        "{FFFFFF}Escribe un correo electronico de contacto.\n\n{B4B5B7}Puedes dejarlo vacio.",
                        "Siguiente", "Omitir");
                    return 1;
                }
                format(Jugador[playerid][jEmail], 64, "%s", inputtext);
                LimpiarTexto(Jugador[playerid][jEmail]);
            }

            ShowPlayerDialog(playerid, DIALOGO_REGISTRO_SEXO, DIALOG_STYLE_LIST,
                "{E8B33A}Crear cuenta  ~  3 de 5",
                "{FFFFFF}Hombre\n{FFFFFF}Mujer",
                "Siguiente", "");
            return 1;
        }

        case DIALOGO_REGISTRO_SEXO:
        {
            if (!response)
            {
                ShowPlayerDialog(playerid, DIALOGO_REGISTRO_SEXO, DIALOG_STYLE_LIST,
                    "{E8B33A}Crear cuenta  ~  3 de 5",
                    "{FFFFFF}Hombre\n{FFFFFF}Mujer", "Siguiente", "");
                return 1;
            }

            Jugador[playerid][jSexo] = (listitem == 1) ? 2 : 1;

            ShowPlayerDialog(playerid, DIALOGO_REGISTRO_EDAD, DIALOG_STYLE_INPUT,
                "{E8B33A}Crear cuenta  ~  4 de 5",
                "{FFFFFF}Que edad tiene tu personaje?\n\n\
                {B4B5B7}Es la edad dentro del rol, no la tuya.\n\
                Tiene que estar entre 18 y 80 anos.",
                "Siguiente", "");
            return 1;
        }

        case DIALOGO_REGISTRO_EDAD:
        {
            new edad = strval(inputtext);

            if (!response || !EsNumeroEntero(inputtext) || edad < 18 || edad > 80)
            {
                Error(playerid, "Escribe una edad entre 18 y 80.");
                ShowPlayerDialog(playerid, DIALOGO_REGISTRO_EDAD, DIALOG_STYLE_INPUT,
                    "{E8B33A}Crear cuenta  ~  4 de 5",
                    "{FFFFFF}Que edad tiene tu personaje?\n\n{B4B5B7}Entre 18 y 80 anos.",
                    "Siguiente", "");
                return 1;
            }

            Jugador[playerid][jEdad] = edad;

            new lista[512];
            for (new i = 0; i < sizeof(g_Origenes); i++)
            {
                format(lista, sizeof lista, "%s%s%s", lista, (i > 0) ? ("\n") : (""), g_Origenes[i]);
            }

            ShowPlayerDialog(playerid, DIALOGO_REGISTRO_ORIGEN, DIALOG_STYLE_LIST,
                "{E8B33A}Crear cuenta  ~  5 de 5", lista, "Siguiente", "");
            return 1;
        }

        case DIALOGO_REGISTRO_ORIGEN:
        {
            new indice = (response && listitem >= 0 && listitem < sizeof(g_Origenes)) ? listitem : 0;
            format(Jugador[playerid][jOrigen], 32, "%s", g_Origenes[indice]);

            Login_IniciarCreacionPersonaje(playerid);
            return 1;
        }

        case DIALOGO_NORMAS:
        {
            if (!Jugador[playerid][jLogueado]) SelectTextDraw(playerid, 0xE8B33AC8);
            return 1;
        }
    }
    return 0;
}
