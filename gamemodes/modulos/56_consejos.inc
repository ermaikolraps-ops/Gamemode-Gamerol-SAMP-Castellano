/* =====================================================================
 *  56_consejos.inc
 *  ---------------------------------------------------------------------
 *  QoL: consejos automaticos por chat para que los jugadores descubran
 *  mecanicas del servidor. Encargo de Vlad, 07/09/2026.
 *
 *  Un consejo cada CONSEJOS_INTERVALO_MINUTOS minutos ACTIVOS por
 *  jugador -- reutiliza EXACTAMENTE el mismo patron que Payday y
 *  Ganancias (jAFK + TicMinuto), sin timer propio: Consejos_Minuto()
 *  se llama desde el bucle que ya recorre a todos los conectados.
 *
 *  Rotacion: baraja Fisher-Yates de los CONSEJOS_TOTAL indices, sin
 *  repetir hasta agotarla; al agotarse se vuelve a barajar, con cuidado
 *  de que el primero de la bolsa nueva no coincida con el ultimo
 *  mostrado de la bolsa anterior.
 *
 *  Coexistencia con las pistas del Historico (43_federico.inc): NO se
 *  toca ese archivo. Este modulo solo LEE dos variables suyas que ya
 *  son globales (g_FedHistEstado, g_FedHistLeyendaSiguienteAt) para no
 *  mandar un consejo el mismo minuto en que va a salir (o acaba de
 *  salir) una pista global -- si coincide, se salta un minuto activo y
 *  se reintenta el siguiente, sin resetear el contador del jugador.
 *
 *  Sin persistencia a proposito (QoL, no progreso de personaje): cada
 *  conexion arranca su propia rotacion nueva desde cero.
 *
 *  Sin acentos a proposito, mismo criterio que "Ganancias ultima hora":
 *  esto va por chat (que si los admite bien), pero se mantiene la
 *  consistencia visual con el resto de avisos cortos del servidor.
 * ===================================================================== */

#define CONSEJOS_INTERVALO_MINUTOS 10
#define CONSEJOS_TOTAL 29

new const g_Consejos[CONSEJOS_TOTAL][100] = {
    "Puedes ver los vehiculos que se buscan con (/verlista)",
    "Federico busca coches y paga bien. Consulta la lista con /verlista",
    "El Teletipo es un programa de radio llevado por Gamerol IA.",
    "/duda lo lleva la IA de Gamerol, preguntale tus dudas del servidor.",
    "Los coches libres pueden esconder loot. Prueba /forzarmaletero",
    "Puedes atracar comercios de la ciudad. Prueba /atracar",
    "Tienes loot? Vendelo en la Casa de Empenos. Encuentrala en /gps",
    "Hay 4 trabajos libres disponibles. Encuentralos en /gps",
    "El tuning de transfender se guarda para siempre! Encuentralo en /gps",
    "Puedes comprar semillas en una tienda (/gps)",
    "Buscas casa? Usa /casas para ver las propiedades libres.",
    "/forzarmaletero - Hay millones ahi fuera esperandote",
    "Compra una cana y sal a pescar. Encuentra los puntos de pesca en /gps",
    "Tienes pescado? Usa una barbacoa y prepara tus capturas con /cocinar",
    "Puedes vender tu pescado. Encuentra los puestos de venta en /gps",
    "Si tu hambre o sed baja al minimo empezaras a morirte RAPIDAMENTE (/comer)",
    "Explora Angel Pine: motel, tienda, banco y mas servicios. Usa /gps",
    "Tu coche esta hecho polvo? Hay 4 mecanicos por el mapa. Usa /gps",
    "Necesitas gasolina? Encuentra las gasolineras usando /gps",
    "Quieres ganar dinero? Abre /gps y entra en la categoria GANAR DINERO $",
    "Tu inventario admite hasta 60 kg. Usa /inv para gestionar tus objetos.",
    "Guarda pescado y recursos en el maletero de tu vehiculo con /maletero",
    "Algunos coches libres esconden autenticos premios en sus maleteros.",
    "No sabes que hacer? /gps reune actividades, trabajos y servicios.",
    "Puedes entregar vehiculos (/entregarcoche) en varios sitios (/gps)",
    "Usa /verlista para ver que coches quedan por entregar (/entregarcoche)",
    "Dicen que algunos coches libres llevan cosas muy valiosas en el maletero...",
    "Se buscan muchisimos vehiculos al dia, no los dejes escapar (/entregarcoche)",
    "Puedes personalizar tu personaje con objetos para siempre. Prueba /vertoys"
};

