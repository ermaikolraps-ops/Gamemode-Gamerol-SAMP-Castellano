/* =====================================================================
 *  43_federico.inc
 *  ---------------------------------------------------------------------
 *  "LOS COCHES DE FEDERICO" -- v1 (03/09/2026, encargo directo de Vlad).
 *
 *  Cada FEDERICO_DURACION_RONDA segundos (2h), Federico busca 3 MODELOS
 *  distintos del pool de vehiculos libres (VEH_DROPEO, sin dueno,
 *  32_negocios.inc no interviene aqui). El jugador que entregue el
 *  PRIMER vehiculo de un modelo buscado, conduciendolo hasta Federico y
 *  usando /entregarcoche, cobra la recompensa; ese modelo queda
 *  ENTREGADO el resto de la ronda -- no se sustituye por otro ni se
 *  puede volver a cobrar por el mismo modelo hasta la siguiente ronda.
 *  (Hubo una version intermedia el mismo 03/09/2026 sin este limite;
 *  Vlad pidio revertirlo horas despues -- vuelve a ser 1 por modelo.)
 *
 *  El vehiculo entregado desaparece 24h (columna vehiculos.federico_until,
 *  NULL = no retirado) y vuelve solo, con su identidad intacta, a su
 *  ULTIMA POSICION CONOCIDA.
 *
 *  ADAPTACION AL SISTEMA REAL, documentada segun se pidio en el encargo:
 *  el proyecto NO tiene dos campos separados de "posicion inicial" y
 *  "ultimo /park" -- 13_vehiculos.inc usa un unico pos_x/y/z/a para las
 *  dos cosas (Veh_Aparcar sobreescribe ese mismo campo). Como ademas
 *  /park solo funciona sobre vehiculos CON dueno (no se puede parquear
 *  un libre todavia sin comprar), un vehiculo libre nunca ha sido
 *  tocado por /park mientras sigue libre -- su pos_x/y/z/a de hoy YA ES
 *  su posicion inicial, o la de su ultimo /park si en algun momento fue
 *  de un jugador y luego se solto/dropeo. No hacia falta un segundo
 *  campo: devolverlo con Veh_Aparecer() sobre su fila actual ya cumple
 *  exactamente la prioridad 1/2 que pedia el encargo.
 *
 *  PERSISTENCIA: tabla `federico_estado` (fila unica, id=1) con la
 *  ronda vigente, y `vehiculos.federico_until` por vehiculo retirado.
 *  Ver sql/ (migracion aplicada directamente, sin fichero versionado en
 *  el proyecto -- ver el informe de entrega para el SQL exacto).
 *
 *  UBICACION: ver FEDERICO_X/Y/Z en 01_configuracion.inc -- SIN
 *  VALIDAR CON CLIENTE REAL, ver LEEME.md.
 *
 *  UI (v2, 03/09/2026 mas tarde): /federico ya NO abre un dialogo de
 *  texto -- abre una pantalla casi completa hecha con TextDraws
 *  globales (g_FedGlobalTD / g_FedSlotTD), con las 3 tarjetas
 *  mostrando el vehiculo real en 3D girando dentro de la caja negra
 *  (TextDrawSetPreviewModel: nativo del juego, renderiza el modelo de
 *  verdad, no una imagen). Nombre, rareza y precio debajo de cada
 *  preview; el modelo ya entregado en la ronda se tacha con una barra
 *  roja (TextDraw en caja, sin rotacion -- SA-MP/open.mp no soporta
 *  rotar un TextDraw normal, solo el preview 3D). El boton "Cerrar" es
 *  un TextDraw seleccionable (TextDrawSetSelectable + SelectTextDraw),
 *  igual que los botones de la pantalla de login (06_login.inc) -- se
 *  siguio ese mismo patron de esta casa para todo: nombres, colores
 *  (COLOR_VERDE, COLOR_GRIS, COLOR_ROJO_FUERTE de 01_configuracion.inc)
 *  y la forma de crear/mostrar/ocultar los TextDraws.
 *  SIN VALIDAR CON CLIENTE REAL -- coordenadas y preview ajustados por
 *  calculo, no por captura de pantalla in-game real.
 * ===================================================================== */

enum E_FEDERICO_SLOT
{
    feModelo,
    feNombre[32],
    feRareza[16],
    feRecompensa,
    bool:feEntregado,
    feVehiculoID            // vehiculos.id entregado en este slot, 0 = ninguno
}
new g_FedericoSlot[FEDERICO_TOTAL_SLOTS][E_FEDERICO_SLOT];

new g_FedericoStartedAt      = 0;
new g_FedericoNextRotationAt = 0;
new bool:g_FedericoListo     = false;      // true tras la primera carga/ronda real

// SINCRONIZACION GENERAL (08/09/2026, peticion expresa de Vlad): ronda
// normal, Coche Secreto, Moto del Dia y Chatarra rotan SIEMPRE juntas, en
// el mismo tick, usando UN SOLO reloj (g_FedericoNextRotationAt) -- asi
// /verlista cambia entera de golpe una vez al dia. El Personaje Historico
// queda fuera a proposito: sigue siendo 100% manual via /crearhistorico.
// g_FedericoSyncListo evita que la sincronizacion se dispare antes de que
// las 4 hayan terminado de cargar de la BD (4 consultas asincronas
// independientes, ver 13_vehiculos.inc). Ver Federico_Segundo() y
// Federico_RondaGeneralSincronizar() mas abajo.
static bool:g_FedericoSyncListo = false;

// Lock explicito anti doble-cobro (ver CMD:entregarcoche). Pawn procesa
// comandos de forma sincrona de por si, pero el contrato queda escrito
// aqui a proposito en vez de confiar solo en eso.
static bool:g_FedericoProcesando = false;

// Antispam del aviso/checkpoint al entrar como conductor: un aviso cada
// FEDERICO_AVISO_COOLDOWN segundos por jugador, no por vehiculo.
static g_FedericoUltimoAviso[MAX_PLAYERS];
static g_FedericoDestinoSlot[MAX_PLAYERS] = {-1, ...};

// Coherencia audiovisual (08/09/2026): aviso grande "COCHE BUSCADO POR
// FEDERICO!" al montarse en CUALQUIER vehiculo que Federico busque ahora
// mismo -- copia exacta del estilo de /arreglarcoche (53_arreglarcoche.inc).
#define FEDERICO_BUSCADO_AVISO_MS 4000   // mismo tiempo que ya usa /arreglarcoche
enum { FBD_TITULO, FBD_SUBTITULO, FBD_TOTAL }
static PlayerText:g_FedBuscadoTD[MAX_PLAYERS][FBD_TOTAL];

// Buffer PROPIO para las consultas SQL de este modulo -- el g_Buffer
// global (02_datos.inc) es de 512 bytes y la consulta REPLACE INTO de
// Federico_Persistir lo desborda (mysql_format lo trunca en silencio y
// dana el contenido para quien use g_Buffer justo despues). Nunca se
// comparte este buffer con nada que no sea una consulta SQL de Federico.
static g_FedericoConsulta[768];

// ---------------------------------------------------------------------
//  COCHE SECRETO (05/09/2026) -- cuarto coche buscado, aparte de los 3
//  de siempre. Estado propio en su propia tabla (federico_secreto), NO
//  mezclado con g_FedericoSlot/federico_estado: asi la ronda normal de
//  2h no se toca para nada. Nunca se expone feModelo/nombre real fuera
//  de este archivo -- solo la letra inicial y el nombre del color.
// ---------------------------------------------------------------------
new g_FedSecretoModelo         = 0;      // 0 = no hay candidato esta ronda de 24h
new g_FedSecretoColor1         = 0;
new g_FedSecretoColorNombre[16] = "";
new g_FedSecretoLetra[2]       = "-";
new g_FedSecretoStartedAt      = 0;
new g_FedSecretoNextRotationAt = 0;
new bool:g_FedSecretoEntregado = false;
new g_FedSecretoVehiculoID     = 0;
new bool:g_FedSecretoListo     = false;

// ---------------------------------------------------------------------
//  LA MOTO DEL DIA (07/09/2026, peticion expresa de Vlad) -- 5o coche
//  buscado, aparte de los 3 normales/Secreto/Historico/chatarra. A
//  diferencia del Secreto, se revela directamente (sin acertijo de letra
//  o color) -- mismo espiritu que un normal, solo que restringido a
//  motos de verdad (nada de bicicletas ni quads) y con su propia
//  recompensa fija. Rota cada 24h, igual que el Secreto. Estado propio
//  en su propia tabla (federico_moto), no mezclado con los demas.
//  Tarjeta SOLO en la web (el juego/federico, entre "Personaje
//  Historico" y la mini lista chatarra, orden pedido por Vlad) -- las 3
//  tarjetas in-game ya llenan el ancho de la pantalla, y el cliente
//  movil no pinta texto por debajo de y~340 (ver el comentario junto a
//  FTD_CERRAR_CAJA), asi que no se le hizo hueco ahi. Funcionalmente SI
//  funciona igual en el juego: aviso al montarse, /entregarcoche la
//  reconoce y paga igual que cualquier otra.
// ---------------------------------------------------------------------
new const g_FedericoMotos[] = {448, 461, 462, 463, 468, 521, 522, 581, 586};

new g_FedMotoModelo         = 0;      // 0 = no hay candidato esta ronda de 24h
new g_FedMotoStartedAt      = 0;
new g_FedMotoNextRotationAt = 0;
new bool:g_FedMotoEntregado = false;
new g_FedMotoVehiculoID     = 0;
new bool:g_FedMotoListo     = false;

// Colores con nombre reconocible (indice de paleta de vehiculo SA-MP,
// hex RGBA real sacado de la tabla oficial de open.mp/docs -- nunca
// adivinado). El Coche Secreto solo puede salir en uno de estos
// colores -- mejor una pista clara y corta que arriesgarse a nombrar
// mal alguno de los 256. Ampliada el 05/09/2026 (encargo de Vlad, meter
// ya un candidato real) leyendo el hex de los indices que de verdad
// tenian los coches unicos del pool en ese momento.
enum E_COLOR_SECRETO { csIndice, csNombre[16] };
new const g_ColoresConNombre[][E_COLOR_SECRETO] = {
    {0,   "Negro"},
    {1,   "Blanco"},
    {2,   "Azul"},
    {3,   "Rojo"},
    {6,   "Naranja"},
    {9,   "Gris"},
    {30,  "Plata"},
    {40,  "Marron"},
    {131, "Morado"},
    {132, "Verde"},
    {145, "Rosa"},
    {31,  "Granate"},       // 0x5F272B
    {49,  "Beige"},         // 0x9D9872
    {50,  "Caqui"},         // 0x7A7560
    {60,  "Beige"},         // 0x9C8D71
    {66,  "Vino"},          // 0x851F2E
    {84,  "Turquesa"},      // 0x0F6A89
    {91,  "Azul marino"},   // 0x134573
    {97,  "Azul"},          // 0x105082
    {102, "Granate"},       // 0x730A27
    {108, "Indigo"}         // 0x25225A
};

stock bool:Federico_ColorConNombre(color1, destino[], tam)
{
    for (new i = 0; i < sizeof g_ColoresConNombre; i++)
    {
        if (g_ColoresConNombre[i][csIndice] == color1)
        {
            strmid(destino, g_ColoresConNombre[i][csNombre], 0, tam - 1, tam);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------
//  TEXTDRAWS -- pantalla de /federico. TextDraws GLOBALES (no
//  PlayerTextDraw): el contenido es el mismo para todo el mundo (la
//  ronda es unica y compartida), asi que se crean UNA vez en
//  Federico_CrearTextdraws() y solo se Show/Hide por jugador. Igual que
//  el patron de 06_login.inc (g_TD[]).
// ---------------------------------------------------------------------
#define FTD_FONDO          0
#define FTD_TITULO         1
#define FTD_SUBTITULO      2
#define FTD_PIE            3
#define FTD_CERRAR_CAJA    4
#define FTD_CERRAR_TXT     5
#define FTD_TOTAL          6
new Text:g_FedGlobalTD[FTD_TOTAL];

#define FSD_PREVIEW        0
#define FSD_NOMBRE         1
#define FSD_RAREZA         2
#define FSD_PRECIO         3
#define FSD_ESTADO         4
#define FSD_TACHON         5
#define FSD_TOTAL          6
new Text:g_FedSlotTD[FEDERICO_TOTAL_SLOTS][FSD_TOTAL];

new bool:g_FedericoUIAbierta[MAX_PLAYERS] = {false, ...};

// Marcador fisico en el mundo (04/09/2026) -- objeto plano con "$" como
// textura de texto (misma tecnica que un filterscript de referencia de
// marcadores 3D revisado ese dia, adaptada: aqui es UN objeto global fijo,
// no un sistema por-jugador) mas la etiqueta 3D, siguiendo el mismo patron
// ya usado en Banco_CrearMarcadores (15_banco.inc): CreateDynamicPickup +
// CreateDynamic3DTextLabel. SIN VALIDAR CON CLIENTE REAL -- igual que el
// resto de UI de Federico (ver cabecera del archivo).
// [0] = Los Santos, [1] = Angel Pine (07/09/2026).
static STREAMER_TAG_OBJECT:g_FedMarcadorDinero[2] = {STREAMER_TAG_OBJECT:-1, ...};
static STREAMER_TAG_3D_TEXT_LABEL:g_FedMarcadorEtiqueta[2] = {STREAMER_TAG_3D_TEXT_LABEL:-1, ...};

// ---------------------------------------------------------------------
//  RAREZA -- 3a tabla de escalones, pedida por Vlad el 10/09/2026 (2a
//  version del 04/09/2026 desplazada un escalon: antes <=1/2/3/>=4,
//  ahora <=2/3/4/>=5). Se calculan UNA VEZ al fijar la ronda; nunca se
//  recalculan durante las 2h.
// ---------------------------------------------------------------------
// Recompensas subidas el 07/09/2026, peticion expresa de Vlad (antes
// 120.000/75.000/40.000/10.000) -- sin cambios el 10/09/2026, solo se
// desplazaron los umbrales de cantidad, no los importes.
stock Federico_Rareza(cantidad, rareza[], tam, &recompensa)
{
    if (cantidad <= 2)      { format(rareza, tam, "SUPER RARE");    recompensa = 135000; }
    else if (cantidad == 3) { format(rareza, tam, "RARE");          recompensa = 90000; }
    else if (cantidad == 4) { format(rareza, tam, "DEL MONTON");    recompensa = 50000; }
    else                    { format(rareza, tam, "CHATARRA");      recompensa = 25000; }
    return 1;
}

// ---------------------------------------------------------------------
//  POOL DE VEHICULOS LIBRES -- lee g_Vehiculos en memoria (ya cargado
//  de la base de datos): es la misma fuente de verdad que usa el resto
//  del gamemode, no una segunda consulta ni una segunda base de datos.
// ---------------------------------------------------------------------
stock Federico_ContarModelo(modelo)
{
    new total = 0;
    for (new i = 0; i < MAX_VEHICULOS; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] != 0) continue;
        if (g_Vehiculos[i][veModelo] != modelo) continue;
        if (g_Vehiculos[i][veObjeto] == INVALID_VEHICLE_ID) continue; // retirado por Federico ahora mismo
        total++;
    }
    return total;
}

// Modelos que Federico nunca busca, pedidos a mano por Vlad. Primero
// 410 (Manana) y 481 (BMX) el 03/09/2026; el mismo dia, mas tarde,
// aviones/trenes/remolques/barcos y demas "no coches" (mismo listado
// que MODELOS_EXCLUIDOS_COCHES en server.js -- se quitaron de /coches
// a la vez). Terminando siempre en 0 (centinela).
new const g_FedericoModelosExcluidos[] = {
    509, 510, 490, 410, 481, // 510 (Mountain Bike) excluido el 07/09/2026, peticion expresa de Vlad -- fuera para siempre
    611, 610, 608, 607, 606, 601, 599, 598, 597, 596, 595, 594, 593, 592, 591, 590,
    584, 583, 582, 577, 574, 572, 571, 570, 569, 564, 563, 553, 548, 544, 538, 537, 532, 531,
    530, 528, 525, 524, 523, 520, 519, 513, 512, 511, 501, 497, 493, 488, 487, 486, 485, 484,
    476, 473, 472, 469, 465, 464, 460, 457, 454, 453, 452, 450, 449, 447, 446, 441, 435,
    433, 432, 430, 428, 427, 425, 420, 417, 416, 408, 407, 406,
    0
};

stock bool:Federico_ModeloExcluido(modelo)
{
    for (new i = 0; g_FedericoModelosExcluidos[i] != 0; i++)
    {
        if (g_FedericoModelosExcluidos[i] == modelo) return true;
    }
    return false;
}

stock Federico_ModelosElegibles(destino[], maxModelos)
{
    // Exclusion mutua con el Coche Secreto (05/09/2026, regla nueva de
    // Vlad): mientras un modelo sea el Secreto vigente, no puede
    // aparecer TAMBIEN en la ronda normal -- ni en el instante en que se
    // elige (eso ya lo hacia Federico_SecretoModelosElegibles al reves),
    // ni en NINGUNA rotacion normal posterior mientras el secreto siga
    // activo (hasta que se entregue o rote a los 24h). Se calcula una
    // vez aqui fuera del bucle, no dentro: el secreto no cambia a medio
    // recorrer el pool de vehiculos.
    new bool:haySecreto = Federico_SecretoActivo();
    new modeloSecreto = haySecreto ? Federico_SecretoModeloActual() : 0;

    new total = 0;
    for (new i = 0; i < MAX_VEHICULOS && total < maxModelos; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] != 0) continue;
        if (g_Vehiculos[i][veObjeto] == INVALID_VEHICLE_ID) continue;
        if (Federico_ModeloExcluido(g_Vehiculos[i][veModelo])) continue;
        if (haySecreto && g_Vehiculos[i][veModelo] == modeloSecreto) continue;

        new modelo = g_Vehiculos[i][veModelo];
        new yaEsta = false;
        for (new j = 0; j < total; j++)
        {
            if (destino[j] == modelo) { yaEsta = true; break; }
        }
        if (!yaEsta)
        {
            destino[total] = modelo;
            total++;
        }
    }
    return total;
}

