/* =====================================================================
 *  35_cancelas.inc
 *  ---------------------------------------------------------------------
 *  Cancelas (verjas) personales, inspiradas en el sistema de gates de
 *  Next Generation Roleplay pero MUCHO mas simples: van ligadas a una
 *  CUENTA, no a una casa ni a una faccion, y no hay editor interactivo
 *  de posiciones.
 *
 *  MECANICA: sube/baja en vertical, no gira. Estudiando el volcado SQL
 *  real de NG-RP (ngrp.sql, tabla `gates`, 3066 filas con verja de
 *  verdad instalada) la rotacion es casi siempre identica entre
 *  cerrado y abierto -- solo cambia Z -- y el modelo mas usado con
 *  diferencia es el 980 (253 veces, familia de verjas nativas de GTA SA
 *  969-988, sin modelos custom). CERRADA = bajo tierra (fuera de la
 *  vista). ABIERTA = sube al nivel del suelo, la posicion exacta donde
 *  el admin estaba de pie al instalarla.
 *
 *  Ciclo de vida (mismo patron que 34_mercado.inc / servicios): el
 *  jugador PAGA desde el panel web (comprar / pedir traslado, ver
 *  31_web.inc: comandos 'comprar_cancela' y 'mover_cancela' de la cola),
 *  y un ADMIN la instala o la mueve de pie en el sitio elegido con
 *  /crearcancela y /movercancela (16_administracion.inc, ADMIN_JEFE+).
 *  Solo el propietario puede abrirla/cerrarla, con /cancela estando
 *  cerca -- no hay contrasena para invitados.
 *
 *  Este archivo se incluye ANTES que 16_administracion.inc (ver
 *  malagacity.pwn) porque esos comandos de admin tocan g_Cancelas
 *  directamente, igual que 13_vehiculos.inc con g_Vehiculos.
 * ===================================================================== */

#define MAX_CANCELAS            300
#define CANCELA_RANGO_USO       10.0
#define CANCELA_VELOCIDAD_MOV   1.2

enum E_CANCELA
{
    bool:caUsada,
    caID,                                 // fila en la tabla `cancelas`
    caPropietario,                        // Jugador[playerid][jID] del dueno
    caPropietarioNombre[MAX_PLAYER_NAME + 1],
    caModelo,
    Float:caX, Float:caY, Float:caZ,       // posicion ABIERTA (nivel del suelo, la que pisa el admin)
    Float:caRot,
    Float:caProfundidad,                  // metros bajo tierra en el estado CERRADO
    caInterior, caMundo,
    bool:caAbierta,                       // solo en memoria, arranca cerrada
    bool:caTieneCamara,                   // extra comprado via web, ver Camara_Comprar
    STREAMER_TAG_OBJECT:caObjeto
}
new g_Cancelas[MAX_CANCELAS][E_CANCELA];

// ---------------------------------------------------------------------
//  CARGA AL ARRANCAR -- solo las que ya estan instaladas de verdad.
// ---------------------------------------------------------------------
stock Cancelas_CargarTodas()
{
    mysql_pquery(g_BD, "SELECT * FROM cancelas WHERE estado = 'instalada'", "AlCargarCancelas");
    return 1;
}

forward AlCargarCancelas();
public AlCargarCancelas()
{
    new filas;
    cache_get_row_count(filas);

    for (new i = 0; i < filas && i < MAX_CANCELAS; i++)
    {
        g_Cancelas[i][caUsada] = true;
        cache_get_value_name_int(i, "id", g_Cancelas[i][caID]);
        cache_get_value_name_int(i, "propietario", g_Cancelas[i][caPropietario]);
        cache_get_value_name(i, "propietario_nombre", g_Cancelas[i][caPropietarioNombre], MAX_PLAYER_NAME + 1);
        cache_get_value_name_int(i, "modelo", g_Cancelas[i][caModelo]);
        cache_get_value_name_float(i, "x", g_Cancelas[i][caX]);
        cache_get_value_name_float(i, "y", g_Cancelas[i][caY]);
        cache_get_value_name_float(i, "z", g_Cancelas[i][caZ]);
        cache_get_value_name_float(i, "rot", g_Cancelas[i][caRot]);
        cache_get_value_name_float(i, "profundidad", g_Cancelas[i][caProfundidad]);
        cache_get_value_name_int(i, "interior", g_Cancelas[i][caInterior]);
        cache_get_value_name_int(i, "mundo", g_Cancelas[i][caMundo]);

        new temporal;
        cache_get_value_name_int(i, "tiene_camara", temporal);
        g_Cancelas[i][caTieneCamara] = bool:(temporal != 0);

        Cancelas_CrearObjeto(i);
    }

    printf("  [CANCELAS] %d cancelas cargadas desde la base de datos.", filas);
    return 1;
}

