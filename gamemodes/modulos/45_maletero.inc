/* =====================================================================
 *  45_maletero.inc
 *  ---------------------------------------------------------------------
 *  MALETERO DE VEHICULO -- almacenamiento persistente por vehiculo,
 *  encargo de Vlad del 05/09/2026 (master prompt "PORTATIL HACKER +
 *  MALETEROS + BOTIN DE VEHICULOS LIBRES"). Fase 1: modelo persistente y
 *  primitivas. Fase 2 (esta entrega): comando /maletero + menu de
 *  guardar/sacar, SOLO PARA EL PROPIETARIO. /forzarmaletero (acceso de
 *  no propietarios) y el botin automatico en coches libres son fases
 *  posteriores, todavia sin construir -- por eso el comando de abajo
 *  solo comprueba propiedad, nunca "acceso forzado vigente" (eso llega
 *  en la Fase 3).
 *
 *  A PROPOSITO, por diseno cerrado del encargo -- NO es un inventario:
 *  cinco recursos con CANTIDAD fija cada uno, nada de slots, peso ni
 *  capacidad por vehiculo. Los cinco (dinero, materiales, marihuana,
 *  semillas, portatiles) son columnas fijas de la tabla `maletero`, no
 *  filas de un catalogo de items.
 *
 *  PERSISTENCIA: tabla `maletero` (fila por `vehiculo_id`, FK con
 *  ON DELETE CASCADE a `vehiculos.id` -- al borrarse un vehiculo su
 *  maletero desaparece solo, sin DELETE propio aqui). Array en memoria
 *  `g_Maletero[MAX_VEHICULOS][E_MALETERO]`, paralelo a `g_Vehiculos` por
 *  el mismo indice -- mismo patron que `g_TuningPintura` en
 *  14_tuning.inc, no un campo mas dentro de E_VEHICULO (ese enum ya es
 *  grande y esto es un sistema aparte).
 *
 *  NO se crea una fila en `maletero` para cada vehiculo al cargar --
 *  la mayoria del pool (el 90%+ sin botin) nunca tendra fila, y en
 *  memoria simplemente vale 0 en los cinco campos por defecto. Solo se
 *  escribe una fila real (INSERT/REPLACE) la primera vez que un
 *  vehiculo concreto recibe contenido.
 * ===================================================================== */

enum E_MALETERO
{
    maDinero,
    maMateriales,
    maMarihuana,
    maSemillas,
    maPortatiles,
    maBarbacoa,     // 07/09/2026, mismo patron que maPortatiles: 0 o 1, item de g_Items
    maObjeto        // id de g_Items del objeto Pawnshop (06/09/2026), 0 = ninguno. Sigue
                     // siendo "cantidad fija" -- solo puede haber 0 o 1, nunca un stack.
}
new g_Maletero[MAX_VEHICULOS][E_MALETERO];

// PESCADO (07/09/2026) -- APARTE de los recursos de arriba a proposito:
// cada pez pesa distinto y vive en su propia ranura (itMaxPila=1, igual
// que en el inventario del jugador), asi que un simple contador
// "cantidad" perderia el peso individual de cada pieza (del que depende
// su precio de venta). Mismo patron que el bahul de casas
// (g_CasaAlmacen/casas_almacen, 24_casas.inc): una fila por pez
// guardado, item + datos (peso en gramos). MALETERO_PESCADO_SLOTS = 20
// para que "guardar todo el pescado" nunca se quede corto -- es el mismo
// tope que MAX_SLOTS_INVENTARIO, el maximo de peces que un jugador puede
// llevar encima a la vez.
#define MALETERO_PESCADO_SLOTS 20
enum { MP_ITEM, MP_DATOS, MP_CAMPOS };
new g_MaleteroPescado[MAX_VEHICULOS][MALETERO_PESCADO_SLOTS][MP_CAMPOS];

// Buffer propio de consultas, mismo motivo que g_FedericoConsulta
// (43_federico.inc): no compartir el g_Buffer global con nada que no
// sea una consulta SQL de este modulo.
static g_MaleteroConsulta[256];

// ---------------------------------------------------------------------
//  CARGA -- una vez, desde AlCargarVehiculos (13_vehiculos.inc), con el
//  pool de vehiculos ya en memoria (necesita mapear vehiculo_id -> indice
//  via veID, igual que Casa_IndicePorID en 24_casas.inc con las cuentas
//  de los bahules de casas).
// ---------------------------------------------------------------------
stock Maletero_CargarTodos()
{
    mysql_pquery(g_BD, "SELECT * FROM maletero", "AlCargarMaleteros");
    return 1;
}

stock Maletero_IndicePorVehiculoID(vehiculoId)
{
    for (new i = 0; i < MAX_VEHICULOS; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veID] == vehiculoId) return i;
    }
    return -1;
}

forward AlCargarMaleteros();
public AlCargarMaleteros()
{
    new filas;
    cache_get_row_count(filas);

    new cargados = 0, vehiculoId;

    for (new i = 0; i < filas; i++)
    {
        cache_get_value_name_int(i, "vehiculo_id", vehiculoId);

        new indice = Maletero_IndicePorVehiculoID(vehiculoId);
        if (indice == -1) continue; // fila huerfana (no deberia pasar, FK lo impide) -- se ignora

        cache_get_value_name_int(i, "dinero",     g_Maletero[indice][maDinero]);
        cache_get_value_name_int(i, "materiales", g_Maletero[indice][maMateriales]);
        cache_get_value_name_int(i, "marihuana",  g_Maletero[indice][maMarihuana]);
        cache_get_value_name_int(i, "semillas",   g_Maletero[indice][maSemillas]);
        cache_get_value_name_int(i, "portatiles", g_Maletero[indice][maPortatiles]);
        cache_get_value_name_int(i, "barbacoa",   g_Maletero[indice][maBarbacoa]);
        cache_get_value_name_int(i, "objeto",     g_Maletero[indice][maObjeto]);
        cargados++;
    }

    printf("  [MALETERO] %d maleteros con contenido cargados desde la base de datos.", cargados);

    // Primera vez que existe este sistema (tabla vacia de verdad, no solo
    // "0 filas en este load") -- se siembra el botin inicial UNA sola
    // vez. Igual que AlCargarFederico (43_federico.inc): filas==0 en la
    // primera carga es la senal de "nunca se ha generado nada todavia",
    // no se vuelve a repetir en reinicios siguientes porque ya habra
    // filas reales.
    if (filas == 0) Maletero_GenerarBotinTodos();

    Maletero_PescadoCargarTodos();
    return 1;
}

// ---------------------------------------------------------------------
//  PESCADO -- carga, persistencia por fila (mismo patron que
//  Casa_AlmacenGuardarFila/BorrarFila en 24_casas.inc).
// ---------------------------------------------------------------------
stock Maletero_PescadoCargarTodos()
{
    mysql_pquery(g_BD, "SELECT * FROM maletero_pescado", "AlCargarMaleteroPescado");
    return 1;
}

forward AlCargarMaleteroPescado();
public AlCargarMaleteroPescado()
{
    new filas;
    cache_get_row_count(filas);

    new cargados = 0, vehiculoId, slot;
    for (new i = 0; i < filas; i++)
    {
        cache_get_value_name_int(i, "vehiculo_id", vehiculoId);
        cache_get_value_name_int(i, "slot", slot);

        new indice = Maletero_IndicePorVehiculoID(vehiculoId);
        if (indice == -1 || slot < 0 || slot >= MALETERO_PESCADO_SLOTS) continue;

        cache_get_value_name_int(i, "item",  g_MaleteroPescado[indice][slot][MP_ITEM]);
        cache_get_value_name_int(i, "datos", g_MaleteroPescado[indice][slot][MP_DATOS]);
        cargados++;
    }

    printf("  [MALETERO] %d peces guardados cargados desde la base de datos.", cargados);
    return 1;
}

stock Maletero_PescadoGuardarFila(indice, slot)
{
    mysql_format(g_BD, g_MaleteroConsulta, sizeof g_MaleteroConsulta,
        "INSERT INTO maletero_pescado (vehiculo_id, slot, item, datos) VALUES (%d, %d, %d, %d) \
         ON DUPLICATE KEY UPDATE item = VALUES(item), datos = VALUES(datos)",
        g_Vehiculos[indice][veID], slot, g_MaleteroPescado[indice][slot][MP_ITEM], g_MaleteroPescado[indice][slot][MP_DATOS]);
    mysql_pquery(g_BD, g_MaleteroConsulta);
    return 1;
}

