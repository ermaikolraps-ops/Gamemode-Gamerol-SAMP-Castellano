/* =====================================================================
 *  58_ascensos.inc
 *  ---------------------------------------------------------------------
 *  SERVER IA + PROGRESION DE CONFIANZA + ENTRADA AUTOMATICA A POLICIA +
 *  STAFF PUNITIVO V1 + SUCESION POLICIAL ACTIVA. Encargo de Vlad,
 *  08/09/2026 (PASO C y la sucesion aprobados el mismo dia). Cubre:
 *
 *    - "[Server IA]": voz institucional del servidor, mensajes globales
 *      rotativos cada 15 min (server-side, independiente de AFK).
 *    - Progresion de confianza por horas REALES activas (reutiliza
 *      jMinutosTotales, que YA excluye AFK -- ver Payday_Minuto en
 *      17_payday.inc -- no hizo falta crear un contador nuevo).
 *    - Ofertas /si /no de rango de staff (titulo), de entrada en la
 *      Policia Nacional (facción 1, rango real "Aspirante" de
 *      g_RangosFaccion, reutilizando Faccion_Dar tal cual), y de asumir
 *      el liderazgo policial cuando esta VACANTE DE VERDAD (ver abajo).
 *    - /ascensos: panel de progreso para cualquier jugador.
 *    - /expulsarcomunidad: UNICO comando punitivo del staff autonomo
 *      (PASO C, aprobado). Rango minimo Vigilante Comunitario (4),
 *      cooldown 10 min, maximo 5 acciones/hora, reutiliza el MISMO
 *      mecanismo de expulsion que /expulsar_jugador (ExpulsarRetardado
 *      + BD_RegistrarSancion + Kick nativo), pero como comando propio,
 *      NO como alias, con su propio log distinguible en admin_logs
 *      ("STAFFAUTO_KICK" en vez de "KICK", nivel admin real = 0).
 *
 *  SUCESION POLICIAL -- decision explicita de Vlad el 08/09/2026
 *  ("vacante de verdad"): cuando NINGUNA cuenta de la Policia Nacional
 *  tiene el flag faccion_lider puesto, el sistema SI actua -- ofrece el
 *  cargo (/si /no) al candidato mejor puntuado (formula de
 *  Ascensos_AlConsultarPolicia) que este conectado en ese momento,
 *  revisado cada ASCENSOS_LIDER_CHECK_MINUTOS minutos
 *  (Ascensos_ComprobarLiderVacanteMinuto). Guardarrail sin cambios:
 *  ninguna cuenta con jAdmin > 0 puede ser candidata (por eso
 *  Vlad_Jailovic, rango 10 sin flag, nunca recibe esta oferta).
 *
 *  LO QUE SIGUE SIN HACER ESTE ARCHIVO (decision YA cerrada por Vlad,
 *  distinta de la vacante de arriba -- ver la conversacion del mismo
 *  dia sobre la "regla de inactividad policial"):
 *    - NINGUN otro comando punitivo (mute/carcel/etc) para staff
 *      autonomo -- solo /expulsarcomunidad, tal como se aprobo.
 *    - NINGUNA retirada automatica de faccion_lider a un lider YA
 *      designado que lleve dias sin conectar. La deteccion de "Lider
 *      INACTIVO" en /ascensos sigue siendo SOLO VISUALIZACION -- Vlad
 *      fue explicito: los 10 dias sirven para detectar y mostrar, no
 *      para degradar ni traspasar nada solo. Si algun dia el lider
 *      designado se va o deja de cumplir, ESTE archivo no hace nada
 *      hasta que Vlad lo quite a mano (o con /liderfaccion) -- la
 *      sucesion activa de esta version solo cubre el caso "nadie tiene
 *      el flag en absoluto", no "el que lo tiene lleva mucho sin jugar".
 * ===================================================================== */

// ---------------------------------------------------------------------
//  CONSTANTES
// ---------------------------------------------------------------------
#define ASCENSOS_FACCION_POLICIA       1       // Ranura 1 de g_Facciones (12_facciones.inc)
#define ASCENSOS_OFERTA_SEGUNDOS       300     // 5 min para responder /si o /no
#define ASCENSOS_HITO_POLICIA_MINUTOS  600     // 10 horas activas

#define OFERTA_NINGUNA        0
#define OFERTA_STAFF          1
#define OFERTA_POLICIA        2
#define OFERTA_LIDER_POLICIA  3

#define ASCENSOS_BIT_POLICIA 0x40

// Sucesion policial activa (08/09/2026, "vacante de verdad" -- ver
// cabecera). Cada cuantos minutos se revisa si la Policia esta vacante Y
// hay un candidato elegible conectado para ofrecerle el cargo.
#define ASCENSOS_LIDER_CHECK_MINUTOS 10

// Staff punitivo V1 (PASO C, aprobado 08/09/2026) -- ver cabecera.
#define ASCENSOS_RANGO_PUNITIVO_MINIMO          4       // Vigilante Comunitario
#define ASCENSOS_ELEGIBILIDAD_PUNITIVO_MINUTOS  600     // 10h activas -- mismo valor que el hito de Policia
                                                          // por coincidencia, no por dependencia (constante propia
                                                          // a proposito para que este claro que es la condicion de
                                                          // elegibilidad punitiva, no la de Policia)
