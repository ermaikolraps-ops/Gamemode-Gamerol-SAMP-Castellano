/* =====================================================================
 *  33_servicios.inc
 *  ---------------------------------------------------------------------
 *  Catalogo de "Servicios" comprables desde el panel web. Dos categorias
 *  que comparten la misma tabla `servicios_jugador`:
 *
 *  1) AYUDANTE DE VENDEDOR (slots 0-4, categoria = CAT_AYUDANTE)
 *     Cinco tiers. Cada ciclo (SERVICIO_AYUDANTE_INTERVALO = 10 min)
 *     consume gramos de Marihuana e ingresa $ en el banco. Se pueden
 *     comprar varios en paralelo  se suman.
 *
 *  2) TRABAJO MEDIA JORNADA (slots 5-9, categoria = CAT_TRABAJO_MJ)
 *     Cinco negocios distintos (Burger Shot, Clucking Bell, Binco,
 *     Fabrica Sprunk, Vigilante de la Playa). 1.000$ de entrada,
 *     paga cada TRABAJO_MJ_INTERVALO segundos (24h reales).
 *     MAXIMO UNO POR JUGADOR  al comprar uno, se rechaza si ya tiene
 *     otro trabajo MJ activo. Auto-ascenso de rango cada 4 semanas
 *     reales (rango max 5), cada rango suma 25$/dia al sueldo base.
 *     Se puede renunciar via web (comando "renunciar_trabajo").
 *
 *  3) AGRICULTOR PERSONAL (slot 10, categoria = CAT_AGRICULTOR)
 *     10.000$ de entrada, una unica ranura. No usa seGramos/sePago (se
 *     dejan a 0) -- su logica completa (cosecha automatica de 10
 *     plantas listas + replante con semillas del inventario, en el
 *     mismo sitio) vive en 28_marihuana.inc, Agricultor_Tick(), porque
 *     necesita tocar directamente el array de plantas g_Plantas[].
 *     Aqui solo se cataloga el precio y se llama a su tick.
 *
 *  Todas las categorias las gestiona Servicios_Tick() (llamado cada
 *  minuto desde TicMinuto en 99_callbacks.inc). El pago/accion funciona
 *  este el jugador conectado o no.
 * ===================================================================== */

#define MAX_SERVICIOS                    11
#define SERVICIO_AYUDANTE_INTERVALO      600      // 10 min

#define TRABAJO_MJ_INTERVALO             86400    // 24 h reales
#define TRABAJO_MJ_DIAS_ASCENSO          28       // 4 semanas
#define TRABAJO_MJ_RANGO_MAX             5
#define TRABAJO_MJ_PAGO_POR_RANGO        500      // $ que suma cada ascenso al pago base

// Categorias  coinciden con el campo seCategoria en g_ServiciosDef.
#define CAT_AYUDANTE                     0
#define CAT_TRABAJO_MJ                   1
#define CAT_AGRICULTOR                   2

enum E_SERVICIO_DEF
{
    seNombre[32],
    sePrecio,             // $ de compra (una sola vez)
    seGramos,             // ayudante: gramos consumidos por ciclo. trabajo MJ: 0
    sePago,               // ayudante: $ por ciclo fijo. trabajo MJ: $ base por dia (rango 1)
    seCategoria,          // CAT_AYUDANTE o CAT_TRABAJO_MJ
    bool:seActivo         // false = ranura de catalogo libre
}