stock Maletero_PescadoBorrarFila(indice, slot)
{
    mysql_format(g_BD, g_MaleteroConsulta, sizeof g_MaleteroConsulta,
        "DELETE FROM maletero_pescado WHERE vehiculo_id = %d AND slot = %d",
        g_Vehiculos[indice][veID], slot);
    mysql_pquery(g_BD, g_MaleteroConsulta);
    return 1;
}

// Busca la primera ranura libre y guarda ahi -- false si el maletero de
// pescado esta lleno de verdad (las 20 ranuras ocupadas, caso extremo).
stock bool:Maletero_PescadoAgregar(indice, item, datos)
{
    for (new s = 0; s < MALETERO_PESCADO_SLOTS; s++)
    {
        if (g_MaleteroPescado[indice][s][MP_ITEM] != 0) continue;

        g_MaleteroPescado[indice][s][MP_ITEM]  = item;
        g_MaleteroPescado[indice][s][MP_DATOS] = datos;
        Maletero_PescadoGuardarFila(indice, s);
        return true;
    }
    return false;
}

stock Maletero_PescadoTotal(indice)
{
    new total = 0;
    for (new s = 0; s < MALETERO_PESCADO_SLOTS; s++)
    {
        if (g_MaleteroPescado[indice][s][MP_ITEM] != 0) total++;
    }
    return total;
}

// ---------------------------------------------------------------------
//  BOTIN AUTOMATICO EN COCHES LIBRES -- "tragaperras", rediseñado el
//  06/09/2026 (encargo de Vlad, tras ver que abrir 20 maleteros del 33%
//  solo daba una cosa suelta cada vez -- unas gafas, o unos materiales,
//  nunca varias a la vez). El diseño anterior repartia el 33% en tres
//  grupos que se EXCLUIAN entre si (normal / extremo / objeto). Ahora:
//
//    50% -- sin nada (era 67%, bajado el 08/09/2026 al subir el otro lado).
//    50% -- CAE EL PREMIO COMPLETO (subido del 33% el 08/09/2026): Dinero
//           + Materiales + Marihuana + 1 objeto Pawnshop, LOS CUATRO A LA
//           VEZ, cada uno con su propia tirada de rareza (ver
//           Botin_TirarDinero y compañia, mas abajo, cantidades tambien
//           subidas un 25% el mismo dia) -- las cantidades altas y los
//           objetos caros siguen siendo raros, pero todo el que "tiene
//           algo" tiene las cuatro cosas, nunca solo una.
//
//  Portatil Hacker sigue sin poder salir aqui -- nunca formo parte de
//  este sorteo (ni antes ni ahora), solo lo dan los historicos de
//  Federico o la tienda. Lo documento explicito para que quede blindado.
//
//  Cada coche se sortea de forma INDEPENDIENTE (no un recuento exacto
//  tipo Federico): asi la misma funcion sirve tanto para regenerar el
//  pool entero de golpe (Maletero_GenerarBotinTodos) como para un coche
//  suelto recien nacido con /creardropeo (22_dropeo.inc).
// ---------------------------------------------------------------------
#define MALETERO_BOTIN_PCT_TOTAL      50   // % de coches libres con el premio completo -- subido del 33% el 08/09/2026

enum E_BOTIN_TRAMO { btMin, btMax, btPeso }

// Tres tramos de rareza por recurso -- pesos sobre 100, common:medio:raro
// = 70:25:5. El objeto ya tiene sus 6 tramos propios en g_PawnshopCatalogo,
// mas abajo, no hace falta duplicarlos aqui.
//
// Cifras subidas un 10% el 06/09/2026 (encargo de Vlad, tercera pasada
// del dia sobre estos numeros): 500-2000 -> 550-2200 y asi con el resto,
// redondeado a numeros limpios. Subidas otro 25% el 08/09/2026 (encargo
// expreso de Vlad), redondeado igual: 550-2200 -> 690-2750 y asi con el
// resto.
new const g_BotinDinero[][E_BOTIN_TRAMO] = {
    {690,    2750,  70},
    {6875,   13750, 25},
    {27500,  34375,  5}
};
new const g_BotinMateriales[][E_BOTIN_TRAMO] = {
    {35,     140,   70},
    {275,    690,   25},
    {1375,   2750,   5}
};
new const g_BotinMarihuana[][E_BOTIN_TRAMO] = {
    {14,     85,    70},
    {140,    275,   25},
    {550,    690,    5}
};
#define BOTIN_TRAMO_PESO_TOTAL 100

stock Botin_TirarDinero()          { return Botin_TirarTramo(g_BotinDinero); }
stock Botin_TirarMateriales()      { return Botin_TirarTramo(g_BotinMateriales); }
stock Botin_TirarMarihuana()       { return Botin_TirarTramo(g_BotinMarihuana); }

// Sorteo ponderado generico sobre una tabla de 3 tramos {min, max, peso}.
// Las tres tablas de arriba suman 100 de peso cada una a proposito, para
// poder compartir esta unica funcion.
stock Botin_TirarTramo(const tabla[][E_BOTIN_TRAMO])
{
    new tirada = random(BOTIN_TRAMO_PESO_TOTAL);
    new acumulado = 0;

    for (new i = 0; i < 3; i++)
    {
        acumulado += tabla[i][btPeso];
        if (tirada < acumulado) return tabla[i][btMin] + random(tabla[i][btMax] - tabla[i][btMin] + 1);
    }
    return tabla[2][btMin]; // redondeo, no deberia llegar aqui
}

// Recorre TODO el pool de coches libres actual y les tira el sorteo de
// abajo, uno por uno. La usa el primer arranque en vacio; tambien sirve
// para rebalancear el pool entero de golpe si algun dia hace falta.
stock Maletero_GenerarBotinTodos()
{
    new totalCandidatos = 0, premiosHechos = 0;

    for (new i = 0; i < MAX_VEHICULOS; i++)
    {
        if (!Maletero_EsCandidatoLibre(i)) continue;
        totalCandidatos++;

        if (Maletero_TirarLootAleatorio(i)) premiosHechos++;
    }

    printf("  [MALETERO] Botin generado sobre %d coches libres: %d con el premio completo.",
        totalCandidatos, premiosHechos);

    format(g_Buffer, sizeof g_Buffer,
        "MALETERO_BOTIN_GENERADO total_libres=%d premios=%d timestamp=%d",
        totalCandidatos, premiosHechos, gettime());
    EscribirLog("maletero", g_Buffer);
    return 1;
}

// Un coche libre es candidato al botin automatico si es VEH_DROPEO sin
// dueno Y NO es el coche historico de Federico vigente ahora mismo (usa
// el mismo tipo de vehiculo y el mismo propietario=0 que cualquier coche
// libre normal, asi que sin este filtro entraria en el sorteo). Ese
// coche tiene su propio contenido fijo (43_federico.inc, g_Historicos) y
// no se le toca ni se le anade nada de este sistema -- encargo expreso
// de Vlad, 06/09/2026.
stock bool:Maletero_EsCandidatoLibre(indice)
{
    if (!g_Vehiculos[indice][veUsado]) return false;
    if (g_Vehiculos[indice][veTipo] != VEH_DROPEO) return false;
    if (g_Vehiculos[indice][vePropietario] != 0) return false;
    if (indice == Federico_HistoricoIndice()) return false;
    return true;
}

// Sorteo independiente para UN coche candidato ya validado. Devuelve
// true si le toco el premio completo, false si no le toco nada.
stock bool:Maletero_TirarLootAleatorio(indice)
{
    if (random(100) >= MALETERO_BOTIN_PCT_TOTAL) return false;

    Maletero_AplicarBotinCompleto(indice);
    return true;
}

// El premio completo: las cuatro cosas a la vez, cada una con su propia
// tirada de rareza. Portatil Hacker NUNCA entra aqui a proposito (ver
// cabecera de la seccion) -- solo dinero/materiales/marihuana/objeto.
stock Maletero_AplicarBotinCompleto(indice)
{
    Maletero_Mover(indice, maDinero,     Botin_TirarDinero());
    Maletero_Mover(indice, maMateriales, Botin_TirarMateriales());
    Maletero_Mover(indice, maMarihuana,  Botin_TirarMarihuana());

    new item = Pawnshop_ElegirObjetoAleatorio();
    if (item != 0) g_Maletero[indice][maObjeto] = item;

    // Maletero_Mover ya guarda por cada llamada, pero maObjeto no pasa
    // por ahi -- un guardado final asegura que las cuatro cosas quedan
    // persistidas juntas, no solo las tres primeras.
    Maletero_Guardar(indice);
    return 1;
}