// Slot BUSCADO (no entregado) cuyo modelo coincide, o -1.
stock Federico_SlotDeModelo(modelo)
{
    for (new s = 0; s < FEDERICO_TOTAL_SLOTS; s++)
    {
        if (g_FedericoSlot[s][feModelo] == 0) continue;
        if (g_FedericoSlot[s][feEntregado]) continue;
        if (g_FedericoSlot[s][feModelo] == modelo) return s;
    }
    return -1;
}

// ---------------------------------------------------------------------
//  RONDA NUEVA -- selecciona 3 MODELOS distintos al azar entre los
//  elegibles, congela rareza/recompensa, persiste y loguea.
// ---------------------------------------------------------------------
stock Federico_NuevaRonda()
{
    new modelos[300];
    new totalModelos = Federico_ModelosElegibles(modelos, sizeof modelos);

    new elegidos[FEDERICO_TOTAL_SLOTS];
    new totalElegidos = 0;

    if (totalModelos > 0)
    {
        new copia[300];
        for (new i = 0; i < totalModelos; i++) copia[i] = modelos[i];

        new restantes = totalModelos;
        while (totalElegidos < FEDERICO_TOTAL_SLOTS && restantes > 0)
        {
            new pick = random(restantes);
            elegidos[totalElegidos] = copia[pick];
            totalElegidos++;
            copia[pick] = copia[restantes - 1];
            restantes--;
        }
    }

    for (new s = 0; s < FEDERICO_TOTAL_SLOTS; s++)
    {
        new vacio[E_FEDERICO_SLOT];
        g_FedericoSlot[s] = vacio;

        if (s >= totalElegidos) continue;

        new modelo = elegidos[s];
        g_FedericoSlot[s][feModelo] = modelo;
        Veh_NombreModelo(modelo, g_FedericoSlot[s][feNombre], 32);

        new cantidad = Federico_ContarModelo(modelo);
        new recompensa;
        Federico_Rareza(cantidad, g_FedericoSlot[s][feRareza], 16, recompensa);
        g_FedericoSlot[s][feRecompensa] = recompensa;
        g_FedericoSlot[s][feEntregado]  = false;
        g_FedericoSlot[s][feVehiculoID] = 0;
    }

    g_FedericoStartedAt      = gettime();
    g_FedericoNextRotationAt = g_FedericoStartedAt + FEDERICO_DURACION_RONDA;
    g_FedericoListo          = true;

    // Se reinicia el destino marcado de todo el mundo: la ronda vieja
    // ya no existe, ningun checkpoint activo apunta a un slot valido.
    for (new p = 0; p < MAX_PLAYERS; p++) g_FedericoDestinoSlot[p] = -1;

    Federico_Persistir();
    Federico_ActualizarTodosLosSlots(); // por si alguien tiene /federico abierto justo al rotar

    new logMsg[300];
    format(logMsg, sizeof logMsg,
        "FEDERICO_ROUND_STARTED slots_ocupados=%d/%d modelos=[%d,%d,%d] rarezas=[%s,%s,%s] recompensas=[%d,%d,%d] started_at=%d next_rotation_at=%d",
        totalElegidos, FEDERICO_TOTAL_SLOTS,
        g_FedericoSlot[0][feModelo], g_FedericoSlot[1][feModelo], g_FedericoSlot[2][feModelo],
        g_FedericoSlot[0][feRareza], g_FedericoSlot[1][feRareza], g_FedericoSlot[2][feRareza],
        g_FedericoSlot[0][feRecompensa], g_FedericoSlot[1][feRecompensa], g_FedericoSlot[2][feRecompensa],
        g_FedericoStartedAt, g_FedericoNextRotationAt);
    EscribirLog("federico", logMsg);
    return 1;
}

// ---------------------------------------------------------------------
//  PERSISTENCIA
// ---------------------------------------------------------------------
stock Federico_Persistir()
{
    mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
        "REPLACE INTO federico_estado \
         (id, started_at, next_rotation_at, \
          slot1_modelo, slot1_nombre, slot1_rareza, slot1_recompensa, slot1_entregado, slot1_vehiculo_id, \
          slot2_modelo, slot2_nombre, slot2_rareza, slot2_recompensa, slot2_entregado, slot2_vehiculo_id, \
          slot3_modelo, slot3_nombre, slot3_rareza, slot3_recompensa, slot3_entregado, slot3_vehiculo_id) \
         VALUES (1, FROM_UNIXTIME(%d), FROM_UNIXTIME(%d), \
          %d, '%e', '%e', %d, %d, %d, \
          %d, '%e', '%e', %d, %d, %d, \
          %d, '%e', '%e', %d, %d, %d)",
        g_FedericoStartedAt, g_FedericoNextRotationAt,
        g_FedericoSlot[0][feModelo], g_FedericoSlot[0][feNombre], g_FedericoSlot[0][feRareza], g_FedericoSlot[0][feRecompensa], _:g_FedericoSlot[0][feEntregado], g_FedericoSlot[0][feVehiculoID],
        g_FedericoSlot[1][feModelo], g_FedericoSlot[1][feNombre], g_FedericoSlot[1][feRareza], g_FedericoSlot[1][feRecompensa], _:g_FedericoSlot[1][feEntregado], g_FedericoSlot[1][feVehiculoID],
        g_FedericoSlot[2][feModelo], g_FedericoSlot[2][feNombre], g_FedericoSlot[2][feRareza], g_FedericoSlot[2][feRecompensa], _:g_FedericoSlot[2][feEntregado], g_FedericoSlot[2][feVehiculoID]
    );
    mysql_pquery(g_BD, g_FedericoConsulta);
    return 1;
}

// Actualizacion ligera de un unico slot (al entregar), sin reescribir
// la fila entera.
stock Federico_PersistirEntrega(slot)
{
    new campoEntregado[24], campoVehId[24];
    format(campoEntregado, sizeof campoEntregado, "slot%d_entregado", slot + 1);
    format(campoVehId, sizeof campoVehId, "slot%d_vehiculo_id", slot + 1);

    mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
        "UPDATE federico_estado SET %s = 1, %s = %d WHERE id = 1",
        campoEntregado, campoVehId, g_FedericoSlot[slot][feVehiculoID]);
    mysql_pquery(g_BD, g_FedericoConsulta);
    return 1;
}

// ---------------------------------------------------------------------
//  COCHE SECRETO -- seleccion, ronda y persistencia. Mismo pool que los
//  3 normales (VEH_DROPEO, libre, no excluido) mas tres condiciones
//  extra: maximo 2 ejemplares libres (ampliado de "exactamente 1" el
//  10/09/2026, peticion expresa de Vlad -- ahora coincide con el umbral
//  de SUPER RARE tras el cambio de rareza del mismo dia), color
//  reconocible (g_ColoresConNombre) y que no sea ya uno de los 3
//  modelos de la ronda normal vigente (para que nunca coincidan).
// ---------------------------------------------------------------------
stock Federico_SecretoModelosElegibles(destino[], maxModelos)
{
    new total = 0;
    for (new i = 0; i < MAX_VEHICULOS && total < maxModelos; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] != 0) continue;
        if (g_Vehiculos[i][veObjeto] == INVALID_VEHICLE_ID) continue;

        new modelo = g_Vehiculos[i][veModelo];
        if (Federico_ModeloExcluido(modelo)) continue;
        if (modelo == g_FedericoSlot[0][feModelo]) continue;
        if (modelo == g_FedericoSlot[1][feModelo]) continue;
        if (modelo == g_FedericoSlot[2][feModelo]) continue;
        if (Federico_ContarModelo(modelo) > 2) continue;

        new colorNombre[16];
        if (!Federico_ColorConNombre(g_Vehiculos[i][veColor1], colorNombre, sizeof colorNombre)) continue;

        new yaEsta = false;
        for (new j = 0; j < total; j++) { if (destino[j] == modelo) { yaEsta = true; break; } }
        if (!yaEsta) { destino[total] = modelo; total++; }
    }
    return total;
}

// RONDA NUEVA -- cada FEDERICO_SECRETO_DURACION (24h). Si ningun modelo
// cumple las 4 condiciones a la vez, se deja vacio esta vuelta -- nunca
// se fuerza un candidato que no cumple, se reintenta en 24h.
stock Federico_SecretoNuevaRonda()
{
    new modelos[50];
    new total = Federico_SecretoModelosElegibles(modelos, sizeof modelos);

    g_FedSecretoEntregado  = false;
    g_FedSecretoVehiculoID = 0;

    if (total == 0)
    {
        g_FedSecretoModelo        = 0;
        g_FedSecretoColor1        = 0;
        g_FedSecretoColorNombre   = "";
        g_FedSecretoLetra[0]      = '-';
        g_FedSecretoLetra[1]      = '\0';
    }
    else
    {
        new elegido = modelos[random(total)];
        g_FedSecretoModelo = elegido;

        // Localizar el color1 real del ejemplar elegido -- hay
        // exactamente uno libre, por el filtro de cantidad==1 de arriba.
        for (new i = 0; i < MAX_VEHICULOS; i++)
        {
            if (!g_Vehiculos[i][veUsado]) continue;
            if (g_Vehiculos[i][veModelo] != elegido) continue;
            if (g_Vehiculos[i][veTipo] != VEH_DROPEO || g_Vehiculos[i][vePropietario] != 0) continue;
            g_FedSecretoColor1 = g_Vehiculos[i][veColor1];
            break;
        }
        Federico_ColorConNombre(g_FedSecretoColor1, g_FedSecretoColorNombre, sizeof g_FedSecretoColorNombre);

        new nombreModelo[32];
        Veh_NombreModelo(elegido, nombreModelo, 32);
        g_FedSecretoLetra[0] = nombreModelo[0];
        g_FedSecretoLetra[1] = '\0';
    }

    g_FedSecretoStartedAt      = gettime();
    g_FedSecretoNextRotationAt = g_FedSecretoStartedAt + FEDERICO_SECRETO_DURACION;
    g_FedSecretoListo          = true;

    Federico_SecretoPersistir();

    new logMsg[220];
    format(logMsg, sizeof logMsg,
        "FEDERICO_SECRETO_ROUND_STARTED modelo=%d color1=%d color_nombre=%s letra=%s started_at=%d next_rotation_at=%d",
        g_FedSecretoModelo, g_FedSecretoColor1, g_FedSecretoColorNombre, g_FedSecretoLetra,
        g_FedSecretoStartedAt, g_FedSecretoNextRotationAt);
    EscribirLog("federico", logMsg);
    return 1;
}

stock Federico_SecretoPersistir()
{
    mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
        "REPLACE INTO federico_secreto (id, modelo, color1, color_nombre, letra, started_at, next_rotation_at, entregado, vehiculo_id) \
         VALUES (1, %d, %d, '%e', '%e', FROM_UNIXTIME(%d), FROM_UNIXTIME(%d), %d, %d)",
        g_FedSecretoModelo, g_FedSecretoColor1, g_FedSecretoColorNombre, g_FedSecretoLetra,
        g_FedSecretoStartedAt, g_FedSecretoNextRotationAt, _:g_FedSecretoEntregado, g_FedSecretoVehiculoID);
    mysql_pquery(g_BD, g_FedericoConsulta);
    return 1;
}

// Igual que Federico_PersistirEntrega, version ligera para el Secreto.
stock Federico_SecretoPersistirEntrega()
{
    mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
        "UPDATE federico_secreto SET entregado = 1, vehiculo_id = %d WHERE id = 1",
        g_FedSecretoVehiculoID);
    mysql_pquery(g_BD, g_FedericoConsulta);
    return 1;
}

// bool: es este modelo el Coche Secreto vigente (y aun sin entregar).
stock bool:Federico_EsVehiculoSecreto(modelo)
{
    if (!g_FedSecretoListo) return false;
    if (g_FedSecretoModelo == 0) return false;
    if (g_FedSecretoEntregado) return false;
    return bool:(modelo == g_FedSecretoModelo);
}

// Accesores para modulos incluidos ANTES que este (31_web.inc, por el
// hack de Federico via Portatil Hacker) -- las variables globales de
// arriba no se ven hacia atras en Pawn, las funciones si.
stock bool:Federico_SecretoActivo()
{
    return bool:(g_FedSecretoListo && g_FedSecretoModelo != 0 && !g_FedSecretoEntregado);
}

stock Federico_SecretoModeloActual()
{
    return g_FedSecretoModelo;
}

// ---------------------------------------------------------------------
//  ARRANQUE DEL SECRETO -- se llama justo despues de Federico_Iniciar
//  (13_vehiculos.inc), mismo requisito: el pool de vehiculos ya cargado.
// ---------------------------------------------------------------------
stock Federico_SecretoIniciar()
{
    mysql_pquery(g_BD,
        "SELECT *, UNIX_TIMESTAMP(started_at) AS started_unix, UNIX_TIMESTAMP(next_rotation_at) AS next_unix FROM federico_secreto WHERE id = 1",
        "AlCargarFedericoSecreto");
    return 1;
}

forward AlCargarFedericoSecreto();
public AlCargarFedericoSecreto()
{
    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        // Primera vez que arranca este sistema: no hay ronda previa.
        Federico_SecretoNuevaRonda();
        return 1;
    }

    new nextUnix;
    cache_get_value_name_int(0, "next_unix", nextUnix);

    // Se carga tal cual, VENCIDA o no -- desde el 08/09/2026 solo
    // Federico_RondaGeneralSincronizar() (Federico_Segundo) decide cuando
    // rotar, para que las 4 categorias vayan siempre juntas.
    cache_get_value_name_int(0, "started_unix", g_FedSecretoStartedAt);
    g_FedSecretoNextRotationAt = nextUnix;
    cache_get_value_name_int(0, "modelo", g_FedSecretoModelo);
    cache_get_value_name_int(0, "color1", g_FedSecretoColor1);
    cache_get_value_name(0, "color_nombre", g_FedSecretoColorNombre, 16);
    cache_get_value_name(0, "letra", g_FedSecretoLetra, 2);

    new entregado;
    cache_get_value_name_int(0, "entregado", entregado);
    g_FedSecretoEntregado = bool:(entregado != 0);
    cache_get_value_name_int(0, "vehiculo_id", g_FedSecretoVehiculoID);

    g_FedSecretoListo = true;

    new logMsg[160];
    format(logMsg, sizeof logMsg, "FEDERICO_SECRETO_ROUND_RESTORED next_rotation_at=%d ahora=%d", nextUnix, gettime());
    EscribirLog("federico", logMsg);
    return 1;
}

