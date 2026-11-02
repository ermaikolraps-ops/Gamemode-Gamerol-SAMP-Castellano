/* =====================================================================
 *  39_atracos.inc
 *  ---------------------------------------------------------------------
 *  Atracos a locales. Portado del filterscript de robos del catalogo de
 *  scripts antiguos, pero con las coordenadas fuera del codigo: un
 *  admin crea cada local de pie en el sitio con /crearatraco (mismo
 *  patron que /creardropeo, 22_dropeo.inc), no hay nada puesto a mano
 *  en este archivo.
 *
 *  MECANICA: /atracar cerca de un local libre empieza una cuenta atras
 *  de ATRACO_DURACION_SEGUNDOS. Si te alejas, te subes a un vehiculo o
 *  te desconectas, se cancela sin premio. Si aguantas hasta el final,
 *  te llevas un botin aleatorio entre el minimo y el maximo de ese
 *  local, y el local queda en cooldown para que no se pueda repetir en
 *  cadena.
 *
 *  CONSECUENCIA: al terminar se avisa a la Policia Nacional con el
 *  mismo sistema que ya usan cultivo, NFS y robo de coches
 *  (Policia_CrearAviso, 37_policia.inc) -- no se toca jBuscado aqui, es
 *  el agente quien te para en la calle y te imputa el codigo si te
 *  pilla.
 * ===================================================================== */

enum E_ATRACO
{
    bool:atUsado,
    atID,                                    // fila en la tabla `atracos`
    atNombre[40],
    Float:atX, Float:atY, Float:atZ,
    atInterior, atMundo,
    atPremioMin, atPremioMax,
    atCooldownHasta,                         // gettime() hasta el que no se puede repetir, 0 = libre
    bool:atOcupado,                          // alguien lo esta atracando ahora mismo
    STREAMER_TAG_3D_TEXT_LABEL:atEtiqueta,
    STREAMER_TAG_OBJECT:atMarcador           // icono "!" rojo, visible antes de llegar al cartel de texto
}
new g_Atracos[MAX_ATRACOS][E_ATRACO];