// ---------------------------------------------------------------------
//  CATALOGO PAWNSHOP -- 25 objetos de loot, encargo de Vlad del
//  06/09/2026. Preparado para un Pawnshop/perista futuro (fase aparte,
//  no implementada todavia): cada objeto vive como una fila real de
//  g_Items (10_inventario.inc, ids 48-72) para que ya funcione con el
//  inventario/maletero/forzarmaletero tal cual estan, sin logica nueva.
//  Lo unico que vive SOLO aqui es lo que no pertenece a un objeto de
//  inventario generico: su valor de reventa futuro y su peso de rareza
//  para este sorteo -- para no anadir un campo a E_ITEM que valdria 0
//  para los otros 47 objetos del catalogo.
//
//  RAREZA: cuanto mas caro, mas alto el peso necesario para que salga
//  menos. Los 6 tramos son directamente proporcionales al precio, no
//  aleatorios -- si se anade un objeto nuevo, que seas tu quien decida
//  su peso, no una formula automatica.
// ---------------------------------------------------------------------
enum E_PAWNSHOP
{
    ppItem,     // id en g_Items
    ppValor,    // $ de venta futura (el Pawnshop en si no esta implementado)
    ppPeso      // peso relativo del sorteo -- mas alto = mas comun
}
// SIN "static" a proposito -- 50_pawnshop.inc (incluido despues) necesita
// leer ppValor/ppPeso directamente para el menu de venta.
new const g_PawnshopCatalogo[][E_PAWNSHOP] = {
    // Comunes (peso 500 c/u -- la mayoria de los hallazgos)
    // Pesos ×10 respecto a la version original (06/09/2026) para poder
    // subir SOLO los objetos caros (10.000$+) un 20% exacto con enteros
    // limpios, sin arrastrar redondeos raros en los pesos pequenos (1, 2,
    // 3...) de los tramos mas altos. Encargo de Vlad: "sube las
    // posibilidades de un articulo caro un 20%".
    {48,    100, 5000},  // Gafas de sol
    {49,    150, 5000},  // Pato de goma
    {50,    250, 5000},  // Mechero Zippo
    {51,    300, 5000},  // Calzoncillos de marca
    {52,    500, 5000},  // Auriculares
    {53,    500, 5000},  // CD de reggaeton de 2009

    // Poco comunes (peso 1500 c/u, sin cambios de proporcion)
    {54,    750, 1500},  // Camara de fotos
    {55,   1000, 1500},  // Zapatillas de marca
    {56,   1500, 1500},  // Altavoz portatil
    {57,   1500, 1500},  // Muneca hinchable

    // Interesantes (peso 500 c/u, sin cambios de proporcion)
    {58,   2000,  500},  // Telefono movil
    {59,   2500,  500},  // Trofeo Mejor Conductor
    {60,   3000,  500},  // Camara GoPro

    // Valiosos (peso 150 c/u, sin cambios de proporcion)
    {61,   4000,  150},  // Tablet
    {62,   5000,  150},  // Consola portatil
    {63,   7500,  150},  // Ordenador portatil

    // Raros -- CAROS (10.000$+): peso base 50, +20% = 60 c/u
    {64,  10000,   60},  // Cadena de oro
    {65,  12500,   60},  // Camara profesional
    {66,  15000,   60},  // Anillo de diamantes
    {67,  20000,   60},  // Bolso de lujo

    // Excepcionalmente raros -- CAROS: peso base 40/30/20/20/10, +20%
    {68,  30000,   48},  // Lingote de oro pequeno
    {69,  40000,   36},  // Reloj de lujo
    {70,  60000,   24},  // Diamante
    {71,  75000,   24},  // Maletin con joyas
    {72, 100000,   12}   // Rolex de oro -- el mas raro de los 25, a proposito
};

// Suma de todos los pesos de arriba: (5000×6)+(1500×4)+(500×3)+(150×3)+
// (60×4)+(48+36+24+24+12) = 30000+6000+1500+450+240+144 = 38334.
// Calculada a mano y no con un bucle en tiempo de carga porque el array
// es const -- si anades, quitas o repesas una fila, actualiza este numero.
#define PAWNSHOP_PESO_TOTAL 38334

// Sorteo ponderado: recorre el catalogo acumulando peso hasta pasarse
// del numero elegido. Devuelve el id de g_Items, nunca 0 (con este
// catalogo siempre hay algo que devolver).
stock Pawnshop_ElegirObjetoAleatorio()
{
    new tirada = random(PAWNSHOP_PESO_TOTAL);
    new acumulado = 0;

    for (new i = 0; i < sizeof g_PawnshopCatalogo; i++)
    {
        acumulado += g_PawnshopCatalogo[i][ppPeso];
        if (tirada < acumulado) return g_PawnshopCatalogo[i][ppItem];
    }
    return g_PawnshopCatalogo[sizeof(g_PawnshopCatalogo) - 1][ppItem]; // redondeo, no deberia llegar aqui
}

// Indice en g_PawnshopCatalogo a partir del id de g_Items, o -1 si ese
// item no es un objeto Pawnshop. Lo usara el Pawnshop/perista futuro
// para saber cuanto pagar; por ahora lo usa /hacker para mostrar el
// nombre real de lo que hay en un maletero.
stock Pawnshop_IndicePorItem(item)
{
    for (new i = 0; i < sizeof g_PawnshopCatalogo; i++)
    {
        if (g_PawnshopCatalogo[i][ppItem] == item) return i;
    }
    return -1;
}

// ---------------------------------------------------------------------
//  GUARDAR -- vuelca los 5 campos de un vehiculo. UPSERT sencillo, igual
//  que Tuning_Guardar (14_tuning.inc): el volumen de escrituras (solo
//  cuando alguien mete/saca algo de un maletero concreto) no justifica
//  nada mas fino que un REPLACE INTO completo.
// ---------------------------------------------------------------------
stock Maletero_Guardar(indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (g_Vehiculos[indice][veID] <= 0) return 0; // vehiculo sin fila propia en `vehiculos" (no deberia pasar)

    mysql_format(g_BD, g_MaleteroConsulta, sizeof g_MaleteroConsulta,
        "REPLACE INTO maletero (vehiculo_id, dinero, materiales, marihuana, semillas, portatiles, barbacoa, objeto) \
         VALUES (%d, %d, %d, %d, %d, %d, %d, %d)",
        g_Vehiculos[indice][veID],
        g_Maletero[indice][maDinero], g_Maletero[indice][maMateriales],
        g_Maletero[indice][maMarihuana], g_Maletero[indice][maSemillas],
        g_Maletero[indice][maPortatiles], g_Maletero[indice][maBarbacoa], g_Maletero[indice][maObjeto]);
    mysql_pquery(g_BD, g_MaleteroConsulta);
    return 1;
}

// ---------------------------------------------------------------------
//  LIMPIEZA -- llamado desde Veh_Borrar (13_vehiculos.inc) antes de
//  vaciar la fila de g_Vehiculos. Solo limpia la memoria: el ON DELETE
//  CASCADE de la tabla `maletero` ya se encarga de la fila en la base de
//  datos en cuanto se borra el vehiculo -- no hace falta un DELETE aqui.
// ---------------------------------------------------------------------
stock Maletero_AlBorrarVehiculo(indice)
{
    if (!VehiculoValido(indice)) return 0;

    new vacio[E_MALETERO];
    g_Maletero[indice] = vacio;

    // Mismo motivo para el pescado: el indice se reutiliza para un
    // vehiculo nuevo, y `maletero_pescado` tambien cae solo por el
    // ON DELETE CASCADE -- esto es solo limpieza de memoria.
    for (new s = 0; s < MALETERO_PESCADO_SLOTS; s++)
    {
        g_MaleteroPescado[indice][s][MP_ITEM]  = 0;
        g_MaleteroPescado[indice][s][MP_DATOS] = 0;
    }
    return 1;
}

// Vacia el maletero de un vehiculo y lo PERSISTE (a diferencia de
// Maletero_AlBorrarVehiculo, que solo limpia memoria porque el vehiculo
// va a desaparecer del todo). La usa /comprarcoche (22_dropeo.inc,
// encargo de Vlad 06/09/2026): comprar un coche libre no debe regalar de
// paso el botin que le hubiera tocado en el sorteo -- las 5 categorias
// (dinero, materiales, marihuana, semillas, objeto) a cero.
stock Maletero_VaciarTodo(indice)
{
    if (!VehiculoValido(indice)) return 0;

    new vacio[E_MALETERO];
    g_Maletero[indice] = vacio;
    Maletero_Guardar(indice);
    return 1;
}

