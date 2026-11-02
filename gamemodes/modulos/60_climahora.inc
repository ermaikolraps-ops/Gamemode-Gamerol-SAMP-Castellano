/* =====================================================================
 *  60_CLIMAHORA.INC -- HORA REAL DE ESPAÑA (libreria) + CLIMA DINAMICO
 *  ---------------------------------------------------------------------
 *  10/09/2026: primera version, hora calculada y aplicada a mano
 *  (SetWorldTime/SetPlayerTime propios cada minuto).
 *  11/09/2026: Vlad pidio quitar esa parte -- sync lento al entrar y
 *  algun caso de dia/noche desincronizado -- y sustituirla por la
 *  libreria de GitHub rsetiawan7/samp-realtime-clock (vendorizada tal
 *  cual, sin modificar, en qawno/include/realtime-clock.inc), que ya
 *  trae sync instantaneo por jugador, OnWorldTimeUpdate y freeze por
 *  jugador para interiores. Este archivo AHORA SOLO calcula que hora
 *  de Madrid es (Sakamoto + regla UE de DST) y se la entrega a la
 *  libreria; la libreria es quien de verdad pone y mantiene la hora
 *  de cada jugador. El CLIMA sigue siendo 100% propio -- la libreria
 *  no toca clima, Vlad no pidio cambiar eso.
 *
 *  ZONA HORARIA: el VPS esta configurado en UTC (comprobado, no en
 *  Europe/Madrid) y no hay ningun plugin de zona horaria instalado.
 *  Cambiar la zona horaria del sistema operativo quedo descartado
 *  a proposito -- afectaria a TODO lo que corre en la maquina (MySQL,
 *  el panel web, el bot de WhatsApp, las tareas programadas), no solo
 *  al juego. En su lugar, el desfase CET/CEST se calcula aqui mismo
 *  aplicando la regla oficial de la Union Europea (cambio de hora el
 *  ultimo domingo de marzo y el ultimo domingo de octubre, a la 01:00
 *  UTC) sobre gettime()/getdate(), que en este VPS ya devuelven UTC
 *  real al estar el sistema en esa zona. Esto NO es un GMT+1 fijo: se
 *  recalcula solo, para siempre, sin mantenimiento.
 *
 *  RELOJ DEL MUNDO: la libreria trae su propio timer interno (1 minuto
 *  de juego = 1 minuto real, "RealTime_SetInterval(60000)", sin ciclo
 *  acelerado) que hace avanzar y sincroniza a todos los jugadores no
 *  congelados sola. Nosotros solo la RE-ANCLAMOS a la hora real de
 *  Madrid una vez por hora (ClimaHora_Tick, TicHora en 99_callbacks.inc)
 *  para que nunca se desvie y para que el cambio de DST (marzo/octubre)
 *  se note solo, sin reiniciar el servidor.
 *
 *  CLIMA: maquina de estados con 7 categorias (ver defines en
 *  01_configuracion.inc). Decision cada ~2h reales (horas PARES de
 *  Madrid), reutilizando el timer TicHora ya existente -- no se crea
 *  ningun timer nuevo. La niebla es un caso especial: nunca aparece
 *  como transicion normal de la tabla, solo se inyecta con una
 *  probabilidad baja durante la ventana de madrugada/amanecer, y
 *  siempre evoluciona hacia fuera en la siguiente decision.
 *
 *  WEATHER IDS: el array g_ClimaWeatherId[] de mas abajo fue probado y
 *  CONFIRMADO VISUALMENTE por Vlad in-game con /climatest el 10/09/2026
 *  (11 despejado, 6 soleado, 14 parcial nubes, 12 nublado, 8 lluvia,
 *  19 tormenta, 9 niebla) -- ya no son valores provisionales.
 *
 *  PERSISTENCIA: reutiliza la tabla generica `servidor_config`
 *  (clave/valor) que YA existe (Config_Leer/Config_Escribir,
 *  05_basedatos.inc) -- la misma que guarda la posicion del banco.
 *  Se guardan dos enteros pequeños (estado actual, ultima hora par
 *  decidida), sin problemas de precision de Float al no ser timestamps.
 *  La hora no se persiste -- se deriva del reloj real en cada arranque.
 *
 *  NO TOCA: Federico, vehiculos, economia, hambre/sed, trabajos,
 *  facciones, casas, negocios, web ni recompensas -- es una capa
 *  ambiental global aislada, sin dependencias de ningun otro sistema.
 * ===================================================================== */

