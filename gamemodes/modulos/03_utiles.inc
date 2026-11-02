/* =====================================================================
 *  03_utiles.inc
 *  ---------------------------------------------------------------------
 *  Funciones auxiliares de uso general. Todo lo que se repite en mas de
 *  un modulo acaba aqui para no duplicar codigo.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  Formatea sobre el buffer global. Uso tipico:
//      Formato("Has recibido %s.", dinero);
//      Info(playerid, g_Buffer);
//  Evita tener que declarar un array temporal en cada funcion.
// ---------------------------------------------------------------------
#define Formato(%0)             format(g_Buffer, sizeof(g_Buffer), %0)

// Comprueba si una cadena esta vacia (compatible con el parametro de CMD:)
#define EstaVacio(%0)           ((%0)[0] == EOS || ((%0)[0] == 1 && (%0)[1] == EOS))

// ---------------------------------------------------------------------
//  MENSAJES DEL SISTEMA
// ---------------------------------------------------------------------
stock Info(playerid, const texto[])
{
    new linea[160];
    format(linea, sizeof linea, PREFIJO "%s", texto);
    return SendClientMessage(playerid, COLOR_INFO, linea);
}

stock Exito(playerid, const texto[])
{
    new linea[160];
    format(linea, sizeof linea, PREFIJO "%s", texto);
    return SendClientMessage(playerid, COLOR_EXITO, linea);
}

stock Error(playerid, const texto[])
{
    new linea[160];
    format(linea, sizeof linea, PREFIJO "%s", texto);
    return SendClientMessage(playerid, COLOR_ERROR, linea);
}

stock Aviso(playerid, const texto[])
{
    new linea[160];
    format(linea, sizeof linea, PREFIJO "%s", texto);
    return SendClientMessage(playerid, COLOR_AVISO, linea);
}

// Mensaje sin prefijo, para bloques de texto y listados.
stock Linea(playerid, color, const texto[])
{
    return SendClientMessage(playerid, color, texto);
}

// Mensaje para todo el servidor.
stock MensajeGlobal(color, const texto[])
{
    return SendClientMessageToAll(color, texto);
}

// ---------------------------------------------------------------------
//  NOMBRES
// ---------------------------------------------------------------------

// Nombre tal cual lo tiene el jugador (Juan_Perez).
stock NombreCuenta(playerid, destino[], tam = MAX_PLAYER_NAME + 1)
{
    GetPlayerName(playerid, destino, tam);
    return 1;
}

// Nombre listo para mostrar en rol (Juan Perez).
stock NombreRP(playerid, destino[], tam = MAX_PLAYER_NAME + 1)
{
    GetPlayerName(playerid, destino, tam);
    for (new i = 0; destino[i] != EOS; i++)
    {
        if (destino[i] == '_') destino[i] = ' ';
    }
    return 1;
}

// Convierte cualquier cadena Nombre_Apellido en Nombre Apellido.
stock SubrayadoAEspacio(destino[])
{
    for (new i = 0; destino[i] != EOS; i++)
    {
        if (destino[i] == '_') destino[i] = ' ';
    }
    return 1;
}

// Valida el nombre de conexion. Formato de rol Nombre_Apellido preferido
// (un guion bajo), pero un nombre de una sola palabra sin guion TAMBIEN
// se admite -- cambio silencioso pedido por Vlad, 12/09/2026, para dejar
// entrar a cualquiera sin anunciarlo. Solo letras, longitud 5..MAX, y si
// hay guion bajo sigue sin poder ir al principio/final ni ir doble.
stock bool:EsNombreRP(const nombre[])
{
    new guiones = 0, largo = strlen(nombre);

    if (largo < 5 || largo > MAX_PLAYER_NAME) return false;

    for (new i = 0; i < largo; i++)
    {
        if (nombre[i] == '_')
        {
            guiones++;
            // No puede empezar ni terminar por guion, ni llevar dos seguidos.
            if (i == 0 || i == largo - 1 || nombre[i + 1] == '_') return false;
            continue;
        }
        if (!((nombre[i] >= 'a' && nombre[i] <= 'z') || (nombre[i] >= 'A' && nombre[i] <= 'Z')))
        {
            return false;
        }
    }
    return (guiones == 0 || guiones == 1);
}

// ---------------------------------------------------------------------
//  FORMATO DE CIFRAS Y TIEMPOS
// ---------------------------------------------------------------------

// 1234567 -> "1.234.567"
stock FormatearNumero(cantidad, destino[], tam)
{
    new bruto[24], negativo = 0, largo, pos = 0;

    if (cantidad < 0) { negativo = 1; cantidad = -cantidad; }

    format(bruto, sizeof bruto, "%d", cantidad);
    largo = strlen(bruto);

    if (negativo) destino[pos++] = '-';

    for (new i = 0; i < largo && pos < tam - 1; i++)
    {
        destino[pos++] = bruto[i];
        new restantes = largo - i - 1;
        if (restantes > 0 && restantes % 3 == 0 && pos < tam - 1) destino[pos++] = '.';
    }
    destino[pos] = EOS;
    return 1;
}

// 1234567 -> "1.234.567$"
stock FormatearDinero(cantidad, destino[], tam)
{
    new numero[24];
    FormatearNumero(cantidad, numero, sizeof numero);
    format(destino, tam, "%s$", numero);
    return 1;
}

// 3725 segundos -> "1h 02m 05s"
stock FormatearTiempo(segundos, destino[], tam)
{
    new h = segundos / 3600;
    new m = (segundos % 3600) / 60;
    new s = segundos % 60;

    if (h > 0)      format(destino, tam, "%dh %02dm %02ds", h, m, s);
    else if (m > 0) format(destino, tam, "%dm %02ds", m, s);
    else            format(destino, tam, "%ds", s);
    return 1;
}

// 185 minutos -> "3 horas y 5 minutos"
stock FormatearMinutos(minutos, destino[], tam)
{
    new h = minutos / 60;
    new m = minutos % 60;

    if (h == 0)      format(destino, tam, "%d minuto%s", m, (m == 1) ? ("") : ("s"));
    else if (m == 0) format(destino, tam, "%d hora%s", h, (h == 1) ? ("") : ("s"));
    else             format(destino, tam, "%d hora%s y %d minuto%s", h, (h == 1) ? ("") : ("s"), m, (m == 1) ? ("") : ("s"));
    return 1;
}

// Fecha y hora del servidor: "03/08/2026 21:15"
stock FechaHoraActual(destino[], tam)
{
    new a, me, d, h, mi, s;
    getdate(a, me, d);
    gettime(h, mi, s);
    format(destino, tam, "%02d/%02d/%d %02d:%02d", d, me, a, h, mi);
    return 1;
}

// ---------------------------------------------------------------------
//  VALIDACION DE ENTRADAS
// ---------------------------------------------------------------------
stock bool:EsNumeroEntero(const texto[])
{
    if (texto[0] == EOS) return false;

    new inicio = (texto[0] == '-') ? 1 : 0;
    if (texto[inicio] == EOS) return false;

    for (new i = inicio; texto[i] != EOS; i++)
    {
        if (texto[i] < '0' || texto[i] > '9') return false;
    }
    return true;
}

// Rechaza caracteres que puedan romper un dialogo o el chat.
stock bool:TextoSeguro(const texto[])
{
    for (new i = 0; texto[i] != EOS; i++)
    {
        if (texto[i] == '%' || texto[i] == '\r' || texto[i] == '\n') return false;
    }
    return true;
}

// Sustituye los % por espacios para que nunca rompan un format().
stock LimpiarTexto(texto[])
{
    for (new i = 0; texto[i] != EOS; i++)
    {
        if (texto[i] == '%' || texto[i] == '\r' || texto[i] == '\n') texto[i] = ' ';
    }
    return 1;
}

// Pone la primera letra en mayuscula y anade punto final si falta.
stock CuidarFrase(texto[], tam)
{
    new largo = strlen(texto);
    if (largo == 0) return 0;

    if (texto[0] >= 'a' && texto[0] <= 'z') texto[0] -= 32;

    new ultimo = texto[largo - 1];
    if (ultimo != '.' && ultimo != '!' && ultimo != '?' && largo < tam - 2)
    {
        texto[largo] = '.';
        texto[largo + 1] = EOS;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  BUSQUEDA DE JUGADORES
//  Acepta un ID numerico o parte del nombre. Devuelve INVALID_PLAYER_ID
//  si no hay coincidencia, o -2 si el nombre es ambiguo.
// ---------------------------------------------------------------------
#define JUGADOR_AMBIGUO         (-2)

stock BuscarJugador(const texto[])
{
    if (EstaVacio(texto)) return INVALID_PLAYER_ID;

    if (EsNumeroEntero(texto))
    {
        new id = strval(texto);
        if (id >= 0 && id < MAX_PLAYERS && IsPlayerConnected(id) && Jugador[id][jLogueado]) return id;
        return INVALID_PLAYER_ID;
    }

    new encontrado = INVALID_PLAYER_ID, coincidencias = 0, nombre[MAX_PLAYER_NAME + 1];

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;

        GetPlayerName(i, nombre, sizeof nombre);
        if (strfind(nombre, texto, true) != -1)
        {
            // Coincidencia exacta: gana siempre.
            if (strcmp(nombre, texto, true) == 0) return i;
            encontrado = i;
            coincidencias++;
        }
    }

    if (coincidencias > 1) return JUGADOR_AMBIGUO;
    return encontrado;
}

// Mensaje estandar para cuando BuscarJugador falla.
stock bool:JugadorInvalido(playerid, objetivo)
{
    if (objetivo == JUGADOR_AMBIGUO)
    {
        Error(playerid, "Hay varios jugadores con ese nombre. Usa el ID o escribe mas letras.");
        return true;
    }
    if (objetivo == INVALID_PLAYER_ID || !IsPlayerConnected(objetivo) || !Jugador[objetivo][jLogueado])
    {
        Error(playerid, "Ese jugador no esta conectado.");
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------
//  DISTANCIAS Y DIMENSIONES
// ---------------------------------------------------------------------
stock bool:EnMismaDimension(playerid, objetivo)
{
    return (GetPlayerInterior(playerid) == GetPlayerInterior(objetivo)
         && GetPlayerVirtualWorld(playerid) == GetPlayerVirtualWorld(objetivo));
}

stock Float:DistanciaEntreJugadores(playerid, objetivo)
{
    new Float:x, Float:y, Float:z;
    GetPlayerPos(objetivo, x, y, z);
    return GetPlayerDistanceFromPoint(playerid, x, y, z);
}

stock bool:EstaCerca(playerid, objetivo, Float:radio)
{
    if (!EnMismaDimension(playerid, objetivo)) return false;
    return (DistanciaEntreJugadores(playerid, objetivo) <= radio);
}

stock bool:EnPunto(playerid, Float:radio, Float:x, Float:y, Float:z)
{
    return (GetPlayerDistanceFromPoint(playerid, x, y, z) <= radio);
}

// ---------------------------------------------------------------------
//  DINERO
//  jDinero es la unica fuente de verdad. El HUD siempre se sincroniza
//  desde ahi, asi nunca hay descuadres con el dinero del cliente.
// ---------------------------------------------------------------------
stock ActualizarDineroHUD(playerid)
{
    ResetPlayerMoney(playerid);
    if (Jugador[playerid][jDinero] != 0) GivePlayerMoney(playerid, Jugador[playerid][jDinero]);
    return 1;
}

// esIngreso=false es SOLO para mover el propio dinero entre efectivo y
// banco (ver /banco -> Retirar, 15_banco.inc) -- no es dinero nuevo, no
// debe sumar a "Ganancias ultima hora" (55_ganancias.inc). Cualquier
// otra llamada real a DarDinero es un ingreso legitimo.
stock DarDinero(playerid, cantidad, bool:esIngreso = true)
{
    Jugador[playerid][jDinero] += cantidad;
    if (Jugador[playerid][jDinero] < 0) Jugador[playerid][jDinero] = 0;
    ActualizarDineroHUD(playerid);
    if (cantidad > 0 && esIngreso) Ganancias_Registrar(playerid, cantidad);
    return Jugador[playerid][jDinero];
}

stock bool:TieneDinero(playerid, cantidad)
{
    return (Jugador[playerid][jDinero] >= cantidad);
}

// ---------------------------------------------------------------------
//  ADMINISTRACION
// ---------------------------------------------------------------------
stock bool:EsAdmin(playerid, nivel)
{
    return (Jugador[playerid][jLogueado] && Jugador[playerid][jAdmin] >= nivel);
}

// Comprueba el nivel y avisa al jugador si no llega. Devuelve true si NO puede.
stock bool:SinPermiso(playerid, nivel)
{
    if (EsAdmin(playerid, nivel)) return false;
    Error(playerid, "No tienes permiso para usar ese comando.");
    return true;
}

// ---------------------------------------------------------------------
//  REGISTRO EN ARCHIVO
//  Escribe en scriptfiles/logs/<archivo>.log con marca de tiempo.
// ---------------------------------------------------------------------
stock EscribirLog(const archivo[], const texto[])
{
    new ruta[64], marca[32], linea[512];

    format(ruta, sizeof ruta, "logs/%s.log", archivo);
    FechaHoraActual(marca, sizeof marca);
    format(linea, sizeof linea, "[%s] %s\r\n", marca, texto);

    new File:f = fopen(ruta, io_append);
    if (!f) return 0;

    fwrite(f, linea);
    fclose(f);
    return 1;
}

// ---------------------------------------------------------------------
//  ESTADO DEL JUGADOR
// ---------------------------------------------------------------------

// Guarda posicion, vida y armadura actuales en la ficha en memoria.
public GuardarPosicionActual(playerid)
{
    if (!Jugador[playerid][jLogueado] || !Jugador[playerid][jSpawneado]) return 0;

    // Si esta en modo espectador la posicion no es real, no la tocamos.
    if (GetPlayerState(playerid) == PLAYER_STATE_SPECTATING) return 0;

    new Float:x, Float:y, Float:z, Float:a, Float:vida, Float:armadura;

    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);
    GetPlayerHealth(playerid, vida);
    GetPlayerArmour(playerid, armadura);

    Jugador[playerid][jPosX]     = x;
    Jugador[playerid][jPosY]     = y;
    Jugador[playerid][jPosZ]     = z;
    Jugador[playerid][jPosA]     = a;
    Jugador[playerid][jSalud]    = vida;
    Jugador[playerid][jArmadura] = armadura;
    Jugador[playerid][jInterior] = GetPlayerInterior(playerid);
    Jugador[playerid][jMundo]    = GetPlayerVirtualWorld(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  LOS DATOS DE REAPARICION
//  ---------------------------------------------------------------------
//  UN SOLO SITIO escribe el SetSpawnInfo, por el mismo motivo que un
//  solo sitio decide el aspecto (ver Skin_Actualizar, mas abajo).
//
//  Ojo con esto, que costo un fallo: SA-MP mete al jugador en la
//  seleccion de clase para el auto-respawn de despues de morir, y al
//  hacerlo PISA los datos de reaparicion con los de la clase. Este
//  gamemode no llama a AddPlayerClass ni una vez, asi que esos datos de
//  clase son todo ceros -- y el skin 0 es CJ. De ahi que al morir
//  renacieras de CJ y solo recuperaras tu piel al reconectar (que es
//  cuando se volvia a llamar a SetSpawnInfo, en SpawnJugadorEnMundo).
//
//  La cura es reescribirlos justo antes de cada SpawnPlayer, con el
//  aspecto y el sitio que le tocan AHORA. Ver OnPlayerRequestClass
//  (99_callbacks.inc).
// ---------------------------------------------------------------------
stock Spawn_RefrescarDatos(playerid)
{
    // Caido: renace donde cayo, que es lo que espera la rama de herido
    // de OnPlayerSpawn. El angulo da igual, esta tumbado.
    if (Jugador[playerid][jHerido])
    {
        SetSpawnInfo(playerid, NO_TEAM, Jugador[playerid][jSkin],
            Jugador[playerid][jCaidaX], Jugador[playerid][jCaidaY], Jugador[playerid][jCaidaZ],
            0.0, WEAPON_FIST, 0, WEAPON_FIST, 0, WEAPON_FIST, 0);
        return 1;
    }

    SetSpawnInfo(playerid, NO_TEAM, Jugador[playerid][jSkin],
        Jugador[playerid][jPosX], Jugador[playerid][jPosY], Jugador[playerid][jPosZ],
        Jugador[playerid][jPosA], WEAPON_FIST, 0, WEAPON_FIST, 0, WEAPON_FIST, 0);
    return 1;
}

// Coloca al jugador en su ultima posicion guardada y lo deja jugable.
public SpawnJugadorEnMundo(playerid)
{
    TogglePlayerSpectating(playerid, false);
    Spawn_RefrescarDatos(playerid);
    SpawnPlayer(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  EL ASPECTO DEL JUGADOR
//  ---------------------------------------------------------------------
//  UN SOLO SITIO decide que aspecto te toca. Antes cada sistema hacia su
//  SetPlayerSkin por su cuenta y nadie deshacia nada: entrabas en la NFS,
//  te ponian el mono de la grua, dejabas el trabajo y te quedabas de
//  gruista de por vida.
//
//  El orden manda, y es este:
//
//    1. Uniforme de FACCION, segun tu rango. Manda sobre todo lo demas:
//       un policia es policia aunque ademas reparta pizzas.
//    2. Uniforme de TRABAJO, si ese trabajo tiene.
//    3. Tu aspecto de siempre (jSkinInicial), el que elegiste al crear
//       el personaje.
//
//  Se llama sola en los cuatro momentos en que esto puede cambiar:
//  entrar y salir de una faccion (Faccion_Dar/Quitar), entrar y salir de
//  un trabajo (Trabajo_Dar/Quitar), ascender o degradar, y al aplicar la
//  ficha al conectar.
// ---------------------------------------------------------------------
stock Skin_Actualizar(playerid)
{
    new skin = 0;

    if (Jugador[playerid][jFaccion] > 0)
    {
        skin = Faccion_SkinDe(Jugador[playerid][jFaccion], Jugador[playerid][jFaccionRango]);
    }

    if (skin == 0)
    {
        for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
        {
            skin = Trabajo_SkinDe(Jugador[playerid][jTrabajo][i]);
            if (skin != 0) break;
        }
    }

    // Ni uniforme de faccion ni de trabajo: a casa, con tu ropa.
    //
    // El 0 de jSkinInicial solo lo tienen las cuentas anteriores a la
    // migracion 033 que ademas no pasaron por ella (no deberia haber
    // ninguna). Se deja el aspecto actual antes que ponerle a nadie el
    // skin 0, que es CJ.
    if (skin == 0) skin = Jugador[playerid][jSkinInicial];
    if (skin == 0) skin = Jugador[playerid][jSkin];
    if (skin == 0) return 0;

    Jugador[playerid][jSkin] = skin;

    if (IsPlayerConnected(playerid)) SetPlayerSkin(playerid, skin);
    return skin;
}

// Aplica al mundo real todo lo que hay en la ficha del jugador.
stock AplicarDatosJugador(playerid)
{
    // Skin_Actualizar en vez de SetPlayerSkin a secas: al conectar hay
    // que volver a decidir el aspecto, no repetir el ultimo guardado.
    // Si al jugador lo expulsaron de la faccion estando desconectado, su
    // uniforme tiene que caerse solo al entrar.
    Skin_Actualizar(playerid);
    SetPlayerPos(playerid, Jugador[playerid][jPosX], Jugador[playerid][jPosY], Jugador[playerid][jPosZ]);
    SetPlayerFacingAngle(playerid, Jugador[playerid][jPosA]);
    SetPlayerInterior(playerid, Jugador[playerid][jInterior]);
    SetPlayerVirtualWorld(playerid, Jugador[playerid][jMundo]);
    SetPlayerHealth(playerid, Jugador[playerid][jSalud]);
    SetPlayerArmour(playerid, Jugador[playerid][jArmadura]);
    SetPlayerScore(playerid, Jugador[playerid][jNivel]);
    SetCameraBehindPlayer(playerid);
    ActualizarDineroHUD(playerid);
    return 1;
}

// Deja la ficha del jugador en blanco (al conectar y al desconectar).
stock ReiniciarJugador(playerid)
{
    new vacio[E_JUGADOR];
    Jugador[playerid] = vacio;

    Jugador[playerid][jSexo]     = 1;
    Jugador[playerid][jEdad]     = 18;
    Jugador[playerid][jSkin]     = 26;
    Jugador[playerid][jSkinInicial] = 0;
    Jugador[playerid][jNivel]    = 1;
    Jugador[playerid][jSalud]    = 100.0;
    Jugador[playerid][jArmadura] = 0.0;
    Jugador[playerid][jHambre]   = 100.0;
    Jugador[playerid][jSed]      = 100.0;
    Jugador[playerid][jPosX]     = SPAWN_X;
    Jugador[playerid][jPosY]     = SPAWN_Y;
    Jugador[playerid][jPosZ]     = SPAWN_Z;
    Jugador[playerid][jPosA]     = SPAWN_A;
    Jugador[playerid][jUltimaActividad] = GetTickCount();
    Jugador[playerid][jSpecObjetivo]    = INVALID_PLAYER_ID;
    Jugador[playerid][jVehiculoDentro]  = -1;   // 0 seria un indice valido
    Jugador[playerid][jVehiculoPasajero] = -1;

    // Policia (37_policia.inc). El resto de campos ya vienen a 0 del
    // `vacio` de arriba; estos dos guardan un playerid, y 0 es un id
    // valido -- si se quedaran a 0, el jugador 0 saldria como el agente
    // que te esposo o que te puso la multa.
    Jugador[playerid][jEsposadoPor] = INVALID_PLAYER_ID;
    Jugador[playerid][jMultaAgente] = INVALID_PLAYER_ID;

    format(Jugador[playerid][jOrigen], 32, "Malaga");
    return 1;
}

// ---------------------------------------------------------------------
//  ANIMACIONES
//  Precarga la libreria para que la animacion se vea a la primera.
// ---------------------------------------------------------------------
stock Animar(playerid, const libreria[], const nombre[], Float:velocidad, bool:bucle, bool:bloqx, bool:bloqy, bool:congelar, tiempo, bool:forzar = true)
{
    if (forzar) ClearAnimations(playerid);
    ApplyAnimation(playerid, libreria, nombre, velocidad, bucle, bloqx, bloqy, congelar, tiempo, SYNC_ALL);
    return 1;
}