// ---------------------------------------------------------------------
//  PRIMITIVAS -- unico camino para tocar un maletero. Todo lo que venga
//  despues (comando /maletero, /forzarmaletero, botin automatico, los
//  hacks) tiene que pasar por aqui, nunca escribir g_Maletero
//  directamente -- es el punto unico de validacion contra duplicacion.
// ---------------------------------------------------------------------

// Cuanto hay de un recurso concreto ahora mismo.
stock Maletero_Obtener(indice, E_MALETERO:recurso)
{
    if (!VehiculoValido(indice)) return 0;
    return g_Maletero[indice][recurso];
}

// Anade `cantidad` (puede ser negativa para sacar) a un recurso. Nunca
// deja el resultado por debajo de 0 ni lo aplica si no hay suficiente
// para restar -- devuelve 0 y no toca nada si la operacion no es
// valida, 1 si se aplico y se persistio.
stock bool:Maletero_Mover(indice, E_MALETERO:recurso, cantidad)
{
    if (!VehiculoValido(indice)) return false;
    if (cantidad == 0) return false;

    new actual = g_Maletero[indice][recurso];
    new Float:nuevoF = float(actual) + float(cantidad);
    if (nuevoF < 0.0) return false; // no hay suficiente para sacar esa cantidad

    g_Maletero[indice][recurso] = actual + cantidad;
    Maletero_Guardar(indice);
    return true;
}

// Accesores para modulos incluidos ANTES que este (43_federico.inc, al
// crear un Coche de Personaje Historico) -- los miembros del enum
// E_MALETERO estan definidos en este archivo, no se pueden nombrar
// desde un archivo incluido antes, pero una funcion si.
stock bool:Maletero_MoverMarihuana(indice, cantidad)
{
    return Maletero_Mover(indice, maMarihuana, cantidad);
}
stock bool:Maletero_MoverDinero(indice, cantidad)
{
    return Maletero_Mover(indice, maDinero, cantidad);
}
stock bool:Maletero_MoverMateriales(indice, cantidad)
{
    return Maletero_Mover(indice, maMateriales, cantidad);
}
stock bool:Maletero_MoverPortatiles(indice, cantidad)
{
    return Maletero_Mover(indice, maPortatiles, cantidad);
}

// bool: tiene el vehiculo algo de contenido, cualquier recurso > 0.
// Lo usara el botin automatico (fase 4) y el hack de maleteros (fase 9)
// para filtrar el subconjunto de coches libres "con algo dentro".
stock bool:Maletero_TieneContenido(indice)
{
    if (!VehiculoValido(indice)) return false;
    return bool:(g_Maletero[indice][maDinero] > 0
        || g_Maletero[indice][maMateriales] > 0
        || g_Maletero[indice][maMarihuana] > 0
        || g_Maletero[indice][maSemillas] > 0
        || g_Maletero[indice][maPortatiles] > 0
        || g_Maletero[indice][maObjeto] > 0);
}

// Accesor para 31_web.inc (Hack_Maleteros), incluido ANTES que este
// archivo -- mismo motivo que Maletero_MoverDinero y compania mas abajo:
// los miembros de E_MALETERO no se pueden nombrar desde ahi directamente.
//
// Construye la lista real de lo que hay AHORA MISMO en el maletero, uno
// por linea (para /hacker -- "CONTENIDO DEL MALETERO", encargo de Vlad
// del 06/09/2026: nunca una copia guardada ni una categoria generica,
// siempre el estado real en el instante del hackeo). Vacio si no tiene
// nada -- lo comprueba antes Maletero_TieneContenido, no aqui.
stock Maletero_DescribirContenidoReal(indice, destino[], tam)
{
    destino[0] = EOS;
    if (!VehiculoValido(indice)) return 0;

    new pieza[64];

    if (g_Maletero[indice][maDinero] > 0)
    {
        new valor[24];
        FormatearDinero(g_Maletero[indice][maDinero], valor, sizeof valor);
        format(pieza, sizeof pieza, "%s\n", valor);
        strcat(destino, pieza, tam);
    }
    if (g_Maletero[indice][maMateriales] > 0)
    {
        format(pieza, sizeof pieza, "%d Materiales\n", g_Maletero[indice][maMateriales]);
        strcat(destino, pieza, tam);
    }
    if (g_Maletero[indice][maMarihuana] > 0)
    {
        format(pieza, sizeof pieza, "%d Marihuana\n", g_Maletero[indice][maMarihuana]);
        strcat(destino, pieza, tam);
    }
    if (g_Maletero[indice][maSemillas] > 0)
    {
        format(pieza, sizeof pieza, "%d Semillas de marihuana\n", g_Maletero[indice][maSemillas]);
        strcat(destino, pieza, tam);
    }
    if (g_Maletero[indice][maPortatiles] > 0)
    {
        format(pieza, sizeof pieza, "%d Portatil(es) Hacker\n", g_Maletero[indice][maPortatiles]);
        strcat(destino, pieza, tam);
    }
    if (g_Maletero[indice][maObjeto] > 0)
    {
        new nombreObjeto[32];
        Item_Nombre(g_Maletero[indice][maObjeto], nombreObjeto, sizeof nombreObjeto);
        format(pieza, sizeof pieza, "%s\n", nombreObjeto);
        strcat(destino, pieza, tam);
    }

    // Quita el ultimo salto de linea sobrante.
    new len = strlen(destino);
    if (len > 0 && destino[len - 1] == '\n') destino[len - 1] = EOS;
    return 1;
}

// Valor total estimado del contenido de un maletero, en $ -- dinero real
// + materiales a 1$/u + marihuana al precio actual de la Casa de
// Empeños (50_pawnshop.inc, incluido DESPUES -- llamada valida, es una
// funcion, no una variable) + el valor de reventa del objeto Pawnshop si
// tiene uno. Lo usa /hacker (31_web.inc, encargo de Vlad 06/09/2026)
// para enseñar los coches con MAS valor, no un hallazgo al azar puro.
stock Maletero_ValorTotal(indice)
{
    if (!VehiculoValido(indice)) return 0;

    new total = g_Maletero[indice][maDinero];
    total += g_Maletero[indice][maMateriales] * EMPENOS_MATERIALES_PRECIO;
    total += g_Maletero[indice][maMarihuana]  * Empenos_PrecioMarihuana();

    if (g_Maletero[indice][maObjeto] > 0)
    {
        new idx = Pawnshop_IndicePorItem(g_Maletero[indice][maObjeto]);
        if (idx != -1) total += g_PawnshopCatalogo[idx][ppValor];
    }
    return total;
}

// ---------------------------------------------------------------------
//  FASE 2 -- /maletero, SOLO PROPIETARIO. La regla de la Fase 3 (ignorar
//  el candado, exigir /forzarmaletero a cualquier no propietario) queda
//  medio implementada aqui: el "no eres el dueno" ya bloquea exactamente
//  como pide la regla; falta anadir la rama de acceso forzado vigente
//  cuando llegue /forzarmaletero.
//
//  Descripcion de cada recurso -- el indice de esta tabla coincide a
//  proposito con E_MALETERO (maDinero=0 ... maPortatiles=4), asi no hay
//  dos listas que puedan desincronizarse. mrItemNombre vacio = es
//  dinero (no pasa por el inventario). "Portatil Hacker" el item
//  todavia no existe (llega en la Fase 5) -- Item_Buscar() devolvera
//  invalido hasta entonces y el propio flujo de guardar/sacar ya lo
//  contempla (ver Maletero_CantidadDisponibleParaGuardar), no hace
//  falta tocar nada de esto cuando se cree el objeto.
// ---------------------------------------------------------------------
enum E_MALETERO_RECURSO { mrEtiqueta[20], mrItemNombre[24] }
new const g_MaleteroRecursos[][E_MALETERO_RECURSO] = {
    {"Dinero",           ""},
    {"Materiales",       "Materiales"},
    {"Marihuana",        "Marihuana"},
    {"Semillas",         "Semilla de marihuana"},
    {"Portatil Hacker",  "Portatil Hacker"},
    {"Barbacoa",         "Barbacoa"}
};

static g_MaleteroVehiculo[MAX_PLAYERS] = {-1, ...};
static g_MaleteroRecursoSel[MAX_PLAYERS];
static bool:g_MaleteroModoGuardar[MAX_PLAYERS];
static g_MaleteroCantidadMax[MAX_PLAYERS];

// Acceso forzado vigente (Fase 3, /forzarmaletero) -- que vehiculo y
// hasta que gettime() real. -1/0 = no hay acceso forzado activo. Es la
// UNICA forma en que un no-propietario puede pasar el cheque de abajo.
static g_MaleteroAccesoVehiculo[MAX_PLAYERS] = {-1, ...};
static g_MaleteroAccesoHasta[MAX_PLAYERS];