static g_ClimaEstadoActual = CLIMA_DESPEJADO;
static g_ClimaUltimaHoraDecidida = -1;

// 11/09/2026: precalculo para El Teletipo -- ver ClimaHora_TickMinuto.
static g_ClimaProximoEstado = -1;          // -1 = aun no se ha precalculado nada
static g_ClimaProximaHoraParaAplicar = -1; // hora PAR de Madrid a la que corresponde g_ClimaProximoEstado

// Confirmados visualmente por Vlad in-game con /climatest el 10/09/2026
// -- ya no son provisionales.
new const g_ClimaWeatherId[CLIMA_TOTAL_ESTADOS] = {
    11,  // CLIMA_DESPEJADO
    6,   // CLIMA_SOLEADO
    14,  // CLIMA_PARCIAL_NUBES
    12,  // CLIMA_NUBLADO
    8,   // CLIMA_LLUVIA
    19,  // CLIMA_TORMENTA
    9    // CLIMA_NIEBLA
};

new const g_ClimaNombre[CLIMA_TOTAL_ESTADOS][16] = {
    "DESPEJADO", "SOLEADO", "PARCIAL NUBES", "NUBLADO", "LLUVIA", "TORMENTA", "NIEBLA"
};

// Transiciones normales (la niebla se resuelve aparte, ver
// ClimaHora_Decidir). Cada fila: hasta 3 pares {estado, peso relativo}.
// -1 = hueco vacio. Los pesos NO son porcentajes absolutos, se
// normalizan en ClimaHora_ElegirPonderado -- lo unico que importa es
// la proporcion entre ellos.
new const g_ClimaTransiciones[CLIMA_TOTAL_ESTADOS][3][2] = {
    /* DESPEJADO     */ {{CLIMA_DESPEJADO, 50}, {CLIMA_SOLEADO, 30}, {CLIMA_PARCIAL_NUBES, 20}},
    /* SOLEADO       */ {{CLIMA_SOLEADO, 50}, {CLIMA_DESPEJADO, 25}, {CLIMA_PARCIAL_NUBES, 25}},
    /* PARCIAL_NUBES */ {{CLIMA_PARCIAL_NUBES, 45}, {CLIMA_SOLEADO, 20}, {CLIMA_NUBLADO, 35}},
    /* NUBLADO       */ {{CLIMA_NUBLADO, 45}, {CLIMA_PARCIAL_NUBES, 25}, {CLIMA_LLUVIA, 30}},
    /* LLUVIA        */ {{CLIMA_LLUVIA, 40}, {CLIMA_NUBLADO, 35}, {CLIMA_TORMENTA, 25}},
    /* TORMENTA      */ {{CLIMA_LLUVIA, 80}, {CLIMA_TORMENTA, 20}, {-1, 0}},
    /* NIEBLA        */ {{-1, 0}, {-1, 0}, {-1, 0}}  // no se usa -- salida gestionada aparte en ClimaHora_Decidir
};

// ---------------------------------------------------------------------
//  FECHA/HORA UTC Y DESFASE DE MADRID (regla oficial UE, sin libreria
//  externa ni plugin -- Sakamoto para el dia de la semana, estandar y
//  ya probado, no es una aproximacion casera).
// ---------------------------------------------------------------------
stock ClimaHora_FechaHoraUTC(&anyo, &mes, &dia, &hora, &minuto, &segundo)
{
    getdate(anyo, mes, dia);
    gettime(hora, minuto, segundo);
    return 1;
}