// Catalogo. Slots 0-4 = Ayudantes I-V. Slots 5-9 = Trabajos media jornada.
static const g_ServiciosDef[MAX_SERVICIOS][E_SERVICIO_DEF] =
{
    // ===== AYUDANTES DE VENDEDOR =====
    // Reequilibrados el 2026-08-19 (ver ECONOMIA.md en la raiz). Antes
    // vendian a 3 $/g fijo cuando el mercado paga 16 $/g: cada ciclo
    // destruian valor, y el tier V costaba 1.000.000 $ por perder 780 $
    // a la hora. Ahora pagan entre 8 y 10 $/g -- entre la mitad y dos
    // tercios del mercado. Lo que compras es NO tener que estar delante,
    // no un mejor precio, y cuanto mas caro el tier mejor el $/g.
    //
    // Estas cifras estan duplicadas a proposito en el catalogo de la web
    // (SERVICIOS_CATALOGO en "web servidor/server.js"). Si tocas una,
    // toca la otra o la web mentira al jugador.
    //
    // nombre                        precio,  gramos, pago,  cat,             activo
    {"Ayudante de Vendedor I",       20000,   5,      40,    CAT_AYUDANTE,    true},
    {"Ayudante de Vendedor II",      60000,   6,      52,    CAT_AYUDANTE,    true},
    {"Ayudante de Vendedor III",     150000,  7,      63,    CAT_AYUDANTE,    true},
    {"Ayudante de Vendedor IV",      300000,  8,      76,    CAT_AYUDANTE,    true},
    {"Ayudante de Vendedor V",       500000,  10,     100,   CAT_AYUDANTE,    true},

    // ===== TRABAJOS MEDIA JORNADA (2.000$/dia base, +500$ por rango, max 5) =====
    // Subidos el 2026-08-19 (ver ECONOMIA.md). Antes pagaban 75 $ cada 24 h
    // REALES: el 2% de una sola nomina, o sea decorativo. Ahora rinden entre
    // 2.000 y 4.000 $/dia, que es "me pago la gasolina y los papeles" -- una
    // renta pasiva de verdad para quien entra un rato al dia, sin competir
    // con la nomina ni con el cultivo. Sigue siendo UNO por jugador.
    // nombre                        precio,  gramos, pago,  cat,             activo
    {"Burger Shot",                  5000,    0,      2000,  CAT_TRABAJO_MJ,  true},
    {"Clucking Bell",                5000,    0,      2000,  CAT_TRABAJO_MJ,  true},
    {"Binco",                        5000,    0,      2000,  CAT_TRABAJO_MJ,  true},
    {"Fabrica Sprunk",               5000,    0,      2000,  CAT_TRABAJO_MJ,  true},
    {"Vigilante de la Playa",        5000,    0,      2000,  CAT_TRABAJO_MJ,  true},

    // ===== AGRICULTOR PERSONAL (logica completa en 28_marihuana.inc) =====
    // Subido de 10.000 a 50.000 $ el 2026-08-19: se amortizaba en la PRIMERA
    // cosecha (10 plantas x 100 g x 16 $/g = 16.000 $) y despues cosechaba y
    // replantaba gratis para siempre. A 50.000 $ sigue siendo compra obvia
    // para el cultivador serio, pero ya es una inversion.
    // nombre                        precio,  gramos, pago,  cat,             activo
    {"Agricultor Personal",          50000,   0,      0,     CAT_AGRICULTOR,  true}
};

// =====================================================================
//  TICK GENERAL  se llama una vez por minuto desde TicMinuto
// =====================================================================
stock Servicios_Tick()
{
    // 1) Ayudantes de vendedor (cada 10 min)
    new consultaAy[280];
    format(consultaAy, sizeof consultaAy,
        "SELECT id, cuenta_id, servicio_id, aviso_stock_enviado FROM servicios_jugador \
         WHERE activo = 1 AND servicio_id BETWEEN 0 AND 4 \
         AND ultimo_tick <= DATE_SUB(NOW(), INTERVAL %d SECOND)",
        SERVICIO_AYUDANTE_INTERVALO);
    mysql_pquery(g_BD, consultaAy, "Servicios_AlListarDebidosAyudantes", "");

    // 2) Trabajos media jornada (cada 24h)
    new consultaTr[260];
    format(consultaTr, sizeof consultaTr,
        "SELECT id, cuenta_id, servicio_id, rango, \
                UNIX_TIMESTAMP(ultimo_ascenso) AS ascenso_unix \
         FROM servicios_jugador \
         WHERE activo = 1 AND servicio_id BETWEEN 5 AND 9 \
         AND ultimo_tick <= DATE_SUB(NOW(), INTERVAL %d SECOND)",
        TRABAJO_MJ_INTERVALO);
    mysql_pquery(g_BD, consultaTr, "Servicios_AlListarDebidosTrabajos", "");

    // El Agricultor Personal NO va aqui -- tiene su propio timer por hora
    // (TicHora, 99_callbacks.inc), no cada minuto como esto. Logica
    // completa en 28_marihuana.inc porque necesita g_Plantas[] directamente.
    return 1;
}

