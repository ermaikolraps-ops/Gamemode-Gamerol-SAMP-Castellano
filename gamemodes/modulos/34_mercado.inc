/* =====================================================================
 *  34_mercado.inc
 *  ---------------------------------------------------------------------
 *  Mercado Libre: anuncios cortos para vender vehiculos propios desde
 *  el panel web (estilo Facebook Marketplace). Solo vehiculos por
 *  ahora -- casas, negocios e inventario llegaran mas adelante.
 *
 *  Todo el ciclo de vida pasa por la cola web_cola (31_web.inc), igual
 *  que renovar papeles o comprar un Servicio: la web solo pide, el
 *  gamemode es quien valida de verdad y toca el estado del juego.
 *
 *  Publicar (comando "mercado_publicar", args = anuncioId):
 *    1. La web ya inserto la fila en mercado_anuncios (estado
 *       'pendiente') con titulo/descripcion/foto -- eso es contenido
 *       puramente de la web, no hace falta que el gamemode lo valide.
 *    2. Aqui se comprueba que el vehiculo de verdad es tuyo, es
 *       personal (ni de trabajo/faccion/servidor, ni ya NFS) y no esta
 *       ya en otro anuncio. Si todo cuadra: se CONGELA (veEnVenta),
 *       se le pone el cartel "A LA VENTA", y el anuncio pasa a 'activo'
 *       con expira_en = ahora + 24h.
 *
 *  Comprar (comando "mercado_comprar", args = anuncioId):
 *    Cobra al comprador y paga al vendedor del BANCO de los dos (no
 *    hace falta que ninguno este conectado), transfiere la propiedad
 *    del vehiculo, lo descongela para el nuevo dueno y quita el cartel.
 *
 *  Cancelar (comando "mercado_cancelar", args = anuncioId):
 *    Solo el vendedor. Descongela el vehiculo tal cual estaba, sin
 *    cobrar ni pagar nada.
 *
 *  Caducidad (Mercado_Tick, llamado cada minuto desde TicMinuto):
 *    Cualquier anuncio 'activo' con expira_en ya pasado se descongela
 *    solo y vuelve a 'caducado' -- el vehiculo se lo queda tal cual su
 *    dueno de siempre, nadie pierde nada.
 * ===================================================================== */

#define MERCADO_DURACION_HORAS   24

// ---------------------------------------------------------------------
//  CONGELAR / DESCONGELAR -- comun a publicar, comprar, cancelar y
//  expirar. Solo toca el vehiculo (Veh_PuedeUsar ya rechaza cualquier
//  intento de uso mientras veEnVenta este a true); no toca dinero ni la
//  fila de mercado_anuncios, eso lo hace cada llamador.
//
//  El aviso "A LA VENTA" no es un cartel aparte: es una linea mas,
//  siempre la ultima, del mismo cartel del sistema de dropeo
//  (22_dropeo.inc, Dropeo_ActualizarCartel) -- por eso aqui solo hace
//  falta tocar el flag y refrescar ese cartel.
// ---------------------------------------------------------------------
stock Mercado_Congelar(indice)
{
    if (!VehiculoValido(indice)) return 0;

    g_Vehiculos[indice][veEnVenta] = true;
    Dropeo_ActualizarCartel(indice);

    // Si por lo que sea alguien esta dentro justo al publicar, lo saca.
    for (new p = 0; p < MAX_PLAYERS; p++)
    {
        if (!IsPlayerConnected(p)) continue;
        if (GetPlayerVehicleID(p) != g_Vehiculos[indice][veObjeto]) continue;
        RemovePlayerFromVehicle(p);
        if (Jugador[p][jLogueado]) Error(p, "Este vehiculo se acaba de publicar en el Mercado Libre.");
    }

    Veh_ActualizarCierreTodos(indice);
    Veh_Guardar(indice);
    return 1;
}