// Algoritmo de Sakamoto -- devuelve 0 = domingo .. 6 = sabado.
stock ClimaHora_DiaSemana(anyo, mes, dia)
{
    new const t[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    new y = anyo;
    if (mes < 3) y -= 1;
    return (y + y/4 - y/100 + y/400 + t[mes-1] + dia) % 7;
}

stock ClimaHora_UltimoDomingo(anyo, mes, ultimoDia)
{
    return ultimoDia - ClimaHora_DiaSemana(anyo, mes, ultimoDia);
}

// Devuelve 2 (CEST, verano) o 1 (CET, invierno) para el instante UTC
// actual, aplicando el cambio de hora oficial de la UE: ultimo domingo
// de marzo y ultimo domingo de octubre, a la 01:00 UTC.
stock ClimaHora_DesfaseMadrid()
{
    new anyo, mes, dia, hora, minuto, segundo;
    ClimaHora_FechaHoraUTC(anyo, mes, dia, hora, minuto, segundo);

    if (mes > 3 && mes < 10) return 2;
    if (mes < 3 || mes > 10) return 1;

    if (mes == 3)
    {
        new ultDom = ClimaHora_UltimoDomingo(anyo, 3, 31);
        if (dia > ultDom || (dia == ultDom && hora >= 1)) return 2;
        return 1;
    }

    // mes == 10
    new ultDomOct = ClimaHora_UltimoDomingo(anyo, 10, 31);
    if (dia < ultDomOct || (dia == ultDomOct && hora < 1)) return 2;
    return 1;
}

stock ClimaHora_HoraMadridActual(&horaMadrid, &minutoMadrid)
{
    new anyo, mes, dia, horaUTC, minuto, segundo;
    ClimaHora_FechaHoraUTC(anyo, mes, dia, horaUTC, minuto, segundo);
    horaMadrid = (horaUTC + ClimaHora_DesfaseMadrid()) % 24;
    minutoMadrid = minuto;
    return 1;
}

// ---------------------------------------------------------------------
//  SINCRONIZACION POR JUGADOR -- la HORA ahora la sincroniza la propia
//  libreria (RealTime_SyncPlayerWorldTime, instantaneo, sin esperar al
//  siguiente tick). El CLIMA sigue siendo cosa nuestra: SetPlayerWeather
//  crea un override PROPIO de ese jugador que deja de seguir a
//  SetWeather() en cuanto se llama una vez, hasta que se le vuelve a
//  llamar -- por eso ClimaHora_SincronizarClimaJugador se repite cada
//  minuto para todos los jugadores activos (TicMinuto, 99_callbacks.inc)
//  como defensa, igual que antes del 10/09/2026 se descubrio que pasaba
//  con la hora (bug real: Vlad, "es de dia" con Madrid en mitad de la
//  noche -- pantallas de login/creacion de personaje en 06_login.inc
//  dejaban el clima de cada jugador pegado para siempre).
//
//  ClimaHora_SincronizarJugador (hora + clima juntos) se llama UNA vez
//  al entrar de verdad al mundo (Login_TerminarSesion, 06_login.inc).
// ---------------------------------------------------------------------
stock ClimaHora_SincronizarClimaJugador(playerid)
{
    SetPlayerWeather(playerid, g_ClimaWeatherId[g_ClimaEstadoActual]);
    return 1;
}

stock ClimaHora_SincronizarJugador(playerid)
{
    RealTime_SyncPlayerWorldTime(playerid);
    ClimaHora_SincronizarClimaJugador(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  CLIMA -- eleccion ponderada y maquina de estados
// ---------------------------------------------------------------------
stock ClimaHora_ElegirPonderado(const candidatos[3][2])
{
    new total = 0;
    for (new i = 0; i < 3; i++) { if (candidatos[i][0] != -1) total += candidatos[i][1]; }
    if (total <= 0) return CLIMA_DESPEJADO;

    new tirada = random(total);
    new acumulado = 0;
    for (new i = 0; i < 3; i++)
    {
        if (candidatos[i][0] == -1) continue;
        acumulado += candidatos[i][1];
        if (tirada < acumulado) return candidatos[i][0];
    }
    return candidatos[0][0];
}

stock ClimaHora_Decidir(estadoActual, horaMadrid)
{
    // La niebla nunca se queda atascada -- siempre evoluciona hacia
    // fuera en la siguiente decision, sin importar la hora.
    if (estadoActual == CLIMA_NIEBLA)
    {
        new const salida[3][2] = {{CLIMA_NUBLADO, 40}, {CLIMA_PARCIAL_NUBES, 35}, {CLIMA_DESPEJADO, 25}};
        return ClimaHora_ElegirPonderado(salida);
    }

    // Entrada de niebla: caso especial, solo de madrugada/amanecer,
    // solo desde cielos tranquilos, y poco frecuente (peticion expresa:
    // "no quiero niebla apareciendo aleatoriamente continuamente").
    if (horaMadrid >= CLIMA_HORA_NIEBLA_INICIO && horaMadrid <= CLIMA_HORA_NIEBLA_FIN &&
        (estadoActual == CLIMA_DESPEJADO || estadoActual == CLIMA_PARCIAL_NUBES || estadoActual == CLIMA_NUBLADO))
    {
        if (random(100) < CLIMA_PROB_NIEBLA) return CLIMA_NIEBLA;
    }

    return ClimaHora_ElegirPonderado(g_ClimaTransiciones[estadoActual]);
}

// ---------------------------------------------------------------------
//  PRECALCULO -- llamado cada minuto (TicMinuto, 99_callbacks.inc) para
//  saber el PROXIMO estado 5 minutos antes de que se aplique de verdad.
//
//  ClimaHora_Decidir() usa random() y NO es repetible: si algo externo
//  (El Teletipo) la llamara por su cuenta para "adivinar" el proximo
//  estado, y luego ClimaHora_Tick() volviese a tirar el dado al aplicar,
//  lo anunciado y lo aplicado podrian no coincidir. Por eso decidir y
//  aplicar quedan separados: se decide UNA sola vez a la xx:55 de la
//  hora impar anterior, se guarda aqui, y ClimaHora_Tick() en la hora
//  par solo APLICA ese valor ya decidido (con un fallback si por lo que
//  sea no hubo tiempo de precalcularlo, ej. el servidor acaba de
//  arrancar dentro de la ventana de los 5 minutos).
// ---------------------------------------------------------------------
stock ClimaHora_TickMinuto()
{
    new horaMadrid, minutoMadrid;
    ClimaHora_HoraMadridActual(horaMadrid, minutoMadrid);

    if (minutoMadrid != 55 || horaMadrid % 2 == 0) return 1;

    new horaObjetivo = (horaMadrid + 1) % 24;
    if (horaObjetivo == g_ClimaProximaHoraParaAplicar) return 1; // ya precalculado

    g_ClimaProximoEstado = ClimaHora_Decidir(g_ClimaEstadoActual, horaObjetivo);
    g_ClimaProximaHoraParaAplicar = horaObjetivo;
    return 1;
}

// Getters de solo lectura para otros modulos (ej. El Teletipo mas
// adelante) -- nunca escriben, solo consultan lo que ya se decidio.
stock ClimaHora_ObtenerProximoEstado()
{
    return g_ClimaProximoEstado;
}

stock ClimaHora_ObtenerProximaHoraCambio()
{
    return g_ClimaProximaHoraParaAplicar;
}

// ---------------------------------------------------------------------
//  TICK DE HORA + CLIMA -- llamado cada hora real (TicHora).
//
//  Hora: re-ancla la libreria a la hora real de Madrid recien calculada
//  (no la del contador interno de la libreria) -- corrige cualquier
//  desvio acumulado por el timer de 60s y hace que el cambio de DST
//  (marzo/octubre) se note sin reiniciar el servidor. restart_timer=false
//  para no cortar el avance minuto a minuto que la libreria ya lleva.
//
//  Clima: solo actua en horas PARES de Madrid y solo una vez por hora
//  (aunque TicHora fallase y se llamase dos veces seguidas por lo que
//  sea, g_ClimaUltimaHoraDecidida lo evita). Si ClimaHora_TickMinuto ya
//  precalculo el estado para esta hora exacta, se aplica ese mismo valor
//  en vez de volver a tirar el dado -- garantiza que lo anunciado 5
//  minutos antes coincida siempre con lo que de verdad se aplica.
// ---------------------------------------------------------------------
stock ClimaHora_Tick()
{
    new horaMadrid, minutoMadrid;
    ClimaHora_HoraMadridActual(horaMadrid, minutoMadrid);

    RealTime_SetWorldTime(horaMadrid, minutoMadrid, false, false);

    if (horaMadrid % 2 != 0) return 1;
    if (horaMadrid == g_ClimaUltimaHoraDecidida) return 1;

    new anterior = g_ClimaEstadoActual;
    new nuevo;

    if (g_ClimaProximaHoraParaAplicar == horaMadrid)
    {
        nuevo = g_ClimaProximoEstado;
    }
    else
    {
        // Fallback: no hubo tiempo de precalcular (ej. el servidor
        // acaba de arrancar en mitad de la ventana de los 5 minutos).
        nuevo = ClimaHora_Decidir(anterior, horaMadrid);
    }

    g_ClimaProximoEstado = -1;
    g_ClimaProximaHoraParaAplicar = -1;

    g_ClimaEstadoActual = nuevo;
    g_ClimaUltimaHoraDecidida = horaMadrid;

    SetWeather(g_ClimaWeatherId[nuevo]);
    Config_Escribir("clima_estado", float(nuevo));
    Config_Escribir("clima_ultima_hora", float(horaMadrid));

    printf("[CLIMA] %02d:%02d %s -> %s", horaMadrid, minutoMadrid, g_ClimaNombre[anterior], g_ClimaNombre[nuevo]);
    return 1;
}

// ---------------------------------------------------------------------
//  ARRANQUE -- llamado UNA vez desde OnGameModeInit, sustituye a
//  SetWeather(2)/SetWorldTime(12). Recupera el clima de la ultima
//  sesion (servidor_config) para no volver siempre a sol tras reiniciar;
//  la hora nunca necesita persistirse porque se deriva del reloj real.
//
//  La libreria arranca en modo "manual" (no RealTime_Sync, que usaria
//  gettime() crudo = UTC del VPS, no Madrid): se le fija el intervalo a
//  60000ms (1 minuto de juego = 1 minuto real, igual que antes, sin
//  ciclo acelerado -- el valor por defecto de la libreria es 1000ms y
//  la dejaria 60 veces mas rapida si no se cambia) y se le entrega la
//  hora de Madrid ya calculada como punto de partida.
// ---------------------------------------------------------------------
stock ClimaHora_Inicializar()
{
    g_ClimaEstadoActual = floatround(Config_Leer("clima_estado", float(CLIMA_DESPEJADO)));
    if (g_ClimaEstadoActual < 0 || g_ClimaEstadoActual >= CLIMA_TOTAL_ESTADOS) g_ClimaEstadoActual = CLIMA_DESPEJADO;
    g_ClimaUltimaHoraDecidida = floatround(Config_Leer("clima_ultima_hora", -1.0));

    SetWeather(g_ClimaWeatherId[g_ClimaEstadoActual]);

    new horaMadrid, minutoMadrid;
    ClimaHora_HoraMadridActual(horaMadrid, minutoMadrid);
    RealTime_SetInterval(60000, false);
    RealTime_SetWorldTime(horaMadrid, minutoMadrid, true, false);

    printf("  [CLIMA] Inicializado: %s | Hora Madrid: %02d:%02d (desfase UTC+%d)",
        g_ClimaNombre[g_ClimaEstadoActual], horaMadrid, minutoMadrid, ClimaHora_DesfaseMadrid());
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN -- diagnostico y pruebas. Nivel ADMIN_ADMIN (no toca economia
//  ni rol, es solo ambiental) -- ajustar si Vlad prefiere otro nivel.
// ---------------------------------------------------------------------
stock ClimaHora_EstadoPorNombre(const nombre[])
{
    if (!strcmp(nombre, "despejado", true)) return CLIMA_DESPEJADO;
    if (!strcmp(nombre, "soleado", true)) return CLIMA_SOLEADO;
    if (!strcmp(nombre, "parcial", true)) return CLIMA_PARCIAL_NUBES;
    if (!strcmp(nombre, "nublado", true)) return CLIMA_NUBLADO;
    if (!strcmp(nombre, "lluvia", true)) return CLIMA_LLUVIA;
    if (!strcmp(nombre, "tormenta", true)) return CLIMA_TORMENTA;
    if (!strcmp(nombre, "niebla", true)) return CLIMA_NIEBLA;
    return -1;
}

CMD:climaestado(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new horaMadrid, minutoMadrid;
    ClimaHora_HoraMadridActual(horaMadrid, minutoMadrid);

    format(g_Buffer, sizeof g_Buffer,
        "Clima: %s (weather id %d) | Hora real Madrid: %02d:%02d (UTC+%d) | Reloj libreria: %02d:%02d | Ultima decision clima: hora %d",
        g_ClimaNombre[g_ClimaEstadoActual], g_ClimaWeatherId[g_ClimaEstadoActual],
        horaMadrid, minutoMadrid, ClimaHora_DesfaseMadrid(),
        RealTime_GetHour(), RealTime_GetMinute(), g_ClimaUltimaHoraDecidida);
    Exito(playerid, g_Buffer);

    if (g_ClimaProximaHoraParaAplicar != -1)
    {
        format(g_Buffer, sizeof g_Buffer,
            "Proximo estado ya precalculado: %s, se aplicara a las %02d:00.",
            g_ClimaNombre[g_ClimaProximoEstado], g_ClimaProximaHoraParaAplicar);
    }
    else
    {
        format(g_Buffer, sizeof g_Buffer, "Proximo estado: aun no precalculado (se decide a la xx:55 anterior).");
    }
    Info(playerid, g_Buffer);
    return 1;
}

CMD:climatest(playerid, params[])
{
    new id;
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;
    if (sscanf(params, "d", id))
    {
        Error(playerid, "Uso: /climatest [weather id]. Solo lo ves tu, no afecta a nadie mas.");
        return 1;
    }

    SetPlayerWeather(playerid, id);
    format(g_Buffer, sizeof g_Buffer, "Probando weather id %d -- solo tu lo ves, el resto del servidor sigue igual.", id);
    Exito(playerid, g_Buffer);
    return 1;
}

CMD:climaforzar(playerid, params[])
{
    new nombreEstado[24];
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;
    if (sscanf(params, "s[24]", nombreEstado))
    {
        Error(playerid, "Uso: /climaforzar [despejado|soleado|parcial|nublado|lluvia|tormenta|niebla]");
        return 1;
    }

    new nuevo = ClimaHora_EstadoPorNombre(nombreEstado);
    if (nuevo == -1)
    {
        Error(playerid, "Estado no reconocido. Usa: despejado, soleado, parcial, nublado, lluvia, tormenta o niebla.");
        return 1;
    }

    g_ClimaEstadoActual = nuevo;
    SetWeather(g_ClimaWeatherId[nuevo]);
    Config_Escribir("clima_estado", float(nuevo));

    // Se marca la hora actual como "ya decidida" -- si no, y la hora
    // actual ya es par, TicHora podia disparar segundos/minutos despues
    // (su temporizador de 1h no esta alineado al reloj de pared, cuenta
    // desde que arranco el proceso) y pisar el forzado casi al momento,
    // en vez de aguantar hasta la SIGUIENTE hora par real. Bug real
    // encontrado el 10/09/2026 probando /climaforzar dos veces seguidas.
    new horaMadridActual, minutoMadridActual;
    ClimaHora_HoraMadridActual(horaMadridActual, minutoMadridActual);
    g_ClimaUltimaHoraDecidida = horaMadridActual;
    Config_Escribir("clima_ultima_hora", float(horaMadridActual));

    format(g_Buffer, sizeof g_Buffer,
        "Clima forzado a %s para todo el servidor. Es temporal: el ciclo automatico retoma en la siguiente hora par.",
        g_ClimaNombre[nuevo]);
    Exito(playerid, g_Buffer);
    printf("[CLIMA] Forzado manualmente a %s por admin (id %d)", g_ClimaNombre[nuevo], playerid);
    return 1;
}
