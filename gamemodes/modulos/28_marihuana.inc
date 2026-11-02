/* =====================================================================
 *  28_marihuana.inc
 *  ---------------------------------------------------------------------
 *  Cultivo casero: siembra donde te de la gana (al aire libre), espera
 *  a que crezca y cosecha. Como es publico y sin proteccion, cualquiera
 *  puede arrancarte la planta si te descuidas -- ese es el riesgo de no
 *  tener un campo fijo y vigilado.
 *
 *  Version sencilla a proposito (nivel A): las plantas activas viven
 *  solo en memoria, no se guardan en base de datos. Si el servidor se
 *  reinicia, las plantas en la calle se pierden. Es una limitacion
 *  conocida, no un descuido -- se puede anadir persistencia mas
 *  adelante si hace falta (tabla nueva + migracion, igual que casas).
 *
 *  Venta: solo entre jugadores.
 *    - /vendermaria, /aceptarmaria: oferta directa a otro jugador,
 *      mismo patron de "oferta pendiente" que /venderarma (25_vendedorarmas.inc).
 *
 *  La venta a clientes NPC (con ofertas automaticas, tiempo limite y
 *  reparto por la ciudad) es otro sistema aparte: el trabajo de Vendedor
 *  de Marihuana, ranura 3, logica completa en 29_vendedormarihuana.inc.
 * ===================================================================== */

// MARIA_MAX_PLANTAS es el tope de plantas simultaneas en TODO el
// servidor (Pawn necesita un array de tamano fijo), puesto muy alto para
// que en la practica no se note.
#define MARIA_MAX_PLANTAS       5000

// Tope POR JUGADOR (2026-08-19). Antes no habia ninguno, y eso convertia
// el ciclo semilla -> planta -> gramos en una fuente de dinero sin techo:
// el limite lo ponia la paciencia del jugador al plantar, no el juego.
// Con 30 plantas el techo son 3.000 g/dia (30 * MARIA_RENDIMIENTO), que a
// 16 $/g son 48.000 $/dia -- rentable de sobra como oficio, pero acotado.
// Ver ECONOMIA.md en la raiz del servidor para el analisis completo.
//
// Solo se comprueba en CMD:plantar, NO en Maria_PlantarEn: asi el
// Agricultor Personal (33_servicios.inc) puede seguir replantando en el
// hueco de una planta que acaba de cosechar, donde el neto no sube.
#define MARIA_MAX_POR_JUGADOR   30
#define MARIA_TIEMPO_CRECER     86400   // segundos hasta estar lista (24 horas)
#define MARIA_RENDIMIENTO       100     // gramos de "Marihuana" por cosecha
#define MARIA_RADIO_ACCION      3.0     // distancia para plantar/cosechar/arrancar

// Modelo de objeto para la planta en el suelo. Pedido expresamente:
// 19473 ("grassplant01").
#define MARIA_MODELO_PLANTA     19473

// Cuanto se hunde el objeto respecto a la posicion del jugador para que
// no quede flotando. Con 0.3 seguia flotando, asi que el pivote del
// modelo debe estar mas arriba de lo que pensaba (posiblemente cerca
// del centro de la malla, no de la base) -- subido a 0.9 como siguiente
// intento razonado. Si con esto se entierra de mas, hay que bajarlo; si
// sigue flotando, subirlo mas todavia. Es el unico numero a tocar.
#define MARIA_HUNDIDO_Z          0.9

// ---------------------------------------------------------------------
//  PORRO: /fumar lia uno con 2 gramos de Marihuana y lo deja liado en
//  la mano. Cada click (KEY_FIRE, enganchado en OnPlayerKeyStateChange
//  de 99_callbacks.inc) es una calada, hasta que se acaba.
// ---------------------------------------------------------------------
#define PORRO_GRAMOS_NECESARIOS  2
#define PORRO_CALADAS            10      // caladas antes de que se acabe
#define PORRO_TIEMPO_LIAR        2000    // ms del GameText "Liando..."
#define PORRO_TIEMPO_CALADA      1500    // ms que se fuerza la animacion de calada (el clip completo dura 7.5s)
#define PORRO_COOLDOWN_MS        1500    // minimo entre dos caladas, para que no se pueda spamear el click
#define PORRO_EFECTO_MS          20000   // cuanto tarda en pasarse el "colocado" tras la ultima calada

#define PORRO_SLOT               1       // ranura de SetPlayerAttachedObject (0-9); no la usa ningun otro sistema
#define PORRO_MODELO             3044    // CIGAR.dff, pedido expresamente
#define PORRO_BONE               6       // mano derecha, esqueleto estandar de CJ

// Offset/rotacion de partida para que el porro quede entre los dedos de
// la mano derecha. Es una primera aproximacion, no un valor verificado
// -- hay que afinarlos en el juego, igual que con MARIA_HUNDIDO_Z aqui
// arriba. Si sale flotando, girado o metido en la palma, tocar aqui.
#define PORRO_OFFSET_X           0.03
#define PORRO_OFFSET_Y           0.02
#define PORRO_OFFSET_Z           0.01
#define PORRO_ROT_X              -40.0
#define PORRO_ROT_Y              0.0
#define PORRO_ROT_Z              0.0

// Clima y hora "normales" del servidor (los mismos que pone
// OnGameModeInit al arrancar) para poder devolver a un jugador a su
// estado normal cuando se le pasa el efecto.
#define PORRO_CLIMA_NORMAL       2
#define PORRO_HORA_NORMAL        12

// Convierte una cantidad en gramos a texto: por debajo de 1000 en
// gramos, de ahi para arriba en kg con dos decimales.
stock Maria_FormatearGramos(cantidad, destino[], tam)
{
    if (cantidad >= 1000)
    {
        format(destino, tam, "%.2f kg", float(cantidad) / 1000.0);
    }
    else
    {
        format(destino, tam, "%d gramos", cantidad);
    }
    return 1;
}