// bool: tiene playerid ahora mismo acceso legitimo a este vehiculo --
// o es el dueno, o tiene una ventana de acceso forzado vigente sobre
// ESE mismo indice (comparado ademas contra gettime(), un reloj de
// servidor que el cliente no puede tocar ni reiniciar cerrando dialogos).
stock bool:Maletero_TieneAcceso(playerid, indice)
{
    if (g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID]) return true;
    if (Jugador[playerid][jAdmin] >= ADMIN_FUNDADOR) return true; // /adminmaletero, ver mas abajo
    if (g_MaleteroAccesoVehiculo[playerid] != indice) return false;
    return bool:(gettime() < g_MaleteroAccesoHasta[playerid]);
}

CMD:maletero(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new indice = Veh_BuscarCercano(playerid, MALETERO_RADIO);
    if (indice == -1)
    {
        Error(playerid, "No hay ningun vehiculo cerca.");
        return 1;
    }

    if (!Maletero_TieneAcceso(playerid, indice))
    {
        Error(playerid, "Este vehiculo no es tuyo");
        return 1;
    }

    g_MaleteroVehiculo[playerid] = indice;
    Maletero_AbrirFisico(indice, true);
    Maletero_MostrarMenu(playerid);
    return 1;
}

// ADMIN · /adminmaletero -- abre el maletero de CUALQUIER vehiculo
// cercano saltandose Maletero_TieneAcceso por completo (ni propietario
// ni ventana de forzado vigente hacen falta). Solo Fundador, encargo de
// Vlad, 06/09/2026. Reutiliza el mismo menu/dialogo que /maletero -- no
// es un sistema aparte, solo se salta la comprobacion de acceso.
CMD:adminmaletero(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_FUNDADOR)) return 1;

    new indice = Veh_BuscarCercano(playerid, MALETERO_RADIO);
    if (indice == -1)
    {
        Error(playerid, "No hay ningun vehiculo cerca.");
        return 1;
    }

    g_MaleteroVehiculo[playerid] = indice;
    Maletero_AbrirFisico(indice, true);
    Maletero_MostrarMenu(playerid);

    format(g_Buffer, sizeof g_Buffer, "ADMINMALETERO admin=%d vehiculo_id=%d", Jugador[playerid][jID], g_Vehiculos[indice][veID]);
    EscribirLog("administracion", g_Buffer);
    return 1;
}

// Abre o cierra el maletero DE VERDAD en el mundo 3D (el componente
// "boot" del vehiculo, si el modelo lo tiene), a la vez que se abre o
// cierra el menu -- pedido expreso de Vlad el 06/09/2026, con foto de
// referencia. Lee los demas parametros antes de tocar solo el maletero
// para no pisar motor/luces/alarma/puertas/capo, que son de otros
// sistemas (Veh_ForzarMotor, Veh_ActualizarCierrePara...).
stock Maletero_AbrirFisico(indice, bool:abrir)
{
    if (!VehiculoValido(indice)) return 0;

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(g_Vehiculos[indice][veObjeto], motor, luces, alarma, puertas, capo, maletero, objetivo);
    SetVehicleParamsEx(g_Vehiculos[indice][veObjeto], motor, luces, alarma, puertas, capo,
        abrir ? VEHICLE_PARAMS_ON : VEHICLE_PARAMS_OFF, objetivo);
    return 1;
}

stock Maletero_MostrarMenu(playerid)
{
    new indice = g_MaleteroVehiculo[playerid];
    if (!VehiculoValido(indice)) return 0;

    // 400 se quedaba corto con Barbacoa + Pescado + los botones fijos
    // nuevos (mismo tipo de bug ya visto con GPS_TOTAL_DESTINOS: format()
    // trunca en silencio, sin avisar) -- subido a 700 con margen de sobra.
    new lista[700], linea[80], valor[24];

    format(lista, sizeof lista, "{E8B33A}Recurso\tCantidad\n");
    for (new r = 0; r < sizeof g_MaleteroRecursos; r++)
    {
        if (r == _:maPortatiles && !ItemValido(Item_Buscar(g_MaleteroRecursos[r][mrItemNombre]))) continue;

        if (r == _:maDinero) FormatearDinero(g_Maletero[indice][E_MALETERO:r], valor, sizeof valor);
        else format(valor, sizeof valor, "%d", g_Maletero[indice][E_MALETERO:r]);

        format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%s\n", g_MaleteroRecursos[r][mrEtiqueta], valor);
        strcat(lista, linea, sizeof lista);
    }

    // Fila extra dinamica del objeto Pawnshop (06/09/2026) -- no es uno
    // de los 5 recursos fijos, asi que no pasa por g_MaleteroRecursos.
    // Solo se puede sacar, nunca guardar uno a mano (ver
    // Maletero_Dialogo/DIALOGO_MALETERO).
    if (g_Maletero[indice][maObjeto] > 0)
    {
        new nombreObjeto[32];
        Item_Nombre(g_Maletero[indice][maObjeto], nombreObjeto, sizeof nombreObjeto);
        format(linea, sizeof linea, "{53C168}%s\t{B4B5B7}1\n", nombreObjeto);
        strcat(lista, linea, sizeof lista);
    }

    // Fila informativa del Pescado guardado (07/09/2026) -- no es un
    // recurso de g_MaleteroRecursos (cada pez pesa distinto, ver
    // g_MaleteroPescado arriba), asi que solo se cuenta cuantos hay.
    // Guardar/sacar de verdad son los dos botones fijos de abajo.
    new totalPescado = Maletero_PescadoTotal(indice);
    if (totalPescado > 0)
    {
        format(linea, sizeof linea, "{4FD1C5}Pescado guardado\t{B4B5B7}%d\n", totalPescado);
        strcat(lista, linea, sizeof lista);
    }

    // Botones fijos al final, peticion expresa de Vlad 07/09/2026 --
    // Guardar/Sacar Pescado mueven TODOS los peces de una vez (cada uno
    // conserva su peso individual, no son un recurso con cantidad como
    // los de arriba), y "Sacar todo" se lleva el resto del maletero
    // (dinero, materiales, marihuana, semillas, portatil, barbacoa,
    // objeto Pawnshop) de un solo golpe. Los indices de estos tres son
    // siempre Maletero_TotalFilasNormales() + (1 si hay objeto Pawnshop)
    // + 0/1/2, ver Maletero_Dialogo.
    strcat(lista, "{FF0000}GUARDAR PESCADO\t \n", sizeof lista);
    strcat(lista, "{FF0000}SACAR PESCADO\t \n", sizeof lista);
    strcat(lista, "{FF0000}Sacar todo el loot de una\t \n", sizeof lista);

    ShowPlayerDialog(playerid, DIALOGO_MALETERO, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8973A}Maletero", lista, "Elegir", "Cerrar");
    return 1;
}

// Traduce la fila del dialogo (que puede saltarse Portatil Hacker si el
// item todavia no existe, ver Maletero_MostrarMenu) al indice real de
// E_MALETERO.
stock Maletero_RecursoDeFila(fila)
{
    new contador = 0;
    for (new r = 0; r < sizeof g_MaleteroRecursos; r++)
    {
        if (r == _:maPortatiles && !ItemValido(Item_Buscar(g_MaleteroRecursos[r][mrItemNombre]))) continue;
        if (contador == fila) return r;
        contador++;
    }
    return -1;
}

// Cuantas filas de recursos fijos se ven en el menu ahora mismo (5, o 4
// si el Portatil Hacker todavia no existe como item) -- la fila del
// objeto Pawnshop, si la hay, es siempre la que va justo despues.
stock Maletero_TotalFilasNormales()
{
    new total = 0;
    for (new r = 0; r < sizeof g_MaleteroRecursos; r++)
    {
        if (r == _:maPortatiles && !ItemValido(Item_Buscar(g_MaleteroRecursos[r][mrItemNombre]))) continue;
        total++;
    }
    return total;
}

// Entrega el objeto Pawnshop del maletero al jugador y lo borra de ahi.
// Solo sacar -- nadie deposita un objeto Pawnshop a mano.
stock Maletero_SacarObjetoEspecial(playerid, indice)
{
    if (!Maletero_TieneAcceso(playerid, indice))
    {
        Error(playerid, "Se te acabo el tiempo de acceso al maletero.");
        return 0;
    }

    new item = g_Maletero[indice][maObjeto];
    if (!ItemValido(item)) return 0;

    new resultado = Inv_DarItem(playerid, item, 1, 0);
    if (resultado <= 0)
    {
        Error(playerid, (resultado == -1) ? "No te cabe en el inventario ahora mismo." : "No tienes ranuras libres.");
        return 0;
    }

    g_Maletero[indice][maObjeto] = 0;
    Maletero_Guardar(indice);

    new nombreObjeto[32];
    Item_Nombre(item, nombreObjeto, sizeof nombreObjeto);
    format(g_Buffer, sizeof g_Buffer, "Te llevas: %s.", nombreObjeto);
    Exito(playerid, g_Buffer);
    return 1;
}