// ---------------------------------------------------------------------
//  AYUDANTES
// ---------------------------------------------------------------------
forward Servicios_AlListarDebidosAyudantes();
public Servicios_AlListarDebidosAyudantes()
{
    new filas;
    cache_get_row_count(filas);
    for (new i = 0; i < filas; i++)
    {
        new servicioJugadorId, cuentaId, servicioId, avisoEnviado;
        cache_get_value_name_int(i, "id",                  servicioJugadorId);
        cache_get_value_name_int(i, "cuenta_id",           cuentaId);
        cache_get_value_name_int(i, "servicio_id",         servicioId);
        cache_get_value_name_int(i, "aviso_stock_enviado", avisoEnviado);
        Servicios_ProcesarAyudante(servicioJugadorId, cuentaId, servicioId, avisoEnviado);
    }
    return 1;
}

// Avisa por WhatsApp UNA sola vez por ciclo de "sin stock" (igual que
// veAvisado24h en 13_vehiculos.inc): marca aviso_stock_enviado=1 para no
// repetir el mensaje cada 10 min mientras el jugador no repone marihuana.
// Servicios_PagarAyudante lo resetea a 0 en cuanto vuelve a cobrar.
stock Servicios_AvisarSinStock(servicioJugadorId, cuentaId, servicioId)
{
    new mensaje[144];
    format(mensaje, sizeof mensaje,
        "Tu %s se ha quedado sin marihuana para cobrar. Repon marihuana para que siga pagando.",
        g_ServiciosDef[servicioId][seNombre]);
    Web_EncolarAviso(cuentaId, "servicio", mensaje);

    new consulta[150];
    format(consulta, sizeof consulta,
        "UPDATE servicios_jugador SET aviso_stock_enviado = 1 WHERE id = %d", servicioJugadorId);
    mysql_pquery(g_BD, consulta);
}

stock Servicios_ProcesarAyudante(servicioJugadorId, cuentaId, servicioId, avisoEnviado)
{
    if (servicioId < 0 || servicioId >= MAX_SERVICIOS) return 0;
    new gramos = g_ServiciosDef[servicioId][seGramos];
    if (gramos <= 0) return 0;

    // Marca el ciclo como consumido YA, pase lo que pase con el cobro.
    new consulta[150];
    format(consulta, sizeof consulta,
        "UPDATE servicios_jugador SET ultimo_tick = NOW() WHERE id = %d", servicioJugadorId);
    mysql_pquery(g_BD, consulta);

    new conectado = INVALID_PLAYER_ID;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jID] != cuentaId) continue;
        conectado = i;
        break;
    }

    if (conectado != INVALID_PLAYER_ID)
    {
        new item = Item_Buscar("Marihuana");
        if (Inv_TieneItem(conectado, item) < gramos)
        {
            format(g_Buffer, sizeof g_Buffer,
                "%s SIN COBRAR (sin marihuana suficiente, conectado): cuenta %d",
                g_ServiciosDef[servicioId][seNombre], cuentaId);
            EscribirLog("servicios", g_Buffer);
            if (!avisoEnviado) Servicios_AvisarSinStock(servicioJugadorId, cuentaId, servicioId);
            return 0;
        }
        Servicios_PagarAyudante(servicioJugadorId, cuentaId, conectado, servicioId);
        return 1;
    }

    new item = Item_Buscar("Marihuana");
    new consultaMaria[180];
    format(consultaMaria, sizeof consultaMaria,
        "SELECT COALESCE(SUM(cantidad),0) AS total FROM inventario WHERE cuenta_id = %d AND item = %d",
        cuentaId, item);
    mysql_pquery(g_BD, consultaMaria, "Servicios_AlComprobarMarihuana", "dddd", servicioJugadorId, cuentaId, servicioId, avisoEnviado);
    return 1;
}