// ---------------------------------------------------------------------
//  BUSQUEDAS EN MEMORIA
// ---------------------------------------------------------------------
stock Cancelas_BuscarPorID(cancelaId)
{
    for (new i = 0; i < MAX_CANCELAS; i++)
    {
        if (!g_Cancelas[i][caUsada]) continue;
        if (g_Cancelas[i][caID] != cancelaId) continue;
        return i;
    }
    return -1;
}

stock Cancelas_BuscarSlotLibre()
{
    for (new i = 0; i < MAX_CANCELAS; i++)
    {
        if (!g_Cancelas[i][caUsada]) return i;
    }
    return -1;
}

// ---------------------------------------------------------------------
//  OBJETO -- crea (o recrea, si ya existia) el objeto. Siempre arranca
//  CERRADA (bajo tierra) -- estado de reposo seguro tras instalar,
//  mover o reiniciar el servidor.
// ---------------------------------------------------------------------
stock Cancelas_CrearObjeto(indice)
{
    if (IsValidDynamicObject(g_Cancelas[indice][caObjeto])) DestroyDynamicObject(g_Cancelas[indice][caObjeto]);

    g_Cancelas[indice][caObjeto] = CreateDynamicObject(g_Cancelas[indice][caModelo],
        g_Cancelas[indice][caX], g_Cancelas[indice][caY], g_Cancelas[indice][caZ] - g_Cancelas[indice][caProfundidad],
        0.0, 0.0, g_Cancelas[indice][caRot],
        g_Cancelas[indice][caMundo], g_Cancelas[indice][caInterior], -1, 100.0);
    g_Cancelas[indice][caAbierta] = false;
    return 1;
}

