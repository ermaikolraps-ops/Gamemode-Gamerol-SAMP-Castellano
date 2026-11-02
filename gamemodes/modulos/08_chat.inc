/* =====================================================================
 *  08_chat.inc
 *  ---------------------------------------------------------------------
 *  Chat de rol por cercanía y comandos de comunicación.
 *
 *  Colores (los clásicos de servidor de rol):
 *    blanco    hablar normal y gritar
 *    morado    /me y /do (acciones y descripciones)
 *    gris      /b y /ooc (fuera de personaje)
 *    amarillo  susurros
 *    dorado    mensajes privados
 * ===================================================================== */

// Distancia a la que se ve el bocadillo de texto sobre la cabeza.
#define BURBUJA_DISTANCIA       20.0
#define BURBUJA_DURACION        6000

// Cooldown compartido para todo lo que es "hablar" (chat normal, grito,
// voz baja, /me, /do, /ame, /ado): evita que se pueda saturar el chat de
// cercania alternando entre estos comandos sin limite alguno.
#define CHAT_COOLDOWN_MS         2500
static g_UltimoChat[MAX_PLAYERS];   // GetTickCount() de la ultima vez que hablo

stock bool:Chat_EnCooldown(playerid)
{
    new ahora = GetTickCount();
    new transcurrido = ahora - g_UltimoChat[playerid];

    if (transcurrido < CHAT_COOLDOWN_MS)
    {
        format(g_Buffer, sizeof g_Buffer, "Espera un momento antes de volver a hablar (%d s).",
            ((CHAT_COOLDOWN_MS - transcurrido) / 1000) + 1);
        Error(playerid, g_Buffer);
        return true;
    }

    g_UltimoChat[playerid] = ahora;
    return false;
}

// ---------------------------------------------------------------------
//  ENVÍO POR CERCANÍA
//  El mensaje se atenúa con la distancia: quien está lejos lo ve más
//  apagado, igual que si le llegara peor la voz.
// ---------------------------------------------------------------------
public MensajeCercanoPos(Float:x, Float:y, Float:z, interior, mundo, Float:radio, color, const texto[])
{
    new Float:distancia;

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado] || !Jugador[i][jSpawneado]) continue;
        if (GetPlayerInterior(i) != interior || GetPlayerVirtualWorld(i) != mundo) continue;

        distancia = GetPlayerDistanceFromPoint(i, x, y, z);
        if (distancia > radio) continue;

        // Tres niveles de intensidad según lo cerca que esté.
        if (distancia <= radio * 0.45)      SendClientMessage(i, color, texto);
        else if (distancia <= radio * 0.75) SendClientMessage(i, Chat_Atenuar(color, 45), texto);
        else                                SendClientMessage(i, Chat_Atenuar(color, 70), texto);
    }
    return 1;
}

public MensajeCercano(playerid, Float:radio, color, const texto[])
{
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    return MensajeCercanoPos(x, y, z, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid), radio, color, texto);
}

// Oscurece un color RGBA un porcentaje dado, manteniendo el alfa.
stock Chat_Atenuar(color, porcentaje)
{
    new r = (color >>> 24) & 0xFF;
    new g = (color >>> 16) & 0xFF;
    new b = (color >>>  8) & 0xFF;
    new a =  color         & 0xFF;

    r = r - (r * porcentaje) / 100;
    g = g - (g * porcentaje) / 100;
    b = b - (b * porcentaje) / 100;

    return (r << 24) | (g << 16) | (b << 8) | a;
}

// ---------------------------------------------------------------------
//  CHAT NORMAL (lo que se escribe sin barra)
//  Un "!" al principio grita y un "#" habla bajo, como atajo rápido.
// ---------------------------------------------------------------------
stock Chat_Hablar(playerid, texto[])
{
    if (EstaSilenciado(playerid)) return 0;
    if (Chat_EnCooldown(playerid)) return 0;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    LimpiarTexto(texto);

    if (texto[0] == '!' && texto[1] != EOS)
    {
        format(mensaje, sizeof mensaje, "%s grita: %s!", nombre, texto[1]);
        MensajeCercano(playerid, RADIO_GRITO, COLOR_GRITO, mensaje);
        SetPlayerChatBubble(playerid, texto[1], COLOR_GRITO, BURBUJA_DISTANCIA * 1.5, BURBUJA_DURACION);
        return 0;
    }

    if (texto[0] == '#' && texto[1] != EOS)
    {
        format(mensaje, sizeof mensaje, "%s dice en voz baja: %s", nombre, texto[1]);
        MensajeCercano(playerid, RADIO_SUSURRO * 2.5, COLOR_SUSURRO, mensaje);
        SetPlayerChatBubble(playerid, texto[1], COLOR_SUSURRO, BURBUJA_DISTANCIA / 3.0, BURBUJA_DURACION);
        return 0;
    }

    format(mensaje, sizeof mensaje, "%s dice: %s", nombre, texto);
    MensajeCercano(playerid, RADIO_NORMAL, COLOR_IC, mensaje);
    SetPlayerChatBubble(playerid, texto, COLOR_IC, BURBUJA_DISTANCIA, BURBUJA_DURACION);

    format(g_Buffer, sizeof g_Buffer, "%s: %s", nombre, texto);
    EscribirLog("chat", g_Buffer);
    return 0;
}

