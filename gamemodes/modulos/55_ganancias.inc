/* =====================================================================
 *  55_ganancias.inc
 *  ---------------------------------------------------------------------
 *  QoL: "Ganancias ultima hora" -- indicador individual de cuanto dinero
 *  ha INGRESADO cada jugador en sus ultimos 60 minutos de juego ACTIVO
 *  (ni AFK ni desconectado cuentan). Encargo de Vlad, 07/09/2026.
 *
 *  DOS puntos de enganche, nada mas -- analizado a fondo antes de tocar
 *  nada para no tener que meter la mano en Federico/pesca/empenos/
 *  nomina/etc. uno por uno:
 *    - DarDinero()       (03_utiles.inc)  -- todo el efectivo.
 *    - Banco_Movimiento()(15_banco.inc)   -- todo el banco.
 *  Los movimientos del propio jugador entre su efectivo y su banco
 *  (Ingresar/Retirar en /banco) estan EXCLUIDOS a proposito -- no son
 *  dinero nuevo. Ver el filtro exacto en cada uno de los dos archivos.
 *
 *  VENTANA: buffer circular de 60 casillas, una por minuto activo. Cada
 *  ingreso se suma a la casilla "actual"; cada minuto activo (llamado
 *  desde TicMinuto, igual que la nomina) se avanza una casilla y se
 *  resta del total lo que hubiera en la que toca reciclar -- asi el
 *  total siempre es la suma de los ultimos 60 minutos activos, nunca
 *  una media ni un bloque que resetea entero.
 *
 *  PERSISTENCIA: una sola columna, `cuentas.ganancias_ventana`
 *  (migracion 043), texto "indice,cubo0,cubo1,...,cubo59". El total NO
 *  se guarda -- se recalcula sumando los 60 cubos al cargar, para que
 *  nunca pueda desincronizarse de ellos. El deserializador es
 *  defensivo: cualquier cosa que no encaje exactamente dejar la ventana
 *  a $0 en vez de arriesgar una lectura fuera de rango.
 * ===================================================================== */

#define GANANCIAS_VENTANA_MINUTOS  60