// Cuanto puede GUARDAR el jugador de ese recurso ahora mismo (lo que
// lleva encima -- dinero en mano o unidades en el inventario).
stock Maletero_CantidadDisponibleParaGuardar(playerid, recurso)
{
    if (recurso == _:maDinero) return Jugador[playerid][jDinero];

    new item = Item_Buscar(g_MaleteroRecursos[recurso][mrItemNombre]);
    if (!ItemValido(item)) return 0;
    return Inv_TieneItem(playerid, item);
}

stock Maletero_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    switch (dialogid)
    {
        case DIALOGO_MALETERO:
        {
            if (!response) { Maletero_AbrirFisico(g_MaleteroVehiculo[playerid], false); g_MaleteroVehiculo[playerid] = -1; return 1; }

            new indiceMenu = g_MaleteroVehiculo[playerid];
            new bool:hayObjeto = (VehiculoValido(indiceMenu) && g_Maletero[indiceMenu][maObjeto] > 0);
            new bool:hayPescadoInfo = (VehiculoValido(indiceMenu) && Maletero_PescadoTotal(indiceMenu) > 0);
            new filaObjeto         = Maletero_TotalFilasNormales();
            new filaPescadoInfo    = filaObjeto      + (hayObjeto ? 1 : 0);
            new filaGuardarPescado = filaPescadoInfo + (hayPescadoInfo ? 1 : 0);
            new filaSacarPescado   = filaGuardarPescado + 1;
            new filaSacarTodo      = filaSacarPescado   + 1;

            // Botones fijos del final (mismo orden que Maletero_MostrarMenu).
            if (VehiculoValido(indiceMenu) && listitem == filaSacarTodo)
            {
                Maletero_SacarTodo(playerid, indiceMenu);
                Maletero_AbrirFisico(indiceMenu, false);
                g_MaleteroVehiculo[playerid] = -1;
                return 1;
            }
            if (VehiculoValido(indiceMenu) && listitem == filaSacarPescado)
            {
                Maletero_SacarPescado(playerid, indiceMenu);
                Maletero_AbrirFisico(indiceMenu, false);
                g_MaleteroVehiculo[playerid] = -1;
                return 1;
            }
            if (VehiculoValido(indiceMenu) && listitem == filaGuardarPescado)
            {
                Maletero_GuardarPescado(playerid, indiceMenu);
                Maletero_AbrirFisico(indiceMenu, false);
                g_MaleteroVehiculo[playerid] = -1;
                return 1;
            }

            // Fila informativa "Pescado guardado" (si la hay): no hace
            // nada al pulsarla, solo ocupaba un hueco en la lista.
            if (hayPescadoInfo && listitem == filaPescadoInfo)
            {
                Maletero_AbrirFisico(indiceMenu, false);
                g_MaleteroVehiculo[playerid] = -1;
                return 1;
            }

            // Fila del objeto Pawnshop (si la hay): se saca directamente,
            // sin pasar por el submenu Guardar/Sacar.
            if (hayObjeto && listitem == filaObjeto)
            {
                Maletero_SacarObjetoEspecial(playerid, indiceMenu);
                Maletero_AbrirFisico(indiceMenu, false);
                g_MaleteroVehiculo[playerid] = -1;
                return 1;
            }

            new recurso = Maletero_RecursoDeFila(listitem);
            if (recurso == -1) { Maletero_AbrirFisico(g_MaleteroVehiculo[playerid], false); g_MaleteroVehiculo[playerid] = -1; return 1; }

            g_MaleteroRecursoSel[playerid] = recurso;

            format(g_Buffer, sizeof g_Buffer, "{E8973A}%s", g_MaleteroRecursos[recurso][mrEtiqueta]);
            ShowPlayerDialog(playerid, DIALOGO_MALETERO_ACCION, DIALOG_STYLE_LIST,
                g_Buffer, "Guardar (de mi, al maletero)\nSacar (del maletero, a mi)", "Elegir", "Atras");
            return 1;
        }

        case DIALOGO_MALETERO_ACCION:
        {
            if (!response) { Maletero_MostrarMenu(playerid); return 1; }

            new indice = g_MaleteroVehiculo[playerid];
            if (!VehiculoValido(indice)) { g_MaleteroVehiculo[playerid] = -1; return 1; }

            new recurso = g_MaleteroRecursoSel[playerid];
            new bool:guardar = (listitem == 0);
            g_MaleteroModoGuardar[playerid] = guardar;

            new maximo = guardar
                ? Maletero_CantidadDisponibleParaGuardar(playerid, recurso)
                : g_Maletero[indice][E_MALETERO:recurso];

            if (maximo <= 0)
            {
                Error(playerid, guardar ? "No llevas nada de eso encima." : "El maletero no tiene nada de eso.");
                Maletero_AbrirFisico(indice, false);
                g_MaleteroVehiculo[playerid] = -1;
                return 1;
            }

            g_MaleteroCantidadMax[playerid] = maximo;

            format(g_Buffer, sizeof g_Buffer, "Cuanto quieres %s? (1 a %d)",
                guardar ? ("guardar") : ("sacar"), maximo);
            ShowPlayerDialog(playerid, DIALOGO_MALETERO_CANTIDAD, DIALOG_STYLE_INPUT,
                "{E8973A}Maletero", g_Buffer, "Confirmar", "Cancelar");
            return 1;
        }

        case DIALOGO_MALETERO_CANTIDAD:
        {
            // Sea cual sea la salida de este dialogo, no se vuelve a
            // abrir ningun otro -- el flujo del maletero termina aqui
            // siempre, asi que el maletero fisico se cierra en los
            // cuatro caminos de abajo.
            new indice = g_MaleteroVehiculo[playerid];

            if (!response) { Maletero_AbrirFisico(indice, false); g_MaleteroVehiculo[playerid] = -1; return 1; }

            if (!EsNumeroEntero(inputtext) || strval(inputtext) < 1 || strval(inputtext) > g_MaleteroCantidadMax[playerid])
            {
                Error(playerid, "Cantidad invalida.");
                Maletero_AbrirFisico(indice, false);
                g_MaleteroVehiculo[playerid] = -1;
                return 1;
            }

            new cantidad = strval(inputtext);
            if (!VehiculoValido(indice)) { g_MaleteroVehiculo[playerid] = -1; return 1; }

            new recurso = g_MaleteroRecursoSel[playerid];

            if (g_MaleteroModoGuardar[playerid]) Maletero_Guardar_Aplicar(playerid, indice, recurso, cantidad);
            else                                 Maletero_Sacar_Aplicar(playerid, indice, recurso, cantidad);

            Maletero_AbrirFisico(indice, false);
            g_MaleteroVehiculo[playerid] = -1;
            return 1;
        }
    }
    return 0;
}

// GUARDAR: primero se anade al maletero (nunca falla, sin tope), y solo
// entonces se quita al jugador -- mismo orden por seguridad que
// Casa_AlmacenAgregar + Inv_QuitarItem en 24_casas.inc (si el segundo
// paso fallase por lo que sea, es mejor que sobre en los dos sitios un
// instante a que desaparezca).
stock Maletero_Guardar_Aplicar(playerid, indice, recurso, cantidad)
{
    // Revalidacion final -- si esto es un acceso forzado y la ventana de
    // 60s vencio mientras el jugador tenia el dialogo de cantidad
    // abierto, no se aplica nada. El reloj es gettime() del servidor,
    // cerrar y reabrir dialogos no lo mueve.
    if (!Maletero_TieneAcceso(playerid, indice))
    {
        Error(playerid, "Se te acabo el tiempo de acceso al maletero.");
        return 0;
    }

    if (recurso == _:maDinero)
    {
        if (!TieneDinero(playerid, cantidad)) { Error(playerid, "No llevas tanto dinero encima."); return 0; }

        Maletero_Mover(indice, E_MALETERO:recurso, cantidad);
        DarDinero(playerid, -cantidad);
        Exito(playerid, "Dinero guardado en el maletero.");
        return 1;
    }

    new item = Item_Buscar(g_MaleteroRecursos[recurso][mrItemNombre]);
    if (!ItemValido(item) || Inv_TieneItem(playerid, item) < cantidad)
    {
        Error(playerid, "No llevas tanto encima.");
        return 0;
    }

    Maletero_Mover(indice, E_MALETERO:recurso, cantidad);
    Inv_QuitarItem(playerid, item, cantidad);
    Exito(playerid, "Guardado en el maletero.");
    return 1;
}