// ---------------------------------------------------------------------
//  LA MOTO DEL DIA -- seleccion, ronda y persistencia. Mismo pool base
//  que el Secreto (VEH_DROPEO, libre, sin excluir), filtrado a la lista
//  de motos de verdad de g_FedericoMotos -- sin exigir cantidad exacta 1
//  ni color reconocible, esto es "una moto libre cualquiera", no un
//  acertijo.
// ---------------------------------------------------------------------
stock Federico_MotoModelosElegibles(destino[], maxModelos)
{
    new total = 0;
    for (new i = 0; i < MAX_VEHICULOS && total < maxModelos; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] != 0) continue;
        if (g_Vehiculos[i][veObjeto] == INVALID_VEHICLE_ID) continue;

        new modelo = g_Vehiculos[i][veModelo];
        new bool:esMoto = false;
        for (new m = 0; m < sizeof g_FedericoMotos; m++)
        {
            if (g_FedericoMotos[m] == modelo) { esMoto = true; break; }
        }
        if (!esMoto) continue;

        new yaEsta = false;
        for (new j = 0; j < total; j++) { if (destino[j] == modelo) { yaEsta = true; break; } }
        if (!yaEsta) { destino[total] = modelo; total++; }
    }
    return total;
}

// RONDA NUEVA -- cada FEDERICO_MOTO_DURACION (24h). Si no hay ninguna
// moto libre esta vuelta, se deja vacio -- se reintenta en 24h, igual
// que el Secreto.
stock Federico_MotoNuevaRonda()
{
    new modelos[sizeof g_FedericoMotos];
    new total = Federico_MotoModelosElegibles(modelos, sizeof modelos);

    g_FedMotoEntregado  = false;
    g_FedMotoVehiculoID = 0;
    g_FedMotoModelo     = (total > 0) ? modelos[random(total)] : 0;

    g_FedMotoStartedAt      = gettime();
    g_FedMotoNextRotationAt = g_FedMotoStartedAt + FEDERICO_MOTO_DURACION;
    g_FedMotoListo          = true;

    Federico_MotoPersistir();

    new logMsg[160];
    format(logMsg, sizeof logMsg, "FEDERICO_MOTO_ROUND_STARTED modelo=%d started_at=%d next_rotation_at=%d",
        g_FedMotoModelo, g_FedMotoStartedAt, g_FedMotoNextRotationAt);
    EscribirLog("federico", logMsg);
    return 1;
}

stock Federico_MotoPersistir()
{
    mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
        "REPLACE INTO federico_moto (id, modelo, started_at, next_rotation_at, entregado, vehiculo_id) \
         VALUES (1, %d, FROM_UNIXTIME(%d), FROM_UNIXTIME(%d), %d, %d)",
        g_FedMotoModelo, g_FedMotoStartedAt, g_FedMotoNextRotationAt, _:g_FedMotoEntregado, g_FedMotoVehiculoID);
    mysql_pquery(g_BD, g_FedericoConsulta);
    return 1;
}

// Igual que Federico_SecretoPersistirEntrega, version para la Moto.
stock Federico_MotoPersistirEntrega()
{
    mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
        "UPDATE federico_moto SET entregado = 1, vehiculo_id = %d WHERE id = 1",
        g_FedMotoVehiculoID);
    mysql_pquery(g_BD, g_FedericoConsulta);
    return 1;
}

// bool: es este modelo la Moto del Dia vigente (y aun sin entregar).
stock bool:Federico_EsVehiculoMoto(modelo)
{
    if (!g_FedMotoListo) return false;
    if (g_FedMotoModelo == 0) return false;
    if (g_FedMotoEntregado) return false;
    return bool:(modelo == g_FedMotoModelo);
}

// ---------------------------------------------------------------------
//  ARRANQUE DE LA MOTO DEL DIA -- mismo requisito que el Secreto: el
//  pool de vehiculos ya cargado.
// ---------------------------------------------------------------------
stock Federico_MotoIniciar()
{
    mysql_pquery(g_BD,
        "SELECT *, UNIX_TIMESTAMP(started_at) AS started_unix, UNIX_TIMESTAMP(next_rotation_at) AS next_unix FROM federico_moto WHERE id = 1",
        "AlCargarFedericoMoto");
    return 1;
}

forward AlCargarFedericoMoto();
public AlCargarFedericoMoto()
{
    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        Federico_MotoNuevaRonda();
        return 1;
    }

    new nextUnix;
    cache_get_value_name_int(0, "next_unix", nextUnix);

    // Se carga tal cual, VENCIDA o no -- ver el mismo comentario en
    // AlCargarFedericoSecreto (sincronizacion general, 08/09/2026).
    cache_get_value_name_int(0, "started_unix", g_FedMotoStartedAt);
    g_FedMotoNextRotationAt = nextUnix;
    cache_get_value_name_int(0, "modelo", g_FedMotoModelo);

    new entregado;
    cache_get_value_name_int(0, "entregado", entregado);
    g_FedMotoEntregado = bool:(entregado != 0);
    cache_get_value_name_int(0, "vehiculo_id", g_FedMotoVehiculoID);

    g_FedMotoListo = true;

    new logMsg[160];
    format(logMsg, sizeof logMsg, "FEDERICO_MOTO_ROUND_RESTORED next_rotation_at=%d ahora=%d", nextUnix, gettime());
    EscribirLog("federico", logMsg);
    return 1;
}

// ---------------------------------------------------------------------
//  MINI LISTA "CHATARRA" (05/09/2026, ampliada a 5 el 08/09/2026) --
//  FEDERICO_CHATARRA_TOTAL_SLOTS coches aparte de los 3 normales y el
//  Secreto, SOLO clase CHATARRA, que NO pagan dinero: pagan materiales,
//  marihuana o un arma. Se muestran en la web (/federico) y desde el
//  08/09/2026 tambien en /verlista ingame -- nunca en la pantalla
//  TextDraw de /federico, por eso no tienen TextDraw propio, solo entran
//  por el mismo /entregarcoche.
//  Rota cada FEDERICO_CHATARRA_DURACION (24h desde el 08/09/2026, antes
//  6h -- ahora IGUAL que el resto de Federico), temporizador propio,
//  independiente de la ronda normal y del Secreto. Recompensa FIJA por
//  posicion (slotChatarra % 3): 0 = materiales, 1 = marihuana,
//  2 = arma, y el ciclo se repite en 3 y 4.
// ---------------------------------------------------------------------
enum E_FEDERICO_CHATARRA
{
    fcModelo,
    fcNombre[32],
    bool:fcEntregado,
    fcVehiculoID
}
new g_FedChatarraSlot[FEDERICO_CHATARRA_TOTAL_SLOTS][E_FEDERICO_CHATARRA];

new g_FedChatarraStartedAt      = 0;
new g_FedChatarraNextRotationAt = 0;
new bool:g_FedChatarraListo     = false;

stock Federico_ChatarraModelosElegibles(destino[], maxModelos)
{
    new bool:haySecreto = Federico_SecretoActivo();
    new modeloSecreto = haySecreto ? Federico_SecretoModeloActual() : 0;

    new total = 0;
    for (new i = 0; i < MAX_VEHICULOS && total < maxModelos; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] != 0) continue;
        if (g_Vehiculos[i][veObjeto] == INVALID_VEHICLE_ID) continue;

        new modelo = g_Vehiculos[i][veModelo];
        if (Federico_ModeloExcluido(modelo)) continue;
        if (modelo == g_FedericoSlot[0][feModelo]) continue;
        if (modelo == g_FedericoSlot[1][feModelo]) continue;
        if (modelo == g_FedericoSlot[2][feModelo]) continue;
        if (haySecreto && modelo == modeloSecreto) continue;

        // Solo CHATARRA -- mismo umbral exacto que Federico_Rareza
        // (5 o mas copias libres del modelo, actualizado 10/09/2026).
        if (Federico_ContarModelo(modelo) <= 4) continue;

        new yaEsta = false;
        for (new j = 0; j < total; j++) { if (destino[j] == modelo) { yaEsta = true; break; } }
        if (!yaEsta) { destino[total] = modelo; total++; }
    }
    return total;
}

stock Federico_ChatarraNuevaRonda()
{
    new modelos[300];
    new totalModelos = Federico_ChatarraModelosElegibles(modelos, sizeof modelos);

    new elegidos[FEDERICO_CHATARRA_TOTAL_SLOTS];
    new totalElegidos = 0;

    if (totalModelos > 0)
    {
        new copia[300];
        for (new i = 0; i < totalModelos; i++) copia[i] = modelos[i];

        new restantes = totalModelos;
        while (totalElegidos < FEDERICO_CHATARRA_TOTAL_SLOTS && restantes > 0)
        {
            new pick = random(restantes);
            elegidos[totalElegidos] = copia[pick];
            totalElegidos++;
            copia[pick] = copia[restantes - 1];
            restantes--;
        }
    }

    new logModelos[120];
    for (new s = 0; s < FEDERICO_CHATARRA_TOTAL_SLOTS; s++)
    {
        new vacio[E_FEDERICO_CHATARRA];
        g_FedChatarraSlot[s] = vacio;

        if (s >= totalElegidos) continue;

        g_FedChatarraSlot[s][fcModelo] = elegidos[s];
        Veh_NombreModelo(elegidos[s], g_FedChatarraSlot[s][fcNombre], 32);

        new pedazo[16];
        format(pedazo, sizeof pedazo, "%s%d", (s > 0) ? (",") : (""), elegidos[s]);
        strcat(logModelos, pedazo, sizeof logModelos);
    }

    g_FedChatarraStartedAt      = gettime();
    g_FedChatarraNextRotationAt = g_FedChatarraStartedAt + FEDERICO_CHATARRA_DURACION;
    g_FedChatarraListo          = true;

    Federico_ChatarraPersistir();

    new logMsg[220];
    format(logMsg, sizeof logMsg,
        "FEDERICO_CHATARRA_ROUND_STARTED modelos=[%s] started_at=%d next_rotation_at=%d",
        logModelos, g_FedChatarraStartedAt, g_FedChatarraNextRotationAt);
    EscribirLog("federico", logMsg);
    return 1;
}

stock Federico_ChatarraPersistir()
{
    mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
        "REPLACE INTO federico_chatarra \
         (id, started_at, next_rotation_at, \
          slot1_modelo, slot1_nombre, slot1_entregado, slot1_vehiculo_id, \
          slot2_modelo, slot2_nombre, slot2_entregado, slot2_vehiculo_id, \
          slot3_modelo, slot3_nombre, slot3_entregado, slot3_vehiculo_id, \
          slot4_modelo, slot4_nombre, slot4_entregado, slot4_vehiculo_id, \
          slot5_modelo, slot5_nombre, slot5_entregado, slot5_vehiculo_id) \
         VALUES (1, FROM_UNIXTIME(%d), FROM_UNIXTIME(%d), \
          %d, '%e', %d, %d, %d, '%e', %d, %d, %d, '%e', %d, %d, \
          %d, '%e', %d, %d, %d, '%e', %d, %d)",
        g_FedChatarraStartedAt, g_FedChatarraNextRotationAt,
        g_FedChatarraSlot[0][fcModelo], g_FedChatarraSlot[0][fcNombre], _:g_FedChatarraSlot[0][fcEntregado], g_FedChatarraSlot[0][fcVehiculoID],
        g_FedChatarraSlot[1][fcModelo], g_FedChatarraSlot[1][fcNombre], _:g_FedChatarraSlot[1][fcEntregado], g_FedChatarraSlot[1][fcVehiculoID],
        g_FedChatarraSlot[2][fcModelo], g_FedChatarraSlot[2][fcNombre], _:g_FedChatarraSlot[2][fcEntregado], g_FedChatarraSlot[2][fcVehiculoID],
        g_FedChatarraSlot[3][fcModelo], g_FedChatarraSlot[3][fcNombre], _:g_FedChatarraSlot[3][fcEntregado], g_FedChatarraSlot[3][fcVehiculoID],
        g_FedChatarraSlot[4][fcModelo], g_FedChatarraSlot[4][fcNombre], _:g_FedChatarraSlot[4][fcEntregado], g_FedChatarraSlot[4][fcVehiculoID]
    );
    mysql_pquery(g_BD, g_FedericoConsulta);
    return 1;
}

stock Federico_ChatarraPersistirEntrega(slot)
{
    new campoEntregado[24], campoVehId[24];
    format(campoEntregado, sizeof campoEntregado, "slot%d_entregado", slot + 1);
    format(campoVehId, sizeof campoVehId, "slot%d_vehiculo_id", slot + 1);

    mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
        "UPDATE federico_chatarra SET %s = 1, %s = %d WHERE id = 1",
        campoEntregado, campoVehId, g_FedChatarraSlot[slot][fcVehiculoID]);
    mysql_pquery(g_BD, g_FedericoConsulta);
    return 1;
}

// Slot BUSCADO (no entregado) cuyo modelo coincide, o -1.
stock Federico_ChatarraSlotDeModelo(modelo)
{
    for (new s = 0; s < FEDERICO_CHATARRA_TOTAL_SLOTS; s++)
    {
        if (g_FedChatarraSlot[s][fcModelo] == 0) continue;
        if (g_FedChatarraSlot[s][fcEntregado]) continue;
        if (g_FedChatarraSlot[s][fcModelo] == modelo) return s;
    }
    return -1;
}

stock Federico_ChatarraIniciar()
{
    mysql_pquery(g_BD,
        "SELECT *, UNIX_TIMESTAMP(started_at) AS started_unix, UNIX_TIMESTAMP(next_rotation_at) AS next_unix FROM federico_chatarra WHERE id = 1",
        "AlCargarFedericoChatarra");
    return 1;
}

forward AlCargarFedericoChatarra();
public AlCargarFedericoChatarra()
{
    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        Federico_ChatarraNuevaRonda();
        return 1;
    }

    new nextUnix;
    cache_get_value_name_int(0, "next_unix", nextUnix);

    // Se carga tal cual, VENCIDA o no -- ver el mismo comentario en
    // AlCargarFedericoSecreto (sincronizacion general, 08/09/2026).
    cache_get_value_name_int(0, "started_unix", g_FedChatarraStartedAt);
    g_FedChatarraNextRotationAt = nextUnix;

    for (new s = 0; s < FEDERICO_CHATARRA_TOTAL_SLOTS; s++)
    {
        new campoModelo[20], campoNombre[20], campoEntregado[24], campoVehId[24];
        format(campoModelo, sizeof campoModelo, "slot%d_modelo", s + 1);
        format(campoNombre, sizeof campoNombre, "slot%d_nombre", s + 1);
        format(campoEntregado, sizeof campoEntregado, "slot%d_entregado", s + 1);
        format(campoVehId, sizeof campoVehId, "slot%d_vehiculo_id", s + 1);

        new modelo, entregado, vehId;
        cache_get_value_name_int(0, campoModelo, modelo);
        cache_get_value_name(0, campoNombre, g_FedChatarraSlot[s][fcNombre], 32);
        cache_get_value_name_int(0, campoEntregado, entregado);
        cache_get_value_name_int(0, campoVehId, vehId);

        g_FedChatarraSlot[s][fcModelo]     = modelo;
        g_FedChatarraSlot[s][fcEntregado]  = bool:(entregado != 0);
        g_FedChatarraSlot[s][fcVehiculoID] = vehId;
    }

    g_FedChatarraListo = true;

    new logMsg[160];
    format(logMsg, sizeof logMsg, "FEDERICO_CHATARRA_ROUND_RESTORED next_rotation_at=%d ahora=%d", nextUnix, gettime());
    EscribirLog("federico", logMsg);
    return 1;
}

// ---------------------------------------------------------------------
//  "COCHE DE PERSONAJES HISTORICOS" (05/09/2026, renombrado el mismo
//  dia desde "El Coche de la Droga" -- ahora son 4 posibles, uno a la
//  vez). Un Fundador elige con /crearhistorico uno de los 4 personajes
//  de un menu; se crea su vehiculo con el maletero cargado segun el
//  personaje elegido. Singleton igual que antes, persistido en
//  `federico_historico` (fila unica, id=1) con un estado explicito, no
//  inferido de si el vehiculo sigue vivo:
//    0 = ninguno (nunca creado, o admin lo borro por fuera del sistema)
//    1 = buscando (existe de verdad, con matricula y maletero cargado)
//    2 = entregado (el ultimo se entrego -- la web sigue mostrando su
//        nombre en rojo "ENTREGADO" hasta que se cree otro)
//  /crearhistorico solo esta bloqueado en estado 1 -- en 0 o 2 se
//  puede crear uno nuevo sin problema.
// ---------------------------------------------------------------------
#define FEDHIST_NINGUNO   0
#define FEDHIST_BUSCANDO  1
#define FEDHIST_ENTREGADO 2

