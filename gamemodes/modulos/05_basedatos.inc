/* =====================================================================
 *  05_basedatos.inc
 *  ---------------------------------------------------------------------
 *  Toda la comunicacion con MySQL pasa por aqui. Ningun otro modulo
 *  escribe consultas SQL de cuentas: llaman a estas funciones.
 *
 *  Todas las consultas son asincronas (mysql_pquery) salvo la carga
 *  inicial del servidor, que si bloquea porque ocurre antes de que
 *  entre nadie.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  CONFIGURACION CARGADA DESDE LA TABLA servidor_config
//  Se puede cambiar en caliente con comandos de administracion.
// ---------------------------------------------------------------------
new Float:g_BancoPos[3];
new g_BancoInterior;

// Casa de Empeños (50_pawnshop.inc, 06/09/2026) -- mismo patron que el
// banco de arriba: los valores de partida son la mejor referencia
// disponible (interior "Zero's RC Shop"), pendientes de afinar en vivo
// con /setempeños si el punto de aparicion no cae exacto.
new Float:g_EmpenosIntPos[3];
new Float:g_EmpenosIntAngulo;
new g_EmpenosIntInterior;

// Gamerol Aventuras: DLC Pesca (51_pesca.inc, 06/09/2026) -- declaradas
// aqui, no en 51_pesca.inc, por el mismo motivo que g_EmpenosIntPos:
// /setpesca y /setventapescado viven en 16_administracion.inc, que se
// incluye ANTES que el modulo de pesca, y Pawn no reordena variables
// globales entre archivos (a diferencia de las funciones, que si admite
// forward implicito con un aviso "forcing reparse").
#define PESCA_TOTAL_PUNTOS 6
new Float:g_PescaPuntoX[PESCA_TOTAL_PUNTOS];
new Float:g_PescaPuntoY[PESCA_TOTAL_PUNTOS];
new Float:g_PescaPuntoZ[PESCA_TOTAL_PUNTOS];
new Float:g_PescaPuntoA[PESCA_TOTAL_PUNTOS];
new bool:g_PescaPuntoActivo[PESCA_TOTAL_PUNTOS];

#define PESCA_TOTAL_VENTAS 3
new Float:g_PescaVentaX[PESCA_TOTAL_VENTAS];
new Float:g_PescaVentaY[PESCA_TOTAL_VENTAS];
new Float:g_PescaVentaZ[PESCA_TOTAL_VENTAS];
new STREAMER_TAG_PICKUP:g_PescaVentaPickup[PESCA_TOTAL_VENTAS];
new STREAMER_TAG_3D_TEXT_LABEL:g_PescaVentaEtiqueta[PESCA_TOTAL_VENTAS];

// ---------------------------------------------------------------------
//  CONEXION
// ---------------------------------------------------------------------
stock BD_Conectar()
{
    mysql_log(ERROR | WARNING);

    g_BD = mysql_connect_file("mysql.ini");

    if (g_BD == MYSQL_INVALID_HANDLE || mysql_errno(g_BD) != 0)
    {
        new detalle[256];
        mysql_error(detalle, sizeof detalle, g_BD);

        print(" ");
        print("  ##################################################");
        print("  #  NO SE HA PODIDO CONECTAR CON MYSQL            #");
        print("  ##################################################");
        printf("  #  Motivo: %s", detalle);
        print("  #");
        print("  #  Comprueba que:");
        print("  #   1. XAMPP esta abierto y MySQL en marcha.");
        print("  #   2. Has importado sql/malagacity.sql.");
        print("  #   3. mysql.ini (raiz del servidor) tiene los datos correctos.");
        print("  ##################################################");
        print(" ");

        SendRconCommand("exit");
        return 0;
    }

    // El cliente de SA-MP trabaja en latin1. Si esto no coincide, los
    // acentos que escriban los jugadores se guardan corruptos.
    mysql_set_charset("latin1", g_BD);

    printf("  [BD] Conectado a MySQL (mysql.ini): %s", BD_NOMBRE);
    return 1;
}

stock BD_Desconectar()
{
    if (g_BD != MYSQL_INVALID_HANDLE) mysql_close(g_BD);
    return 1;
}

// ---------------------------------------------------------------------
//  CONFIGURACION DEL SERVIDOR (posiciones, porcentajes...)
// ---------------------------------------------------------------------
stock Config_Leer(const clave[], Float:pordefecto)
{
    new consulta[160], Float:valor = pordefecto;

    mysql_format(g_BD, consulta, sizeof consulta,
        "SELECT valor FROM servidor_config WHERE clave = '%e' LIMIT 1", clave);

    new Cache:resultado = mysql_query(g_BD, consulta);
    new filas;
    cache_get_row_count(filas);

    if (filas > 0)
    {
        new texto[32];
        cache_get_value_name(0, "valor", texto, sizeof texto);
        valor = floatstr(texto);
    }
    cache_delete(resultado);
    return _:valor;
}

stock Config_Escribir(const clave[], Float:valor)
{
    new consulta[200];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO servidor_config (clave, valor) VALUES ('%e', '%.4f') \
         ON DUPLICATE KEY UPDATE valor = VALUES(valor)", clave, valor);
    mysql_pquery(g_BD, consulta);
    return 1;
}

stock BD_CargarConfiguracion()
{
    g_BancoPos[0]      = Float:Config_Leer("banco_x", 378.5350);
    g_BancoPos[1]      = Float:Config_Leer("banco_y", 173.7106);
    g_BancoPos[2]      = Float:Config_Leer("banco_z", 1008.3828);
    g_BancoInterior    = floatround(Float:Config_Leer("banco_int", 3.0));

    g_EmpenosIntPos[0] = Float:Config_Leer("empenos_x", EMPENOS_INT_X);
    g_EmpenosIntPos[1] = Float:Config_Leer("empenos_y", EMPENOS_INT_Y);
    g_EmpenosIntPos[2] = Float:Config_Leer("empenos_z", EMPENOS_INT_Z);
    g_EmpenosIntAngulo = Float:Config_Leer("empenos_a", EMPENOS_INT_ANGULO);
    g_EmpenosIntInterior = floatround(Float:Config_Leer("empenos_int", float(EMPENOS_INT_INTERIOR)));

    print("  [BD] Configuracion del servidor cargada.");
    return 1;
}

// ---------------------------------------------------------------------
//  SEMANA ISO EN CURSO
//  Las vistas de ranking filtran por DATE_FORMAT(NOW(), '%x-%v'), asi que
//  el valor tiene que salir de MySQL para que coincida exactamente. Se
//  cachea en g_SemanaIso y se refresca una vez por minuto: asi los modulos
//  de trabajos no lanzan una consulta por cada carrera terminada.
// ---------------------------------------------------------------------
stock BD_RefrescarSemana()
{
    mysql_pquery(g_BD, "SELECT DATE_FORMAT(NOW(), '%x-%v') AS iso", "AlRefrescarSemana");
    return 1;
}

forward AlRefrescarSemana();
public AlRefrescarSemana()
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0) return 0;

    cache_get_value_name(0, "iso", g_SemanaIso, sizeof g_SemanaIso);
    return 1;
}

// ---------------------------------------------------------------------
//  COMPROBACION DE CUENTA AL CONECTAR
//  Averigua si el nombre existe y si tiene una sancion activa.
// ---------------------------------------------------------------------
stock BD_ComprobarCuenta(playerid)
{
    new nombre[MAX_PLAYER_NAME + 1], consulta[512];
    GetPlayerName(playerid, nombre, sizeof nombre);

    mysql_format(g_BD, consulta, sizeof consulta,
        "SELECT id, password, nivel, admin, minutos_totales, baneado, ban_motivo, ban_admin, \
                DATE_FORMAT(fecha_registro, '%%d/%%m/%%Y') AS alta, \
                IFNULL(DATE_FORMAT(ban_expira, '%%d/%%m/%%Y %%H:%%i'), 'nunca') AS caduca, \
                (baneado = 1 AND (ban_expira IS NULL OR ban_expira > NOW())) AS ban_activo \
         FROM cuentas WHERE nombre = '%e' LIMIT 1", nombre);

    mysql_pquery(g_BD, consulta, "AlComprobarCuenta", "d", playerid);
    return 1;
}

forward AlComprobarCuenta(playerid);
public AlComprobarCuenta(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        // Cuenta nueva.
        Jugador[playerid][jRegistrado] = false;
        Login_MostrarPantalla(playerid);
        return 1;
    }

    new banActivo = 0;
    cache_get_value_name_int(0, "ban_activo", banActivo);

    if (banActivo)
    {
        new motivo[128], admin[MAX_PLAYER_NAME + 1], caduca[32];
        cache_get_value_name(0, "ban_motivo", motivo, sizeof motivo);
        cache_get_value_name(0, "ban_admin", admin, sizeof admin);
        cache_get_value_name(0, "caduca", caduca, sizeof caduca);

        SendClientMessage(playerid, COLOR_ROJO_FUERTE, " ");
        SendClientMessage(playerid, COLOR_ROJO_FUERTE, "  ESTA CUENTA ESTA BANEADA DE " SERVIDOR_NOMBRE);
        format(g_Buffer, sizeof g_Buffer, "  Motivo: %s", motivo);
        SendClientMessage(playerid, COLOR_BLANCO, g_Buffer);
        format(g_Buffer, sizeof g_Buffer, "  Administrador: %s", admin);
        SendClientMessage(playerid, COLOR_BLANCO, g_Buffer);
        format(g_Buffer, sizeof g_Buffer, "  Caduca: %s", caduca);
        SendClientMessage(playerid, COLOR_BLANCO, g_Buffer);
        SendClientMessage(playerid, COLOR_GRIS, "  Puedes apelar en " SERVIDOR_DISCORD);
        SendClientMessage(playerid, COLOR_ROJO_FUERTE, " ");

        SetTimerEx("ExpulsarRetardado", 700, false, "d", playerid);
        return 1;
    }

    // Si habia un ban y ya ha caducado, lo limpiamos.
    new baneado = 0;
    cache_get_value_name_int(0, "baneado", baneado);
    if (baneado)
    {
        new nombre[MAX_PLAYER_NAME + 1], consulta[200];
        GetPlayerName(playerid, nombre, sizeof nombre);
        mysql_format(g_BD, consulta, sizeof consulta,
            "UPDATE cuentas SET baneado = 0, ban_motivo = '', ban_expira = NULL WHERE nombre = '%e'", nombre);
        mysql_pquery(g_BD, consulta);
    }

    // Cuenta existente: guardamos el hash para verificar la contrasena.
    Jugador[playerid][jRegistrado] = true;
    cache_get_value_name_int(0, "id", Jugador[playerid][jID]);
    cache_get_value_name(0, "password", Jugador[playerid][jPassword], BCRYPT_HASH_LENGTH);
    cache_get_value_name_int(0, "nivel", Jugador[playerid][jNivel]);
    cache_get_value_name_int(0, "admin", Jugador[playerid][jAdmin]);
    cache_get_value_name_int(0, "minutos_totales", Jugador[playerid][jMinutosTotales]);

    Login_MostrarPantalla(playerid);
    return 1;
}

forward ExpulsarRetardado(playerid);
public ExpulsarRetardado(playerid)
{
    if (IsPlayerConnected(playerid)) Kick(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  CREACION DE CUENTA
// ---------------------------------------------------------------------
stock BD_CrearCuenta(playerid)
{
    // El aspecto elegido en la creacion es el original de este personaje,
    // y hay que dejarlo tambien EN MEMORIA, no solo en el INSERT de
    // abajo: si no, durante esta primera sesion jSkinInicial valdria 0 y
    // Skin_Actualizar no sabria a que aspecto devolverle al dejar un
    // trabajo con uniforme.
    Jugador[playerid][jSkinInicial] = Jugador[playerid][jSkin];

    new nombre[MAX_PLAYER_NAME + 1], ip[46], consulta[768];

    GetPlayerName(playerid, nombre, sizeof nombre);
    GetPlayerIp(playerid, ip, sizeof ip);

    // Numero de cuenta bancaria: pseudoaleatorio y unico por cuenta.
    new numeroCuenta = 100000 + random(899999);

    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO cuentas (nombre, password, email, sexo, edad, skin, skin_inicial, origen, \
                              dinero, banco, cuenta_bancaria, telefono, ip, \
                              pos_x, pos_y, pos_z, pos_a) \
         VALUES ('%e', '%e', '%e', %d, %d, %d, %d, '%e', %d, %d, %d, %d, '%e', %f, %f, %f, %f)",
        nombre,
        Jugador[playerid][jPassword],
        Jugador[playerid][jEmail],
        Jugador[playerid][jSexo],
        Jugador[playerid][jEdad],
        Jugador[playerid][jSkin],
        // El aspecto elegido en la creacion del personaje es, por
        // definicion, el original. Aqui es el unico sitio del gamemode
        // donde skin_inicial se escribe con el mismo valor que skin.
        Jugador[playerid][jSkin],
        Jugador[playerid][jOrigen],
        DINERO_INICIAL,
        BANCO_INICIAL,
        numeroCuenta,
        600000000 + random(99999999),
        ip,
        SPAWN_X, SPAWN_Y, SPAWN_Z, SPAWN_A);

    mysql_pquery(g_BD, consulta, "AlCrearCuenta", "d", playerid);
    return 1;
}

forward AlCrearCuenta(playerid);
public AlCrearCuenta(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    Jugador[playerid][jID]         = cache_insert_id();
    Jugador[playerid][jRegistrado] = true;
    Jugador[playerid][jLogueado]   = true;
    Jugador[playerid][jDinero]     = DINERO_INICIAL;
    Jugador[playerid][jBanco]      = BANCO_INICIAL;
    Jugador[playerid][jNivel]      = 1;

    new nombre[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, nombre, sizeof nombre);
    printf("  [CUENTA] Nueva cuenta creada: %s (id %d)", nombre, Jugador[playerid][jID]);

    format(g_Buffer, sizeof g_Buffer, "Cuenta creada: %s (id %d)", nombre, Jugador[playerid][jID]);
    EscribirLog("cuentas", g_Buffer);

    // Aviso al dueno por WhatsApp (09/09/2026) -- solo la creacion, la
    // primera conexion de esta cuenta NO manda ademas un 'conectado'
    // (peticion expresa de Vlad, para no duplicar el mismo hecho).
    new ipNueva[46];
    GetPlayerIp(playerid, ipNueva, sizeof ipNueva);
    Web_EncolarAvisoOwner("cuenta_nueva", nombre, Jugador[playerid][jEmail], ipNueva);

    g_Conectados++;
    Login_OcultarPantalla(playerid);

    // Sin tutorial: el jugador entra directo al mundo con el equipo
    // inicial. Antes esto lo hacia Tutorial_Finalizar (07_tutorial.inc,
    // eliminado) al terminar los pasos guiados.
    Jugador[playerid][jSpawneado] = true;
    Jugador[playerid][jPosX]      = SPAWN_X;
    Jugador[playerid][jPosY]      = SPAWN_Y;
    Jugador[playerid][jPosZ]      = SPAWN_Z;
    Jugador[playerid][jPosA]      = SPAWN_A;
    Jugador[playerid][jInterior]  = 0;
    Jugador[playerid][jMundo]     = MUNDO_NORMAL;

    // El motor todavia tiene al jugador en MUNDO_CREACION (pantalla de
    // elegir skin, 06_login.inc). Hay que sacarlo de ahi de verdad y no
    // solo en la ficha: GuardarPosicionActual (03_utiles.inc, la llama
    // BD_GuardarCuenta mas abajo) relee jInterior/jMundo EN VIVO del motor
    // con GetPlayerInterior/GetPlayerVirtualWorld, así que si el motor
    // sigue en MUNDO_CREACION en ese momento, pisa lo que acabamos de
    // poner arriba y la cuenta se guarda en el mundo equivocado.
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);

    Inv_DarEquipoInicial(playerid);
    SpawnJugadorEnMundo(playerid);
    BD_GuardarCuenta(playerid);

    new nombreRP[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombreRP, sizeof nombreRP);

    SendClientMessage(playerid, COLOR_MARCA, " ");
    format(g_Buffer, sizeof g_Buffer, PREFIJO "Bienvenido a {FFFFFF}Gamerol 2026{B4B5B7}. Usa {FFFFFF}/comprarcoche{B4B5B7} dentro de un vehiculo y empieza tu historia!");
    SendClientMessage(playerid, COLOR_GRIS, g_Buffer);

    new dinero[24];
    FormatearDinero(DINERO_INICIAL, dinero, sizeof dinero);
    format(g_Buffer, sizeof g_Buffer, PREFIJO "Llevas {3D9970}%s{B4B5B7} encima y algo mas en el banco. Escribe {FFFFFF}/ayuda{B4B5B7} cuando lo necesites.", dinero);
    SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
    SendClientMessage(playerid, COLOR_MARCA, " ");

    GPS_MostrarTextdrawFijo(playerid);

    format(g_Buffer, sizeof g_Buffer, "{E8B33A}%s{FFFFFF} acaba de llegar a Los Santos. Dale la bienvenida.", nombreRP);
    MensajeGlobal(COLOR_BLANCO, g_Buffer);

    return 1;
}

// ---------------------------------------------------------------------
//  CARGA DE CUENTA
// ---------------------------------------------------------------------
stock BD_CargarCuenta(playerid)
{
    new consulta[128];
    mysql_format(g_BD, consulta, sizeof consulta, "SELECT * FROM cuentas WHERE id = %d LIMIT 1", Jugador[playerid][jID]);
    mysql_pquery(g_BD, consulta, "AlCargarCuenta", "d", playerid);
    return 1;
}

forward AlCargarCuenta(playerid);
public AlCargarCuenta(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Error(playerid, "No se ha podido cargar tu cuenta. Vuelve a conectar.");
        SetTimerEx("ExpulsarRetardado", 700, false, "d", playerid);
        return 0;
    }

    new temporal;

    // Identidad
    cache_get_value_name(0, "nombre", Jugador[playerid][jNombre], MAX_PLAYER_NAME + 1);
    cache_get_value_name(0, "email",  Jugador[playerid][jEmail], 64);
    cache_get_value_name(0, "origen", Jugador[playerid][jOrigen], 32);

    // Personaje
    cache_get_value_name_int(0, "sexo",             Jugador[playerid][jSexo]);
    cache_get_value_name_int(0, "edad",             Jugador[playerid][jEdad]);
    cache_get_value_name_int(0, "skin",             Jugador[playerid][jSkin]);
    cache_get_value_name_int(0, "skin_inicial",     Jugador[playerid][jSkinInicial]);
    cache_get_value_name_int(0, "nivel",            Jugador[playerid][jNivel]);
    cache_get_value_name_int(0, "respeto",          Jugador[playerid][jRespeto]);
    cache_get_value_name_int(0, "minutos_totales",  Jugador[playerid][jMinutosTotales]);
    cache_get_value_name_int(0, "minutos_payday",   Jugador[playerid][jMinutosPayday]);
    cache_get_value_name_int(0, "paydays_cobrados", Jugador[playerid][jPaydays]);

    // Economia
    cache_get_value_name_int(0, "dinero",           Jugador[playerid][jDinero]);
    cache_get_value_name_int(0, "banco",            Jugador[playerid][jBanco]);
    cache_get_value_name_int(0, "cuenta_bancaria",  Jugador[playerid][jCuentaBancaria]);

    // Ocupacion
    cache_get_value_name_int(0, "job1",             Jugador[playerid][jTrabajo][0]);
    cache_get_value_name_int(0, "job1_rango",       Jugador[playerid][jTrabajoRango][0]);
    cache_get_value_name_int(0, "job2",             Jugador[playerid][jTrabajo][1]);
    cache_get_value_name_int(0, "job2_rango",       Jugador[playerid][jTrabajoRango][1]);
    cache_get_value_name_int(0, "faccion",          Jugador[playerid][jFaccion]);
    cache_get_value_name_int(0, "faccion_rango",    Jugador[playerid][jFaccionRango]);

    cache_get_value_name_int(0, "faccion_lider", temporal);
    Jugador[playerid][jFaccionLider] = bool:(temporal != 0);

    // Staff autonomo / Server IA (58_ascensos.inc)
    cache_get_value_name_int(0, "staff_rango",     Jugador[playerid][jStaffRango]);
    cache_get_value_name_int(0, "staff_ofertas",   Jugador[playerid][jStaffOfertas]);
    cache_get_value_name_int(0, "policia_ingreso", Jugador[playerid][jPoliciaIngreso]);

    // Estado fisico
    cache_get_value_name_float(0, "salud",    Jugador[playerid][jSalud]);
    cache_get_value_name_float(0, "armadura", Jugador[playerid][jArmadura]);
    cache_get_value_name_float(0, "pos_x",    Jugador[playerid][jPosX]);
    cache_get_value_name_float(0, "pos_y",    Jugador[playerid][jPosY]);
    cache_get_value_name_float(0, "pos_z",    Jugador[playerid][jPosZ]);
    cache_get_value_name_float(0, "pos_a",    Jugador[playerid][jPosA]);
    cache_get_value_name_int(0, "interior",   Jugador[playerid][jInterior]);
    cache_get_value_name_int(0, "mundo",      Jugador[playerid][jMundo]);

    // "herido" ya NO se lee con intencion real: es estado de sesion del
    // sistema de caido/medicos (36_medicos.inc), no algo que tenga
    // sentido arrastrar entre conexiones. jHerido se queda en false (el
    // valor por defecto de ReiniciarJugador). La columna sigue en el
    // esquema, pero queda inerte -- nadie la vuelve a leer con intencion.

    // Hambre y sed (44_hambre_sed.inc)
    cache_get_value_name_float(0, "hambre", Jugador[playerid][jHambre]);
    cache_get_value_name_float(0, "sed",    Jugador[playerid][jSed]);

    // Gamerol Aventuras: DLC Pesca (51_pesca.inc)
    cache_get_value_name_int(0, "pesca_capturas", Jugador[playerid][jPescaCapturas]);
    cache_get_value_name_int(0, "pesca_comidos",  Jugador[playerid][jPescaComidos]);

    // Inventario
    cache_get_value_name_float(0, "peso_extra", Jugador[playerid][jPesoExtra]);

    // Administracion
    cache_get_value_name_int(0, "admin",           Jugador[playerid][jAdmin]);
    cache_get_value_name_int(0, "avisos",          Jugador[playerid][jAvisos]);
    cache_get_value_name_int(0, "mute_segundos",   Jugador[playerid][jMuteSegundos]);
    cache_get_value_name_int(0, "carcel_segundos", Jugador[playerid][jCarcelSegundos]);

    // Policia (37_policia.inc). carcel_rol_segundos es la carcel de ROL,
    // distinta de carcel_segundos, que es la sancion administrativa.
    cache_get_value_name_int(0, "telefono",            Jugador[playerid][jTelefono]);
    cache_get_value_name_int(0, "carnet",              Jugador[playerid][jCarnet]);
    cache_get_value_name_int(0, "carnet_puntos",       Jugador[playerid][jCarnetPuntos]);
    cache_get_value_name_int(0, "buscado",             Jugador[playerid][jBuscado]);
    cache_get_value_name_int(0, "condena_segundos",    Jugador[playerid][jCondenaSegundos]);
    cache_get_value_name_int(0, "carcel_rol_segundos", Jugador[playerid][jCarcelRolSegundos]);

    // Salud minima: si murio con 0 lo dejamos entrar con algo de vida.
    if (Jugador[playerid][jSalud] < 5.0) Jugador[playerid][jSalud] = 30.0;

    Jugador[playerid][jLogueado] = true;
    g_Conectados++;

    // Actualiza la ultima conexion y la IP.
    new ip[46], consulta[256];
    GetPlayerIp(playerid, ip, sizeof ip);
    mysql_format(g_BD, consulta, sizeof consulta,
        "UPDATE cuentas SET ultima_conexion = NOW(), ip = '%e' WHERE id = %d", ip, Jugador[playerid][jID]);
    mysql_pquery(g_BD, consulta);

    // Aviso al dueno por WhatsApp (09/09/2026) -- solo cuentas YA
    // existentes (esta funcion es carga de cuenta, no creacion). La
    // creacion tiene su propio aviso 'cuenta_nueva' en AlCrearCuenta, sin
    // duplicar aqui.
    Web_EncolarAvisoOwner("conectado", Jugador[playerid][jNombre], "", ip);

    Inv_Cargar(playerid);
    Toys_Cargar(playerid);

    // "Ganancias ultima hora" (55_ganancias.inc) -- deserializador
    // defensivo, nunca falla ni deja una lectura fuera de rango aunque
    // la columna venga vacia, a "0" o corrupta.
    new gananciasCadena[768];
    cache_get_value_name(0, "ganancias_ventana", gananciasCadena, sizeof gananciasCadena);
    Ganancias_Deserializar(playerid, gananciasCadena);

    Login_TerminarSesion(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  GUARDADO DE CUENTA
// ---------------------------------------------------------------------
public BD_GuardarCuenta(playerid)
{
    if (!Jugador[playerid][jLogueado] || Jugador[playerid][jID] == 0) return 0;

    GuardarPosicionActual(playerid);

    // "Ganancias ultima hora" (55_ganancias.inc): el total NO se guarda,
    // se reconstruye solo al cargar sumando los 60 cubos.
    new gananciasCadena[768];
    Ganancias_Serializar(playerid, gananciasCadena, sizeof gananciasCadena);

    // 2048 y no 1024 -- el campo de ganancias por si solo puede llegar a
    // rondar los 700 caracteres, y ya nos ha mordido antes un buffer de
    // consulta ajustado que trunca en silencio (GPS_TOTAL_DESTINOS,
    // Maletero_MostrarMenu). Mejor de sobra.
    new consulta[2048];
    mysql_format(g_BD, consulta, sizeof consulta,
        "UPDATE cuentas SET \
            email = '%e', sexo = %d, edad = %d, skin = %d, skin_inicial = %d, origen = '%e', \
            nivel = %d, respeto = %d, minutos_totales = %d, minutos_payday = %d, paydays_cobrados = %d, \
            dinero = %d, banco = %d, \
            job1 = %d, job1_rango = %d, job2 = %d, job2_rango = %d, \
            faccion = %d, faccion_rango = %d, faccion_lider = %d, \
            staff_rango = %d, staff_ofertas = %d, policia_ingreso = %d, \
            salud = %f, armadura = %f, pos_x = %f, pos_y = %f, pos_z = %f, pos_a = %f, \
            interior = %d, mundo = %d, herido = %d, peso_extra = %f, \
            hambre = %f, sed = %f, \
            pesca_capturas = %d, pesca_comidos = %d, \
            admin = %d, avisos = %d, mute_segundos = %d, carcel_segundos = %d, \
            buscado = %d, condena_segundos = %d, carcel_rol_segundos = %d, \
            carnet = %d, carnet_puntos = %d, ganancias_ventana = '%e' \
         WHERE id = %d",
        Jugador[playerid][jEmail],
        Jugador[playerid][jSexo],
        Jugador[playerid][jEdad],
        Jugador[playerid][jSkin],
        Jugador[playerid][jSkinInicial],
        Jugador[playerid][jOrigen],
        Jugador[playerid][jNivel],
        Jugador[playerid][jRespeto],
        Jugador[playerid][jMinutosTotales],
        Jugador[playerid][jMinutosPayday],
        Jugador[playerid][jPaydays],
        Jugador[playerid][jDinero],
        Jugador[playerid][jBanco],
        Jugador[playerid][jTrabajo][0],
        Jugador[playerid][jTrabajoRango][0],
        Jugador[playerid][jTrabajo][1],
        Jugador[playerid][jTrabajoRango][1],
        Jugador[playerid][jFaccion],
        Jugador[playerid][jFaccionRango],
        _:Jugador[playerid][jFaccionLider],
        Jugador[playerid][jStaffRango],
        Jugador[playerid][jStaffOfertas],
        Jugador[playerid][jPoliciaIngreso],
        Jugador[playerid][jSalud],
        Jugador[playerid][jArmadura],
        Jugador[playerid][jPosX],
        Jugador[playerid][jPosY],
        Jugador[playerid][jPosZ],
        Jugador[playerid][jPosA],
        Jugador[playerid][jInterior],
        Jugador[playerid][jMundo],
        _:Jugador[playerid][jHerido],
        Jugador[playerid][jPesoExtra],
        Jugador[playerid][jHambre],
        Jugador[playerid][jSed],
        Jugador[playerid][jPescaCapturas],
        Jugador[playerid][jPescaComidos],
        Jugador[playerid][jAdmin],
        Jugador[playerid][jAvisos],
        Jugador[playerid][jMuteSegundos],
        Jugador[playerid][jCarcelSegundos],
        Jugador[playerid][jBuscado],
        Jugador[playerid][jCondenaSegundos],
        Jugador[playerid][jCarcelRolSegundos],
        Jugador[playerid][jCarnet],
        Jugador[playerid][jCarnetPuntos],
        gananciasCadena,
        Jugador[playerid][jID]);

    mysql_pquery(g_BD, consulta);
    Inv_Guardar(playerid);
    Toys_Guardar(playerid);
    return 1;
}

// Guarda a todos los jugadores conectados (guardado automatico y cierre).
stock BD_GuardarTodos()
{
    new guardados = 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        BD_GuardarCuenta(i);
        guardados++;
    }
    return guardados;
}

// ---------------------------------------------------------------------
//  CAMBIO DE CONTRASENA
// ---------------------------------------------------------------------
stock BD_GuardarPassword(playerid)
{
    new consulta[256];
    mysql_format(g_BD, consulta, sizeof consulta,
        "UPDATE cuentas SET password = '%e' WHERE id = %d",
        Jugador[playerid][jPassword], Jugador[playerid][jID]);
    mysql_pquery(g_BD, consulta);
    return 1;
}

// ---------------------------------------------------------------------
//  REGISTROS
// ---------------------------------------------------------------------
public BD_RegistrarLog(const admin[], nivel, const accion[], const objetivo[], const detalles[])
{
    new consulta[512];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO admin_logs (admin, nivel, accion, objetivo, detalles) \
         VALUES ('%e', %d, '%e', '%e', '%e')", admin, nivel, accion, objetivo, detalles);
    mysql_pquery(g_BD, consulta);

    format(g_Buffer, sizeof g_Buffer, "%s (nivel %d) -> %s | objetivo: %s | %s", admin, nivel, accion, objetivo, detalles);
    EscribirLog("administracion", g_Buffer);
    return 1;
}

public BD_RegistrarSancion(cuentaid, const nombre[], tipo, const admin[], const motivo[], duracion)
{
    new consulta[512];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO sanciones (cuenta_id, nombre, tipo, admin, motivo, duracion) \
         VALUES (%d, '%e', %d, '%e', '%e', %d)", cuentaid, nombre, tipo, admin, motivo, duracion);
    mysql_pquery(g_BD, consulta);
    return 1;
}