static g_GananciasCubo[MAX_PLAYERS][GANANCIAS_VENTANA_MINUTOS];
static g_GananciasIndice[MAX_PLAYERS];
static g_GananciasTotal[MAX_PLAYERS];
static PlayerText:g_GananciasTD[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  TEXTDRAW
//  Reubicado y restilado 13/09/2026 a peticion de Vlad ("no debe gritar
//  ni ensuciar arriba centro"): ya no va arriba centrado en rojo Aharoni
//  Bold -- ahora Pricedown, mismo color que "PUENTEANDO..." (22_dropeo.inc,
//  0x4FD1C5FF), alineado a la derecha justo debajo de la fila de
//  HAMBRE/SED (44_hambre_sed.inc), mismo borde derecho (HAMBRESED_X).
// ---------------------------------------------------------------------
stock Ganancias_CrearTextdraw(playerid)
{
    g_GananciasTD[playerid] = CreatePlayerTextDraw(playerid, HAMBRESED_X, 61.0, "Ganancias ultima hora: $0");
    PlayerTextDrawFont(playerid, g_GananciasTD[playerid], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_GananciasTD[playerid], 0.17, 0.80);
    PlayerTextDrawColour(playerid, g_GananciasTD[playerid], HAMBRESED_COLOR_HUD);
    PlayerTextDrawSetShadow(playerid, g_GananciasTD[playerid], 1);
    PlayerTextDrawSetOutline(playerid, g_GananciasTD[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_GananciasTD[playerid], true);
    PlayerTextDrawAlignment(playerid, g_GananciasTD[playerid], TEXT_DRAW_ALIGN_RIGHT);

    // Ocultado 13/09/2026 a peticion expresa de Vlad, tras verlo en
    // directo con el resto del HUD nuevo ("no me gusta"). No se borra el
    // sistema: Ganancias_Registrar/Ganancias_Minuto y la persistencia en
    // cuentas.ganancias_ventana siguen funcionando exactamente igual,
    // solo el textdraw deja de mostrarse. PlayerTextDrawSetString sobre
    // un textdraw oculto no lo hace aparecer, así que el resto del
    // archivo no necesita ningún otro cambio.
    // PlayerTextDrawShow(playerid, g_GananciasTD[playerid]);
    return 1;
}

// Sin acentos a proposito -- ningun TextDraw de todo el gamemode los
// lleva (comprobado antes de escribir esto): la fuente de TextDraw no
// los representa bien. "ultima", no "última".
stock Ganancias_ActualizarTexto(playerid)
{
    new numero[24], texto[64];
    FormatearNumero(g_GananciasTotal[playerid], numero, sizeof numero);
    format(texto, sizeof texto, "Ganancias ultima hora: $%s", numero);
    PlayerTextDrawSetString(playerid, g_GananciasTD[playerid], texto);
    return 1;
}

// ---------------------------------------------------------------------
//  CONEXION
// ---------------------------------------------------------------------
stock Ganancias_OnConectar(playerid)
{
    Ganancias_Reiniciar(playerid);
    Ganancias_CrearTextdraw(playerid);
    return 1;
}

stock Ganancias_Reiniciar(playerid)
{
    for (new b = 0; b < GANANCIAS_VENTANA_MINUTOS; b++) g_GananciasCubo[playerid][b] = 0;
    g_GananciasIndice[playerid] = 0;
    g_GananciasTotal[playerid] = 0;
    return 1;
}

// ---------------------------------------------------------------------
//  REGISTRO DE UN INGRESO (llamado SOLO desde DarDinero y Banco_Movimiento)
// ---------------------------------------------------------------------
stock Ganancias_Registrar(playerid, cantidad)
{
    if (!Jugador[playerid][jLogueado]) return 0;
    if (cantidad <= 0) return 0;

    g_GananciasCubo[playerid][g_GananciasIndice[playerid]] += cantidad;
    g_GananciasTotal[playerid] += cantidad;
    Ganancias_ActualizarTexto(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  TICK DE UN MINUTO ACTIVO -- llamado desde TicMinuto (99_callbacks.inc),
//  igual que Payday_Minuto, con la MISMA guarda de AFK. Un jugador AFK o
//  desconectado simplemente no avanza: la ventana se queda congelada tal
//  cual estaba, ni pierde ni gana minutos.
// ---------------------------------------------------------------------
stock Ganancias_Minuto(playerid)
{
    if (!Jugador[playerid][jLogueado]) return 0;
    if (Jugador[playerid][jAFK]) return 0;

    new siguiente = (g_GananciasIndice[playerid] + 1) % GANANCIAS_VENTANA_MINUTOS;

    // La casilla que toca reciclar era la de hace exactamente 60 minutos
    // activos -- lo que tuviera sale de la ventana ahora.
    g_GananciasTotal[playerid] -= g_GananciasCubo[playerid][siguiente];
    g_GananciasCubo[playerid][siguiente] = 0;
    g_GananciasIndice[playerid] = siguiente;

    Ganancias_ActualizarTexto(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  PERSISTENCIA
// ---------------------------------------------------------------------
stock Ganancias_Serializar(playerid, salida[], tamano)
{
    format(salida, tamano, "%d", g_GananciasIndice[playerid]);

    new trozo[16];
    for (new b = 0; b < GANANCIAS_VENTANA_MINUTOS; b++)
    {
        format(trozo, sizeof trozo, ",%d", g_GananciasCubo[playerid][b]);
        strcat(salida, trozo, tamano);
    }
    return 1;
}

// Defensivo a proposito: "0" (valor por defecto de cuenta nueva), cadena
// vacia, datos incompletos, datos corruptos o un indice fuera de rango
// SIEMPRE dejan la ventana limpia a $0 -- nunca un runtime error, nunca
// una lectura fuera de rango. No se escribe nada en g_GananciasCubo/
// Indice/Total hasta que TODO el contenido ha pasado la validacion
// completa, para no dejar la ventana a medias si algo falla a mitad.
stock Ganancias_Deserializar(playerid, const cadena[])
{
    Ganancias_Reiniciar(playerid);

    new len = strlen(cadena);
    if (len == 0 || (len == 1 && cadena[0] == '0')) return 1;

    new campos[GANANCIAS_VENTANA_MINUTOS + 1];
    new totalCampos = 0;
    new trozo[16];
    new posTrozo = 0;

    for (new i = 0; i < len; i++)
    {
        if (cadena[i] >= '0' && cadena[i] <= '9')
        {
            if (posTrozo >= sizeof(trozo) - 1) return 0;   // numero imposible de largo -- corrupto
            trozo[posTrozo] = cadena[i];
            posTrozo++;
        }
        else if (cadena[i] == ',')
        {
            if (posTrozo == 0) return 0;                    // coma sin numero antes -- corrupto
            if (totalCampos >= GANANCIAS_VENTANA_MINUTOS + 1) return 0; // demasiados campos -- corrupto
            trozo[posTrozo] = EOS;
            campos[totalCampos] = strval(trozo);
            totalCampos++;
            posTrozo = 0;
        }
        else
        {
            return 0;   // caracter que no es digito ni coma -- corrupto
        }
    }

    // Ultimo campo, el que queda tras la ultima coma (o el unico si no
    // hay ninguna).
    if (posTrozo == 0) return 0;                            // termina en coma o esta vacio -- corrupto
    if (totalCampos >= GANANCIAS_VENTANA_MINUTOS + 1) return 0;
    trozo[posTrozo] = EOS;
    campos[totalCampos] = strval(trozo);
    totalCampos++;

    if (totalCampos != GANANCIAS_VENTANA_MINUTOS + 1) return 0; // datos incompletos

    new indice = campos[0];
    if (indice < 0 || indice >= GANANCIAS_VENTANA_MINUTOS) return 0; // indice fuera de rango

    // Valida TODO antes de escribir nada.
    for (new b = 0; b < GANANCIAS_VENTANA_MINUTOS; b++)
    {
        if (campos[b + 1] < 0 || campos[b + 1] > 999999999) return 0; // negativo o disparatado -- corrupto
    }

    new total = 0;
    for (new b = 0; b < GANANCIAS_VENTANA_MINUTOS; b++)
    {
        g_GananciasCubo[playerid][b] = campos[b + 1];
        total += campos[b + 1];
    }
    g_GananciasIndice[playerid] = indice;
    g_GananciasTotal[playerid] = total;
    Ganancias_ActualizarTexto(playerid);
    return 1;
}