enum E_HISTORICO
{
    hModelo,
    hPersonaje[24],
    hMaleteroTexto[80],  // descripcion tal cual para la web, ver server.js
    hDinero, hMateriales, hMarihuana, hPortatiles
}
new const g_Historicos[FEDERICO_HISTORICO_TOTAL][E_HISTORICO] = {
    {551, "Sergio Arias",  "1.000 de Marihuana",                                                       0,    0, 1000, 0},
    {586, "Dani Bequer",   "100.000$",                                                            100000,    0,    0, 0},
    {541, "JC",            "500 de Marihuana y 5.000 de Materiales",                                    0, 5000,  500, 0},
    {429, "Irene Reinosa", "1 Portatil Hacker, 50.000$, 100g de Marihuana y 1.000 de Materiales",    50000, 1000,  100, 1}
};

new g_FedHistPersonajeID = -1;
new g_FedHistVehiculoID  = 0;
new g_FedHistEstado      = FEDHIST_NINGUNO;

// ---------------------------------------------------------------------
//  PISTAS ("Dice la leyenda que...") -- encargo de Vlad del 06/09/2026.
//  Mitologia interna de Gamerol: mientras el historico este BUSCANDO, se
//  manda una pista global cada hora, en orden, y al llegar a la 5 vuelve
//  a la 1. Sin coordenadas ni marcadores -- solo texto, y a proposito no
//  explican quien es cada personaje de verdad.
//
//  No sobrevive a un reinicio del servidor a medio ciclo (se retoma
//  desde la pista 1 del personaje activo): mismo criterio que
//  g_FedericoNextRotationAt y compania mas abajo, que tampoco persisten
//  el segundo exacto restante, solo el hecho de que hay una ronda viva.
// ---------------------------------------------------------------------
#define FEDERICO_HIST_LEYENDA_TOTAL     5
#define FEDERICO_HIST_LEYENDA_INTERVALO 3600   // 1h real entre pistas

new const g_HistLeyendas[FEDERICO_HISTORICO_TOTAL][FEDERICO_HIST_LEYENDA_TOTAL][192] = {
    { // Sergio Arias -- Merit -- San Fierro
        "Dice la leyenda que... un viejo Merit ha vuelto a recorrer las calles despues de muchos anos.",
        "Dice la leyenda que... algunos veteranos recuerdan un Merit haciendo negocios por las calles de San Fierro.",
        "Dice la leyenda que... alguien ha visto un Merit subiendo las empinadas calles de San Fierro con el maletero demasiado cargado.",
        "Dice la leyenda que... el conductor de aquel Merit conocia cada rincon de San Fierro y sabia donde desaparecer sin dejar rastro.",
        "Dice la leyenda que... el Merit de Sergio Arias sigue escondido en algun lugar de San Fierro... y dicen que todavia lleva mercancia dentro."
    },
    { // Dani Bequer -- Wayfarer -- Los Santos
        "Dice la leyenda que... una vieja Wayfarer ha vuelto a escucharse por las calles de Los Santos.",
        "Dice la leyenda que... algunos todavia recuerdan aquella Wayfarer recorriendo Los Santos como si tuviera algun negocio pendiente.",
        "Dice la leyenda que... la NFS todavia recuerda una Wayfarer que aparecia cuando habia dinero de por medio.",
        "Dice la leyenda que... algunos negocios de Los Santos empezaban simplemente con el sonido de aquella Wayfarer acercandose.",
        "Dice la leyenda que... la Wayfarer de Dani Bequer sigue en algun lugar de Los Santos... y puede que todavia lleve algo interesante consigo."
    },
    { // JC -- Bullet -- campo / afueras de Los Santos
        "Dice la leyenda que... un viejo Bullet ha vuelto a rugir lejos del ruido de la ciudad.",
        "Dice la leyenda que... JC siempre prefirio llevar su Bullet por el campo y la naturaleza antes que perderse entre el trafico de Los Santos.",
        "Dice la leyenda que... alguien ha visto un Bullet desaparecer por una carretera secundaria a las afueras de Los Santos.",
        "Dice la leyenda que... el Bullet podria estar esperando donde terminan los edificios y empiezan los caminos y la naturaleza.",
        "Dice la leyenda que... el Bullet de JC sigue oculto en algun lugar del campo que rodea Los Santos... y su maletero no esta precisamente vacio."
    },
    { // Irene Reinosa -- Banshee -- Las Venturas
        "Dice la leyenda que... un Banshee ha vuelto a aparecer entre las luces de Las Venturas.",
        "Dice la leyenda que... Irene sabia que un Banshee, Las Venturas y una noche de apuestas podian ser una combinacion peligrosa.",
        "Dice la leyenda que... alguien ha visto un Banshee desaparecer entre las luces y los casinos de Las Venturas.",
        "Dice la leyenda que... hay apuestas que empiezan con unas fichas... y otras buscando un Banshee perdido por Las Venturas.",
        "Dice la leyenda que... el Banshee de Irene Reinosa sigue escondido en algun lugar de Las Venturas... y encontrarlo podria valer mucho mas que una buena noche en el casino."
    }
};

new g_FedHistLeyendaIndice     = 0;  // 0-4, la proxima pista a mandar
new g_FedHistLeyendaSiguienteAt = 0; // gettime() del proximo disparo, 0 = ciclo parado

// Arranca el ciclo desde la pista 1 -- llamar justo al dejar
// g_FedHistEstado en BUSCANDO (Federico_HistoricoDialogo, mas abajo).
stock Federico_HistLeyenda_Iniciar()
{
    g_FedHistLeyendaIndice = 0;
    Federico_HistLeyenda_Enviar();
    return 1;
}

// Manda la pista actual del personaje activo y programa la siguiente.
// Se usa tanto para la primera (Iniciar) como desde el tick cada hora.
stock Federico_HistLeyenda_Enviar()
{
    if (g_FedHistPersonajeID < 0 || g_FedHistPersonajeID >= FEDERICO_HISTORICO_TOTAL) return 0;

    MensajeGlobal(COLOR_BLANCO, g_HistLeyendas[g_FedHistPersonajeID][g_FedHistLeyendaIndice]);

    g_FedHistLeyendaIndice = (g_FedHistLeyendaIndice + 1) % FEDERICO_HIST_LEYENDA_TOTAL;
    g_FedHistLeyendaSiguienteAt = gettime() + FEDERICO_HIST_LEYENDA_INTERVALO;
    return 1;
}

// Indice en g_Vehiculos del coche historico vigente, o -1 si no hay
// ninguno vivo ahora mismo (nunca se creo, esta entregado, o el que
// habia se borro por fuera de este sistema).
stock Federico_HistoricoIndice()
{
    if (g_FedHistEstado != FEDHIST_BUSCANDO || g_FedHistVehiculoID == 0) return -1;
    for (new i = 0; i < MAX_VEHICULOS; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veID] == g_FedHistVehiculoID) return i;
    }
    return -1;
}

stock Federico_HistoricoPersistir()
{
    mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
        "UPDATE federico_historico SET personaje_id = %d, vehiculo_id = %d, estado = %d WHERE id = 1",
        g_FedHistPersonajeID, g_FedHistVehiculoID, g_FedHistEstado);
    mysql_pquery(g_BD, g_FedericoConsulta);
    return 1;
}

stock Federico_HistoricoIniciar()
{
    mysql_pquery(g_BD, "SELECT personaje_id, vehiculo_id, estado FROM federico_historico WHERE id = 1", "AlCargarFedericoHistorico");
    return 1;
}

forward AlCargarFedericoHistorico();
public AlCargarFedericoHistorico()
{
    new filas;
    cache_get_row_count(filas);

    if (filas > 0)
    {
        cache_get_value_name_int(0, "personaje_id", g_FedHistPersonajeID);
        cache_get_value_name_int(0, "vehiculo_id", g_FedHistVehiculoID);
        cache_get_value_name_int(0, "estado", g_FedHistEstado);
    }

    // Si decia "buscando" pero el vehiculo ya no existe de verdad
    // (borrado por fuera de este sistema, /borrarveh de un admin,
    // etc.), se limpia para que /crearhistorico vuelva a funcionar.
    if (g_FedHistEstado == FEDHIST_BUSCANDO && Federico_HistoricoIndice() == -1)
    {
        g_FedHistEstado = FEDHIST_NINGUNO;
        Federico_HistoricoPersistir();
    }
    return 1;
}

CMD:crearhistorico(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_FUNDADOR)) return 1;

    if (g_FedHistEstado == FEDHIST_BUSCANDO)
    {
        Error(playerid, "Ya existe un Coche de Personaje Historico en el mapa. Hay que entregarlo primero.");
        return 1;
    }

    new lista[256];
    for (new i = 0; i < FEDERICO_HISTORICO_TOTAL; i++)
    {
        new nombreModelo[32];
        Veh_NombreModelo(g_Historicos[i][hModelo], nombreModelo, 32);
        format(g_Buffer, sizeof g_Buffer, "%s -- %s\n", g_Historicos[i][hPersonaje], nombreModelo);
        strcat(lista, g_Buffer, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_FEDERICO_HISTORICO, DIALOG_STYLE_LIST,
        "{E8B33A}Coche de Personaje Historico", lista, "Elegir", "Cancelar");
    return 1;
}

stock Federico_HistoricoDialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext
    if (dialogid != DIALOGO_FEDERICO_HISTORICO) return 0;
    if (!response) return 1;
    if (listitem < 0 || listitem >= FEDERICO_HISTORICO_TOTAL) return 1;

    if (g_FedHistEstado == FEDHIST_BUSCANDO)
    {
        Error(playerid, "Ya existe un Coche de Personaje Historico en el mapa. Hay que entregarlo primero.");
        return 1;
    }

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    new modelo = g_Historicos[listitem][hModelo];
    new precio = Dropeo_PrecioParaLibre(modelo);
    new indice = Veh_Crear(modelo, 0, VEH_DROPEO, 0, 0, x, y + 3.0, z, a, -1, -1, 0, MUNDO_NORMAL, precio);
    if (indice == -1)
    {
        Error(playerid, "No hay hueco libre en la flota (MAX_VEHICULOS lleno).");
        return 1;
    }

    // Mismo patron que /creardropeo: nace abierto, motor apagado.
    g_Vehiculos[indice][veCerrado] = false;
    Dropeo_MarcarApagado(indice);
    Veh_ActualizarCierreTodos(indice);
    Veh_Guardar(indice);
    Dropeo_CrearCartel(indice);

    // OJO: el maletero NO se rellena aqui. g_Vehiculos[indice][veID]
    // sigue siendo 0 en este instante (el INSERT de Veh_Crear es
    // asincrono) y Maletero_Guardar exige veID > 0 para persistir --
    // llamarlo aqui lo dejaria en memoria pero SIN GUARDAR, y se
    // perderia entero en el primer reinicio (bug real, encontrado el
    // 05/09/2026 con el primer Coche de la Droga). Se rellena en
    // Federico_HistoricoCapturarID, cuando el veID ya es real.
    g_FedHistPersonajeID = listitem;
    g_FedHistVehiculoID  = 0; // se rellena de verdad en Federico_HistoricoCapturarID
    g_FedHistEstado      = FEDHIST_BUSCANDO;
    Federico_HistoricoPersistir(); // guarda ya el estado, el vehiculo_id llega un momento despues
    Federico_HistLeyenda_Iniciar(); // dispara la pista 1 ya mismo y arranca el ciclo de 1h

    SetTimerEx("Federico_HistoricoCapturarID", 500, false, "i", indice);

    format(g_Buffer, sizeof g_Buffer,
        "Coche de %s creado. Matricula %s, con %s en el maletero.",
        g_Historicos[listitem][hPersonaje], g_Vehiculos[indice][veMatricula], g_Historicos[listitem][hMaleteroTexto]);
    Exito(playerid, g_Buffer);

    EscribirLog("federico", "FEDERICO_HISTORICO_CREADO");
    return 1;
}

forward Federico_HistoricoCapturarID(indice);
public Federico_HistoricoCapturarID(indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (g_Vehiculos[indice][veID] == 0) return 0; // AlCrearVehiculo aun no ha llegado, se pierde -- caso raro
    if (g_FedHistPersonajeID < 0 || g_FedHistPersonajeID >= FEDERICO_HISTORICO_TOTAL) return 0;

    g_FedHistVehiculoID = g_Vehiculos[indice][veID];
    Federico_HistoricoPersistir();

    // Ahora si -- veID ya es real, Maletero_Guardar puede persistir.
    new personaje = g_FedHistPersonajeID;
    if (g_Historicos[personaje][hDinero]     > 0) Maletero_MoverDinero(indice,     g_Historicos[personaje][hDinero]);
    if (g_Historicos[personaje][hMateriales] > 0) Maletero_MoverMateriales(indice, g_Historicos[personaje][hMateriales]);
    if (g_Historicos[personaje][hMarihuana]  > 0) Maletero_MoverMarihuana(indice,  g_Historicos[personaje][hMarihuana]);
    if (g_Historicos[personaje][hPortatiles] > 0) Maletero_MoverPortatiles(indice, g_Historicos[personaje][hPortatiles]);
    return 1;
}

// ---------------------------------------------------------------------
//  ARRANQUE -- se llama desde AlCargarVehiculos (13_vehiculos.inc), una
//  vez el pool de vehiculos ya esta en memoria.
// ---------------------------------------------------------------------
stock Federico_Iniciar()
{
    mysql_pquery(g_BD,
        "SELECT *, UNIX_TIMESTAMP(started_at) AS started_unix, UNIX_TIMESTAMP(next_rotation_at) AS next_unix FROM federico_estado WHERE id = 1",
        "AlCargarFederico");
    return 1;
}

forward AlCargarFederico();
public AlCargarFederico()
{
    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        // Primera vez que arranca este sistema: no hay ronda previa.
        Federico_NuevaRonda();
        Federico_RestaurarRetirados();
        return 1;
    }

    new startedUnix, nextUnix;
    cache_get_value_name_int(0, "started_unix", startedUnix);
    cache_get_value_name_int(0, "next_unix", nextUnix);

    // Se carga tal cual esta persistido, VENCIDA o no -- desde el
    // 08/09/2026 (sincronizacion general, peticion expresa de Vlad) quien
    // decide cuando rotar es Federico_RondaGeneralSincronizar() (ver
    // Federico_Segundo), nunca esta funcion por su cuenta -- si no, las 4
    // categorias se desincronizan otra vez la primera vez que una tarde
    // mas o menos que las demas en vencer.
    g_FedericoStartedAt      = startedUnix;
    g_FedericoNextRotationAt = nextUnix;

    for (new s = 0; s < FEDERICO_TOTAL_SLOTS; s++)
    {
        new campoModelo[20], campoNombre[20], campoRareza[20], campoRecompensa[24], campoEntregado[24], campoVehId[24];
        format(campoModelo, sizeof campoModelo, "slot%d_modelo", s + 1);
        format(campoNombre, sizeof campoNombre, "slot%d_nombre", s + 1);
        format(campoRareza, sizeof campoRareza, "slot%d_rareza", s + 1);
        format(campoRecompensa, sizeof campoRecompensa, "slot%d_recompensa", s + 1);
        format(campoEntregado, sizeof campoEntregado, "slot%d_entregado", s + 1);
        format(campoVehId, sizeof campoVehId, "slot%d_vehiculo_id", s + 1);

        new modelo, entregado, vehId, recompensa;
        cache_get_value_name_int(0, campoModelo, modelo);
        cache_get_value_name(0, campoNombre, g_FedericoSlot[s][feNombre], 32);
        cache_get_value_name(0, campoRareza, g_FedericoSlot[s][feRareza], 16);
        cache_get_value_name_int(0, campoRecompensa, recompensa);
        cache_get_value_name_int(0, campoEntregado, entregado);
        cache_get_value_name_int(0, campoVehId, vehId);

        g_FedericoSlot[s][feModelo]     = modelo;
        g_FedericoSlot[s][feRecompensa] = recompensa;
        g_FedericoSlot[s][feEntregado]  = bool:(entregado != 0);
        g_FedericoSlot[s][feVehiculoID] = vehId;
    }

    g_FedericoListo = true;
    Federico_RestaurarRetirados();

    new logMsg[160];
    format(logMsg, sizeof logMsg, "FEDERICO_ROUND_RESTORED next_rotation_at=%d ahora=%d", nextUnix, gettime());
    EscribirLog("federico", logMsg);
    return 1;
}

