/* =====================================================================
 *  44_hambre_sed.inc
 *  ---------------------------------------------------------------------
 *  HAMBRE y SED, 100 -> 0 en tiempo de juego EFECTIVO (conectado,
 *  logueado, ya en la calle -jSpawneado- y sin estar AFK). Constantes de
 *  tiempo/avisos en HAMBRE_SEGUNDOS_VACIO / SED_SEGUNDOS_VACIO y compania
 *  (01_configuracion.inc). Persisten en `cuentas.hambre` / `cuentas.sed`
 *  -- carga y guardado en 05_basedatos.inc, igual que salud o armadura.
 *
 *  NO hay inventario ni productos nuevos aqui: solo se conecta con lo
 *  que ya existe --
 *    - Inv_Usar(), caso ITEM_CONSUMIBLE (10_inventario.inc): Bocadillo,
 *      Hamburguesa, Botella de agua y Cafe (los unicos ITEM_CONSUMIBLE
 *      que son comida o bebida de verdad; el Paquete de tabaco tambien es
 *      consumible pero no es ni una cosa ni la otra, se deja intacto).
 *    - Comida_Dialogo() (42_comida.inc): los 9 productos de las 3
 *      franquicias de comida rapida.
 *
 *  HUD: DOS PlayerTextDraw pequenos por jugador, sin barras -- pedido
 *  expreso de Vlad tras ver la primera version (las barras quedaban
 *  enormes y fuera de sitio en su resolucion). Reubicados 13/09/2026,
 *  otra peticion expresa: ya NO van pegados a la fila de "casas/negocios
 *  libres" de 30_gps.inc (esa fila se oculto del HUD permanente ese
 *  mismo dia) -- ahora viven debajo del dinero, arriba a la derecha,
 *  alineados a la derecha para que la anchura variable del texto
 *  ("HAMBRE 100%" vs "HAMBRE 5%") no desplace nada, mismo truco que ya
 *  resolvio ese problema para los contadores de GPS.
 *
 *  OJO: las etiquetas de color "{RRGGBB}" (las que sí funcionan dentro
 *  de un mensaje de chat, como en HambreSed_Sumar() mas abajo) NO las
 *  interpreta un TextDraw -- ahi se imprimen literales ("Se vio en la
 *  pantalla de Vlad: "?F2A93C HAMBRE 99%"). Por eso el color de cada
 *  palabra se pone con PlayerTextDrawColour() sobre DOS textdraws en vez
 *  de intentar mezclar colores dentro de uno solo.
 *
 *  PlayerTextDrawSetString actualiza en caliente sin ocultar y volver a
 *  mostrar, igual que ese contador. Se crean en OnPlayerConnect junto al
 *  resto del HUD y se muestran/ocultan desde
 *  GPS_MostrarTextdrawFijo/OcultarTextdrawFijo (30_gps.inc) para heredar
 *  gratis el mismo ciclo de vida que esa fila -- incluido ocultarse
 *  mientras esta abierta la pantalla de Federico.
 * ===================================================================== */

// HAMBRESED_COLOR_HAMBRE/_SED se conservan tal cual (rojo/verde): las
// sigue usando el aviso grande "Te estas muriendo de hambre!"
// (g_MuerteHambreTD, mas abajo) y ese SI debe seguir en rojo de alarma --
// no se ha tocado, no formaba parte de este encargo.
#define HAMBRESED_COLOR_HAMBRE       0xFF0000FF
#define HAMBRESED_COLOR_SED          0x32FF32FF

// Color del HUD pequeno de HAMBRE/SED, 13/09/2026 a peticion expresa de
// Vlad: mismo color que "PUENTEANDO..." (22_dropeo.inc), 0x4FD1C5FF --
// separado de HAMBRESED_COLOR_HAMBRE/_SED de arriba a proposito, para no
// tocar el color del aviso de "muriendo de hambre" de paso.
#define HAMBRESED_COLOR_HUD          0x4FD1C5FF

