/* =====================================================================
 *  46_guardias.inc
 *  ---------------------------------------------------------------------
 *  GUARDIAS ARMADOS -- encargo de Vlad del 05/09/2026, pensado para
 *  defender un punto en misiones o golpes. Un admin planta uno con
 *  /crearguardia justo delante de si mismo; el guardia se queda quieto
 *  para siempre (nunca usa movimiento) y dispara sin distincion a
 *  cualquier jugador que entre en rango, este en el bando que este.
 *
 *  API usada: FCNPC (qawno/include/FCNPC.inc, componente
 *  components/FCNPC.dll -- version "omp-FCNPC" de Mac-Andreas, NO la
 *  FCNPC clasica de toda la vida, que no funciona en open.mp). Es una
 *  capa de compatibilidad que por dentro llama al mismo INPCComponent
 *  nativo de open.mp (NPCs.dll) que se probo primero con la API NPC_*
 *  pura y no consiguio que el NPC levantara el arma visualmente aunque
 *  disparaba de verdad -- documentado el 05/09/2026 con /debugguardia
 *  (arma=29, apuntando=1, disparando=1 por dentro, brazos abajo en
 *  pantalla). Se cambia a FCNPC a peticion expresa de Vlad para ver si
 *  su capa de compatibilidad se comporta distinto en ese punto
 *  concreto, aun sabiendo que el motor de fondo es el mismo.
 *
 *  Los eventos OnNPCCreate/OnNPCSpawn/OnNPCDeath de la API nativa
 *  siguen disparandose para un NPC creado con FCNPC_Create (es el mismo
 *  objeto INPCComponent por debajo) -- pero por si acaso esta capa de
 *  compatibilidad concreta no los dispara igual, Guardias_Tick hace
 *  ademas su propia comprobacion de FCNPC_IsDead() cada segundo como
 *  red de seguridad, sin depender solo de OnNPCDeath.
 *
 *  PERSISTENCIA: tabla `guardias` (fila por guardia, igual que
 *  `atracos` en 39_atracos.inc). Se recrean todos los NPC al arrancar
 *  el servidor a partir de esa tabla -- el proceso del servidor no
 *  sobrevive un reinicio, pero el SITIO donde debe reaparecer si.
 *
 *  gdSerial existe para que un temporizador tardio (el respawn de un
 *  guardia que mientras tanto se borro con /borrarguardia y el hueco
 *  se reuso para uno nuevo) no reviva ni vuelva hostil al guardia
 *  equivocado -- se compara antes de actuar.
 * ===================================================================== */

enum E_GUARDIA
{
    bool:gdUsado,
    gdID,                    // fila en la tabla `guardias`
    gdSerial,                // se incrementa en cada creacion/revivir de este hueco
    Float:gdX, Float:gdY, Float:gdZ, Float:gdAngulo,
    gdMundo, gdInterior,
    gdSkin,
    gdNpc,                   // ID devuelto por FCNPC_Create, INVALID_PLAYER_ID si no esta vivo ahora
    bool:gdHostil,
    gdObjetivo               // playerid al que apunta ahora mismo, o INVALID_PLAYER_ID
}
new g_Guardias[MAX_GUARDIAS][E_GUARDIA];

// Pool de skins "guardia de seguridad" verificados contra la wiki de
// SA-MP -- 71 Guardia de seguridad, 163 Portero negro, 164 Portero
// blanco. Nada de policia/ejercito/SWAT: esos ya son la faccion real.
static const g_GuardiaSkins[] = {71, 163, 164};

static g_GuardiaConsulta[256];

// ---------------------------------------------------------------------
//  CARGA AL ARRANCAR
// ---------------------------------------------------------------------
stock Guardias_CargarTodos()
{
    mysql_pquery(g_BD, "SELECT * FROM guardias ORDER BY id ASC", "AlCargarGuardias");
    return 1;
}