// Recorre el pool y devuelve al mundo cualquier vehiculo cuyo plazo de
// 24h ya haya vencido -- se llama al arrancar (por si el servidor
// estuvo apagado cuando vencio) y cada segundo (Federico_Segundo).
stock Federico_RestaurarRetirados()
{
    new ahora = gettime();
    for (new i = 0; i < MAX_VEHICULOS; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veFedericoUntil] <= 0) continue;
        if (g_Vehiculos[i][veFedericoUntil] <= ahora) Federico_DevolverVehiculo(i);
    }
    return 1;
}

// Rota las 5 categorias JUNTAS, en el mismo tick -- llamadas en orden
// (normal -> Secreto -> Moto -> Chatarra -> Coches de Jugadores) para
// que cada exclusion mutua (Federico_SecretoModelosElegibles etc, todas
// ya excluyen a las demas por sus propias funciones) vea siempre el
// estado RECIEN elegido de las categorias anteriores, nunca una mezcla
// de viejo/nuevo. Como las duraciones son identicas
// (FEDERICO_DURACION_RONDA/SECRETO/MOTO/CHATARRA = 86400 =
// FEDJUGADORES no tiene duracion propia, se limita a existir dentro de
// esta misma rotacion), las 5 terminan alineadas al MISMO
// next_rotation_at sin necesidad de forzar ninguna variable compartida
// -- Coches de Jugadores no persiste su propio "next_rotation_at" en
// ningun sitio precisamente por esto, monta sobre el reloj de arriba.
// El Personaje Historico no entra aqui: sigue siendo 100% manual via
// /crearhistorico, tal como pidio el jefe.
stock Federico_RondaGeneralSincronizar()
{
    new ahora = gettime();

    Federico_NuevaRonda();
    Federico_SecretoNuevaRonda();
    Federico_MotoNuevaRonda();
    Federico_ChatarraNuevaRonda();
    FedJugadores_NuevaRonda();   // 59_federicojugadores.inc -- 100% sincrona (sin admin>0 que excluir desde el 08/09), misma "ahora" que las 4 de arriba

    // Fuerza el MISMO segundo exacto en las 4 -- por si el reloj cambio
    // de segundo entre una llamada y la siguiente (raro, cada
    // *_NuevaRonda usa su propio gettime() interno, pero es posible) se
    // vuelve a escribir el mismo "ahora" capturado al principio y se
    // repersiste, para que las 4 queden IDENTICAS sin depender de suerte.
    g_FedericoStartedAt        = ahora;
    g_FedericoNextRotationAt   = ahora + FEDERICO_DURACION_RONDA;
    g_FedSecretoStartedAt      = ahora;
    g_FedSecretoNextRotationAt = ahora + FEDERICO_SECRETO_DURACION;
    g_FedMotoStartedAt         = ahora;
    g_FedMotoNextRotationAt    = ahora + FEDERICO_MOTO_DURACION;
    g_FedChatarraStartedAt        = ahora;
    g_FedChatarraNextRotationAt   = ahora + FEDERICO_CHATARRA_DURACION;

    Federico_Persistir();
    Federico_SecretoPersistir();
    Federico_MotoPersistir();
    Federico_ChatarraPersistir();

    Teletipo_FedericoNuevaRonda(); // 62_teletipo.inc -- reinicia los avisos de hitos 2/4/6/8/10, solo observa
    return 1;
}

// ---------------------------------------------------------------------
//  TICK -- llamado cada segundo real desde TicSegundo (99_callbacks.inc)
// ---------------------------------------------------------------------
stock Federico_Segundo()
{
    if (!g_FedericoListo) return 0;

    if (!g_FedericoSyncListo)
    {
        // Las 4 categorias cargan con 4 consultas SQL independientes
        // (13_vehiculos.inc) -- se espera a que las 4 hayan terminado
        // antes de decidir si hace falta sincronizar, para no dejar a
        // ninguna fuera de la primera rotacion conjunta por llegar tarde.
        if (!g_FedSecretoListo || !g_FedMotoListo || !g_FedChatarraListo) return 0;
        g_FedericoSyncListo = true;

        if (gettime() >= g_FedericoNextRotationAt || gettime() >= g_FedSecretoNextRotationAt
            || gettime() >= g_FedMotoNextRotationAt || gettime() >= g_FedChatarraNextRotationAt)
        {
            Federico_RondaGeneralSincronizar();
        }
    }
    else if (gettime() >= g_FedericoNextRotationAt)
    {
        Federico_RondaGeneralSincronizar();
    }

    // Pistas del historico: solo mientras esta BUSCANDO. En cuanto se
    // entrega o se da de baja, g_FedHistEstado deja de ser BUSCANDO y
    // esta linea deja de disparar sola, sin tener que cancelar nada.
    if (g_FedHistEstado == FEDHIST_BUSCANDO && gettime() >= g_FedHistLeyendaSiguienteAt) Federico_HistLeyenda_Enviar();

    Federico_RestaurarRetirados();
    return 1;
}

// ---------------------------------------------------------------------
//  RETIRAR / DEVOLVER UN VEHICULO
// ---------------------------------------------------------------------
stock Federico_RetirarVehiculo(indice)
{
    if (!VehiculoValido(indice)) return 0;

    new hasta = gettime() + FEDERICO_COOLDOWN;

    if (g_Vehiculos[indice][veID] > 0)
    {
        mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
            "UPDATE vehiculos SET federico_until = FROM_UNIXTIME(%d) WHERE id = %d",
            hasta, g_Vehiculos[indice][veID]);
        mysql_pquery(g_BD, g_FedericoConsulta);
    }

    DestroyVehicle(g_Vehiculos[indice][veObjeto]);
    Dropeo_AlBorrarVehiculo(indice);  // limpia cartel/flags de sesion del sistema de dropeo, sin tocar la BD

    g_Vehiculos[indice][veObjeto]         = INVALID_VEHICLE_ID;
    g_Vehiculos[indice][veFedericoUntil]  = hasta;
    return 1;
}

stock Federico_DevolverVehiculo(indice)
{
    if (indice < 0 || indice >= MAX_VEHICULOS) return 0;
    if (!g_Vehiculos[indice][veUsado]) return 0;

    new modeloTexto[32];
    Veh_NombreModelo(g_Vehiculos[indice][veModelo], modeloTexto, 32);

    // Vuelve exactamente donde su fila dice que esta -- ultimo /park si
    // alguna vez lo tuvo (como jugador, antes de dropearlo), su posicion
    // original si no. Ver cabecera del archivo.
    Veh_Aparecer(indice);
    if (g_Vehiculos[indice][veTipo] == VEH_DROPEO) Dropeo_CrearCartel(indice);

    if (g_Vehiculos[indice][veID] > 0)
    {
        mysql_format(g_BD, g_FedericoConsulta, sizeof g_FedericoConsulta,
            "UPDATE vehiculos SET federico_until = NULL WHERE id = %d", g_Vehiculos[indice][veID]);
        mysql_pquery(g_BD, g_FedericoConsulta);
    }

    new logMsg[220];
    format(logMsg, sizeof logMsg,
        "FEDERICO_VEHICLE_RETURNED vehiculo_id=%d modelo=%s pos=%.2f,%.2f,%.2f timestamp=%d",
        g_Vehiculos[indice][veID], modeloTexto, g_Vehiculos[indice][veX], g_Vehiculos[indice][veY], g_Vehiculos[indice][veZ], gettime());
    EscribirLog("federico", logMsg);

    g_Vehiculos[indice][veFedericoUntil] = 0;
    return 1;
}

