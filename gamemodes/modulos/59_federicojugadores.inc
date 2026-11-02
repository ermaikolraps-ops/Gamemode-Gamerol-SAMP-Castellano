/* =====================================================================
 *  59_federicojugadores.inc
 *  ---------------------------------------------------------------------
 *  "COCHES DE JUGADORES" -- v1 (08/09/2026, encargo directo de Vlad,
 *  aprobado tras informe tecnico PASO A / INFORME-TECNICO-COCHES-
 *  JUGADORES-2026-09-08.md, Gamerol Estudio/ARCHIVO RAIZ).
 *
 *  UNICA categoria de Federico que acepta vehiculos CON dueno real
 *  (vePropietario != 0) -- el resto de 43_federico.inc (ronda normal,
 *  Coche Secreto, Moto del Dia, Chatarra, Robo Express) exige siempre
 *  un coche libre, y ese filtro NO se toca aqui para nada. Este modulo
 *  es una rama aparte, aislada:
 *
 *    - NO hay confiscacion automatica: el coche sigue en el mundo, el
 *      dueno lo puede seguir usando con total normalidad. Otro jugador
 *      tiene que ENCONTRARLO fisicamente y forzarle la cerradura
 *      (/forzarcerradura) para poder llevarselo a Federico.
 *    - 3 vehiculos EXACTOS por ronda, identificados siempre por veID
 *      persistente (nunca por modelo ni por indice de g_Vehiculos, que
 *      se recicla -- mismo patron ya usado por Robo Express).
 *    - Comparte la rotacion de 24h del resto de Federico (llamada desde
 *      Federico_RondaGeneralSincronizar, 43_federico.inc), pero NO la
 *      retirada de 24h de alli: cada vehiculo entregado tiene su propio
 *      plazo INDEPENDIENTE de 48h reales (FEDJUGADORES_RETORNO_SEGUNDOS),
 *      con persistencia PROPIA en la tabla `federico_jugadores`
 *      (sql/migraciones/046_federico_coches_jugadores.sql) -- nunca en
 *      `vehiculos.federico_until`, que es la contabilidad generica del
 *      resto de Federico y tiene su propia duracion fija de 24h.
 *    - Recompensas FIJAS propias (130k/100k/75k/50k, decision explicita
 *      del jefe) -- se reutiliza SOLO la clasificacion de umbral de
 *      Federico_Rareza (cuantas copias libres quedan del modelo), nunca
 *      su importe.
 *    - Zona (LS/SF/LV/CAMPO) via Hack_CiudadDe (31_web.inc), ya
 *      existente, sin duplicar logica de coordenadas -- se calcula una
 *      vez al elegir la ronda y queda congelada (SNAPSHOT) el resto de
 *      la ronda, aunque el dueno mueva el coche despues. Decision
 *      explicita del jefe.
 *    - Elegibilidad SIN exclusion de admin: el encargo original la
 *      pedia (jAdmin>0 fuera del pool), pero el jefe lo revirtio en
 *      directo durante esta misma sesion ("los admins jugamos
 *      tambien!", 08/09/2026) antes de terminar la implementacion --
 *      todo vehiculo con propietario real entra, sea quien sea.
 *
 *  DOS estructuras en memoria, con proposito distinto:
 *    g_FedJugSlot[3]      -- los 3 candidatos de la ronda VIGENTE
 *                            (buscados o ya entregados esta ronda).
 *    g_FedJugPendiente[]  -- vehiculos YA entregados, escondidos,
 *                            esperando su propia cuenta atras de 48h.
 *                            Sobrevive a que la ronda general rote otra
 *                            vez (un coche entregado a la hora 2 de una
 *                            ronda de 24h sigue escondido bastante mas
 *                            alla de que esa ronda termine).
 *  FedJugadores_EstaProtegido(veID) consulta AMBAS -- es el punto unico
 *  que usan las protecciones externas (Mercado Libre, /abandonarcoche,
 *  /puente, /dropearcoche, DropeoTicHora) para saber si un vehiculo esta
 *  "en juego" para esta feature ahora mismo.
 *
 *  PERSISTENCIA: tabla `federico_jugadores`, fila por seleccion (no fila
 *  unica como federico_estado -- el jefe pidio explicitamente tabla de
 *  filas). Una fila con entregado=0 y restaurado=0 es un slot vigente;
 *  con entregado=1 y restaurado=0 es un pendiente escondido; restaurado=1
 *  significa "esta fila ya no representa nada vivo", tanto si volvio de
 *  verdad como si la ronda roto sin que se entregara nunca (ver
 *  FedJugadores_NuevaRonda, que cierra los slots no entregados de la
 *  ronda saliente para que un arranque futuro no los cargue por error).
 *
 *  LIMITACION CONOCIDA Y ACEPTADA (aviso de retorno al propietario,
 *  caso E del encargo): si el propietario esta offline cuando su coche
 *  vuelve, el aviso "tu coche ha vuelto" para su proxima conexion vive
 *  SOLO en memoria (g_FedJugAvisoRetorno*) -- si el servidor se
 *  reinicia entre el retorno real (que si es persistente, el coche
 *  vuelve bien) y la proxima conexion del dueno, se pierde el aviso en
 *  si, no el coche. Aceptado a proposito por simplicidad (encargo:
 *  "mantenerlo sencillo, persistir solo lo necesario").
 * ===================================================================== */

enum E_FEDJUG_SLOT
{
    bool:fjActivo,
    fjVehiculoID,
    fjPropietario,
    fjModelo,
    fjNombre[32],
    fjRareza[16],
    fjRecompensa,
    fjZona[8],
    bool:fjEntregado,
    fjFilaID
}
new g_FedJugSlot[FEDJUGADORES_TOTAL_SLOTS][E_FEDJUG_SLOT];

enum E_FEDJUG_PENDIENTE
{
    bool:fpActivo,
    fpVehiculoID,
    fpPropietario,
    fpRetornoUntil,
    fpFilaID
}
new g_FedJugPendiente[FEDJUGADORES_MAX_PENDIENTES][E_FEDJUG_PENDIENTE];