forward Servicios_AlComprobarMarihuana(servicioJugadorId, cuentaId, servicioId, avisoEnviado);
public Servicios_AlComprobarMarihuana(servicioJugadorId, cuentaId, servicioId, avisoEnviado)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0) return 0;

    new total;
    cache_get_value_name_int(0, "total", total);

    new gramos = g_ServiciosDef[servicioId][seGramos];
    if (total < gramos)
    {
        format(g_Buffer, sizeof g_Buffer,
            "%s SIN COBRAR (solo %dg, hacen falta %d): cuenta %d",
            g_ServiciosDef[servicioId][seNombre], total, gramos, cuentaId);
        EscribirLog("servicios", g_Buffer);
        if (!avisoEnviado) Servicios_AvisarSinStock(servicioJugadorId, cuentaId, servicioId);
        return 0;
    }

    Servicios_PagarAyudante(servicioJugadorId, cuentaId, INVALID_PLAYER_ID, servicioId);
    return 1;
}

stock Servicios_PagarAyudante(servicioJugadorId, cuentaId, conectado, servicioId)
{
    new gramos = g_ServiciosDef[servicioId][seGramos];
    new pago   = g_ServiciosDef[servicioId][sePago];
    new item   = Item_Buscar("Marihuana");

    new consultaQuitar[220];
    format(consultaQuitar, sizeof consultaQuitar,
        "UPDATE inventario SET cantidad = cantidad - %d WHERE cuenta_id = %d AND item = %d \
         ORDER BY cantidad DESC LIMIT 1",
        gramos, cuentaId, item);
    mysql_pquery(g_BD, consultaQuitar);

    new consultaLimpiar[150];
    format(consultaLimpiar, sizeof consultaLimpiar,
        "DELETE FROM inventario WHERE cuenta_id = %d AND item = %d AND cantidad <= 0",
        cuentaId, item);
    mysql_pquery(g_BD, consultaLimpiar);

    new consultaBanco[120];
    format(consultaBanco, sizeof consultaBanco,
        "UPDATE cuentas SET banco = banco + %d WHERE id = %d", pago, cuentaId);
    mysql_pquery(g_BD, consultaBanco);

    // Vuelve a cobrar: si el ciclo anterior habia mandado el aviso de "sin
    // stock", se resetea para que el proximo hueco vuelva a avisar.
    new consultaReset[150];
    format(consultaReset, sizeof consultaReset,
        "UPDATE servicios_jugador SET aviso_stock_enviado = 0 WHERE id = %d AND aviso_stock_enviado = 1",
        servicioJugadorId);
    mysql_pquery(g_BD, consultaReset);

    if (conectado != INVALID_PLAYER_ID)
    {
        Jugador[conectado][jBanco] += pago;
        Banco_Movimiento(cuentaId, 1, pago, Jugador[conectado][jBanco], "", g_ServiciosDef[servicioId][seNombre]);
        Inv_QuitarItem(conectado, item, gramos);
    }

    // Sin aviso en el chat a proposito -- el Ayudante de Vendedor se
    // gestiona y se consulta desde el panel web, no hace falta duplicar
    // el aviso dentro del juego.
    format(g_Buffer, sizeof g_Buffer,
        "%s cobrado: cuenta %d, +%d$, -%dg marihuana",
        g_ServiciosDef[servicioId][seNombre], cuentaId, pago, gramos);
    EscribirLog("servicios", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  TRABAJOS MEDIA JORNADA
// ---------------------------------------------------------------------
forward Servicios_AlListarDebidosTrabajos();
public Servicios_AlListarDebidosTrabajos()
{
    new filas;
    cache_get_row_count(filas);
    new ahora = gettime();

    for (new i = 0; i < filas; i++)
    {
        new servicioJugadorId, cuentaId, servicioId, rango, ascensoUnix;
        cache_get_value_name_int(i, "id",           servicioJugadorId);
        cache_get_value_name_int(i, "cuenta_id",    cuentaId);
        cache_get_value_name_int(i, "servicio_id",  servicioId);
        cache_get_value_name_int(i, "rango",        rango);
        cache_get_value_name_int(i, "ascenso_unix", ascensoUnix);

        // Cobro base + bonus por rango
        new pago = g_ServiciosDef[servicioId][sePago] + (rango - 1) * TRABAJO_MJ_PAGO_POR_RANGO;

        // Marca el ciclo como consumido YA
        new q1[160];
        format(q1, sizeof q1,
            "UPDATE servicios_jugador SET ultimo_tick = NOW() WHERE id = %d", servicioJugadorId);
        mysql_pquery(g_BD, q1);

        // Ingresa el pago en el banco
        new q2[160];
        format(q2, sizeof q2,
            "UPDATE cuentas SET banco = banco + %d WHERE id = %d", pago, cuentaId);
        mysql_pquery(g_BD, q2);

        // Espejo en caliente si esta conectado
        new conectado = INVALID_PLAYER_ID;
        for (new p = 0; p < MAX_PLAYERS; p++)
        {
            if (!IsPlayerConnected(p) || !Jugador[p][jLogueado]) continue;
            if (Jugador[p][jID] != cuentaId) continue;
            conectado = p;
            break;
        }
        if (conectado != INVALID_PLAYER_ID)
        {
            Jugador[conectado][jBanco] += pago;
            Banco_Movimiento(cuentaId, 1, pago, Jugador[conectado][jBanco], "", g_ServiciosDef[servicioId][seNombre]);

            new dinero[24], mensaje[160];
            FormatearDinero(pago, dinero, sizeof dinero);
            format(mensaje, sizeof mensaje,
                "Tu sueldo diario de %s (rango %d) te ha ingresado %s en el banco.",
                g_ServiciosDef[servicioId][seNombre], rango, dinero);
            Exito(conectado, mensaje);
        }

        format(g_Buffer, sizeof g_Buffer,
            "Trabajo MJ cobrado: cuenta %d, %s (rango %d), +%d$",
            cuentaId, g_ServiciosDef[servicioId][seNombre], rango, pago);
        EscribirLog("servicios", g_Buffer);

        // Comprueba ascenso  cada TRABAJO_MJ_DIAS_ASCENSO dias reales
        if (rango < TRABAJO_MJ_RANGO_MAX)
        {
            new diasDesdeAscenso = (ahora - ascensoUnix) / 86400;
            if (diasDesdeAscenso >= TRABAJO_MJ_DIAS_ASCENSO)
            {
                new nuevoRango = rango + 1;
                new q3[200];
                format(q3, sizeof q3,
                    "UPDATE servicios_jugador SET rango = %d, ultimo_ascenso = NOW() WHERE id = %d",
                    nuevoRango, servicioJugadorId);
                mysql_pquery(g_BD, q3);

                if (conectado != INVALID_PLAYER_ID)
                {
                    new mensaje[160];
                    format(mensaje, sizeof mensaje,
                        "¡ASCENSO! Ahora eres rango %d en %s (+%d$/dia).",
                        nuevoRango, g_ServiciosDef[servicioId][seNombre], TRABAJO_MJ_PAGO_POR_RANGO);
                    Exito(conectado, mensaje);
                }

                format(g_Buffer, sizeof g_Buffer,
                    "Trabajo MJ ascenso: cuenta %d, %s rango %d -> %d",
                    cuentaId, g_ServiciosDef[servicioId][seNombre], rango, nuevoRango);
                EscribirLog("servicios", g_Buffer);
            }
        }
    }
    return 1;
}

// ---------------------------------------------------------------------
//  COMPRA DESDE LA WEB  comando "comprar_servicio"
// ---------------------------------------------------------------------
stock Servicios_Comprar(colaId, cuentaId, servicioId)
{
    if (servicioId < 0 || servicioId >= MAX_SERVICIOS || !g_ServiciosDef[servicioId][seActivo])
    {
        EscribirLog("servicios", "comprar_servicio rechazado: ranura invalida o vacia");
        Web_MarcarTerminado(colaId, "error", "Ese servicio no existe todavia.");
        return 0;
    }

    new consulta[150];
    format(consulta, sizeof consulta, "SELECT banco FROM cuentas WHERE id = %d LIMIT 1", cuentaId);
    mysql_pquery(g_BD, consulta, "Servicios_AlComprobarCompra", "ddd", colaId, cuentaId, servicioId);
    return 1;
}

forward Servicios_AlComprobarCompra(colaId, cuentaId, servicioId);
public Servicios_AlComprobarCompra(colaId, cuentaId, servicioId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Cuenta no encontrada.");
        return 0;
    }

    new banco;
    cache_get_value_name_int(0, "banco", banco);

    new precio = g_ServiciosDef[servicioId][sePrecio];
    if (banco < precio)
    {
        EscribirLog("servicios", "comprar_servicio rechazado: saldo insuficiente en el banco");
        Web_MarcarTerminado(colaId, "error", "Saldo insuficiente en el banco.");
        return 0;
    }

    // Para trabajos MJ hay que comprobar que no tenga OTRO trabajo activo.
    // Para ayudantes solo hay que comprobar que no tenga ESE mismo activo.
    if (g_ServiciosDef[servicioId][seCategoria] == CAT_TRABAJO_MJ)
    {
        new q[220];
        format(q, sizeof q,
            "SELECT servicio_id FROM servicios_jugador \
             WHERE cuenta_id = %d AND activo = 1 AND servicio_id BETWEEN 5 AND 9 LIMIT 1",
            cuentaId);
        mysql_pquery(g_BD, q, "Servicios_AlComprobarSinOtroTrabajo", "dddd", colaId, cuentaId, servicioId, precio);
    }
    else
    {
        new q[150];
        format(q, sizeof q,
            "SELECT activo FROM servicios_jugador WHERE cuenta_id = %d AND servicio_id = %d LIMIT 1",
            cuentaId, servicioId);
        mysql_pquery(g_BD, q, "Servicios_AlComprobarSiYaTiene", "dddd", colaId, cuentaId, servicioId, precio);
    }
    return 1;
}

forward Servicios_AlComprobarSinOtroTrabajo(colaId, cuentaId, servicioId, precio);
public Servicios_AlComprobarSinOtroTrabajo(colaId, cuentaId, servicioId, precio)
{
    new filas;
    cache_get_row_count(filas);
    if (filas > 0)
    {
        EscribirLog("servicios", "comprar_servicio (MJ) rechazado: ya tiene otro trabajo media jornada activo");
        Web_MarcarTerminado(colaId, "error", "Ya tienes un trabajo media jornada activo. Renuncia primero.");
        return 0;
    }
    Servicios_AplicarCompra(colaId, cuentaId, servicioId, precio);
    return 1;
}

forward Servicios_AlComprobarSiYaTiene(colaId, cuentaId, servicioId, precio);
public Servicios_AlComprobarSiYaTiene(colaId, cuentaId, servicioId, precio)
{
    new filas;
    cache_get_row_count(filas);
    if (filas > 0)
    {
        new activo;
        cache_get_value_name_int(0, "activo", activo);
        if (activo)
        {
            EscribirLog("servicios", "comprar_servicio rechazado: ya lo tiene activo");
            Web_MarcarTerminado(colaId, "error", "Ya tienes este servicio activo.");
            return 0;
        }
    }
    Servicios_AplicarCompra(colaId, cuentaId, servicioId, precio);
    return 1;
}

Servicios_AplicarCompra(colaId, cuentaId, servicioId, precio)
{
    new consulta[100];
    format(consulta, sizeof consulta, "UPDATE cuentas SET banco = banco - %d WHERE id = %d", precio, cuentaId);
    mysql_pquery(g_BD, consulta);

    // ON DUPLICATE KEY resetea todos los campos importantes  util si un
    // trabajo MJ se volvio a comprar tras renunciar (rango, ascenso, tick).
    new consultaUpsert[380];
    format(consultaUpsert, sizeof consultaUpsert,
        "INSERT INTO servicios_jugador (cuenta_id, servicio_id, activo, rango, comprado_en, ultimo_tick, ultimo_ascenso) \
         VALUES (%d, %d, 1, 1, NOW(), NOW(), NOW()) \
         ON DUPLICATE KEY UPDATE activo = 1, rango = 1, comprado_en = NOW(), ultimo_tick = NOW(), ultimo_ascenso = NOW()",
        cuentaId, servicioId);
    mysql_pquery(g_BD, consultaUpsert);

    new conectado = INVALID_PLAYER_ID;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jID] != cuentaId) continue;
        conectado = i;
        break;
    }

    if (conectado != INVALID_PLAYER_ID)
    {
        Jugador[conectado][jBanco] -= precio;
        Banco_Movimiento(cuentaId, 2, -precio, Jugador[conectado][jBanco], "", g_ServiciosDef[servicioId][seNombre]);
    }

    // Agricultor Personal: reclama YA hasta AGRICULTOR_LOTE de las
    // plantas que el jugador ya tenga plantadas (28_marihuana.inc). Si
    // tiene menos de 10, reclama las que tenga -- el resto se ira
    // rellenando solo segun plante mas, en el tick de cada hora.
    if (servicioId == AGRICULTOR_SERVICIO_ID) Agricultor_ReclamarInicial(cuentaId);

    format(g_Buffer, sizeof g_Buffer,
        "Servicio comprado via cola: cuenta %d, servicio %d (%s)",
        cuentaId, servicioId, g_ServiciosDef[servicioId][seNombre]);
    EscribirLog("servicios", g_Buffer);
    Web_MarcarTerminado(colaId, "ok", "Servicio activado.");
    return 1;
}