#define ASCENSOS_COOLDOWN_PUNITIVO_SEGUNDOS     600     // 10 minutos entre usos, por cuenta staff-auto
#define ASCENSOS_MAX_ACCIONES_HORA              5       // Tope de acciones punitivas por ventana de 1h
#define ASCENSOS_SANCIONES_DIAS                 14      // Ventana de "sancion reciente" que bloquea el uso

#define ASCENSOS_TOTAL_HITOS_STAFF 6
new const g_AscensosHitoMinutos[ASCENSOS_TOTAL_HITOS_STAFF] = {60, 180, 300, 600, 1200, 1800};
new const g_AscensosHitoRango[ASCENSOS_TOTAL_HITOS_STAFF]   = {1, 2, 3, 4, 5, 6};
new const g_AscensosHitoBit[ASCENSOS_TOTAL_HITOS_STAFF]     = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};

// Ranks 7-10 del boceto original (Coordinador/Administrador/Administrador
// Senior) NO se implementan todavia: chocarian de nombre con la escala
// admin real (16_administracion.inc: Soporte/Moderador/Administrador/...)
// y necesitarian criterios objetivos que Vlad aun no ha fijado (ver
// informe). Indice 0 = jStaffRango 0 (nadie).
new const g_AscensosNombreRango[ASCENSOS_TOTAL_HITOS_STAFF + 1][32] =
{
    "Ninguno",
    "Jugador Ayudante",
    "Ayudante",
    "Guia",
    "Vigilante Comunitario",
    "Vigilante Senior",
    "Guardian de Gamerol"
};

// La rotacion institucional "Server IA" (5 mensajes globales cada 15 min:
// g_AscensosMensajes[], ASCENSOS_TOTAL_MENSAJES, ASCENSOS_MENSAJE_MINUTOS,
// g_ServerIAPosicion/Proximo/Listo, ServerIA_MensajeGlobal(), ServerIA_Minuto())
// se quito entera el 10/09/2026 (peticion expresa de Vlad: "lineas
// informativas ingame"). ServerIA_Mensaje() (privada, mas abajo) SIGUE
// existiendo -- la usan las ofertas reales de ascenso/policia de este
// mismo archivo, que no son relleno informativo sino mensajes ligados a
// un evento real del jugador.

// ---------------------------------------------------------------------
//  SERVER IA -- mensajeria privada (ligada a eventos reales de ascenso)
// ---------------------------------------------------------------------
stock ServerIA_Mensaje(playerid, const texto[])
{
    new linea[200];
    format(linea, sizeof linea, "%s[Server IA]: {FFFFFF}%s", HEX_SERVERIA, texto);
    SendClientMessage(playerid, COLOR_BLANCO, linea);
    return 1;
}

// ---------------------------------------------------------------------
//  CICLO DE VIDA
// ---------------------------------------------------------------------
stock Ascensos_OnConectar(playerid)
{
    Jugador[playerid][jOfertaTipo]   = OFERTA_NINGUNA;
    Jugador[playerid][jOfertaExpira] = 0;
    Jugador[playerid][jOfertaDato]   = 0;
    Jugador[playerid][jAscensosReoferta] = false;

    Jugador[playerid][jStaffCooldown]     = 0;
    Jugador[playerid][jStaffHoraInicio]   = 0;
    Jugador[playerid][jStaffAccionesHora] = 0;
    return 1;
}

