/* =====================================================================
 *  38_drift.inc
 *  ---------------------------------------------------------------------
 *  Dinero por derrapar. Portado del filterscript "Dinero al derrapar"
 *  del catalogo de scripts antiguos, reescrito sobre el vector de
 *  velocidad real del vehiculo (GetVehicleVelocity) en vez del calculo
 *  trigonometrico del original, y pagando con DarDinero (03_utiles.inc)
 *  para que el HUD de dinero no se desincronice.
 *
 *  COMO SE MIDE: cada DRIFT_INTERVALO_MS se compara el angulo al que
 *  APUNTA el coche (GetVehicleZAngle) con el angulo al que SE MUEVE de
 *  verdad (el vector de velocidad). Cuanto mas se separan esos dos
 *  angulos sin dejar de moverse rapido, mas se esta derrapando.
 *
 *  Solo se paga si, al cortar el derrape, la salud del vehiculo sigue
 *  parecida a como estaba al empezar -- si ha bajado de golpe es que ha
 *  habido un golpe por el camino, no un derrape limpio, y no se paga
 *  nada.
 *
 *  TEXTO EN PANTALLA -- SIN chat, todo en el widget del velocimetro
 *  (13_vehiculos.inc, Veh_HUD_Iniciar), abajo a la derecha del todo.
 *  Mientras derrapas ensena "DRIFT $X" subiendo en dorado; al cortar con
 *  premio, el mismo texto se pone en verde con "+X$" y PARPADEA un par
 *  de veces antes de esconderse -- esa es la unica senal de que has
 *  cobrado. Si el derrape no llega al minimo o te llevas un golpe, el
 *  texto simplemente desaparece, sin aviso de ningun tipo. 1 punto de
 *  derrape = 1$, para que el numero en pantalla sea exactamente el que
 *  cobras (DRIFT_DINERO_POR_PUNTO).
 * ===================================================================== */

