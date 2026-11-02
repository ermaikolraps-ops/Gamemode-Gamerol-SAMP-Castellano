/* =====================================================================
 *  10_inventario.inc
 *  ---------------------------------------------------------------------
 *  Inventario con ranuras y peso.
 *
 *  Para añadir un objeto nuevo basta con añadir una fila al array
 *  g_Items de abajo. El identificador del objeto es su posición en el
 *  array (la fila 0 es "nada", no la toques).
 *
 *  Tipos:
 *    ITEM_OBJETO      objeto normal
 *    ITEM_ARMA        al usarlo pasa a las manos; itDato = id del arma
 *    ITEM_CONSUMIBLE  al usarlo desaparece; itDato = vida que recupera
 *    ITEM_MATERIAL    se apila en grandes cantidades
 *    ITEM_CONTENEDOR  al usarlo aumenta la carga; itDato = kilos extra
 *    ITEM_MUNICION    al usarlo recarga el arma; itDato = id del arma
 * ===================================================================== */

#define ITEM_OBJETO             0
#define ITEM_ARMA               1
#define ITEM_CONSUMIBLE         2
#define ITEM_MATERIAL           3
#define ITEM_CONTENEDOR         4
#define ITEM_MUNICION           5

// Peso base que puede cargar cualquiera, en kilos. Subido de 30 a 60 el
// 07/09/2026, peticion expresa de Vlad.
#define PESO_BASE               60.0

// Segundos que permanece un objeto tirado en el suelo antes de perderse.
#define SUELO_DURACION          600

enum E_ITEM
{
    itNombre[32],
    Float:itPeso,
    itTipo,
    itDato,
    itModelo,               // Modelo que se ve al tirarlo al suelo
    itMaxPila,
    itDescripcion[112]
}