// Llamado desde TicMinuto (99_callbacks.inc), mismo bucle y misma guarda
// que Payday_Minuto/Ganancias_Minuto/Consejos_Minuto.
stock Ascensos_Minuto(playerid)
{
    if (!Jugador[playerid][jLogueado]) return 0;
    if (Jugador[playerid][jAFK]) return 0;

    // Ninguna cuenta con admin tradicional (Soporte..Fundador) es
    // candidata a staff autonomo ni a la entrada automatica a Policia --
    // mismo principio que la exclusion de sucesion: el sistema automatico
    // nunca actua sobre una cuenta que ya tiene poder real.
    if (Jugador[playerid][jAdmin] > 0) return 0;

    if (Jugador[playerid][jOfertaTipo] != OFERTA_NINGUNA && gettime() >= Jugador[playerid][jOfertaExpira])
    {
        Jugador[playerid][jOfertaTipo] = OFERTA_NINGUNA;
    }

    // Nunca se pisan dos ofertas -- importante porque el hito de Policia
    // (10h) coincide EXACTAMENTE con el 4o hito de staff. Se espera a que
    // la primera se resuelva (/si, /no o caduque) antes de generar otra.
    if (Jugador[playerid][jOfertaTipo] != OFERTA_NINGUNA) return 0;

    if (Ascensos_ComprobarHitosStaff(playerid)) return 1;
    Ascensos_ComprobarHitoPolicia(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  HITOS
// ---------------------------------------------------------------------
stock bool:Ascensos_ComprobarHitosStaff(playerid)
{
    for (new h = 0; h < ASCENSOS_TOTAL_HITOS_STAFF; h++)
    {
        if (Jugador[playerid][jStaffOfertas] & g_AscensosHitoBit[h]) continue;
        if (Jugador[playerid][jMinutosTotales] < g_AscensosHitoMinutos[h]) break;

        Jugador[playerid][jStaffOfertas] |= g_AscensosHitoBit[h];
        BD_GuardarCuenta(playerid);
        Ascensos_OfrecerStaff(playerid, g_AscensosHitoRango[h]);
        return true;
    }
    return false;
}

stock Ascensos_ComprobarHitoPolicia(playerid)
{
    if (Jugador[playerid][jStaffOfertas] & ASCENSOS_BIT_POLICIA) return 0;
    if (Jugador[playerid][jMinutosTotales] < ASCENSOS_HITO_POLICIA_MINUTOS) return 0;

    Jugador[playerid][jStaffOfertas] |= ASCENSOS_BIT_POLICIA;
    BD_GuardarCuenta(playerid);

    if (Jugador[playerid][jFaccion] != 0) return 0;   // ya esta en una faccion, no aplica
    Ascensos_OfrecerPolicia(playerid);
    return 1;
}

stock Ascensos_OfrecerStaff(playerid, rango)
{
    Jugador[playerid][jOfertaTipo]   = OFERTA_STAFF;
    Jugador[playerid][jOfertaExpira] = gettime() + ASCENSOS_OFERTA_SEGUNDOS;
    Jugador[playerid][jOfertaDato]   = rango;

    new horas = g_AscensosHitoMinutos[rango - 1] / 60;
    new texto[176];
    format(texto, sizeof texto,
        "Llevas %d hora%s con nosotros. Puedes convertirte en %s. Quieres aceptar? /si - /no",
        horas, (horas == 1) ? ("") : ("s"), g_AscensosNombreRango[rango]);
    ServerIA_Mensaje(playerid, texto);
    return 1;
}

stock Ascensos_OfrecerPolicia(playerid)
{
    Jugador[playerid][jOfertaTipo]   = OFERTA_POLICIA;
    Jugador[playerid][jOfertaExpira] = gettime() + ASCENSOS_OFERTA_SEGUNDOS;

    ServerIA_Mensaje(playerid,
        "Llevas 10 horas con nosotros. Quieres entrar en la Policia Nacional? /si - /no");
    return 1;
}

// Sucesion policial activa (08/09/2026, "vacante de verdad") -- ver
// Ascensos_ComprobarLiderVacanteMinuto, unico sitio que llama a esto.
stock Ascensos_OfrecerLiderPolicia(playerid)
{
    Jugador[playerid][jOfertaTipo]   = OFERTA_LIDER_POLICIA;
    Jugador[playerid][jOfertaExpira] = gettime() + ASCENSOS_OFERTA_SEGUNDOS;

    ServerIA_Mensaje(playerid,
        "La Policia Nacional no tiene lider. Quieres asumir el cargo? /si - /no");
    return 1;
}

// ---------------------------------------------------------------------
//  /SI /NO
// ---------------------------------------------------------------------
CMD:si(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (Jugador[playerid][jOfertaTipo] == OFERTA_NINGUNA || gettime() >= Jugador[playerid][jOfertaExpira])
    {
        Jugador[playerid][jOfertaTipo] = OFERTA_NINGUNA;
        Error(playerid, "No tienes ninguna oferta pendiente. Usa /ascensos para ver tu progreso.");
        return 1;
    }

    new tipo = Jugador[playerid][jOfertaTipo];
    Jugador[playerid][jOfertaTipo] = OFERTA_NINGUNA;

    if (tipo == OFERTA_STAFF)
    {
        new rango = Jugador[playerid][jOfertaDato];
        Jugador[playerid][jStaffRango] = rango;
        BD_GuardarCuenta(playerid);

        new texto[128];
        format(texto, sizeof texto, "Enhorabuena. Ahora eres %s. Sigue jugando para seguir subiendo.",
            g_AscensosNombreRango[rango]);
        ServerIA_Mensaje(playerid, texto);
    }
    else if (tipo == OFERTA_POLICIA)
    {
        if (Jugador[playerid][jFaccion] != 0)
        {
            Error(playerid, "Ya perteneces a una faccion, la oferta ya no es valida.");
            return 1;
        }

        Faccion_Dar(playerid, ASCENSOS_FACCION_POLICIA, 1);
        Jugador[playerid][jPoliciaIngreso] = gettime();
        BD_GuardarCuenta(playerid);

        ServerIA_Mensaje(playerid,
            "Bienvenido a la Policia Nacional. Tu rango: Aspirante. Usa /mifaccion para ver tu escalafon.");
    }
    else if (tipo == OFERTA_LIDER_POLICIA)
    {
        if (Jugador[playerid][jFaccion] != ASCENSOS_FACCION_POLICIA)
        {
            Error(playerid, "Ya no perteneces a la Policia Nacional, la oferta ya no es valida.");
            return 1;
        }

        // Revalidacion en vivo antes de conceder el mando -- pudo
        // aceptar otro candidato, o Vlad pudo nombrar lider a mano con
        // /liderfaccion, mientras esta oferta seguia esperando respuesta.
        mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
            "SELECT COUNT(*) AS total FROM cuentas WHERE faccion = %d AND faccion_lider = 1",
            ASCENSOS_FACCION_POLICIA);
        mysql_pquery(g_BD, g_Buffer, "Ascensos_AlAceptarLiderPolicia", "d", playerid);
    }
    return 1;
}

forward Ascensos_AlAceptarLiderPolicia(playerid);
public Ascensos_AlAceptarLiderPolicia(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new total;
    cache_get_value_name_int(0, "total", total);
    if (total > 0)
    {
        Error(playerid, "Alguien ya asumio el mando mientras tanto.");
        return 1;
    }
    if (Jugador[playerid][jFaccion] != ASCENSOS_FACCION_POLICIA)
    {
        Error(playerid, "Ya no perteneces a la Policia Nacional.");
        return 1;
    }

    Jugador[playerid][jFaccionLider] = true;
    BD_GuardarCuenta(playerid);

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);
    new texto[144];
    format(texto, sizeof texto, "%s asume el mando de la Policia Nacional.", nombre);
    Faccion_Mensaje(ASCENSOS_FACCION_POLICIA, COLOR_MARCA, texto);

    ServerIA_Mensaje(playerid, "Enhorabuena. Ahora eres el lider de la Policia Nacional.");
    return 1;
}