new bool:g_FedJugListo = false;

// Avisos de retorno pendientes de mostrar en la siguiente conexion del
// propietario (ver limitacion conocida en la cabecera del archivo).
new g_FedJugAvisoRetornoCuenta[FEDJUGADORES_MAX_PENDIENTES] = {0, ...};
new g_FedJugAvisoRetornoNombre[FEDJUGADORES_MAX_PENDIENTES][32];
new g_FedJugTotalAvisosRetorno = 0;

// Buffer PROPIO para las consultas SQL de este modulo -- mismo motivo
// que g_FedericoConsulta (43_federico.inc): el g_Buffer global de 512
// bytes se desborda con consultas de varios campos y mysql_format lo
// trunca en silencio, danando el contenido para quien lo use justo
// despues. Nunca compartir este buffer con nada que no sea una consulta
// SQL de este modulo.
static g_FedJugConsulta[768];

// ---------------------------------------------------------------------
//  /FORZARCERRADURA -- estado de sesion por jugador/vehiculo.
// ---------------------------------------------------------------------
static bool:g_FedJugForzando[MAX_PLAYERS];
static g_FedJugForzarVehiculo[MAX_PLAYERS] = {-1, ...};
static g_FedJugForzarSegundos[MAX_PLAYERS];
static g_FedJugForzarTimer[MAX_PLAYERS] = {-1, ...};
static bool:g_FedJugOcupado[MAX_VEHICULOS];   // candado por-vehiculo, evita dos jugadores forzando el mismo a la vez

// ---------------------------------------------------------------------
//  PROTECCION -- punto UNICO que consultan Mercado Libre, /abandonarcoche,
//  /puente, /dropearcoche y DropeoTicHora para saber si un vehiculo esta
//  "en juego" para esta feature ahora mismo (buscado O escondido en su
//  ventana de 48h). No proteger un solo camino si hay varios -- por eso
//  esta funcion, no una comprobacion repetida distinta en cada sitio.
// ---------------------------------------------------------------------
// OJO -- el parametro se llama "vehId", NUNCA "veID": "veID" es el
// nombre del campo del enum E_VEHICULO (13_vehiculos.inc), usado en
// TODO el proyecto como g_Vehiculos[indice][veID]. Nombrar asi una
// variable local lo TAPA (shadowing) y el compilador deja de saber si
// "veID" dentro de unos corchetes es el campo del enum o la variable --
// error real de indexado, no solo un aviso cosmetico. Aprendido de la
// forma dificil durante esta misma sesion, ver informe de entrega.
stock bool:FedJugadores_EstaProtegido(vehId)
{
    if (vehId <= 0) return false;

    for (new s = 0; s < FEDJUGADORES_TOTAL_SLOTS; s++)
    {
        if (g_FedJugSlot[s][fjActivo] && g_FedJugSlot[s][fjVehiculoID] == vehId) return true;
    }
    for (new p = 0; p < FEDJUGADORES_MAX_PENDIENTES; p++)
    {
        if (g_FedJugPendiente[p][fpActivo] && g_FedJugPendiente[p][fpVehiculoID] == vehId) return true;
    }
    return false;
}

// bool: este vehiculo es HOY uno de los 3 candidatos de la ronda vigente
// Y todavia no se ha entregado -- lo unico que hace falta para que
// /entregarcoche y /forzarcerradura lo acepten. Un vehiculo ya
// entregado (escondido, en g_FedJugPendiente) NO es "entregable de
// nuevo": no esta fisicamente en el mundo.
stock bool:FedJugadores_PuedeEntregar(indice)
{
    if (!VehiculoValido(indice)) return false;
    new vehId = g_Vehiculos[indice][veID];
    if (vehId <= 0) return false;

    for (new s = 0; s < FEDJUGADORES_TOTAL_SLOTS; s++)
    {
        if (g_FedJugSlot[s][fjActivo] && !g_FedJugSlot[s][fjEntregado] && g_FedJugSlot[s][fjVehiculoID] == vehId) return true;
    }
    return false;
}

stock FedJugadores_SlotDeVehiculo(vehId)
{
    for (new s = 0; s < FEDJUGADORES_TOTAL_SLOTS; s++)
    {
        if (g_FedJugSlot[s][fjActivo] && !g_FedJugSlot[s][fjEntregado] && g_FedJugSlot[s][fjVehiculoID] == vehId) return s;
    }
    return -1;
}

// ---------------------------------------------------------------------
//  ACCESORES DE SOLO LECTURA -- g_FedJugSlot es un array normal (no
//  static), pero este modulo se incluye DESPUES de 43_federico.inc en
//  malagacity.pwn: a diferencia de las funciones (que si se resuelven
//  hacia atras via "forcing reparse", ya usado en todo el proyecto para
//  llamar de 43_federico.inc a RoboExpress_*/Ascensos_*, definidas mas
//  abajo), una variable global leida antes de su declaracion textual es
//  error duro de compilacion, no una advertencia. El propio proyecto
//  nunca lee el array de otro modulo a pelo (22_dropeo.inc llama
//  Federico_SlotDeModelo(), no lee g_FedericoSlot directo) -- estos
//  accesores siguen exactamente ese mismo patron, usados por
//  CMD:verlista (43_federico.inc) para pintar la lista completa.
// ---------------------------------------------------------------------
stock bool:FedJugadores_SlotActivo(slot)
{
    if (slot < 0 || slot >= FEDJUGADORES_TOTAL_SLOTS) return false;
    return g_FedJugSlot[slot][fjActivo];
}

stock bool:FedJugadores_SlotEntregado(slot)
{
    if (slot < 0 || slot >= FEDJUGADORES_TOTAL_SLOTS) return false;
    return g_FedJugSlot[slot][fjEntregado];
}

stock FedJugadores_VehiculoIdDeSlot(slot)
{
    if (slot < 0 || slot >= FEDJUGADORES_TOTAL_SLOTS) return 0;
    return g_FedJugSlot[slot][fjVehiculoID];
}