// ---------------------------------------------------------------------
//  CATALOGO DE OBJETOS
// ---------------------------------------------------------------------
static const g_Items[][E_ITEM] =
{
    // Nombre                Peso   Tipo             Dato  Modelo  Pila  Descripcion
    {"",                      0.0,  ITEM_OBJETO,        0,   1210,    1, ""},

    // --- Personales ---------------------------------------------------
    {"Movil",                 0.2,  ITEM_OBJETO,        0,   1210,    1, "Un Motorola de los grandes. En 1992 esto es un lujo."},
    {"Cartera",               0.2,  ITEM_OBJETO,        0,   1210,    1, "Cuero gastado. Dentro llevas lo poco que tienes."},
    {"Documentacion",         0.1,  ITEM_OBJETO,        0,   1210,    1, "Tu identificacion. La policia puede pedirtela."},
    {"Llaves de casa",        0.1,  ITEM_OBJETO,        0,   1210,    1, "Un llavero con dos llaves y un abridor."},
    {"Llaves del coche",      0.1,  ITEM_OBJETO,        0,   1210,    1, "Las llaves de tu vehiculo."},
    {"Reloj",                 0.1,  ITEM_OBJETO,        0,   1210,    1, "Marca la hora. A veces incluso la correcta."},
    {"Mechero",               0.1,  ITEM_OBJETO,        0,   1210,    1, "Zippo plateado con una abolladura."},
    {"Paquete de tabaco",     0.1,  ITEM_CONSUMIBLE,    0,   1210,   20, "Rubio americano. Quedan unos cuantos."},
    {"Linterna",              0.5,  ITEM_OBJETO,        0,   1210,    1, "Pesada, de las de pilas grandes."},
    {"Periodico",             0.2,  ITEM_OBJETO,        0,   1210,    5, "El Los Santos Herald de esta manana."},
    {"Camara de fotos",       1.2,  ITEM_OBJETO,        0,   1210,    1, "Una reflex con carrete de 36."},
    {"Radio portatil",        0.6,  ITEM_OBJETO,        0,   1210,    1, "Emisora de mano. Util en servicio."},
    {"Maletin",               2.0,  ITEM_OBJETO,        0,   1210,    1, "Rigido, con cierre de combinacion."},

    // --- Contenedores --------------------------------------------------
    {"Mochila",               1.0,  ITEM_CONTENEDOR,   15,   1210,    1, "Al ponertela puedes cargar 15 kg mas."},

    // --- Comida y salud ------------------------------------------------
    {"Botella de agua",       0.5,  ITEM_CONSUMIBLE,    5,   1210,   10, "Agua fresca. Recupera un poco de energia."},
    {"Bocadillo",             0.4,  ITEM_CONSUMIBLE,   15,   1210,   10, "Jamon y queso. Quita el hambre."},
    {"Hamburguesa",           0.4,  ITEM_CONSUMIBLE,   20,   1210,   10, "Grasienta, tibia y perfecta."},
    {"Cafe",                  0.3,  ITEM_CONSUMIBLE,    5,   1210,   10, "Solo y muy cargado."},
    {"Te Helado",             0.4,  ITEM_CONSUMIBLE,    5,   1210,   10, "Bien frio, con un toque de limon."},
    {"Gaseosa",               0.4,  ITEM_CONSUMIBLE,    5,   1210,   10, "Refresco con gas, de lata."},
    {"Vendas",                0.3,  ITEM_CONSUMIBLE,   35,   1240,   10, "Detienen una hemorragia leve."},
    {"Botiquin",              1.5,  ITEM_CONSUMIBLE,   70,   1240,    5, "Equipo de primeros auxilios completo."},
    {"Chaleco antibalas",     4.0,  ITEM_OBJETO,      100,   1242,    1, "Kevlar. Al usarlo te lo pones."},

    // --- Herramientas y trabajo ----------------------------------------
    {"Bidon de gasolina",    12.0,  ITEM_OBJETO,        0,   1210,    1, "20 litros. Pesa una barbaridad."},
    {"Kit de reparacion",     6.0,  ITEM_OBJETO,        0,   1210,    3, "Sirve para arreglar un vehiculo averiado."},
    {"Caja de herramientas",  8.0,  ITEM_OBJETO,        0,   1210,    1, "Llaves, destornilladores y un martillo."},
    {"Palanca",               2.5,  ITEM_OBJETO,        0,   1210,    1, "Hierro de metro y medio. Abre cosas."},
    {"Cuerda",                1.5,  ITEM_OBJETO,        0,   1210,    3, "Diez metros de cuerda de nylon."},
    {"Bote de spray",         0.6,  ITEM_OBJETO,        0,   1210,    5, "Pintura roja. Para marcar territorio."},
    {"Bolsa de basura",       0.5,  ITEM_OBJETO,        0,   1210,   10, "Industrial, de las negras grandes."},
    {"Materiales",          0.001,  ITEM_MATERIAL,      0,   1210, 5000, "Componentes sueltos. Se acumulan."},
    {"Chatarra",              0.3,  ITEM_MATERIAL,      0,   1210,  500, "Metal reciclable. Se vende al peso."},

    // --- Armas ---------------------------------------------------------
    {"Navaja",                0.4,  ITEM_ARMA,          4,    335,    1, "Mango de nacar, hoja corta."},
    {"Bate de beisbol",       1.5,  ITEM_ARMA,          5,    336,    1, "De madera, con la marca borrada."},
    {"Pistola 9mm",           1.1,  ITEM_ARMA,         22,    346,    1, "Semiautomatica. Cabe en el bolsillo."},
    {"Escopeta recortada",    3.4,  ITEM_ARMA,         25,    349,    1, "Calibre 12. Ruidosa y poco sutil."},
    {"Subfusil Tec-9",        1.6,  ITEM_ARMA,         32,    372,    1, "Rafaga rapida, punteria discutible."},
    {"Fusil de asalto",       3.8,  ITEM_ARMA,         30,    355,    1, "Kalashnikov. Esto ya son palabras mayores."},
    {"Extintor",              5.0,  ITEM_ARMA,         42,    365,    1, "Polvo ABC. Apaga fuegos y discusiones."},

    // --- Municion ------------------------------------------------------
    {"Municion 9mm",         0.02,  ITEM_MUNICION,     22,   1210,  500, "Cajita de balas de 9 milimetros."},
    {"Cartuchos",            0.05,  ITEM_MUNICION,     25,   1210,  200, "Cartuchos del 12 para escopeta."},
    {"Municion de fusil",    0.03,  ITEM_MUNICION,     30,   1210,  500, "Munición de 7,62. Pesa lo suyo."},

    // --- Cultivo (28_marihuana.inc) -------------------------------------
    {"Semilla de marihuana",  0.05, ITEM_OBJETO,        0,   1210, 1000, "Se planta al aire libre con /plantar."},
    {"Marihuana",              0.0, ITEM_MATERIAL,      0,   1210, 999999999, "Cogollos secos. No pesa nada, se vende por gramos."},

    // --- Portatil Hacker (45_maletero.inc, 05/09/2026) ------------------
    // Objeto real de inventario, no una variable booleana de cuenta --
    // se apila (x1, x2, x5...), se guarda en el maletero como cualquier
    // otro recurso, y la web (Panel del Jugador) comprueba tenerlo
    // encima consultando el inventario real, no un flag aparte.
    {"Portatil Hacker",       2.0,  ITEM_OBJETO,        0,   1210,   10, "Objeto de esta edicion. No abre ninguna pagina."},

    // --- Uzi (05/09/2026, premio de la mini lista chatarra de Federico) -
    // AL FINAL a proposito: el id de cada objeto es su posicion en este
    // array, insertar en medio desincroniza el espejo web (ver el fix
    // del 05/09/2026 en itemNames.js -- no repetir ese error).
    {"Uzi",                   1.4,  ITEM_ARMA,         28,    372,    1, "A dos manos. Rafaga corta, mucho retroceso."},
    {"Municion de Uzi",      0.02,  ITEM_MUNICION,     28,   1210,  999, "Cargadores de 9 milimetros para la Uzi."},

    // --- Objetos Pawnshop (45_maletero.inc, 06/09/2026) -----------------
    // Loot de coches libres, sin valor de venta todavia utilizable (el
    // Pawnshop/perista es una fase posterior) -- el precio previsto de
    // cada uno vive en g_PawnshopCatalogo (45_maletero.inc), no aqui, para
    // no anadir un campo mas a E_ITEM que valdria 0 para los otros 47
    // objetos de este catalogo. AL FINAL a proposito, mismo motivo que la
    // Uzi de arriba: el id es la posicion en este array.
    {"Gafas de sol",          0.1,  ITEM_OBJETO,        0,   1210,    1, "De pasta, con un cristal rayado."},
    {"Pato de goma",          0.2,  ITEM_OBJETO,        0,   1210,    1, "Amarillo chillon. Ni se sabe por que iba ahi."},
    {"Mechero Zippo",         0.1,  ITEM_OBJETO,        0,   1210,    1, "Grabado con unas iniciales que no son las tuyas."},
    {"Calzoncillos de marca", 0.1,  ITEM_OBJETO,        0,   1210,    1, "Todavia con la etiqueta puesta."},
    {"Auriculares",           0.2,  ITEM_OBJETO,        0,   1210,    1, "Inalambricos, con el estuche de carga."},
    {"CD de reggaeton de 2009", 0.1, ITEM_OBJETO,       0,   1210,    1, "La portada ya dice todo lo que hace falta saber."},
    {"Camara de fotos",       1.0,  ITEM_OBJETO,        0,   1210,    1, "Compacta, de las de apuntar y disparar."},
    {"Zapatillas de marca",   0.8,  ITEM_OBJETO,        0,   1210,    1, "Edicion limitada. Apenas usadas."},
    {"Altavoz portatil",      1.0,  ITEM_OBJETO,        0,   1210,    1, "Bluetooth, con la bateria a la mitad."},
    {"Muneca hinchable",      1.5,  ITEM_OBJETO,        0,   1210,    1, "Todavia en la caja. Mejor no preguntar."},
    {"Telefono movil",        0.2,  ITEM_OBJETO,        0,   1210,    1, "Ultimo modelo, bloqueado con PIN."},
    {"Trofeo Mejor Conductor",1.5,  ITEM_OBJETO,        0,   1210,    1, "De una liga de karts de la que nadie se acuerda."},
    {"Camara GoPro",          0.3,  ITEM_OBJETO,        0,   1210,    1, "Con soporte y todo, lista para grabar."},
    {"Tablet",                0.5,  ITEM_OBJETO,        0,   1210,    1, "Pantalla agrietada en una esquina."},
    {"Consola portatil",      0.4,  ITEM_OBJETO,        0,   1210,    1, "Con la funda y un juego metido dentro."},
    {"Ordenador portatil",    2.5,  ITEM_OBJETO,        0,   1210,    1, "Buena marca. Ojala tuviera la contrasena."},
    {"Cadena de oro",         0.3,  ITEM_OBJETO,        0,   1210,    1, "Gruesa, con un colgante a juego."},
    {"Camara profesional",    1.8,  ITEM_OBJETO,        0,   1210,    1, "Reflex de las buenas, con dos objetivos."},
    {"Anillo de diamantes",  0.05,  ITEM_OBJETO,        0,   1210,    1, "Una piedra pequena pero de las de verdad."},
    {"Bolso de lujo",         1.2,  ITEM_OBJETO,        0,   1210,    1, "Firma italiana. El cuero todavia huele a nuevo."},
    {"Lingote de oro pequeno",1.0,  ITEM_OBJETO,        0,   1210,    1, "Mas pesado de lo que parece."},
    {"Reloj de lujo",         0.2,  ITEM_OBJETO,        0,   1210,    1, "Suizo, de los que valen un coche."},
    {"Diamante",             0.02,  ITEM_OBJETO,        0,   1210,    1, "Sin engarzar. Alguien lo tenia guardado con cuidado."},
    {"Maletin con joyas",     2.0,  ITEM_OBJETO,        0,   1210,    1, "Forrado por dentro. No lleva ni una pieza barata."},
    {"Rolex de oro",          0.2,  ITEM_OBJETO,        0,   1210,    1, "El de verdad, no una copia. Certificado incluido."},

    // --- Gamerol Aventuras: DLC Pesca (51_pesca.inc, 06/09/2026) --------
    // AL FINAL a proposito, mismo motivo que todo lo de arriba: el id de
    // cada objeto es su posicion en este array. Espejo web actualizado
    // en GamerolOnlineWeb/src/itemNames.js en el mismo orden exacto.
    //
    // Los 10 peces crudos van con itMaxPila=1: cada captura pesa distinto
    // (el peso individual real se guarda en SLOT_DATOS, en gramos, igual
    // patron que la municion guardada en la ranura de un arma) y NUNCA
    // deben fundirse en una sola pila -- perderiamos el peso de cada
    // pieza, que hace falta para el valor de venta, los records y las
    // estadisticas. itPeso aqui es solo un peso MEDIO de la especie para
    // el calculo de carga del inventario (Inv_PesoActual/Inv_PesoMaximo
    // trabajan con un peso fijo por fila de catalogo, no con SLOT_DATOS) --
    // decision documentada: el peso EXACTO de cada pieza vive aparte.
    {"Cana de pescar",        1.5,  ITEM_OBJETO,        0,   1210,    1, "Unica y sencilla. Necesaria para usar /pescar."},
    {"Barbacoa",             15.0,  ITEM_OBJETO,        0,   1481,    1, "Se despliega con /sacarbarbacoa. Es personal: solo tu puedes usarla."},

    {"Sardina",               0.5,  ITEM_OBJETO,        0,   1210,    1, "Pescada al azar. El peso varia con cada pieza."},
    {"Boqueron",              0.4,  ITEM_OBJETO,        0,   1210,    1, "Pequeno y plateado. El peso varia con cada pieza."},
    {"Caballa",               0.8,  ITEM_OBJETO,        0,   1210,    1, "De lomo rayado. El peso varia con cada pieza."},
    {"Trucha",                1.2,  ITEM_OBJETO,        0,   1210,    1, "De agua dulce. El peso varia con cada pieza."},
    {"Dorada",                1.9,  ITEM_OBJETO,        0,   1210,    1, "Muy apreciada. El peso varia con cada pieza."},
    {"Lubina",                2.2,  ITEM_OBJETO,        0,   1210,    1, "Carne blanca y firme. El peso varia con cada pieza."},
    {"Salmon",                3.2,  ITEM_OBJETO,        0,   1210,    1, "Rosado por dentro. El peso varia con cada pieza."},
    {"Pez globo",             1.2,  ITEM_OBJETO,        0,   1210,    1, "Curioso y esponjoso. El peso varia con cada pieza."},
    {"Pez espada",            5.5,  ITEM_OBJETO,        0,   1210,    1, "Grande y alargado. El peso varia con cada pieza."},
    {"Atun",                  7.0,  ITEM_OBJETO,        0,   1210,    1, "El mas grande de todos. El peso varia con cada pieza."},

    // 30 platos cocinados (10 especies x 3 recetas). ITEM_CONSUMIBLE,
    // pero su beneficio NO sale de itDato (queda a 0, sin usar): lo
    // calcula Pesca_ComerPlato() aparte -- vida al 100% y +10% de escudo
    // con tope 50%, distinto del resto de comidas del servidor.
    {"Sardina al limon",      0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Sardina al ajillo",     0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Sardina al carbon",     0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Boqueron al limon",     0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Boqueron al ajillo",    0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Boqueron al carbon",    0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Caballa al limon",      0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Caballa al ajillo",     0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Caballa al carbon",     0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Trucha al limon",       0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Trucha al ajillo",      0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Trucha al carbon",      0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Dorada al limon",       0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Dorada al ajillo",      0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Dorada al carbon",      0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Lubina al limon",       0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Lubina al ajillo",      0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Lubina al carbon",      0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparada a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Salmon al limon",       0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Salmon al ajillo",      0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Salmon al carbon",      0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Pez globo al limon",    0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Pez globo al ajillo",   0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Pez globo al carbon",   0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Pez espada al limon",   0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Pez espada al ajillo",  0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Pez espada al carbon",  0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Atun al limon",         0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Atun al ajillo",        0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},
    {"Atun al carbon",        0.3,  ITEM_CONSUMIBLE,    0,   1210,    5, "Preparado a la barbacoa. Restaura la vida y da algo de escudo."},

    // --- Spaz 12 (13/09/2026, cambio de premio arma chatarra Federico --
    // antes daba una Uzi, ver FEDERICO_CHATARRA_ARMA en 01_configuracion.inc
    // y el switch de 43_federico.inc). AL FINAL a proposito, mismo motivo
    // que la Uzi de mas arriba: el id es la posicion en este array.
    // Espejo web actualizado en GamerolOnlineWeb/src/itemNames.js en el
    // mismo orden exacto.
    {"Spaz 12",               3.6,  ITEM_ARMA,         27,    349,    1, "Escopeta de combate. Recarga lenta, parada segura."},
    {"Municion de Spaz 12",  0.05,  ITEM_MUNICION,     27,   1210,  999, "Cartuchos del 12 para la Spaz 12."}
};