CMD:no(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (Jugador[playerid][jOfertaTipo] == OFERTA_NINGUNA)
    {
        Error(playerid, "No tienes ninguna oferta pendiente.");
        return 1;
    }

    Jugador[playerid][jOfertaTipo] = OFERTA_NINGUNA;
    Info(playerid, "De acuerdo. Puedes volver a consultarlo cuando quieras con /ascensos.");
    return 1;
}

// ---------------------------------------------------------------------
//  SUCESION POLICIAL -- SOLO CALCULO Y VISUALIZACION (ver cabecera: no
//  hay ningun timer que actue sobre jFaccionLider todavia).
//
//  Formula PROPUESTA (pendiente de aprobacion final, ver informe):
//    puntos = (rango * 100) + (horas activas totales)
//             + (dias de antiguedad policial, si hay ingreso registrado)
//             - (avisos * 50) - (dias sin conectar * 5)
//  Nunca considera candidata una cuenta con jAdmin > 0 (Vlad/fundador y
//  el resto de staff tradicional quedan fuera de la sucesion automatica
//  por completo, se gestionan a mano con /liderfaccion).
//
//  REGLA DE INACTIVIDAD DEL LIDER (decidida por Vlad, 08/09/2026): un
//  lider FORMALMENTE designado (faccion_lider=1) que lleve
//  ASCENSOS_LIDER_INACTIVO_DIAS dias completos o mas sin conectarse
//  (cuentas.ultima_conexion) se considera INACTIVO. Esto es PURA
//  DETECCION Y VISUALIZACION en /ascensos -- NINGUN timer toca
//  faccion_lider, faccion_rango, ni ejecuta ascenso/degradacion/traspaso
//  automatico por esto. Sigue sin asumirse nunca que rango 10 equivale a
//  lider formal (ver nombreRangoMaximo mas abajo, caso ya existente).
// ---------------------------------------------------------------------
#define ASCENSOS_MAX_CANDIDATOS 30
#define ASCENSOS_LIDER_INACTIVO_DIAS 10

// ---------------------------------------------------------------------
//  SUCESION ACTIVA -- "vacante de verdad" (decision de Vlad, 08/09/2026).
//  Revisa cada ASCENSOS_LIDER_CHECK_MINUTOS minutos si NINGUNA cuenta de
//  la Policia tiene faccion_lider puesto; si es asi, ofrece el cargo
//  (/si /no) al candidato mejor puntuado (misma formula que
//  Ascensos_AlConsultarPolicia, duplicada a proposito porque una vive en
//  el flujo de /ascensos -- por jugador -- y esta en un tick global) que
//  este CONECTADO en ese momento y sin otra oferta pendiente. NUNCA
//  ofrece el cargo a una cuenta con jAdmin > 0 -- mismo guardarrail de
//  siempre. Si el lider YA esta designado (aunque este inactivo), esta
//  funcion no hace nada -- la inactividad es solo visualizacion, ver
//  cabecera del archivo.
// ---------------------------------------------------------------------
static g_AscensosLiderProximoCheck = 0;

stock Ascensos_ComprobarLiderVacanteMinuto()
{
    new ahora = gettime();
    if (g_AscensosLiderProximoCheck == 0)
    {
        // Lazy-init: da tiempo a que el servidor termine de arrancar del
        // todo antes del primer chequeo, mismo patron que RoboExpress.
        g_AscensosLiderProximoCheck = ahora + (ASCENSOS_LIDER_CHECK_MINUTOS * 60);
        return 0;
    }
    if (ahora < g_AscensosLiderProximoCheck) return 0;
    g_AscensosLiderProximoCheck = ahora + (ASCENSOS_LIDER_CHECK_MINUTOS * 60);

    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "SELECT id, faccion_rango, faccion_lider, admin, minutos_totales, avisos, policia_ingreso, \
            TIMESTAMPDIFF(DAY, IFNULL(ultima_conexion, fecha_registro), NOW()) AS dias_inactivo \
         FROM cuentas WHERE faccion = %d",
        ASCENSOS_FACCION_POLICIA);
    mysql_pquery(g_BD, g_Buffer, "Ascensos_AlComprobarVacante", "");
    return 1;
}

