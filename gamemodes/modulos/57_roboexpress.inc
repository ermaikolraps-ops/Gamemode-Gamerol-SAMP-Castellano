/* =====================================================================
 *  57_roboexpress.inc
 *  ---------------------------------------------------------------------
 *  ROBO EXPRESS -- mision excepcional de Federico. Encargo de Vlad,
 *  08/09/2026. Primera de un futuro sistema de 4 misiones dinamicas (una
 *  cada 30 min, cambiado desde 20 el mismo dia por peticion expresa);
 *  esta V1 es independiente y autocontenida, pero expone
 *  RoboExpress_Iniciar() como API publica para que un controlador
 *  superior la llame directamente el dia que exista.
 *
 *  CONCEPTO: el servidor elige UN VEHICULO FISICO CONCRETO (no un
 *  modelo) de entre los coches libres cuyo modelo este clasificado AHORA
 *  MISMO como SUPER RARE por el propio Federico (Federico_ContarModelo
 *  == 1, exactamente el mismo criterio que ya usa Federico_Rareza). Como
 *  SUPER RARE ya significa "solo queda 1 copia libre", el modelo YA
 *  identifica un vehiculo unico -- se guarda su `veID` persistente
 *  (nunca el indice de g_Vehiculos, que se recicla).
 *
 *  Exclusion mutua con el resto de Federico: se le pregunta directamente
 *  a Federico con sus propias funciones (Federico_SlotDeModelo,
 *  Federico_EsVehiculoSecreto, Federico_EsVehiculoMoto,
 *  Federico_ChatarraSlotDeModelo) mas una comprobacion aparte contra el
 *  Personaje Historico vigente (por veID exacto, es el unico "coche
 *  libre" que en realidad es especial). Cero listas duplicadas.
 *
 *  ESTADO: en memoria, sin persistencia (evento temporal, V1). Un solo
 *  jugador asignado a la vez, sin cola de reserva.
 *
 *  INTEGRACION (minima, en los 3 archivos que ya se avisaron):
 *    - malagacity.pwn: el #include.
 *    - 99_callbacks.inc: OnPlayerConnect (crear textdraws),
 *      OnPlayerStateChange (hook de "te montas en el vehiculo"),
 *      OnPlayerDisconnect (abandono), TicMinuto (caducidad + validez).
 *    - 43_federico.inc: esRoboExpress como rama explicita en
 *      /entregarcoche, ANTES de tocar ningun slot -- ver el comentario
 *      en ese archivo junto al cambio.
 *    - 01_configuracion.inc: UNA linea, ROBOEXPRESS_RECOMPENSA (tiene
 *      que existir ANTES de 43_federico.inc en el orden de #include,
 *      por eso no puede vivir aqui igual que el resto de constantes de
 *      Federico).
 * ===================================================================== */

#define ROBOEXPRESS_DURACION        900   // 15 minutos reales, desde que aparece
#define ROBOEXPRESS_CICLO_MINUTOS   30    // Coherencia audiovisual (08/09/2026): 20 -> 30 min, peticion expresa de Vlad
#define ROBOEXPRESS_AVISO_MS        4000  // mismo tiempo que ya usa /arreglarcoche

static bool:g_RoboExpressActiva          = false;
static g_RoboExpressVehiculoID           = 0;      // veID persistente, NUNCA un indice de g_Vehiculos
static g_RoboExpressModelo               = 0;
static g_RoboExpressJugador              = INVALID_PLAYER_ID;
static g_RoboExpressExpiraEn             = 0;      // gettime() absoluto
static bool:g_RoboExpressEncontrado      = false;
static g_RoboExpressUltimoAvisoIncorrecto = 0;      // antispam del "no es este" -- un solo jugador asignado a la vez, basta un escalar
static bool:g_RoboExpressCicloListo      = false;   // lazy-init del reloj de 30 min, ver RoboExpress_Minuto
static g_RoboExpressProximoCiclo         = 0;

enum { RTD_TITULO, RTD_SUBTITULO, RTD_TOTAL }
static PlayerText:g_RoboExpressTD[MAX_PLAYERS][RTD_TOTAL];