// Norma nueva (08/09/2026, encargo expreso del jefe): el propio dueno
// NO puede entregarse su vehiculo a si mismo -- tiene que traerlo otro
// jugador de verdad. Usado por CMD:entregarcoche (43_federico.inc);
// esta comprobacion hacia falta ahi porque el dueno puede conducir su
// propio coche sin necesitar /forzarcerradura (el candado de propiedad
// nunca le bloquea a el), asi que sin este corte podia "robarse" su
// propio coche sin que nadie se lo quitara antes.
stock bool:FedJugadores_EsPropietario(slot, cuentaId)
{
    if (slot < 0 || slot >= FEDJUGADORES_TOTAL_SLOTS) return false;
    return bool:(g_FedJugSlot[slot][fjPropietario] == cuentaId);
}

stock FedJugadores_RecompensaDeSlot(slot)
{
    if (slot < 0 || slot >= FEDJUGADORES_TOTAL_SLOTS) return 0;
    return g_FedJugSlot[slot][fjRecompensa];
}

stock FedJugadores_NombreDeSlot(slot, destino[], tam)
{
    if (slot < 0 || slot >= FEDJUGADORES_TOTAL_SLOTS) { destino[0] = EOS; return 0; }
    strmid(destino, g_FedJugSlot[slot][fjNombre], 0, tam - 1, tam);
    return 1;
}

stock FedJugadores_RarezaDeSlot(slot, destino[], tam)
{
    if (slot < 0 || slot >= FEDJUGADORES_TOTAL_SLOTS) { destino[0] = EOS; return 0; }
    strmid(destino, g_FedJugSlot[slot][fjRareza], 0, tam - 1, tam);
    return 1;
}

stock FedJugadores_ZonaDeSlot(slot, destino[], tam)
{
    if (slot < 0 || slot >= FEDJUGADORES_TOTAL_SLOTS) { destino[0] = EOS; return 0; }
    strmid(destino, g_FedJugSlot[slot][fjZona], 0, tam - 1, tam);
    return 1;
}

stock FedJugadores_IndicePorVehiculoID(vehId)
{
    for (new i = 0; i < MAX_VEHICULOS; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veID] == vehId) return i;
    }
    return -1;
}

// Nombre de cuenta del propietario, leido en vivo de g_Vehiculos (nunca
// una copia propia -- vePropietarioNombre no se toca en ningun momento
// del ciclo de esta feature, asi que sigue siendo el mismo dueno de
// siempre incluso mientras el coche esta escondido). Usado por
// CMD:verlista (43_federico.inc) y por el endpoint web.
stock FedJugadores_NombrePropietario(vehId, destino[], tam)
{
    new indice = FedJugadores_IndicePorVehiculoID(vehId);
    if (indice == -1)
    {
        strmid(destino, "?", 0, tam - 1, tam);
        return 0;
    }
    strmid(destino, g_Vehiculos[indice][vePropietarioNombre], 0, tam - 1, tam);
    return 1;
}

stock FedJugadores_SiguientePendienteLibre()
{
    for (new p = 0; p < FEDJUGADORES_MAX_PENDIENTES; p++)
    {
        if (!g_FedJugPendiente[p][fpActivo]) return p;
    }
    return -1;
}

// "Los Santos"/"San Fierro"/"Las Venturas"/"el campo" (Hack_CiudadDe,
// 31_web.inc) -> LS/SF/LV/CAMPO para mostrar. NO es un clasificador
// nuevo: solo reformatea el resultado del ya existente, tal como pidio
// el jefe ("no crear otro clasificador duplicado").
stock FedJugadores_ZonaAbreviada(const nombreCompleto[], destino[], tam)
{
    if (!strcmp(nombreCompleto, "Los Santos", true))        strmid(destino, "LS", 0, tam - 1, tam);
    else if (!strcmp(nombreCompleto, "San Fierro", true))   strmid(destino, "SF", 0, tam - 1, tam);
    else if (!strcmp(nombreCompleto, "Las Venturas", true)) strmid(destino, "LV", 0, tam - 1, tam);
    else                                                     strmid(destino, "CAMPO", 0, tam - 1, tam);
    return 1;
}

stock FedJugadores_AvisarPropietario(cuentaId, const mensaje[])
{
    for (new p = 0; p < MAX_PLAYERS; p++)
    {
        if (!IsPlayerConnected(p) || !Jugador[p][jLogueado]) continue;
        if (Jugador[p][jID] != cuentaId) continue;
        Aviso(p, mensaje);
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------
//  ELEGIBILIDAD -- devuelve INDICES de g_Vehiculos (no modelos: aqui
//  hace falta el vehiculo exacto). Reglas, todas obligatorias:
//    - VEH_PERSONAL o VEH_DROPEO, con dueno real (vePropietario != 0).
//    - No publicado en el Mercado Libre (veEnVenta).
//    - No retirado ya por CUALQUIER otra categoria de Federico
//      (veFedericoUntil > 0 -- Secreto/Moto/Chatarra/normal/RoboExpress
//      nunca coincidiran de todas formas al exigir vePropietario==0,
//      pero un vehiculo no puede estar "escondido" por partida doble).
//    - No ya protegido por esta misma feature (ronda vigente o
//      escondido de una entrega anterior).
//    - Papeles NO caducados EN ESTE INSTANTE (veDropeoExpira==0 sin
//      caducidad -- VEH_PERSONAL de catalogo -- o en el futuro). Uno
//      con 5 minutos o 30 dias es igual de valido, el encargo solo pide
//      excluir los que YA caducaron.
//
//  SIN exclusion de admin: el encargo original la pedia, pero el jefe
//  la revirtio en directo durante esta misma sesion de implementacion
//  ("los admins jugamos tambien!" -- 08/09/2026). TODO vehiculo con
//  propietario real entra en el pool, sea quien sea el dueno.
// ---------------------------------------------------------------------
stock FedJugadores_VehiculosElegibles(destino[], maxVehiculos)
{
    new ahora = gettime();
    new total = 0;

    for (new i = 0; i < MAX_VEHICULOS && total < maxVehiculos; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_PERSONAL && g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] == 0) continue;
        if (g_Vehiculos[i][veEnVenta]) continue;
        if (g_Vehiculos[i][veFedericoUntil] > 0) continue;
        if (g_Vehiculos[i][veDropeoExpira] != 0 && g_Vehiculos[i][veDropeoExpira] <= ahora) continue;
        if (FedJugadores_EstaProtegido(g_Vehiculos[i][veID])) continue;

        destino[total] = i;
        total++;
    }
    return total;
}

