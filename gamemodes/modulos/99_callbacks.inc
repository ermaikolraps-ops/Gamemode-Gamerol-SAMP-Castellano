/* =====================================================================
 *  99_callbacks.inc
 *  ---------------------------------------------------------------------
 *  Todos los callbacks de open.mp viven aqui y en ningun otro sitio.
 *
 *  Cada callback se limita a repartir el trabajo entre los modulos. Si
 *  quieres saber que pasa cuando alguien conecta, muere o escribe algo,
 *  este es el mapa completo.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  ARRANQUE Y CIERRE
// ---------------------------------------------------------------------
public OnGameModeInit()
{
    print(" ");
    print("  --------------------------------------------------");
    print("   " SERVIDOR_NOMBRE);
    print("   Los Santos, 1992");
    print("  --------------------------------------------------");

    // 1. Base de datos. Si falla, el servidor no arranca.
    if (!BD_Conectar()) return 0;
    BD_CargarConfiguracion();
    BD_RefrescarSemana();

    // 2. Ajustes generales del mundo.
    SetGameModeText("Gamerol 2026");
    ShowPlayerMarkers(PLAYER_MARKERS_MODE_OFF);
    ShowNameTags(true);
    SetNameTagDrawDistance(25.0);
    DisableInteriorEnterExits();
    EnableStuntBonusForAll(false);
    ManualVehicleEngineAndLights();
    // Hora real de Europe/Madrid + clima dinamico (60_climahora.inc,
    // 10/09/2026) -- sustituye a los SetWeather(2)/SetWorldTime(12)
    // fijos de siempre, peticion expresa de Vlad.
    ClimaHora_Inicializar();

    // 3. Interfaces.
    Login_CrearTextdraws();
    Veh_HUD_Iniciar();
    GPS_CrearTextdraw();
    Federico_CrearTextdraws();

    // 4. Contenido del mundo.
    Trabajo_CrearMarcadores();
    Faccion_CrearMarcadores();
    Federico_CrearMarcador();
    Banco_CrearMarcadores();
    HospitalInterior_CrearObjetos();
    Hospital_CrearMarcadores();
    PuntosInteres_CrearMarcadores();
    Tuning_Iniciar();          // antes de Veh_CargarTodos: deja g_TuningPintura a -1
    Tuning_CrearMarcadores();
    Veh_CargarTodos();
    Veh_CrearCartelesGasolinera();
    Veh_CrearCartelesPetrolpumpsLS();
    Casa_CargarTodas();
    Coord_CargarTodas();
    VendedorArmas_Iniciar();
    NFS_Iniciar();
    Dropeo_Iniciar();
    Tienda_Iniciar();
    Maria_Iniciar();
    Maria_CargarTodas();
    MariaVendedor_Iniciar();
    Negocio_Iniciar();
    Negocio_CargarTodas();
    Policia_Iniciar();
    Cancelas_CargarTodas();
    Atracos_CargarTodos();
    Comida_Iniciar();
    Bienvenida_Iniciar();
    Guia_Iniciar();
    GuiaVeh_Iniciar();
    Empenos_Iniciar();
    Pesca_Iniciar();
    Perrito_Iniciar();
    Arreglarcoche_Iniciar();
    Motel_Iniciar();
    Motel_CargarHabitaciones();
    Cronos_Iniciar();
    AirDrop_Iniciar();

    // 5. Temporizadores.
    SetTimer("TicSegundo",   TIEMPO_SEGUNDO,   true);
    SetTimer("TicMedioMinuto", 30000,          true);
    SetTimer("TicMinuto",    TIEMPO_MINUTO,    true);
    SetTimer("TicHora",      3600000,          true);
    SetTimer("TicGuardado",  TIEMPO_GUARDADO,  true);
    SetTimer("Tactil_Timer", 500,              true);
    SetTimer("Drift_Tick",   DRIFT_INTERVALO_MS, true);

    print("  --------------------------------------------------");
    print("   Servidor listo. Buen rol.");
    print("  --------------------------------------------------");
    print(" ");
    return 1;
}

public OnGameModeExit()
{
    print("  [SERVIDOR] Guardando todo antes de cerrar...");

    BD_GuardarTodos();
    Veh_GuardarTodos();

    BD_Desconectar();

    print("  [SERVIDOR] Cerrado correctamente.");
    return 1;
}

// ---------------------------------------------------------------------
//  CONEXION
// ---------------------------------------------------------------------
public OnPlayerConnect(playerid)
{
    // Los guardias armados (46_guardias.inc) son NPC de la API nativa
    // NPC_*, pero por debajo se conectan como si fueran un jugador mas
    // y disparan este mismo callback. Sin este corte, la validacion de
    // nombre "Nombre_Apellido" de mas abajo los expulsaba a los 900ms
    // de conectar -- por eso se unian y se iban solos casi al momento.
    if (IsPlayerNPC(playerid)) return 1;

    ReiniciarJugador(playerid);
    Login_ReiniciarTextdraws(playerid);
    Admin_CerrarReportes(playerid);
    Banco_ReiniciarEstado(playerid);

    new nombre[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, nombre, sizeof nombre);

    // Exigimos nombres de rol: Nombre_Apellido.
    if (!EsNombreRP(nombre))
    {
        SendClientMessage(playerid, COLOR_ROJO_FUERTE, " ");
        SendClientMessage(playerid, COLOR_ROJO_FUERTE, "  TU NOMBRE NO ES VALIDO PARA ESTE SERVIDOR");
        SendClientMessage(playerid, COLOR_BLANCO,      "  Aqui se juega con nombres de personaje reales,");
        SendClientMessage(playerid, COLOR_BLANCO,      "  con el formato  Nombre_Apellido");
        SendClientMessage(playerid, COLOR_GRIS,        "  Por ejemplo: Juan_Perez, Maria_Lopez, Antonio_Ruiz");
        SendClientMessage(playerid, COLOR_GRIS,        "  Cambialo en el menu del SA-MP y vuelve a entrar.");
        SendClientMessage(playerid, COLOR_ROJO_FUERTE, " ");

        SetTimerEx("ExpulsarRetardado", 900, false, "d", playerid);
        return 1;
    }

    format(Jugador[playerid][jNombre], MAX_PLAYER_NAME + 1, "%s", nombre);

    SetPlayerColor(playerid, COLOR_BLANCO);
    Veh_HUD_Crear(playerid);
    GPS_CrearTextdrawJugador(playerid);
    HambreSed_CrearTextdrawJugador(playerid);
    VendedorArmas_OnConnect(playerid);
    NFS_OnConnect(playerid);
    MariaVendedor_OnConnect(playerid);
    Porro_OnConectar(playerid);
    Negocio_OnConectar(playerid);
    Dropeo_OnConectar(playerid);
    Maletero_OnConectar(playerid);
    Banco_OnConectar(playerid);
    Tuning_OnConectar(playerid);
    Comida_OnConectar(playerid);
    Tienda_OnConectar(playerid);
    Tactil_OnConectar(playerid);
    Drift_Reiniciar(playerid);
    Atracos_Reiniciar(playerid);
    Bienvenida_OnConnect(playerid);
    Guia_OnConnect(playerid);
    GuiaVeh_OnConnect(playerid);
    Empenos_OnConectar(playerid);
    Pesca_OnConectar(playerid);
    Veh_OnConectarPetrolpumpsLS(playerid);
    Perrito_OnConectar(playerid);
    Arreglarcoche_OnConectar(playerid);
    Motel_OnConectar(playerid);
    Federico_OnConectar(playerid);
    Ganancias_OnConectar(playerid);
    Consejos_OnConectar(playerid);
    Medicos_OnConectar(playerid);
    RoboExpress_OnConectar(playerid);
    Ascensos_OnConectar(playerid);
    FedJugadores_OnConectar(playerid);
    Hospital_OnConectar(playerid);
    Cronos_OnConectar(playerid);
    AirDrop_OnConectar(playerid);

    // Icono del hospital quitado el 11/09/2026 -- peticion expresa de
    // Vlad de limpiar todos los iconos del mapa (la gente usa /gps).

    Login_Preparar(playerid);
    BD_ComprobarCuenta(playerid);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    if (Jugador[playerid][jLogueado])
    {
        // Si estaba caido, se resuelve ANTES de guardar -- si no, se
        // guardaria su posicion de caida y su vida baja tal cual, y
        // reconectaria de pie en ese mismo sitio sin haber pagado nada.
        Medicos_OnDisconnect(playerid);
        Policia_OnDisconnect(playerid);
        RoboExpress_OnDisconnect(playerid);
        FedJugadores_OnDesconectar(playerid);
        Cronos_OnDesconectar(playerid);

        BD_GuardarCuenta(playerid);
        g_Conectados--;
        if (g_Conectados < 0) g_Conectados = 0;

        new nombre[MAX_PLAYER_NAME + 1];
        NombreRP(playerid, nombre, sizeof nombre);

        static const motivos[][] = { "perdio la conexion", "salio del servidor", "fue expulsado" };
        if (reason < 0 || reason > 2) reason = 1;

        format(g_Buffer, sizeof g_Buffer, "%s %s.", nombre, motivos[reason]);
        EscribirLog("conexiones", g_Buffer);
    }

    Admin_CerrarReportes(playerid);
    Login_BorrarTextdrawsJugador(playerid);
    VendedorArmas_OnDisconnect(playerid);
    NFS_OnDisconnect(playerid);
    MariaVendedor_OnDisconnect(playerid);
    Bienvenida_OnDisconnect(playerid);
    Porro_OnDesconectar(playerid);
    Dropeo_OnDesconectar(playerid);
    Maletero_OnDesconectar(playerid);
    Tactil_OnDesconectar(playerid);
    Drift_Reiniciar(playerid);
    Atracos_Cancelar(playerid, false);
    Pesca_OnDesconectar(playerid);
    Inv_Limpiar(playerid);
    Toys_Limpiar(playerid);
    ReiniciarJugador(playerid);
    return 1;
}

// La seleccion de personaje de SA-MP no se usa: todo pasa por la
// pantalla de entrada propia. Si devolvemos 1 aqui, el cliente muestra
// igualmente la pantalla nativa de flechitas + "Spawn" -- pasa sobre
// todo tras morir, cuando SA-MP pide clase para el auto-respawn. Para
// que no aparezca nunca, spawneamos nosotros mismos desde aqui.
public OnPlayerRequestClass(playerid, classid)
{
    #pragma unused classid
    if (!Jugador[playerid][jLogueado])
    {
        TogglePlayerSpectating(playerid, true);
        return 0;
    }
    // Aqui es donde SA-MP acaba de pisar los datos de reaparicion con
    // los de la clase (todo ceros, skin 0 = CJ, porque este gamemode no
    // registra ninguna clase). Se reescriben con el aspecto y el sitio
    // reales ANTES del SpawnPlayer, asi que el jugador renace ya con su
    // piel puesta: no hay ni un fotograma de CJ. Ver Spawn_RefrescarDatos
    // (03_utiles.inc).
    if (!Login_EnCreacion(playerid)) Spawn_RefrescarDatos(playerid);

    SpawnPlayer(playerid);
    return 0;
}

public OnPlayerSpawn(playerid)
{
    // Creando el personaje: todavia no hay cuenta, pero el jugador si
    // tiene que aparecer en el mundo para verse la piel.
    if (Login_EnCreacion(playerid)) return 1;

    if (!Jugador[playerid][jLogueado])
    {
        TogglePlayerSpectating(playerid, true);
        return 0;
    }

    AplicarDatosJugador(playerid);
    Jugador[playerid][jSpawneado] = true;

    if (Jugador[playerid][jCarcelSegundos] > 0)
    {
        SetPlayerPos(playerid, CARCEL_X, CARCEL_Y, CARCEL_Z);
        SetPlayerInterior(playerid, CARCEL_INTERIOR);
        SetPlayerVirtualWorld(playerid, 1000 + playerid);
        Aviso(playerid, "Sigues cumpliendo carcel administrativa.");
    }

    Policia_OnSpawn(playerid);

    if (Jugador[playerid][jCongelado]) TogglePlayerControllable(playerid, false);

    // Color del nametag segun la faccion.
    if (FaccionValida(Jugador[playerid][jFaccion])) SetPlayerColor(playerid, Faccion_Color(Jugador[playerid][jFaccion]));
    else                                            SetPlayerColor(playerid, COLOR_BLANCO);

    // Abre o cierra cada vehiculo del mapa segun su trabajo y su faccion.
    Veh_ActualizarCierreJugador(playerid);

    VendedorArmas_OnSpawn(playerid);
    Dropeo_OnSpawn(playerid);
    Toys_OnSpawn(playerid);
    return 1;
}

public OnPlayerDeath(playerid, killerid, WEAPON:reason)
{
    #pragma unused reason
    if (!Jugador[playerid][jLogueado]) return 1;

    MariaVendedor_OnDeath(playerid);
    Porro_OnMuerte(playerid);
    Drift_Reiniciar(playerid);
    Atracos_Cancelar(playerid, false);
    Pesca_OnMuerte(playerid);
    Cronos_LimpiarEstado(playerid);


    // Aviso a la Policia Nacional SOLO si te mato otro jugador (no
    // caidas, hambre/sed, NPCs, etc.) -- retirado el sistema de heridos/
    // medicos automatico, peticion expresa de Vlad, 07/09/2026. El
    // trabajo de Medicos ya no tiene relacion con esto, ver 36_medicos.inc.
    if (killerid != INVALID_PLAYER_ID && killerid != playerid && IsPlayerConnected(killerid))
    {
        new nombreVictima[MAX_PLAYER_NAME + 1], nombreAsesino[MAX_PLAYER_NAME + 1], aviso[144];
        NombreRP(playerid, nombreVictima, sizeof nombreVictima);
        NombreRP(killerid, nombreAsesino, sizeof nombreAsesino);
        format(aviso, sizeof aviso, "%s fue herido de gravedad por %s y fue enviado al hospital.", nombreVictima, nombreAsesino);
        Faccion_Mensaje(POLICIA_FACCION, Faccion_Color(POLICIA_FACCION), aviso);
    }

    // Reaparicion tras morir: en el hospital, peticion expresa de Vlad
    // 11/09/2026 -- ya no en la posicion donde murio. Sigue sin perder
    // armas ni inventario, ni hay estado de "herido". Movido el
    // 13/09/2026 DENTRO del interior nuevo del hospital
    // (HOSPITAL_RESPAWN_X/Y/Z/A, 64_hospitalinterior.inc; antes
    // HOSPITAL_X/Y/Z/A, en la calle) -- pedido expreso de Vlad.
    //
    // Se escribe DIRECTAMENTE en jPosX/Y/Z/A/jInterior/jMundo -- es la
    // UNICA fuente que de verdad usan tanto Spawn_RefrescarDatos
    // (llamado por SA-MP en OnPlayerRequestClass para el auto-respawn)
    // como AplicarDatosJugador (OnPlayerSpawn) para posicionar al
    // jugador. Un SetSpawnInfo aparte (lo que hacia antes esto) no
    // servia de nada: SA-MP siempre vuelve a pisarlo en la seleccion de
    // clase del auto-respawn, que llama a Spawn_RefrescarDatos con la
    // posicion guardada de ANTES de morir -- bug real confirmado por
    // Vlad, 12/09/2026 (seguia reapareciendo donde moria).
    Jugador[playerid][jPosX]     = HOSPITAL_RESPAWN_X;
    Jugador[playerid][jPosY]     = HOSPITAL_RESPAWN_Y;
    Jugador[playerid][jPosZ]     = HOSPITAL_RESPAWN_Z;
    Jugador[playerid][jPosA]     = HOSPITAL_RESPAWN_A;
    Jugador[playerid][jInterior] = 0;
    Jugador[playerid][jMundo]    = MUNDO_HOSPITAL_INTERIOR;
    return 1;
}

// Invulnerabilidad mientras esta caido: SA-MP ya ha aplicado el dano
// cuando este callback se dispara, asi que "cancelar" con return 0 no
// basta -- hay que devolver la vida que se acaba de perder.
public OnPlayerTakeDamage(playerid, issuerid, Float:amount, WEAPON:weaponid, bodypart)
{
    #pragma unused issuerid, weaponid, bodypart
    if (Jugador[playerid][jHerido])
    {
        new Float:vida;
        GetPlayerHealth(playerid, vida);
        SetPlayerHealth(playerid, vida + amount);
        return 0;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  TECLAS
// ---------------------------------------------------------------------
public OnPlayerKeyStateChange(playerid, KEY:newkeys, KEY:oldkeys)
{
    // Click (fuego) recien pulsado: si lleva un porro liado en la mano,
    // es una calada. Si no, no hace nada -- no toca el disparo real.
    if ((newkeys & KEY_FIRE) && !(oldkeys & KEY_FIRE))
    {
        Porro_DarCalada(playerid);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  CHAT Y COMANDOS
// ---------------------------------------------------------------------
public OnPlayerText(playerid, text[])
{
    if (!Jugador[playerid][jLogueado])
    {
        Error(playerid, "Tienes que iniciar sesion para poder hablar.");
        return 0;
    }

    Jugador[playerid][jUltimaActividad] = GetTickCount();
    return Chat_Hablar(playerid, text);
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    Jugador[playerid][jUltimaActividad] = GetTickCount();

    // Auditoria de comandos -- peticion expresa de Vlad, 11/09/2026:
    // TODO lo que un jugador manda que empiece por "/" queda registrado
    // aqui, exista el comando o no, ANTES de procesarlo. logs/comandos.log
    new nombreConexion[MAX_PLAYER_NAME + 1], lineaComando[300];
    GetPlayerName(playerid, nombreConexion, sizeof nombreConexion);
    if (Jugador[playerid][jLogueado])
    {
        format(lineaComando, sizeof lineaComando, "%s (cuenta=%d): %s", nombreConexion, Jugador[playerid][jID], cmdtext);
    }
    else
    {
        format(lineaComando, sizeof lineaComando, "%s (sin sesion): %s", nombreConexion, cmdtext);
    }
    EscribirLog("comandos", lineaComando);

    new resultado = ProcesarComando(playerid, cmdtext);

    if (resultado == -1) return 1;                  // Bloqueado por el antispam.

    if (resultado == 0)
    {
        Error(playerid, "Ese comando no existe. Escribe /ayuda para ver la lista.");
        return 1;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  DIALOGOS
//  Cada modulo intenta gestionar el dialogo; el primero que lo reconoce
//  devuelve 1 y la cadena se detiene.
// ---------------------------------------------------------------------
public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    Jugador[playerid][jUltimaActividad] = GetTickCount();

    if (dialogid == DIALOGO_AYUDA)
    {
        if (response) Ayuda_MostrarSeccion(playerid, listitem);
        return 1;
    }
    if (dialogid == DIALOGO_AYUDA_SECCION)
    {
        new vacio[2];
        vacio[0] = 1;
        vacio[1] = EOS;
        if (response) return cmd_ayuda(playerid, vacio);
        return 1;
    }
    if (dialogid == DIALOGO_SISTEMAS)
    {
        if (!response)
        {
            new vacio[2];
            vacio[0] = 1;
            vacio[1] = EOS;
            return cmd_ayuda(playerid, vacio);
        }
        return Sistemas_MostrarDetalle(playerid, listitem);
    }
    if (dialogid == DIALOGO_SISTEMAS_DETALLE)
    {
        if (response) return Sistemas_MostrarLista(playerid);
        return 1;
    }
    if (dialogid == DIALOGO_ACTIVIDADES)
    {
        if (!response)
        {
            new vacio[2];
            vacio[0] = 1;
            vacio[1] = EOS;
            return cmd_ayuda(playerid, vacio);
        }
        return Actividades_MostrarDetalle(playerid, listitem);
    }
    if (dialogid == DIALOGO_ACTIVIDADES_DETALLE)
    {
        if (response) return Actividades_MostrarLista(playerid);
        return 1;
    }

    if (Login_Dialogo(playerid, dialogid, response, listitem, inputtext))   return 1;
    if (Cuenta_Dialogo(playerid, dialogid, response, listitem, inputtext))  return 1;
    if (Inv_Dialogo(playerid, dialogid, response, listitem, inputtext))     return 1;
    if (Trabajo_Dialogo(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Faccion_Dialogo(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Veh_Dialogo(playerid, dialogid, response, listitem, inputtext))     return 1;
    if (Dropeo_Dialogo(playerid, dialogid, response, listitem, inputtext))  return 1;
    if (Banco_Dialogo(playerid, dialogid, response, listitem, inputtext))   return 1;
    if (Panel_Dialogo(playerid, dialogid, response, listitem, inputtext))     return 1;
    if (Casa_Dialogo(playerid, dialogid, response, listitem, inputtext))      return 1;
    if (Tienda_Dialogo(playerid, dialogid, response, listitem, inputtext))    return 1;
    if (GPS_Dialogo(playerid, dialogid, response, listitem, inputtext))       return 1;
    if (Negocio_Dialogo(playerid, dialogid, response, listitem, inputtext))   return 1;
    if (Medicos_Dialogo(playerid, dialogid, response, listitem, inputtext))   return 1;
    if (Policia_Dialogo(playerid, dialogid, response, listitem, inputtext))   return 1;
    if (Comida_Dialogo(playerid, dialogid, response, listitem, inputtext))    return 1;
    if (Hospital_TratamientoDialogo(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Maletero_Dialogo(playerid, dialogid, response, listitem, inputtext))  return 1;
    if (Federico_HistoricoDialogo(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Guia_Dialogo(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (GuiaVeh_Dialogo(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Empenos_Dialogo(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Pesca_Dialogo(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Perrito_Dialogo(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Ascensos_Dialogo(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Cronos_Dialogo(playerid, dialogid, response, listitem, inputtext)) return 1;

    return 1;
}

// ---------------------------------------------------------------------
//  TEXTDRAWS CLICABLES
// ---------------------------------------------------------------------
public OnPlayerClickTextDraw(playerid, Text:clickedid)
{
    Jugador[playerid][jUltimaActividad] = GetTickCount();

    if (Federico_ClickTextDraw(playerid, clickedid)) return 1;
    return Login_ClickTextDraw(playerid, clickedid);
}

public OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
    Jugador[playerid][jUltimaActividad] = GetTickCount();
    Tactil_OnClickPlayerTextDraw(playerid, playertextid);
    return 1;
}

// ---------------------------------------------------------------------
//  VEHICULOS
// ---------------------------------------------------------------------
public OnPlayerStateChange(playerid, PLAYER_STATE:newstate, PLAYER_STATE:oldstate)
{
    if (!Jugador[playerid][jLogueado]) return 1;

    if (newstate == PLAYER_STATE_DRIVER)
    {
        Veh_AlConducir(playerid, GetPlayerVehicleID(playerid));
        new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
        if (VehiculoValido(indice)) Dropeo_AlSubir(indice);
        if (VehiculoValido(indice) && !Federico_AlEntrarComoConductor(playerid, indice)) Veh_MostrarEtiquetaCategoria(playerid, indice);
        if (VehiculoValido(indice)) RoboExpress_AlEntrarComoConductor(playerid, indice);
    }
    else if (oldstate == PLAYER_STATE_DRIVER)
    {
        new indice = Jugador[playerid][jVehiculoDentro];
        Jugador[playerid][jVehiculoDentro] = -1; // antes de Veh_ResolverCriticoVacio: que no se cuente a si mismo como ocupante
        if (VehiculoValido(indice))
        {
            Veh_AdminMotorAlBajar(indice);
            // Fix vehiculos ardiendo vacios (12/09/2026): refresca la
            // salud real (Veh_Guardar no la lee sola) y, si se queda
            // vacio y critico, lo resuelve ya en vez de dejarlo ardiendo
            // congelado para la proxima persona que se suba.
            Veh_RefrescarSalud(indice);
            Veh_ResolverCriticoVacio(indice);
            Veh_Guardar(indice);
            Dropeo_AlBajar(indice);
        }
        Drift_Reiniciar(playerid);
    }

    // Cartel de dropeo: tambien se oculta si entras de pasajero, no solo
    // como conductor (Dropeo_AlSubir/AlBajar cuentan ocupantes).
    if (newstate == PLAYER_STATE_PASSENGER)
    {
        new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
        if (VehiculoValido(indice)) Dropeo_AlSubir(indice);
        Jugador[playerid][jVehiculoPasajero] = indice;
    }
    else if (oldstate == PLAYER_STATE_PASSENGER)
    {
        new indice = Jugador[playerid][jVehiculoPasajero];
        Jugador[playerid][jVehiculoPasajero] = -1; // antes de Veh_ResolverCriticoVacio: que no se cuente a si mismo como ocupante
        if (VehiculoValido(indice))
        {
            Dropeo_AlBajar(indice);
            // Mismo fix que al bajar de conductor: si el ultimo que
            // quedaba dentro era un pasajero (el conductor ya se habia
            // bajado antes), tambien hay que resolverlo si esta critico.
            Veh_RefrescarSalud(indice);
            Veh_ResolverCriticoVacio(indice);
        }
    }

    return 1;
}

public OnPlayerEnterVehicle(playerid, vehicleid, ispassenger)
{
    if (!Jugador[playerid][jLogueado] || ispassenger) return 1;

    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (indice == -1) return 1;

    // Aviso antes de entrar, para que no pierda el tiempo.
    if (!Veh_PuedeUsar(playerid, indice)) Veh_ExplicarRechazo(playerid, indice);
    return 1;
}

public OnVehicleDeath(vehicleid, killerid)
{
    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (indice == -1) return 1;

    g_Vehiculos[indice][veSalud] = 350.0;
    Veh_Guardar(indice);
    return 1;
}

// Cada vez que un vehiculo aparece en la pantalla de alguien hay que
// decidir si para EL esta abierto o cerrado. Es el punto donde se aplica
// de verdad la norma de los vehiculos de trabajo y de faccion.
public OnVehicleStreamIn(vehicleid, forplayerid)
{
    new indice = Veh_BuscarPorObjeto(vehicleid);

    if (indice == -1) return 1;

    Veh_ActualizarCierrePara(forplayerid, indice);
    return 1;
}

public OnVehicleSpawn(vehicleid)
{
    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (indice == -1) return 1;

    // Los vehiculos de trabajo son de la empresa: vuelven del taller como
    // nuevos. Si no, la flota se degrada sola y el trabajo deja de rendir.
    if (g_Vehiculos[indice][veTipo] == VEH_TRABAJO)
    {
        g_Vehiculos[indice][veSalud]       = 1000.0;
        g_Vehiculos[indice][veCombustible] = 100.0;
        SetVehicleHealth(vehicleid, 1000.0);
        Veh_Guardar(indice);
    }

    // Al reaparecer vuelve a su sitio de aparcamiento.
    SetVehiclePos(vehicleid, g_Vehiculos[indice][veX], g_Vehiculos[indice][veY], g_Vehiculos[indice][veZ]);
    SetVehicleZAngle(vehicleid, g_Vehiculos[indice][veA]);
    LinkVehicleToInterior(vehicleid, g_Vehiculos[indice][veInterior]);
    SetVehicleVirtualWorld(vehicleid, g_Vehiculos[indice][veMundo]);
    SetVehicleNumberPlate(vehicleid, g_Vehiculos[indice][veMatricula]);

    // Al reaparecer se pierde el cierre por jugador: hay que reponerlo.
    Veh_ActualizarCierreTodos(indice);
    return 1;
}

// El jugador ha elegido una pieza en un taller nativo de GTA:SA
// (Transfender, Wheel Arch Angels, Loco Low Co...). Se guarda para que
// siga puesta la proxima vez que el coche aparezca (14_tuning.inc).
public OnVehicleMod(playerid, vehicleid, componentid)
{
    #pragma unused playerid, componentid
    Tuning_AlModificar(vehicleid);
    return 1;
}

// Igual que arriba pero para la pintura elegida en Loco Low Co.
public OnVehiclePaintjob(playerid, vehicleid, paintjobid)
{
    #pragma unused playerid
    Tuning_AlPintar(vehicleid, paintjobid);
    return 1;
}

// El color normal (Pay 'n' Spray / Loco Low Co), que es OTRO callback
// distinto del paintjob de arriba. g_Vehiculos no se entera solo del
// cambio -- si no se guarda aqui, el siguiente OnVehicleSpawn o
// reinicio lo repinta con el color viejo que sigue en la ficha.
public OnVehicleRespray(playerid, vehicleid, color1, color2)
{
    #pragma unused playerid
    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (indice == -1) return 1;

    g_Vehiculos[indice][veColor1] = color1;
    g_Vehiculos[indice][veColor2] = color2;
    Veh_Guardar(indice);
    return 1;
}

// ---------------------------------------------------------------------
//  ACTIVIDAD Y AUSENCIA
// ---------------------------------------------------------------------
public OnPlayerUpdate(playerid)
{
    if (!Jugador[playerid][jLogueado]) return 1;

    Jugador[playerid][jUltimaActividad] = GetTickCount();

    // Aviso de "has vuelto" quitado 13/09/2026 a peticion de Vlad -- la
    // deteccion de AFK en si (jAFK, exclusion de nomina/hambre-sed/
    // ganancias) sigue funcionando igual, solo dejo de avisar por chat.
    if (Jugador[playerid][jAFK])
    {
        Jugador[playerid][jAFK] = false;
    }
    return 1;
}

public OnPlayerEnterCheckpoint(playerid)
{
    // Los modulos que gestionan sus propios checkpoints devuelven 1 y se
    // encargan de mantener o cambiar el marcador. Si nadie lo gestiona,
    // lo quitamos por defecto.
    if (VendedorArmas_OnEnterCheckpoint(playerid)) return 1;
    if (MariaVendedor_OnEnterCheckpoint(playerid)) return 1;
    DisablePlayerCheckpoint(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  RCON: el resto de comandos (gmx, exit, players...) siguen su camino
//  normal. El puente web ya no vive aqui -- se movio a una cola en
//  MySQL (tabla web_cola, ver 31_web.inc) porque el RCON heredado de
//  SA-MP no era fiable en open.mp 1.5.8.
// ---------------------------------------------------------------------
public OnRconCommand(cmd[])
{
    return 0;
}

// ---------------------------------------------------------------------
//  TEMPORIZADORES
// ---------------------------------------------------------------------
forward TicSegundo();
public TicSegundo()
{
    g_SegundosServidor++;

    new ahora = GetTickCount();

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        if (IsPlayerNPC(i)) continue;

        if (!Jugador[i][jLogueado])
        {
            Login_Segundo(i);
            continue;
        }

        Jugador[i][jSegundosSesion]++;
        Admin_Segundo(i);
        Policia_Segundo(i);
        Veh_HUD_Segundo(i);
        Camara_Segundo(i);
        Atracos_Segundo(i);
        HambreSed_Segundo(i);
        HambreSed_ActualizarHUD(i);

        // Deteccion de ausencia. Aviso por chat quitado 13/09/2026 a
        // peticion de Vlad -- jAFK se sigue marcando igual (nomina,
        // hambre/sed y ganancias lo siguen respetando), solo deja de
        // notificarse.
        if (!Jugador[i][jAFK] && (ahora - Jugador[i][jUltimaActividad]) > (PAYDAY_AFK_SEGUNDOS * 1000))
        {
            Jugador[i][jAFK] = true;
        }
    }

    Suelo_Segundo();
    Admin_SegundoGlobal();
    Policia_AvisosSegundo();
    Policia_ComisariaSegundo();
    MariaVendedor_SegundoGlobal();
    GPS_ActualizarContador();
    if (g_SegundosServidor % RASTREADOR_INTERVALO_SEGUNDOS == 0) Rastreador_Segundo();
    if (g_SegundosServidor % COMIDA_SEGUNDOS_DIA == 0) Comida_IngresoDiario();
    if (g_SegundosServidor % MOTEL_ALQUILER_SEGUNDOS == 0) Motel_IngresoDiario();
    Motel_Segundo();
    Veh_AvisoCaducidad_Segundo();
    Federico_Segundo();
    Bienvenida_Segundo();
    Guia_Segundo();
    GuiaVeh_Segundo();
    Pesca_Segundo();
    return 1;
}

forward TicMedioMinuto();
public TicMedioMinuto()
{
    Veh_ConsumirCombustible();
    return 1;
}

forward TicMinuto();
public TicMinuto()
{
    // Mantiene fresca g_SemanaIso para los rankings semanales de los
    // trabajos, sin que ellos tengan que consultar la base por cada carrera.
    BD_RefrescarSemana();

    Atracos_RefrescarCarteles();

    Maria_ComprobarPlantas();
    Servicios_Tick();
    Mercado_Tick();
    RoboExpress_Minuto();   // global, no per-player -- caducidad + validez del objetivo + ciclo de 30 min
    ClimaHora_TickMinuto(); // global -- precalcula el proximo estado 5 min antes de aplicarlo (60_climahora.inc)
    // ServerIA_Minuto() (rotacion institucional cada 15 min) quitada el
    // 10/09/2026 -- peticion expresa de Vlad, ver 58_ascensos.inc.
    Ascensos_ComprobarLiderVacanteMinuto();   // global -- sucesion policial activa, cada 10 min (58_ascensos.inc)
    FedJugadores_Minuto();  // global, no per-player -- retorno independiente de 48h por vehiculo (59_federicojugadores.inc)

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        Payday_Minuto(i);
        Policia_Minuto(i);
        Ganancias_Minuto(i);
        Consejos_Minuto(i);
        Ascensos_Minuto(i);
        ClimaHora_SincronizarClimaJugador(i);  // 60_climahora.inc -- defensa contra overrides de SetPlayerWeather que se queden pegados (login, creacion de personaje...); la hora ya la sincroniza sola la libreria realtime-clock
    }
    return 1;
}

// El Agricultor Personal trabaja SOLO sus propias plantas reclamadas
// (plAgricultor), una vez por hora -- ver 28_marihuana.inc.
forward TicHora();
public TicHora()
{
    Agricultor_Tick();
    ClimaHora_Tick();  // 60_climahora.inc -- decision de clima cada ~2h reales (horas pares de Madrid)
    return 1;
}

forward TicGuardado();
public TicGuardado()
{
    new cuentas = BD_GuardarTodos();
    new vehiculos = Veh_GuardarTodos();

    if (cuentas > 0) printf("  [GUARDADO] %d cuentas y %d vehiculos guardados.", cuentas, vehiculos);
    return 1;
}

// Resultado del editor 3D nativo de /editartoy (41_toys.inc) -- el
// jugador arrastra el objeto con el raton y esto llega cuando confirma
// o cancela.
public OnPlayerEditAttachedObject(playerid, EDIT_RESPONSE:response, index, modelid, boneid,
    Float:fOffsetX, Float:fOffsetY, Float:fOffsetZ,
    Float:fRotX, Float:fRotY, Float:fRotZ,
    Float:fScaleX, Float:fScaleY, Float:fScaleZ)
{
    #pragma unused index, modelid, boneid
    Toys_TerminarEdicion(playerid, _:response, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ, fScaleX, fScaleY, fScaleZ);
    return 1;
}
