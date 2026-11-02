/* =====================================================================
 *  64_HOSPITALINTERIOR.INC -- Hospital interior, PROTOTIPO (13/09/2026)
 *  ---------------------------------------------------------------------
 *  Encargo de Vlad (email 13/09/2026, "Estudio hospital interior spawn
 *  nuevos"): un interior de hospital sencillo, cerca del punto donde
 *  aparecen/respawnean los jugadores nuevos -- la puerta del hospital de
 *  Los Santos, misma coordenada que ya estaba guardada en /panel >
 *  Coordenadas con la clave "prttionn" ("entrada hospital /entrar").
 *  NO es un sistema medico: solo presencia y mundo, sin comandos,
 *  economia ni rol forzado propios.
 *
 *  MAPA: "Hospital Interior by El Marcapiel", descargado de
 *  https://pastebin.com/raw/ZgWifRgu (paquete preparado por Penny,
 *  SHA256 8c31656a900faa114ac72ab7f5948251ad8c7c034ba41f24d345082e4e19a4c4).
 *  Limpieza aplicada sobre el dump original:
 *    - Quitadas las 7 lineas CreateDynamicObjectEx(0, ...) -- modelo 0
 *      es invalido, restos del editor de origen (riesgo ya avisado por
 *      Penny).
 *    - Cada objeto pasa por Hospital_CrearObjeto() en vez de llamar a
 *      CreateDynamicObjectEx() directo: el dump original no fijaba
 *      mundo/interior, asi que con el valor por defecto del streamer
 *      (worlds[] = {-1} = "todos los mundos") los 215 objetos se
 *      habrian visto tambien en el mundo normal, donde juega todo el
 *      mundo -- riesgo NO detectado por Penny, corregido aqui.
 *    215 objetos finales (222 del dump original - 7 con modelo invalido).
 *
 *  AISLAMIENTO: interior 0, pero mundo virtual propio
 *  (MUNDO_HOSPITAL_INTERIOR) -- el mapa esta ademas en una zona sin usar
 *  del mundo real (X 2517-2593, Y 1260-1314, Z 864-871), sin interior
 *  numerado de SA-MP ni RemoveBuilding: no toca nada mas del mapa.
 *
 *  ENTRAR Y SALIR: pensado para colgarse del sistema generico
 *  CMD:entrar/CMD:salir (15_banco.inc), igual que Tienda/Policia/Casa/
 *  Comida/Empenos/Motel -- ver Hospital_IntentarEntrar/Salir mas abajo.
 *  /salir funciona desde CUALQUIER punto dentro del interior (pedido
 *  expreso de Vlad, 13/09/2026): solo comprueba el mundo virtual, sin
 *  radio de EnPunto -- mismo patron que Casa_IntentarSalir
 *  (24_casas.inc), no el de punto fijo de Banco_IntentarSalir.
 *
 *  ESTADO: PROTOTIPO AISLADO. Este archivo NO esta incluido en
 *  malagacity.pwn todavia -- a proposito, para no tocar produccion sin
 *  autorizacion expresa de Vlad. Ver INFORME-HOSPITAL-INTERIOR-20260913.md
 *  (Claude Claudia/ENTREGAS) para el dictamen y los 4 pasos exactos para
 *  activarlo cuando se autorice.
 * ===================================================================== */

// Puerta de la calle y MUNDO_HOSPITAL_INTERIOR: movidos a
// 01_configuracion.inc el 13/09/2026 -- GPS (30_gps.inc) y el trabajo de
// Medicos (11_trabajos.inc) los necesitan y ambos se incluyen ANTES que
// este modulo en malagacity.pwn. HOSPITAL_ENTRADA_X/Y/Z/A y
// MUNDO_HOSPITAL_INTERIOR siguen siendo los mismos nombres, solo cambian
// de sitio.

// Color del letrero de la calle -- rojo oscuro, distinto de COLOR_ROJO/
// COLOR_ROJO_FUERTE (01_configuracion.inc, ambos mas claros/vivos),
// pedido expreso de Vlad para "Hospital LS".
#define HOSPITAL_COLOR_ROJO_OSCURO 0x8B0000FF

// Punto de llegada dentro del interior (usado por Hospital_IntentarEntrar,
// mas abajo) -- estimado sobre el mapa original (zona abierta de la sala
// de espera/recepcion, lejos de los grupos de sillas y del mostrador). Z
// subida dos veces el 13/09/2026 tras pruebas de Vlad con cliente real:
// 865.6 -> 865.9 (seguia cayendo por el suelo) -> 866.9 (salto mas grande
// esta vez para dar margen de verdad, no otro ajuste minimo).
#define HOSPITAL_INTERIOR_X     2580.0
#define HOSPITAL_INTERIOR_Y     1290.0
#define HOSPITAL_INTERIOR_Z     866.9
#define HOSPITAL_INTERIOR_A     0.0

// Punto de reaparicion tras morir (usado desde 99_callbacks.inc,
// OnPlayerDeath) -- pedido expreso de Vlad 13/09/2026: reaparecer DENTRO
// del hospital en vez de en la calle. Coordenada marcada por Vlad en
// /panel (clave "ppoossiittionn"); Z = la del /entrar (arriba) + un
// margen pequeno, para no arriesgarse a caer por el suelo igual que paso
// con el punto de llegada del /entrar.
#define HOSPITAL_RESPAWN_X      2559.36
#define HOSPITAL_RESPAWN_Y      1269.43
#define HOSPITAL_RESPAWN_Z      (HOSPITAL_INTERIOR_Z + 0.3)
#define HOSPITAL_RESPAWN_A      88.7094