stock Mercado_Descongelar(indice)
{
    if (!VehiculoValido(indice)) return 0;

    g_Vehiculos[indice][veEnVenta] = false;
    Dropeo_ActualizarCartel(indice);

    Veh_ActualizarCierreTodos(indice);
    Veh_Guardar(indice);
    return 1;
}

// Busca el indice en memoria de un vehiculo por su id de base de datos.
stock Mercado_BuscarVehiculoPorID(vehiculoId)
{
    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veID] != vehiculoId) continue;
        return i;
    }
    return -1;
}

// Busca si una cuenta esta conectada ahora mismo. Mismo patron repetido
// en 33_servicios.inc / 31_web.inc; se queda aqui tal cual por
// consistencia con el resto del archivo.
stock Mercado_JugadorConectado(cuentaId)
{
    for (new p = 0; p < MAX_PLAYERS; p++)
    {
        if (!IsPlayerConnected(p) || !Jugador[p][jLogueado]) continue;
        if (Jugador[p][jID] != cuentaId) continue;
        return p;
    }
    return INVALID_PLAYER_ID;
}

// ---------------------------------------------------------------------
//  PUBLICAR  comando "mercado_publicar", args = anuncioId
// ---------------------------------------------------------------------
stock Mercado_Publicar(colaId, cuentaId, anuncioId)
{
    new consulta[150];
    format(consulta, sizeof consulta,
        "SELECT vehiculo_id, estado FROM mercado_anuncios WHERE id = %d AND vendedor_id = %d LIMIT 1",
        anuncioId, cuentaId);
    mysql_pquery(g_BD, consulta, "Mercado_AlComprobarPublicar", "ddd", colaId, cuentaId, anuncioId);
    return 1;
}

forward Mercado_AlComprobarPublicar(colaId, cuentaId, anuncioId);
public Mercado_AlComprobarPublicar(colaId, cuentaId, anuncioId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Ese anuncio no existe.");
        return 0;
    }

    new vehiculoId, estado[16];
    cache_get_value_name_int(0, "vehiculo_id", vehiculoId);
    cache_get_value_name(0, "estado", estado, sizeof estado);

    if (strcmp(estado, "pendiente", true))
    {
        Web_MarcarTerminado(colaId, "error", "Ese anuncio ya no esta pendiente de publicar.");
        return 0;
    }

    new indice = Mercado_BuscarVehiculoPorID(vehiculoId);
    new rechazo[100];
    new bool:rechazado = false;

    // Vendibles: VEH_PERSONAL (legado) y VEH_DROPEO con dueno (asi se
    // compran los coches de verdad hoy, por la calle con /comprarcoche --
    // ya no hay concesionario). vePropietario ya se comprobo arriba, asi
    // que un VEH_DROPEO que llega hasta aqui tiene dueno seguro.
    if (indice == -1) { format(rechazo, sizeof rechazo, "El vehiculo ya no existe."); rechazado = true; }
    else if (g_Vehiculos[indice][vePropietario] != cuentaId) { format(rechazo, sizeof rechazo, "Ese vehiculo ya no es tuyo."); rechazado = true; }
    else if (g_Vehiculos[indice][veTipo] != VEH_PERSONAL && g_Vehiculos[indice][veTipo] != VEH_DROPEO) { format(rechazo, sizeof rechazo, "Solo se pueden publicar vehiculos propios (no de trabajo, faccion o servidor)."); rechazado = true; }
    else if (g_Vehiculos[indice][veEsNFS]) { format(rechazo, sizeof rechazo, "Ese vehiculo esta marcado para NFS, no se puede publicar aqui."); rechazado = true; }
    else if (g_Vehiculos[indice][veEnVenta]) { format(rechazo, sizeof rechazo, "Ese vehiculo ya esta en otro anuncio."); rechazado = true; }
    // Coches de Jugadores (08/09/2026): ni publicable mientras Federico
    // lo busca/lo tiene escondido -- no queremos que cambie de dueno vía
    // el Mercado Libre mientras esta "en ronda".
    else if (FedJugadores_EstaProtegido(vehiculoId)) { format(rechazo, sizeof rechazo, "Ese vehiculo esta siendo buscado por Federico ahora mismo, no se puede publicar."); rechazado = true; }

    if (rechazado)
    {
        new q[100];
        format(q, sizeof q, "UPDATE mercado_anuncios SET estado = 'cancelado' WHERE id = %d", anuncioId);
        mysql_pquery(g_BD, q);
        Web_MarcarTerminado(colaId, "error", rechazo);
        return 0;
    }

    Mercado_Congelar(indice);

    new q2[200];
    format(q2, sizeof q2,
        "UPDATE mercado_anuncios SET estado = 'activo', expira_en = NOW() + INTERVAL %d HOUR WHERE id = %d",
        MERCADO_DURACION_HORAS, anuncioId);
    mysql_pquery(g_BD, q2);

    format(g_Buffer, sizeof g_Buffer, "Mercado Libre: anuncio %d publicado (vehiculo %d, cuenta %d)", anuncioId, vehiculoId, cuentaId);
    EscribirLog("mercado", g_Buffer);
    Web_MarcarTerminado(colaId, "ok", "Anuncio publicado.");
    return 1;
}