// ---------------------------------------------------------------------
//  ABRIR / CERRAR -- sube/baja en vertical, la rotacion no cambia.
//  Cerrada = caZ - caProfundidad (bajo tierra). Abierta = caZ (nivel
//  del suelo, la posicion exacta donde el admin estaba de pie al
//  instalarla). Sin temporizador de auto-cierre a proposito: el dueno
//  la abre y la cierra el mismo con el mismo comando.
// ---------------------------------------------------------------------
stock Cancelas_Alternar(indice)
{
    if (!g_Cancelas[indice][caAbierta])
    {
        MoveDynamicObject(g_Cancelas[indice][caObjeto],
            g_Cancelas[indice][caX], g_Cancelas[indice][caY], g_Cancelas[indice][caZ],
            CANCELA_VELOCIDAD_MOV, 0.0, 0.0, g_Cancelas[indice][caRot]);
        g_Cancelas[indice][caAbierta] = true;
    }
    else
    {
        MoveDynamicObject(g_Cancelas[indice][caObjeto],
            g_Cancelas[indice][caX], g_Cancelas[indice][caY], g_Cancelas[indice][caZ] - g_Cancelas[indice][caProfundidad],
            CANCELA_VELOCIDAD_MOV, 0.0, 0.0, g_Cancelas[indice][caRot]);
        g_Cancelas[indice][caAbierta] = false;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  COMPRAR / PEDIR TRASLADO -- llamado desde Web_Dispatch (31_web.inc,
//  comandos de cola 'comprar_cancela' / 'mover_cancela'). Mismo patron
//  de dos pasos que Web_RenovarPapeles: se revalida el banco de verdad
//  aqui, la web solo pide.
// ---------------------------------------------------------------------
stock Cancelas_Comprar(colaId, cuentaId)
{
    new consulta[220];
    format(consulta, sizeof consulta,
        "SELECT nombre, banco, (SELECT COUNT(*) FROM cancelas WHERE propietario = %d) AS total FROM cuentas WHERE id = %d LIMIT 1",
        cuentaId, cuentaId);
    mysql_pquery(g_BD, consulta, "Cancelas_AlComprobarCompra", "dd", colaId, cuentaId);
    return 1;
}

forward Cancelas_AlComprobarCompra(colaId, cuentaId);
public Cancelas_AlComprobarCompra(colaId, cuentaId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Cuenta no encontrada.");
        return 0;
    }

    new nombre[MAX_PLAYER_NAME + 1], banco, total;
    cache_get_value_name(0, "nombre", nombre, MAX_PLAYER_NAME + 1);
    cache_get_value_name_int(0, "banco", banco);
    cache_get_value_name_int(0, "total", total);

    // Sin limite de cuantas -- cada cancela extra cuesta CANCELA_INCREMENTO
    // mas que la anterior (1a = CANCELA_PRECIO_COMPRA, 2a = +100k, 3a = +200k...).
    new precio = CANCELA_PRECIO_COMPRA + (total * CANCELA_INCREMENTO);

    if (banco < precio)
    {
        Web_MarcarTerminado(colaId, "error", "Saldo insuficiente en el banco.");
        return 0;
    }

    new consultaCobro[120];
    format(consultaCobro, sizeof consultaCobro, "UPDATE cuentas SET banco = banco - %d WHERE id = %d", precio, cuentaId);
    mysql_pquery(g_BD, consultaCobro);

    new consultaInsert[300];
    mysql_format(g_BD, consultaInsert, sizeof consultaInsert,
        "INSERT INTO cancelas (propietario, propietario_nombre, estado) VALUES (%d, '%e', 'pendiente_instalar')",
        cuentaId, nombre);
    mysql_pquery(g_BD, consultaInsert);

    new conectado = -1;
    for (new p = 0; p < MAX_PLAYERS; p++)
    {
        if (!IsPlayerConnected(p) || !Jugador[p][jLogueado]) continue;
        if (Jugador[p][jID] != cuentaId) continue;
        conectado = p;
        break;
    }
    if (conectado != -1)
    {
        Jugador[conectado][jBanco] -= precio;
        Banco_Movimiento(cuentaId, 2, -precio, Jugador[conectado][jBanco], "", "Compra de cancela via web");
    }
    else
    {
        Banco_Movimiento(cuentaId, 2, -precio, banco - precio, "", "Compra de cancela via web");
    }

    format(g_Buffer, sizeof g_Buffer, "Cancela numero %d comprada via web: cuenta %d, -%d$", total + 1, cuentaId, precio);
    EscribirLog("cancelas", g_Buffer);
    Web_MarcarTerminado(colaId, "ok", "Cancela comprada. Pide a un admin que te la instale donde quieras.");
    return 1;
}

stock Cancelas_PedirTraslado(colaId, cuentaId, cancelaId)
{
    new consulta[220];
    format(consulta, sizeof consulta,
        "SELECT banco, (SELECT id FROM cancelas WHERE id = %d AND propietario = %d AND estado = 'instalada' LIMIT 1) AS cancela_id FROM cuentas WHERE id = %d LIMIT 1",
        cancelaId, cuentaId, cuentaId);
    mysql_pquery(g_BD, consulta, "Cancelas_AlComprobarTraslado", "dd", colaId, cuentaId);
    return 1;
}

forward Cancelas_AlComprobarTraslado(colaId, cuentaId);
public Cancelas_AlComprobarTraslado(colaId, cuentaId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Cuenta no encontrada.");
        return 0;
    }

    new banco, cancelaId;
    cache_get_value_name_int(0, "banco", banco);
    cache_get_value_name_int(0, "cancela_id", cancelaId);

    if (cancelaId == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Esa cancela no es tuya, no esta instalada, o ya no existe.");
        return 0;
    }
    if (banco < CANCELA_PRECIO_MOVER)
    {
        Web_MarcarTerminado(colaId, "error", "Saldo insuficiente en el banco.");
        return 0;
    }

    new consultaCobro[120];
    format(consultaCobro, sizeof consultaCobro, "UPDATE cuentas SET banco = banco - %d WHERE id = %d", CANCELA_PRECIO_MOVER, cuentaId);
    mysql_pquery(g_BD, consultaCobro);

    new consultaEstado[150];
    format(consultaEstado, sizeof consultaEstado, "UPDATE cancelas SET estado = 'pendiente_mover' WHERE id = %d", cancelaId);
    mysql_pquery(g_BD, consultaEstado);

    new conectado = -1;
    for (new p = 0; p < MAX_PLAYERS; p++)
    {
        if (!IsPlayerConnected(p) || !Jugador[p][jLogueado]) continue;
        if (Jugador[p][jID] != cuentaId) continue;
        conectado = p;
        break;
    }
    if (conectado != -1)
    {
        Jugador[conectado][jBanco] -= CANCELA_PRECIO_MOVER;
        Banco_Movimiento(cuentaId, 2, -CANCELA_PRECIO_MOVER, Jugador[conectado][jBanco], "", "Traslado de cancela via web");
    }
    else
    {
        Banco_Movimiento(cuentaId, 2, -CANCELA_PRECIO_MOVER, banco - CANCELA_PRECIO_MOVER, "", "Traslado de cancela via web");
    }

    format(g_Buffer, sizeof g_Buffer, "Traslado de cancela pedido via web: cuenta %d, cancela %d, -%d$", cuentaId, cancelaId, CANCELA_PRECIO_MOVER);
    EscribirLog("cancelas", g_Buffer);
    Web_MarcarTerminado(colaId, "ok", "Traslado pedido. Pide a un admin que la mueva al sitio nuevo.");
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDO DEL JUGADOR
// ---------------------------------------------------------------------
CMD:cancela(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new indice = -1;
    new Float:mejor = CANCELA_RANGO_USO, Float:distancia;

    for (new i = 0; i < MAX_CANCELAS; i++)
    {
        if (!g_Cancelas[i][caUsada]) continue;
        if (g_Cancelas[i][caPropietario] != Jugador[playerid][jID]) continue;

        distancia = GetPlayerDistanceFromPoint(playerid, g_Cancelas[i][caX], g_Cancelas[i][caY], g_Cancelas[i][caZ]);
        if (distancia < mejor) { mejor = distancia; indice = i; }
    }

    if (indice == -1)
    {
        Error(playerid, "No tienes ninguna cancela tuya cerca.");
        return 1;
    }

    Cancelas_Alternar(indice);
    return 1;
}

// ---------------------------------------------------------------------
//  CAMARA DE SEGURIDAD -- extra de UNA cancela ya instalada, comprado
//  via web (31_web.inc, comando de cola 'comprar_camara'). Mismo patron
//  de dos pasos que Cancelas_Comprar: la web solo pide, aqui se revalida
//  el banco y la propiedad de verdad.
// ---------------------------------------------------------------------
stock Camara_Comprar(colaId, cuentaId, cancelaId)
{
    new consulta[400];
    format(consulta, sizeof consulta,
        "SELECT cu.banco, \
         (SELECT id FROM cancelas WHERE id = %d AND propietario = %d AND estado = 'instalada' LIMIT 1) AS cancela_id, \
         (SELECT tiene_camara FROM cancelas WHERE id = %d AND propietario = %d LIMIT 1) AS ya_tiene \
         FROM cuentas cu WHERE cu.id = %d LIMIT 1",
        cancelaId, cuentaId, cancelaId, cuentaId, cuentaId);
    mysql_pquery(g_BD, consulta, "Camara_AlComprobarCompra", "ddd", colaId, cuentaId, cancelaId);
    return 1;
}

forward Camara_AlComprobarCompra(colaId, cuentaId, cancelaId);
public Camara_AlComprobarCompra(colaId, cuentaId, cancelaId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Cuenta no encontrada.");
        return 0;
    }

    new banco, cancelaValida, yaTiene;
    cache_get_value_name_int(0, "banco", banco);
    cache_get_value_name_int(0, "cancela_id", cancelaValida);
    cache_get_value_name_int(0, "ya_tiene", yaTiene);

    if (cancelaValida == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Esa cancela no es tuya o todavia no esta instalada.");
        return 0;
    }
    if (yaTiene != 0)
    {
        Web_MarcarTerminado(colaId, "error", "Esa cancela ya tiene camara.");
        return 0;
    }
    if (banco < CAMARA_PRECIO)
    {
        Web_MarcarTerminado(colaId, "error", "Saldo insuficiente en el banco.");
        return 0;
    }

    new consultaCobro[120];
    format(consultaCobro, sizeof consultaCobro, "UPDATE cuentas SET banco = banco - %d WHERE id = %d", CAMARA_PRECIO, cuentaId);
    mysql_pquery(g_BD, consultaCobro);

    new consultaInstalar[100];
    format(consultaInstalar, sizeof consultaInstalar, "UPDATE cancelas SET tiene_camara = 1 WHERE id = %d", cancelaId);
    mysql_pquery(g_BD, consultaInstalar);

    // Si la cancela ya esta cargada en memoria (siempre que 'instalada'),
    // activa la camara ahi mismo -- sin esto habria que esperar a un
    // reinicio para que Camara_Segundo empezara a vigilarla.
    new indice = Cancelas_BuscarPorID(cancelaId);
    if (indice != -1) g_Cancelas[indice][caTieneCamara] = true;

    new conectado = -1;
    for (new p = 0; p < MAX_PLAYERS; p++)
    {
        if (!IsPlayerConnected(p) || !Jugador[p][jLogueado]) continue;
        if (Jugador[p][jID] != cuentaId) continue;
        conectado = p;
        break;
    }
    if (conectado != -1)
    {
        Jugador[conectado][jBanco] -= CAMARA_PRECIO;
        Banco_Movimiento(cuentaId, 2, -CAMARA_PRECIO, Jugador[conectado][jBanco], "", "Camara de seguridad via web");
    }
    else
    {
        Banco_Movimiento(cuentaId, 2, -CAMARA_PRECIO, banco - CAMARA_PRECIO, "", "Camara de seguridad via web");
    }

    format(g_Buffer, sizeof g_Buffer, "Camara instalada via web: cuenta %d, cancela %d, -%d$", cuentaId, cancelaId, CAMARA_PRECIO);
    EscribirLog("cancelas", g_Buffer);
    Web_MarcarTerminado(colaId, "ok", "Camara instalada. Ya esta vigilando la cancela.");
    return 1;
}

// ---------------------------------------------------------------------
//  DETECCION -- llamada cada segundo (TicSegundo, 99_callbacks.inc) por
//  cada jugador conectado. Recorre solo las cancelas con camara: a
//  CAMARA_RADIO_AVISO avisa por chat, a CAMARA_RADIO_DETECCION registra
//  nombre + matricula. Sin distinguir disfraces -- todo el que pase,
//  sale. El REGISTRO tiene cooldown por camara (cada cancela lleva su
//  propio historial, no se puede mezclar). El AVISO de chat tiene
//  cooldown GLOBAL por jugador, sin importar de que cancela venga --
//  si tienes varias cancelas juntas, el mensaje no se repite por cada
//  una, solo una vez cada CAMARA_COOLDOWN_SEGUNDOS en total.
// ---------------------------------------------------------------------
stock Camara_Segundo(playerid)
{
    new ahora = GetTickCount();

    for (new i = 0; i < MAX_CANCELAS; i++)
    {
        if (!g_Cancelas[i][caUsada] || !g_Cancelas[i][caTieneCamara]) continue;

        new Float:distancia = GetPlayerDistanceFromPoint(playerid, g_Cancelas[i][caX], g_Cancelas[i][caY], g_Cancelas[i][caZ]);
        if (distancia > CAMARA_RADIO_DETECCION) continue;

        if (g_Cancelas[i][caID] != Jugador[playerid][jCamaraRegistroCancela] || ahora > Jugador[playerid][jCamaraRegistroHasta])
        {
            Jugador[playerid][jCamaraRegistroCancela] = g_Cancelas[i][caID];
            Jugador[playerid][jCamaraRegistroHasta] = ahora + (CAMARA_COOLDOWN_SEGUNDOS * 1000);
            Camara_Registrar(playerid, g_Cancelas[i][caID]);
        }

        // Aviso IC de "esta cancela esta vigilada" RETIRADO el 08/09/2026
        // (peticion expresa de Vlad: cero avisos IC de camaras/cancelas).
        // El registro silencioso de arriba (nombre + matricula) sigue
        // intacto -- es la parte que de verdad sirve de prueba/RP, una
        // camara real tampoco te avisaria de que te esta grabando.
    }
    return 1;
}

stock Camara_Registrar(playerid, cancelaId)
{
    new matricula[12] = "-";
    new vehiculoIndice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (vehiculoIndice != -1) format(matricula, sizeof matricula, "%s", g_Vehiculos[vehiculoIndice][veMatricula]);

    new consulta[300];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO camara_eventos (cancela_id, cuenta_id, nombre, matricula) VALUES (%d, %d, '%e', '%e')",
        cancelaId, Jugador[playerid][jID], Jugador[playerid][jNombre], matricula);
    mysql_pquery(g_BD, consulta);
    return 1;
}