#define TOTAL_ITEMS             sizeof(g_Items)

// ---------------------------------------------------------------------
//  ALMACENAMIENTO EN MEMORIA
// ---------------------------------------------------------------------
enum { SLOT_ITEM, SLOT_CANTIDAD, SLOT_DATOS, SLOT_CAMPOS };

static g_Inv[MAX_PLAYERS][MAX_SLOTS_INVENTARIO][SLOT_CAMPOS];
static bool:g_InvCargado[MAX_PLAYERS];

// Objetos tirados por el suelo.
enum E_SUELO
{
    bool:suUsado,
    suItem,
    suCantidad,
    suDatos,
    Float:suX, Float:suY, Float:suZ,
    suInterior,
    suMundo,
    STREAMER_TAG_OBJECT:suObjeto,
    STREAMER_TAG_3D_TEXT_LABEL:suEtiqueta,
    suSegundos
}
static g_Suelo[MAX_OBJETOS_SUELO][E_SUELO];

// ---------------------------------------------------------------------
//  CONSULTAS BÁSICAS
// ---------------------------------------------------------------------
stock bool:ItemValido(item)
{
    return (item > 0 && item < TOTAL_ITEMS);
}

stock Item_Nombre(item, destino[], tam)
{
    if (!ItemValido(item)) { format(destino, tam, "Desconocido"); return 0; }
    format(destino, tam, "%s", g_Items[item][itNombre]);
    return 1;
}

// Item ITEM_ARMA cuyo itDato coincide con ese arma de GTA, o 0 si esa
// arma no tiene item asociado (g_Items es "static": este es el unico
// punto de acceso para otros modulos que necesiten la equivalencia).
stock Item_BuscarPorArma(WEAPON:arma)
{
    for (new i = 1; i < TOTAL_ITEMS; i++)
    {
        if (g_Items[i][itTipo] == ITEM_ARMA && g_Items[i][itDato] == _:arma) return i;
    }
    return 0;
}

// Busca un objeto por nombre completo o parcial. Devuelve 0 si no existe
// y -1 si el texto encaja con más de uno.
stock Item_Buscar(const texto[])
{
    if (EstaVacio(texto)) return 0;

    if (EsNumeroEntero(texto))
    {
        new id = strval(texto);
        return ItemValido(id) ? id : 0;
    }

    new encontrado = 0, coincidencias = 0;

    for (new i = 1; i < TOTAL_ITEMS; i++)
    {
        if (strcmp(g_Items[i][itNombre], texto, true) == 0) return i;
        if (strfind(g_Items[i][itNombre], texto, true) != -1)
        {
            encontrado = i;
            coincidencias++;
        }
    }
    return (coincidencias > 1) ? -1 : encontrado;
}

public Float:Inv_PesoMaximo(playerid)
{
    return PESO_BASE + Jugador[playerid][jPesoExtra];
}

public Float:Inv_PesoActual(playerid)
{
    new Float:total = 0.0;

    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        new item = g_Inv[playerid][s][SLOT_ITEM];
        if (!ItemValido(item)) continue;
        total += g_Items[item][itPeso] * float(g_Inv[playerid][s][SLOT_CANTIDAD]);
    }
    return total;
}

public Inv_TieneItem(playerid, item)
{
    new total = 0;
    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        if (g_Inv[playerid][s][SLOT_ITEM] == item) total += g_Inv[playerid][s][SLOT_CANTIDAD];
    }
    return total;
}

stock Inv_SlotsLibres(playerid)
{
    new libres = 0;
    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        if (g_Inv[playerid][s][SLOT_ITEM] == 0) libres++;
    }
    return libres;
}