// ---------------------------------------------------------------------
//  COMPRAR  comando "mercado_comprar", args = anuncioId. cuentaId es
//  el COMPRADOR.
// ---------------------------------------------------------------------
stock Mercado_Comprar(colaId, cuentaId, anuncioId)
{
    new consulta[150];
    format(consulta, sizeof consulta,
        "SELECT vendedor_id, vehiculo_id, precio, estado FROM mercado_anuncios WHERE id = %d LIMIT 1",
        anuncioId);
    mysql_pquery(g_BD, consulta, "Mercado_AlComprobarCompra", "ddd", colaId, cuentaId, anuncioId);
    return 1;
}

forward Mercado_AlComprobarCompra(colaId, cuentaId, anuncioId);
public Mercado_AlComprobarCompra(colaId, cuentaId, anuncioId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Ese anuncio no existe.");
        return 0;
    }

    new vendedorId, vehiculoId, precio, estado[16];
    cache_get_value_name_int(0, "vendedor_id", vendedorId);
    cache_get_value_name_int(0, "vehiculo_id", vehiculoId);
    cache_get_value_name_int(0, "precio", precio);
    cache_get_value_name(0, "estado", estado, sizeof estado);

    if (strcmp(estado, "activo", true))
    {
        Web_MarcarTerminado(colaId, "error", "Ese anuncio ya no esta disponible.");
        return 0;
    }
    if (vendedorId == cuentaId)
    {
        Web_MarcarTerminado(colaId, "error", "No puedes comprar tu propio anuncio.");
        return 0;
    }

    new q[150];
    format(q, sizeof q, "SELECT nombre, banco FROM cuentas WHERE id = %d LIMIT 1", cuentaId);
    mysql_pquery(g_BD, q, "Mercado_AlConfirmarCompra", "dddddd", colaId, cuentaId, anuncioId, vendedorId, vehiculoId, precio);
    return 1;
}