// ---------------------------------------------------------------------
//  RENUNCIA A TRABAJO MEDIA JORNADA  comando "renunciar_trabajo"
//  Marca el trabajo activo del jugador como inactivo. El rango se
//  pierde: al volver a contratar el mismo trabajo se empieza desde
//  rango 1 (Servicios_AplicarCompra resetea rango a 1 en el upsert).
// ---------------------------------------------------------------------
stock Trabajo_MJ_Renunciar(colaId, cuentaId)
{
    new consulta[200];
    format(consulta, sizeof consulta,
        "SELECT servicio_id FROM servicios_jugador \
         WHERE cuenta_id = %d AND activo = 1 AND servicio_id BETWEEN 5 AND 9 LIMIT 1",
        cuentaId);
    mysql_pquery(g_BD, consulta, "Trabajo_MJ_AlComprobarRenuncia", "dd", colaId, cuentaId);
    return 1;
}

forward Trabajo_MJ_AlComprobarRenuncia(colaId, cuentaId);
public Trabajo_MJ_AlComprobarRenuncia(colaId, cuentaId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "No tienes ningun trabajo media jornada activo.");
        return 0;
    }

    new servicioId;
    cache_get_value_name_int(0, "servicio_id", servicioId);

    new q[200];
    format(q, sizeof q,
        "UPDATE servicios_jugador SET activo = 0 \
         WHERE cuenta_id = %d AND servicio_id = %d",
        cuentaId, servicioId);
    mysql_pquery(g_BD, q);

    new conectado = INVALID_PLAYER_ID;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jID] != cuentaId) continue;
        conectado = i;
        break;
    }
    // Aviso ingame quitado (usuario prefiere que el panel no genere avisos)

    format(g_Buffer, sizeof g_Buffer,
        "Trabajo MJ renunciado: cuenta %d, %s",
        cuentaId, g_ServiciosDef[servicioId][seNombre]);
    EscribirLog("servicios", g_Buffer);
    Web_MarcarTerminado(colaId, "ok", "Trabajo dejado.");
    return 1;
}