// SACAR: primero se retira del maletero (Maletero_Mover ya valida que
// haya suficiente), y si dárselo al jugador falla (sin hueco/demasiado
// peso) se REEMBOLSA al maletero antes de avisar del error -- nunca se
// pierde nada por el camino.
stock Maletero_Sacar_Aplicar(playerid, indice, recurso, cantidad)
{
    if (!Maletero_TieneAcceso(playerid, indice))
    {
        Error(playerid, "Se te acabo el tiempo de acceso al maletero.");
        return 0;
    }

    if (recurso == _:maDinero)
    {
        if (!Maletero_Mover(indice, E_MALETERO:recurso, -cantidad))
        {
            Error(playerid, "El maletero no tiene tanto.");
            return 0;
        }
        DarDinero(playerid, cantidad);
        Exito(playerid, "Dinero sacado del maletero.");
        return 1;
    }

    new item = Item_Buscar(g_MaleteroRecursos[recurso][mrItemNombre]);
    if (!ItemValido(item))
    {
        Error(playerid, "Ese objeto no existe todavia.");
        return 0;
    }

    if (!Maletero_Mover(indice, E_MALETERO:recurso, -cantidad))
    {
        Error(playerid, "El maletero no tiene tanto.");
        return 0;
    }

    new resultado = Inv_DarItem(playerid, item, cantidad, 0);
    if (resultado <= 0)
    {
        Maletero_Mover(indice, E_MALETERO:recurso, cantidad); // reembolso, nada se pierde
        Error(playerid, (resultado == -1) ? "No te cabe en el inventario ahora mismo." : "No tienes ranuras libres.");
        return 0;
    }

    Exito(playerid, "Sacado del maletero.");
    return 1;
}

// "Sacar todo el loot de una" (07/09/2026, peticion expresa de Vlad) --
// intenta darle al jugador CADA recurso que haya en el maletero (dinero
// y los items uno a uno, incluido el objeto Pawnshop si hay). Lo que no
// quepa en el inventario se queda dentro sin perderse -- mismo
// comportamiento de reembolso que ya usa Maletero_Sacar_Aplicar, solo
// que aqui se resume todo en UN mensaje final en vez de uno por recurso.
stock Maletero_SacarTodo(playerid, indice)
{
    if (!Maletero_TieneAcceso(playerid, indice))
    {
        Error(playerid, "Se te acabo el tiempo de acceso al maletero.");
        return 0;
    }

    new bool:algoSacado = false;
    new bool:algoSeQuedo = false;

    if (g_Maletero[indice][maDinero] > 0)
    {
        new cantidad = g_Maletero[indice][maDinero];
        Maletero_Mover(indice, E_MALETERO:maDinero, -cantidad);
        DarDinero(playerid, cantidad);
        algoSacado = true;
    }

    for (new r = 0; r < sizeof g_MaleteroRecursos; r++)
    {
        if (r == _:maDinero) continue;
        if (r == _:maPortatiles && !ItemValido(Item_Buscar(g_MaleteroRecursos[r][mrItemNombre]))) continue;

        new cantidad = g_Maletero[indice][E_MALETERO:r];
        if (cantidad <= 0) continue;

        new item = Item_Buscar(g_MaleteroRecursos[r][mrItemNombre]);
        if (!ItemValido(item)) continue;

        new resultado = Inv_DarItem(playerid, item, cantidad, 0);
        if (resultado <= 0)
        {
            algoSeQuedo = true; // sin hueco/demasiado peso -- se queda dentro, nada que reembolsar
            continue;
        }

        Maletero_Mover(indice, E_MALETERO:r, -cantidad);
        algoSacado = true;
    }

    if (g_Maletero[indice][maObjeto] > 0)
    {
        new item = g_Maletero[indice][maObjeto];
        new resultado = Inv_DarItem(playerid, item, 1, 0);
        if (resultado <= 0)
        {
            algoSeQuedo = true;
        }
        else
        {
            g_Maletero[indice][maObjeto] = 0;
            algoSacado = true;
        }
    }

    Maletero_Guardar(indice);

    if (!algoSacado && !algoSeQuedo)
    {
        Error(playerid, "El maletero no tiene nada.");
    }
    else if (algoSeQuedo)
    {
        Info(playerid, "Te has llevado lo que te cabia. Algo se quedo en el maletero por falta de espacio.");
    }
    else
    {
        Exito(playerid, "Te has llevado todo el maletero.");
    }
    return 1;
}

// "GUARDAR PESCADO" (07/09/2026, peticion expresa de Vlad) -- recorre
// TODO el inventario del jugador y mete cada pez crudo que lleve en la
// primera ranura libre del maletero de pescado, conservando su peso
// individual (SLOT_DATOS). Todo o nada por pieza: si el maletero de
// pescado ya esta lleno (las 20 ranuras ocupadas, caso extremo), los
// peces que no quepan se quedan en el inventario del jugador, nunca se
// pierden.
stock Maletero_GuardarPescado(playerid, indice)
{
    if (!Maletero_TieneAcceso(playerid, indice))
    {
        Error(playerid, "Se te acabo el tiempo de acceso al maletero.");
        return 0;
    }

    new guardados = 0, noCupo = 0;
    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        new item = Inv_ItemEnSlot(playerid, s);
        if (!Pesca_EsPezCrudo(item)) continue;

        new datos = Inv_DatosEnSlot(playerid, s);
        if (Maletero_PescadoAgregar(indice, item, datos))
        {
            Inv_VaciarSlot(playerid, s);
            guardados++;
        }
        else
        {
            noCupo++;
        }
    }

    if (guardados == 0 && noCupo == 0)
    {
        Error(playerid, "No llevas ningun pescado encima.");
    }
    else if (noCupo > 0)
    {
        format(g_Buffer, sizeof g_Buffer, "Guardados %d peces. El maletero de pescado esta lleno, %d se quedaron contigo.", guardados, noCupo);
        Info(playerid, g_Buffer);
    }
    else
    {
        format(g_Buffer, sizeof g_Buffer, "Guardados %d peces en el maletero.", guardados);
        Exito(playerid, g_Buffer);
    }
    return 1;
}