// Icono/letrero de "/salir" -- movido el 13/09/2026 a su propia
// coordenada (antes compartia sitio con el punto de llegada del
// /entrar). Marcada por Vlad en /panel (clave "ppoozzzzitionne").
#define HOSPITAL_SALIDA_ICONO_X 2583.86
#define HOSPITAL_SALIDA_ICONO_Y 1288.17
#define HOSPITAL_SALIDA_ICONO_Z 866.276

// Icono + letrero del menu de /tratamiento medico (14/09/2026, pedido
// expreso de Vlad) -- coordenada marcada en /panel (tabla `coordenadas`,
// clave "pos1", nombre "/tratamiento medico"). Solo marca el sitio: el
// comando /tratamiento funciona desde CUALQUIER punto dentro del mundo
// del hospital, mismo criterio que Hospital_IntentarSalir (no hace falta
// EnPunto porque solo hay un hospital, un mundo).
#define HOSPITAL_TRATAMIENTO_ICONO_X 2570.16
#define HOSPITAL_TRATAMIENTO_ICONO_Y 1289.68
#define HOSPITAL_TRATAMIENTO_ICONO_Z 866.276

static STREAMER_TAG_PICKUP:g_PickupHospitalFuera;
static STREAMER_TAG_PICKUP:g_PickupHospitalDentro;
static STREAMER_TAG_PICKUP:g_PickupHospitalTratamiento;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaHospitalFuera;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaHospitalDentro;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaHospitalTratamiento;

// ---------------------------------------------------------------------
//  /tratamiento -- menu de tratamientos medicos, mismo patron de dialogo
//  que Comida_MostrarMenu/Comida_Dialogo (42_comida.inc):
//  DIALOG_STYLE_TABLIST_HEADERS, animacion retrasada tras usar uno. A
//  diferencia de /comer, el menu SI se cierra solo tras aplicar un
//  tratamiento (pedido expreso de Vlad 14/09/2026). Cooldown compartido
//  entre los 3 tratamientos: 1 hora desde el ULTIMO usado.
//
//  Cooldown en gettime() (tiempo real, segundos), NO GetTickCount() --
//  arreglado 14/09/2026: GetTickCount() cuenta desde que arranco el
//  PROCESO del servidor, no desde que el jugador uso el tratamiento. Con
//  el valor por defecto (0) y un cooldown de una hora en milisegundos,
//  cualquier jugador que probara /tratamiento durante la primera hora
//  tras un reinicio veia un cooldown falso (bug real, no era una prueba
//  con la cuenta de Vlad). gettime() es el mismo patron que ya usa
//  FEDERICO_COOLDOWN (01_configuracion.inc) para sus 24h reales: con
//  0 por defecto, gettime() (ano 2026, timestamp enorme) siempre es mayor,
//  asi que un jugador que nunca lo uso nunca esta "en cooldown", y el
//  cooldown real sobrevive un reinicio del servidor sin resetearse ni
//  dispararse en falso.
// ---------------------------------------------------------------------
#define HOSPITAL_TOTAL_TRATAMIENTOS        3
#define HOSPITAL_TRATAMIENTO_COOLDOWN_SEGUNDOS 3600
#define HOSPITAL_TRATAMIENTO_ANIMACION_RETRASO_MS 300

static const g_TratamientoNombre[HOSPITAL_TOTAL_TRATAMIENTOS][32] =
{
    "Marihuana medicinal", "Medicina convencional", "Medicina experimental"
};
// Color de cada fila del dialogo (embebido en el texto) -- pedido exacto
// de Vlad: verde / rojo / morado, mismos HEX_* de 01_configuracion.inc.
static const g_TratamientoColorHex[HOSPITAL_TOTAL_TRATAMIENTOS][16] =
{
    HEX_VERDE, HEX_ROJO, HEX_MORADO
};
static const g_TratamientoPrecio[HOSPITAL_TOTAL_TRATAMIENTOS] =
{
    100, 200, 300
};
// % de armadura al que se FIJA (no se suma) -- la vida siempre sube al
// 100% en los 3, pedido tal cual por Vlad.
static const Float:g_TratamientoEscudo[HOSPITAL_TOTAL_TRATAMIENTOS] =
{
    10.0, 20.0, 30.0
};

// gettime() hasta el que el jugador esta en cooldown, 0 = nunca lo uso.
static g_TratamientoCooldownHasta[MAX_PLAYERS];

// Aviso en pantalla al aplicar un tratamiento -- pedido expreso de Vlad
// 14/09/2026: NO el GameText grande de "PUENTE EXITOSO" (estilo 3), sino
// el mismo textdraw titulo+subtitulo que se ve MIENTRAS se hace el
// /puente (Dropeo_CrearTextdrawsPuente, 22_dropeo.inc) -- misma posicion,
// fuente y tamanos, solo que aqui el texto es fijo y se autooculta a los
// 2s en vez de hacer cuenta atras.
enum { HTRAT_TITULO, HTRAT_SUBTITULO, HTRAT_TOTAL }
static PlayerText:g_HospitalTratamientoTD[MAX_PLAYERS][HTRAT_TOTAL];
#define HOSPITAL_TRATAMIENTO_AVISO_DURACION_MS 2000