// ---------------------------------------------------------------------
//  DAR Y QUITAR
//  Devuelve  1 correcto · 0 sin ranuras libres · -1 demasiado peso
//
//  FIX 05/09/2026 (encargo de Vlad, encontrado via "el bote de spray
//  pesa 240 kg"): esta funcion SOLO respetaba itMaxPila al rellenar una
//  ranura YA existente -- al abrir una ranura NUEVA metia la cantidad
//  entera pedida sin tope, asi que /darobjeto X "Bote de spray" 400
//  (tope real 5) creaba una unica ranura de 400 unidades, 240 kg.
//  Ahora reparte en tantas ranuras de itMaxPila como haga falta, y
//  comprueba ANTES de escribir nada que hay ranuras libres suficientes
//  para el resto -- todo o nada, nunca se pierde ni se trunca en
//  silencio parte de lo que se iba a dar.
// ---------------------------------------------------------------------
public Inv_DarItem(playerid, item, cantidad, datos)
{
    if (!ItemValido(item) || cantidad < 1) return 0;

    new Float:pesoNuevo = g_Items[item][itPeso] * float(cantidad);
    if (Inv_PesoActual(playerid) + pesoNuevo > Inv_PesoMaximo(playerid)) return -1;

    new maxPila = g_Items[item][itMaxPila];
    if (maxPila < 1) maxPila = 1;

    new restante = cantidad;

    // Fase 1: rellenar pilas existentes de este mismo item/datos hasta
    // su tope, nunca por encima.
    if (maxPila > 1)
    {
        for (new s = 0; s < MAX_SLOTS_INVENTARIO && restante > 0; s++)
        {
            if (g_Inv[playerid][s][SLOT_ITEM] != item) continue;
            if (g_Inv[playerid][s][SLOT_DATOS] != datos) continue;

            new hueco = maxPila - g_Inv[playerid][s][SLOT_CANTIDAD];
            if (hueco <= 0) continue;

            new mover = (restante < hueco) ? restante : hueco;
            g_Inv[playerid][s][SLOT_CANTIDAD] += mover;
            restante -= mover;
        }
    }

    // Fase 2: lo que sobre, en ranuras libres nuevas, cada una con como
    // mucho itMaxPila unidades. Se comprueba primero que hay ranuras de
    // sobra para todo el resto -- si no las hay, no se toca ni una sola
    // (mejor "no cabe" claro que dar solo una parte sin avisar).
    if (restante > 0)
    {
        new necesarias = (restante + maxPila - 1) / maxPila;
        new libres = 0;
        for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++) { if (g_Inv[playerid][s][SLOT_ITEM] == 0) libres++; }
        if (libres < necesarias) return 0;

        for (new s = 0; s < MAX_SLOTS_INVENTARIO && restante > 0; s++)
        {
            if (g_Inv[playerid][s][SLOT_ITEM] != 0) continue;

            new mover = (restante < maxPila) ? restante : maxPila;
            g_Inv[playerid][s][SLOT_ITEM]     = item;
            g_Inv[playerid][s][SLOT_CANTIDAD] = mover;
            g_Inv[playerid][s][SLOT_DATOS]    = datos;
            restante -= mover;
        }
    }

    return 1;
}

// ---------------------------------------------------------------------
//  DAR ITEM A UNA CUENTA QUE PUEDE ESTAR DESCONECTADA (cosecha a
//  distancia via web, Agricultor Personal...). Nunca insertes una fila
//  nueva a pelo para esto -- intenta apilar primero en una fila ya
//  existente (respetando itMaxPila, igual que Inv_DarItem en memoria) y
//  solo crea una fila si no hay hueco en ninguna. Sin esto, cada cosecha
//  offline se ganaba su propia fila nueva y el inventario se llenaba de
//  pilas repetidas del mismo objeto.
// ---------------------------------------------------------------------
stock Inv_DarItemOffline(cuentaId, item, cantidad, datos)
{
    if (!ItemValido(item) || cantidad < 1) return 0;

    new consulta[280];
    if (g_Items[item][itMaxPila] > 1)
    {
        format(consulta, sizeof consulta,
            "UPDATE inventario SET cantidad = cantidad + %d \
             WHERE cuenta_id = %d AND item = %d AND datos = %d AND cantidad + %d <= %d LIMIT 1",
            cantidad, cuentaId, item, datos, cantidad, g_Items[item][itMaxPila]);
    }
    else
    {
        format(consulta, sizeof consulta,
            "UPDATE inventario SET cantidad = cantidad + %d WHERE cuenta_id = %d AND item = %d AND datos = %d LIMIT 1",
            cantidad, cuentaId, item, datos);
    }
    mysql_pquery(g_BD, consulta, "Inv_AlSumarOffline", "dddd", cuentaId, item, cantidad, datos);
    return 1;
}

forward Inv_AlSumarOffline(cuentaId, item, cantidad, datos);
public Inv_AlSumarOffline(cuentaId, item, cantidad, datos)
{
    // Ya se apilo en una fila existente, no hace falta crear ninguna.
    if (cache_affected_rows() > 0) return 1;

    new consulta[320];
    format(consulta, sizeof consulta,
        "INSERT INTO inventario (cuenta_id, slot, item, cantidad, datos) \
         VALUES (%d, (SELECT COALESCE(MAX(slot),-1)+1 FROM (SELECT slot FROM inventario WHERE cuenta_id=%d) x), %d, %d, %d)",
        cuentaId, cuentaId, item, cantidad, datos);
    mysql_pquery(g_BD, consulta);
    return 1;
}

public Inv_QuitarItem(playerid, item, cantidad)
{
    new restante = cantidad;

    for (new s = 0; s < MAX_SLOTS_INVENTARIO && restante > 0; s++)
    {
        if (g_Inv[playerid][s][SLOT_ITEM] != item) continue;

        if (g_Inv[playerid][s][SLOT_CANTIDAD] > restante)
        {
            g_Inv[playerid][s][SLOT_CANTIDAD] -= restante;
            restante = 0;
        }
        else
        {
            restante -= g_Inv[playerid][s][SLOT_CANTIDAD];
            Inv_VaciarSlot(playerid, s);
        }
    }
    return (restante == 0);
}

stock Inv_VaciarSlot(playerid, slot)
{
    g_Inv[playerid][slot][SLOT_ITEM]     = 0;
    g_Inv[playerid][slot][SLOT_CANTIDAD] = 0;
    g_Inv[playerid][slot][SLOT_DATOS]    = 0;
    return 1;
}

public Inv_Limpiar(playerid)
{
    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++) Inv_VaciarSlot(playerid, s);
    return 1;
}

public Inv_DarEquipoInicial(playerid)
{
    Inv_DarItem(playerid, Item_Buscar("Documentacion"), 1, 0);
    Inv_DarItem(playerid, Item_Buscar("Cartera"), 1, 0);
    Inv_DarItem(playerid, Item_Buscar("Movil"), 1, 0);
    Inv_DarItem(playerid, Item_Buscar("Llaves de casa"), 1, 0);
    Inv_DarItem(playerid, Item_Buscar("Bocadillo"), 1, 0);
    Inv_DarItem(playerid, Item_Buscar("Botella de agua"), 1, 0);
    Inv_DarItem(playerid, Item_Buscar("Semilla de marihuana"), 1, 0);

    // Pistola en el inventario, no en la mano: sigue el mismo patron que
    // /guardararma, con la municion guardada en el dato de la ranura.
    Inv_DarItem(playerid, Item_Buscar("Pistola 9mm"), 1, 5);
    return 1;
}