// ---------------------------------------------------------------------
//  ACCIONES Y DESCRIPCIONES
// ---------------------------------------------------------------------
CMD:me(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/me [accion que realiza tu personaje]");
        Linea(playerid, COLOR_GRIS, "Ejemplo: /me saca un cigarro del bolsillo y lo enciende.");
        return 1;
    }
    if (Chat_EnCooldown(playerid)) return 1;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    LimpiarTexto(params);
    CuidarFrase(params, MAX_LARGO_PARAMS);

    format(mensaje, sizeof mensaje, "* %s %s", nombre, params);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
    return 1;
}

CMD:do(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/do [descripcion de la situacion o del entorno]");
        Linea(playerid, COLOR_GRIS, "Ejemplo: /do Huele a tabaco recien encendido.");
        return 1;
    }
    if (Chat_EnCooldown(playerid)) return 1;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    LimpiarTexto(params);
    CuidarFrase(params, MAX_LARGO_PARAMS);

    format(mensaje, sizeof mensaje, "* %s (( %s ))", params, nombre);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_DESCRIPCION, mensaje);
    return 1;
}

// Acción flotante sobre la cabeza del personaje.
CMD:ame(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/ame [accion breve que aparece sobre tu cabeza]");
        return 1;
    }
    if (Chat_EnCooldown(playerid)) return 1;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    LimpiarTexto(params);

    format(mensaje, sizeof mensaje, "* %s %s", nombre, params);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
    SetPlayerChatBubble(playerid, params, COLOR_ACCION, BURBUJA_DISTANCIA, BURBUJA_DURACION);
    return 1;
}

CMD:ado(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/ado [descripcion breve que aparece sobre tu cabeza]");
        return 1;
    }
    if (Chat_EnCooldown(playerid)) return 1;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    LimpiarTexto(params);

    format(mensaje, sizeof mensaje, "* %s (( %s ))", params, nombre);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_DESCRIPCION, mensaje);
    SetPlayerChatBubble(playerid, params, COLOR_DESCRIPCION, BURBUJA_DISTANCIA, BURBUJA_DURACION);
    return 1;
}

// ---------------------------------------------------------------------
//  VOLUMEN DE VOZ
// ---------------------------------------------------------------------
CMD:gritar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/gritar [lo que gritas]");
        return 1;
    }
    if (Chat_EnCooldown(playerid)) return 1;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    LimpiarTexto(params);

    format(mensaje, sizeof mensaje, "%s grita: %s!", nombre, params);
    MensajeCercano(playerid, RADIO_GRITO, COLOR_GRITO, mensaje);
    SetPlayerChatBubble(playerid, params, COLOR_GRITO, BURBUJA_DISTANCIA * 1.5, BURBUJA_DURACION);
    return 1;
}
CMD:g(playerid, params[]) return cmd_gritar(playerid, params);

CMD:l(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/l [lo que dices en voz baja]");
        return 1;
    }
    if (Chat_EnCooldown(playerid)) return 1;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    LimpiarTexto(params);

    format(mensaje, sizeof mensaje, "%s dice en voz baja: %s", nombre, params);
    MensajeCercano(playerid, RADIO_SUSURRO * 2.5, COLOR_SUSURRO, mensaje);
    SetPlayerChatBubble(playerid, params, COLOR_SUSURRO, BURBUJA_DISTANCIA / 3.0, BURBUJA_DURACION);
    return 1;
}
CMD:bajo(playerid, params[]) return cmd_l(playerid, params);

// /s es el atajo de /l: susurrar es hablar en voz muy baja, por
// cercania (no dirigido a un id concreto). El antiguo /susurrar [id]
// [texto] se elimino (2026-08-12) para no tener dos comandos haciendo
// casi lo mismo.
CMD:s(playerid, params[]) return cmd_l(playerid, params);

// ---------------------------------------------------------------------
//  FUERA DE PERSONAJE
// ---------------------------------------------------------------------
CMD:b(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/b [comentario fuera de personaje]");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    LimpiarTexto(params);

    format(mensaje, sizeof mensaje, "(( %s: %s ))", nombre, params);
    MensajeCercano(playerid, RADIO_OOC_LOCAL, COLOR_OOC_LOCAL, mensaje);
    return 1;
}

CMD:ooc(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/ooc [mensaje para todo el servidor, fuera de rol]");
        return 1;
    }

    // El OOC global se limita para que no reemplace al rol.
    if (Jugador[playerid][jNivel] < 2 && Jugador[playerid][jAdmin] == 0)
    {
        Error(playerid, "Necesitas nivel 2 para usar el chat OOC global.");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    LimpiarTexto(params);

    format(mensaje, sizeof mensaje, "(( [OOC] %s: %s ))", nombre, params);
    MensajeGlobal(COLOR_OOC_GLOBAL, mensaje);
    return 1;
}
CMD:o(playerid, params[]) return cmd_ooc(playerid, params);