enum E_PLANTA
{
    bool:plUsada,
    plID,                                 // fila en la tabla `plantas`
    plDueno,                             // Jugador[playerid][jID] del dueno
    plDuenoNombre[MAX_PLAYER_NAME + 1],
    Float:plX, Float:plY, Float:plZ,
    plHoraLista,                         // gettime() en que estara lista
    bool:plLista,
    plRaza[16],                          // sorteada al plantar, ver g_MariaRazas
    bool:plAgricultor,                   // true si la planto/replanto el Agricultor Personal
    STREAMER_TAG_OBJECT:plObjeto,
    STREAMER_TAG_3D_TEXT_LABEL:plEtiqueta
};
new g_Plantas[MARIA_MAX_PLANTAS][E_PLANTA];

// Razas posibles, sorteada una al azar cada vez que se planta.
new const g_MariaRazas[][16] = { "WhiteWidow", "Amnesia", "CriticalMass", "Blueberry", "Ak47" };
#define TOTAL_MARIA_RAZAS (sizeof (g_MariaRazas))

// Cuatro estados de crecimiento, 6 horas cada uno (24 h / 4), de menor a
// mayor segun avanzan las horas desde que se planto.
new const g_MariaEstados[][16] = { "Crecimiento", "Formandose", "Floreciendo", "Madurando" };