forward Ascensos_AlComprobarVacante();
public Ascensos_AlComprobarVacante()
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0) return 0;   // Policia sin miembros -- nada que sucesionar

    new mejorId = -1, mejorPuntos = -999999;
    new ahora = gettime();

    for (new r = 0; r < filas; r++)
    {
        new lider;
        cache_get_value_name_int(r, "faccion_lider", lider);
        if (lider != 0) return 0;   // ya hay lider designado -- no vacante, nada que hacer

        new esAdmin;
        cache_get_value_name_int(r, "admin", esAdmin);
        if (esAdmin > 0) continue;   // guardarrail: jamas candidato automatico

        new id, rango, minutos, avisos, ingreso, diasInactivo;
        cache_get_value_name_int(r, "id", id);
        cache_get_value_name_int(r, "faccion_rango", rango);
        cache_get_value_name_int(r, "minutos_totales", minutos);
        cache_get_value_name_int(r, "avisos", avisos);
        cache_get_value_name_int(r, "policia_ingreso", ingreso);
        cache_get_value_name_int(r, "dias_inactivo", diasInactivo);

        new puntos = (rango * 100) + (minutos / 60) - (avisos * 50) - (diasInactivo * 5);
        if (ingreso > 0) puntos += (ahora - ingreso) / 86400;

        if (puntos > mejorPuntos) { mejorPuntos = puntos; mejorId = id; }
    }

    if (mejorId == -1) return 0;   // vacante, pero sin ningun candidato elegible ahora mismo

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jID] != mejorId) continue;
        if (Jugador[i][jOfertaTipo] != OFERTA_NINGUNA) return 0;   // ya tiene otra oferta activa, se reintenta en el siguiente ciclo
        Ascensos_OfrecerLiderPolicia(i);
        return 1;
    }
    return 0;   // el mejor candidato no esta conectado ahora mismo
}

stock Ascensos_PedirEstadoPolicia(playerid)
{
    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "SELECT id, nombre, faccion_rango, faccion_lider, admin, minutos_totales, avisos, policia_ingreso, \
            TIMESTAMPDIFF(DAY, IFNULL(ultima_conexion, fecha_registro), NOW()) AS dias_inactivo \
         FROM cuentas WHERE faccion = %d ORDER BY faccion_rango DESC",
        ASCENSOS_FACCION_POLICIA);
    mysql_pquery(g_BD, g_Buffer, "Ascensos_AlConsultarPolicia", "d", playerid);
    return 1;
}

forward Ascensos_AlConsultarPolicia(playerid);
public Ascensos_AlConsultarPolicia(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);
    if (filas > ASCENSOS_MAX_CANDIDATOS) filas = ASCENSOS_MAX_CANDIDATOS;

    new id[ASCENSOS_MAX_CANDIDATOS], nombre[ASCENSOS_MAX_CANDIDATOS][MAX_PLAYER_NAME + 1],
        rango[ASCENSOS_MAX_CANDIDATOS], lider[ASCENSOS_MAX_CANDIDATOS], esAdmin[ASCENSOS_MAX_CANDIDATOS],
        minutos[ASCENSOS_MAX_CANDIDATOS], avisos[ASCENSOS_MAX_CANDIDATOS], ingreso[ASCENSOS_MAX_CANDIDATOS],
        diasInactivo[ASCENSOS_MAX_CANDIDATOS], puntos[ASCENSOS_MAX_CANDIDATOS], orden[ASCENSOS_MAX_CANDIDATOS];

    new nombreLider[MAX_PLAYER_NAME + 1] = "";
    new nombreRangoMaximo[MAX_PLAYER_NAME + 1] = "";
    new diasInactivoLider = -1;   // -1 = no aplica (no hay lider formal)
    new ahora = gettime();

    for (new r = 0; r < filas; r++)
    {
        cache_get_value_name_int(r, "id", id[r]);
        cache_get_value_name(r, "nombre", nombre[r], MAX_PLAYER_NAME + 1);
        cache_get_value_name_int(r, "faccion_rango", rango[r]);
        cache_get_value_name_int(r, "faccion_lider", lider[r]);
        cache_get_value_name_int(r, "admin", esAdmin[r]);
        cache_get_value_name_int(r, "minutos_totales", minutos[r]);
        cache_get_value_name_int(r, "avisos", avisos[r]);
        cache_get_value_name_int(r, "policia_ingreso", ingreso[r]);
        cache_get_value_name_int(r, "dias_inactivo", diasInactivo[r]);

        if (lider[r] != 0)
        {
            format(nombreLider, sizeof nombreLider, "%s", nombre[r]);
            diasInactivoLider = diasInactivo[r];
        }
        if (rango[r] >= MAX_RANGOS && nombreRangoMaximo[0] == EOS) format(nombreRangoMaximo, sizeof nombreRangoMaximo, "%s", nombre[r]);

        puntos[r] = (rango[r] * 100) + (minutos[r] / 60) - (avisos[r] * 50) - (diasInactivo[r] * 5);
        if (ingreso[r] > 0) puntos[r] += (ahora - ingreso[r]) / 86400;

        orden[r] = r;
    }

    // Orden descendente por puntuacion (insercion simple, maximo 30 filas).
    for (new a = 1; a < filas; a++)
    {
        new clave = orden[a], claveP = puntos[clave];
        new b = a - 1;
        while (b >= 0 && puntos[orden[b]] < claveP)
        {
            orden[b + 1] = orden[b];
            b--;
        }
        orden[b + 1] = clave;
    }

    // Posicion del jugador entre CANDIDATOS validos (excluye jAdmin > 0).
    new totalCandidatos = 0, miId = Jugador[playerid][jID], miPosicion = -1;
    for (new a = 0; a < filas; a++)
    {
        new r = orden[a];
        if (esAdmin[r] > 0) continue;
        totalCandidatos++;
        if (id[r] == miId) miPosicion = totalCandidatos;
    }

    Ascensos_MostrarDialogo(playerid, nombreLider, nombreRangoMaximo, diasInactivoLider, miPosicion, totalCandidatos);
    return 1;
}

// ---------------------------------------------------------------------
//  /ASCENSOS
// ---------------------------------------------------------------------
CMD:ascensos(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    Ascensos_PedirEstadoPolicia(playerid);
    return 1;
}