// ---------------------------------------------------------------------
//  AVISO AUDIOVISUAL "COCHE BUSCADO POR FEDERICO!" -- copia exacta del
//  estilo de /arreglarcoche (posicion, fuente, tamanos, colores, outline)
//  y de su mismo sonido (1133, "BUY_CAR_MOD"). Coherencia audiovisual,
//  08/09/2026.
// ---------------------------------------------------------------------
stock Federico_CrearTextdrawsBuscado(playerid)
{
    g_FedBuscadoTD[playerid][FBD_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 130.0, "COCHE BUSCADO POR FEDERICO!");
    PlayerTextDrawAlignment(playerid, g_FedBuscadoTD[playerid][FBD_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_FedBuscadoTD[playerid][FBD_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_FedBuscadoTD[playerid][FBD_TITULO], 0.48, 2.0);
    PlayerTextDrawColour(playerid, g_FedBuscadoTD[playerid][FBD_TITULO], 0x4FD1C5FF);
    PlayerTextDrawSetOutline(playerid, g_FedBuscadoTD[playerid][FBD_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_FedBuscadoTD[playerid][FBD_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_FedBuscadoTD[playerid][FBD_TITULO], true);

    g_FedBuscadoTD[playerid][FBD_SUBTITULO] = CreatePlayerTextDraw(playerid, 320.0, 158.0, "Marcado en mapa - /entregarcoche para cobrar");
    PlayerTextDrawAlignment(playerid, g_FedBuscadoTD[playerid][FBD_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_FedBuscadoTD[playerid][FBD_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_FedBuscadoTD[playerid][FBD_SUBTITULO], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_FedBuscadoTD[playerid][FBD_SUBTITULO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_FedBuscadoTD[playerid][FBD_SUBTITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_FedBuscadoTD[playerid][FBD_SUBTITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_FedBuscadoTD[playerid][FBD_SUBTITULO], true);
    return 1;
}

// Recibe el titulo (+ color)/subtitulo en vez de darlos por hechos --
// reutilizado desde el 12/09/2026 tambien para "coche libre"/"coche con
// propietario" (Veh_MostrarEtiquetaCategoria, mas abajo), mismo font/
// tamano/posicion/duracion, peticion expresa de Vlad ("mismo font mismo
// todo"). Por eso SIEMPRE fija el texto/color antes de mostrar: al
// compartir el mismo PlayerTextDraw entre varios llamantes, no se puede
// confiar en lo que quedara puesto de la ultima vez que se mostro.
//
// OJO: el subtitulo va SIEMPRE en un solo color (blanco, fijado al
// crear el textdraw). Los codigos de color embebidos tipo {RRGGBB} NO
// funcionan dentro de un PlayerTextDraw (eso es solo para chat/dialogos)
// -- probado en directo el 12/09/2026: salia literalmente "?53C168$" en
// pantalla. La fuente PRICEDOWN tampoco soporta ~r~/~g~/~w~ (ver nota
// identica en 57_roboexpress.inc). Si algun dia hace falta de verdad
// multicolor DENTRO de una linea, la unica tecnica que funciona en este
// server es la de 57_roboexpress.inc (RUD_PRECIO/RUD_COMANDO): dos
// PlayerTextDraw distintos, uno ALIGN_RIGHT y otro ALIGN_LEFT, cuadrados
// a mano en X -- necesita probarse en directo, no a ciegas.
stock Federico_MostrarAviso(playerid, const titulo[], colorTitulo, const subtitulo[])
{
    PlayerTextDrawSetString(playerid, g_FedBuscadoTD[playerid][FBD_TITULO], titulo);
    PlayerTextDrawColour(playerid, g_FedBuscadoTD[playerid][FBD_TITULO], colorTitulo);
    PlayerTextDrawSetString(playerid, g_FedBuscadoTD[playerid][FBD_SUBTITULO], subtitulo);
    PlayerTextDrawShow(playerid, g_FedBuscadoTD[playerid][FBD_TITULO]);
    PlayerTextDrawShow(playerid, g_FedBuscadoTD[playerid][FBD_SUBTITULO]);
    PlayerPlaySound(playerid, 1133, 0.0, 0.0, 0.0);   // identidad audiovisual: mismo sonido que /arreglarcoche
    SetTimerEx("Federico_OcultarAvisoBuscado", FEDERICO_BUSCADO_AVISO_MS, false, "d", playerid);
    return 1;
}

forward Federico_OcultarAvisoBuscado(playerid);
public Federico_OcultarAvisoBuscado(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    PlayerTextDrawHide(playerid, g_FedBuscadoTD[playerid][FBD_TITULO]);
    PlayerTextDrawHide(playerid, g_FedBuscadoTD[playerid][FBD_SUBTITULO]);
    return 1;
}

// ---------------------------------------------------------------------
//  DESCUBRIMIENTO -- llamado desde OnPlayerStateChange (99_callbacks.inc)
//  SOLO cuando newstate == PLAYER_STATE_DRIVER (nunca como pasajero).
// ---------------------------------------------------------------------
stock Federico_AlEntrarComoConductor(playerid, indice)
{
    if (!g_FedericoListo) return 0;
    if (!VehiculoValido(indice)) return 0;
    if (g_Vehiculos[indice][veTipo] != VEH_DROPEO) return 0;
    if (g_Vehiculos[indice][vePropietario] != 0) return 0;

    // Robo Express es el vehiculo EXACTO de una mision activa -- tiene su
    // propio aviso ("LO ENCONTRASTE!") y prioridad visual total. Nunca
    // mostrar tambien el aviso generico de Federico sobre el mismo coche
    // (coherencia audiovisual, 08/09/2026).
    if (RoboExpress_EsVehiculoObjetivo(indice)) return 0;

    new modelo = g_Vehiculos[indice][veModelo];
    new slot = Federico_SlotDeModelo(modelo);
    // La mini lista chatarra usa el mismo aviso/checkpoint -- se marca
    // con slot+100 en g_FedericoDestinoSlot para no confundirlo con un
    // slot normal (0/1/2) al comparar el antispam de abajo. Moto = 200,
    // Coche Secreto = 300, Personaje Historico = 400 (mismo truco,
    // namespaces propios -- antes NINGUNA de estas dos ultimas avisaba
    // nada al montarse, hueco cerrado el 08/09/2026).
    if (slot == -1)
    {
        if (Federico_EsVehiculoMoto(modelo))
        {
            slot = 200;
        }
        else if (Federico_EsVehiculoSecreto(modelo))
        {
            slot = 300;
        }
        else if (g_FedHistEstado == FEDHIST_BUSCANDO && g_FedHistVehiculoID != 0
            && g_Vehiculos[indice][veID] == g_FedHistVehiculoID)
        {
            slot = 400;
        }
        else
        {
            new slotChatarra = Federico_ChatarraSlotDeModelo(modelo);
            if (slotChatarra == -1) return 0;
            slot = slotChatarra + 100;
        }
    }

    new ahora = gettime();

    // Antispam: si ya se le aviso de ESTE mismo slot hace menos de
    // FEDERICO_AVISO_COOLDOWN segundos, no se repite el mensaje ni se
    // vuelve a fijar el checkpoint (evita spam al entrar/salir seguido).
    if (g_FedericoDestinoSlot[playerid] == slot
        && (ahora - g_FedericoUltimoAviso[playerid]) < FEDERICO_AVISO_COOLDOWN)
    {
        return 1;   // SI es un objetivo de Federico, solo que ya se aviso hace poco -- no es "coche libre" generico
    }

    SendClientMessage(playerid, COLOR_ROJO_FUERTE, "Este vehiculo lo busca Federico. Entregalo por una recompensa.");
    SetPlayerCheckpoint(playerid, FEDERICO_X, FEDERICO_Y, FEDERICO_Z, FEDERICO_RADIO_ENTREGA);
    Federico_MostrarAviso(playerid, "COCHE BUSCADO POR FEDERICO!", 0x4FD1C5FF,
        "Usa /Puente y entregalo con /Entregarcoche (/GPS)");

    g_FedericoUltimoAviso[playerid] = ahora;
    g_FedericoDestinoSlot[playerid] = slot;
    return 1;
}

// ---------------------------------------------------------------------
//  ETIQUETA GENERICA "COCHE LIBRE" / "COCHE CON PROPIETARIO" -- 12/09/2026,
//  peticion expresa de Vlad: mismo aviso audiovisual que el de Federico
//  (mismo font, tamano, color, posicion, duracion) para CUALQUIER
//  vehiculo de calle (VEH_DROPEO). Se llama SOLO si
//  Federico_AlEntrarComoConductor no reclamo ya el vehiculo (ver
//  OnPlayerStateChange, 99_callbacks.inc) -- por eso vuelve a excluir
//  Robo Express aqui: ese vehiculo tiene prioridad visual total y
//  Federico_AlEntrarComoConductor pudo declinar por ese mismo motivo.
// ---------------------------------------------------------------------
stock Veh_MostrarEtiquetaCategoria(playerid, indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (g_Vehiculos[indice][veTipo] != VEH_DROPEO) return 0;
    if (RoboExpress_EsVehiculoObjetivo(indice)) return 0;

    new propietario = g_Vehiculos[indice][vePropietario];
    if (propietario == Jugador[playerid][jID]) return 0;   // tu propio coche, nada que avisar

    if (propietario == 0)
    {
        Federico_MostrarAviso(playerid, "COCHE LIBRE", COLOR_VERDE, "Usa /Comprarcoche o /Puente");
    }
    else
    {
        Federico_MostrarAviso(playerid, "COCHE CON PROPIETARIO", COLOR_ROJO, "Usa /Puente y llevalo a la NFS");
    }
    return 1;
}

// ---------------------------------------------------------------------
//  MARCADOR -- una vez, desde OnGameModeInit (99_callbacks.inc), junto
//  al resto de _CrearMarcadores de los demas modulos (Trabajo, Faccion,
//  Banco...). Objeto plano (19329, el mismo "lienzo en blanco" que usa
//  la comunidad SA-MP para dibujar texto sobre un objeto) con el signo
//  "$" como textura de texto -- no depende de acertar un ID de modelo
//  de pickup concreto, el caracter se dibuja literal.
// ---------------------------------------------------------------------
// Dos puntos desde el 07/09/2026 -- mismo objeto/cartel, uno en cada sitio.
stock Federico_CrearMarcador()
{
    g_FedMarcadorDinero[0] = CreateDynamicObject(19329,
        FEDERICO_X, FEDERICO_Y, FEDERICO_Z + 1.2, 0.0, 0.0, 0.0, 0, 0, -1, 100.0);
    SetDynamicObjectMaterialText(g_FedMarcadorDinero[0], 0, "$",
        OBJECT_MATERIAL_SIZE_32x32, "Impact", 32, 1, 0xFF53C168, 0x00000000, 1);

    g_FedMarcadorEtiqueta[0] = CreateDynamic3DTextLabel(
        "FEDERICO\n{FFFFFF}Trae un coche buscado y escribe {E8B33A}/entregarcoche",
        COLOR_MARCA, FEDERICO_X, FEDERICO_Y, FEDERICO_Z + 0.7, 14.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    g_FedMarcadorDinero[1] = CreateDynamicObject(19329,
        FEDERICO_ANGELPINE_X, FEDERICO_ANGELPINE_Y, FEDERICO_ANGELPINE_Z + 1.2, 0.0, 0.0, 0.0, 0, 0, -1, 100.0);
    SetDynamicObjectMaterialText(g_FedMarcadorDinero[1], 0, "$",
        OBJECT_MATERIAL_SIZE_32x32, "Impact", 32, 1, 0xFF53C168, 0x00000000, 1);

    g_FedMarcadorEtiqueta[1] = CreateDynamic3DTextLabel(
        "FEDERICO (Angel Pine)\n{FFFFFF}Trae un coche buscado y escribe {E8B33A}/entregarcoche",
        COLOR_MARCA, FEDERICO_ANGELPINE_X, FEDERICO_ANGELPINE_Y, FEDERICO_ANGELPINE_Z + 0.7, 14.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    print("  [FEDERICO] Marcadores de los dos puntos de entrega preparados.");
    return 1;
}

// Iconos de minimapa quitados el 11/09/2026 -- peticion expresa de Vlad
// de limpiar todos los iconos del mapa (la gente usa /gps). Se mantiene
// el textdraw de "buscado", que no es un icono de mapa.
stock Federico_OnConectar(playerid)
{
    Federico_CrearTextdrawsBuscado(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  CREACION -- una vez, desde OnGameModeInit (99_callbacks.inc), antes
//  de Veh_CargarTodos (que dispara Federico_Iniciar al final).
// ---------------------------------------------------------------------
stock Federico_CrearTextdraws()
{
    // Fondo ceñido al contenido, con un margen pequeño (no toda la
    // pantalla) -- el ancho real de las 3 columnas va de x=85 a x=555,
    // el contenido en alto de y=85 a y=335; aqui con margen de ~10
    // unidades por lado. Tamano fijado con TextDrawTextSize (ancho,
    // alto EXPLICITOS -- no 0), igual que se hace ya con el boton
    // "Cerrar" (x2=370, alto=20): con alto=0 el motor decide el tamano
    // el solo y salia mucho mas grande de lo esperado (lo que paso
    // antes, fondo enorme para lo poco que hay dentro).
    g_FedGlobalTD[FTD_FONDO] = TextDrawCreate(75.0, 78.0, "_");
    TextDrawUseBox(g_FedGlobalTD[FTD_FONDO], true);
    TextDrawBoxColour(g_FedGlobalTD[FTD_FONDO], 0x000000C8);
    TextDrawTextSize(g_FedGlobalTD[FTD_FONDO], 565.0, 280.0);
    TextDrawLetterSize(g_FedGlobalTD[FTD_FONDO], 0.0, 1.0);
    TextDrawSetShadow(g_FedGlobalTD[FTD_FONDO], 0);
    TextDrawSetOutline(g_FedGlobalTD[FTD_FONDO], 0);

    g_FedGlobalTD[FTD_TITULO] = TextDrawCreate(320.0, 94.0, "LOS COCHES DE FEDERICO");
    TextDrawAlignment(g_FedGlobalTD[FTD_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_FedGlobalTD[FTD_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    TextDrawLetterSize(g_FedGlobalTD[FTD_TITULO], 0.55, 2.4);
    TextDrawColour(g_FedGlobalTD[FTD_TITULO], COLOR_VERDE);
    TextDrawSetOutline(g_FedGlobalTD[FTD_TITULO], 1);
    TextDrawSetShadow(g_FedGlobalTD[FTD_TITULO], 0);
    TextDrawSetProportional(g_FedGlobalTD[FTD_TITULO], true);

    // Mismo font que el titulo (FTD_TITULO, PRICEDOWN) -- pedido expreso
    // de Vlad el 05/09/2026: con AHARONI_BOLD a tamano pequeno la pista
    // del Coche Secreto "apenas se veia". Letra mas chica que el titulo
    // (la frase es mucho mas larga que "LOS COCHES DE FEDERICO") para
    // que quepa en el ancho del panel sin desbordar.
    g_FedGlobalTD[FTD_SUBTITULO] = TextDrawCreate(320.0, 116.0, "Entregalos todos para una recompensa especial!");
    TextDrawAlignment(g_FedGlobalTD[FTD_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_FedGlobalTD[FTD_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    TextDrawLetterSize(g_FedGlobalTD[FTD_SUBTITULO], 0.24, 1.15);
    TextDrawColour(g_FedGlobalTD[FTD_SUBTITULO], 0xE8B33AFF);
    TextDrawSetShadow(g_FedGlobalTD[FTD_SUBTITULO], 0);
    TextDrawSetOutline(g_FedGlobalTD[FTD_SUBTITULO], 1);
    TextDrawBackgroundColour(g_FedGlobalTD[FTD_SUBTITULO], 0x000000AA);
    TextDrawSetProportional(g_FedGlobalTD[FTD_SUBTITULO], true);

    // Pie y boton "Cerrar": comprobado in-game (03/09/2026) que el
    // cliente movil (SAMP Unity / samplauncher.com, ver /tactil) NO
    // pinta TextDraws de texto/boton mas alla de y~340 -- se quedan
    // invisibles aunque el fondo (un box normal) si llega mas abajo sin
    // problema. Por eso todo el contenido interactivo se quedo por
    // encima de esa linea; no hay boton "Cerrar" aparte, ESC cierra.
    g_FedGlobalTD[FTD_PIE] = TextDrawCreate(320.0, 300.0, "Nueva lista en --");
    TextDrawAlignment(g_FedGlobalTD[FTD_PIE], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_FedGlobalTD[FTD_PIE], TEXT_DRAW_FONT_AHARONI_BOLD);
    TextDrawLetterSize(g_FedGlobalTD[FTD_PIE], 0.20, 1.0);
    TextDrawColour(g_FedGlobalTD[FTD_PIE], 0xFFFFFFFF);
    TextDrawSetShadow(g_FedGlobalTD[FTD_PIE], 1);
    TextDrawSetOutline(g_FedGlobalTD[FTD_PIE], 0);
    TextDrawBackgroundColour(g_FedGlobalTD[FTD_PIE], 0x000000AA);
    TextDrawSetProportional(g_FedGlobalTD[FTD_PIE], true);

    g_FedGlobalTD[FTD_CERRAR_CAJA] = TextDrawCreate(270.0, 312.0, "_");
    TextDrawUseBox(g_FedGlobalTD[FTD_CERRAR_CAJA], true);
    TextDrawBoxColour(g_FedGlobalTD[FTD_CERRAR_CAJA], 0x8A2BE2FF);
    TextDrawTextSize(g_FedGlobalTD[FTD_CERRAR_CAJA], 370.0, 20.0);
    TextDrawLetterSize(g_FedGlobalTD[FTD_CERRAR_CAJA], 0.0, 1.6);
    TextDrawSetShadow(g_FedGlobalTD[FTD_CERRAR_CAJA], 0);
    TextDrawSetOutline(g_FedGlobalTD[FTD_CERRAR_CAJA], 0);
    TextDrawSetSelectable(g_FedGlobalTD[FTD_CERRAR_CAJA], true);

    g_FedGlobalTD[FTD_CERRAR_TXT] = TextDrawCreate(320.0, 316.0, "CERRAR");
    TextDrawAlignment(g_FedGlobalTD[FTD_CERRAR_TXT], TEXT_DRAW_ALIGN_CENTRE);
    TextDrawFont(g_FedGlobalTD[FTD_CERRAR_TXT], TEXT_DRAW_FONT_BANK_GOTHIC);
    TextDrawLetterSize(g_FedGlobalTD[FTD_CERRAR_TXT], 0.24, 1.1);
    TextDrawColour(g_FedGlobalTD[FTD_CERRAR_TXT], 0xFFFFFFFF);
    TextDrawSetOutline(g_FedGlobalTD[FTD_CERRAR_TXT], 0);
    TextDrawSetShadow(g_FedGlobalTD[FTD_CERRAR_TXT], 1);
    TextDrawBackgroundColour(g_FedGlobalTD[FTD_CERRAR_TXT], 0x000000AA);
    TextDrawSetProportional(g_FedGlobalTD[FTD_CERRAR_TXT], true);

    new cx[FEDERICO_TOTAL_SLOTS] = {160, 320, 480};
    for (new s = 0; s < FEDERICO_TOTAL_SLOTS; s++)
    {
        new Float:cxF = float(cx[s]);
        new Float:x1  = cxF - 75.0;

        // OJO: con la fuente TEXT_DRAW_FONT_MODEL_PREVIEW, TextDrawTextSize
        // deja de ser "esquina" y pasa a ser ANCHO/ALTO (documentado en
        // omp_textdraw.inc) -- usar aqui el mismo x2 absoluto que en el
        // resto del archivo (esquina) habria dado una caja casi el doble
        // de ancha de lo previsto. Aqui SI son literalmente 140x85.
        g_FedSlotTD[s][FSD_PREVIEW] = TextDrawCreate(x1, 132.0, "_");
        TextDrawUseBox(g_FedSlotTD[s][FSD_PREVIEW], true);
        TextDrawBoxColour(g_FedSlotTD[s][FSD_PREVIEW], 0xFFFFFFFF);
        // El color detras del modelo NO es BoxColour (eso se ignora en
        // modo preview) -- el propio manual de open.mp dice que es
        // BackgroundColour el que pinta el fondo del preview 3D. Blanco
        // a proposito: la mayoria de coches del pool son oscuros
        // (negro/gris) y sobre fondo negro no se distinguian nada.
        TextDrawBackgroundColour(g_FedSlotTD[s][FSD_PREVIEW], 0xFFFFFFFF);
        TextDrawFont(g_FedSlotTD[s][FSD_PREVIEW], TEXT_DRAW_FONT_MODEL_PREVIEW);
        TextDrawTextSize(g_FedSlotTD[s][FSD_PREVIEW], 140.0, 85.0);
        TextDrawSetShadow(g_FedSlotTD[s][FSD_PREVIEW], 0);
        TextDrawSetOutline(g_FedSlotTD[s][FSD_PREVIEW], 0);
        TextDrawSetPreviewModel(g_FedSlotTD[s][FSD_PREVIEW], 411); // Infernus de relleno hasta la 1a ronda real
        TextDrawSetPreviewRot(g_FedSlotTD[s][FSD_PREVIEW], -10.0, 0.0, -30.0, 1.0);
        TextDrawSetPreviewVehicleColours(g_FedSlotTD[s][FSD_PREVIEW], -1, -1);

        g_FedSlotTD[s][FSD_NOMBRE] = TextDrawCreate(cxF, 222.0, "-");
        TextDrawAlignment(g_FedSlotTD[s][FSD_NOMBRE], TEXT_DRAW_ALIGN_CENTRE);
        TextDrawFont(g_FedSlotTD[s][FSD_NOMBRE], TEXT_DRAW_FONT_BANK_GOTHIC);
        TextDrawLetterSize(g_FedSlotTD[s][FSD_NOMBRE], 0.26, 1.2);
        TextDrawColour(g_FedSlotTD[s][FSD_NOMBRE], 0xFFFFFFFF);
        TextDrawSetShadow(g_FedSlotTD[s][FSD_NOMBRE], 1);
        TextDrawSetOutline(g_FedSlotTD[s][FSD_NOMBRE], 0);
        TextDrawBackgroundColour(g_FedSlotTD[s][FSD_NOMBRE], 0x000000AA);
        TextDrawSetProportional(g_FedSlotTD[s][FSD_NOMBRE], true);

        g_FedSlotTD[s][FSD_RAREZA] = TextDrawCreate(cxF, 242.0, "-");
        TextDrawAlignment(g_FedSlotTD[s][FSD_RAREZA], TEXT_DRAW_ALIGN_CENTRE);
        TextDrawFont(g_FedSlotTD[s][FSD_RAREZA], TEXT_DRAW_FONT_AHARONI_BOLD);
        TextDrawLetterSize(g_FedSlotTD[s][FSD_RAREZA], 0.15, 0.8);
        TextDrawColour(g_FedSlotTD[s][FSD_RAREZA], COLOR_GRIS);
        TextDrawSetShadow(g_FedSlotTD[s][FSD_RAREZA], 1);
        TextDrawSetOutline(g_FedSlotTD[s][FSD_RAREZA], 0);
        TextDrawBackgroundColour(g_FedSlotTD[s][FSD_RAREZA], 0x000000AA);
        TextDrawSetProportional(g_FedSlotTD[s][FSD_RAREZA], true);

        g_FedSlotTD[s][FSD_PRECIO] = TextDrawCreate(cxF, 258.0, "-");
        TextDrawAlignment(g_FedSlotTD[s][FSD_PRECIO], TEXT_DRAW_ALIGN_CENTRE);
        TextDrawFont(g_FedSlotTD[s][FSD_PRECIO], TEXT_DRAW_FONT_BANK_GOTHIC);
        TextDrawLetterSize(g_FedSlotTD[s][FSD_PRECIO], 0.28, 1.35);
        TextDrawColour(g_FedSlotTD[s][FSD_PRECIO], COLOR_VERDE);
        TextDrawSetShadow(g_FedSlotTD[s][FSD_PRECIO], 1);
        TextDrawSetOutline(g_FedSlotTD[s][FSD_PRECIO], 0);
        TextDrawBackgroundColour(g_FedSlotTD[s][FSD_PRECIO], 0x000000AA);
        TextDrawSetProportional(g_FedSlotTD[s][FSD_PRECIO], true);

        g_FedSlotTD[s][FSD_ESTADO] = TextDrawCreate(cxF, 278.0, "SE BUSCA");
        TextDrawAlignment(g_FedSlotTD[s][FSD_ESTADO], TEXT_DRAW_ALIGN_CENTRE);
        TextDrawFont(g_FedSlotTD[s][FSD_ESTADO], TEXT_DRAW_FONT_BANK_GOTHIC);
        TextDrawLetterSize(g_FedSlotTD[s][FSD_ESTADO], 0.19, 0.95);
        TextDrawColour(g_FedSlotTD[s][FSD_ESTADO], COLOR_VERDE);
        TextDrawSetShadow(g_FedSlotTD[s][FSD_ESTADO], 1);
        TextDrawSetOutline(g_FedSlotTD[s][FSD_ESTADO], 0);
        TextDrawBackgroundColour(g_FedSlotTD[s][FSD_ESTADO], 0x000000AA);
        TextDrawSetProportional(g_FedSlotTD[s][FSD_ESTADO], true);

        // Tachon: barra roja recta sobre precio+estado cuando el modelo
        // ya se entrego esta ronda. Sin rotacion -- un TextDraw normal
        // (no el preview 3D) no se puede rotar en SA-MP/open.mp.
        g_FedSlotTD[s][FSD_TACHON] = TextDrawCreate(cxF - 72.0, 251.0, "_");
        TextDrawUseBox(g_FedSlotTD[s][FSD_TACHON], true);
        TextDrawBoxColour(g_FedSlotTD[s][FSD_TACHON], COLOR_ROJO_FUERTE);
        TextDrawTextSize(g_FedSlotTD[s][FSD_TACHON], cxF + 72.0, 0.0);
        TextDrawLetterSize(g_FedSlotTD[s][FSD_TACHON], 0.0, 3.0);
        TextDrawSetShadow(g_FedSlotTD[s][FSD_TACHON], 0);
        TextDrawSetOutline(g_FedSlotTD[s][FSD_TACHON], 0);
    }
    return 1;
}

// Refresca el contenido (texto + preview) de un slot para TODOS los que
// lo tengan abierto ahora mismo -- TextDrawSetString/SetPreviewModel
// sobre un TextDraw ya mostrado se ve en vivo, sin re-Show.
stock Federico_ActualizarSlotTextdraw(s)
{
    if (g_FedericoSlot[s][feModelo] == 0)
    {
        TextDrawSetString(g_FedSlotTD[s][FSD_NOMBRE], "---");
        TextDrawSetString(g_FedSlotTD[s][FSD_RAREZA], "");
        TextDrawSetString(g_FedSlotTD[s][FSD_PRECIO], "");
        TextDrawSetString(g_FedSlotTD[s][FSD_ESTADO], "SIN ASIGNAR");
        return 0;
    }

    TextDrawSetPreviewModel(g_FedSlotTD[s][FSD_PREVIEW], g_FedericoSlot[s][feModelo]);
    TextDrawSetString(g_FedSlotTD[s][FSD_NOMBRE], g_FedericoSlot[s][feNombre]);
    TextDrawSetString(g_FedSlotTD[s][FSD_RAREZA], g_FedericoSlot[s][feRareza]);

    new precioTexto[24];
    FormatearDinero(g_FedericoSlot[s][feRecompensa], precioTexto, sizeof precioTexto);
    TextDrawSetString(g_FedSlotTD[s][FSD_PRECIO], precioTexto);

    if (g_FedericoSlot[s][feEntregado])
    {
        TextDrawSetString(g_FedSlotTD[s][FSD_ESTADO], "ENTREGADO");
        TextDrawColour(g_FedSlotTD[s][FSD_ESTADO], COLOR_GRIS);
    }
    else
    {
        TextDrawSetString(g_FedSlotTD[s][FSD_ESTADO], "SE BUSCA");
        TextDrawColour(g_FedSlotTD[s][FSD_ESTADO], COLOR_VERDE);
    }
    return 1;
}

stock Federico_ActualizarTodosLosSlots()
{
    for (new s = 0; s < FEDERICO_TOTAL_SLOTS; s++) Federico_ActualizarSlotTextdraw(s);
    return 1;
}

// ---------------------------------------------------------------------
//  MOSTRAR / OCULTAR -- pantalla completa por jugador.
// ---------------------------------------------------------------------
stock Federico_MostrarUI(playerid)
{
    if (!g_FedericoListo)
    {
        Error(playerid, "Federico esta preparando su lista. Prueba en un momento.");
        return 0;
    }

    Federico_ActualizarTodosLosSlots();

    new restante = g_FedericoNextRotationAt - gettime();
    if (restante < 0) restante = 0;
    new horas = restante / 3600, minutos = (restante % 3600) / 60;
    new pie[48];
    format(pie, sizeof pie, "Nueva lista en %dh %02dm", horas, minutos);
    TextDrawSetString(g_FedGlobalTD[FTD_PIE], pie);

    // Coche Secreto: sustituye el subtitulo fijo por la pista mientras
    // haya un candidato activo y sin entregar -- nunca el modelo real.
    new subtitulo[96];
    if (g_FedSecretoListo && g_FedSecretoModelo != 0 && !g_FedSecretoEntregado)
    {
        format(subtitulo, sizeof subtitulo, "Coche Secreto - Empieza por la letra (%s) - Color (%s) - 300.000$",
            g_FedSecretoLetra, g_FedSecretoColorNombre);
    }
    else
    {
        format(subtitulo, sizeof subtitulo, "Entregalos todos para una recompensa especial!");
    }
    TextDrawSetString(g_FedGlobalTD[FTD_SUBTITULO], subtitulo);

    for (new i = 0; i < FTD_TOTAL; i++) TextDrawShowForPlayer(playerid, g_FedGlobalTD[i]);

    for (new s = 0; s < FEDERICO_TOTAL_SLOTS; s++)
    {
        TextDrawShowForPlayer(playerid, g_FedSlotTD[s][FSD_PREVIEW]);
        TextDrawShowForPlayer(playerid, g_FedSlotTD[s][FSD_NOMBRE]);
        TextDrawShowForPlayer(playerid, g_FedSlotTD[s][FSD_RAREZA]);
        TextDrawShowForPlayer(playerid, g_FedSlotTD[s][FSD_PRECIO]);
        TextDrawShowForPlayer(playerid, g_FedSlotTD[s][FSD_ESTADO]);

        if (g_FedericoSlot[s][feEntregado]) TextDrawShowForPlayer(playerid, g_FedSlotTD[s][FSD_TACHON]);
        else                                TextDrawHideForPlayer(playerid, g_FedSlotTD[s][FSD_TACHON]);
    }

    GPS_OcultarTextdrawFijo(playerid); // la fila de "coches/casas/negocios libres" estorbaria debajo

    g_FedericoUIAbierta[playerid] = true;
    SelectTextDraw(playerid, 0x53C168C8);
    return 1;
}

stock Federico_OcultarUI(playerid)
{
    if (!g_FedericoUIAbierta[playerid]) return 0;

    for (new i = 0; i < FTD_TOTAL; i++) TextDrawHideForPlayer(playerid, g_FedGlobalTD[i]);

    for (new s = 0; s < FEDERICO_TOTAL_SLOTS; s++)
    {
        TextDrawHideForPlayer(playerid, g_FedSlotTD[s][FSD_PREVIEW]);
        TextDrawHideForPlayer(playerid, g_FedSlotTD[s][FSD_NOMBRE]);
        TextDrawHideForPlayer(playerid, g_FedSlotTD[s][FSD_RAREZA]);
        TextDrawHideForPlayer(playerid, g_FedSlotTD[s][FSD_PRECIO]);
        TextDrawHideForPlayer(playerid, g_FedSlotTD[s][FSD_ESTADO]);
        TextDrawHideForPlayer(playerid, g_FedSlotTD[s][FSD_TACHON]);
    }

    GPS_MostrarTextdrawFijo(playerid); // se repone la fila oculta al abrir

    g_FedericoUIAbierta[playerid] = false;
    CancelSelectTextDraw(playerid);
    return 1;
}

// Llamado desde OnPlayerClickTextDraw (99_callbacks.inc), ANTES que
// Login_ClickTextDraw -- si el jugador no tiene la UI de Federico
// abierta, no hace nada y deja pasar el clic a los demas modulos.
stock Federico_ClickTextDraw(playerid, Text:clickedid)
{
    #pragma unused clickedid
    if (!g_FedericoUIAbierta[playerid]) return 0;

    // ESC o clic en "Cerrar": las dos cosas cierran la pantalla. No hay
    // ningun otro elemento clicable dentro de la UI, asi que cualquier
    // otro id que llegue aqui tambien se traga sin hacer nada.
    Federico_OcultarUI(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  /FEDERICO
// ---------------------------------------------------------------------
CMD:federico(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (g_FedericoUIAbierta[playerid])
    {
        Federico_OcultarUI(playerid);
        return 1;
    }

    Federico_MostrarUI(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  /VERLISTA (08/09/2026) -- lista completa ingame, estilo /gps: un
//  Dialog en vez del TextDraw de /federico (que no puede mostrar todo a
//  la vez por el limite de Y~340 del cliente movil). Muestra TODAS las
//  categorias de Federico, entregadas o no -- el Coche Secreto NUNCA
//  revela su modelo real aqui, solo letra/color (mismo criterio que la
//  web y que los Consejos: la pista es publica, la respuesta no).
//  Colores pedidos por Vlad (Chatarra marron, Personaje Historico oro,
//  Moto del Dia azul) + dos inventados para las categorias sin color
//  asignado (ronda normal verde -- paga dinero puro; Coche Secreto
//  morado -- misterio).
// ---------------------------------------------------------------------
CMD:verlista(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    Federico_MostrarListaCompleta(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  /CREARLISTA (10/09/2026) -- fuerza una regeneracion manual de TODA la
//  lista de Federico con un contador nuevo, para anuncios/eventos.
//  Reutiliza Federico_RondaGeneralSincronizar() tal cual (la misma
//  funcion que ya rota las 5 categorias juntas cada 24h de forma
//  automatica) -- no se duplica logica, solo se dispara a mano. El
//  Personaje Historico NO entra aqui (sigue siendo 100% manual via
//  /crearhistorico, esa categoria no tiene rotacion por tiempo).
// ---------------------------------------------------------------------
CMD:crearlista(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_FUNDADOR)) return 1;

    Federico_RondaGeneralSincronizar();

    Exito(playerid, "Lista de Federico regenerada con un contador nuevo.");
    EscribirLog("federico", "FEDERICO_LISTA_FORZADA_MANUAL");
    return 1;
}

stock Federico_MostrarListaCompleta(playerid)
{
    // El buffer se quedo en 2200 (ampliado el 08/09/2026 al anadir Coches
    // de Jugadores) aunque esa categoria se quito de esta lista el
    // 10/09/2026 (peticion expresa de Vlad, igual que en la web y en la
    // imagen del post de open.mp: /forzarcerradura y la rotacion de
    // Coches de Jugadores SIGUEN funcionando en 59_federicojugadores.inc,
    // solo se dejan de listar aqui) -- no hace falta encoger el limite,
    // sobra margen de sobra sin esas filas.
    //
    // Desde el 08/09/2026 (peticion expresa de Vlad), los ya ENTREGADOS
    // van todos al final de la lista, sin importar la categoria: cada
    // fila se acumula en "pendientes" o "entregados" segun su propio
    // estado, y al final se concatenan pendientes primero. El total de
    // texto no cambia (son las mismas filas de siempre, solo reordenadas),
    // asi que el limite de 2200 sigue valiendo para el buffer final.
    new pendientes[2200], entregados[2200];
    pendientes[0] = EOS;
    entregados[0] = EOS;

    // Ronda normal (3 slots) -- verde.
    for (new s = 0; s < FEDERICO_TOTAL_SLOTS; s++)
    {
        if (!g_FedericoListo || g_FedericoSlot[s][feModelo] == 0) continue;
        new bool:entregado = g_FedericoSlot[s][feEntregado];
        new fila[144];
        format(fila, sizeof fila, "{53C168}%s (%s){FFFFFF} - $%d - %s\n",
            g_FedericoSlot[s][feNombre], g_FedericoSlot[s][feRareza], g_FedericoSlot[s][feRecompensa],
            entregado ? ("{FF0000}ENTREGADO{FFFFFF}") : ("BUSCADO"));
        strcat(entregado ? entregados : pendientes, fila, entregado ? sizeof entregados : sizeof pendientes);
    }

    // Coche Secreto -- morado, nunca revela el modelo real.
    if (g_FedSecretoListo && g_FedSecretoModelo != 0)
    {
        new fila[160];
        format(fila, sizeof fila, "{C2A2DA}Coche Secreto (letra %s, color %s){FFFFFF} - %s\n",
            g_FedSecretoLetra, g_FedSecretoColorNombre,
            g_FedSecretoEntregado ? ("{FF0000}ENTREGADO{FFFFFF}") : ("BUSCANDO"));
        strcat(g_FedSecretoEntregado ? entregados : pendientes, fila, g_FedSecretoEntregado ? sizeof entregados : sizeof pendientes);
    }

    // Moto del Dia -- azul.
    if (g_FedMotoListo && g_FedMotoModelo != 0)
    {
        new nombreMoto[32];
        Veh_NombreModelo(g_FedMotoModelo, nombreMoto, sizeof nombreMoto);
        new fila[128];
        format(fila, sizeof fila, "{5A9BD5}%s (Moto del Dia){FFFFFF} - %s\n",
            nombreMoto, g_FedMotoEntregado ? ("{FF0000}ENTREGADO{FFFFFF}") : ("BUSCADA"));
        strcat(g_FedMotoEntregado ? entregados : pendientes, fila, g_FedMotoEntregado ? sizeof entregados : sizeof pendientes);
    }

    // Chatarra (5 slots) -- marron.
    for (new s = 0; s < FEDERICO_CHATARRA_TOTAL_SLOTS; s++)
    {
        if (!g_FedChatarraListo || g_FedChatarraSlot[s][fcModelo] == 0) continue;
        new bool:entregado = g_FedChatarraSlot[s][fcEntregado];
        new fila[128];
        format(fila, sizeof fila, "%s%s (Chatarra){FFFFFF} - %s\n",
            HEX_MARRON, g_FedChatarraSlot[s][fcNombre],
            entregado ? ("{FF0000}ENTREGADO{FFFFFF}") : ("BUSCADO"));
        strcat(entregado ? entregados : pendientes, fila, entregado ? sizeof entregados : sizeof pendientes);
    }

    // Personaje Historico -- oro. Si esta entregado o nunca se creo, no
    // rota solo: hace falta un admin con /crearhistorico.
    new bool:histEntregado = (g_FedHistEstado == FEDHIST_ENTREGADO);
    new filaHist[160];
    if (g_FedHistEstado == FEDHIST_NINGUNO || g_FedHistPersonajeID < 0)
    {
        format(filaHist, sizeof filaHist, "{E8B33A}Personaje Historico{FFFFFF} - Sin crear (a la espera de un admin)\n");
    }
    else
    {
        format(filaHist, sizeof filaHist, "{E8B33A}Personaje Historico: %s{FFFFFF} - %s\n",
            g_Historicos[g_FedHistPersonajeID][hPersonaje],
            histEntregado ? ("{FF0000}ENTREGADO (a la espera de otro){FFFFFF}") : ("BUSCANDO"));
    }
    strcat(histEntregado ? entregados : pendientes, filaHist, histEntregado ? sizeof entregados : sizeof pendientes);

    new lista[2200];
    lista[0] = EOS;
    strcat(lista, pendientes, sizeof lista);
    strcat(lista, entregados, sizeof lista);

    if (lista[0] == EOS)
    {
        format(lista, sizeof lista, "Federico no tiene ningun coche activo ahora mismo.");
    }

    ShowPlayerDialog(playerid, DIALOGO_FEDERICO_VERLISTA, DIALOG_STYLE_LIST,
        "{E8B33A}Federico -- Lista completa", lista, "Cerrar", "");
    return 1;
}

// ---------------------------------------------------------------------
//  /ENTREGARCOCHE
// ---------------------------------------------------------------------
CMD:entregarcoche(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del vehiculo para entregarselo a Federico.");
        return 1;
    }

    // Dos puntos de entrega desde el 07/09/2026 (LS y Angel Pine) -- la
    // logica de abajo (slot, secreto, chatarra, droga, cobro) no depende
    // de cual, solo hace falta estar junto a CUALQUIERA de los dos.
    if (!IsPlayerInRangeOfPoint(playerid, FEDERICO_RADIO_ENTREGA, FEDERICO_X, FEDERICO_Y, FEDERICO_Z)
        && !IsPlayerInRangeOfPoint(playerid, FEDERICO_RADIO_ENTREGA, FEDERICO_ANGELPINE_X, FEDERICO_ANGELPINE_Y, FEDERICO_ANGELPINE_Z))
    {
        Error(playerid, "Tienes que estar junto a Federico para entregarle el vehiculo.");
        return 1;
    }

    if (g_FedericoProcesando)
    {
        Error(playerid, "Federico esta atendiendo a otro jugador. Intentalo de nuevo en un segundo.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (!VehiculoValido(indice))
    {
        Error(playerid, "Ese vehiculo no es valido.");
        return 1;
    }

    // Coches de Jugadores (59_federicojugadores.inc, 08/09/2026): UNICA
    // categoria de Federico que acepta un vehiculo CON dueno -- se
    // calcula ANTES del filtro de propiedad de abajo para poder
    // saltarselo SOLO en este caso concreto, siguiendo el mismo patron
    // de guarda que esRoboExpress/esDroga (calcular primero, nunca
    // relajar el filtro para las demas categorias). El filtro de abajo
    // sigue exigiendo coche libre sin dueno para TODO lo demas, sin
    // ninguna excepcion -- no se toca la logica global de propiedad.
    new bool:esCocheJugador = FedJugadores_PuedeEntregar(indice);

    if (!esCocheJugador && (g_Vehiculos[indice][veTipo] != VEH_DROPEO || g_Vehiculos[indice][vePropietario] != 0))
    {
        Error(playerid, "Federico solo acepta vehiculos libres, sin dueno.");
        return 1;
    }

    new modeloVehiculo = g_Vehiculos[indice][veModelo];

    // Robo Express (57_roboexpress.inc) SE CALCULA DESPUES de Coches de
    // Jugadores, pero ANTES que cualquier otra categoria y antes de
    // tocar "slot" -- identifica un VEHICULO fisico exacto por veID, no
    // un modelo, asi que nunca debe caer en la logica de slots de abajo.
    // Ver la nota de 07/09/2026 sobre el bug de g_FedericoSlot[-1]: esto
    // sigue exactamente el mismo patron de guarda que ya protege a
    // esDroga/esSecreto/esMoto. esCocheJugador y esRoboExpress son
    // mutuamente excluyentes por construccion (uno exige dueno, el otro
    // exige libre sin dueno -- dominios disjuntos hoy), pero se guardan
    // explicitamente el uno al otro de todas formas: exclusion mutua
    // garantizada por codigo, no solo confiada a la seleccion.
    new bool:esRoboExpress = (!esCocheJugador) && RoboExpress_PuedeEntregar(playerid, indice);
    new bool:esDroga = bool:(!esCocheJugador && !esRoboExpress && g_FedHistEstado == FEDHIST_BUSCANDO && g_FedHistVehiculoID != 0 && g_Vehiculos[indice][veID] == g_FedHistVehiculoID);
    new slot = -1;
    new bool:esSecreto = false;
    new bool:esMoto = false;
    new slotChatarra = -1;

    if (!esDroga && !esRoboExpress && !esCocheJugador)
    {
        slot = Federico_SlotDeModelo(modeloVehiculo);

        if (slot == -1)
        {
            if (Federico_EsVehiculoSecreto(modeloVehiculo))
            {
                esSecreto = true;
            }
            else if (Federico_EsVehiculoMoto(modeloVehiculo))
            {
                esMoto = true;
            }
            else
            {
                slotChatarra = Federico_ChatarraSlotDeModelo(modeloVehiculo);
                if (slotChatarra == -1)
                {
                    Error(playerid, "Federico no busca este vehiculo.");
                    return 1;
                }
            }
        }
    }
    new bool:esChatarra = bool:(slotChatarra != -1);

    // --- Zona critica: entre aqui y soltar el lock no hay ninguna
    // llamada que ceda el turno a otro callback (nada asincrono antes
    // de marcar feEntregado/g_FedSecretoEntregado/fcEntregado/borrar el
    // coche de la droga). Contrato explicito contra doble cobro, no
    // solo "Pawn es de un hilo". ---
    g_FedericoProcesando = true;

    if (!esDroga && !esRoboExpress && !esCocheJugador && !esSecreto && !esChatarra && !esMoto && g_FedericoSlot[slot][feEntregado])
    {
        // Alguien lo entrego un instante antes (dos jugadores con el
        // mismo modelo, casi a la vez): este llega segundo, no cobra.
        g_FedericoProcesando = false;
        Error(playerid, "Ya han entregado ese modelo esta ronda.");
        return 1;
    }

    if (esSecreto && g_FedSecretoEntregado)
    {
        // Mismo caso que arriba, para el Secreto.
        g_FedericoProcesando = false;
        Error(playerid, "Ya han entregado el Coche Secreto esta ronda.");
        return 1;
    }

    if (esMoto && g_FedMotoEntregado)
    {
        // Mismo caso, para la Moto del Dia.
        g_FedericoProcesando = false;
        Error(playerid, "Ya han entregado la Moto del Dia esta ronda.");
        return 1;
    }

    if (esChatarra && g_FedChatarraSlot[slotChatarra][fcEntregado])
    {
        // Mismo caso, para la mini lista chatarra.
        g_FedericoProcesando = false;
        Error(playerid, "Ya han entregado ese vehiculo esta ronda.");
        return 1;
    }

    new recompensa, modeloTexto[48];
    new vehId = g_Vehiculos[indice][veID];
    new pagoTexto[48] = "";  // solo se rellena en la mini lista chatarra, para el mensaje/log

    if (esCocheJugador)
    {
        // Ya se confirmo hace un instante (esCocheJugador arriba) que
        // este veID es uno de los 3 candidatos vigentes y sin entregar
        // -- se vuelve a resolver el slot aqui, dentro de la zona
        // critica, para no arrastrar un indice calculado antes del
        // candado. Si por lo que sea no aparece (nunca deberia, nada
        // asincrono entre medias), se trata igual que "ya entregado" en
        // vez de arriesgarse a tocar un slot invalido.
        new slotJug = FedJugadores_SlotDeVehiculo(vehId);
        if (slotJug == -1)
        {
            g_FedericoProcesando = false;
            Error(playerid, "Ya han entregado ese vehiculo.");
            return 1;
        }

        // Norma nueva (08/09/2026): el propio dueno no puede entregarse
        // su vehiculo a si mismo -- tiene que traerlo otro jugador de
        // verdad. Sin este corte, el dueno podia "robarse" su propio
        // coche sin que nadie se lo quitara antes (nunca necesita
        // /forzarcerradura sobre algo que ya es suyo).
        if (FedJugadores_EsPropietario(slotJug, Jugador[playerid][jID]))
        {
            g_FedericoProcesando = false;
            Error(playerid, "No puedes entregar tu propio vehiculo a Federico -- tiene que traerlo otro jugador.");
            return 1;
        }

        recompensa = FedJugadores_RecompensaDeSlot(slotJug);
        new nombreJug[32];
        FedJugadores_NombreDeSlot(slotJug, nombreJug, sizeof nombreJug);
        format(modeloTexto, sizeof modeloTexto, "%s (Coche de Jugador)", nombreJug);

        // Retirada + persistencia propia de 48h (59_federicojugadores.inc)
        // -- NO Federico_RetirarVehiculo, que usa el cooldown generico
        // de 24h de esta tabla; ver cabecera de ese modulo para el
        // porque de mantenerlo aislado.
        FedJugadores_Entregar(slotJug, indice);
    }
    else if (esRoboExpress)
    {
        recompensa = ROBOEXPRESS_RECOMPENSA;
        new nombreRobo[32];
        Veh_NombreModelo(modeloVehiculo, nombreRobo, 32);
        format(modeloTexto, sizeof modeloTexto, "%s (Robo Express)", nombreRobo);

        // Mismo camino seguro que ya usa el resto de Federico para
        // retirar un coche entregado (cooldown + respawn, NO borrado
        // permanente -- Robo Express no es el Personaje Historico).
        Federico_RetirarVehiculo(indice);
        RoboExpress_Finalizar();
    }
    else if (esDroga)
    {
        recompensa = FEDERICO_HISTORICO_RECOMPENSA;
        new nombreModeloHist[32];
        Veh_NombreModelo(modeloVehiculo, nombreModeloHist, 32);
        format(modeloTexto, sizeof modeloTexto, "%s (Coche de %s)", nombreModeloHist, g_Historicos[g_FedHistPersonajeID][hPersonaje]);

        // Desaparece PARA SIEMPRE -- a diferencia del resto de Federico,
        // no vuelve a los 24h. Veh_Borrar ya hace DELETE en `vehiculos`
        // (el ON DELETE CASCADE se lleva su fila de `maletero` con el).
        Veh_Borrar(indice);

        // Se queda en estado ENTREGADO (no NINGUNO): la web sigue
        // mostrando su nombre en rojo hasta que se cree otro con
        // /crearhistorico -- ver g_FedHistEstado arriba.
        g_FedHistEstado = FEDHIST_ENTREGADO;
        Federico_HistoricoPersistir();
    }
    else if (esSecreto)
    {
        recompensa = FEDERICO_SECRETO_RECOMPENSA;
        new nombreReal[32];
        Veh_NombreModelo(modeloVehiculo, nombreReal, 32);
        format(modeloTexto, sizeof modeloTexto, "%s (Coche Secreto)", nombreReal);

        g_FedSecretoEntregado  = true;
        g_FedSecretoVehiculoID = vehId;

        Federico_RetirarVehiculo(indice);
        Federico_SecretoPersistirEntrega();
        // Ya NO se busca uno nuevo de inmediato (cambio del 08/09/2026,
        // sincronizacion general): se queda en ENTREGADO hasta que
        // Federico_RondaGeneralSincronizar() rote las 4 categorias
        // juntas, para que /verlista cambie entera de golpe una vez al
        // dia en vez de que el Secreto vaya a su propio ritmo.
    }
    else if (esMoto)
    {
        recompensa = FEDERICO_MOTO_RECOMPENSA;
        new nombreMoto[32];
        Veh_NombreModelo(modeloVehiculo, nombreMoto, 32);
        format(modeloTexto, sizeof modeloTexto, "%s (Moto del Dia)", nombreMoto);

        g_FedMotoEntregado  = true;
        g_FedMotoVehiculoID = vehId;

        Federico_RetirarVehiculo(indice);
        Federico_MotoPersistirEntrega();
        // Igual que el Secreto (ver comentario de arriba): ya NO se busca
        // una moto nueva de inmediato, se espera a la sincronizacion
        // general de las 4 categorias.
    }
    else if (esChatarra)
    {
        recompensa = 0; // no paga dinero -- ver pagoTexto/reparto de abajo
        strmid(modeloTexto, g_FedChatarraSlot[slotChatarra][fcNombre], 0, 32, sizeof modeloTexto);

        g_FedChatarraSlot[slotChatarra][fcEntregado]  = true;
        g_FedChatarraSlot[slotChatarra][fcVehiculoID] = vehId;

        Federico_RetirarVehiculo(indice);
        Federico_ChatarraPersistirEntrega(slotChatarra);

        // Reparto fijo por posicion (05/09/2026): 0=materiales,
        // 1=marihuana, 2=arma + municion -- ciclo de 3 en 3
        // (slotChatarra % 3) desde que la lista paso de 3 a 5 ranuras el
        // 08/09/2026, asi los slots 3 y 4 repiten materiales/marihuana
        // sin inventar un cuarto/quinto tipo de pago. Materiales y
        // marihuana subidos un 500% el 13/09/2026; el arma paso de Uzi a
        // Spaz 12 con 200 balas fijas (no escalada por el 500%, peticion
        // expresa de Vlad). Si el inventario no tiene hueco (Inv_DarItem
        // devuelve -1), se avisa pero la
        // entrega del vehiculo ya es irreversible, igual que si a
        // alguien no le cupiera el dinero (aqui no puede pasar con
        // dinero, pero si con objetos).
        new bool:cupo = true;
        switch (slotChatarra % 3)
        {
            case 0:
            {
                if (Inv_DarItem(playerid, Item_Buscar("Materiales"), FEDERICO_CHATARRA_MATERIALES, 0) == -1) cupo = false;
                format(pagoTexto, sizeof pagoTexto, "%d de Materiales", FEDERICO_CHATARRA_MATERIALES);
            }
            case 1:
            {
                if (Inv_DarItem(playerid, Item_Buscar("Marihuana"), FEDERICO_CHATARRA_MARIHUANA, 0) == -1) cupo = false;
                format(pagoTexto, sizeof pagoTexto, "%d de Marihuana", FEDERICO_CHATARRA_MARIHUANA);
            }
            case 2:
            {
                if (Inv_DarItem(playerid, Item_Buscar("Spaz 12"), 1, 0) == -1) cupo = false;
                if (Inv_DarItem(playerid, Item_Buscar("Municion de Spaz 12"), FEDERICO_CHATARRA_ARMA_MUNICION, 0) == -1) cupo = false;
                format(pagoTexto, sizeof pagoTexto, "una Spaz 12 con %d balas", FEDERICO_CHATARRA_ARMA_MUNICION);
            }
        }
        if (!cupo) Error(playerid, "Llevabas demasiado peso encima -- parte del premio no ha cabido en tu inventario.");
    }
    else
    {
        recompensa = g_FedericoSlot[slot][feRecompensa];
        strmid(modeloTexto, g_FedericoSlot[slot][feNombre], 0, 32, sizeof modeloTexto);

        g_FedericoSlot[slot][feEntregado]  = true;
        g_FedericoSlot[slot][feVehiculoID] = vehId;

        Federico_RetirarVehiculo(indice);
        Federico_PersistirEntrega(slot);
        Federico_ActualizarSlotTextdraw(slot); // quien tenga /federico abierto ve el tachado en vivo
    }

    Teletipo_FedericoEntrega(); // 62_teletipo.inc -- solo cuenta y avisa en 2/4/6/8/10, no cambia nada de Federico

    if (recompensa > 0) DarDinero(playerid, recompensa);

    // Contador persistente para el ranking web (/ranking, categoria
    // "Coches de Federico"). Mismo patron que nfs_coches_vendidos
    // (26_nfs.inc): UPDATE directo, sin campo en memoria. Cuenta igual
    // para el Secreto y la mini lista chatarra -- siguen siendo una
    // entrega a Federico.
    format(g_FedericoConsulta, sizeof g_FedericoConsulta,
        "UPDATE cuentas SET federico_entregas = federico_entregas + 1 WHERE id = %d", Jugador[playerid][jID]);
    mysql_pquery(g_BD, g_FedericoConsulta);

    // Contador aparte, solo para el Coche Secreto (05/09/2026) -- web
    // "Ranking de coches secretos", categoria propia en /ranking.
    if (esSecreto)
    {
        format(g_FedericoConsulta, sizeof g_FedericoConsulta,
            "UPDATE cuentas SET federico_secretos = federico_secretos + 1 WHERE id = %d", Jugador[playerid][jID]);
        mysql_pquery(g_BD, g_FedericoConsulta);
    }

    // Contador aparte, solo para Personaje Historico (07/09/2026) --
    // mismo patron que el del Coche Secreto de arriba, web "Personaje
    // Historico de Federico", categoria propia en /ranking y tarjeta
    // propia en /federico.
    if (esDroga)
    {
        format(g_FedericoConsulta, sizeof g_FedericoConsulta,
            "UPDATE cuentas SET federico_historicos = federico_historicos + 1 WHERE id = %d", Jugador[playerid][jID]);
        mysql_pquery(g_BD, g_FedericoConsulta);
    }

    g_FedericoProcesando = false;
    // --- Fin de zona critica ---

    if (esChatarra)
    {
        format(g_Buffer, sizeof g_Buffer, "Federico acepta el %s. Has recibido %s.", modeloTexto, pagoTexto);
    }
    else
    {
        new precioTexto[24];
        FormatearDinero(recompensa, precioTexto, sizeof precioTexto);
        format(g_Buffer, sizeof g_Buffer, "Federico acepta el %s. Has recibido %s.", modeloTexto, precioTexto);
    }
    Exito(playerid, g_Buffer);

    new nombreJugador[MAX_PLAYER_NAME + 1], logMsg[280];
    NombreRP(playerid, nombreJugador, sizeof nombreJugador);
    format(logMsg, sizeof logMsg,
        "FEDERICO_VEHICLE_DELIVERED%s jugador=%s vehiculo_id=%d modelo=%s recompensa=%d pago=%s timestamp=%d federico_until=%d",
        (esCocheJugador) ? ("_COCHEJUGADOR") : ((esRoboExpress) ? ("_ROBOEXPRESS") : ((esDroga) ? ("_HISTORICO") : ((esSecreto) ? ("_SECRETO") : ((esMoto) ? ("_MOTO") : ((esChatarra) ? ("_CHATARRA") : ("")))))),
        nombreJugador, vehId, modeloTexto, recompensa, (esChatarra) ? (pagoTexto) : (""), gettime(),
        gettime() + (esCocheJugador ? FEDJUGADORES_RETORNO_SEGUNDOS : FEDERICO_COOLDOWN));
    EscribirLog("federico", logMsg);
    return 1;
}