// Oferta de venta pendiente, guardada en el jugador que la RECIBE
// (mismo patron que gArmas[][aaOferta*] en 25_vendedorarmas.inc).
static g_MariaOfertaDe[MAX_PLAYERS];
static g_MariaOfertaCantidad[MAX_PLAYERS];
static g_MariaOfertaPrecio[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  ARRANQUE
// ---------------------------------------------------------------------
stock Maria_Iniciar()
{
    print("  [MARIHUANA] Sistema de cultivo listo.");
    return 1;
}

// ---------------------------------------------------------------------
//  CARGA AL ARRANCAR: recupera las plantas que quedaron en la calle la
//  ultima vez que se apago el servidor, para que no desaparezcan.
// ---------------------------------------------------------------------
stock Maria_CargarTodas()
{
    mysql_pquery(g_BD, "SELECT * FROM plantas ORDER BY id ASC", "AlCargarPlantas");
    return 1;
}

forward AlCargarPlantas();
public AlCargarPlantas()
{
    new filas;
    cache_get_row_count(filas);

    new texto[160];

    for (new i = 0; i < filas && i < MARIA_MAX_PLANTAS; i++)
    {
        g_Plantas[i][plUsada] = true;
        cache_get_value_name_int(i, "id", g_Plantas[i][plID]);
        cache_get_value_name_int(i, "propietario", g_Plantas[i][plDueno]);
        cache_get_value_name(i, "propietario_nombre", g_Plantas[i][plDuenoNombre], MAX_PLAYER_NAME + 1);
        cache_get_value_name_float(i, "x", g_Plantas[i][plX]);
        cache_get_value_name_float(i, "y", g_Plantas[i][plY]);
        cache_get_value_name_float(i, "z", g_Plantas[i][plZ]);
        cache_get_value_name_int(i, "hora_lista", g_Plantas[i][plHoraLista]);
        cache_get_value_name(i, "raza", g_Plantas[i][plRaza], 16);
        new agricultorTemp;
        cache_get_value_name_int(i, "agricultor", agricultorTemp);
        g_Plantas[i][plAgricultor] = bool:(agricultorTemp != 0);
        g_Plantas[i][plLista] = (gettime() >= g_Plantas[i][plHoraLista]);

        g_Plantas[i][plObjeto] = CreateDynamicObject(MARIA_MODELO_PLANTA,
            g_Plantas[i][plX], g_Plantas[i][plY], g_Plantas[i][plZ] - MARIA_HUNDIDO_Z, 0.0, 0.0, 0.0, 0, 0, -1, 100.0);

        Maria_TextoEtiqueta(i, texto, sizeof texto);
        g_Plantas[i][plEtiqueta] = CreateDynamic3DTextLabel(texto, 0xFFFFFFFF,
            g_Plantas[i][plX], g_Plantas[i][plY], g_Plantas[i][plZ] + 0.6, 8.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 15.0);
    }

    printf("  [MARIHUANA] %d plantas recuperadas de la base de datos.", filas);
    return 1;
}

// ---------------------------------------------------------------------
//  TEXTO DEL CARTEL: nombre del dueno en rojo, debajo en azul el tiempo
//  que queda (o el aviso de que ya esta lista). Mismo estilo que los
//  carteles de los coches (13_vehiculos.inc / 22_dropeo.inc), con
//  colores propios pedidos para esta planta.
// ---------------------------------------------------------------------
stock Maria_TextoEtiqueta(indice, texto[], tam)
{
    if (g_Plantas[indice][plLista])
    {
        format(texto, tam, "{FF3232}%s\n{3399FF}Lista para cosechar\n{E8B33A}/cosechar\n{E8973A}%s",
            g_Plantas[indice][plDuenoNombre], g_Plantas[indice][plRaza]);
    }
    else
    {
        new restante = g_Plantas[indice][plHoraLista] - gettime();
        if (restante < 0) restante = 0;

        new horas = restante / 3600;
        new minutos = (restante % 3600) / 60;

        // Cuatro estados de 6 horas cada uno (24 h / 4), segun lo transcurrido
        // desde que se planto, no segun lo que falta.
        new transcurrido = MARIA_TIEMPO_CRECER - restante;
        new estadoIdx = transcurrido / (MARIA_TIEMPO_CRECER / 4);
        if (estadoIdx < 0) estadoIdx = 0;
        if (estadoIdx > 3) estadoIdx = 3;

        format(texto, tam, "{FF3232}%s\n{3399FF}%dh %d Minutos\n{E8973A}%s\n{F2C94C}%s",
            g_Plantas[indice][plDuenoNombre], horas, minutos, g_Plantas[indice][plRaza], g_MariaEstados[estadoIdx]);
    }

    // Linea extra, siempre la ultima, solo si la planto/replanto el
    // Agricultor Personal (33_servicios.inc) en vez de un /plantar manual.
    if (g_Plantas[indice][plAgricultor])
    {
        strcat(texto, "\n{8a6fd8}*Agricultor Personal", tam);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  CUANTAS PLANTAS TIENE UNA CUENTA AHORA MISMO
//  Cuenta sobre el array en memoria, que es espejo de la tabla `plantas`
//  (se carga entera al arrancar y se mantiene al dia). Incluye las que
//  haya puesto el Agricultor Personal: son suyas igual.
// ---------------------------------------------------------------------
stock Maria_ContarDeJugador(cuentaId)
{
    new total = 0;

    for (new i = 0; i < MARIA_MAX_PLANTAS; i++)
    {
        if (!g_Plantas[i][plUsada]) continue;
        if (g_Plantas[i][plDueno] != cuentaId) continue;
        total++;
    }
    return total;
}

// Total de plantas vivas en TODO el servidor (creciendo + maduras), para
// El Teletipo (62_teletipo.inc) -- misma logica que Maria_ContarDeJugador
// pero sin filtrar por dueno. No existe estado "expirada": una planta
// madura se queda madura para siempre hasta que la cosechen o la arranquen.
stock Maria_ContarVivasServidor()
{
    new total = 0;

    for (new i = 0; i < MARIA_MAX_PLANTAS; i++)
    {
        if (g_Plantas[i][plUsada]) total++;
    }
    return total;
}

// ---------------------------------------------------------------------
//  COMPROBACION DE CRECIMIENTO Y REFRESCO DEL CARTEL
//  (llamado desde TicMinuto, 99_callbacks.inc)
// ---------------------------------------------------------------------
stock Maria_ComprobarPlantas()
{
    new texto[160];

    for (new i = 0; i < MARIA_MAX_PLANTAS; i++)
    {
        if (!g_Plantas[i][plUsada]) continue;

        if (!g_Plantas[i][plLista] && gettime() >= g_Plantas[i][plHoraLista])
        {
            g_Plantas[i][plLista] = true;
            Web_EncolarAviso(g_Plantas[i][plDueno], "cosecha", "Tu planta ya esta lista para cosechar.");
        }

        Maria_TextoEtiqueta(i, texto, sizeof texto);
        UpdateDynamic3DTextLabelText(g_Plantas[i][plEtiqueta], 0xFFFFFFFF, texto);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  CONSULTA: planta mas cercana dentro de un radio, o -1
// ---------------------------------------------------------------------
stock Maria_MasCercana(playerid, Float:radio)
{
    new indice = -1;
    new Float:mejor = radio, Float:distancia;

    for (new i = 0; i < MARIA_MAX_PLANTAS; i++)
    {
        if (!g_Plantas[i][plUsada]) continue;

        distancia = GetPlayerDistanceFromPoint(playerid, g_Plantas[i][plX], g_Plantas[i][plY], g_Plantas[i][plZ]);
        if (distancia < mejor) { mejor = distancia; indice = i; }
    }
    return indice;
}

// Herramienta de admin (panel, ADMIN_GESTION+): fuerza que TODAS las
// plantas activas del servidor pasen a "lista para cosechar" ya mismo.
// Tambien ajusta hora_lista en la base de datos para que la cosecha
// instantanea sobreviva a un reinicio, no solo en memoria.
stock Maria_ForzarTodasListas()
{
    new texto[160], total = 0, ahora = gettime();

    for (new i = 0; i < MARIA_MAX_PLANTAS; i++)
    {
        if (!g_Plantas[i][plUsada]) continue;

        g_Plantas[i][plHoraLista] = ahora;
        g_Plantas[i][plLista] = true;
        total++;

        Maria_TextoEtiqueta(i, texto, sizeof texto);
        UpdateDynamic3DTextLabelText(g_Plantas[i][plEtiqueta], 0xFFFFFFFF, texto);

        if (g_Plantas[i][plID] > 0)
        {
            new consulta[80];
            format(consulta, sizeof consulta, "UPDATE plantas SET hora_lista = %d WHERE id = %d", ahora, g_Plantas[i][plID]);
            mysql_pquery(g_BD, consulta);
        }
    }
    return total;
}

stock Maria_Destruir(indice)
{
    if (indice < 0 || indice >= MARIA_MAX_PLANTAS || !g_Plantas[indice][plUsada]) return 0;

    DestroyDynamicObject(g_Plantas[indice][plObjeto]);
    DestroyDynamic3DTextLabel(g_Plantas[indice][plEtiqueta]);

    if (g_Plantas[indice][plID] > 0)
    {
        new consulta[64];
        format(consulta, sizeof consulta, "DELETE FROM plantas WHERE id = %d", g_Plantas[indice][plID]);
        mysql_pquery(g_BD, consulta);
    }

    g_Plantas[indice][plUsada] = false;
    return 1;
}

// ---------------------------------------------------------------------
//  PLANTAR / COSECHAR / ARRANCAR
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
//  PLANTAR EN UNA POSICION CONCRETA, a nombre de una cuenta -- sin
//  depender de que haya un playerid conectado. Es el nucleo que usa
//  CMD:plantar (traduce desde el jugador que escribe el comando) y
//  tambien el Agricultor Personal (33_servicios.inc, via Agricultor_Tick
//  aqui abajo) para replantar automaticamente en el mismo sitio que
//  acaba de cosechar. Quien llama es responsable de haber quitado ya la
//  semilla del inventario -- esta funcion no toca inventario para nada.
//  Devuelve el indice de la planta nueva, o -1 si no habia hueco libre.
// ---------------------------------------------------------------------
stock Maria_PlantarEn(cuentaId, const nombreDueno[], Float:x, Float:y, Float:z, bool:esAgricultor = false)
{
    new indice = -1;
    for (new i = 0; i < MARIA_MAX_PLANTAS; i++)
    {
        if (!g_Plantas[i][plUsada]) { indice = i; break; }
    }
    if (indice == -1) return -1;

    g_Plantas[indice][plUsada] = true;
    g_Plantas[indice][plID] = 0;
    g_Plantas[indice][plDueno] = cuentaId;
    format(g_Plantas[indice][plDuenoNombre], MAX_PLAYER_NAME + 1, "%s", nombreDueno);
    g_Plantas[indice][plX] = x;
    g_Plantas[indice][plY] = y;
    g_Plantas[indice][plZ] = z;
    g_Plantas[indice][plHoraLista] = gettime() + MARIA_TIEMPO_CRECER;
    g_Plantas[indice][plLista] = false;
    format(g_Plantas[indice][plRaza], 16, "%s", g_MariaRazas[random(TOTAL_MARIA_RAZAS)]);
    g_Plantas[indice][plAgricultor] = esAgricultor;

    // Se hunde MARIA_HUNDIDO_Z para que no quede flotando (ver comentario
    // junto a la constante, arriba del todo del archivo).
    g_Plantas[indice][plObjeto] = CreateDynamicObject(MARIA_MODELO_PLANTA, x, y, z - MARIA_HUNDIDO_Z, 0.0, 0.0, 0.0, 0, 0, -1, 100.0);

    new texto[160];
    Maria_TextoEtiqueta(indice, texto, sizeof texto);
    g_Plantas[indice][plEtiqueta] = CreateDynamic3DTextLabel(texto, 0xFFFFFFFF,
        x, y, z + 0.6, 8.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 15.0);

    // Persistencia: se guarda ya mismo para que sobreviva a un reinicio
    // aunque el servidor se caiga antes de que se cumplan las 24 horas.
    new consulta[320];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO plantas (propietario, propietario_nombre, x, y, z, hora_lista, raza, agricultor) VALUES (%d, '%e', %f, %f, %f, %d, '%e', %d)",
        cuentaId, nombreDueno, x, y, z, g_Plantas[indice][plHoraLista], g_Plantas[indice][plRaza], _:esAgricultor);
    mysql_pquery(g_BD, consulta, "AlPlantarMaria", "d", indice);

    return indice;
}

CMD:plantar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerInterior(playerid) != 0)
    {
        Error(playerid, "Aqui dentro no puedes plantar nada. Tiene que ser al aire libre.");
        return 1;
    }

    new item = Item_Buscar("Semilla de marihuana");
    if (!Inv_TieneItem(playerid, item))
    {
        Error(playerid, "Necesitas una semilla de marihuana para plantar.");
        return 1;
    }

    // Tope por jugador. Se comprueba ANTES de gastar la semilla, para no
    // cobrarsela a quien no va a poder plantar.
    new tuyas = Maria_ContarDeJugador(Jugador[playerid][jID]);
    if (tuyas >= MARIA_MAX_POR_JUGADOR)
    {
        Formato("Ya tienes %d plantas y el maximo es %d. Cosecha o arranca alguna antes de plantar mas.", tuyas, MARIA_MAX_POR_JUGADOR);
        Error(playerid, g_Buffer);
        return 1;
    }

    Inv_QuitarItem(playerid, item, 1);

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    new nombreDueno[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombreDueno, sizeof nombreDueno);

    if (Maria_PlantarEn(Jugador[playerid][jID], nombreDueno, x, y, z) == -1)
    {
        Inv_DarItem(playerid, item, 1, 0); // no habia hueco libre -- se devuelve la semilla
        Error(playerid, "Hay demasiadas plantas sueltas por ahi ahora mismo. Prueba mas tarde.");
        return 1;
    }

    Exito(playerid, "Has plantado una semilla. Tardara 24 horas en crecer -- si te alejas mucho, cualquiera te la puede arrancar.");
    return 1;
}

forward AlPlantarMaria(indice);
public AlPlantarMaria(indice)
{
    if (indice < 0 || indice >= MARIA_MAX_PLANTAS || !g_Plantas[indice][plUsada]) return 0;
    g_Plantas[indice][plID] = cache_insert_id();
    return 1;
}

CMD:cosechar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new indice = Maria_MasCercana(playerid, MARIA_RADIO_ACCION);
    if (indice == -1)
    {
        Error(playerid, "No hay ninguna planta cerca.");
        return 1;
    }

    if (g_Plantas[indice][plDueno] != Jugador[playerid][jID])
    {
        Error(playerid, "Esta planta no es tuya. Puedes arrancarsela a su dueno con /arrancar.");
        return 1;
    }

    if (!g_Plantas[indice][plLista])
    {
        Error(playerid, "Todavia no esta lista para cosechar.");
        return 1;
    }

    new item = Item_Buscar("Marihuana");
    if (Inv_DarItem(playerid, item, MARIA_RENDIMIENTO, 0) <= 0)
    {
        Error(playerid, "No te cabe la cosecha en el inventario ahora mismo.");
        return 1;
    }

    Maria_Destruir(indice);

    // Contador persistente para el panel web.
    new consultaStat[110];
    format(consultaStat, sizeof consultaStat, "UPDATE cuentas SET maria_plantas_cosechadas = maria_plantas_cosechadas + 1 WHERE id = %d", Jugador[playerid][jID]);
    mysql_pquery(g_BD, consultaStat);

    new mensaje[64], gramos[16];
    Maria_FormatearGramos(MARIA_RENDIMIENTO, gramos, sizeof gramos);
    format(mensaje, sizeof mensaje, "Has cosechado %s de marihuana.", gramos);
    Exito(playerid, mensaje);

    // Aviso a la policia (37_policia.inc). No siempre salta: la
    // probabilidad y la agrupacion por cercania las decide alli.
    new Float:avx, Float:avy, Float:avz;
    GetPlayerPos(playerid, avx, avy, avz);
    Policia_CrearAviso(AVISO_CULTIVO, playerid, avx, avy, avz, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));
    return 1;
}