stock Ascensos_MostrarDialogo(playerid, const nombreLider[], const nombreRangoMaximo[], diasInactivoLider, miPosicion, totalCandidatos)
{
    new cuerpo[1400];
    new horas = Jugador[playerid][jMinutosTotales] / 60;
    new minutosResto = Jugador[playerid][jMinutosTotales] % 60;

    format(cuerpo, sizeof cuerpo,
        "{8E7CFC}Tu progreso en Gamerol{FFFFFF}\n\nTiempo activo de confianza: {FFD24D}%d h %d min{FFFFFF} (online, sin AFK)\nRango actual: {FFD24D}%s{FFFFFF}\n",
        horas, minutosResto, g_AscensosNombreRango[Jugador[playerid][jStaffRango]]);

    new bool:hayReoferta = false;
    new siguienteHito = -1;
    for (new h = 0; h < ASCENSOS_TOTAL_HITOS_STAFF; h++)
    {
        if (Jugador[playerid][jStaffOfertas] & g_AscensosHitoBit[h]) continue;
        siguienteHito = h;
        break;
    }

    if (siguienteHito == -1)
    {
        strcat(cuerpo, "Has alcanzado el ultimo rango disponible por ahora.\n", sizeof cuerpo);
    }
    else
    {
        new faltan = g_AscensosHitoMinutos[siguienteHito] - Jugador[playerid][jMinutosTotales];
        if (faltan < 0)
        {
            faltan = 0;
            if (Jugador[playerid][jOfertaTipo] == OFERTA_NINGUNA) hayReoferta = true;
        }
        new extra[200];
        format(extra, sizeof extra,
            "Siguiente rango: {FFD24D}%s{FFFFFF} (a las %d horas activas -- te faltan %d min)\n",
            g_AscensosNombreRango[g_AscensosHitoRango[siguienteHito]],
            g_AscensosHitoMinutos[siguienteHito] / 60, faltan);
        strcat(cuerpo, extra, sizeof cuerpo);
    }

    if (Jugador[playerid][jOfertaTipo] != OFERTA_NINGUNA && gettime() < Jugador[playerid][jOfertaExpira])
    {
        strcat(cuerpo, "\n{53C168}Tienes una oferta esperando respuesta -- escribe /si o /no.{FFFFFF}\n", sizeof cuerpo);
        hayReoferta = false;   // ya tiene una activa, no se ofrece otra encima
    }

    strcat(cuerpo, "\n{8E7CFC}Policia Nacional{FFFFFF}\n", sizeof cuerpo);
    if (Jugador[playerid][jFaccion] == ASCENSOS_FACCION_POLICIA)
    {
        new rangoNombre[32];
        Faccion_NombreRango(ASCENSOS_FACCION_POLICIA, Jugador[playerid][jFaccionRango], rangoNombre, sizeof rangoNombre);
        new extra2[220];
        format(extra2, sizeof extra2, "Tu rango: {FFD24D}%s{FFFFFF}%s\n",
            rangoNombre, Jugador[playerid][jFaccionLider] ? (" (Lider)") : (""));
        strcat(cuerpo, extra2, sizeof cuerpo);

        if (miPosicion != -1)
        {
            new extra3[160];
            format(extra3, sizeof extra3, "Tu posicion en la lista de sucesion: {FFD24D}%d de %d{FFFFFF}\n", miPosicion, totalCandidatos);
            strcat(cuerpo, extra3, sizeof cuerpo);
        }
    }
    else if (Jugador[playerid][jFaccion] != 0)
    {
        strcat(cuerpo, "Perteneces a otra faccion.\n", sizeof cuerpo);
    }
    else if (Jugador[playerid][jMinutosTotales] >= ASCENSOS_HITO_POLICIA_MINUTOS)
    {
        strcat(cuerpo, "Disponible para ti ahora mismo.\n", sizeof cuerpo);
        if (Jugador[playerid][jOfertaTipo] == OFERTA_NINGUNA) hayReoferta = true;
    }
    else
    {
        new extra4[120];
        format(extra4, sizeof extra4, "Disponible a partir de las 10 horas activas (te faltan %d min).\n",
            ASCENSOS_HITO_POLICIA_MINUTOS - Jugador[playerid][jMinutosTotales]);
        strcat(cuerpo, extra4, sizeof cuerpo);
    }

    new extraLider[200];
    if (nombreLider[0] != EOS)
    {
        // Lider FORMALMENTE designado -- distingue activo/inactivo segun
        // ASCENSOS_LIDER_INACTIVO_DIAS. Solo deteccion/visualizacion, ver
        // cabecera de la seccion de sucesion: nada se modifica por esto.
        if (diasInactivoLider >= ASCENSOS_LIDER_INACTIVO_DIAS)
        {
            format(extraLider, sizeof extraLider,
                "Lider INACTIVO: {FFD24D}%s{FFFFFF} ({E05A5A}%d dias sin conectar{FFFFFF})\n",
                nombreLider, diasInactivoLider);
        }
        else
        {
            format(extraLider, sizeof extraLider, "Lider activo: {FFD24D}%s{FFFFFF}\n", nombreLider);
        }
    }
    else if (nombreRangoMaximo[0] != EOS)
    {
        format(extraLider, sizeof extraLider, "Sin lider designado formalmente (rango maximo: {FFD24D}%s{FFFFFF})\n", nombreRangoMaximo);
    }
    else
    {
        format(extraLider, sizeof extraLider, "Vacante.\n");
    }
    strcat(cuerpo, extraLider, sizeof cuerpo);

    // Se guarda si ESTE dialogo concreto tiene de verdad el boton "Pedir
    // info", porque SA-MP/open.mp manda response=1 tanto al pulsar ese
    // boton COMO al pulsar el unico boton "Cerrar" cuando el segundo
    // boton va vacio -- sin esto, cerrar el dialogo sin reoferta se leia
    // como si se hubiera pedido una oferta nueva (bug real reportado por
    // Vlad, 12/09/2026: /ascensos ofrecia entrar a Policia Nacional al
    // pulsar "Cerrar").
    Jugador[playerid][jAscensosReoferta] = hayReoferta;

    if (hayReoferta)
    {
        ShowPlayerDialog(playerid, DIALOGO_ASCENSOS, DIALOG_STYLE_MSGBOX, "/ascensos", cuerpo, "Pedir info", "Cerrar");
    }
    else
    {
        ShowPlayerDialog(playerid, DIALOGO_ASCENSOS, DIALOG_STYLE_MSGBOX, "/ascensos", cuerpo, "Cerrar", "");
    }
    return 1;
}