// ---------------------------------------------------------------------
//  PERSISTENCIA
// ---------------------------------------------------------------------
public Inv_Cargar(playerid)
{
    Inv_Limpiar(playerid);
    g_InvCargado[playerid] = false;

    new consulta[160];
    format(consulta, sizeof consulta,
        "SELECT slot, item, cantidad, datos FROM inventario WHERE cuenta_id = %d", Jugador[playerid][jID]);

    mysql_pquery(g_BD, consulta, "AlCargarInventario", "d", playerid);
    return 1;
}

forward AlCargarInventario(playerid);
public AlCargarInventario(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas, slot;
    cache_get_row_count(filas);

    for (new i = 0; i < filas; i++)
    {
        cache_get_value_name_int(i, "slot", slot);
        if (slot < 0 || slot >= MAX_SLOTS_INVENTARIO) continue;

        cache_get_value_name_int(i, "item",     g_Inv[playerid][slot][SLOT_ITEM]);
        cache_get_value_name_int(i, "cantidad", g_Inv[playerid][slot][SLOT_CANTIDAD]);
        cache_get_value_name_int(i, "datos",    g_Inv[playerid][slot][SLOT_DATOS]);

        if (!ItemValido(g_Inv[playerid][slot][SLOT_ITEM])) Inv_VaciarSlot(playerid, slot);
    }

    // Alguna cuenta puede tener el mismo objeto repartido en varias
    // ranuras de cuando su pila maxima era mas baja (p.ej. las semillas
    // de marihuana, antes 10, ahora mucho mas) -- las junta en cuanto
    // entra, sin que el jugador tenga que hacer nada.
    Inv_ConsolidarPilas(playerid);

    g_InvCargado[playerid] = true;
    return 1;
}

// Junta un mismo objeto repartido en varias ranuras en las menos
// posibles, respetando itMaxPila. Se llama al cargar el inventario
// (arriba); el proximo Inv_Guardar ya escribe el resultado consolidado.
stock Inv_ConsolidarPilas(playerid)
{
    for (new a = 0; a < MAX_SLOTS_INVENTARIO; a++)
    {
        new item = g_Inv[playerid][a][SLOT_ITEM];
        if (!ItemValido(item)) continue;
        if (g_Items[item][itMaxPila] <= 1) continue;
        if (g_Inv[playerid][a][SLOT_CANTIDAD] >= g_Items[item][itMaxPila]) continue;

        for (new b = a + 1; b < MAX_SLOTS_INVENTARIO; b++)
        {
            if (g_Inv[playerid][b][SLOT_ITEM] != item) continue;
            if (g_Inv[playerid][b][SLOT_DATOS] != g_Inv[playerid][a][SLOT_DATOS]) continue;

            new hueco = g_Items[item][itMaxPila] - g_Inv[playerid][a][SLOT_CANTIDAD];
            if (hueco <= 0) break;

            new mover = (g_Inv[playerid][b][SLOT_CANTIDAD] < hueco) ? g_Inv[playerid][b][SLOT_CANTIDAD] : hueco;

            g_Inv[playerid][a][SLOT_CANTIDAD] += mover;
            g_Inv[playerid][b][SLOT_CANTIDAD] -= mover;

            if (g_Inv[playerid][b][SLOT_CANTIDAD] <= 0) Inv_VaciarSlot(playerid, b);
        }
    }
    return 1;
}

public Inv_Guardar(playerid)
{
    // Si la carga no llegó a completarse no guardamos: borraríamos todo.
    if (!g_InvCargado[playerid] || Jugador[playerid][jID] == 0) return 0;

    new consulta[128];
    format(consulta, sizeof consulta, "DELETE FROM inventario WHERE cuenta_id = %d", Jugador[playerid][jID]);
    mysql_pquery(g_BD, consulta);

    new valores[1024], trozo[64], usados = 0;
    valores[0] = EOS;

    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        if (!ItemValido(g_Inv[playerid][s][SLOT_ITEM])) continue;

        format(trozo, sizeof trozo, "%s(%d,%d,%d,%d,%d)",
            (usados > 0) ? (",") : (""),
            Jugador[playerid][jID], s,
            g_Inv[playerid][s][SLOT_ITEM],
            g_Inv[playerid][s][SLOT_CANTIDAD],
            g_Inv[playerid][s][SLOT_DATOS]);

        strcat(valores, trozo, sizeof valores);
        usados++;
    }

    if (usados == 0) return 1;

    new insercion[1200];
    format(insercion, sizeof insercion,
        "INSERT INTO inventario (cuenta_id, slot, item, cantidad, datos) VALUES %s", valores);
    mysql_pquery(g_BD, insercion);
    return 1;
}