forward AlCargarGuardias();
public AlCargarGuardias()
{
    new filas;
    cache_get_row_count(filas);

    new total = 0;
    for (new i = 0; i < filas && i < MAX_GUARDIAS; i++)
    {
        g_Guardias[i][gdUsado] = true;
        cache_get_value_name_int(i, "id", g_Guardias[i][gdID]);
        cache_get_value_name_float(i, "x", g_Guardias[i][gdX]);
        cache_get_value_name_float(i, "y", g_Guardias[i][gdY]);
        cache_get_value_name_float(i, "z", g_Guardias[i][gdZ]);
        cache_get_value_name_float(i, "angulo", g_Guardias[i][gdAngulo]);
        cache_get_value_name_int(i, "mundo", g_Guardias[i][gdMundo]);
        cache_get_value_name_int(i, "interior", g_Guardias[i][gdInterior]);
        cache_get_value_name_int(i, "skin", g_Guardias[i][gdSkin]);

        g_Guardias[i][gdSerial]   = 0;
        g_Guardias[i][gdNpc]      = INVALID_PLAYER_ID;
        g_Guardias[i][gdHostil]   = false;
        g_Guardias[i][gdObjetivo] = INVALID_PLAYER_ID;

        total++;
    }

    // Igual que con la API nativa: crear los NPC de verdad se retrasa un
    // segundo tras OnGameModeInit para no chocar con la inicializacion
    // del plugin streamer.
    if (total > 0) SetTimer("Guardias_NacerCargados", 1000, false);

    printf("  [GUARDIAS] %d guardias armados cargados desde la base de datos.", total);
    return 1;
}