// ---------------------------------------------------------------------
//  RONDA NUEVA -- llamada desde Federico_RondaGeneralSincronizar()
//  (43_federico.inc), junto con las otras 4 categorias, cada 24h.
//  Cierra los slots de la ronda saliente que NUNCA se entregaron (si
//  no, un arranque futuro los cargaria como vigentes por error -- ver
//  FedJugadores_AlCargarEstado). Los que SI se entregaron NO se tocan
//  aqui: ya viven en g_FedJugPendiente con vida propia, independiente
//  de esta rotacion. 100% sincrona (sin cache de admin que refrescar
//  desde que se quito esa exclusion).
// ---------------------------------------------------------------------
stock FedJugadores_NuevaRonda()
{
    for (new s = 0; s < FEDJUGADORES_TOTAL_SLOTS; s++)
    {
        if (g_FedJugSlot[s][fjActivo] && !g_FedJugSlot[s][fjEntregado] && g_FedJugSlot[s][fjFilaID] > 0)
        {
            format(g_FedJugConsulta, sizeof g_FedJugConsulta,
                "UPDATE federico_jugadores SET restaurado = 1 WHERE id = %d", g_FedJugSlot[s][fjFilaID]);
            mysql_pquery(g_BD, g_FedJugConsulta);
        }
        new vacio[E_FEDJUG_SLOT];
        g_FedJugSlot[s] = vacio;
    }

    FedJugadores_SeleccionarYPersistir();
    return 1;
}

stock FedJugadores_SeleccionarYPersistir()
{
    new candidatos[300];
    new totalCandidatos = FedJugadores_VehiculosElegibles(candidatos, sizeof candidatos);

    new elegidos[FEDJUGADORES_TOTAL_SLOTS];
    new totalElegidos = 0;

    if (totalCandidatos > 0)
    {
        new copia[300];
        for (new i = 0; i < totalCandidatos; i++) copia[i] = candidatos[i];

        new restantes = totalCandidatos;
        while (totalElegidos < FEDJUGADORES_TOTAL_SLOTS && restantes > 0)
        {
            new pick = random(restantes);
            elegidos[totalElegidos] = copia[pick];
            totalElegidos++;
            copia[pick] = copia[restantes - 1];
            restantes--;
        }
    }

    new logResumen[280] = "";
    for (new s = 0; s < FEDJUGADORES_TOTAL_SLOTS; s++)
    {
        if (s >= totalElegidos) continue;

        new indice = elegidos[s];

        g_FedJugSlot[s][fjActivo]      = true;
        g_FedJugSlot[s][fjVehiculoID]  = g_Vehiculos[indice][veID];
        g_FedJugSlot[s][fjPropietario] = g_Vehiculos[indice][vePropietario];
        g_FedJugSlot[s][fjModelo]      = g_Vehiculos[indice][veModelo];
        g_FedJugSlot[s][fjEntregado]   = false;
        g_FedJugSlot[s][fjFilaID]      = 0; // lo rellena FedJugadores_AlInsertarSlot cuando vuelva el INSERT

        Veh_NombreModelo(g_FedJugSlot[s][fjModelo], g_FedJugSlot[s][fjNombre], 32);

        // Rareza: SOLO se reutiliza el umbral de Federico_Rareza (cuantas
        // copias libres quedan del modelo); el importe es propio de esta
        // categoria, decision explicita del jefe -- ver cabecera.
        new cantidad = Federico_ContarModelo(g_FedJugSlot[s][fjModelo]);
        new rarezaTmp[16], recompensaSinUsar;
        Federico_Rareza(cantidad, rarezaTmp, 16, recompensaSinUsar);
        strmid(g_FedJugSlot[s][fjRareza], rarezaTmp, 0, 16, 16);

        if (!strcmp(rarezaTmp, "SUPER RARE", true))       g_FedJugSlot[s][fjRecompensa] = FEDJUGADORES_RECOMPENSA_SUPERRARE;
        else if (!strcmp(rarezaTmp, "RARE", true))        g_FedJugSlot[s][fjRecompensa] = FEDJUGADORES_RECOMPENSA_RARE;
        else if (!strcmp(rarezaTmp, "DEL MONTON", true))  g_FedJugSlot[s][fjRecompensa] = FEDJUGADORES_RECOMPENSA_DELMONTON;
        else                                                g_FedJugSlot[s][fjRecompensa] = FEDJUGADORES_RECOMPENSA_CHATARRA;

        // Zona: SNAPSHOT en el instante de la seleccion (decision
        // explicita del jefe, no se recalcula despues aunque el dueno
        // mueva el coche). Posicion fisica real si el objeto esta
        // materializado; si no, la ultima guardada en la fila.
        new Float:zx, Float:zy;
        if (g_Vehiculos[indice][veObjeto] != INVALID_VEHICLE_ID)
        {
            new Float:zz;
            GetVehiclePos(g_Vehiculos[indice][veObjeto], zx, zy, zz);
        }
        else
        {
            zx = g_Vehiculos[indice][veX];
            zy = g_Vehiculos[indice][veY];
        }
        new zonaCompleta[24];
        Hack_CiudadDe(zx, zy, zonaCompleta, sizeof zonaCompleta);
        FedJugadores_ZonaAbreviada(zonaCompleta, g_FedJugSlot[s][fjZona], 8);

        mysql_format(g_BD, g_FedJugConsulta, sizeof g_FedJugConsulta,
            "INSERT INTO federico_jugadores (slot, vehiculo_id, propietario_id, modelo, rareza, recompensa, zona) \
             VALUES (%d, %d, %d, %d, '%e', %d, '%e')",
            s, g_FedJugSlot[s][fjVehiculoID], g_FedJugSlot[s][fjPropietario], g_FedJugSlot[s][fjModelo],
            g_FedJugSlot[s][fjRareza], g_FedJugSlot[s][fjRecompensa], g_FedJugSlot[s][fjZona]);
        mysql_pquery(g_BD, g_FedJugConsulta, "FedJugadores_AlInsertarSlot", "d", s);

        // Aviso privado al propietario si esta conectado ahora mismo
        // (caso A del encargo: "si esta conectado cuando su coche entra
        // en la lista").
        format(g_Buffer, sizeof g_Buffer,
            "Ojo: Federico esta buscando tu %s. Otro jugador puede forzar la cerradura y llevarselo -- escondelo bien o entregaselo tu mismo con /entregarcoche.",
            g_FedJugSlot[s][fjNombre]);
        FedJugadores_AvisarPropietario(g_FedJugSlot[s][fjPropietario], g_Buffer);

        new pedazo[70];
        format(pedazo, sizeof pedazo, "%sslot%d=veh%d(%s,%s,%d$)",
            (s > 0) ? (" ") : (""), s, g_FedJugSlot[s][fjVehiculoID], g_FedJugSlot[s][fjRareza], g_FedJugSlot[s][fjZona], g_FedJugSlot[s][fjRecompensa]);
        strcat(logResumen, pedazo, sizeof logResumen);
    }

    g_FedJugListo = true;

    new logMsg[340];
    format(logMsg, sizeof logMsg, "FEDJUGADORES_ROUND_STARTED elegidos=%d/%d candidatos_totales=%d %s",
        totalElegidos, FEDJUGADORES_TOTAL_SLOTS, totalCandidatos, logResumen);
    EscribirLog("federico", logMsg);
    return 1;
}