// ---------------------------------------------------------------------
//  USAR UN OBJETO
// ---------------------------------------------------------------------
stock Inv_Usar(playerid, slot)
{
    if (slot < 0 || slot >= MAX_SLOTS_INVENTARIO) return 0;

    new item = g_Inv[playerid][slot][SLOT_ITEM];
    if (!ItemValido(item))
    {
        Error(playerid, "Esa ranura esta vacia.");
        return 0;
    }

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);

    switch (g_Items[item][itTipo])
    {
        // ------------------------------------------------------- ARMAS
        case ITEM_ARMA:
        {
            if (IsPlayerInAnyVehicle(playerid))
            {
                Error(playerid, "No puedes sacar eso dentro de un vehiculo.");
                return 0;
            }

            GivePlayerWeapon(playerid, WEAPON:g_Items[item][itDato], g_Inv[playerid][slot][SLOT_DATOS]);
            Inv_VaciarSlot(playerid, slot);

            format(mensaje, sizeof mensaje, "* %s saca %s.", nombre, g_Items[item][itNombre]);
            MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

            format(mensaje, sizeof mensaje, "Has empunado tu %s. Usa /guardararma para volver a guardarla.", g_Items[item][itNombre]);
            Info(playerid, mensaje);
        }

        // -------------------------------------------------- CONSUMIBLES
        case ITEM_CONSUMIBLE:
        {
            // Los 30 platos de Gamerol Aventuras: DLC Pesca (51_pesca.inc)
            // no siguen la regla generica de abajo (itDato + tope 100):
            // curan al 100% y dan escudo acumulativo con su propio tope.
            // Se desvia aqui, antes de tocar nada, para no duplicar el
            // vaciado de ranura ni el mensaje.
            if (Pesca_EsPlatoCocinado(item))
            {
                g_Inv[playerid][slot][SLOT_CANTIDAD]--;
                if (g_Inv[playerid][slot][SLOT_CANTIDAD] <= 0) Inv_VaciarSlot(playerid, slot);

                Pesca_ComerPlato(playerid, item);
                return 1;
            }

            new Float:vida;
            GetPlayerHealth(playerid, vida);

            vida += float(g_Items[item][itDato]);
            if (vida > 100.0) vida = 100.0;
            SetPlayerHealth(playerid, vida);

            g_Inv[playerid][slot][SLOT_CANTIDAD]--;
            if (g_Inv[playerid][slot][SLOT_CANTIDAD] <= 0) Inv_VaciarSlot(playerid, slot);

            format(mensaje, sizeof mensaje, "* %s usa %s.", nombre, g_Items[item][itNombre]);
            MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

            Animar(playerid, "FOOD", "EAT_Burger", 4.1, false, false, false, false, 0);

            // Hambre y sed (44_hambre_sed.inc): solo los ITEM_CONSUMIBLE
            // que son comida o bebida de verdad. El paquete de tabaco
            // tambien es ITEM_CONSUMIBLE pero no es ni una cosa ni la
            // otra -- se deja sin tocar a proposito.
            new Float:hsHambre = 0.0, Float:hsSed = 0.0;
            if (strcmp(g_Items[item][itNombre], "Hamburguesa") == 0) hsHambre = 30.0;
            else if (strcmp(g_Items[item][itNombre], "Bocadillo") == 0) hsHambre = 25.0;
            else if (strcmp(g_Items[item][itNombre], "Botella de agua") == 0) hsSed = 25.0;
            else if (strcmp(g_Items[item][itNombre], "Cafe") == 0) hsSed = 15.0;
            else if (strcmp(g_Items[item][itNombre], "Te Helado") == 0) hsSed = 50.0;
            else if (strcmp(g_Items[item][itNombre], "Gaseosa") == 0) hsSed = 45.0;

            if (hsHambre > 0.0 || hsSed > 0.0)
            {
                new hsTexto[64];
                HambreSed_Sumar(playerid, hsHambre, hsSed, hsTexto, sizeof hsTexto);

                format(mensaje, sizeof mensaje, "Has consumido %s.%s", g_Items[item][itNombre], hsTexto);
                Exito(playerid, mensaje);
            }
        }

        // -------------------------------------------------- CONTENEDOR
        case ITEM_CONTENEDOR:
        {
            if (Jugador[playerid][jPesoExtra] > 0.0)
            {
                Error(playerid, "Ya llevas una mochila puesta. Usa /quitarmochila primero.");
                return 0;
            }

            Jugador[playerid][jPesoExtra] = float(g_Items[item][itDato]);
            Inv_VaciarSlot(playerid, slot);

            format(mensaje, sizeof mensaje, "* %s se pone una mochila a la espalda.", nombre);
            MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

            format(mensaje, sizeof mensaje, "Ahora puedes cargar %.1f kg mas.", Jugador[playerid][jPesoExtra]);
            Exito(playerid, mensaje);
        }

        // ---------------------------------------------------- MUNICION
        case ITEM_MUNICION:
        {
            new WEAPON:arma = GetPlayerWeapon(playerid);
            if (arma != WEAPON:g_Items[item][itDato])
            {
                Error(playerid, "No llevas en las manos el arma que usa esa municion.");
                return 0;
            }

            GivePlayerWeapon(playerid, arma, g_Inv[playerid][slot][SLOT_CANTIDAD]);

            format(mensaje, sizeof mensaje, "Has cargado %d balas.", g_Inv[playerid][slot][SLOT_CANTIDAD]);
            Exito(playerid, mensaje);

            Inv_VaciarSlot(playerid, slot);

            format(mensaje, sizeof mensaje, "* %s recarga su arma.", nombre);
            MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
        }

        // ------------------------------------------------------ OBJETO
        default:
        {
            // El chaleco es un objeto normal con un uso especial.
            if (strcmp(g_Items[item][itNombre], "Chaleco antibalas") == 0)
            {
                SetPlayerArmour(playerid, 100.0);
                Inv_VaciarSlot(playerid, slot);

                format(mensaje, sizeof mensaje, "* %s se coloca un chaleco antibalas.", nombre);
                MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
                return 1;
            }

            // El bidon rellena el deposito de un coche TUYO que tengas al
            // lado, sin necesidad de gasolinera. No hace falta estar
            // dentro, basta con estar cerca, pero el motor tiene que
            // estar apagado (igual criterio que /repostar). Toda la
            // logica de vehiculos vive en 13_vehiculos.inc.
            if (strcmp(g_Items[item][itNombre], "Bidon de gasolina") == 0)
            {
                new pctFinal;
                new resultado = Veh_RepostarConBidon(playerid, 5.0, 20.0, pctFinal);

                if (resultado == 0)
                {
                    Error(playerid, "No tienes ningun coche tuyo cerca.");
                    return 0;
                }
                if (resultado == -1)
                {
                    Error(playerid, "Apaga el motor antes de echar el bidon.");
                    return 0;
                }
                if (resultado == -2)
                {
                    Error(playerid, "El deposito ya esta lleno.");
                    return 0;
                }

                Inv_VaciarSlot(playerid, slot);

                format(mensaje, sizeof mensaje, "* %s vacia un bidon de gasolina en el deposito de su coche.", nombre);
                MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

                format(mensaje, sizeof mensaje, "Deposito al %d%%.", pctFinal);
                Exito(playerid, mensaje);
                return 1;
            }

            format(mensaje, sizeof mensaje, "* %s saca %s y lo mira un momento.", nombre, g_Items[item][itNombre]);
            MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

            format(mensaje, sizeof mensaje, "%s", g_Items[item][itDescripcion]);
            Info(playerid, mensaje);
        }
    }
    return 1;
}

// ---------------------------------------------------------------------
//  TIRAR AL SUELO
// ---------------------------------------------------------------------
stock Inv_Tirar(playerid, slot, cantidad)
{
    new item = g_Inv[playerid][slot][SLOT_ITEM];
    if (!ItemValido(item)) return 0;

    if (cantidad < 1) cantidad = 1;
    if (cantidad > g_Inv[playerid][slot][SLOT_CANTIDAD]) cantidad = g_Inv[playerid][slot][SLOT_CANTIDAD];

    new indice = -1;
    for (new i = 0; i < MAX_OBJETOS_SUELO; i++)
    {
        if (!g_Suelo[i][suUsado]) { indice = i; break; }
    }

    if (indice == -1)
    {
        Error(playerid, "Hay demasiadas cosas tiradas por el suelo ahora mismo.");
        return 0;
    }

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    // Lo deja medio metro por delante del personaje.
    x += 0.6 * floatsin(-a, degrees);
    y += 0.6 * floatcos(-a, degrees);
    z -= 0.9;

    g_Suelo[indice][suUsado]    = true;
    g_Suelo[indice][suItem]     = item;
    g_Suelo[indice][suCantidad] = cantidad;
    g_Suelo[indice][suDatos]    = g_Inv[playerid][slot][SLOT_DATOS];
    g_Suelo[indice][suX]        = x;
    g_Suelo[indice][suY]        = y;
    g_Suelo[indice][suZ]        = z;
    g_Suelo[indice][suInterior] = GetPlayerInterior(playerid);
    g_Suelo[indice][suMundo]    = GetPlayerVirtualWorld(playerid);
    g_Suelo[indice][suSegundos] = 0;

    g_Suelo[indice][suObjeto] = CreateDynamicObject(g_Items[item][itModelo], x, y, z, 0.0, 0.0, 0.0,
        g_Suelo[indice][suMundo], g_Suelo[indice][suInterior], -1, 60.0);

    new etiqueta[96];
    if (cantidad > 1) format(etiqueta, sizeof etiqueta, "%s (x%d)\n{B4B5B7}/coger", g_Items[item][itNombre], cantidad);
    else              format(etiqueta, sizeof etiqueta, "%s\n{B4B5B7}/coger", g_Items[item][itNombre]);

    g_Suelo[indice][suEtiqueta] = CreateDynamic3DTextLabel(etiqueta, COLOR_MARCA, x, y, z + 0.4, 8.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, g_Suelo[indice][suMundo], g_Suelo[indice][suInterior], -1, 25.0);

    g_Inv[playerid][slot][SLOT_CANTIDAD] -= cantidad;
    if (g_Inv[playerid][slot][SLOT_CANTIDAD] <= 0) Inv_VaciarSlot(playerid, slot);

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    format(mensaje, sizeof mensaje, "* %s deja %s en el suelo.", nombre, g_Items[item][itNombre]);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
    return 1;
}

stock Suelo_Borrar(indice)
{
    if (!g_Suelo[indice][suUsado]) return 0;

    DestroyDynamicObject(g_Suelo[indice][suObjeto]);
    DestroyDynamic3DTextLabel(g_Suelo[indice][suEtiqueta]);

    g_Suelo[indice][suUsado] = false;
    return 1;
}

