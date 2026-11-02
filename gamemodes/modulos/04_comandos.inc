/* =====================================================================
 *  04_comandos.inc
 *  ---------------------------------------------------------------------
 *  Procesador de comandos del servidor.
 *
 *  Para crear un comando nuevo, en cualquier modulo:
 *
 *      CMD:saludar(playerid, params[])
 *      {
 *          Info(playerid, "Hola.");
 *          return 1;
 *      }
 *
 *  Para crear un alias de otro comando:
 *
 *      CMD:hola(playerid, params[]) return cmd_saludar(playerid, params);
 *
 *  Si el comando devuelve 0, el jugador vera el aviso de comando
 *  desconocido. Devuelve siempre 1 salvo que quieras ese comportamiento.
 * ===================================================================== */

#define CMD:%1(%2)              forward cmd_%1(%2); public cmd_%1(%2)

#define MAX_LARGO_COMANDO       32
#define MAX_LARGO_PARAMS        144

// Milisegundos minimos entre dos comandos del mismo jugador.
#define COMANDO_ESPERA          250

new g_UltimoComando[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  Separa "/comando parametros" y llama a cmd_comando.
//  Devuelve:  1 ejecutado  ·  0 no existe  ·  -1 bloqueado por espera
// ---------------------------------------------------------------------
stock ProcesarComando(playerid, const cmdtext[])
{
    new indice = 1;

    // Permite escribir "/ comando" sin romperse.
    while (cmdtext[indice] == ' ') indice++;
    if (cmdtext[indice] == EOS) return 0;

    // Anti spam de comandos.
    new ahora = GetTickCount();
    if (ahora - g_UltimoComando[playerid] < COMANDO_ESPERA) return -1;
    g_UltimoComando[playerid] = ahora;

    // Construye el nombre de la funcion: cmd_<comando>
    new funcion[MAX_LARGO_COMANDO + 4];
    funcion[0] = 'c';
    funcion[1] = 'm';
    funcion[2] = 'd';
    funcion[3] = '_';

    new pos = 4;
    while (cmdtext[indice] > ' ' && pos < sizeof(funcion) - 1)
    {
        new caracter = cmdtext[indice];
        if (caracter >= 'A' && caracter <= 'Z') caracter += 32;   // a minusculas
        funcion[pos++] = caracter;
        indice++;
    }
    funcion[pos] = EOS;

    // Si el comando era mas largo que el limite, no existe.
    if (cmdtext[indice] > ' ') return 0;

    // El resto de la linea son los parametros.
    while (cmdtext[indice] == ' ') indice++;

    new params[MAX_LARGO_PARAMS];
    if (cmdtext[indice] == EOS)
    {
        // sscanf y EstaVacio entienden este valor como "sin parametros".
        params[0] = 1;
        params[1] = EOS;
    }
    else
    {
        strmid(params, cmdtext, indice, strlen(cmdtext), sizeof params);
    }

    if (funcidx(funcion) == -1) return 0;

    return CallLocalFunction(funcion, "is", playerid, params);
}

// ---------------------------------------------------------------------
//  Guardas de uso frecuente al principio de un comando.
// ---------------------------------------------------------------------

// Devuelve true (y avisa) si el jugador todavia no ha iniciado sesion.
stock bool:NoLogueado(playerid)
{
    if (Jugador[playerid][jLogueado]) return false;
    Error(playerid, "Tienes que iniciar sesion para usar comandos.");
    return true;
}

// Guarda estandar: sesion iniciada.
stock bool:NoPuedeJugar(playerid)
{
    if (NoLogueado(playerid)) return true;
    return false;
}

// Devuelve true (y avisa) si el jugador esta silenciado.
stock bool:EstaSilenciado(playerid)
{
    if (Jugador[playerid][jMuteSegundos] <= 0) return false;

    new tiempo[32];
    FormatearTiempo(Jugador[playerid][jMuteSegundos], tiempo, sizeof tiempo);
    Formato("Estas silenciado. Te quedan %s.", tiempo);
    Error(playerid, g_Buffer);
    return true;
}

// Devuelve true (y avisa) si el jugador esta al volante de un vehiculo.
stock bool:VaConduciendo(playerid)
{
    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER) return false;
    Error(playerid, "No puedes hacer eso mientras conduces.");
    return true;
}

// Devuelve true (y avisa) si el jugador esta herido en el suelo.
stock bool:EstaHerido(playerid)
{
    if (!Jugador[playerid][jHerido]) return false;
    Error(playerid, "Estas herido, no puedes hacer eso ahora mismo.");
    return true;
}

// ---------------------------------------------------------------------
//  Recordatorio de sintaxis unificado para todos los comandos.
// ---------------------------------------------------------------------
stock Uso(playerid, const sintaxis[])
{
    new linea[160];
    format(linea, sizeof linea, PREFIJO HEX_GRIS "Uso: %s", sintaxis);
    return SendClientMessage(playerid, COLOR_GRIS, linea);
}

// ---------------------------------------------------------------------
//  Codigo secreto de un solo uso. Sin pasar por el sistema de admin a
//  proposito -- el UPDATE con "codigo_dani_usado = 0" en el WHERE es
//  atomico en la propia base de datos, asi que ni el lag ni escribirlo
//  dos veces seguidas lo puede canjear mas de una vez por cuenta.
// ---------------------------------------------------------------------
CMD:danijailovic(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new consulta[150];
    format(consulta, sizeof consulta,
        "UPDATE cuentas SET banco = banco + 5000000, codigo_dani_usado = 1 WHERE id = %d AND codigo_dani_usado = 0",
        Jugador[playerid][jID]);
    mysql_pquery(g_BD, consulta, "AlUsarCodigoDani", "d", playerid);
    return 1;
}

forward AlUsarCodigoDani(playerid);
public AlUsarCodigoDani(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    if (cache_affected_rows() == 0)
    {
        Error(playerid, "Ese codigo ya no esta disponible.");
        return 0;
    }

    Jugador[playerid][jBanco] += 5000000;
    Exito(playerid, "Codigo canjeado. Se han ingresado 5.000.000$ en tu banco.");

    format(g_Buffer, sizeof g_Buffer, "CODIGO DANIJAILOVIC usado: cuenta %d (%s)", Jugador[playerid][jID], Jugador[playerid][jNombre]);
    EscribirLog("economia", g_Buffer);
    return 1;
}