forward FedJugadores_AlInsertarSlot(slot);
public FedJugadores_AlInsertarSlot(slot)
{
    if (slot < 0 || slot >= FEDJUGADORES_TOTAL_SLOTS) return 0;
    if (!g_FedJugSlot[slot][fjActivo]) return 0; // defensivo: la ronda pudo rotar otra vez mientras el INSERT volaba
    g_FedJugSlot[slot][fjFilaID] = cache_insert_id();
    return 1;
}

// ---------------------------------------------------------------------
//  ENTREGA -- llamada desde dentro de la zona critica de /entregarcoche
//  (43_federico.inc, CMD:entregarcoche), con g_FedericoProcesando ya en
//  true. Todo aqui es SINCRONO (ningun await/callback intermedio antes
//  de que el llamador suelte el candado) -- mismo contrato que el resto
//  de ramas de esa funcion.
// ---------------------------------------------------------------------
stock FedJugadores_Entregar(slot, indice)
{
    g_FedJugSlot[slot][fjEntregado] = true;

    // Retirada fisica -- mismos 3 pasos que Federico_RetirarVehiculo,
    // pero SIN tocar vehiculos.federico_until (persistencia propia, ver
    // cabecera del archivo) y sin su cooldown fijo de 24h.
    DestroyVehicle(g_Vehiculos[indice][veObjeto]);
    Dropeo_AlBorrarVehiculo(indice);
    g_Vehiculos[indice][veObjeto] = INVALID_VEHICLE_ID;

    new hasta = gettime() + FEDJUGADORES_RETORNO_SEGUNDOS;

    new p = FedJugadores_SiguientePendienteLibre();
    if (p != -1)
    {
        g_FedJugPendiente[p][fpActivo]       = true;
        g_FedJugPendiente[p][fpVehiculoID]   = g_FedJugSlot[slot][fjVehiculoID];
        g_FedJugPendiente[p][fpPropietario]  = g_FedJugSlot[slot][fjPropietario];
        g_FedJugPendiente[p][fpRetornoUntil] = hasta;
        g_FedJugPendiente[p][fpFilaID]       = g_FedJugSlot[slot][fjFilaID];
    }
    else
    {
        // No deberia pasar nunca con un limite tan generoso -- pero si
        // pasara, se deja constancia en vez de perder el rastro en
        // silencio. El vehiculo sigue protegido igualmente porque su
        // fila en g_FedJugSlot se queda con fjActivo/fjEntregado=true.
        format(g_Buffer, sizeof g_Buffer,
            "FEDJUGADORES_SIN_HUECO_PENDIENTE vehiculo_id=%d -- FEDJUGADORES_MAX_PENDIENTES lleno",
            g_FedJugSlot[slot][fjVehiculoID]);
        EscribirLog("federico", g_Buffer);
    }

    mysql_format(g_BD, g_FedJugConsulta, sizeof g_FedJugConsulta,
        "UPDATE federico_jugadores SET entregado = 1, retorno_until = FROM_UNIXTIME(%d) WHERE id = %d",
        hasta, g_FedJugSlot[slot][fjFilaID]);
    mysql_pquery(g_BD, g_FedJugConsulta);

    // Aviso privado al propietario si esta conectado (caso C del
    // encargo: "si otro jugador entrega su coche, avisarle si esta
    // conectado").
    FedJugadores_AvisarPropietario(g_FedJugSlot[slot][fjPropietario],
        "Te han robado un vehiculo que Federico estaba buscando. Volvera a su ultimo sitio en 48 horas reales.");
    return 1;
}

// ---------------------------------------------------------------------
//  RETORNO A LAS 48H -- comprobado cada minuto (FedJugadores_Minuto,
//  llamado desde TicMinuto) y una vez al arrancar (por si el plazo ya
//  habia vencido con el servidor apagado -- "no prolongar las 48h por
//  downtime", encargo explicito).
// ---------------------------------------------------------------------
stock FedJugadores_ComprobarRetornos()
{
    new ahora = gettime();
    for (new p = 0; p < FEDJUGADORES_MAX_PENDIENTES; p++)
    {
        if (!g_FedJugPendiente[p][fpActivo]) continue;
        if (g_FedJugPendiente[p][fpRetornoUntil] > ahora) continue;

        FedJugadores_DevolverVehiculo(p);
    }
    return 1;
}