// Se llama una vez por segundo desde el temporizador global.
stock Suelo_Segundo()
{
    for (new i = 0; i < MAX_OBJETOS_SUELO; i++)
    {
        if (!g_Suelo[i][suUsado]) continue;

        g_Suelo[i][suSegundos]++;
        if (g_Suelo[i][suSegundos] >= SUELO_DURACION) Suelo_Borrar(i);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDOS
// ---------------------------------------------------------------------
CMD:inventario(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    Inv_MostrarDialogo(playerid);
    return 1;
}
CMD:inv(playerid, params[]) return cmd_inventario(playerid, params);
CMD:i(playerid, params[]) return cmd_inventario(playerid, params);

stock Inv_MostrarDialogo(playerid)
{
    new lista[2048], linea[160], usados = 0;

    format(lista, sizeof lista, "{E8B33A}Objeto\tCantidad\tPeso\n");

    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        new item = g_Inv[playerid][s][SLOT_ITEM];
        if (!ItemValido(item)) continue;

        new Float:peso = g_Items[item][itPeso] * float(g_Inv[playerid][s][SLOT_CANTIDAD]);

        format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%d\t{B4B5B7}%.2f kg\n",
            g_Items[item][itNombre], g_Inv[playerid][s][SLOT_CANTIDAD], peso);

        strcat(lista, linea, sizeof lista);
        usados++;
    }

    if (usados == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_NADA, DIALOG_STYLE_MSGBOX,
            "{E8B33A}Inventario",
            "{B4B5B7}No llevas nada encima.\n\nLos objetos se consiguen trabajando, comprando o cogiendolos del suelo.",
            "Cerrar", "");
        return 1;
    }

    new titulo[96];
    format(titulo, sizeof titulo, "{E8B33A}Inventario   %.1f / %.1f kg   ~   %d/%d ranuras",
        Inv_PesoActual(playerid), Inv_PesoMaximo(playerid), usados, MAX_SLOTS_INVENTARIO);

    ShowPlayerDialog(playerid, DIALOGO_INVENTARIO, DIALOG_STYLE_TABLIST_HEADERS, titulo, lista, "Elegir", "Cerrar");
    return 1;
}

// Traduce la fila elegida en el diálogo a la ranura real del inventario.
stock Inv_SlotDeFila(playerid, fila)
{
    new contador = 0;
    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        if (!ItemValido(g_Inv[playerid][s][SLOT_ITEM])) continue;
        if (contador == fila) return s;
        contador++;
    }
    return -1;
}

// g_Inv es "static" (privado de este archivo). Estos tres accesores son
// la puerta de entrada para que otros modulos (23_panel.inc, 24_casas.inc)
// puedan leer una ranura sin tocar el array directamente.
stock Inv_ItemEnSlot(playerid, slot)
{
    if (slot < 0 || slot >= MAX_SLOTS_INVENTARIO) return 0;
    return g_Inv[playerid][slot][SLOT_ITEM];
}

stock Inv_CantidadEnSlot(playerid, slot)
{
    if (slot < 0 || slot >= MAX_SLOTS_INVENTARIO) return 0;
    return g_Inv[playerid][slot][SLOT_CANTIDAD];
}

stock Inv_DatosEnSlot(playerid, slot)
{
    if (slot < 0 || slot >= MAX_SLOTS_INVENTARIO) return 0;
    return g_Inv[playerid][slot][SLOT_DATOS];
}

CMD:usar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/usar [nombre del objeto]");
        return 1;
    }

    new item = Item_Buscar(params);
    if (item == -1) { Error(playerid, "Ese nombre encaja con varios objetos. Se mas concreto."); return 1; }
    if (item == 0)  { Error(playerid, "No existe ningun objeto con ese nombre."); return 1; }

    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        if (g_Inv[playerid][s][SLOT_ITEM] == item) return Inv_Usar(playerid, s);
    }

    Error(playerid, "No llevas eso encima.");
    return 1;
}

CMD:tirar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new nombreItem[32], cantidad;
    if (sscanf(params, "s[32]D(1)", nombreItem, cantidad))
    {
        Uso(playerid, "/tirar [nombre del objeto] [cantidad]");
        return 1;
    }

    new item = Item_Buscar(nombreItem);
    if (item == -1) { Error(playerid, "Ese nombre encaja con varios objetos."); return 1; }
    if (item == 0)  { Error(playerid, "No existe ningun objeto con ese nombre."); return 1; }

    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        if (g_Inv[playerid][s][SLOT_ITEM] == item) return Inv_Tirar(playerid, s, cantidad);
    }

    Error(playerid, "No llevas eso encima.");
    return 1;
}

CMD:coger(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new mejor = -1, Float:mejorDistancia = 2.5, Float:distancia;

    for (new i = 0; i < MAX_OBJETOS_SUELO; i++)
    {
        if (!g_Suelo[i][suUsado]) continue;
        if (g_Suelo[i][suInterior] != GetPlayerInterior(playerid)) continue;
        if (g_Suelo[i][suMundo] != GetPlayerVirtualWorld(playerid)) continue;

        distancia = GetPlayerDistanceFromPoint(playerid, g_Suelo[i][suX], g_Suelo[i][suY], g_Suelo[i][suZ]);
        if (distancia < mejorDistancia)
        {
            mejorDistancia = distancia;
            mejor = i;
        }
    }

    if (mejor == -1)
    {
        Error(playerid, "No hay nada que coger a tu alrededor.");
        return 1;
    }

    new resultado = Inv_DarItem(playerid, g_Suelo[mejor][suItem], g_Suelo[mejor][suCantidad], g_Suelo[mejor][suDatos]);

    if (resultado == -1) { Error(playerid, "Pesa demasiado para lo que ya llevas encima."); return 1; }
    if (resultado == 0)  { Error(playerid, "No te queda ninguna ranura libre en el inventario."); return 1; }

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);

    format(mensaje, sizeof mensaje, "* %s recoge %s del suelo.", nombre, g_Items[g_Suelo[mejor][suItem]][itNombre]);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

    format(mensaje, sizeof mensaje, "Has cogido %s (x%d).", g_Items[g_Suelo[mejor][suItem]][itNombre], g_Suelo[mejor][suCantidad]);
    Exito(playerid, mensaje);

    Animar(playerid, "BD_FIRE", "wash_up", 4.0, false, false, false, false, 0);
    Suelo_Borrar(mejor);
    return 1;
}