static bool:g_AtracoActivo[MAX_PLAYERS];
static g_AtracoIndice[MAX_PLAYERS];
static g_AtracoRestante[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  CARGA
// ---------------------------------------------------------------------
stock Atracos_CargarTodos()
{
    mysql_pquery(g_BD, "SELECT * FROM atracos ORDER BY id ASC", "AlCargarAtracos");
    return 1;
}

forward AlCargarAtracos();
public AlCargarAtracos()
{
    new filas;
    cache_get_row_count(filas);

    for (new i = 0; i < filas && i < MAX_ATRACOS; i++)
    {
        g_Atracos[i][atUsado] = true;
        cache_get_value_name_int(i, "id", g_Atracos[i][atID]);
        cache_get_value_name(i, "nombre", g_Atracos[i][atNombre], 40);
        cache_get_value_name_float(i, "x", g_Atracos[i][atX]);
        cache_get_value_name_float(i, "y", g_Atracos[i][atY]);
        cache_get_value_name_float(i, "z", g_Atracos[i][atZ]);
        cache_get_value_name_int(i, "interior", g_Atracos[i][atInterior]);
        cache_get_value_name_int(i, "mundo", g_Atracos[i][atMundo]);
        cache_get_value_name_int(i, "premio_min", g_Atracos[i][atPremioMin]);
        cache_get_value_name_int(i, "premio_max", g_Atracos[i][atPremioMax]);

        g_Atracos[i][atCooldownHasta] = 0;
        g_Atracos[i][atOcupado] = false;
        g_Atracos[i][atEtiqueta] = STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID;
        g_Atracos[i][atMarcador] = STREAMER_TAG_OBJECT:INVALID_STREAMER_ID;

        Atracos_CrearCartel(i);
    }

    printf("  [ATRACOS] %d locales cargados desde la base de datos.", filas);
    return 1;
}

// ---------------------------------------------------------------------
//  CARTEL 3D -- muestra si el local esta libre o en cooldown.
// ---------------------------------------------------------------------
stock Atracos_CrearCartel(indice)
{
    if (IsValidDynamic3DTextLabel(g_Atracos[indice][atEtiqueta])) DestroyDynamic3DTextLabel(g_Atracos[indice][atEtiqueta]);

    g_Atracos[indice][atEtiqueta] = CreateDynamic3DTextLabel("_", COLOR_BLANCO,
        g_Atracos[indice][atX], g_Atracos[indice][atY], g_Atracos[indice][atZ] + 0.9, 8.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, g_Atracos[indice][atMundo], g_Atracos[indice][atInterior], -1, 20.0);

    // Icono "!" rojo (04/09/2026) -- mismo patron que Federico_CrearMarcador
    // (43_federico.inc) y Tuning_CrearMarcadores (14_tuning.inc): objeto
    // plano (19329) con texto como textura. El signo de exclamacion en
    // rojo es la equivalencia mas cercana al icono "Alarma"/aviso que
    // traia el filterscript de referencia (p_markers.txt) sin depender
    // de acertar un ID de icono HUD concreto. A diferencia del cartel de
    // texto (que ya cambia de color/mensaje segun libre o en cooldown),
    // este icono es fijo -- solo marca "aqui hay un comercio atracable",
    // visible de mas lejos que el cartel (radio de dibujo mayor).
    if (IsValidDynamicObject(g_Atracos[indice][atMarcador])) DestroyDynamicObject(g_Atracos[indice][atMarcador]);

    g_Atracos[indice][atMarcador] = CreateDynamicObject(19329,
        g_Atracos[indice][atX], g_Atracos[indice][atY], g_Atracos[indice][atZ] + 1.6, 0.0, 0.0, 0.0,
        g_Atracos[indice][atMundo], g_Atracos[indice][atInterior], -1, 35.0);
    SetDynamicObjectMaterialText(g_Atracos[indice][atMarcador], 0, "!",
        OBJECT_MATERIAL_SIZE_32x32, "Impact", 32, 1, 0xFFC0392B, 0x00000000, 1);

    Atracos_ActualizarCartel(indice);
    return 1;
}

stock Atracos_ActualizarCartel(indice)
{
    if (!IsValidDynamic3DTextLabel(g_Atracos[indice][atEtiqueta])) return 0;

    if (g_Atracos[indice][atCooldownHasta] > gettime())
    {
        new restante = g_Atracos[indice][atCooldownHasta] - gettime();
        format(g_Buffer, sizeof g_Buffer, "%s\n{E05A5A}Vigilado, vuelve en %d min", g_Atracos[indice][atNombre], (restante / 60) + 1);
    }
    else
    {
        format(g_Buffer, sizeof g_Buffer, "%s\n{53C168}Escribe {E8B33A}/atracar", g_Atracos[indice][atNombre]);
    }

    UpdateDynamic3DTextLabelText(g_Atracos[indice][atEtiqueta], COLOR_BLANCO, g_Buffer);
    return 1;
}

stock Atracos_RefrescarCarteles()
{
    for (new i = 0; i < MAX_ATRACOS; i++)
    {
        if (!g_Atracos[i][atUsado]) continue;
        Atracos_ActualizarCartel(i);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  BUSQUEDA
// ---------------------------------------------------------------------
stock Atracos_BuscarCercano(playerid)
{
    for (new i = 0; i < MAX_ATRACOS; i++)
    {
        if (!g_Atracos[i][atUsado]) continue;
        if (GetPlayerInterior(playerid) != g_Atracos[i][atInterior] || GetPlayerVirtualWorld(playerid) != g_Atracos[i][atMundo]) continue;
        if (EnPunto(playerid, ATRACO_RADIO, g_Atracos[i][atX], g_Atracos[i][atY], g_Atracos[i][atZ])) return i;
    }
    return -1;
}

// ---------------------------------------------------------------------
//  SESION DEL JUGADOR
// ---------------------------------------------------------------------
stock Atracos_Reiniciar(playerid)
{
    g_AtracoActivo[playerid] = false;
    g_AtracoIndice[playerid] = -1;
    g_AtracoRestante[playerid] = 0;
    return 1;
}

stock Atracos_Cancelar(playerid, bool:avisar)
{
    if (!g_AtracoActivo[playerid]) return 0;

    new indice = g_AtracoIndice[playerid];
    if (indice != -1 && g_Atracos[indice][atUsado]) g_Atracos[indice][atOcupado] = false;

    g_AtracoActivo[playerid] = false;
    g_AtracoIndice[playerid] = -1;
    g_AtracoRestante[playerid] = 0;

    if (avisar)
    {
        Error(playerid, "Has dejado el atraco a medias.");
        GameTextForPlayer(playerid, "~r~Atraco cancelado", 2000, 4);
    }
    return 1;
}

// Llamado una vez por segundo por jugador desde TicSegundo (99_callbacks.inc).
stock Atracos_Segundo(playerid)
{
    if (!g_AtracoActivo[playerid]) return 0;

    new indice = g_AtracoIndice[playerid];
    if (indice == -1 || !g_Atracos[indice][atUsado])
    {
        Atracos_Cancelar(playerid, true);
        return 0;
    }

    if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER || GetPlayerState(playerid) == PLAYER_STATE_PASSENGER)
    {
        Atracos_Cancelar(playerid, true);
        return 0;
    }

    if (GetPlayerInterior(playerid) != g_Atracos[indice][atInterior]
        || GetPlayerVirtualWorld(playerid) != g_Atracos[indice][atMundo]
        || !EnPunto(playerid, ATRACO_RADIO, g_Atracos[indice][atX], g_Atracos[indice][atY], g_Atracos[indice][atZ]))
    {
        Atracos_Cancelar(playerid, true);
        return 0;
    }

    g_AtracoRestante[playerid]--;
    if (g_AtracoRestante[playerid] <= 0)
    {
        Atracos_Completar(playerid);
        return 1;
    }

    format(g_Buffer, sizeof g_Buffer, "~r~Atracando...~n~~w~%d s", g_AtracoRestante[playerid]);
    GameTextForPlayer(playerid, g_Buffer, 1100, 4);
    return 1;
}

stock Atracos_Completar(playerid)
{
    new indice = g_AtracoIndice[playerid];

    g_AtracoActivo[playerid] = false;
    g_AtracoIndice[playerid] = -1;
    g_AtracoRestante[playerid] = 0;
    g_Atracos[indice][atOcupado] = false;

    new premioMin = g_Atracos[indice][atPremioMin];
    new premioMax = g_Atracos[indice][atPremioMax];
    new premio = premioMin + random(premioMax - premioMin + 1);
    DarDinero(playerid, premio);

    g_Atracos[indice][atCooldownHasta] = gettime() + (ATRACO_COOLDOWN_MINUTOS * 60);
    Atracos_ActualizarCartel(indice);

    new dinero[24];
    FormatearDinero(premio, dinero, sizeof dinero);
    format(g_Buffer, sizeof g_Buffer, "Atraco a %s completado. Te llevas %s.", g_Atracos[indice][atNombre], dinero);
    Exito(playerid, g_Buffer);
    GameTextForPlayer(playerid, "~g~Atraco completado", 3000, 4);

    Policia_CrearAviso(AVISO_ATRACO, playerid,
        g_Atracos[indice][atX], g_Atracos[indice][atY], g_Atracos[indice][atZ],
        g_Atracos[indice][atInterior], g_Atracos[indice][atMundo]);

    Teletipo_RegistrarAtraco(g_Atracos[indice][atNombre]); // 62_teletipo.inc -- no depende del 65% de Policia_CrearAviso

    format(g_Buffer, sizeof g_Buffer, "ATRACO  cuenta=%d  local=%s  premio=%d",
        Jugador[playerid][jID], g_Atracos[indice][atNombre], premio);
    EscribirLog("atracos", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDO DE JUGADOR
// ---------------------------------------------------------------------
CMD:atracar(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (g_AtracoActivo[playerid])
    {
        Error(playerid, "Ya estas en mitad de un atraco.");
        return 1;
    }

    if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER || GetPlayerState(playerid) == PLAYER_STATE_PASSENGER)
    {
        Error(playerid, "Tienes que bajarte del vehiculo para atracar.");
        return 1;
    }

    new indice = Atracos_BuscarCercano(playerid);
    if (indice == -1)
    {
        Error(playerid, "No hay ningun local que se pueda atracar cerca.");
        return 1;
    }

    if (g_Atracos[indice][atOcupado])
    {
        Error(playerid, "Ese local ya lo esta atracando otra persona ahora mismo.");
        return 1;
    }

    if (g_Atracos[indice][atCooldownHasta] > gettime())
    {
        new restante = g_Atracos[indice][atCooldownHasta] - gettime();
        format(g_Buffer, sizeof g_Buffer, "Ese local esta vigilado todavia. Vuelve en %d minutos.", (restante / 60) + 1);
        Error(playerid, g_Buffer);
        return 1;
    }

    g_AtracoActivo[playerid] = true;
    g_AtracoIndice[playerid] = indice;
    g_AtracoRestante[playerid] = ATRACO_DURACION_SEGUNDOS;
    g_Atracos[indice][atOcupado] = true;

    Info(playerid, "Has empezado el atraco. Quedate quieto y no te subas a ningun vehiculo.");
    GameTextForPlayer(playerid, "~r~Atracando...", 2000, 4);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN
// ---------------------------------------------------------------------
CMD:crearatraco(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new nombre[40], premioMin, premioMax;
    if (sscanf(params, "s[40]dd", nombre, premioMin, premioMax))
    {
        Uso(playerid, "/crearatraco <nombre> <premio minimo> <premio maximo>");
        return 1;
    }

    if (premioMin <= 0 || premioMax <= premioMin)
    {
        Error(playerid, "El premio maximo tiene que ser mayor que el minimo, y ambos positivos.");
        return 1;
    }

    new slot = -1;
    for (new i = 0; i < MAX_ATRACOS; i++)
    {
        if (!g_Atracos[i][atUsado]) { slot = i; break; }
    }
    if (slot == -1)
    {
        Error(playerid, "No hay hueco libre (MAX_ATRACOS lleno).");
        return 1;
    }

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    new interior = GetPlayerInterior(playerid);
    new mundo = GetPlayerVirtualWorld(playerid);

    new consulta[300];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO atracos (nombre, x, y, z, interior, mundo, premio_min, premio_max, creado_por) VALUES ('%e', %f, %f, %f, %d, %d, %d, %d, %d)",
        nombre, x, y, z, interior, mundo, premioMin, premioMax, Jugador[playerid][jID]);
    mysql_pquery(g_BD, consulta, "AlCrearAtraco", "d", slot);

    g_Atracos[slot][atUsado] = true;
    format(g_Atracos[slot][atNombre], 40, "%s", nombre);
    g_Atracos[slot][atX] = x;
    g_Atracos[slot][atY] = y;
    g_Atracos[slot][atZ] = z;
    g_Atracos[slot][atInterior] = interior;
    g_Atracos[slot][atMundo] = mundo;
    g_Atracos[slot][atPremioMin] = premioMin;
    g_Atracos[slot][atPremioMax] = premioMax;
    g_Atracos[slot][atCooldownHasta] = 0;
    g_Atracos[slot][atOcupado] = false;
    g_Atracos[slot][atEtiqueta] = STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID;

    Atracos_CrearCartel(slot);

    format(g_Buffer, sizeof g_Buffer, "Local de atraco creado: %s (premio %d-%d).", nombre, premioMin, premioMax);
    Exito(playerid, g_Buffer);
    return 1;
}

forward AlCrearAtraco(slot);
public AlCrearAtraco(slot)
{
    if (slot < 0 || slot >= MAX_ATRACOS || !g_Atracos[slot][atUsado]) return 0;
    g_Atracos[slot][atID] = cache_insert_id();
    return 1;
}

CMD:borraratraco(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new indice = -1;
    new Float:mejorDistancia = 9999.0;
    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    for (new i = 0; i < MAX_ATRACOS; i++)
    {
        if (!g_Atracos[i][atUsado]) continue;
        if (GetPlayerInterior(playerid) != g_Atracos[i][atInterior] || GetPlayerVirtualWorld(playerid) != g_Atracos[i][atMundo]) continue;

        new Float:d = GetPlayerDistanceFromPoint(playerid, g_Atracos[i][atX], g_Atracos[i][atY], g_Atracos[i][atZ]);
        if (d <= 10.0 && d < mejorDistancia) { mejorDistancia = d; indice = i; }
    }

    if (indice == -1)
    {
        Error(playerid, "No hay ningun local de atraco cerca.");
        return 1;
    }

    if (g_Atracos[indice][atID] > 0)
    {
        new consulta[64];
        format(consulta, sizeof consulta, "DELETE FROM atracos WHERE id = %d", g_Atracos[indice][atID]);
        mysql_pquery(g_BD, consulta);
    }

    if (IsValidDynamic3DTextLabel(g_Atracos[indice][atEtiqueta])) DestroyDynamic3DTextLabel(g_Atracos[indice][atEtiqueta]);
    if (IsValidDynamicObject(g_Atracos[indice][atMarcador])) DestroyDynamicObject(g_Atracos[indice][atMarcador]);

    format(g_Buffer, sizeof g_Buffer, "Local de atraco borrado: %s.", g_Atracos[indice][atNombre]);
    Exito(playerid, g_Buffer);

    new vacio[E_ATRACO];
    g_Atracos[indice] = vacio;
    return 1;
}