stock FedJugadores_DevolverVehiculo(p)
{
    new vehId         = g_FedJugPendiente[p][fpVehiculoID];
    new propietarioId = g_FedJugPendiente[p][fpPropietario];
    new indice        = FedJugadores_IndicePorVehiculoID(vehId);

    if (indice == -1)
    {
        // El vehiculo ya no existe (borrado por fuera de este sistema,
        // p.ej. /borrarveh de un admin) -- no hay nada fisico que
        // devolver. Se cierra la fila igual para no reintentarlo cada
        // minuto para siempre.
        format(g_Buffer, sizeof g_Buffer,
            "FEDJUGADORES_RETORNO_FALLIDO vehiculo_id=%d motivo=vehiculo_no_encontrado", vehId);
        EscribirLog("federico", g_Buffer);
    }
    else
    {
        // Mismo mecanismo que Federico_DevolverVehiculo: vuelve a su
        // ULTIMA posicion conocida (veX/veY/veZ/veA), sin tocar
        // vePropietario para nada -- nunca se toco durante la retirada,
        // asi que sigue siendo el mismo dueno de antes por construccion.
        Veh_Aparecer(indice);
        if (g_Vehiculos[indice][veTipo] == VEH_DROPEO) Dropeo_CrearCartel(indice);

        format(g_Buffer, sizeof g_Buffer,
            "FEDJUGADORES_RETORNO vehiculo_id=%d propietario=%d pos=%.2f,%.2f,%.2f timestamp=%d",
            vehId, propietarioId, g_Vehiculos[indice][veX], g_Vehiculos[indice][veY], g_Vehiculos[indice][veZ], gettime());
        EscribirLog("federico", g_Buffer);
    }

    format(g_FedJugConsulta, sizeof g_FedJugConsulta,
        "UPDATE federico_jugadores SET restaurado = 1 WHERE id = %d", g_FedJugPendiente[p][fpFilaID]);
    mysql_pquery(g_BD, g_FedJugConsulta);

    // Caso E del encargo: avisar si esta conectado; si no, dejar aviso
    // para la siguiente conexion (ver limitacion conocida, cabecera).
    new bool:conectado = bool:FedJugadores_AvisarPropietario(propietarioId,
        "Tu vehiculo que tenia Federico ha vuelto a su ultimo sitio conocido.");

    if (!conectado && g_FedJugTotalAvisosRetorno < FEDJUGADORES_MAX_PENDIENTES)
    {
        new a = g_FedJugTotalAvisosRetorno;
        g_FedJugAvisoRetornoCuenta[a] = propietarioId;
        if (indice != -1) Veh_NombreModelo(g_Vehiculos[indice][veModelo], g_FedJugAvisoRetornoNombre[a], 32);
        else g_FedJugAvisoRetornoNombre[a][0] = EOS;
        g_FedJugTotalAvisosRetorno++;
    }

    g_FedJugPendiente[p][fpActivo] = false;
    return 1;
}

// ---------------------------------------------------------------------
//  ARRANQUE -- llamado desde AlCargarVehiculos (13_vehiculos.inc), justo
//  despues de Federico_HistoricoIniciar, una vez el pool de vehiculos ya
//  esta en memoria (misma exigencia que el resto de sub-arranques de
//  Federico).
// ---------------------------------------------------------------------
stock FedJugadores_Iniciar()
{
    mysql_pquery(g_BD,
        "SELECT *, UNIX_TIMESTAMP(retorno_until) AS retorno_unix FROM federico_jugadores WHERE restaurado = 0",
        "FedJugadores_AlCargarEstado");
    return 1;
}

forward FedJugadores_AlCargarEstado();
public FedJugadores_AlCargarEstado()
{
    new filas;
    cache_get_row_count(filas);

    for (new r = 0; r < filas; r++)
    {
        new entregado;
        cache_get_value_name_int(r, "entregado", entregado);

        if (!entregado)
        {
            new slot;
            cache_get_value_name_int(r, "slot", slot);
            if (slot < 0 || slot >= FEDJUGADORES_TOTAL_SLOTS) continue;

            g_FedJugSlot[slot][fjActivo] = true;
            cache_get_value_name_int(r, "id", g_FedJugSlot[slot][fjFilaID]);
            cache_get_value_name_int(r, "vehiculo_id", g_FedJugSlot[slot][fjVehiculoID]);
            cache_get_value_name_int(r, "propietario_id", g_FedJugSlot[slot][fjPropietario]);
            cache_get_value_name_int(r, "modelo", g_FedJugSlot[slot][fjModelo]);
            cache_get_value_name(r, "rareza", g_FedJugSlot[slot][fjRareza], 16);
            cache_get_value_name_int(r, "recompensa", g_FedJugSlot[slot][fjRecompensa]);
            cache_get_value_name(r, "zona", g_FedJugSlot[slot][fjZona], 8);
            g_FedJugSlot[slot][fjEntregado] = false;

            Veh_NombreModelo(g_FedJugSlot[slot][fjModelo], g_FedJugSlot[slot][fjNombre], 32);
        }
        else
        {
            new p = FedJugadores_SiguientePendienteLibre();
            if (p == -1) continue; // no deberia pasar con un limite tan generoso, pero no revienta si pasa

            g_FedJugPendiente[p][fpActivo] = true;
            cache_get_value_name_int(r, "id", g_FedJugPendiente[p][fpFilaID]);
            cache_get_value_name_int(r, "vehiculo_id", g_FedJugPendiente[p][fpVehiculoID]);
            cache_get_value_name_int(r, "propietario_id", g_FedJugPendiente[p][fpPropietario]);
            cache_get_value_name_int(r, "retorno_unix", g_FedJugPendiente[p][fpRetornoUntil]);
        }
    }

    g_FedJugListo = true;

    // Si algun plazo ya vencio con el servidor apagado, se restaura
    // AHORA -- "no prolongar las 48h por downtime", encargo explicito.
    FedJugadores_ComprobarRetornos();

    new logMsg[120];
    format(logMsg, sizeof logMsg, "FEDJUGADORES_ESTADO_RESTAURADO filas=%d", filas);
    EscribirLog("federico", logMsg);
    return 1;
}