static bool:g_DriftActivo[MAX_PLAYERS];
static Float:g_DriftPuntos[MAX_PLAYERS];
static Float:g_DriftSaludInicial[MAX_PLAYERS];
static PlayerText:g_DriftTexto[MAX_PLAYERS];
static bool:g_DriftTextoCreado[MAX_PLAYERS];
static g_DriftGen[MAX_PLAYERS];            // se incrementa cada vez que cambia lo que hay que ensenar, para que un parpadeo viejo no se coma un derrape nuevo
static g_DriftParpadeosRestantes[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  Normaliza un angulo a -180..180. GetVehicleZAngle y atan2 pueden
//  devolver valores fuera de ese rango segun el cuadrante.
// ---------------------------------------------------------------------
stock Float:Drift_NormalizarAngulo(Float:angulo)
{
    while (angulo > 180.0)  angulo -= 360.0;
    while (angulo < -180.0) angulo += 360.0;
    return angulo;
}

// ---------------------------------------------------------------------
//  TEXTO EN PANTALLA -- un solo textdraw por jugador, creado la primera
//  vez que hace falta y reutilizado el resto de la sesion. Metido en la
//  esquina inferior derecha del velocimetro (caja de Veh_HUD_Iniciar,
//  que ocupa aprox. x:500-630, y:410-490), alineado a la derecha para
//  que quede pegado al borde de la caja.
// ---------------------------------------------------------------------
stock Drift_AsegurarTexto(playerid)
{
    if (g_DriftTextoCreado[playerid]) return 1;

    g_DriftTexto[playerid] = CreatePlayerTextDraw(playerid, 622.0, 431.0, "_");
    PlayerTextDrawFont(playerid, g_DriftTexto[playerid], TEXT_DRAW_FONT_BANK_GOTHIC);
    PlayerTextDrawLetterSize(playerid, g_DriftTexto[playerid], 0.17, 0.75);
    PlayerTextDrawColour(playerid, g_DriftTexto[playerid], 0xE8B33AFF);
    PlayerTextDrawSetShadow(playerid, g_DriftTexto[playerid], 0);
    PlayerTextDrawSetOutline(playerid, g_DriftTexto[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_DriftTexto[playerid], true);
    PlayerTextDrawAlignment(playerid, g_DriftTexto[playerid], TEXT_DRAW_ALIGN_RIGHT);

    g_DriftTextoCreado[playerid] = true;
    return 1;
}

// Numero que sube mientras derrapas, en dorado (mismo color que el resto del HUD).
stock Drift_ActualizarTexto(playerid)
{
    Drift_AsegurarTexto(playerid);

    PlayerTextDrawColour(playerid, g_DriftTexto[playerid], 0xE8B33AFF);
    format(g_Buffer, sizeof g_Buffer, "DRIFT $%d", floatround(g_DriftPuntos[playerid]));
    PlayerTextDrawSetString(playerid, g_DriftTexto[playerid], g_Buffer);
    PlayerTextDrawShow(playerid, g_DriftTexto[playerid]);
    return 1;
}

// Premio cobrado: el texto se pone verde con "+X$" y parpadea
// DRIFT_FLASH_PARPADEOS veces (encendido/apagado) antes de esconderse
// solo. Es la UNICA senal de que se ha cobrado -- no hay chat.
stock Drift_MostrarFlash(playerid, monto)
{
    Drift_AsegurarTexto(playerid);

    g_DriftGen[playerid]++;
    new gen = g_DriftGen[playerid];

    format(g_Buffer, sizeof g_Buffer, "+%d$", monto);
    PlayerTextDrawSetString(playerid, g_DriftTexto[playerid], g_Buffer);
    PlayerTextDrawColour(playerid, g_DriftTexto[playerid], COLOR_VERDE);
    PlayerTextDrawShow(playerid, g_DriftTexto[playerid]);

    // Visible ahora mismo -> el primer parpadeo toca apagarlo. Cada
    // parpadeo son dos pasos (apagado + encendido), y el ultimo paso se
    // deja apagado para que no se quede el texto pegado en pantalla.
    g_DriftParpadeosRestantes[playerid] = (DRIFT_FLASH_PARPADEOS * 2) - 1;
    SetTimerEx("Drift_Parpadeo", DRIFT_FLASH_INTERVALO_MS, false, "dd", playerid, gen);
    return 1;
}

forward Drift_Parpadeo(playerid, gen);
public Drift_Parpadeo(playerid, gen)
{
    // Si mientras tanto ha empezado otro derrape (o otro cobro), esa
    // generacion ya no coincide y no tocamos nada.
    if (g_DriftGen[playerid] != gen) return 0;
    if (g_DriftActivo[playerid]) return 0;

    // Pasos que quedan pares = tocaba encender, impares = apagar
    // (arranca en apagar porque el texto ya estaba visible al llamar
    // a Drift_MostrarFlash).
    if (g_DriftParpadeosRestantes[playerid] % 2 == 1) PlayerTextDrawHide(playerid, g_DriftTexto[playerid]);
    else PlayerTextDrawShow(playerid, g_DriftTexto[playerid]);

    g_DriftParpadeosRestantes[playerid]--;
    if (g_DriftParpadeosRestantes[playerid] <= 0)
    {
        Drift_OcultarTexto(playerid);
        return 1;
    }

    SetTimerEx("Drift_Parpadeo", DRIFT_FLASH_INTERVALO_MS, false, "dd", playerid, gen);
    return 1;
}

stock Drift_OcultarTexto(playerid)
{
    if (!g_DriftTextoCreado[playerid]) return 0;
    PlayerTextDrawHide(playerid, g_DriftTexto[playerid]);
    return 1;
}

// ---------------------------------------------------------------------
//  CORTAR -- fin del derrape actual. Paga si llega al minimo de puntos
//  y el coche no se ha llevado un golpe por el camino.
// ---------------------------------------------------------------------
stock Drift_Cortar(playerid)
{
    if (!g_DriftActivo[playerid]) return 0;

    g_DriftActivo[playerid] = false;
    g_DriftGen[playerid]++;

    new puntos = floatround(g_DriftPuntos[playerid]);
    g_DriftPuntos[playerid] = 0.0;

    if (puntos < DRIFT_PUNTOS_MINIMOS)
    {
        Drift_OcultarTexto(playerid);
        return 0;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    if (vehicleid == INVALID_VEHICLE_ID)
    {
        Drift_OcultarTexto(playerid);
        return 0;
    }

    new Float:saludAhora;
    GetVehicleHealth(vehicleid, saludAhora);
    if (saludAhora < g_DriftSaludInicial[playerid] - DRIFT_SALUD_TOLERANCIA)
    {
        Drift_OcultarTexto(playerid);
        return 0;
    }

    new premio = puntos * DRIFT_DINERO_POR_PUNTO;
    DarDinero(playerid, premio);
    Drift_MostrarFlash(playerid, premio);
    return 1;
}

// Version para los ganchos de 99_callbacks.inc (bajarse del coche,
// morir, desconectar): corta sin avisos de golpe, solo esconde.
stock Drift_Reiniciar(playerid)
{
    if (g_DriftActivo[playerid]) Drift_Cortar(playerid);
    g_DriftActivo[playerid] = false;
    g_DriftPuntos[playerid] = 0.0;
    g_DriftGen[playerid]++;
    Drift_OcultarTexto(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  TICK -- SetTimer("Drift_Tick", DRIFT_INTERVALO_MS, true), registrado
//  en OnGameModeInit (99_callbacks.inc).
// ---------------------------------------------------------------------
forward Drift_Tick();
public Drift_Tick()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;

        if (GetPlayerState(i) != PLAYER_STATE_DRIVER)
        {
            if (g_DriftActivo[i]) Drift_Cortar(i);
            continue;
        }

        new vehicleid = GetPlayerVehicleID(i);
        new Float:vx, Float:vy, Float:vz;
        GetVehicleVelocity(vehicleid, vx, vy, vz);

        new Float:velocidad = floatsqroot((vx * vx) + (vy * vy) + (vz * vz));
        if (velocidad < DRIFT_VELOCIDAD_MINIMA)
        {
            if (g_DriftActivo[i]) Drift_Cortar(i);
            continue;
        }

        new Float:anguloCoche;
        GetVehicleZAngle(vehicleid, anguloCoche);

        // OJO: atan2(y, x) es el matematico de toda la vida (0 grados en
        // +X, sentido antihorario), pero GetVehicleZAngle usa el rumbo de
        // brujula de GTA (0 grados al norte/+Y, sentido horario). Para
        // comparar los dos angulos en el mismo sistema hay que invertir
        // el orden de los argumentos: atan2(vx, vy), no atan2(vy, vx).
        new Float:anguloMovimiento = atan2(vx, vy);
        new Float:diferencia = floatabs(Drift_NormalizarAngulo(anguloCoche - anguloMovimiento));

        if (diferencia < DRIFT_ANGULO_MINIMO || diferencia > DRIFT_ANGULO_MAXIMO)
        {
            if (g_DriftActivo[i]) Drift_Cortar(i);
            continue;
        }

        // Derrapando de verdad.
        if (!g_DriftActivo[i])
        {
            g_DriftActivo[i] = true;
            g_DriftPuntos[i] = 0.0;
            GetVehicleHealth(vehicleid, g_DriftSaludInicial[i]);
        }

        g_DriftPuntos[i] += (diferencia * 0.05) + (velocidad * 4.0);
        if (g_DriftPuntos[i] > DRIFT_PUNTOS_MAXIMOS) g_DriftPuntos[i] = Float:DRIFT_PUNTOS_MAXIMOS;

        Drift_ActualizarTexto(i);
    }
    return 1;
}