stock Hospital_OnConectar(playerid)
{
    g_HospitalTratamientoTD[playerid][HTRAT_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 380.0, "TRATAMIENTO APLICADO");
    PlayerTextDrawAlignment(playerid, g_HospitalTratamientoTD[playerid][HTRAT_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_HospitalTratamientoTD[playerid][HTRAT_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_HospitalTratamientoTD[playerid][HTRAT_TITULO], 0.42, 1.8);
    PlayerTextDrawColour(playerid, g_HospitalTratamientoTD[playerid][HTRAT_TITULO], COLOR_VERDE);
    PlayerTextDrawSetOutline(playerid, g_HospitalTratamientoTD[playerid][HTRAT_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_HospitalTratamientoTD[playerid][HTRAT_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_HospitalTratamientoTD[playerid][HTRAT_TITULO], true);

    g_HospitalTratamientoTD[playerid][HTRAT_SUBTITULO] = CreatePlayerTextDraw(playerid, 320.0, 402.0, "Te encuentras mejor");
    PlayerTextDrawAlignment(playerid, g_HospitalTratamientoTD[playerid][HTRAT_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_HospitalTratamientoTD[playerid][HTRAT_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_HospitalTratamientoTD[playerid][HTRAT_SUBTITULO], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_HospitalTratamientoTD[playerid][HTRAT_SUBTITULO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_HospitalTratamientoTD[playerid][HTRAT_SUBTITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_HospitalTratamientoTD[playerid][HTRAT_SUBTITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_HospitalTratamientoTD[playerid][HTRAT_SUBTITULO], true);
    return 1;
}

// ---------------------------------------------------------------------
//  OBJETOS DEL MAPA
// ---------------------------------------------------------------------
stock STREAMER_TAG_OBJECT:Hospital_CrearObjeto(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
{
    new worlds[1] = {MUNDO_HOSPITAL_INTERIOR};
    return CreateDynamicObjectEx(modelid, x, y, z, rx, ry, rz, STREAMER_OBJECT_SD, STREAMER_OBJECT_DD, worlds);
}

stock HospitalInterior_CrearObjetos()
{
    new STREAMER_TAG_OBJECT:Textura;

Textura = Hospital_CrearObjeto(19456, 2543.91602, 1290.16479, 867.44934,   0.00000, 0.00000, 0.00000); //Textura Pared de Madera
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_wallPanel", -1);
Textura = Hospital_CrearObjeto(19456, 2543.91602, 1280.52502, 867.44928,   0.00000, 0.00000, 0.00000); //Textura Pared de Madera
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_wallPanel", -1);
Textura = Hospital_CrearObjeto(19456, 2543.91602, 1290.16479, 870.95422,   0.00000, 0.00000, 0.00000); //Textura Pared de Madera
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_wallPanel", -1);
Textura = Hospital_CrearObjeto(19456, 2543.91602, 1280.52502, 870.95422,   0.00000, 0.00000, 0.00000); //Textura Pared de Madera
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_wallPanel", -1);

Textura = Hospital_CrearObjeto(19404, 2543.91846, 1285.51343, 864.02179,   -180.00000, 0.00000, 0.00000);//Borde bajo de la pared
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_SkirtingGrey", -1);
Textura = Hospital_CrearObjeto(19404, 2543.91846, 1288.71997, 864.02179,   -180.00000, 0.00000, 0.00000);//Borde bajo de la pared
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_SkirtingGrey", -1);
Textura = Hospital_CrearObjeto(19404, 2543.91846, 1291.93005, 864.02179,   -180.00000, 0.00000, 0.00000);//Borde bajo de la pared
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_SkirtingGrey", -1);
Textura = Hospital_CrearObjeto(19404, 2543.91846, 1292.93005, 864.02179,   -180.00000, 0.00000, 0.00000);//Borde bajo de la pared
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_SkirtingGrey", -1);

Textura = Hospital_CrearObjeto(19456, 2537.29883, 1296.98975, 866.94006,   0.00000, 0.00000, 90.00000); //Paredes del /equipo
SetDynamicObjectMaterial(Textura, 0, 14597, "paperchaseoffice", "ab_wallPanel", -1);
Textura = Hospital_CrearObjeto(19456, 2527.66797, 1296.98975, 866.94012,   0.00000, 0.00000, 90.00000); //Paredes del /equipo
SetDynamicObjectMaterial(Textura, 0, 14597, "paperchaseoffice", "ab_wallPanel", -1);
Textura = Hospital_CrearObjeto(19456, 2527.66797, 1296.98975, 870.43591,   0.00000, 0.00000, 90.00000); //Paredes del /equipo
SetDynamicObjectMaterial(Textura, 0, 14597, "paperchaseoffice", "ab_wallPanel", -1);
Textura = Hospital_CrearObjeto(19456, 2537.29883, 1296.98975, 870.43591,   0.00000, 0.00000, 90.00000); //Paredes del /equipo
SetDynamicObjectMaterial(Textura, 0, 14597, "paperchaseoffice", "ab_wallPanel", -1);

Textura = Hospital_CrearObjeto(19404, 2540.55713, 1296.99536, 864.04468,   180.00000, 0.00000, 90.00000);//Borde bajo de la pared /equipo
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_SkirtingGrey", -1);
Textura = Hospital_CrearObjeto(19404, 2537.35327, 1296.99536, 864.04468,   180.00000, 0.00000, 90.00000);//Borde bajo de la pared /equipo
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_SkirtingGrey", -1);
Textura = Hospital_CrearObjeto(19404, 2534.15527, 1296.99536, 864.04468,   180.00000, 0.00000, 90.00000);//Borde bajo de la pared /equipo
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_SkirtingGrey", -1);
Textura = Hospital_CrearObjeto(19404, 2530.98901, 1296.99536, 864.04468,   180.00000, 0.00000, 90.00000);//Borde bajo de la pared /equipo
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_SkirtingGrey", -1);
Textura = Hospital_CrearObjeto(19404, 2527.79248, 1296.99536, 864.04468,   180.00000, 0.00000, 90.00000);//Borde bajo de la pared /equipo
SetDynamicObjectMaterial(Textura, 0, 14594, "papaerchaseoffice", "ab_SkirtingGrey", -1);



//Objetos sin texturas
Hospital_CrearObjeto(14597, 2530.87329, 1300.26855, 867.28973,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(14597, 2530.87329, 1300.26855, 867.28864,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(14594, 2565.38232, 1288.89966, 865.26050,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1569, 2584.80908, 1289.51782, 865.27338,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(1569, 2584.80908, 1286.51575, 865.27338,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(1491, 2578.69531, 1308.85571, 865.26703,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(1491, 2578.69531, 1297.36035, 865.26703,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(1491, 2565.28589, 1305.83130, 865.26703,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19449, 2588.86450, 1297.60266, 869.00537,   180.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1491, 2588.85474, 1302.39404, 865.26019,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(19449, 2588.86450, 1297.60266, 864.65918,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19430, 2588.86450, 1303.22412, 869.50269,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2202, 2593.36865, 1295.98560, 865.27289,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(1663, 2590.66943, 1296.65002, 865.70319,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(1671, 2590.57324, 1300.38159, 865.70319,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(1706, 2573.87476, 1277.25195, 865.27289,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(1706, 2577.99463, 1281.93494, 865.27289,   0.00000, 0.00000, 270.00000);
Hospital_CrearObjeto(1706, 2576.74585, 1277.19568, 865.27289,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(1706, 2571.40942, 1280.49243, 865.27289,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2370, 2574.39526, 1280.55286, 865.00891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2059, 2574.64990, 1280.77258, 865.87811,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2267, 2570.89185, 1281.12842, 867.63452,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2284, 2578.14551, 1281.72180, 867.34650,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(2283, 2574.85498, 1276.62793, 867.53851,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(19430, 2567.51611, 1296.80115, 866.02258,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2528, 2566.89648, 1296.50085, 865.27252,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2159, 2561.52686, 1304.54126, 865.27539,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2518, 2561.95801, 1304.18298, 865.52863,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(19430, 2565.82031, 1296.80115, 866.02258,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19430, 2564.12427, 1296.80115, 866.02258,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19430, 2562.45630, 1296.80115, 866.02258,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2528, 2564.96851, 1296.50085, 865.27252,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2528, 2563.23657, 1296.50085, 865.27252,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2159, 2561.52686, 1303.86926, 865.27539,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2159, 2561.52686, 1303.19727, 865.27539,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2159, 2561.52686, 1302.54932, 865.27539,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2159, 2561.52686, 1301.87732, 865.27539,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2159, 2561.52686, 1301.22937, 865.27539,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2159, 2561.52686, 1300.58142, 865.27539,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2159, 2561.52686, 1299.93335, 865.27539,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2518, 2561.95801, 1303.17505, 865.52863,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2518, 2561.95801, 1302.29907, 865.52863,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2518, 2561.95801, 1301.38708, 865.52863,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2518, 2561.95801, 1300.33105, 865.52863,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(3386, 2593.32080, 1297.36255, 865.27258,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(14855, 2567.71411, 1294.78333, 869.68689,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2252, 2567.71875, 1289.72095, 866.41046,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2252, 2570.18237, 1293.39685, 866.41046,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2273, 2571.43457, 1280.89404, 867.12164,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(1806, 2567.82764, 1292.16418, 865.27258,   0.00000, 0.00000, -124.68000);
Hospital_CrearObjeto(2614, 2581.86108, 1293.84436, 868.09241,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19825, 2569.69092, 1293.86816, 868.39117,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2206, 2568.19067, 1290.72925, 865.03284,   0.00000, 0.00000, 53.93998);
Hospital_CrearObjeto(1997, 2571.45972, 1296.81555, 865.26984,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1997, 2571.44019, 1300.38599, 865.26984,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1997, 2571.45239, 1304.43884, 865.26984,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1775, 2571.38501, 1278.60815, 866.43658,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(19813, 2568.32007, 1293.89001, 865.57819,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19813, 2578.69165, 1283.24890, 865.55029,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(2603, 2552.01758, 1265.11426, 865.71112,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2603, 2555.36060, 1265.11597, 865.71112,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2603, 2558.73926, 1265.17786, 865.71112,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2603, 2561.96045, 1265.18579, 865.71112,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2603, 2565.62842, 1265.06677, 865.71112,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2603, 2568.49048, 1265.07544, 865.71112,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2603, 2572.17334, 1265.05188, 865.71112,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2603, 2575.39502, 1265.03210, 865.71112,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2603, 2575.11938, 1273.97827, 865.71112,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2603, 2571.99390, 1274.15491, 865.71112,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2603, 2568.54858, 1274.16028, 865.71112,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2603, 2565.18628, 1274.20178, 865.71112,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2603, 2561.94385, 1274.17664, 865.71112,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2603, 2558.82153, 1274.29248, 865.71112,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2603, 2555.27808, 1274.25085, 865.71112,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2603, 2552.03564, 1274.21033, 865.71112,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2700, 2571.17554, 1283.30872, 868.08899,   0.00000, 0.00000, -1.31995);
Hospital_CrearObjeto(1997, 2557.71069, 1293.39490, 865.26978,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(1661, 2555.97241, 1269.83081, 868.99146,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1661, 2563.53101, 1269.84338, 868.99146,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1661, 2570.73145, 1269.90393, 868.99146,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(11706, 2579.14404, 1293.60327, 865.27643,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(11728, 2534.52271, 1297.30908, 866.85126,   0.00000, 0.00000, -180.00000);
Hospital_CrearObjeto(19826, 2570.27441, 1283.90710, 867.10468,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(19826, 2579.40527, 1276.97302, 866.98737,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(19826, 2564.83130, 1305.38428, 866.78326,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19826, 2579.29224, 1309.38818, 866.82635,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(19826, 2548.70044, 1275.38757, 866.83282,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1998, 2585.95630, 1312.08398, 865.27228,   0.00000, 0.00000, 143.93999);
Hospital_CrearObjeto(19813, 2586.99585, 1312.44580, 865.60803,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(633, 2580.04492, 1313.65686, 866.26062,   0.00000, 0.00000, -1.02000);
Hospital_CrearObjeto(19835, 2585.61890, 1312.02393, 866.17389,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2685, 2561.39893, 1302.79700, 867.13257,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2686, 2561.40283, 1302.10669, 867.10626,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2686, 2567.91553, 1305.93359, 867.37653,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(1661, 2583.39551, 1309.82581, 868.89795,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19994, 2583.80908, 1310.77844, 865.27295,   0.00000, 0.00000, 143.39996);
Hospital_CrearObjeto(19994, 2584.67944, 1310.22009, 865.27295,   0.00000, 0.00000, 161.33992);
Hospital_CrearObjeto(1671, 2585.31738, 1312.62561, 865.71222,   0.00000, 0.00000, -34.26000);
Hospital_CrearObjeto(2796, 2566.26782, 1305.83777, 868.27441,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1742, 2579.18091, 1310.13538, 865.27319,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(1742, 2579.19043, 1311.75757, 865.27319,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(956, 2544.46240, 1281.06873, 865.66650,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(932, 2544.41284, 1298.56824, 865.25549,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(932, 2553.00537, 1263.80200, 865.27228,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(932, 2563.29785, 1264.01001, 865.27228,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(932, 2574.19043, 1264.08740, 865.27228,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(932, 2573.78979, 1275.07397, 865.27094,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(932, 2566.90088, 1274.87988, 865.27094,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(932, 2563.71411, 1274.99036, 865.27094,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(932, 2553.75781, 1274.88855, 865.27094,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(932, 2559.54175, 1293.51636, 865.27161,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2174, 2586.53198, 1306.84570, 865.27283,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(2146, 2582.81470, 1305.20422, 865.74658,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2611, 2568.13940, 1293.85327, 867.27167,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1738, 2586.86890, 1308.90344, 865.81073,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2286, 2582.23901, 1314.31189, 867.62909,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2289, 2586.98242, 1311.54041, 867.53467,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(2286, 2583.19409, 1304.65283, 867.86285,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(1722, 2584.53394, 1303.86450, 865.27130,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(1722, 2583.16211, 1303.85815, 865.27130,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(1722, 2579.34229, 1300.48877, 865.27130,   0.00000, 0.00000, 270.00000);
Hospital_CrearObjeto(1722, 2579.39502, 1298.82837, 865.27130,   0.00000, 0.00000, 270.00000);
Hospital_CrearObjeto(2813, 2586.14795, 1312.15649, 866.10260,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19808, 2568.94067, 1291.98901, 865.98737,   0.00000, 0.00000, -98.69997);
Hospital_CrearObjeto(19873, 2561.62256, 1299.68286, 866.39899,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19874, 2561.69751, 1301.37195, 866.32507,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19874, 2561.61401, 1303.24988, 866.32507,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19874, 2561.59961, 1305.27734, 866.32507,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(11707, 2563.00049, 1305.38318, 866.35278,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2811, 2561.73291, 1298.71802, 865.27209,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2195, 2571.39233, 1283.01770, 865.88770,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(3386, 2538.97852, 1313.38110, 865.29199,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(3389, 2540.23706, 1313.41492, 865.29132,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(3386, 2530.96362, 1313.34644, 865.29199,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(3389, 2532.20166, 1313.37244, 865.29132,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2206, 2589.31543, 1299.52100, 865.15527,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2206, 2589.28662, 1295.77466, 865.15527,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(19893, 2589.39478, 1296.70508, 866.07068,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(19893, 2589.38086, 1300.34509, 866.07068,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(19807, 2589.42773, 1296.00476, 866.14703,   0.00000, 0.00000, 120.84002);
Hospital_CrearObjeto(19807, 2589.35767, 1299.63354, 866.14703,   0.00000, 0.00000, 120.84002);
Hospital_CrearObjeto(19813, 2589.83618, 1294.62805, 865.60577,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2202, 2593.20996, 1300.95642, 865.27289,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(19813, 2593.85986, 1298.56946, 865.55090,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(19813, 2588.95630, 1298.62512, 865.55090,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(2813, 2589.37524, 1301.09717, 866.10614,   0.00000, 0.00000, 80.51999);
Hospital_CrearObjeto(2059, 2589.19897, 1297.47839, 866.11017,   0.00000, 0.00000, -285.59967);
Hospital_CrearObjeto(19903, 2536.38818, 1311.20496, 865.27759,   0.00000, 0.00000, -69.24002);
Hospital_CrearObjeto(19903, 2528.44824, 1311.00623, 865.29156,   0.00000, 0.00000, -69.24002);
Hospital_CrearObjeto(2961, 2579.32788, 1283.95508, 867.19391,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(2961, 2534.50439, 1303.11975, 866.96057,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2557.26758, 1264.99524, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2553.96167, 1264.71802, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2560.39111, 1264.99463, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2564.14404, 1265.02942, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2567.55933, 1264.92957, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2570.32056, 1264.86963, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2573.89941, 1264.99915, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2572.99805, 1274.21436, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2569.99341, 1273.82007, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2566.27515, 1274.05969, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2562.87231, 1274.08850, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2559.84937, 1273.88696, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2556.42090, 1273.86963, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2552.87695, 1273.93408, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2549.57983, 1273.84766, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(19362, 2550.89380, 1264.76978, 865.69891,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(932, 2545.21069, 1269.30713, 865.25726,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(932, 2546.67700, 1268.37427, 865.25732,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(11738, 2547.17480, 1263.69116, 866.18127,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(2208, 2547.08643, 1263.93616, 865.27527,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1722, 2550.23193, 1265.41370, 865.27197,   0.00000, 0.00000, 45.11998);
Hospital_CrearObjeto(1722, 2546.90430, 1265.47375, 865.27197,   0.00000, 0.00000, -34.74002);
Hospital_CrearObjeto(1738, 2578.23218, 1274.55554, 865.82507,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(19903, 2570.73486, 1274.92798, 865.24103,   0.00000, 0.00000, -66.84000);
Hospital_CrearObjeto(19903, 2566.79224, 1263.98657, 865.27216,   0.00000, 0.00000, 120.59997);
Hospital_CrearObjeto(3383, 2537.63843, 1308.93726, 865.27094,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(14782, 2519.10913, 1297.93335, 866.27704,   0.00000, 0.00000, 180.00000);
Hospital_CrearObjeto(1998, 2518.95947, 1301.67529, 865.27490,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1714, 2519.82031, 1301.29297, 865.29791,   0.00000, 0.00000, 178.73990);
Hospital_CrearObjeto(2894, 2518.92749, 1301.74146, 866.09839,   0.00000, 0.00000, 93.53999);
Hospital_CrearObjeto(2241, 2525.91284, 1302.71777, 865.71832,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1516, 2518.39526, 1299.08215, 865.44775,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(11736, 2518.57959, 1298.74951, 865.97900,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(11736, 2518.05884, 1298.97095, 865.97900,   0.00000, 0.00000, -19.86001);
Hospital_CrearObjeto(2611, 2517.80493, 1300.35376, 867.07819,   0.00000, 0.00000, 90.00000);
Hospital_CrearObjeto(3394, 2541.55176, 1310.78662, 865.29309,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(3383, 2530.06323, 1308.32007, 865.27094,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(3394, 2533.66846, 1310.76965, 865.29309,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(11738, 2518.46167, 1299.34546, 866.01434,   0.00000, 0.00000, 70.20002);
Hospital_CrearObjeto(2190, 2569.71875, 1291.56592, 866.14001,   0.00000, 0.00000, -129.66008);
Hospital_CrearObjeto(2894, 2568.36914, 1291.25439, 865.98193,   0.00000, 0.00000, -137.58000);
Hospital_CrearObjeto(2196, 2568.35132, 1291.06641, 865.96313,   0.00000, 0.00000, 168.29991);
Hospital_CrearObjeto(2059, 2569.24438, 1290.73877, 866.22443,   0.00000, 0.00000, 50.76002);
Hospital_CrearObjeto(1535, 2574.02051, 1310.49878, 865.27240,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1535, 2565.23779, 1310.55969, 865.24548,   0.00000, 0.00000, 0.00000);
Hospital_CrearObjeto(1535, 2578.95361, 1303.10583, 865.25244,   0.00000, 0.00000, -90.00000);
Hospital_CrearObjeto(1535, 2578.94019, 1279.36926, 865.25244,   0.00000, 0.00000, -90.00000);

    print("  [HOSPITAL] Interior cargado (215 objetos, prototipo).");
    return 1;
}

// ---------------------------------------------------------------------
//  MARCADORES -- puerta de la calle + puerta de dentro, mismo patron que
//  Banco_CrearMarcadores (15_banco.inc).
// ---------------------------------------------------------------------
stock Hospital_CrearMarcadores()
{
    // Icono "i" de info (1239) en vez del "$" copiado del banco (1274) --
    // pedido expreso de Vlad 13/09/2026, el dollar no pegaba con un hospital.
    g_PickupHospitalFuera = CreateDynamicPickup(1239, 23, HOSPITAL_ENTRADA_X, HOSPITAL_ENTRADA_Y, HOSPITAL_ENTRADA_Z, 0, 0, -1, 100.0);

    // Texto pedido por Vlad 13/09/2026: "Hospital LS" en rojo oscuro,
    // "/entrar" debajo (mismo patron que Banco/Motel: comando en gold).
    g_EtiquetaHospitalFuera = CreateDynamic3DTextLabel(
        "Hospital LS\n{E8B33A}/entrar",
        HOSPITAL_COLOR_ROJO_OSCURO, HOSPITAL_ENTRADA_X, HOSPITAL_ENTRADA_Y, HOSPITAL_ENTRADA_Z + 0.7, 14.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    // Icono/letrero de "/salir" en su propia coordenada (13/09/2026,
    // pedido expreso de Vlad) -- ya no comparte sitio con el punto de
    // llegada del /entrar (HOSPITAL_INTERIOR_X/Y/Z, arriba).
    g_PickupHospitalDentro = CreateDynamicPickup(1239, 23, HOSPITAL_SALIDA_ICONO_X, HOSPITAL_SALIDA_ICONO_Y, HOSPITAL_SALIDA_ICONO_Z,
        MUNDO_HOSPITAL_INTERIOR, 0, -1, 100.0);

    g_EtiquetaHospitalDentro = CreateDynamic3DTextLabel(
        "{FFFFFF}Escribe {E8B33A}/salir{FFFFFF} para volver a la calle",
        COLOR_BLANCO, HOSPITAL_SALIDA_ICONO_X, HOSPITAL_SALIDA_ICONO_Y, HOSPITAL_SALIDA_ICONO_Z + 0.7, 8.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, MUNDO_HOSPITAL_INTERIOR, 0, -1, 25.0);

    // Icono del menu de /tratamiento medico (14/09/2026, pedido expreso de
    // Vlad): pill (1241), letrero "Tratamiento medico" en rojo y
    // "/tratamiento" debajo en gold -- mismo patron que "Hospital LS"/
    // "/entrar" de la puerta de la calle, arriba.
    g_PickupHospitalTratamiento = CreateDynamicPickup(1241, 23,
        HOSPITAL_TRATAMIENTO_ICONO_X, HOSPITAL_TRATAMIENTO_ICONO_Y, HOSPITAL_TRATAMIENTO_ICONO_Z,
        MUNDO_HOSPITAL_INTERIOR, 0, -1, 100.0);

    g_EtiquetaHospitalTratamiento = CreateDynamic3DTextLabel(
        "Tratamiento medico\n{E8B33A}/tratamiento",
        COLOR_ROJO, HOSPITAL_TRATAMIENTO_ICONO_X, HOSPITAL_TRATAMIENTO_ICONO_Y, HOSPITAL_TRATAMIENTO_ICONO_Z + 0.7, 14.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, MUNDO_HOSPITAL_INTERIOR, 0, -1, 30.0);

    print("  [HOSPITAL] Marcadores preparados.");
    return 1;
}

// ---------------------------------------------------------------------
//  ENTRAR Y SALIR -- para colgar de CMD:entrar/CMD:salir (15_banco.inc)
//  cuando se autorice: anadir "if (Hospital_IntentarEntrar(playerid))
//  return 1;" / "...Salir..." a la cadena de esos dos comandos, antes
//  del Error final. Ver el informe para el paso a paso completo.
// ---------------------------------------------------------------------
stock bool:Hospital_IntentarEntrar(playerid)
{
    if (GetPlayerInterior(playerid) != 0) return false;
    if (!EnPunto(playerid, 2.5, HOSPITAL_ENTRADA_X, HOSPITAL_ENTRADA_Y, HOSPITAL_ENTRADA_Z)) return false;

    SetPlayerPos(playerid, HOSPITAL_INTERIOR_X, HOSPITAL_INTERIOR_Y, HOSPITAL_INTERIOR_Z);
    SetPlayerFacingAngle(playerid, HOSPITAL_INTERIOR_A);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, MUNDO_HOSPITAL_INTERIOR);
    SetCameraBehindPlayer(playerid);

    Info(playerid, "Hospital de Los Santos.");
    return true;
}

// /salir funciona desde CUALQUIER punto dentro del interior (pedido
// expreso de Vlad, 13/09/2026): solo comprueba el mundo virtual, sin
// radio de EnPunto -- mismo patron que Casa_IntentarSalir
// (24_casas.inc/Casa_DondeEstoy), no el de punto fijo de Banco.
stock bool:Hospital_IntentarSalir(playerid)
{
    if (GetPlayerVirtualWorld(playerid) != MUNDO_HOSPITAL_INTERIOR) return false;

    SetPlayerPos(playerid, HOSPITAL_ENTRADA_X, HOSPITAL_ENTRADA_Y, HOSPITAL_ENTRADA_Z);
    SetPlayerFacingAngle(playerid, HOSPITAL_ENTRADA_A);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
    SetCameraBehindPlayer(playerid);
    return true;
}

// ---------------------------------------------------------------------
//  /tratamiento -- 3 tratamientos fijos, sube vida al 100% y armadura a
//  un % fijo segun el tratamiento (10/20/30%). Cooldown compartido de 1
//  hora entre los 3. Mismo patron que /comer (42_comida.inc): dialogo
//  TABLIST_HEADERS, animacion retrasada, GameText de exito estilo /puente
//  (22_dropeo.inc).
// ---------------------------------------------------------------------
CMD:tratamiento(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerVirtualWorld(playerid) != MUNDO_HOSPITAL_INTERIOR)
    {
        Error(playerid, "Tienes que estar dentro del hospital para esto.");
        return 1;
    }

    Hospital_TratamientoMostrarMenu(playerid);
    return 1;
}

stock Hospital_TratamientoMostrarMenu(playerid)
{
    new lista[256], linea[64], precio[24];
    format(lista, sizeof lista, "Tratamiento\tPrecio\n");

    for (new t = 0; t < HOSPITAL_TOTAL_TRATAMIENTOS; t++)
    {
        FormatearDinero(g_TratamientoPrecio[t], precio, sizeof precio);
        format(linea, sizeof linea, "%s%s\t%s\n", g_TratamientoColorHex[t], g_TratamientoNombre[t], precio);
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_HOSPITAL_TRATAMIENTO, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8B33A}Tratamiento medico", lista, "Aplicar", "Cerrar");
    return 1;
}

stock Hospital_TratamientoDialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext

    if (dialogid != DIALOGO_HOSPITAL_TRATAMIENTO) return 0;
    if (!response) return 1;

    if (GetPlayerVirtualWorld(playerid) != MUNDO_HOSPITAL_INTERIOR)
    {
        Error(playerid, "Ya no estas dentro del hospital.");
        return 1;
    }

    if (listitem < 0 || listitem >= HOSPITAL_TOTAL_TRATAMIENTOS) return 1;

    new ahora = gettime();
    if (ahora < g_TratamientoCooldownHasta[playerid])
    {
        new tiempo[32];
        FormatearTiempo(g_TratamientoCooldownHasta[playerid] - ahora, tiempo, sizeof tiempo);
        format(g_Buffer, sizeof g_Buffer, "Tienes que esperar %s para otro tratamiento.", tiempo);
        Error(playerid, g_Buffer);
        return 1;
    }

    new precio = g_TratamientoPrecio[listitem];
    if (!TieneDinero(playerid, precio))
    {
        Error(playerid, "No te llega el dinero para eso.");
        Hospital_TratamientoMostrarMenu(playerid);
        return 1;
    }

    DarDinero(playerid, -precio);
    SetPlayerHealth(playerid, 100.0);
    SetPlayerArmour(playerid, g_TratamientoEscudo[listitem]);
    g_TratamientoCooldownHasta[playerid] = ahora + HOSPITAL_TRATAMIENTO_COOLDOWN_SEGUNDOS;

    // La animacion se retrasa un poco, mismo motivo y mismo patron que
    // Comida_AnimarRetardado (42_comida.inc): aplicarla en el mismo tick
    // en que se cierra el dialogo no se ve.
    SetTimerEx("Hospital_TratamientoAnimarRetardado", HOSPITAL_TRATAMIENTO_ANIMACION_RETRASO_MS, false, "d", playerid);

    // Aviso en pantalla: "TRATAMIENTO APLICADO" / "Te encuentras mejor",
    // 2 segundos -- pedido expreso de Vlad 14/09/2026 (el textdraw de
    // /puente mientras puentea, NO el GameText grande de exito/fallo).
    PlayerTextDrawShow(playerid, g_HospitalTratamientoTD[playerid][HTRAT_TITULO]);
    PlayerTextDrawShow(playerid, g_HospitalTratamientoTD[playerid][HTRAT_SUBTITULO]);
    SetTimerEx("Hospital_TratamientoOcultarAviso", HOSPITAL_TRATAMIENTO_AVISO_DURACION_MS, false, "d", playerid);

    // Sin mensaje de chat -- pedido expreso de Vlad 14/09/2026, el aviso
    // en pantalla de arriba ya lo deja claro, era redundante (mismo
    // criterio ya aplicado al "Motor arrancado."/"FALLO AL PUENTEAR" de
    // /puente, 22_dropeo.inc).

    // El menu se cierra solo tras usar un tratamiento -- pedido expreso
    // de Vlad 14/09/2026 (a diferencia de /comer, aqui NO se reabre).
    return 1;
}

forward Hospital_TratamientoOcultarAviso(playerid);
public Hospital_TratamientoOcultarAviso(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    PlayerTextDrawHide(playerid, g_HospitalTratamientoTD[playerid][HTRAT_TITULO]);
    PlayerTextDrawHide(playerid, g_HospitalTratamientoTD[playerid][HTRAT_SUBTITULO]);
    return 1;
}

forward Hospital_TratamientoAnimarRetardado(playerid);
public Hospital_TratamientoAnimarRetardado(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    if (GetPlayerVirtualWorld(playerid) != MUNDO_HOSPITAL_INTERIOR) return 0;

    Animar(playerid, "FOOD", "EAT_Burger", 4.1, false, false, false, false, 0);
    return 1;
}