// Aviso GRANDE a TODOS los conectados en el instante en que aparece una
// mision (coherencia audiovisual, 08/09/2026) -- posicion inferior
// reutilizada de 44_hambre_sed.inc (unico sistema del servidor que ya usa
// un aviso grande anclado abajo, Y=380/402, comprobado compatible con
// Android/SAMP Unity). "/aceptarmision" en rojo: los PlayerTextDraw de
// este servidor NO soportan codigos de color GXT (~r~/~w~) dentro de la
// cadena -- comprobado, solo GameTextForPlayer los soporta -- asi que la
// unica parte roja va en un textdraw aparte, alineado a la derecha del
// blanco (RIGHT termina justo donde LEFT empieza) para que se lea como
// una sola linea sin tener que adivinar el ancho del texto.
enum { RUD_TITULO, RUD_PRECIO, RUD_COMANDO, RUD_TOTAL }
static PlayerText:g_RoboExpressUrgenteTD[MAX_PLAYERS][RUD_TOTAL];

// ---------------------------------------------------------------------
//  TEXTDRAW -- copia EXACTA del estilo de /arreglarcoche
//  (53_arreglarcoche.inc: Arreglarcoche_CrearTextdraws/MostrarAviso/
//  OcultarAviso). Mismas posiciones, fuente, tamanos, colores, outline y
//  duracion -- un unico par reutilizado para los dos momentos (MISION
//  ACEPTADA y LO ENCONTRASTE), cambiando el texto con SetString antes de
//  mostrarlo en vez de crear dos pares.
// ---------------------------------------------------------------------
stock RoboExpress_CrearTextdraws(playerid)
{
    g_RoboExpressTD[playerid][RTD_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 130.0, "MISION ACEPTADA");
    PlayerTextDrawAlignment(playerid, g_RoboExpressTD[playerid][RTD_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_RoboExpressTD[playerid][RTD_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_RoboExpressTD[playerid][RTD_TITULO], 0.48, 2.0);
    PlayerTextDrawColour(playerid, g_RoboExpressTD[playerid][RTD_TITULO], 0x4FD1C5FF);
    PlayerTextDrawSetOutline(playerid, g_RoboExpressTD[playerid][RTD_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_RoboExpressTD[playerid][RTD_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_RoboExpressTD[playerid][RTD_TITULO], true);

    g_RoboExpressTD[playerid][RTD_SUBTITULO] = CreatePlayerTextDraw(playerid, 320.0, 158.0, "Busca el vehiculo");
    PlayerTextDrawAlignment(playerid, g_RoboExpressTD[playerid][RTD_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_RoboExpressTD[playerid][RTD_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_RoboExpressTD[playerid][RTD_SUBTITULO], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_RoboExpressTD[playerid][RTD_SUBTITULO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_RoboExpressTD[playerid][RTD_SUBTITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_RoboExpressTD[playerid][RTD_SUBTITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_RoboExpressTD[playerid][RTD_SUBTITULO], true);
    return 1;
}

// Segundo par -- el aviso grande "MISSION FEDERICO URGENTE!" que ve TODO
// el mundo cuando aparece la mision. Titulo y precio con la identidad
// habitual (turquesa/blanco de /arreglarcoche); "/aceptarmision" en rojo,
// en su propio textdraw alineado a la derecha del anterior. Posicion
// Y=380/402 copiada literal de 44_hambre_sed.inc (ver comentario de la
// declaracion de g_RoboExpressUrgenteTD).
stock RoboExpress_CrearTextdrawsUrgente(playerid)
{
    g_RoboExpressUrgenteTD[playerid][RUD_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 380.0, "MISSION FEDERICO URGENTE!");
    PlayerTextDrawAlignment(playerid, g_RoboExpressUrgenteTD[playerid][RUD_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_RoboExpressUrgenteTD[playerid][RUD_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_RoboExpressUrgenteTD[playerid][RUD_TITULO], 0.42, 1.8);
    PlayerTextDrawColour(playerid, g_RoboExpressUrgenteTD[playerid][RUD_TITULO], 0x4FD1C5FF);
    PlayerTextDrawSetOutline(playerid, g_RoboExpressUrgenteTD[playerid][RUD_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_RoboExpressUrgenteTD[playerid][RUD_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_RoboExpressUrgenteTD[playerid][RUD_TITULO], true);

    g_RoboExpressUrgenteTD[playerid][RUD_PRECIO] = CreatePlayerTextDraw(playerid, 318.0, 402.0, "$1.000.000 -");
    PlayerTextDrawAlignment(playerid, g_RoboExpressUrgenteTD[playerid][RUD_PRECIO], TEXT_DRAW_ALIGN_RIGHT);
    PlayerTextDrawFont(playerid, g_RoboExpressUrgenteTD[playerid][RUD_PRECIO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_RoboExpressUrgenteTD[playerid][RUD_PRECIO], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_RoboExpressUrgenteTD[playerid][RUD_PRECIO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_RoboExpressUrgenteTD[playerid][RUD_PRECIO], 1);
    PlayerTextDrawSetShadow(playerid, g_RoboExpressUrgenteTD[playerid][RUD_PRECIO], 0);
    PlayerTextDrawSetProportional(playerid, g_RoboExpressUrgenteTD[playerid][RUD_PRECIO], true);

    g_RoboExpressUrgenteTD[playerid][RUD_COMANDO] = CreatePlayerTextDraw(playerid, 322.0, 402.0, "/aceptarmision");
    PlayerTextDrawAlignment(playerid, g_RoboExpressUrgenteTD[playerid][RUD_COMANDO], TEXT_DRAW_ALIGN_LEFT);
    PlayerTextDrawFont(playerid, g_RoboExpressUrgenteTD[playerid][RUD_COMANDO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_RoboExpressUrgenteTD[playerid][RUD_COMANDO], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_RoboExpressUrgenteTD[playerid][RUD_COMANDO], 0xFF0000FF);
    PlayerTextDrawSetOutline(playerid, g_RoboExpressUrgenteTD[playerid][RUD_COMANDO], 1);
    PlayerTextDrawSetShadow(playerid, g_RoboExpressUrgenteTD[playerid][RUD_COMANDO], 0);
    PlayerTextDrawSetProportional(playerid, g_RoboExpressUrgenteTD[playerid][RUD_COMANDO], true);
    return 1;
}

stock RoboExpress_OnConectar(playerid)
{
    RoboExpress_CrearTextdraws(playerid);
    RoboExpress_CrearTextdrawsUrgente(playerid);
    return 1;
}

stock RoboExpress_MostrarAviso(playerid, const titulo[], const subtitulo[])
{
    PlayerTextDrawSetString(playerid, g_RoboExpressTD[playerid][RTD_TITULO], titulo);
    PlayerTextDrawSetString(playerid, g_RoboExpressTD[playerid][RTD_SUBTITULO], subtitulo);
    PlayerTextDrawShow(playerid, g_RoboExpressTD[playerid][RTD_TITULO]);
    PlayerTextDrawShow(playerid, g_RoboExpressTD[playerid][RTD_SUBTITULO]);
    PlayerPlaySound(playerid, 1133, 0.0, 0.0, 0.0);   // identidad audiovisual: mismo sonido que /arreglarcoche
    SetTimerEx("RoboExpress_OcultarAviso", ROBOEXPRESS_AVISO_MS, false, "d", playerid);
    return 1;
}

forward RoboExpress_OcultarAviso(playerid);
public RoboExpress_OcultarAviso(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    PlayerTextDrawHide(playerid, g_RoboExpressTD[playerid][RTD_TITULO]);
    PlayerTextDrawHide(playerid, g_RoboExpressTD[playerid][RTD_SUBTITULO]);
    return 1;
}

// Aviso grande inferior a TODOS los conectados, una sola vez, en el
// instante exacto en que aparece la mision -- 1 aviso visual + 1 sonido
// por jugador, nunca repetido mientras esta visible.
stock RoboExpress_MostrarBannerGlobal()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        PlayerTextDrawShow(i, g_RoboExpressUrgenteTD[i][RUD_TITULO]);
        PlayerTextDrawShow(i, g_RoboExpressUrgenteTD[i][RUD_PRECIO]);
        PlayerTextDrawShow(i, g_RoboExpressUrgenteTD[i][RUD_COMANDO]);
        PlayerPlaySound(i, 1133, 0.0, 0.0, 0.0);
        SetTimerEx("RoboExpress_OcultarBannerGlobal", ROBOEXPRESS_AVISO_MS, false, "d", i);
    }
    return 1;
}

forward RoboExpress_OcultarBannerGlobal(playerid);
public RoboExpress_OcultarBannerGlobal(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    PlayerTextDrawHide(playerid, g_RoboExpressUrgenteTD[playerid][RUD_TITULO]);
    PlayerTextDrawHide(playerid, g_RoboExpressUrgenteTD[playerid][RUD_PRECIO]);
    PlayerTextDrawHide(playerid, g_RoboExpressUrgenteTD[playerid][RUD_COMANDO]);
    return 1;
}

// Mayusculas simple (A-Z) -- los nombres de Veh_NombreModelo son ASCII
// sin acentos, para "BULLET" en el aviso de mision aceptada tal como se
// pidio.
stock RoboExpress_Mayusculas(texto[])
{
    for (new i = 0; texto[i] != EOS; i++)
    {
        if (texto[i] >= 'a' && texto[i] <= 'z') texto[i] -= 32;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  SELECCION DEL VEHICULO
// ---------------------------------------------------------------------
stock RoboExpress_VehiculosElegibles(destino[], maxVehiculos)
{
    new total = 0;
    for (new i = 0; i < MAX_VEHICULOS && total < maxVehiculos; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] != 0) continue;
        if (g_Vehiculos[i][veObjeto] == INVALID_VEHICLE_ID) continue;

        new modelo = g_Vehiculos[i][veModelo];
        if (Federico_ModeloExcluido(modelo)) continue;
        if (Federico_ContarModelo(modelo) != 1) continue;   // SUPER RARE real, misma regla que Federico_Rareza

        // El Personaje Historico vigente es "libre" en la tabla pero es
        // un vehiculo especial -- nunca objetivo de Robo Express.
        if (g_FedHistEstado == FEDHIST_BUSCANDO && g_Vehiculos[i][veID] == g_FedHistVehiculoID) continue;

        // Exclusion mutua con el resto de categorias activas de
        // Federico -- preguntado directamente con sus propias
        // funciones, nunca una segunda lista hardcodeada.
        if (Federico_SlotDeModelo(modelo) != -1) continue;
        if (Federico_EsVehiculoSecreto(modelo)) continue;
        if (Federico_EsVehiculoMoto(modelo)) continue;
        if (Federico_ChatarraSlotDeModelo(modelo) != -1) continue;

        destino[total] = i;
        total++;
    }
    return total;
}

// ---------------------------------------------------------------------
//  API PUBLICA -- esto es lo unico que llamara el futuro controlador de
//  4 misiones. Devuelve false SIN dejar ningun estado a medias si ya
//  hay una activa o si no hay ningun elegible ahora mismo.
// ---------------------------------------------------------------------
stock bool:RoboExpress_Iniciar()
{
    if (g_RoboExpressActiva) return false;

    new elegibles[MAX_VEHICULOS];
    new total = RoboExpress_VehiculosElegibles(elegibles, sizeof elegibles);
    if (total == 0) return false;

    new indice = elegibles[random(total)];
    new modelo = g_Vehiculos[indice][veModelo];

    g_RoboExpressActiva                = true;
    g_RoboExpressVehiculoID            = g_Vehiculos[indice][veID];
    g_RoboExpressModelo                = modelo;
    g_RoboExpressJugador               = INVALID_PLAYER_ID;
    g_RoboExpressExpiraEn              = gettime() + ROBOEXPRESS_DURACION;
    g_RoboExpressEncontrado            = false;
    g_RoboExpressUltimoAvisoIncorrecto = 0;

    new nombreModelo[32];
    Veh_NombreModelo(modelo, nombreModelo, sizeof nombreModelo);

    new mensaje[200];
    format(mensaje, sizeof mensaje,
        "{FF0000}[Robo Express]{FFFFFF} Federico necesita AHORA MISMO un %s, entregalo por $1.000.000! Tienes 15 minutos. {FF0000}/aceptarmision",
        nombreModelo);
    MensajeGlobal(COLOR_BLANCO, mensaje);
    RoboExpress_MostrarBannerGlobal();

    new logMsg[160];
    format(logMsg, sizeof logMsg, "ROBOEXPRESS_STARTED vehiculo_id=%d modelo=%d expira_en=%d",
        g_RoboExpressVehiculoID, modelo, g_RoboExpressExpiraEn);
    EscribirLog("federico", logMsg);
    return true;
}

// Limpieza pura de memoria, sin mensajes -- los mensajes los decide
// quien llama (expiracion = global, desaparicion = privado, entrega =
// ninguno propio).
stock RoboExpress_LimpiarEstado()
{
    g_RoboExpressActiva     = false;
    g_RoboExpressVehiculoID = 0;
    g_RoboExpressModelo     = 0;
    g_RoboExpressJugador    = INVALID_PLAYER_ID;
    g_RoboExpressExpiraEn   = 0;
    g_RoboExpressEncontrado = false;
    return 1;
}

// Indice ACTUAL en g_Vehiculos del objetivo, resuelto por veID (nunca
// por indice guardado, que se recicla). -1 si la fila ya no existe.
stock RoboExpress_IndiceObjetivo()
{
    for (new i = 0; i < MAX_VEHICULOS; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veID] == g_RoboExpressVehiculoID) return i;
    }
    return -1;
}

// Diferencia clave: un vehiculo que acaba de explotar y va a respawnear
// SIGUE teniendo veUsado/veID/veObjeto intactos (OnVehicleDeath, en
// 99_callbacks.inc, solo resetea la salud -- confirmado leyendo el
// codigo antes de escribir esto). Lo unico que invalida de verdad es que
// la fila desaparezca (Veh_Borrar), cambie de tipo/dueno (comprado por
// alguien), o quede retirado del mundo por otra via de Federico
// (veObjeto puesto a INVALID_VEHICLE_ID por Federico_RetirarVehiculo).
stock bool:RoboExpress_ObjetivoValido()
{
    if (!g_RoboExpressActiva) return false;
    new indice = RoboExpress_IndiceObjetivo();
    if (indice == -1) return false;
    if (g_Vehiculos[indice][veTipo] != VEH_DROPEO) return false;
    if (g_Vehiculos[indice][vePropietario] != 0) return false;
    if (g_Vehiculos[indice][veObjeto] == INVALID_VEHICLE_ID) return false;
    return true;
}

// Llamado UNA vez desde TicMinuto (99_callbacks.inc), no por jugador --
// Robo Express es un estado global, no per-player.
stock RoboExpress_Minuto()
{
    new ahora = gettime();

    if (!g_RoboExpressCicloListo)
    {
        g_RoboExpressCicloListo   = true;
        g_RoboExpressProximoCiclo = ahora + (ROBOEXPRESS_CICLO_MINUTOS * 60);
    }

    if (!g_RoboExpressActiva)
    {
        // V1 unicamente: intento automatico cada 20 min. El futuro
        // controlador de 4 misiones sustituira esto por su propia
        // logica, llamando a RoboExpress_Iniciar() cuando le toque.
        if (ahora >= g_RoboExpressProximoCiclo)
        {
            RoboExpress_Iniciar();   // si no hay elegibles, no pasa nada, se reintenta en el siguiente ciclo
            g_RoboExpressProximoCiclo = ahora + (ROBOEXPRESS_CICLO_MINUTOS * 60);
        }
        return 1;
    }

    if (ahora >= g_RoboExpressExpiraEn)
    {
        if (g_RoboExpressJugador != INVALID_PLAYER_ID && IsPlayerConnected(g_RoboExpressJugador) && g_RoboExpressEncontrado)
        {
            DisablePlayerCheckpoint(g_RoboExpressJugador);
        }
        MensajeGlobal(COLOR_BLANCO, "{FF0000}[Robo Express]{FFFFFF} Tiempo agotado. Federico ya no necesita el vehiculo.");
        RoboExpress_LimpiarEstado();
        return 1;
    }

    if (!RoboExpress_ObjetivoValido())
    {
        if (g_RoboExpressJugador != INVALID_PLAYER_ID && IsPlayerConnected(g_RoboExpressJugador))
        {
            if (g_RoboExpressEncontrado) DisablePlayerCheckpoint(g_RoboExpressJugador);
            SendClientMessage(g_RoboExpressJugador, COLOR_BLANCO,
                "{FF0000}[Robo Express]{FFFFFF} El vehiculo objetivo ya no esta disponible. Mision cancelada.");
        }
        RoboExpress_LimpiarEstado();
        return 1;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  /ACEPTARMISION
// ---------------------------------------------------------------------
CMD:aceptarmision(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (!g_RoboExpressActiva)
    {
        Error(playerid, "No hay ninguna mision Robo Express activa ahora mismo.");
        return 1;
    }

    if (g_RoboExpressJugador != INVALID_PLAYER_ID)
    {
        Error(playerid, "Robo Express ya ha sido aceptada por otro jugador.");
        return 1;
    }

    g_RoboExpressJugador = playerid;

    new nombreModelo[32];
    Veh_NombreModelo(g_RoboExpressModelo, nombreModelo, sizeof nombreModelo);
    RoboExpress_Mayusculas(nombreModelo);

    new subtitulo[64];
    format(subtitulo, sizeof subtitulo, "Busca el \"%s\"", nombreModelo);
    RoboExpress_MostrarAviso(playerid, "MISION ACEPTADA", subtitulo);

    Exito(playerid, "Has aceptado Robo Express. El vehiculo no se marca en el mapa -- tienes que encontrarlo tu mismo.");
    return 1;
}

// ---------------------------------------------------------------------
//  DESCUBRIMIENTO -- llamado desde OnPlayerStateChange (99_callbacks.inc)
//  SOLO cuando newstate == PLAYER_STATE_DRIVER, igual que el hook
//  equivalente de Federico (Federico_AlEntrarComoConductor).
// ---------------------------------------------------------------------
stock RoboExpress_AlEntrarComoConductor(playerid, indice)
{
    if (!g_RoboExpressActiva) return 0;
    if (playerid != g_RoboExpressJugador) return 0;        // para cualquier otro jugador esto no existe
    if (g_Vehiculos[indice][veModelo] != g_RoboExpressModelo) return 0;  // ni siquiera es el modelo buscado

    if (g_Vehiculos[indice][veID] == g_RoboExpressVehiculoID)
    {
        if (g_RoboExpressEncontrado)
        {
            // Ya se habia encontrado antes (se bajo y volvio a subir) --
            // solo se refresca el checkpoint por si se perdio, sin
            // repetir el GameText ni el mensaje.
            SetPlayerCheckpoint(playerid, FEDERICO_X, FEDERICO_Y, FEDERICO_Z, FEDERICO_RADIO_ENTREGA);
            return 1;
        }

        g_RoboExpressEncontrado = true;
        RoboExpress_MostrarAviso(playerid, "LO ENCONTRASTE!", "Llevalo a Federico");
        SendClientMessage(playerid, COLOR_BLANCO,
            "{FF0000}[Robo Express]{FFFFFF} Es este! Federico paga $1.000.000 por este vehiculo. Usa /entregarcoche");

        // V1: mismo destino privado de Los Santos que ya usa Federico
        // para el resto de vehiculos buscados -- no se toca la logica
        // global de destinos.
        SetPlayerCheckpoint(playerid, FEDERICO_X, FEDERICO_Y, FEDERICO_Z, FEDERICO_RADIO_ENTREGA);
        return 1;
    }

    // Mismo modelo, vehiculo distinto -- antispam por tiempo (mismo
    // umbral que ya usa el resto de Federico, FEDERICO_AVISO_COOLDOWN).
    new ahora = gettime();
    if ((ahora - g_RoboExpressUltimoAvisoIncorrecto) < FEDERICO_AVISO_COOLDOWN) return 0;
    g_RoboExpressUltimoAvisoIncorrecto = ahora;

    new nombreModelo[32], mensaje[144];
    Veh_NombreModelo(g_RoboExpressModelo, nombreModelo, sizeof nombreModelo);
    format(mensaje, sizeof mensaje, "{FF0000}[Robo Express]{FFFFFF} Este no es el %s que busca Federico.", nombreModelo);
    SendClientMessage(playerid, COLOR_BLANCO, mensaje);
    return 1;
}

// ---------------------------------------------------------------------
//  VALIDACION PARA /entregarcoche (43_federico.inc) -- se le pasa el
//  indice YA resuelto, no vuelve a buscar el vehiculo.
// ---------------------------------------------------------------------
stock bool:RoboExpress_PuedeEntregar(playerid, indice)
{
    if (!g_RoboExpressActiva) return false;
    if (playerid != g_RoboExpressJugador) return false;
    if (gettime() >= g_RoboExpressExpiraEn) return false;
    if (g_Vehiculos[indice][veID] != g_RoboExpressVehiculoID) return false;
    return true;
}

// Usado por Federico_AlEntrarComoConductor (43_federico.inc, coherencia
// audiovisual 08/09/2026) para NO mostrar tambien el aviso GENERICO de
// Federico sobre el vehiculo EXACTO de una mision Robo Express activa --
// Robo Express tiene prioridad visual total y su propio aviso ("LO
// ENCONTRASTE!"), da igual que playerid sea o no el asignado.
stock bool:RoboExpress_EsVehiculoObjetivo(indice)
{
    if (!g_RoboExpressActiva) return false;
    return bool:(g_Vehiculos[indice][veID] == g_RoboExpressVehiculoID);
}

// Usado por 58_ascensos.inc (Server IA, 08/09/2026) para retrasar su
// mensaje institucional un minuto si coincide con una mision activa --
// prioridad: evento/mision > mensaje institucional, sin acoplarse a nada
// mas de este archivo.
stock bool:RoboExpress_HayMisionActiva()
{
    return g_RoboExpressActiva;
}

// Llamado desde 43_federico.inc justo despues de pagar y de
// Federico_RetirarVehiculo -- limpia TODO el estado y el checkpoint
// privado. Sin mensaje propio: el exito ya lo anuncia /entregarcoche.
stock RoboExpress_Finalizar()
{
    if (g_RoboExpressJugador != INVALID_PLAYER_ID && IsPlayerConnected(g_RoboExpressJugador))
    {
        DisablePlayerCheckpoint(g_RoboExpressJugador);
    }
    RoboExpress_LimpiarEstado();
    return 1;
}

// ---------------------------------------------------------------------
//  DESCONEXION -- V1: si se va el asignado, la mision termina entera
//  (no vuelve a quedar libre para otro jugador, peticion expresa).
// ---------------------------------------------------------------------
stock RoboExpress_OnDisconnect(playerid)
{
    if (!g_RoboExpressActiva) return 0;
    if (playerid != g_RoboExpressJugador) return 0;

    RoboExpress_LimpiarEstado();
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN: /forzarroboexpress -- mismo patron de permisos que
//  /forzarnomina (16_administracion.inc), para probar sin esperar el
//  ciclo de 20 min.
// ---------------------------------------------------------------------
CMD:forzarroboexpress(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    if (g_RoboExpressActiva)
    {
        Error(playerid, "Ya hay una mision Robo Express activa ahora mismo.");
        return 1;
    }

    if (!RoboExpress_Iniciar())
    {
        Error(playerid, "No hay ningun vehiculo SUPER RARE elegible ahora mismo para Robo Express.");
        return 1;
    }

    Exito(playerid, "Robo Express iniciado.");
    Admin_Registrar(playerid, "ROBOEXPRESS", "-", "Inicio forzado");
    return 1;
}