// El boton "Pedir info" re-arma la oferta correspondiente (staff si el
// siguiente hito ya esta alcanzado, si no, policia) -- asi /no nunca es
// definitivo (peticion expresa de Vlad).
stock Ascensos_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused listitem, inputtext
    if (dialogid != DIALOGO_ASCENSOS) return 0;
    if (!response) return 1;
    if (!Jugador[playerid][jAscensosReoferta]) return 1;   // solo "Cerrar" (un boton) -- no habia "Pedir info" que pulsar
    if (Jugador[playerid][jOfertaTipo] != OFERTA_NINGUNA) return 1;   // ya tiene una activa

    for (new h = 0; h < ASCENSOS_TOTAL_HITOS_STAFF; h++)
    {
        if (Jugador[playerid][jStaffOfertas] & g_AscensosHitoBit[h]) continue;
        if (Jugador[playerid][jMinutosTotales] < g_AscensosHitoMinutos[h]) break;
        Ascensos_OfrecerStaff(playerid, g_AscensosHitoRango[h]);
        return 1;
    }

    if (!(Jugador[playerid][jStaffOfertas] & ASCENSOS_BIT_POLICIA)
        || (Jugador[playerid][jFaccion] == 0 && Jugador[playerid][jMinutosTotales] >= ASCENSOS_HITO_POLICIA_MINUTOS))
    {
        if (Jugador[playerid][jFaccion] == 0) Ascensos_OfrecerPolicia(playerid);
    }
    return 1;
}

// =======================================================================
//  STAFF PUNITIVO V1 -- /expulsarcomunidad (PASO C, aprobado 08/09/2026)
// =======================================================================
//
//  Condiciones de elegibilidad para USAR el comando (no solo para
//  obtener el rango -- se revalidan TODAS aqui, en el momento de
//  ejecutar, nunca solo al conceder el titulo):
//    1) jStaffRango >= ASCENSOS_RANGO_PUNITIVO_MINIMO (4, Vigilante Comunitario)
//    2) jMinutosTotales >= ASCENSOS_ELEGIBILIDAD_PUNITIVO_MINUTOS (10h) --
//       en la practica SIEMPRE cierto si ya tiene el rango 4+, porque
//       jMinutosTotales nunca baja; se deja la comprobacion explicita de
//       todos modos porque Vlad pidio revalidar TODO, no confiar en que
//       el rango ya lo garantiza.
//    3) !jBaneado (en la practica tambien casi siempre cierto para un
//       jugador conectado, ver nota 2 -- mismo motivo, se deja explicita).
//    4) jAvisos < 3 (SI puede cambiar durante la sesion: puede recibir un
//       aviso nuevo despues de haber alcanzado el rango).
//    5) Sin sancion "relevante" en los ultimos ASCENSOS_SANCIONES_DIAS
//       dias -- ver interpretacion exacta mas abajo, en
//       Ascensos_ComprobarSancionesRecientes. Esta es la UNICA condicion
//       que necesita una consulta a la base de datos (las otras 4 son
//       instantaneas, con el estado ya cargado en memoria).
//
//  INTERPRETACION DE "SANCION RELEVANTE" (pedido explicito de Vlad:
//  documentar exactamente que comprobacion se aplico):
//    La tabla `sanciones` (05_basedatos.inc: BD_RegistrarSancion) usa un
//    catalogo real de 5 tipos (16_administracion.inc:27-31):
//      SANCION_AVISO=1, SANCION_KICK=2, SANCION_MUTE=3, SANCION_CARCEL=4,
//      SANCION_BAN=5.
//    Se EXCLUYE SANCION_AVISO de esta comprobacion a proposito: los
//    avisos formales YA se comprueban aparte via jAvisos < 3 (condicion
//    4). Contarlos tambien aqui penalizaria dos veces la misma senal.
//    Se CUENTAN KICK/MUTE/CARCEL/BAN de los ultimos 14 dias -- incluido
//    BAN aunque exista tambien la comprobacion de jBaneado=0: un BAN que
//    ya caduco (baneado ya vuelve a 0) sigue siendo una sancion GRAVE
//    reciente y no deberia ignorarse solo porque el castigo ya termino.
// =======================================================================