forward Mercado_AlConfirmarCompra(colaId, cuentaId, anuncioId, vendedorId, vehiculoId, precio);
public Mercado_AlConfirmarCompra(colaId, cuentaId, anuncioId, vendedorId, vehiculoId, precio)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Cuenta no encontrada.");
        return 0;
    }

    new nombreComprador[MAX_PLAYER_NAME + 1], bancoComprador;
    cache_get_value_name(0, "nombre", nombreComprador, MAX_PLAYER_NAME + 1);
    cache_get_value_name_int(0, "banco", bancoComprador);

    if (bancoComprador < precio)
    {
        Web_MarcarTerminado(colaId, "error", "No te llega el saldo del banco.");
        return 0;
    }

    // Revalidacion en memoria (verdad fresca: Mercado_Congelar/Descongelar
    // siempre se llaman junto al cambio de estado, sin huecos async por
    // medio) -- cubre que el anuncio se haya vendido, cancelado o caducado
    // justo en el rato que ha tardado esta consulta en volver.
    new indice = Mercado_BuscarVehiculoPorID(vehiculoId);
    if (indice == -1 || !g_Vehiculos[indice][veEnVenta])
    {
        Web_MarcarTerminado(colaId, "error", "El vehiculo ya no esta disponible.");
        return 0;
    }

    // Coches de Jugadores: revalidacion de verdad justo antes de cerrar
    // la operacion (no basta con lo comprobado al publicar, encargo
    // explicito) -- por si Federico selecciono este vehiculo DESPUES de
    // que se publicara el anuncio, mientras la compra estaba en curso.
    if (FedJugadores_EstaProtegido(vehiculoId))
    {
        Web_MarcarTerminado(colaId, "error", "El vehiculo ya no esta disponible.");
        return 0;
    }

    // Dinero: se cobra al comprador y se paga al vendedor, del banco de
    // los dos, esten conectados o no.
    new qCobra[100];
    format(qCobra, sizeof qCobra, "UPDATE cuentas SET banco = banco - %d WHERE id = %d", precio, cuentaId);
    mysql_pquery(g_BD, qCobra);

    new qPaga[100];
    format(qPaga, sizeof qPaga, "UPDATE cuentas SET banco = banco + %d WHERE id = %d", precio, vendedorId);
    mysql_pquery(g_BD, qPaga);

    new compradorConectado = Mercado_JugadorConectado(cuentaId);
    new vendedorConectado  = Mercado_JugadorConectado(vendedorId);

    if (compradorConectado != INVALID_PLAYER_ID)
    {
        Jugador[compradorConectado][jBanco] = bancoComprador - precio;
        Banco_Movimiento(cuentaId, 2, -precio, Jugador[compradorConectado][jBanco], "", "Compra en el Mercado Libre");
    }
    else
    {
        Banco_Movimiento(cuentaId, 2, -precio, bancoComprador - precio, "", "Compra en el Mercado Libre");
    }

    if (vendedorConectado != INVALID_PLAYER_ID)
    {
        Jugador[vendedorConectado][jBanco] += precio;
        Banco_Movimiento(vendedorId, 1, precio, Jugador[vendedorConectado][jBanco], "", "Venta en el Mercado Libre");
    }

    // Traspaso de propiedad: se deja cerrado con llave, como cualquier
    // vehiculo personal recien adquirido.
    g_Vehiculos[indice][vePropietario] = cuentaId;
    format(g_Vehiculos[indice][vePropietarioNombre], MAX_PLAYER_NAME + 1, "%s", nombreComprador);
    g_Vehiculos[indice][veCerrado] = true;

    Mercado_Descongelar(indice);

    new q[150];
    format(q, sizeof q, "UPDATE mercado_anuncios SET estado = 'vendido', comprador_id = %d WHERE id = %d", cuentaId, anuncioId);
    mysql_pquery(g_BD, q);

    if (compradorConectado != INVALID_PLAYER_ID) Exito(compradorConectado, "Compra hecha. El vehiculo ya es tuyo.");
    if (vendedorConectado != INVALID_PLAYER_ID) Exito(vendedorConectado, "Han comprado tu anuncio del Mercado Libre.");

    format(g_Buffer, sizeof g_Buffer,
        "Mercado Libre: anuncio %d vendido (vehiculo %d, vendedor %d, comprador %d, %d$)",
        anuncioId, vehiculoId, vendedorId, cuentaId, precio);
    EscribirLog("mercado", g_Buffer);
    Web_MarcarTerminado(colaId, "ok", "Compra realizada.");
    return 1;
}