// Reubicado 13/09/2026 a peticion de Vlad: debajo del dinero, arriba a
// la derecha, en vez de pegado a la fila de "casas/negocios libres" de
// 30_gps.inc (esa fila ya no es un HUD permanente, ver ese archivo).
// ALIGN_RIGHT en las dos textdraws de mas abajo, asi HAMBRESED_X marca
// el borde derecho y ninguna longitud de texto desplaza nada.
//
// Y bajado el mismo dia, misma peticion, tras verlo en directo: el primer
// intento (Y=35/48) quedaba pisando el dinero Y el icono de arma/municion
// del HUD nativo (esquina superior derecha: dinero, estrellas de busqueda,
// arma, todo apilado ahi).
//
// Segundo ajuste, mismo dia, con captura de pantalla real de Vlad
// (Screenshot 480): Y=145/158 se paso de largo -- quedaba muy por debajo
// del dinero, con un hueco enorme. Pedido "debajo del dinero a un
// centimetro": bajado solo lo justo para librar el bloque nativo
// (puno/arma + barra + dinero, que en esa captura termina sobre el 100
// en esta escala), no toda la pantalla. X tambien acercado del borde:
// pegaba demasiado al lado, ahora un poco mas adentro para leerse "junto"
// al dinero en vez de aislado en la esquina.
#define HAMBRESED_X                  600.0
#define HAMBRESED_Y_HAMBRE           108.0
#define HAMBRESED_Y_SED              121.0

// ---------------------------------------------------------------------
//  TEXTDRAWS POR JUGADOR
// ---------------------------------------------------------------------
static PlayerText:g_HambreTD[MAX_PLAYERS];
static PlayerText:g_SedTD[MAX_PLAYERS];

// Aviso grande de "te estas muriendo" -- mismo font (Pricedown) y estilo
// que el titulo de /federico y el icono de bienvenida (43_federico.inc /
// 47_bienvenida.inc), pedido expreso de Vlad el 06/09/2026 para que se
// note de verdad cuando hambre o sed llegan a 0.
enum { MTD_TITULO, MTD_SUBTITULO, MTD_TOTAL }
static PlayerText:g_MuerteHambreTD[MAX_PLAYERS][MTD_TOTAL];
static bool:g_MuerteHambreVisible[MAX_PLAYERS];