CMD:expulsarcomunidad(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (!Ascensos_EsElegiblePunitivoSesion(playerid))
    {
        Error(playerid, "No cumples los requisitos para usar este comando ahora mismo. Usa /ascensos para ver tu estado.");
        return 1;
    }

    new ahora = gettime();
    if (Jugador[playerid][jStaffCooldown] > ahora)
    {
        new espera = Jugador[playerid][jStaffCooldown] - ahora;
        new texto[96];
        format(texto, sizeof texto, "Debes esperar %d min %d s antes de volver a usar este comando.", espera / 60, espera % 60);
        Error(playerid, texto);
        return 1;
    }

    if (Jugador[playerid][jStaffHoraInicio] == 0 || (ahora - Jugador[playerid][jStaffHoraInicio]) >= 3600)
    {
        Jugador[playerid][jStaffHoraInicio]  = ahora;
        Jugador[playerid][jStaffAccionesHora] = 0;
    }
    if (Jugador[playerid][jStaffAccionesHora] >= ASCENSOS_MAX_ACCIONES_HORA)
    {
        Error(playerid, "Has alcanzado el limite de acciones de staff comunitario para esta hora.");
        return 1;
    }

    new objetivoTexto[24], motivo[128];
    if (sscanf(params, "s[24]s[128]", objetivoTexto, motivo))
    {
        Uso(playerid, "/expulsarcomunidad [id o nombre] [motivo]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (objetivo == playerid)
    {
        Error(playerid, "No puedes usar este comando contigo mismo.");
        return 1;
    }
    if (Jugador[objetivo][jAdmin] > 0 || Jugador[objetivo][jStaffRango] >= Jugador[playerid][jStaffRango])
    {
        Error(playerid, "No puedes usar este comando con ese jugador.");
        return 1;
    }

    LimpiarTexto(motivo);
    if (EstaVacio(motivo))
    {
        Error(playerid, "Escribe un motivo.");
        return 1;
    }

    // Unica comprobacion que necesita BD -- se hace justo antes de
    // ejecutar, nunca por adelantado (peticion expresa de Vlad).
    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "SELECT COUNT(*) AS total FROM sanciones WHERE cuenta_id = %d AND tipo IN (%d,%d,%d,%d) AND fecha > NOW() - INTERVAL %d DAY",
        Jugador[playerid][jID], SANCION_KICK, SANCION_MUTE, SANCION_CARCEL, SANCION_BAN, ASCENSOS_SANCIONES_DIAS);
    mysql_pquery(g_BD, g_Buffer, "Ascensos_AlValidarExpulsion", "dds", playerid, objetivo, motivo);
    return 1;
}

// Las 4 condiciones que NO necesitan consulta a BD -- estado ya cargado
// en memoria, comprobacion instantanea.
stock bool:Ascensos_EsElegiblePunitivoSesion(playerid)
{
    if (Jugador[playerid][jStaffRango] < ASCENSOS_RANGO_PUNITIVO_MINIMO) return false;
    if (Jugador[playerid][jMinutosTotales] < ASCENSOS_ELEGIBILIDAD_PUNITIVO_MINUTOS) return false;
    if (Jugador[playerid][jBaneado]) return false;
    if (Jugador[playerid][jAvisos] >= 3) return false;
    return true;
}

forward Ascensos_AlValidarExpulsion(playerid, objetivo, motivo[]);
public Ascensos_AlValidarExpulsion(playerid, objetivo, motivo[])
{
    if (!IsPlayerConnected(playerid)) return 0;

    new sanciones;
    cache_get_value_name_int(0, "total", sanciones);

    if (sanciones > 0)
    {
        Error(playerid, "Tienes una sancion reciente (menos de 14 dias) y no puedes usar este comando ahora mismo.");
        return 1;
    }

    // Revalidacion final: el objetivo pudo desconectarse, o el propio
    // staff pudo perder el rango minimo, mientras se resolvia la
    // consulta -- nunca se ejecuta sobre un estado que ya cambio.
    if (!Ascensos_EsElegiblePunitivoSesion(playerid))
    {
        Error(playerid, "Ya no cumples los requisitos para usar este comando. Usa /ascensos para ver tu estado.");
        return 1;
    }
    if (!IsPlayerConnected(objetivo) || !Jugador[objetivo][jLogueado])
    {
        Error(playerid, "Ese jugador ya no esta conectado.");
        return 1;
    }

    new admin[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, admin, sizeof admin);
    NombreRP(objetivo, suyo, sizeof suyo);
    GetPlayerName(objetivo, cuenta, sizeof cuenta);

    new texto[144];
    format(texto, sizeof texto, "[Staff] %s ha expulsado a %s. Motivo: %s", admin, suyo, motivo);
    MensajeGlobal(COLOR_ADMIN_ACCION, texto);

    BD_RegistrarSancion(Jugador[objetivo][jID], cuenta, SANCION_KICK, admin, motivo, 0);

    new detalles[160];
    format(detalles, sizeof detalles, "rango=%d motivo=%s", Jugador[playerid][jStaffRango], motivo);
    Admin_Registrar(playerid, "STAFFAUTO_KICK", cuenta, detalles);

    BD_GuardarCuenta(objetivo);
    SetTimerEx("ExpulsarRetardado", 500, false, "d", objetivo);

    Jugador[playerid][jStaffCooldown] = gettime() + ASCENSOS_COOLDOWN_PUNITIVO_SEGUNDOS;
    Jugador[playerid][jStaffAccionesHora]++;
    return 1;
}