CMD:dar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new objetivoTexto[24], nombreItem[32], cantidad;
    if (sscanf(params, "s[24]s[32]D(1)", objetivoTexto, nombreItem, cantidad))
    {
        Uso(playerid, "/dar [id o nombre] [objeto] [cantidad]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (objetivo == playerid) { Error(playerid, "No puedes darte cosas a ti mismo."); return 1; }
    if (!EstaCerca(playerid, objetivo, RADIO_INTERACCION))
    {
        Error(playerid, "Tienes que estar al lado de esa persona.");
        return 1;
    }

    new item = Item_Buscar(nombreItem);
    if (item == -1) { Error(playerid, "Ese nombre encaja con varios objetos."); return 1; }
    if (item == 0)  { Error(playerid, "No existe ningun objeto con ese nombre."); return 1; }

    new slot = -1;
    for (new s = 0; s < MAX_SLOTS_INVENTARIO; s++)
    {
        if (g_Inv[playerid][s][SLOT_ITEM] == item) { slot = s; break; }
    }

    if (slot == -1) { Error(playerid, "No llevas eso encima."); return 1; }
    if (cantidad < 1) cantidad = 1;
    if (cantidad > g_Inv[playerid][slot][SLOT_CANTIDAD]) cantidad = g_Inv[playerid][slot][SLOT_CANTIDAD];

    new resultado = Inv_DarItem(objetivo, item, cantidad, g_Inv[playerid][slot][SLOT_DATOS]);
    if (resultado == -1) { Error(playerid, "Esa persona no puede cargar con tanto peso."); return 1; }
    if (resultado == 0)  { Error(playerid, "Esa persona no tiene ranuras libres."); return 1; }

    g_Inv[playerid][slot][SLOT_CANTIDAD] -= cantidad;
    if (g_Inv[playerid][slot][SLOT_CANTIDAD] <= 0) Inv_VaciarSlot(playerid, slot);

    new mio[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, mio, sizeof mio);
    NombreRP(objetivo, suyo, sizeof suyo);

    format(mensaje, sizeof mensaje, "Le has dado %s (x%d) a %s.", g_Items[item][itNombre], cantidad, suyo);
    Exito(playerid, mensaje);

    format(mensaje, sizeof mensaje, "%s te ha dado %s (x%d).", mio, g_Items[item][itNombre], cantidad);
    Exito(objetivo, mensaje);

    format(mensaje, sizeof mensaje, "* %s le entrega algo a %s.", mio, suyo);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
    return 1;
}

CMD:guardararma(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new WEAPON:arma = GetPlayerWeapon(playerid);
    if (arma == WEAPON_FIST)
    {
        Error(playerid, "No llevas ningun arma en las manos.");
        return 1;
    }

    new item = Item_BuscarPorArma(arma);
    if (item == 0)
    {
        Error(playerid, "Esa arma no se puede guardar en el inventario.");
        return 1;
    }

    new municion = GetPlayerAmmo(playerid);
    new resultado = Inv_DarItem(playerid, item, 1, municion);

    if (resultado == -1) { Error(playerid, "No puedes cargar con mas peso."); return 1; }
    if (resultado == 0)  { Error(playerid, "No te queda ninguna ranura libre."); return 1; }

    RemovePlayerWeapon(playerid, arma);

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);
    format(mensaje, sizeof mensaje, "* %s guarda %s.", nombre, g_Items[item][itNombre]);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

    Exito(playerid, "Arma guardada en el inventario.");
    return 1;
}

CMD:quitarmochila(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (Jugador[playerid][jPesoExtra] <= 0.0)
    {
        Error(playerid, "No llevas ninguna mochila puesta.");
        return 1;
    }

    new item = Item_Buscar("Mochila");
    Jugador[playerid][jPesoExtra] = 0.0;

    if (Inv_PesoActual(playerid) > Inv_PesoMaximo(playerid))
    {
        Jugador[playerid][jPesoExtra] = float(g_Items[item][itDato]);
        Error(playerid, "Llevas demasiado peso: suelta algo antes de quitarte la mochila.");
        return 1;
    }

    Inv_DarItem(playerid, item, 1, 0);
    Exito(playerid, "Te has quitado la mochila.");
    return 1;
}

CMD:objetos(playerid, params[])
{
    if (NoLogueado(playerid)) return 1;

    new lista[3000], linea[160];
    format(lista, sizeof lista, "{E8B33A}Objeto\tPeso\tTipo\n");

    static const nombresTipo[][] = { "Objeto", "Arma", "Consumible", "Material", "Contenedor", "Municion" };

    for (new i = 1; i < TOTAL_ITEMS; i++)
    {
        format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%.2f kg\t{B4B5B7}%s\n",
            g_Items[i][itNombre], g_Items[i][itPeso], nombresTipo[g_Items[i][itTipo]]);

        if (strlen(lista) + strlen(linea) >= sizeof(lista) - 8) break;
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_NADA, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8B33A}Objetos que existen en el servidor", lista, "Cerrar", "");
    return 1;
}

// ---------------------------------------------------------------------
//  DIÁLOGOS DEL INVENTARIO
// ---------------------------------------------------------------------
stock Inv_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    switch (dialogid)
    {
        case DIALOGO_INVENTARIO:
        {
            if (!response) return 1;

            new slot = Inv_SlotDeFila(playerid, listitem);
            if (slot == -1) return 1;

            Jugador[playerid][jTemporal] = slot;

            new item = g_Inv[playerid][slot][SLOT_ITEM];
            new titulo[96];

            // La descripción va por chat para que el diálogo sólo tenga
            // opciones seleccionables y los índices nunca se desalineen.
            format(g_Buffer, sizeof g_Buffer, "{FFFFFF}%s{8BC4E8}: %s", g_Items[item][itNombre], g_Items[item][itDescripcion]);
            Info(playerid, g_Buffer);

            format(titulo, sizeof titulo, "{E8B33A}%s   ~   x%d   ~   %.2f kg/ud",
                g_Items[item][itNombre], g_Inv[playerid][slot][SLOT_CANTIDAD], g_Items[item][itPeso]);

            ShowPlayerDialog(playerid, DIALOGO_INVENTARIO_OPCIONES, DIALOG_STYLE_LIST, titulo,
                "{FFFFFF}Usar\n\
                {FFFFFF}Tirar al suelo\n\
                {FFFFFF}Dar a alguien\n\
                {B4B5B7}Volver al inventario",
                "Elegir", "Cerrar");
            return 1;
        }

        case DIALOGO_INVENTARIO_OPCIONES:
        {
            if (!response) return 1;

            new slot = Jugador[playerid][jTemporal];
            if (slot < 0 || slot >= MAX_SLOTS_INVENTARIO) return 1;

            switch (listitem)
            {
                case 0: Inv_Usar(playerid, slot);
                case 1:
                {
                    ShowPlayerDialog(playerid, DIALOGO_INVENTARIO_CANTIDAD, DIALOG_STYLE_INPUT,
                        "{E8B33A}Tirar al suelo",
                        "{FFFFFF}Cuantas unidades quieres dejar en el suelo?",
                        "Tirar", "Cancelar");
                }
                case 2:
                {
                    ShowPlayerDialog(playerid, DIALOGO_INVENTARIO_DAR, DIALOG_STYLE_INPUT,
                        "{E8B33A}Dar a alguien",
                        "{FFFFFF}Escribe el ID de la persona y la cantidad.\n\n{B4B5B7}Ejemplo: 5 2",
                        "Dar", "Cancelar");
                }
                case 3: Inv_MostrarDialogo(playerid);
            }
            return 1;
        }

        case DIALOGO_INVENTARIO_CANTIDAD:
        {
            if (!response) return 1;

            new cantidad = strval(inputtext);
            if (!EsNumeroEntero(inputtext) || cantidad < 1)
            {
                Error(playerid, "Escribe un numero valido.");
                return 1;
            }

            Inv_Tirar(playerid, Jugador[playerid][jTemporal], cantidad);
            return 1;
        }

        case DIALOGO_INVENTARIO_DAR:
        {
            if (!response) return 1;

            new destino, cantidad;
            if (sscanf(inputtext, "dD(1)", destino, cantidad))
            {
                Error(playerid, "Formato incorrecto. Escribe el ID y la cantidad.");
                return 1;
            }

            new slot = Jugador[playerid][jTemporal];
            if (slot < 0 || slot >= MAX_SLOTS_INVENTARIO) return 1;
            if (!ItemValido(g_Inv[playerid][slot][SLOT_ITEM])) return 1;

            new completo[80];
            format(completo, sizeof completo, "%d %s %d", destino, g_Items[g_Inv[playerid][slot][SLOT_ITEM]][itNombre], cantidad);
            return cmd_dar(playerid, completo);
        }
    }
    return 0;
}