// Llamado desde OnPlayerConnect (99_callbacks.inc), junto al resto del
// HUD por jugador (Veh_HUD_Crear, GPS_CrearTextdrawJugador).
stock HambreSed_CrearTextdrawJugador(playerid)
{
    g_HambreTD[playerid] = CreatePlayerTextDraw(playerid, HAMBRESED_X, HAMBRESED_Y_HAMBRE, "HAMBRE 100%");
    PlayerTextDrawAlignment(playerid, g_HambreTD[playerid], TEXT_DRAW_ALIGN_RIGHT);
    PlayerTextDrawFont(playerid, g_HambreTD[playerid], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_HambreTD[playerid], 0.20, 0.95);
    PlayerTextDrawColour(playerid, g_HambreTD[playerid], HAMBRESED_COLOR_HUD);
    PlayerTextDrawSetShadow(playerid, g_HambreTD[playerid], 0);
    PlayerTextDrawSetOutline(playerid, g_HambreTD[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_HambreTD[playerid], true);

    g_SedTD[playerid] = CreatePlayerTextDraw(playerid, HAMBRESED_X, HAMBRESED_Y_SED, "SED 100%");
    PlayerTextDrawAlignment(playerid, g_SedTD[playerid], TEXT_DRAW_ALIGN_RIGHT);
    PlayerTextDrawFont(playerid, g_SedTD[playerid], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_SedTD[playerid], 0.20, 0.95);
    PlayerTextDrawColour(playerid, g_SedTD[playerid], HAMBRESED_COLOR_HUD);
    PlayerTextDrawSetShadow(playerid, g_SedTD[playerid], 0);
    PlayerTextDrawSetOutline(playerid, g_SedTD[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_SedTD[playerid], true);

    g_MuerteHambreTD[playerid][MTD_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 380.0, "Te estas muriendo de hambre!");
    PlayerTextDrawAlignment(playerid, g_MuerteHambreTD[playerid][MTD_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_MuerteHambreTD[playerid][MTD_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_MuerteHambreTD[playerid][MTD_TITULO], 0.42, 1.8);
    PlayerTextDrawColour(playerid, g_MuerteHambreTD[playerid][MTD_TITULO], HAMBRESED_COLOR_HAMBRE);
    PlayerTextDrawSetOutline(playerid, g_MuerteHambreTD[playerid][MTD_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_MuerteHambreTD[playerid][MTD_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_MuerteHambreTD[playerid][MTD_TITULO], true);

    g_MuerteHambreTD[playerid][MTD_SUBTITULO] = CreatePlayerTextDraw(playerid, 320.0, 402.0, "Come algo con urgencia!");
    PlayerTextDrawAlignment(playerid, g_MuerteHambreTD[playerid][MTD_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_MuerteHambreTD[playerid][MTD_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_MuerteHambreTD[playerid][MTD_SUBTITULO], 0.20, 1.1);
    PlayerTextDrawColour(playerid, g_MuerteHambreTD[playerid][MTD_SUBTITULO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_MuerteHambreTD[playerid][MTD_SUBTITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_MuerteHambreTD[playerid][MTD_SUBTITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_MuerteHambreTD[playerid][MTD_SUBTITULO], true);

    g_MuerteHambreVisible[playerid] = false;
    return 1;
}

// Se llama cada segundo, independiente del cooldown de la penalizacion
// de vida (esa solo controla cada cuanto se resta HP, no la visibilidad
// del aviso) -- asi el texto se queda fijo en pantalla todo el rato que
// sigas a 0, y desaparece en el segundo exacto en que comes o bebes.
stock HambreSed_ActualizarAvisoMuerte(playerid)
{
    new bool:critico = (Jugador[playerid][jHambre] <= 0.0 || Jugador[playerid][jSed] <= 0.0);

    if (critico && !g_MuerteHambreVisible[playerid])
    {
        PlayerTextDrawShow(playerid, g_MuerteHambreTD[playerid][MTD_TITULO]);
        PlayerTextDrawShow(playerid, g_MuerteHambreTD[playerid][MTD_SUBTITULO]);
        g_MuerteHambreVisible[playerid] = true;
    }
    else if (!critico && g_MuerteHambreVisible[playerid])
    {
        PlayerTextDrawHide(playerid, g_MuerteHambreTD[playerid][MTD_TITULO]);
        PlayerTextDrawHide(playerid, g_MuerteHambreTD[playerid][MTD_SUBTITULO]);
        g_MuerteHambreVisible[playerid] = false;
    }
    return 1;
}

// Llamadas desde GPS_MostrarTextdrawFijo/GPS_OcultarTextdrawFijo
// (30_gps.inc) para compartir el mismo ciclo de vida que la fila de
// "casas libres" -- incluido ocultarse junto con ella mientras esta
// abierta la pantalla de Federico.
stock HambreSed_Mostrar(playerid)
{
    PlayerTextDrawShow(playerid, g_HambreTD[playerid]);
    PlayerTextDrawShow(playerid, g_SedTD[playerid]);
    return 1;
}

stock HambreSed_Ocultar(playerid)
{
    PlayerTextDrawHide(playerid, g_HambreTD[playerid]);
    PlayerTextDrawHide(playerid, g_SedTD[playerid]);
    return 1;
}

// Se llama una vez por segundo desde TicSegundo (99_callbacks.inc),
// igual que Veh_HUD_Segundo -- misma clase de coste que el resto del
// HUD por segundo, dos SetString sin tags de color (ver nota de arriba).
stock HambreSed_ActualizarHUD(playerid)
{
    format(g_Buffer, sizeof g_Buffer, "HAMBRE %d%%", floatround(Jugador[playerid][jHambre]));
    PlayerTextDrawSetString(playerid, g_HambreTD[playerid], g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "SED %d%%", floatround(Jugador[playerid][jSed]));
    PlayerTextDrawSetString(playerid, g_SedTD[playerid], g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  CONSUMO -- llamado desde Inv_Usar (10_inventario.inc) y
//  Comida_Dialogo (42_comida.inc) cuando se consume de verdad, nunca al
//  comprar. Sube jHambre/jSed sin pasar de 100 y devuelve un trozo de
//  texto tipo " Hambre +30" / " Hambre +30 | Sed +20" / " Sed +25" listo
//  para pegar detras del mensaje de exito que ya muestra cada sistema.
// ---------------------------------------------------------------------
stock HambreSed_Sumar(playerid, Float:hambre, Float:sed, destino[], tam)
{
    destino[0] = EOS;
    if (hambre <= 0.0 && sed <= 0.0) return 0;

    if (hambre > 0.0)
    {
        Jugador[playerid][jHambre] += hambre;
        if (Jugador[playerid][jHambre] > 100.0) Jugador[playerid][jHambre] = 100.0;
    }
    if (sed > 0.0)
    {
        Jugador[playerid][jSed] += sed;
        if (Jugador[playerid][jSed] > 100.0) Jugador[playerid][jSed] = 100.0;
    }

    if (hambre > 0.0 && sed > 0.0)
    {
        format(destino, tam, " {F2A93C}Hambre +%d{FFFFFF} | {3CC8F2}Sed +%d", floatround(hambre), floatround(sed));
    }
    else if (hambre > 0.0)
    {
        format(destino, tam, " {F2A93C}Hambre +%d", floatround(hambre));
    }
    else
    {
        format(destino, tam, " {3CC8F2}Sed +%d", floatround(sed));
    }
    return 1;
}

// ---------------------------------------------------------------------
//  TIC -- llamado una vez por segundo por jugador logueado desde
//  TicSegundo (99_callbacks.inc), junto a Veh_HUD_Segundo y compania.
//  Solo descuenta con el jugador YA en la calle (jSpawneado) y sin estar
//  AFK -- exactamente el "tiempo de juego efectivo" pedido. El tiempo
//  desconectado nunca pasa por aqui (no hay timer que corra sin sesion),
//  asi que offline no consume nada; al reconectar, BD_CargarCuenta ya
//  trajo el valor exacto con el que se guardo.
// ---------------------------------------------------------------------
stock HambreSed_Segundo(playerid)
{
    if (!Jugador[playerid][jSpawneado] || Jugador[playerid][jAFK]) return 0;

    Jugador[playerid][jHambre] -= (100.0 / HAMBRE_SEGUNDOS_VACIO);
    if (Jugador[playerid][jHambre] < 0.0) Jugador[playerid][jHambre] = 0.0;

    Jugador[playerid][jSed] -= (100.0 / SED_SEGUNDOS_VACIO);
    if (Jugador[playerid][jSed] < 0.0) Jugador[playerid][jSed] = 0.0;

    HambreSed_AvisoHambre(playerid, Jugador[playerid][jHambre]);
    HambreSed_AvisoSed(playerid, Jugador[playerid][jSed]);
    HambreSed_ActualizarAvisoMuerte(playerid);
    HambreSed_Penalizacion(playerid);
    return 1;
}

stock HambreSed_AvisoHambre(playerid, Float:valor)
{
    if (valor > HAMBRESED_UMBRAL_AVISO) return 0;

    new ahora = GetTickCount();
    new cooldown = (valor <= HAMBRESED_UMBRAL_CRITICO) ? HAMBRESED_AVISO_COOLDOWN_BAJO : HAMBRESED_AVISO_COOLDOWN_ALTO;

    if (ahora < Jugador[playerid][jHambreAvisoHasta]) return 0;
    Jugador[playerid][jHambreAvisoHasta] = ahora + (cooldown * 1000);

    HambreSed_MensajeUmbral(playerid, valor, "hambre");
    return 1;
}

stock HambreSed_AvisoSed(playerid, Float:valor)
{
    if (valor > HAMBRESED_UMBRAL_AVISO) return 0;

    new ahora = GetTickCount();
    new cooldown = (valor <= HAMBRESED_UMBRAL_CRITICO) ? HAMBRESED_AVISO_COOLDOWN_BAJO : HAMBRESED_AVISO_COOLDOWN_ALTO;

    if (ahora < Jugador[playerid][jSedAvisoHasta]) return 0;
    Jugador[playerid][jSedAvisoHasta] = ahora + (cooldown * 1000);

    HambreSed_MensajeUmbral(playerid, valor, "sed");
    return 1;
}

stock HambreSed_MensajeUmbral(playerid, Float:valor, const nombre[])
{
    if (valor <= 0.0)
    {
        format(g_Buffer, sizeof g_Buffer, "Te mueres de %s.", nombre);
    }
    else if (valor <= HAMBRESED_UMBRAL_CRITICO)
    {
        format(g_Buffer, sizeof g_Buffer, "Tienes mucha %s.", nombre);
    }
    else
    {
        format(g_Buffer, sizeof g_Buffer, "Empiezas a tener %s.", nombre);
    }
    Aviso(playerid, g_Buffer);
    return 1;
}

// A 0% de hambre O sed: drenado de vida SIN suelo, encargo de Vlad el
// 06/09/2026 -- si no comes o bebes a tiempo, te mueres de verdad. Ver
// HAMBRESED_* en 01_configuracion.inc para el ritmo (100 en 90s).
//
// jHerido de por medio: una vez caido por esto, deja de restar -- si no,
// seguiria descontando vida (ya en 0, o negativa via el motor) sobre un
// jugador que el sistema de medicos ya esta tratando como muerto.
stock HambreSed_Penalizacion(playerid)
{
    if (Jugador[playerid][jHambre] > 0.0 && Jugador[playerid][jSed] > 0.0) return 0;
    if (Jugador[playerid][jHerido]) return 0;

    new ahora = gettime();
    if (ahora < Jugador[playerid][jHambreSedPenalizacionHasta]) return 0;
    Jugador[playerid][jHambreSedPenalizacionHasta] = ahora + HAMBRESED_PENALIZACION_SEGUNDOS;

    new Float:vida;
    GetPlayerHealth(playerid, vida);

    vida -= HAMBRESED_PENALIZACION_VIDA;
    if (vida < 0.0) vida = 0.0;
    SetPlayerHealth(playerid, vida);
    return 1;
}