// ---------------------------------------------------------------------
//  TICK -- llamado cada minuto desde TicMinuto (99_callbacks.inc), mismo
//  patron que RoboExpress_Minuto: global, no per-player.
// ---------------------------------------------------------------------
stock FedJugadores_Minuto()
{
    if (!g_FedJugListo) return 0;
    FedJugadores_ComprobarRetornos();
    return 1;
}

// ---------------------------------------------------------------------
//  CONEXION -- llamado desde OnPlayerConnect (99_callbacks.inc), justo
//  despues de Federico_OnConectar.
// ---------------------------------------------------------------------
stock FedJugadores_OnConectar(playerid)
{
    g_FedJugForzando[playerid]        = false;
    g_FedJugForzarVehiculo[playerid]  = -1;
    g_FedJugForzarTimer[playerid]     = -1;

    if (!g_FedJugListo) return 1;

    new cuentaId = Jugador[playerid][jID];

    // Casos A/B combinados: tiene un vehiculo actualmente buscado
    // (sin entregar) por Federico.
    for (new s = 0; s < FEDJUGADORES_TOTAL_SLOTS; s++)
    {
        if (!g_FedJugSlot[s][fjActivo] || g_FedJugSlot[s][fjEntregado]) continue;
        if (g_FedJugSlot[s][fjPropietario] != cuentaId) continue;

        format(g_Buffer, sizeof g_Buffer,
            "Ojo: Federico esta buscando tu %s ahora mismo. Escondelo bien o entregaselo tu con /entregarcoche antes de que te lo roben.",
            g_FedJugSlot[s][fjNombre]);
        Aviso(playerid, g_Buffer);
    }

    // Caso D: coche ya entregado por otro jugador, todavia en su
    // ventana de 48h.
    for (new p = 0; p < FEDJUGADORES_MAX_PENDIENTES; p++)
    {
        if (!g_FedJugPendiente[p][fpActivo]) continue;
        if (g_FedJugPendiente[p][fpPropietario] != cuentaId) continue;

        new restante = g_FedJugPendiente[p][fpRetornoUntil] - gettime();
        if (restante < 0) restante = 0;
        format(g_Buffer, sizeof g_Buffer,
            "Uno de tus vehiculos fue entregado a Federico por otro jugador. Vuelve en unas %d horas.",
            (restante / 3600) + 1);
        Aviso(playerid, g_Buffer);
    }

    // Caso E: avisos de retorno que se perdieron por estar offline en el
    // momento exacto -- se consumen aqui, uno por uno.
    for (new a = 0; a < g_FedJugTotalAvisosRetorno; a++)
    {
        if (g_FedJugAvisoRetornoCuenta[a] != cuentaId) continue;

        format(g_Buffer, sizeof g_Buffer, "Mientras estabas fuera, Federico te devolvio tu %s.",
            (g_FedJugAvisoRetornoNombre[a][0] != EOS) ? (g_FedJugAvisoRetornoNombre[a]) : ("vehiculo"));
        Aviso(playerid, g_Buffer);

        for (new b = a; b < g_FedJugTotalAvisosRetorno - 1; b++)
        {
            g_FedJugAvisoRetornoCuenta[b] = g_FedJugAvisoRetornoCuenta[b + 1];
            g_FedJugAvisoRetornoNombre[b] = g_FedJugAvisoRetornoNombre[b + 1];
        }
        g_FedJugTotalAvisosRetorno--;
        a--;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  /FORZARCERRADURA -- referencia estructural: /forzarmaletero
//  (45_maletero.inc). MISMO esqueleto (freeze, animacion, cuenta atras
//  por segundo, tirada de exito, aviso policial, revalidacion tras el
//  freeze, limpieza al desconectar) con constantes y efecto de exito
//  PROPIOS -- nunca abre el menu del maletero, eso es un sistema aparte.
// ---------------------------------------------------------------------
CMD:forzarcerradura(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (g_FedJugForzando[playerid])
    {
        Error(playerid, "Ya estas forzando una cerradura. Espera.");
        return 1;
    }

    new indice = Veh_BuscarCercano(playerid, FORZARCERRADURA_RADIO);
    if (indice == -1)
    {
        Error(playerid, "No hay ningun vehiculo cerca.");
        return 1;
    }

    if (!FedJugadores_PuedeEntregar(indice))
    {
        Error(playerid, "Federico no busca este vehiculo.");
        return 1;
    }

    if (g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID])
    {
        Error(playerid, "Este vehiculo ya es tuyo.");
        return 1;
    }

    if (g_FedJugOcupado[indice])
    {
        Error(playerid, "Alguien esta forzando este vehiculo ahora mismo.");
        return 1;
    }

    g_FedJugForzando[playerid]       = true;
    g_FedJugForzarVehiculo[playerid] = indice;
    g_FedJugForzarSegundos[playerid] = FORZARCERRADURA_FREEZE_MS / 1000;
    g_FedJugOcupado[indice]          = true;

    TogglePlayerControllable(playerid, false);
    Animar(playerid, "CAR", "crckdoor", 4.0, true, true, true, false, 0);

    format(g_Buffer, sizeof g_Buffer, "~w~Forzando Cerradura...~n~~r~%d s", g_FedJugForzarSegundos[playerid]);
    GameTextForPlayer(playerid, g_Buffer, 1100, 4);

    g_FedJugForzarTimer[playerid] = SetTimerEx("FedJugadores_ForzarTick", 1000, true, "i", playerid);
    return 1;
}

forward FedJugadores_ForzarTick(playerid);
public FedJugadores_ForzarTick(playerid)
{
    if (!IsPlayerConnected(playerid) || !g_FedJugForzando[playerid])
    {
        if (g_FedJugForzarTimer[playerid] != -1)
        {
            KillTimer(g_FedJugForzarTimer[playerid]);
            g_FedJugForzarTimer[playerid] = -1;
        }
        return 0;
    }

    g_FedJugForzarSegundos[playerid]--;
    if (g_FedJugForzarSegundos[playerid] <= 0)
    {
        KillTimer(g_FedJugForzarTimer[playerid]);
        g_FedJugForzarTimer[playerid] = -1;
        FedJugadores_ForzarFin(playerid);
        return 1;
    }

    format(g_Buffer, sizeof g_Buffer, "~w~Forzando Cerradura...~n~~r~%d s", g_FedJugForzarSegundos[playerid]);
    GameTextForPlayer(playerid, g_Buffer, 1100, 4);
    return 1;
}

stock FedJugadores_ForzarFin(playerid)
{
    TogglePlayerControllable(playerid, true);
    g_FedJugForzando[playerid] = false;

    new indice = g_FedJugForzarVehiculo[playerid];
    g_FedJugForzarVehiculo[playerid] = -1;

    if (indice == -1 || !VehiculoValido(indice))
    {
        if (indice != -1) g_FedJugOcupado[indice] = false;
        Error(playerid, "El vehiculo ya no esta disponible.");
        return 0;
    }

    g_FedJugOcupado[indice] = false;

    // Revalidacion tras el freeze -- pudo dejar de ser un Coche de
    // Jugador entregable mientras forzabas (otro jugador lo entrego un
    // instante antes, o roto la ronda general).
    if (!FedJugadores_PuedeEntregar(indice))
    {
        Error(playerid, "Este vehiculo ya no esta buscado por Federico.");
        return 0;
    }

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);
    Policia_CrearAviso(AVISO_CERRADURA, playerid, px, py, pz, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));

    if (random(100) < FORZARCERRADURA_EXITO)
    {
        // Exito: desbloquea el vehiculo para conducirlo -- NUNCA el
        // menu del maletero (45_maletero.inc es un sistema aparte).
        // veCerrado + Veh_ActualizarCierreTodos es el mecanismo real de
        // bloqueo del proyecto; Dropeo_MarcarArrancable es el accesor ya
        // expuesto por 22_dropeo.inc para el motor (g_DropeoArrancado es
        // static, no se lee/escribe directo desde otro archivo).
        g_Vehiculos[indice][veCerrado] = false;
        Veh_ActualizarCierreTodos(indice);
        Dropeo_MarcarArrancable(indice);

        GameTextForPlayer(playerid, "~g~CERRADURA FORZADA", 3000, 4);
        // Este SI se queda (a diferencia de /forzarmaletero, 08/09/2026):
        // lleva una instruccion real ("llevalo hasta Federico") que el
        // GameText no dice, no es una simple repeticion del resultado.
        Exito(playerid, "Has forzado la cerradura. Ya puedes entrar y llevartelo hasta Federico.");

        format(g_Buffer, sizeof g_Buffer, "FEDJUGADORES_FORZAR_EXITO jugador=%d vehiculo_id=%d",
            Jugador[playerid][jID], g_Vehiculos[indice][veID]);
        EscribirLog("federico", g_Buffer);
    }
    else
    {
        // Chat de fallo retirado el 08/09/2026: pura repeticion del texto
        // grande en pantalla, sin ningun dato nuevo (mismo criterio que
        // /forzarmaletero).
        GameTextForPlayer(playerid, "~r~NO HAS CONSEGUIDO FORZARLA", 2500, 4);

        format(g_Buffer, sizeof g_Buffer, "FEDJUGADORES_FORZAR_FALLO jugador=%d vehiculo_id=%d",
            Jugador[playerid][jID], g_Vehiculos[indice][veID]);
        EscribirLog("federico", g_Buffer);
    }
    return 1;
}