forward Guardias_NacerCargados();
public Guardias_NacerCargados()
{
    for (new i = 0; i < MAX_GUARDIAS; i++)
    {
        if (g_Guardias[i][gdUsado]) Guardias_Nacer(i);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  NACER / REVIVIR -- crea y configura el NPC de FCNPC de una sola vez
//  (a diferencia de la API nativa NPC_*, FCNPC_Spawn no es asincrono:
//  se puede seguir configurando en la misma linea de codigo, sin
//  esperar a ningun callback de "ya ha aparecido de verdad").
// ---------------------------------------------------------------------
stock Guardias_Nacer(indice)
{
    g_Guardias[indice][gdSerial]++;

    new nombre[24];
    format(nombre, sizeof nombre, "Guardia_%d_%d", indice, g_Guardias[indice][gdSerial]);

    new npc = FCNPC_Create(nombre);
    if (npc == INVALID_PLAYER_ID)
    {
        printf("  [GUARDIAS] No se pudo crear el NPC del guardia %d (sin cupo de bots -- revisar max_bots).", indice);
        return 0;
    }

    g_Guardias[indice][gdNpc]      = npc;
    g_Guardias[indice][gdHostil]   = true;
    g_Guardias[indice][gdObjetivo] = INVALID_PLAYER_ID;

    FCNPC_SetVirtualWorld(npc, g_Guardias[indice][gdMundo]);
    FCNPC_SetInterior(npc, g_Guardias[indice][gdInterior]);

    FCNPC_Spawn(npc, g_Guardias[indice][gdSkin], g_Guardias[indice][gdX], g_Guardias[indice][gdY], g_Guardias[indice][gdZ]);
    FCNPC_SetAngle(npc, g_Guardias[indice][gdAngulo]);

    // FCNPC_Spawn deberia dejar puesta la skin del tercer parametro, pero
    // en pruebas (05/09/2026) salieron todos con la skin por defecto
    // (CJ) -- se reafirma a mano por si el spawn de esta capa de
    // compatibilidad la resetea a 0 en algun punto interno.
    FCNPC_SetSkin(npc, g_Guardias[indice][gdSkin]);

    FCNPC_SetHealth(npc, 100.0);
    FCNPC_SetInvulnerable(npc, false);

    FCNPC_SetWeaponSkillLevel(npc, _:WEAPONSKILL_MP5, 999);
    FCNPC_SetWeaponSkillLevel(npc, _:WEAPONSKILL_MICRO_UZI, 999);

    FCNPC_SetWeapon(npc, _:WEAPON_MP5);
    FCNPC_UseInfiniteAmmo(npc, true);
    FCNPC_UseReloading(npc, false);
    FCNPC_SetAmmo(npc, 9999);
    FCNPC_SetAmmoInClip(npc, 9999);
    FCNPC_SetWeaponState(npc, _:WEAPONSTATE_MORE_BULLETS);
    FCNPC_SetWeaponAccuracy(npc, _:WEAPON_MP5, 1.0);

    return 1;
}

// ---------------------------------------------------------------------
//  COMPROBACION DE OBJETIVOS -- llamado una vez por segundo desde
//  TicSegundo (99_callbacks.inc). Nada de esto corre si no hay ningun
//  guardia con vida, asi que el coste en el caso normal (0 guardias)
//  es una unica comprobacion de "hay algo que hacer".
// ---------------------------------------------------------------------
stock Guardias_Tick()
{
    for (new i = 0; i < MAX_GUARDIAS; i++)
    {
        if (!g_Guardias[i][gdUsado]) continue;
        if (g_Guardias[i][gdNpc] == INVALID_PLAYER_ID) continue;

        // Red de seguridad por si OnNPCDeath no llega a dispararse para
        // un NPC creado via FCNPC (misma capa, motor distinto por
        // debajo de lo esperado en teoria, pero sin garantia probada) --
        // se detecta la muerte aqui tambien y se dispara el mismo
        // camino que Guardias_AlMorir.
        if (FCNPC_IsDead(g_Guardias[i][gdNpc]))
        {
            Guardias_AlMorir(g_Guardias[i][gdNpc]);
            continue;
        }

        if (!g_Guardias[i][gdHostil]) continue;

        new npc = g_Guardias[i][gdNpc];

        // Sigue valiendo el mismo objetivo de antes si sigue conectado,
        // logueado y dentro de rango -- evita reenganchar el apuntado
        // cada segundo si no hace falta.
        new objetivo = g_Guardias[i][gdObjetivo];
        if (objetivo != INVALID_PLAYER_ID && !Guardias_JugadorValido(i, objetivo))
        {
            objetivo = INVALID_PLAYER_ID;
        }

        if (objetivo == INVALID_PLAYER_ID)
        {
            new Float:mejorDistancia = GUARDIA_RANGO + 1.0;
            for (new p = 0; p < MAX_PLAYERS; p++)
            {
                if (!Guardias_JugadorValido(i, p)) continue;

                new Float:d = GetPlayerDistanceFromPoint(p, g_Guardias[i][gdX], g_Guardias[i][gdY], g_Guardias[i][gdZ]);
                if (d < mejorDistancia) { mejorDistancia = d; objetivo = p; }
            }
        }

        if (objetivo == INVALID_PLAYER_ID)
        {
            if (g_Guardias[i][gdObjetivo] != INVALID_PLAYER_ID)
            {
                FCNPC_StopAim(npc);
                FCNPC_ClearAnimations(npc);
            }
            g_Guardias[i][gdObjetivo] = INVALID_PLAYER_ID;
            continue;
        }

        // Igual que con la API nativa: no reemitir AimAtPlayer cada
        // segundo sobre el mismo objetivo si ya esta enganchado -- solo
        // al adquirir uno nuevo.
        if (objetivo == g_Guardias[i][gdObjetivo] && FCNPC_IsAimingAtPlayer(npc, objetivo)) continue;

        g_Guardias[i][gdObjetivo] = objetivo;
        FCNPC_SetWeapon(npc, _:WEAPON_MP5);
        FCNPC_AimAtPlayer(npc, objetivo, true, GUARDIA_DISPARO_DELAY_MS, true,
            0.0, 0.0, 0.8, 0.0, 0.0, 0.6, FCNPC_ENTITY_MODE_AUTO, FCNPC_ENTITY_CHECK_NONE);

        // Intento de forzar la pose a mano (05/09/2026, peticion de
        // Vanessa) -- AimAtPlayer ya calcula el disparo real, esto es
        // PURAMENTE cosmetico por encima. No hay ningun clip oficial
        // documentado de "de pie apuntando con subfusil", asi que esto
        // es un tanteo: si "UZI"/"UZI_stand" no existe o no se ve bien,
        // no pasa nada grave, el disparo real sigue intacto.
        FCNPC_ApplyAnimation(npc, "UZI", "UZI_stand", 4.1, 1, 0, 0, 0, 0);
    }
    return 1;
}

// Un jugador cuenta como objetivo valido si esta conectado, ha entrado
// (no en la pantalla de login), no es el ni tampoco es un NPC (un
// guardia jamas dispara a otro guardia ni a ningun otro bot), comparte
// mundo/interior con el guardia `indice` y esta dentro de rango.
stock bool:Guardias_JugadorValido(indice, playerid)
{
    if (playerid == INVALID_PLAYER_ID || !IsPlayerConnected(playerid)) return false;
    if (IsPlayerNPC(playerid)) return false;
    if (!Jugador[playerid][jLogueado]) return false;
    if (GetPlayerVirtualWorld(playerid) != g_Guardias[indice][gdMundo]) return false;
    if (GetPlayerInterior(playerid) != g_Guardias[indice][gdInterior]) return false;
    if (GetPlayerDistanceFromPoint(playerid, g_Guardias[indice][gdX], g_Guardias[indice][gdY], g_Guardias[indice][gdZ]) > GUARDIA_RANGO) return false;
    return true;
}

// ---------------------------------------------------------------------
//  MUERTE Y REVIVIR
// ---------------------------------------------------------------------
stock Guardias_AlMorir(npcid)
{
    for (new i = 0; i < MAX_GUARDIAS; i++)
    {
        if (!g_Guardias[i][gdUsado] || g_Guardias[i][gdNpc] != npcid) continue;

        g_Guardias[i][gdHostil]   = false;
        g_Guardias[i][gdObjetivo] = INVALID_PLAYER_ID;
        g_Guardias[i][gdNpc]      = INVALID_PLAYER_ID;

        MensajeCercanoPos(g_Guardias[i][gdX], g_Guardias[i][gdY], g_Guardias[i][gdZ],
            g_Guardias[i][gdInterior], g_Guardias[i][gdMundo], RADIO_ACCION, COLOR_ACCION,
            "* El guardia armado cae abatido.");

        SetTimerEx("Guardias_Revivir", GUARDIA_RESPAWN_MS, false, "ii", i, g_Guardias[i][gdSerial]);
        return 1;
    }
    return 0;
}

forward Guardias_Revivir(indice, serial);
public Guardias_Revivir(indice, serial)
{
    if (!g_Guardias[indice][gdUsado] || g_Guardias[indice][gdSerial] != serial) return 0;

    if (g_Guardias[indice][gdNpc] != INVALID_PLAYER_ID && FCNPC_IsValid(g_Guardias[indice][gdNpc]))
        FCNPC_Destroy(g_Guardias[indice][gdNpc]);

    Guardias_Nacer(indice);

    MensajeCercanoPos(g_Guardias[indice][gdX], g_Guardias[indice][gdY], g_Guardias[indice][gdZ],
        g_Guardias[indice][gdInterior], g_Guardias[indice][gdMundo], RADIO_ACCION, COLOR_ACCION,
        "* Un nuevo guardia armado ocupa el puesto.");
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDOS DE ADMINISTRACION
// ---------------------------------------------------------------------
CMD:crearguardia(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new slot = -1;
    for (new i = 0; i < MAX_GUARDIAS; i++)
    {
        if (!g_Guardias[i][gdUsado]) { slot = i; break; }
    }
    if (slot == -1)
    {
        Error(playerid, "No hay hueco libre (MAX_GUARDIAS lleno).");
        return 1;
    }

    new Float:px, Float:py, Float:pz, Float:pa;
    GetPlayerPos(playerid, px, py, pz);
    GetPlayerFacingAngle(playerid, pa);

    new Float:x = px + GUARDIA_DISTANCIA_SPAWN * floatsin(-pa, degrees);
    new Float:y = py + GUARDIA_DISTANCIA_SPAWN * floatcos(-pa, degrees);
    new Float:angulo = pa + 180.0;
    if (angulo >= 360.0) angulo -= 360.0;

    new interior = GetPlayerInterior(playerid);
    new mundo = GetPlayerVirtualWorld(playerid);
    new skin = g_GuardiaSkins[random(sizeof g_GuardiaSkins)];

    g_Guardias[slot][gdUsado]    = true;
    g_Guardias[slot][gdID]       = 0;
    g_Guardias[slot][gdX]        = x;
    g_Guardias[slot][gdY]        = y;
    g_Guardias[slot][gdZ]        = pz;
    g_Guardias[slot][gdAngulo]   = angulo;
    g_Guardias[slot][gdMundo]    = mundo;
    g_Guardias[slot][gdInterior] = interior;
    g_Guardias[slot][gdSkin]     = skin;
    g_Guardias[slot][gdSerial]   = 0;
    g_Guardias[slot][gdNpc]      = INVALID_PLAYER_ID;

    mysql_format(g_BD, g_GuardiaConsulta, sizeof g_GuardiaConsulta,
        "INSERT INTO guardias (x, y, z, angulo, mundo, interior, skin, creado_por) VALUES (%f, %f, %f, %f, %d, %d, %d, %d)",
        x, y, pz, angulo, mundo, interior, skin, Jugador[playerid][jID]);
    mysql_pquery(g_BD, g_GuardiaConsulta, "AlCrearGuardia", "d", slot);

    Guardias_Nacer(slot);

    Exito(playerid, "Guardia armado creado. Dispara a cualquiera que se acerque.");
    return 1;
}

forward AlCrearGuardia(slot);
public AlCrearGuardia(slot)
{
    if (slot < 0 || slot >= MAX_GUARDIAS || !g_Guardias[slot][gdUsado]) return 0;
    g_Guardias[slot][gdID] = cache_insert_id();
    return 1;
}

CMD:borrarguardia(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new indice = -1;
    new Float:mejorDistancia = 9999.0;

    for (new i = 0; i < MAX_GUARDIAS; i++)
    {
        if (!g_Guardias[i][gdUsado]) continue;
        if (GetPlayerInterior(playerid) != g_Guardias[i][gdInterior] || GetPlayerVirtualWorld(playerid) != g_Guardias[i][gdMundo]) continue;

        new Float:d = GetPlayerDistanceFromPoint(playerid, g_Guardias[i][gdX], g_Guardias[i][gdY], g_Guardias[i][gdZ]);
        if (d <= 15.0 && d < mejorDistancia) { mejorDistancia = d; indice = i; }
    }

    if (indice == -1)
    {
        Error(playerid, "No hay ningun guardia cerca.");
        return 1;
    }

    if (g_Guardias[indice][gdNpc] != INVALID_PLAYER_ID && FCNPC_IsValid(g_Guardias[indice][gdNpc]))
        FCNPC_Destroy(g_Guardias[indice][gdNpc]);

    if (g_Guardias[indice][gdID] > 0)
    {
        format(g_GuardiaConsulta, sizeof g_GuardiaConsulta, "DELETE FROM guardias WHERE id = %d", g_Guardias[indice][gdID]);
        mysql_pquery(g_BD, g_GuardiaConsulta);
    }

    new vacio[E_GUARDIA];
    g_Guardias[indice] = vacio;
    g_Guardias[indice][gdNpc] = INVALID_PLAYER_ID;
    g_Guardias[indice][gdObjetivo] = INVALID_PLAYER_ID;

    Exito(playerid, "Guardia borrado.");
    return 1;
}

// Diagnostico temporal (05/09/2026) para el problema de la postura de
// apuntado -- vuelca lo que el propio componente NPC cree que esta
// pasando con el guardia mas cercano, para compararlo contra lo que se
// ve en pantalla. No borrar sin avisar, todavia se esta usando para
// depurar en vivo con Vlad.
CMD:debugguardia(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new indice = -1;
    new Float:mejorDistancia = 9999.0;

    for (new i = 0; i < MAX_GUARDIAS; i++)
    {
        if (!g_Guardias[i][gdUsado] || g_Guardias[i][gdNpc] == INVALID_PLAYER_ID) continue;

        new Float:d = GetPlayerDistanceFromPoint(playerid, g_Guardias[i][gdX], g_Guardias[i][gdY], g_Guardias[i][gdZ]);
        if (d < mejorDistancia) { mejorDistancia = d; indice = i; }
    }

    if (indice == -1)
    {
        Error(playerid, "No hay ningun guardia creado.");
        return 1;
    }

    new npc = g_Guardias[indice][gdNpc];
    if (!FCNPC_IsValid(npc))
    {
        Error(playerid, "El NPC de ese guardia no es valido ahora mismo.");
        return 1;
    }

    format(g_Buffer, sizeof g_Buffer,
        "[GUARDIA %d] npc=%d muerto=%d arma=%d municion=%d cargador=%d estadoArma=%d",
        indice, npc, FCNPC_IsDead(npc), FCNPC_GetWeapon(npc), FCNPC_GetAmmo(npc), FCNPC_GetAmmoInClip(npc), FCNPC_GetWeaponState(npc));
    Info(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer,
        "[GUARDIA %d] hostil=%d objetivoGuardado=%d apuntando=%d disparando=%d apuntandoAlObjetivo=%d",
        indice, g_Guardias[indice][gdHostil], g_Guardias[indice][gdObjetivo], FCNPC_IsAiming(npc), FCNPC_IsShooting(npc),
        (g_Guardias[indice][gdObjetivo] != INVALID_PLAYER_ID) ? FCNPC_IsAimingAtPlayer(npc, g_Guardias[indice][gdObjetivo]) : -1);
    Info(playerid, g_Buffer);
    return 1;
}

// Prueba de control TEMPORAL (05/09/2026, peticion de Vanessa) -- te arma
// a TI, jugador real, con la MP5. Sirve para aislar si este gamemode en
// concreto sabe representar la postura de apuntar cuando quien la lleva
// es un jugador de verdad, no un NPC. Si TU tampoco la sostienes bien,
// el problema no es de NPCs -- hay algo en el propio gamemode (algun
// ResetPlayerWeapons/OnPlayerUpdate corriendo por detras) que lo impide
// para cualquiera. Borrar este comando en cuanto se tenga la respuesta.
CMD:pruebapose(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    GivePlayerWeapon(playerid, WEAPON_MP5, 9999);
    SetPlayerArmedWeapon(playerid, WEAPON_MP5);

    Info(playerid, "MP5 dada y armada. Mirate en otro cliente/espejo: si TU la sostienes bien, el problema es solo de los NPC.");
    return 1;
}