// ---------------------------------------------------------------------
//  CANCELAR  comando "mercado_cancelar", args = anuncioId. Solo el
//  propio vendedor.
// ---------------------------------------------------------------------
stock Mercado_Cancelar(colaId, cuentaId, anuncioId)
{
    new consulta[150];
    format(consulta, sizeof consulta,
        "SELECT vendedor_id, vehiculo_id, estado FROM mercado_anuncios WHERE id = %d LIMIT 1",
        anuncioId);
    mysql_pquery(g_BD, consulta, "Mercado_AlComprobarCancelar", "ddd", colaId, cuentaId, anuncioId);
    return 1;
}

forward Mercado_AlComprobarCancelar(colaId, cuentaId, anuncioId);
public Mercado_AlComprobarCancelar(colaId, cuentaId, anuncioId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Ese anuncio no existe.");
        return 0;
    }

    new vendedorId, vehiculoId, estado[16];
    cache_get_value_name_int(0, "vendedor_id", vendedorId);
    cache_get_value_name_int(0, "vehiculo_id", vehiculoId);
    cache_get_value_name(0, "estado", estado, sizeof estado);

    if (vendedorId != cuentaId)
    {
        Web_MarcarTerminado(colaId, "error", "Ese anuncio no es tuyo.");
        return 0;
    }
    if (strcmp(estado, "activo", true))
    {
        Web_MarcarTerminado(colaId, "error", "Ese anuncio ya no esta activo.");
        return 0;
    }

    new indice = Mercado_BuscarVehiculoPorID(vehiculoId);
    if (indice != -1) Mercado_Descongelar(indice);

    new q[100];
    format(q, sizeof q, "UPDATE mercado_anuncios SET estado = 'cancelado' WHERE id = %d", anuncioId);
    mysql_pquery(g_BD, q);

    format(g_Buffer, sizeof g_Buffer, "Mercado Libre: anuncio %d cancelado por el vendedor (vehiculo %d, cuenta %d)", anuncioId, vehiculoId, cuentaId);
    EscribirLog("mercado", g_Buffer);
    Web_MarcarTerminado(colaId, "ok", "Anuncio cancelado. El vehiculo vuelve a estar disponible.");
    return 1;
}

// ---------------------------------------------------------------------
//  CADUCIDAD  llamado una vez por minuto desde TicMinuto
//  (99_callbacks.inc), igual que Servicios_Tick.
// ---------------------------------------------------------------------
stock Mercado_Tick()
{
    mysql_pquery(g_BD,
        "SELECT id, vehiculo_id, vendedor_id FROM mercado_anuncios WHERE estado = 'activo' AND expira_en <= NOW()",
        "Mercado_AlListarCaducados");
    return 1;
}

forward Mercado_AlListarCaducados();
public Mercado_AlListarCaducados()
{
    new filas;
    cache_get_row_count(filas);
    for (new i = 0; i < filas; i++)
    {
        new anuncioId, vehiculoId, vendedorId;
        cache_get_value_name_int(i, "id", anuncioId);
        cache_get_value_name_int(i, "vehiculo_id", vehiculoId);
        cache_get_value_name_int(i, "vendedor_id", vendedorId);
        Mercado_ExpirarAnuncio(anuncioId, vehiculoId, vendedorId);
    }
    return 1;
}

stock Mercado_ExpirarAnuncio(anuncioId, vehiculoId, vendedorId)
{
    new indice = Mercado_BuscarVehiculoPorID(vehiculoId);
    if (indice != -1) Mercado_Descongelar(indice);

    new q[100];
    format(q, sizeof q, "UPDATE mercado_anuncios SET estado = 'caducado' WHERE id = %d", anuncioId);
    mysql_pquery(g_BD, q);

    new conectado = Mercado_JugadorConectado(vendedorId);
    if (conectado != INVALID_PLAYER_ID)
    {
        Info(conectado, "Tu anuncio del Mercado Libre ha caducado sin venderse. El vehiculo vuelve a estar disponible.");
    }

    format(g_Buffer, sizeof g_Buffer, "Mercado Libre: anuncio %d caducado (vehiculo %d, cuenta %d)", anuncioId, vehiculoId, vendedorId);
    EscribirLog("mercado", g_Buffer);
    return 1;
}