// "SACAR PESCADO" -- inverso: da al jugador TODOS los peces guardados en
// este maletero, uno a uno con su peso original. Lo que no quepa en el
// inventario (peso o ranuras) se queda dentro, nunca se pierde -- mismo
// criterio que Maletero_SacarTodo.
stock Maletero_SacarPescado(playerid, indice)
{
    if (!Maletero_TieneAcceso(playerid, indice))
    {
        Error(playerid, "Se te acabo el tiempo de acceso al maletero.");
        return 0;
    }

    new sacados = 0, noCupo = 0;
    for (new s = 0; s < MALETERO_PESCADO_SLOTS; s++)
    {
        new item = g_MaleteroPescado[indice][s][MP_ITEM];
        if (item == 0) continue;

        new datos = g_MaleteroPescado[indice][s][MP_DATOS];
        new resultado = Inv_DarItem(playerid, item, 1, datos);
        if (resultado <= 0)
        {
            noCupo++;
            continue;
        }

        g_MaleteroPescado[indice][s][MP_ITEM]  = 0;
        g_MaleteroPescado[indice][s][MP_DATOS] = 0;
        Maletero_PescadoBorrarFila(indice, s);
        sacados++;
    }

    if (sacados == 0 && noCupo == 0)
    {
        Error(playerid, "El maletero no tiene pescado guardado.");
    }
    else if (noCupo > 0)
    {
        format(g_Buffer, sizeof g_Buffer, "Te has llevado %d peces. %d se quedaron dentro por falta de espacio.", sacados, noCupo);
        Info(playerid, g_Buffer);
    }
    else
    {
        format(g_Buffer, sizeof g_Buffer, "Te has llevado %d peces del maletero.", sacados);
        Exito(playerid, g_Buffer);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  FASE 3 -- /forzarmaletero. Mecanica inspirada en /puente
//  (22_dropeo.inc: freeze de 4s + tirada al terminar), pero con su
//  propia probabilidad (33%, no el 30% real de /puente -- desacoplado
//  a proposito, encargo de Vlad) y un segundo efecto independiente: el
//  aviso a la policia se tira SIEMPRE al ejecutar el comando, gane o
//  pierda el forzado.
// ---------------------------------------------------------------------
static bool:g_MaleteroForzando[MAX_PLAYERS];
static g_MaleteroForzarTimer[MAX_PLAYERS] = {-1, ...};
static g_MaleteroForzarVehiculo[MAX_PLAYERS] = {-1, ...};
static bool:g_MaleteroOcupado[MAX_VEHICULOS];
static g_MaleteroForzarSegundos[MAX_PLAYERS]; // cuenta atras en pantalla, ver Maletero_ForzarTick

CMD:forzarmaletero(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (g_MaleteroForzando[playerid])
    {
        Error(playerid, "Ya estas forzando un maletero. Espera.");
        return 1;
    }

    new indice = Veh_BuscarCercano(playerid, MALETERO_RADIO);
    if (indice == -1)
    {
        Error(playerid, "No hay ningun vehiculo cerca.");
        return 1;
    }

    // No se puede usar sobre el propio vehiculo para saltarse las
    // reglas normales -- eso es lo que ya hace /maletero sin riesgo.
    if (g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID])
    {
        Error(playerid, "Es tu vehiculo -- usa /maletero.");
        return 1;
    }

    if (g_MaleteroOcupado[indice])
    {
        Error(playerid, "Alguien esta forzando este maletero ahora mismo.");
        return 1;
    }

    g_MaleteroForzando[playerid]        = true;
    g_MaleteroForzarVehiculo[playerid]  = indice;
    g_MaleteroOcupado[indice]           = true;

    TogglePlayerControllable(playerid, false);

    // Animacion de forzado en vez de quedarse plantado en pose neutra
    // durante los 8s -- pedido expreso de Vlad el 06/09/2026 ("que no nos
    // congele, no parece premium"). Sigue sin poder moverse
    // (TogglePlayerControllable ya se encarga de eso), pero ahora se ve
    // haciendo algo. "CAR"/"crckdoor" es la animacion de forzar una
    // puerta de coche del propio GTA:SA -- sin verificar en este
    // servidor todavia, decir si no encaja para cambiarla.
    Animar(playerid, "CAR", "crckdoor", 4.0, true, true, true, false, 0);

    // Cuenta atras en pantalla en vez del chat -- pedido expreso de Vlad
    // el 06/09/2026, mismo patron que Atracos_Segundo (39_atracos.inc):
    // un GameText que se reescribe cada segundo hasta llegar a 0.
    g_MaleteroForzarSegundos[playerid] = FORZAR_MALETERO_FREEZE_MS / 1000;
    format(g_Buffer, sizeof g_Buffer, "~w~Forzando Maletero...~n~~r~%d s", g_MaleteroForzarSegundos[playerid]);
    GameTextForPlayer(playerid, g_Buffer, 1100, 4);

    g_MaleteroForzarTimer[playerid] =
        SetTimerEx("Maletero_ForzarTick", 1000, true, "i", playerid);
    return 1;
}

// Un tic por segundo mientras dura el forzado: actualiza la cuenta atras
// en pantalla y, al llegar a 0, resuelve el intento (MaleteroForzarFin)
// y se autodestruye -- sustituye al timer unico de antes.
forward Maletero_ForzarTick(playerid);
public  Maletero_ForzarTick(playerid)
{
    if (!IsPlayerConnected(playerid) || !g_MaleteroForzando[playerid])
    {
        if (g_MaleteroForzarTimer[playerid] != -1)
        {
            KillTimer(g_MaleteroForzarTimer[playerid]);
            g_MaleteroForzarTimer[playerid] = -1;
        }
        return 0;
    }

    g_MaleteroForzarSegundos[playerid]--;
    if (g_MaleteroForzarSegundos[playerid] <= 0)
    {
        KillTimer(g_MaleteroForzarTimer[playerid]);
        g_MaleteroForzarTimer[playerid] = -1;
        MaleteroForzarFin(playerid);
        return 1;
    }

    format(g_Buffer, sizeof g_Buffer, "~w~Forzando Maletero...~n~~r~%d s", g_MaleteroForzarSegundos[playerid]);
    GameTextForPlayer(playerid, g_Buffer, 1100, 4);
    return 1;
}

forward MaleteroForzarFin(playerid);
public  MaleteroForzarFin(playerid)
{
    if (!IsPlayerConnected(playerid) || !g_MaleteroForzando[playerid]) return 0;

    g_MaleteroForzando[playerid]  = false;
    g_MaleteroForzarTimer[playerid] = -1;
    TogglePlayerControllable(playerid, true);
    ClearAnimations(playerid);

    new indice = g_MaleteroForzarVehiculo[playerid];
    g_MaleteroForzarVehiculo[playerid] = -1;

    if (!VehiculoValido(indice))
    {
        Error(playerid, "El vehiculo ya no esta disponible.");
        return 0;
    }
    g_MaleteroOcupado[indice] = false;

    // --- Aviso policial: tirada INDEPENDIENTE del resultado del
    // forzado, siempre ocurre al ejecutar el comando (regla explicita
    // del encargo, no solo si el forzado tiene exito). ---
    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);
    if (Policia_CrearAviso(AVISO_MALETERO, playerid, px, py, pz, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid)))
    {
        SendClientMessage(playerid, COLOR_AZUL, "La policia esta de camino.");
    }

    new nombreJugador[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombreJugador, sizeof nombreJugador);

    if (random(100) < FORZAR_MALETERO_EXITO)
    {
        g_MaleteroAccesoVehiculo[playerid] = indice;
        g_MaleteroAccesoHasta[playerid]    = gettime() + FORZAR_MALETERO_VENTANA;
        Teletipo_RegistrarMaleteroExito(); // 62_teletipo.inc -- solo cuenta para el aviso de racha

        // Chat de exito/fallo retirado el 08/09/2026 -- el texto grande en
        // pantalla ya lo dice, el del chat era innecesario (peticion
        // expresa de Vlad). Los "60 segundos" del texto viejo pasan al
        // propio GameText para no perder esa info.
        GameTextForPlayer(playerid, "~g~MALETERO FORZADO ~w~(60s)", 3000, 3);

        g_MaleteroVehiculo[playerid] = indice;
        Maletero_AbrirFisico(indice, true);
        Maletero_MostrarMenu(playerid);

        format(g_Buffer, sizeof g_Buffer,
            "MALETERO_FORZADO_EXITO jugador=%s vehiculo_id=%d timestamp=%d",
            nombreJugador, g_Vehiculos[indice][veID], gettime());
        EscribirLog("maletero", g_Buffer);
    }
    else
    {
        GameTextForPlayer(playerid, "~r~NO HAS CONSEGUIDO FORZARLO", 3000, 3);

        format(g_Buffer, sizeof g_Buffer,
            "MALETERO_FORZADO_FALLO jugador=%s vehiculo_id=%d timestamp=%d",
            nombreJugador, g_Vehiculos[indice][veID], gettime());
        EscribirLog("maletero", g_Buffer);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  CICLO DE VIDA -- mismo patron que Dropeo_OnConectar/OnDesconectar
//  (22_dropeo.inc): limpiar estado de sesion al entrar y liberar
//  cualquier candado (g_MaleteroOcupado) si se desconecta a mitad del
//  freeze de /forzarmaletero.
// ---------------------------------------------------------------------
forward Maletero_OnConectar(playerid);
public  Maletero_OnConectar(playerid)
{
    g_MaleteroVehiculo[playerid]        = -1;
    g_MaleteroForzando[playerid]        = false;
    g_MaleteroForzarTimer[playerid]     = -1;
    g_MaleteroForzarVehiculo[playerid]  = -1;
    g_MaleteroAccesoVehiculo[playerid]  = -1;
    g_MaleteroAccesoHasta[playerid]     = 0;
    return 1;
}

forward Maletero_OnDesconectar(playerid);
public  Maletero_OnDesconectar(playerid)
{
    // Seguridad: si se desconecta con el menu de /maletero abierto en
    // pantalla, que no se quede el maletero abierto en el mundo para
    // siempre (nadie va a mandar el "Cerrar" que ya no puede pulsar).
    if (g_MaleteroVehiculo[playerid] != -1) Maletero_AbrirFisico(g_MaleteroVehiculo[playerid], false);

    if (g_MaleteroForzando[playerid] && g_MaleteroForzarTimer[playerid] != -1)
    {
        KillTimer(g_MaleteroForzarTimer[playerid]);
        g_MaleteroForzarTimer[playerid] = -1;
    }
    g_MaleteroForzando[playerid] = false;

    if (g_MaleteroForzarVehiculo[playerid] != -1)
    {
        new i = g_MaleteroForzarVehiculo[playerid];
        if (i >= 0 && i < MAX_VEHICULOS) g_MaleteroOcupado[i] = false;
        g_MaleteroForzarVehiculo[playerid] = -1;
    }
    return 1;
}