new g_ConsejosMinutos[MAX_PLAYERS];
new g_ConsejosOrden[MAX_PLAYERS][CONSEJOS_TOTAL];
new g_ConsejosPosicion[MAX_PLAYERS];
new g_ConsejosUltimo[MAX_PLAYERS];

stock Consejos_OnConectar(playerid)
{
    g_ConsejosMinutos[playerid] = 0;
    g_ConsejosUltimo[playerid] = -1;
    Consejos_Barajar(playerid, -1);
    return 1;
}

// Fisher-Yates. evitarPrimero: indice a evitar en la posicion 0 (el
// ultimo mostrado del ciclo anterior), -1 si no hay ninguno que evitar.
stock Consejos_Barajar(playerid, evitarPrimero)
{
    for (new i = 0; i < CONSEJOS_TOTAL; i++) g_ConsejosOrden[playerid][i] = i;

    for (new i = CONSEJOS_TOTAL - 1; i > 0; i--)
    {
        new j = random(i + 1);
        new tmp = g_ConsejosOrden[playerid][i];
        g_ConsejosOrden[playerid][i] = g_ConsejosOrden[playerid][j];
        g_ConsejosOrden[playerid][j] = tmp;
    }

    if (evitarPrimero != -1 && CONSEJOS_TOTAL > 1 && g_ConsejosOrden[playerid][0] == evitarPrimero)
    {
        new tmp = g_ConsejosOrden[playerid][0];
        g_ConsejosOrden[playerid][0] = g_ConsejosOrden[playerid][1];
        g_ConsejosOrden[playerid][1] = tmp;
    }

    g_ConsejosPosicion[playerid] = 0;
    return 1;
}

stock Consejos_Enviar(playerid)
{
    if (g_ConsejosPosicion[playerid] >= CONSEJOS_TOTAL)
    {
        Consejos_Barajar(playerid, g_ConsejosUltimo[playerid]);
    }

    new indice = g_ConsejosOrden[playerid][g_ConsejosPosicion[playerid]];
    g_ConsejosPosicion[playerid]++;
    g_ConsejosUltimo[playerid] = indice;

    new linea[200];
    format(linea, sizeof linea, "{8BC4E8}(Gamerol Info) {FFFFFF}%s", g_Consejos[indice]);
    SendClientMessage(playerid, COLOR_BLANCO, linea);
    return 1;
}

// Llamado desde TicMinuto (99_callbacks.inc), igual que Payday_Minuto y
// Ganancias_Minuto -- mismo bucle, misma guarda de AFK, sin timer propio.
stock Consejos_Minuto(playerid)
{
    if (!Jugador[playerid][jLogueado]) return 0;
    if (Jugador[playerid][jAFK]) return 0;

    g_ConsejosMinutos[playerid]++;
    if (g_ConsejosMinutos[playerid] < CONSEJOS_INTERVALO_MINUTOS) return 0;

    // Prioridad para la pista del Historico si coincide en el mismo
    // minuto (o el de al lado): no se resetea el contador, se reintenta
    // el minuto que viene -- para entonces la pista ya salio y se
    // reprogramo 1h despues, fuera de la ventana de choque.
    if (g_FedHistEstado == FEDHIST_BUSCANDO)
    {
        new restante = g_FedHistLeyendaSiguienteAt - gettime();
        if (restante > -60 && restante < 60) return 0;
    }

    g_ConsejosMinutos[playerid] = 0;
    Consejos_Enviar(playerid);
    return 1;
}