// /pm y /r eliminados a proposito (2026-08-12): el servidor se queda
// solo con /b y /ooc para lo que es fuera de personaje.

// ---------------------------------------------------------------------
//  INTERACCIÓN BÁSICA
// ---------------------------------------------------------------------
CMD:pagar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new objetivoTexto[24], cantidad;
    if (sscanf(params, "s[24]d", objetivoTexto, cantidad))
    {
        Uso(playerid, "/pagar [id o nombre] [cantidad]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (objetivo == playerid)
    {
        Error(playerid, "No puedes pagarte a ti mismo.");
        return 1;
    }
    if (cantidad < 1)
    {
        Error(playerid, "La cantidad tiene que ser mayor que cero.");
        return 1;
    }
    if (!TieneDinero(playerid, cantidad))
    {
        Error(playerid, "No llevas tanto dinero encima.");
        return 1;
    }
    if (!EstaCerca(playerid, objetivo, RADIO_INTERACCION))
    {
        Error(playerid, "Tienes que estar al lado de esa persona.");
        return 1;
    }

    DarDinero(playerid, -cantidad);
    DarDinero(objetivo, cantidad);

    new mio[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], dinero[24], mensaje[192];
    NombreRP(playerid, mio, sizeof mio);
    NombreRP(objetivo, suyo, sizeof suyo);
    FormatearDinero(cantidad, dinero, sizeof dinero);

    format(mensaje, sizeof mensaje, "Le has dado %s a %s.", dinero, suyo);
    Exito(playerid, mensaje);

    format(mensaje, sizeof mensaje, "%s te ha dado %s.", mio, dinero);
    Exito(objetivo, mensaje);

    format(mensaje, sizeof mensaje, "* %s le entrega algo de dinero a %s.", mio, suyo);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

    format(g_Buffer, sizeof g_Buffer, "%s pago %d a %s", mio, cantidad, suyo);
    EscribirLog("economia", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  UTILIDADES DE CHAT
// ---------------------------------------------------------------------
CMD:id(playerid, params[])
{
    if (NoLogueado(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/id [parte del nombre]");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], encontrados = 0;

    Linea(playerid, COLOR_MARCA, "Jugadores encontrados:");
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;

        GetPlayerName(i, nombre, sizeof nombre);
        if (strfind(nombre, params, true) == -1) continue;

        SubrayadoAEspacio(nombre);
        format(g_Buffer, sizeof g_Buffer, "  {FFFFFF}%s{B4B5B7} - ID %d", nombre, i);
        SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
        encontrados++;
    }

    if (encontrados == 0) Error(playerid, "Nadie coincide con esa busqueda.");
    return 1;
}

CMD:quienes(playerid, params[])
{
    if (NoLogueado(playerid)) return 1;

    new nombre[MAX_PLAYER_NAME + 1], cerca = 0;

    Linea(playerid, COLOR_MARCA, "Personas a tu alrededor:");
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado] || i == playerid) continue;
        if (!EstaCerca(playerid, i, RADIO_NORMAL)) continue;

        NombreRP(i, nombre, sizeof nombre);
        format(g_Buffer, sizeof g_Buffer, "  {FFFFFF}%s{B4B5B7} - ID %d - a %.1f metros",
            nombre, i, DistanciaEntreJugadores(playerid, i));
        SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
        cerca++;
    }

    if (cerca == 0) Linea(playerid, COLOR_GRIS, "  No hay nadie cerca.");
    return 1;
}

CMD:tiempo(playerid, params[])
{
    if (NoLogueado(playerid)) return 1;

    new h, m, s;
    gettime(h, m, s);

    format(g_Buffer, sizeof g_Buffer, "Son las %02d:%02d en Los Santos.", h, m);
    Info(playerid, g_Buffer);

    new sesion[32];
    FormatearTiempo(Jugador[playerid][jSegundosSesion], sesion, sizeof sesion);
    format(g_Buffer, sizeof g_Buffer, "Llevas %s conectado en esta sesion.", sesion);
    Info(playerid, g_Buffer);
    return 1;
}

CMD:hora(playerid, params[])
{
    if (NoLogueado(playerid)) return 1;

    new anio, mes, dia, h, m, s;
    getdate(anio, mes, dia);
    gettime(h, m, s);

    format(g_Buffer, sizeof g_Buffer, "Hora: %02d:%02d:%02d  Fecha: %02d/%02d/%04d", h, m, s, dia, mes, anio);
    Info(playerid, g_Buffer);
    return 1;
}

// /duda quitado de aqui el 11/09/2026 -- peticion expresa de Vlad de
// sustituirlo por un asistente con IA que responde solo (ver
// 61_duda.inc). /reportar sigue siendo la via para hablar con el
// equipo de verdad.