// Limpieza si el jugador se desconecta a mitad de un forzado -- sin
// esto, el candado por-vehiculo (g_FedJugOcupado) se quedaria puesto
// para siempre, bloqueando ese vehiculo para cualquier otro (mismo tipo
// de bug que ya se evito en 45_maletero.inc con Maletero_OnDesconectar).
stock FedJugadores_OnDesconectar(playerid)
{
    if (g_FedJugForzarTimer[playerid] != -1)
    {
        KillTimer(g_FedJugForzarTimer[playerid]);
        g_FedJugForzarTimer[playerid] = -1;
    }

    if (g_FedJugForzando[playerid])
    {
        new indice = g_FedJugForzarVehiculo[playerid];
        if (indice != -1) g_FedJugOcupado[indice] = false;
        g_FedJugForzando[playerid] = false;
        g_FedJugForzarVehiculo[playerid] = -1;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  HERRAMIENTAS DE PRUEBA (DEBUG, ADMIN_JEFE) -- 08/09/2026, encargo
//  explicito del jefe para no tener que esperar 24h/48h reales durante
//  PASO C. Mismo patron de permisos que /forzarroboexpress
//  (16_administracion.inc). Considerar retirarlas -- o dejarlas
//  claramente marcadas como estan ahora -- una vez terminadas las
//  pruebas. NO accesibles a jugadores normales.
// ---------------------------------------------------------------------
CMD:forzarrondajugadores(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    FedJugadores_NuevaRonda();
    Exito(playerid, "[DEBUG] Ronda de Coches de Jugadores forzada -- la seleccion es asincrona, revisa servidor.log en unos segundos.");
    Admin_Registrar(playerid, "FORZARRONDAJUGADORES", "-", "Ronda forzada (debug)");
    return 1;
}

CMD:forzarretornojugadores(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new total = 0;
    for (new p = 0; p < FEDJUGADORES_MAX_PENDIENTES; p++)
    {
        if (!g_FedJugPendiente[p][fpActivo]) continue;
        g_FedJugPendiente[p][fpRetornoUntil] = gettime();
        total++;
    }
    FedJugadores_ComprobarRetornos();

    format(g_Buffer, sizeof g_Buffer, "[DEBUG] %d vehiculo(s) pendiente(s) forzado(s) a devolver ahora mismo.", total);
    Exito(playerid, g_Buffer);
    Admin_Registrar(playerid, "FORZARRETORNOJUGADORES", "-", "Retorno forzado (debug)");
    return 1;
}