CMD:arrancar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new indice = Maria_MasCercana(playerid, MARIA_RADIO_ACCION);
    if (indice == -1)
    {
        Error(playerid, "No hay ninguna planta cerca.");
        return 1;
    }

    if (g_Plantas[indice][plDueno] == Jugador[playerid][jID])
    {
        Error(playerid, "Esta planta es tuya -- usa /cosechar si ya esta lista.");
        return 1;
    }

    if (g_Plantas[indice][plLista])
    {
        new item = Item_Buscar("Marihuana");
        if (Inv_DarItem(playerid, item, MARIA_RENDIMIENTO, 0) <= 0)
        {
            Error(playerid, "No te cabe en el inventario ahora mismo.");
            return 1;
        }
        Exito(playerid, "Le has arrancado la planta a otro y te has quedado con la cosecha. Que se espabile la proxima vez.");
    }
    else
    {
        Exito(playerid, "Has arrancado la planta antes de que madurase. No queda nada aprovechable.");
    }

    Maria_Destruir(indice);
    return 1;
}

// ---------------------------------------------------------------------
//  VENTA JUGADOR A JUGADOR: /vendermaria + /aceptarmaria
//  Mismo patron de "oferta pendiente" que /venderarma en
//  25_vendedorarmas.inc: la oferta se guarda en el jugador que la
//  recibe, y solo se cierra el trato cuando la acepta.
// ---------------------------------------------------------------------
CMD:vendermaria(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new objetivoTexto[MAX_PLAYER_NAME + 1], cantidad, precio;
    if (sscanf(params, "s[24]dd", objetivoTexto, cantidad, precio))
    {
        Uso(playerid, "/vendermaria [jugador] [cantidad] [precio]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (objetivo == INVALID_PLAYER_ID || objetivo == playerid)
    {
        Error(playerid, "Ese jugador no esta conectado.");
        return 1;
    }

    if (!EstaCerca(playerid, objetivo, ARMAS_OFERTA_RADIO))
    {
        Error(playerid, "Tienes que estar cerca de esa persona para ofrecerle nada.");
        return 1;
    }

    if (cantidad < 1)
    {
        Error(playerid, "La cantidad tiene que ser al menos 1.");
        return 1;
    }

    new item = Item_Buscar("Marihuana");
    if (Inv_TieneItem(playerid, item) < cantidad)
    {
        Error(playerid, "No llevas encima esa cantidad de marihuana.");
        return 1;
    }

    if (precio < 0)
    {
        Error(playerid, "El precio no puede ser negativo.");
        return 1;
    }

    g_MariaOfertaDe[objetivo] = playerid;
    g_MariaOfertaCantidad[objetivo] = cantidad;
    g_MariaOfertaPrecio[objetivo] = precio;

    new nombre[MAX_PLAYER_NAME + 1], mensaje[144], dinero[24], gramos[16];
    NombreRP(playerid, nombre, sizeof nombre);
    FormatearDinero(precio, dinero, sizeof dinero);
    Maria_FormatearGramos(cantidad, gramos, sizeof gramos);

    format(mensaje, sizeof mensaje, "%s te ofrece %s de marihuana por %s. Escribe /aceptarmaria para cerrar el trato.",
        nombre, gramos, dinero);
    Info(objetivo, mensaje);

    Exito(playerid, "Oferta enviada. Si la acepta, se cerrara el trato.");
    return 1;
}

CMD:aceptarmaria(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (g_MariaOfertaCantidad[playerid] <= 0)
    {
        Error(playerid, "No tienes ninguna oferta de marihuana pendiente.");
        return 1;
    }

    new vendedor = g_MariaOfertaDe[playerid];
    new cantidad = g_MariaOfertaCantidad[playerid];
    new precio = g_MariaOfertaPrecio[playerid];

    if (!IsPlayerConnected(vendedor) || !EstaCerca(playerid, vendedor, ARMAS_OFERTA_RADIO))
    {
        Error(playerid, "El vendedor ya no esta cerca. La oferta se ha cancelado.");
        g_MariaOfertaCantidad[playerid] = 0;
        return 1;
    }

    new item = Item_Buscar("Marihuana");
    if (Inv_TieneItem(vendedor, item) < cantidad)
    {
        Error(playerid, "El vendedor ya no tiene esa cantidad encima.");
        g_MariaOfertaCantidad[playerid] = 0;
        return 1;
    }

    if (!TieneDinero(playerid, precio))
    {
        Error(playerid, "No tienes dinero suficiente para pagar.");
        g_MariaOfertaCantidad[playerid] = 0;
        return 1;
    }

    Inv_QuitarItem(vendedor, item, cantidad);
    if (Inv_DarItem(playerid, item, cantidad, 0) <= 0)
    {
        Error(playerid, "No te cabe esa cantidad en el inventario. Trato cancelado.");
        Inv_DarItem(vendedor, item, cantidad, 0);
        g_MariaOfertaCantidad[playerid] = 0;
        return 1;
    }

    DarDinero(playerid, -precio);
    DarDinero(vendedor, precio);
    g_MariaOfertaCantidad[playerid] = 0;

    new dinero[24];
    FormatearDinero(precio, dinero, sizeof dinero);

    Exito(playerid, "Trato cerrado.");
    Exito(vendedor, "Trato cerrado, te han pagado.");
    #pragma unused dinero
    return 1;
}

// ---------------------------------------------------------------------
//  AGRICULTOR PERSONAL  Servicio (ROL PANEL, 33_servicios.inc, ranura
//  10). Cuesta 10.000$ una vez. NO cosecha "cualquier planta lista tuya":
//  al comprarlo reclama hasta AGRICULTOR_LOTE (10) plantas que YA
//  tuvieras plantadas (Agricultor_ReclamarInicial, llamado desde
//  Servicios_AplicarCompra en 33_servicios.inc) -- si tienes menos de
//  10, reclama las que tengas. Esas quedan marcadas plAgricultor=true y
//  son LAS SUYAS: nunca toca ninguna otra planta tuya que no este
//  reclamada. Si una reclamada se pierde (te la roban, la destruyen...)
//  el cupo se queda corto -- no hay reposicion automatica, solo se
//  rellena solo si luego plantas mas (el propio tick reclama plantas
//  sueltas tuyas hasta volver a los 10).
//
//  Cada HORA (TicHora, 99_callbacks.inc) revisa cuales de SUS plantas
//  reclamadas ya estan listas y cosecha esas -- puede ser 1, puede ser
//  hasta 10, lo que le toque esa hora. Replanta semillas nuevas EN LAS
//  MISMAS COORDENADAS de lo cosechado, solo tantas como semillas tenga
//  la cuenta en ese momento. Si no tiene ninguna semilla, el hueco se
//  queda vacio hasta que compre mas.
//
//  Funciona este el jugador conectado o no (mismo patron que los
//  Ayudantes de Vendedor en 33_servicios.inc): si esta conectado se usa
//  Inv_DarItem/Inv_QuitarItem para que lo vea reflejado al instante; si
//  no, se escribe directo en la tabla `inventario`.
// ---------------------------------------------------------------------
#define AGRICULTOR_SERVICIO_ID   10
#define AGRICULTOR_LOTE          10

stock Agricultor_Tick()
{
    new consulta[110];
    format(consulta, sizeof consulta,
        "SELECT cuenta_id FROM servicios_jugador WHERE activo = 1 AND servicio_id = %d",
        AGRICULTOR_SERVICIO_ID);
    mysql_pquery(g_BD, consulta, "Agricultor_AlListarActivos");
    return 1;
}

forward Agricultor_AlListarActivos();
public Agricultor_AlListarActivos()
{
    new filas;
    cache_get_row_count(filas);
    for (new i = 0; i < filas; i++)
    {
        new cuentaId;
        cache_get_value_name_int(i, "cuenta_id", cuentaId);
        Agricultor_ComprobarCuenta(cuentaId);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  RECLAMAR -- marca como suyas (plAgricultor=true) hasta AGRICULTOR_LOTE
//  plantas sueltas de esta cuenta que todavia no lo sean, y lo persiste.
//  Se usa tanto al comprar el servicio (cupo inicial) como en cada
//  chequeo por hora (rellena huecos si has plantado mas desde entonces).
//  Si ya tiene el cupo completo, no hace nada.
// ---------------------------------------------------------------------
stock Agricultor_ReclamarInicial(cuentaId)
{
    new cupo = 0;
    for (new i = 0; i < MARIA_MAX_PLANTAS; i++)
    {
        if (!g_Plantas[i][plUsada]) continue;
        if (g_Plantas[i][plDueno] != cuentaId) continue;
        if (g_Plantas[i][plAgricultor]) cupo++;
    }

    new reclamadas = 0;
    for (new i = 0; i < MARIA_MAX_PLANTAS && cupo < AGRICULTOR_LOTE; i++)
    {
        if (!g_Plantas[i][plUsada]) continue;
        if (g_Plantas[i][plDueno] != cuentaId) continue;
        if (g_Plantas[i][plAgricultor]) continue;

        g_Plantas[i][plAgricultor] = true;
        if (g_Plantas[i][plID] > 0)
        {
            new consulta[80];
            format(consulta, sizeof consulta, "UPDATE plantas SET agricultor = 1 WHERE id = %d", g_Plantas[i][plID]);
            mysql_pquery(g_BD, consulta);
        }
        cupo++;
        reclamadas++;
    }
    return reclamadas;
}

// Rellena el cupo (por si has plantado mas desde el ultimo chequeo) y
// mira en memoria, sin tocar la base de datos, si hay algo SUYO listo
// para cosechar. Si no hay nada listo, no se molesta en consultar
// semillas.
stock Agricultor_ComprobarCuenta(cuentaId)
{
    Agricultor_ReclamarInicial(cuentaId);

    new listas = 0;
    for (new i = 0; i < MARIA_MAX_PLANTAS; i++)
    {
        if (!g_Plantas[i][plUsada]) continue;
        if (g_Plantas[i][plDueno] != cuentaId) continue;
        if (!g_Plantas[i][plAgricultor]) continue;
        if (!g_Plantas[i][plLista]) continue;
        listas++;
    }
    if (listas == 0) return 0;

    new item = Item_Buscar("Semilla de marihuana");
    new consulta[180];
    format(consulta, sizeof consulta,
        "SELECT COALESCE(SUM(cantidad),0) AS total FROM inventario WHERE cuenta_id = %d AND item = %d",
        cuentaId, item);
    mysql_pquery(g_BD, consulta, "Agricultor_AlComprobarSemillas", "d", cuentaId);
    return 1;
}

forward Agricultor_AlComprobarSemillas(cuentaId);
public Agricultor_AlComprobarSemillas(cuentaId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0) return 0;

    new semillasDisponibles;
    cache_get_value_name_int(0, "total", semillasDisponibles);

    // Revalida en memoria -- puede haber cambiado (cosecha manual, robo
    // de planta ajena) en el rato que ha tardado la consulta en volver.
    // El cupo nunca pasa de AGRICULTOR_LOTE, asi que aqui caben todas
    // las que esten listas sin tener que descartar ninguna.
    new indicesListas[AGRICULTOR_LOTE];
    new total = 0;

    for (new i = 0; i < MARIA_MAX_PLANTAS && total < AGRICULTOR_LOTE; i++)
    {
        if (!g_Plantas[i][plUsada]) continue;
        if (g_Plantas[i][plDueno] != cuentaId) continue;
        if (!g_Plantas[i][plAgricultor]) continue;
        if (!g_Plantas[i][plLista]) continue;

        indicesListas[total] = i;
        total++;
    }

    if (total == 0) return 0; // se perdieron entre medias, ya no queda ninguna lista

    new aReplantar = semillasDisponibles;
    if (aReplantar > total) aReplantar = total;

    new conectado = INVALID_PLAYER_ID;
    for (new p = 0; p < MAX_PLAYERS; p++)
    {
        if (!IsPlayerConnected(p) || !Jugador[p][jLogueado]) continue;
        if (Jugador[p][jID] != cuentaId) continue;
        conectado = p;
        break;
    }

    new itemMaria = Item_Buscar("Marihuana");
    new itemSemilla = Item_Buscar("Semilla de marihuana");
    new nombreDueno[MAX_PLAYER_NAME + 1];
    format(nombreDueno, sizeof nombreDueno, "%s", g_Plantas[indicesListas[0]][plDuenoNombre]);

    // Cosecha lo que le toque (1 a AGRICULTOR_LOTE): guarda sus
    // coordenadas, destruye la planta (objeto + cartel + fila en
    // `plantas`). La marihuana se da UNA sola vez, fuera de este bucle,
    // para que sea una unica llamada que apila en vez de varias que
    // crearian varias filas si el jugador esta desconectado.
    new Float:coordX[AGRICULTOR_LOTE], Float:coordY[AGRICULTOR_LOTE], Float:coordZ[AGRICULTOR_LOTE];
    for (new k = 0; k < total; k++)
    {
        new idx = indicesListas[k];
        coordX[k] = g_Plantas[idx][plX];
        coordY[k] = g_Plantas[idx][plY];
        coordZ[k] = g_Plantas[idx][plZ];

        Maria_Destruir(idx);
    }

    new totalMarihuana = total * MARIA_RENDIMIENTO;
    if (conectado != INVALID_PLAYER_ID)
    {
        Inv_DarItem(conectado, itemMaria, totalMarihuana, 0);
    }
    else
    {
        Inv_DarItemOffline(cuentaId, itemMaria, totalMarihuana, 0);
    }

    new consultaStat[110];
    format(consultaStat, sizeof consultaStat,
        "UPDATE cuentas SET maria_plantas_cosechadas = maria_plantas_cosechadas + %d WHERE id = %d",
        total, cuentaId);
    mysql_pquery(g_BD, consultaStat);

    // Consume semillas y replanta -- solo tantas como de verdad tenia.
    // Las replantadas ocupan el hueco en el cupo (siguen plAgricultor=true).
    if (aReplantar > 0)
    {
        if (conectado != INVALID_PLAYER_ID)
        {
            Inv_QuitarItem(conectado, itemSemilla, aReplantar);
        }
        else
        {
            new consultaQuitar[220];
            format(consultaQuitar, sizeof consultaQuitar,
                "UPDATE inventario SET cantidad = cantidad - %d WHERE cuenta_id = %d AND item = %d \
                 ORDER BY cantidad DESC LIMIT 1",
                aReplantar, cuentaId, itemSemilla);
            mysql_pquery(g_BD, consultaQuitar);

            new consultaLimpiar[150];
            format(consultaLimpiar, sizeof consultaLimpiar,
                "DELETE FROM inventario WHERE cuenta_id = %d AND item = %d AND cantidad <= 0",
                cuentaId, itemSemilla);
            mysql_pquery(g_BD, consultaLimpiar);
        }

        for (new k = 0; k < aReplantar; k++)
        {
            Maria_PlantarEn(cuentaId, nombreDueno, coordX[k], coordY[k], coordZ[k], true);
        }
    }

    if (conectado != INVALID_PLAYER_ID)
    {
        new mensaje[180];
        if (aReplantar == total)
        {
            format(mensaje, sizeof mensaje,
                "Tu Agricultor Personal ha cosechado %d planta(s) y ha replantado otras tantas en el mismo sitio.",
                total);
            Exito(conectado, mensaje);
        }
        else if (aReplantar > 0)
        {
            format(mensaje, sizeof mensaje,
                "Tu Agricultor Personal ha cosechado %d planta(s). Solo tenias semillas para replantar %d -- comprale mas semillas.",
                total, aReplantar);
            Exito(conectado, mensaje);
        }
        else
        {
            format(mensaje, sizeof mensaje,
                "Tu Agricultor Personal ha cosechado %d planta(s), pero no tenias ninguna semilla para replantar.",
                total);
            Error(conectado, mensaje);
        }
    }

    format(g_Buffer, sizeof g_Buffer,
        "Agricultor Personal: cuenta %d cosecho %d plantas, replanto %d (semillas disponibles: %d)",
        cuentaId, total, aReplantar, semillasDisponibles);
    EscribirLog("servicios", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  PORRO: liar y fumar
//  /fumar gasta 2 gramos de Marihuana, anima "Liando..." 2 segundos y
//  engancha un porro (modelo 3044, CIGAR.dff) a la mano derecha. Cada
//  click (KEY_FIRE) da una calada -- animacion + efecto de "colocado" al
//  estilo NGRP (SetPlayerDrunkLevel + clima), hasta que se acaban las
//  PORRO_CALADAS.
// ---------------------------------------------------------------------
enum E_PORRO_JUGADOR
{
    bool:poLiando,           // true durante los 2s de "Liando..."
    bool:poActivo,           // true si ya tiene el porro liado en la mano
    poCaladasRestantes,
    poUltimaCalada,          // GetTickCount() de la ultima calada (cooldown)
    poTimerEfecto            // handle del timer que revierte el "colocado", -1 si no hay ninguno activo
}
static gPorro[MAX_PLAYERS][E_PORRO_JUGADOR];

// Si el jugador lleva un arma real en la mano, la guarda en el
// inventario antes de liar el porro (mismo hueco de "mano" para los
// dos, no tiene sentido llevar ambos a la vez). Logica de busqueda de
// item calcada de CMD:guardararma en 10_inventario.inc, pero aqui no
// bloqueamos si el arma no se puede guardar (caso raro): solo bloqueamos
// si de verdad no cabe en el inventario.
//
// Devuelve true si se puede seguir (no llevaba arma, no era guardable,
// o se guardo bien) y false si hay que cancelar /fumar (no cabe).
stock bool:Porro_GuardarArmaSiLleva(playerid)
{
    new WEAPON:arma = GetPlayerWeapon(playerid);
    if (arma == WEAPON_FIST) return true;

    new item = Item_BuscarPorArma(arma);
    if (item == 0) return true;

    new municion = GetPlayerAmmo(playerid);
    if (Inv_DarItem(playerid, item, 1, municion) <= 0) return false;

    RemovePlayerWeapon(playerid, arma);

    new nombreArma[32];
    Item_Nombre(item, nombreArma, sizeof nombreArma);

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    format(mensaje, sizeof mensaje, "* %s guarda %s antes de liar un porro.", nombre, nombreArma);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
    return true;
}

CMD:fumar(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (gPorro[playerid][poLiando] || gPorro[playerid][poActivo])
    {
        Error(playerid, "Ya tienes un porro entre manos.");
        return 1;
    }

    if (IsPlayerInAnyVehicle(playerid))
    {
        Error(playerid, "No puedes liar un porro dentro de un vehiculo.");
        return 1;
    }

    new item = Item_Buscar("Marihuana");
    if (Inv_TieneItem(playerid, item) < PORRO_GRAMOS_NECESARIOS)
    {
        Error(playerid, "Necesitas al menos 2 gramos de marihuana para liar un porro.");
        return 1;
    }

    if (!Porro_GuardarArmaSiLleva(playerid))
    {
        Error(playerid, "Tienes las manos ocupadas y no te cabe el arma en el inventario. Haz hueco primero.");
        return 1;
    }

    Inv_QuitarItem(playerid, item, PORRO_GRAMOS_NECESARIOS);
    gPorro[playerid][poLiando] = true;

    Animar(playerid, "BD_FIRE", "M_smklean_loop", 4.0, false, false, false, false, PORRO_TIEMPO_LIAR);
    GameTextForPlayer(playerid, "~w~Liando...", PORRO_TIEMPO_LIAR, 3);

    new nombre[MAX_PLAYER_NAME + 1], mensaje[96];
    NombreRP(playerid, nombre, sizeof nombre);
    format(mensaje, sizeof mensaje, "* %s se pone a liar un porro.", nombre);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

    SetTimerEx("Porro_TerminarLiado", PORRO_TIEMPO_LIAR, false, "i", playerid);
    return 1;
}

forward Porro_TerminarLiado(playerid);
public Porro_TerminarLiado(playerid)
{
    if (!gPorro[playerid][poLiando]) return 0;   // se desconecto o se canceló mientras liaba
    gPorro[playerid][poLiando] = false;

    if (!IsPlayerConnected(playerid) || !Jugador[playerid][jLogueado]) return 0;

    gPorro[playerid][poActivo] = true;
    gPorro[playerid][poCaladasRestantes] = PORRO_CALADAS;
    gPorro[playerid][poUltimaCalada] = 0;

    SetPlayerAttachedObject(playerid, PORRO_SLOT, PORRO_MODELO, PORRO_BONE,
        PORRO_OFFSET_X, PORRO_OFFSET_Y, PORRO_OFFSET_Z,
        PORRO_ROT_X, PORRO_ROT_Y, PORRO_ROT_Z,
        1.0, 1.0, 1.0);

    GameTextForPlayer(playerid, "~w~Liado!", 2000, 3);
    Exito(playerid, "Porro liado. Haz click (fuego) para darle una calada.");
    return 1;
}

// Llamado desde OnPlayerKeyStateChange (99_callbacks.inc) cuando se
// pulsa KEY_FIRE. No toca nada del sistema de armas real: si el
// jugador no tiene un porro activo, esto no hace nada.
stock Porro_DarCalada(playerid)
{
    if (!gPorro[playerid][poActivo]) return 0;
    if (!Jugador[playerid][jLogueado]) return 0;

    new ahora = GetTickCount();
    if (ahora - gPorro[playerid][poUltimaCalada] < PORRO_COOLDOWN_MS) return 0;
    gPorro[playerid][poUltimaCalada] = ahora;

    Animar(playerid, "GANGS", "smkcig_prtl", 4.0, false, false, false, false, PORRO_TIEMPO_CALADA);
    Porro_AplicarEfecto(playerid);

    gPorro[playerid][poCaladasRestantes]--;
    if (gPorro[playerid][poCaladasRestantes] <= 0)
    {
        Porro_Terminar(playerid, true);
    }
    return 1;
}

// Efecto de "colocado": mismo mecanismo que usa NGRP para su sistema de
// drogas (SetPlayerDrunkLevel simula la vision borrosa, mas un cambio
// de clima para el ambiente). Cada calada reinicia el temporizador que
// lo revierte, asi que el efecto se mantiene mientras sigues fumando y
// se apaga solo unos segundos despues de la ultima calada.
stock Porro_AplicarEfecto(playerid)
{
    SetPlayerDrunkLevel(playerid, 50000);
    SetPlayerWeather(playerid, 9);
    SetPlayerTime(playerid, 0, 0);

    if (gPorro[playerid][poTimerEfecto] != -1) KillTimer(gPorro[playerid][poTimerEfecto]);
    gPorro[playerid][poTimerEfecto] = SetTimerEx("Porro_RevertirEfecto", PORRO_EFECTO_MS, false, "i", playerid);
    return 1;
}

forward Porro_RevertirEfecto(playerid);
public Porro_RevertirEfecto(playerid)
{
    gPorro[playerid][poTimerEfecto] = -1;
    if (!IsPlayerConnected(playerid)) return 0;

    SetPlayerDrunkLevel(playerid, 0);
    SetPlayerWeather(playerid, PORRO_CLIMA_NORMAL);
    SetPlayerTime(playerid, PORRO_HORA_NORMAL, 0);

    if (Jugador[playerid][jLogueado]) Info(playerid, "Se te va pasando el efecto poco a poco...");
    return 1;
}

// Quita el porro de la mano y limpia el estado. consumido=true muestra
// el aviso de "se te ha acabado"; false se usa en limpiezas silenciosas
// (desconexion, muerte).
stock Porro_Terminar(playerid, bool:consumido = false)
{
    if (!gPorro[playerid][poActivo] && !gPorro[playerid][poLiando]) return 0;

    if (gPorro[playerid][poActivo]) RemovePlayerAttachedObject(playerid, PORRO_SLOT);

    gPorro[playerid][poActivo] = false;
    gPorro[playerid][poLiando] = false;
    gPorro[playerid][poCaladasRestantes] = 0;

    if (consumido && Jugador[playerid][jLogueado]) Info(playerid, "Se te ha acabado el porro.");
    return 1;
}

// ---------------------------------------------------------------------
//  CONEXION / DESCONEXION / MUERTE (llamados desde 99_callbacks.inc)
// ---------------------------------------------------------------------
public Porro_OnConectar(playerid)
{
    new vacio[E_PORRO_JUGADOR];
    gPorro[playerid] = vacio;
    gPorro[playerid][poTimerEfecto] = -1;
    return 1;
}

public Porro_OnDesconectar(playerid)
{
    if (gPorro[playerid][poTimerEfecto] != -1)
    {
        KillTimer(gPorro[playerid][poTimerEfecto]);
        gPorro[playerid][poTimerEfecto] = -1;
    }
    Porro_Terminar(playerid);
    return 1;
}

// Al morir se reaparece "sin lo que llevaba en las manos" (mismo criterio
// que ya aplica OnPlayerDeath con las armas), asi que el porro tambien
// se pierde.
public Porro_OnMuerte(playerid)
{
    Porro_OnDesconectar(playerid);
    return 1;
}
