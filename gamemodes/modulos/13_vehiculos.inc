/* =====================================================================
 *  13_vehiculos.inc
 *  ---------------------------------------------------------------------
 *  Propiedad de vehiculos, persistencia y control de acceso.
 *
 *  Tipos de vehiculo:
 *    VEH_PERSONAL   comprado por un jugador en el concesionario
 *    VEH_TRABAJO    de un trabajo; solo lo conduce quien tiene ese trabajo
 *    VEH_FACCION    de una faccion; solo lo conduce quien pertenece a ella
 *    VEH_SERVIDOR   de la administracion
 *
 *  Los de trabajo y faccion pueden ademas exigir un rango minimo.
 * ===================================================================== */

#define VEH_PERSONAL            0
#define VEH_TRABAJO             1
#define VEH_FACCION             2
#define VEH_SERVIDOR            3
// Coche del sistema de dropeo (creado con /creardropeo). Vive como libre
// hasta que alguien lo legaliza. Ver modulos/22_dropeo.inc.
#define VEH_DROPEO              4

// Sistema de combustible. Ponlo a 0 si prefieres desactivarlo.
// Rediseno 2026-08-13: se gasta rapido y llenar el deposito es carisimo
// (100 litros a 15$/litro = 1.500$ el deposito lleno).
// Ajuste 2026-08-15: gasto bajado de 3.5 a 2.1875 (3.5 / 1.6) para que
// un deposito lleno dure un 60% mas -- de ~14.3 minutos a ~22.9 minutos
// con el motor en marcha todo el rato.
// Ajuste 2026-09-10 (peticion expresa de Vlad): gasto bajado a 1/3 del
// anterior (2.1875 / 3) para que un deposito lleno dure 3 veces mas --
// de ~22.9 minutos a ~68.6 minutos con el motor en marcha todo el rato.
#define USAR_COMBUSTIBLE        1
#define COMBUSTIBLE_GASTO       0.729166    // Litros por cada 30 segundos en marcha
#define COMBUSTIBLE_PRECIO      15          // Precio por litro
#define GASOLINERA_RADIO        6.0

// Umbrales de color del medidor del HUD. Por encima de AMARILLO va en
// verde, entre ROJO y AMARILLO en amarillo, por debajo de ROJO en rojo
// parpadeante.
#define HUD_GAS_UMBRAL_AMARILLO 40.0
#define HUD_GAS_UMBRAL_ROJO     15.0

// Gasolinera de Legion Square (Los Santos): la unica con carteles
// flotantes en los surtidores, puesta a mano cerca de la comisaria.
#define GASOLINERA_LEGION_X     1944.0000
#define GASOLINERA_LEGION_Y     -1772.9000
#define GASOLINERA_LEGION_Z     13.3906

enum E_VEHICULO
{
    veUsado,
    veID,                                   // vehiculos.id de la base de datos
    veObjeto,                               // vehicleid dentro del juego
    veModelo,
    vePropietario,                          // cuentas.id, 0 = sin dueno
    vePropietarioNombre[MAX_PLAYER_NAME + 1],
    veTipo,
    veTipoID,
    veRangoMinimo,
    veMatricula[12],
    veColor1,
    veColor2,
    Float:veX, Float:veY, Float:veZ, Float:veA,
    veInterior,
    veMundo,
    Float:veCombustible,
    Float:veKilometros,                     // Odometro persistente
    Float:veSalud,
    bool:veCerrado,
    vePrecio,
    bool:vePrecioManual,                    // true = precio puesto a mano con /cambiarprecio: el reciclaje a coche libre NO lo recalcula (22_dropeo.inc)
    veDropeoExpira,                         // unix timestamp, 0 = sin caducidad (solo VEH_DROPEO con dueno)
    bool:veAvisado24h,                      // ya se encolo el aviso WhatsApp de "caduca en 24h" para esta expiracion. Solo en memoria: no se persiste, un reinicio en mitad de la ventana puede reencolarlo una vez mas (ver Gamerol WhatsApp/README.md)
    bool:veEsNFS,                           // marcado con /nfscoche: solo se vende via NFS
    STREAMER_TAG_3D_TEXT_LABEL:veEtiqueta,  // cartel sobre el coche, solo VEH_DROPEO
    bool:veEnVenta,                         // Mercado Libre (34_mercado.inc): congelado mientras esta en un anuncio; anade una linea al cartel de arriba (22_dropeo.inc)
    bool:veTieneRastreador,                 // Rastreador GPS, extra comprado via web (ver Rastreador_Comprar)
    bool:veTieneAlarma,                     // Alarma nativa al puentear, extra comprado via web (ver Alarma_Comprar)
    veFedericoUntil                         // unix timestamp, 0 = no retirado. > gettime() = retirado por Federico (43_federico.inc), sin objeto fisico
}

new g_Vehiculos[MAX_VEHICULOS][E_VEHICULO];
new g_UltimoVehiculo = 0;                   // Ultimo indice usado + 1

// Vehiculos arrancados a la fuerza por un admin con /adminmotor, sin
// comprarlos ni tener derecho normal de uso. Se apagan solos en cuanto
// el admin se baja del vehiculo (ver OnPlayerStateChange, 99_callbacks.inc).
static bool:g_AdminMotorForzado[MAX_VEHICULOS];

// Se pone a true justo antes de meter a un admin a la fuerza con
// /adminlock; Veh_AlConducir la consume (la vuelve a false) para saltarse
// Veh_PuedeUsar UNA sola vez. Sin esto, Veh_AlConducir lo echaria del
// vehiculo al instante igual que a cualquier otro jugador sin llave.
static bool:g_AdminLockOverride[MAX_PLAYERS];

// Gasolineras de San Andreas. Puedes anadir o quitar las que quieras.
//
// Las primeras TOTAL_GASOLINERAS_ORIGINALES son el sumidero de siempre
// (el dinero de /repostar se destruye). Las 3 anadidas el 07/09/2026 al
// FINAL (indices 12-14, "petrolpump nuevo1/2/3") son distintas: su
// repostaje alimenta la caja fuerte de la Gasolinera Los Santos -- ver
// Veh_IndiceGasolinera/Veh_RepostarCompletar, mas abajo. Si anades una
// gasolinera NORMAL en el futuro, hazlo ANTES de estas tres o cambia
// TOTAL_GASOLINERAS_ORIGINALES a la cuenta correcta.
#define TOTAL_GASOLINERAS_ORIGINALES 12
static const Float:g_Gasolineras[][3] =
{
    {1004.0490, -939.3055,  42.1797},       // Idlewood
    {1944.0000, -1772.9000, 13.3906},       // Idlewood
    {2202.2000, -1741.0000, 13.3618},       // El Corona
    {2115.0000, -1780.0000, 13.5547},       // Idlewood sur
    {2639.0000, -1930.0000, 13.3828},       // Willowfield
    { 611.0000, -1701.0000, 14.0000},       // Verona Beach
    {-1609.8000, -2718.0000, 48.5000},      // Angel Pine
    {1380.0000,  462.0000,  20.3000},       // Montgomery
    {-2029.0000, 156.0000,  28.9000},       // Juniper Hollow
    {-2408.0000, 976.0000,  45.4000},       // Doherty
    {2202.0000, 2474.0000,  16.0000},       // Las Venturas
    {2115.0000,  920.0000,  10.8000},       // Redsands
    {PETROLPUMP1_X, PETROLPUMP1_Y, PETROLPUMP1_Z},  // petrolpump nuevo1 (indice 12)
    {PETROLPUMP2_X, PETROLPUMP2_Y, PETROLPUMP2_Z},  // petrolpump nuevo2 (indice 13)
    {PETROLPUMP3_X, PETROLPUMP3_Y, PETROLPUMP3_Z}   // petrolpump nuevo3 (indice 14)
};

// ---------------------------------------------------------------------
//  UTILIDADES
// ---------------------------------------------------------------------
stock bool:VehiculoValido(indice)
{
    return (indice >= 0 && indice < MAX_VEHICULOS && g_Vehiculos[indice][veUsado] != 0);
}

public Veh_BuscarPorObjeto(vehicleid)
{
    if (vehicleid == INVALID_VEHICLE_ID) return -1;

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (g_Vehiculos[i][veUsado] && g_Vehiculos[i][veObjeto] == vehicleid) return i;
    }
    return -1;
}

public Veh_ContarDeJugador(playerid)
{
    if (Jugador[playerid][jID] == 0) return 0;

    new total = 0;
    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_PERSONAL && g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] == Jugador[playerid][jID]) total++;
    }
    return total;
}

// Nombres de todos los vehiculos del jugador, separados por comas, listos
// para meter en un dialogo (g_Vehiculos es de este archivo -- 09_cuenta.inc
// no puede leerlo directo porque se incluye antes). Usado por /cuenta.
stock Veh_ListaTexto(playerid, destino[], tam)
{
    format(destino, tam, "{FFFFFF}");
    new total = 0;

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_PERSONAL && g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] != Jugador[playerid][jID]) continue;

        new modelo[48];
        Veh_NombreModelo(g_Vehiculos[i][veModelo], modelo, sizeof modelo);

        if (total > 0) strcat(destino, "{B4B5B7}, {FFFFFF}", tam);
        strcat(destino, modelo, tam);
        total++;
    }
    if (total == 0) format(destino, tam, "{B4B5B7}Ninguno");
    return total;
}

stock Veh_GenerarMatricula(destino[], tam)
{
    static const letras[] = "BCDFGHJKLMNPRSTVWXYZ";

    format(destino, tam, "JLV %d%d%d%d %c%c",
        random(10), random(10), random(10), random(10),
        letras[random(sizeof(letras) - 1)],
        letras[random(sizeof(letras) - 1)]);
    return 1;
}

// ---------------------------------------------------------------------
//  CATALOGO DE MODELOS  (nombres y precio de referencia de 1992)
//  Vivia en 14_concesionario.inc; se traslado aqui al quitar la tienda
//  fisica (2026-08-12). Lo sigue usando el desguace y el precio base de
//  los coches libres del sistema de dropeo (22_dropeo.inc).
// ---------------------------------------------------------------------
#define CAT_UTILITARIO          0
#define CAT_BERLINA             1
#define CAT_DEPORTIVO           2
#define CAT_LUJO                3
#define CAT_CLASICO             4
#define CAT_TODOTERRENO         5
#define CAT_FURGONETA           6
#define CAT_PICKUP              7
#define CAT_CAMION              8
#define CAT_MOTO                9
#define CAT_BICICLETA           10
#define CAT_INDUSTRIAL          11
#define CAT_COMPETICION         12
#define CAT_NAUTICA             13
#define CAT_AVIACION            14
#define TOTAL_CATEGORIAS        15

enum E_COCHE
{
    coModelo,
    coNombre[28],
    coCategoria,
    coPrecio,
    coPlazas
}

static const g_Catalogo[][E_COCHE] =
{
    // --- Utilitarios y compactos --------------------------------------
    {604, "Glendale destartalado", CAT_UTILITARIO,       800, 4},
    {466, "Glendale",              CAT_UTILITARIO,      2800, 4},
    {516, "Nebula",                CAT_UTILITARIO,      3000, 4},
    {479, "Regina",                CAT_UTILITARIO,      3200, 4},
    {401, "Bravura",               CAT_UTILITARIO,      3200, 2},
    {410, "Manana",                CAT_UTILITARIO,      3500, 2},
    {492, "Greenwood",             CAT_UTILITARIO,      4000, 4},
    {527, "Cadrona",               CAT_UTILITARIO,      4200, 2},
    {404, "Perennial",             CAT_UTILITARIO,      4500, 4},
    {436, "Previon",               CAT_UTILITARIO,      6500, 2},
    {458, "Solair",                CAT_UTILITARIO,      6500, 4},
    {438, "Cabbie",                CAT_UTILITARIO,      8000, 4},
    {589, "Club",                  CAT_UTILITARIO,      8000, 2},
    {496, "Blista Compact",        CAT_UTILITARIO,      9500, 2},
    {565, "Flash",                 CAT_UTILITARIO,      9500, 2},
    {420, "Taxi",                  CAT_UTILITARIO,     12000, 4},
    {602, "Alpha",                 CAT_UTILITARIO,     12000, 2},
    {561, "Stratum",               CAT_UTILITARIO,     12000, 4},

    // --- Berlinas y familiares ----------------------------------------
    {526, "Fortune",               CAT_BERLINA,         7000, 2},
    {517, "Majestic",              CAT_BERLINA,         7500, 2},
    {529, "Willard",               CAT_BERLINA,         9000, 4},
    {540, "Vincent",               CAT_BERLINA,         9500, 4},
    {550, "Sunrise",               CAT_BERLINA,        10500, 4},
    {547, "Primo",                 CAT_BERLINA,        11000, 4},
    {442, "Romero",                CAT_BERLINA,        12000, 2},
    {426, "Premier",               CAT_BERLINA,        13500, 4},
    {551, "Merit",                 CAT_BERLINA,        14000, 4},
    {546, "Intruder",              CAT_BERLINA,        16000, 4},
    {421, "Washington",            CAT_BERLINA,        17000, 4},
    {585, "Emperor",               CAT_BERLINA,        18000, 4},
    {445, "Admiral",               CAT_BERLINA,        30000, 4},
    {405, "Sentinel",              CAT_BERLINA,        38000, 4},

    // --- Deportivos ----------------------------------------------------
    {402, "Buffalo",               CAT_DEPORTIVO,      16000, 2},
    {558, "Uranus",                CAT_DEPORTIVO,      18000, 2},
    {603, "Phoenix",               CAT_DEPORTIVO,      19000, 2},
    {559, "Jester",                CAT_DEPORTIVO,      22000, 2},
    {560, "Sultan",                CAT_DEPORTIVO,      24000, 4},
    {587, "Euros",                 CAT_DEPORTIVO,      26000, 2},
    {562, "Elegy",                 CAT_DEPORTIVO,      28000, 2},
    {477, "ZR-350",                CAT_DEPORTIVO,      32000, 2},
    {555, "Windsor",               CAT_DEPORTIVO,      42000, 2},
    {429, "Banshee",               CAT_DEPORTIVO,      50000, 2},
    {533, "Feltzer",               CAT_DEPORTIVO,      65000, 2},
    {480, "Comet",                 CAT_DEPORTIVO,      75000, 2},
    {506, "Super GT",              CAT_DEPORTIVO,      95000, 2},
    {415, "Cheetah",               CAT_DEPORTIVO,     145000, 2},
    {541, "Bullet",                CAT_DEPORTIVO,     150000, 2},
    {451, "Turismo",               CAT_DEPORTIVO,     180000, 2},
    {411, "Infernus",              CAT_DEPORTIVO,     240000, 2},

    // --- Lujo y representacion ----------------------------------------
    {507, "Elegant",               CAT_LUJO,           32000, 4},
    {409, "Stretch",               CAT_LUJO,           95000, 4},
    {580, "Stafford",              CAT_LUJO,          110000, 4},

    // --- Clasicos y lowriders -----------------------------------------
    {549, "Tampa",                 CAT_CLASICO,         4000, 2},
    {542, "Clover",                CAT_CLASICO,         4500, 2},
    {518, "Buccaneer",             CAT_CLASICO,         5000, 2},
    {419, "Esperanto",             CAT_CLASICO,         5000, 2},
    {576, "Tornado",               CAT_CLASICO,         5500, 2},
    {439, "Stallion",              CAT_CLASICO,         6000, 2},
    {474, "Hermes",                CAT_CLASICO,         6000, 2},
    {467, "Oceanic",               CAT_CLASICO,         6000, 4},
    {491, "Virgo",                 CAT_CLASICO,         6500, 2},
    {566, "Tahoma",                CAT_CLASICO,         6500, 4},
    {575, "Broadway",              CAT_CLASICO,         6800, 2},
    {536, "Blade",                 CAT_CLASICO,         7000, 2},
    {534, "Remington",             CAT_CLASICO,         7500, 2},
    {412, "Voodoo",                CAT_CLASICO,         8000, 2},
    {475, "Sabre",                 CAT_CLASICO,         8500, 2},
    {567, "Savanna",               CAT_CLASICO,         9000, 2},
    {535, "Slamvan",               CAT_CLASICO,         9500, 2},
    {434, "Hotknife",              CAT_CLASICO,        18000, 2},
    {545, "Hustler",               CAT_CLASICO,        22000, 2},

    // --- Todoterreno y 4x4 ---------------------------------------------
    {568, "Bandito",               CAT_TODOTERRENO,    10000, 1},
    {424, "BF Injection",          CAT_TODOTERRENO,    12000, 2},
    {505, "Rancher clasico",       CAT_TODOTERRENO,    13000, 2},
    {500, "Mesa",                  CAT_TODOTERRENO,    14500, 2},
    {489, "Rancher",               CAT_TODOTERRENO,    15000, 2},
    {400, "Landstalker",           CAT_TODOTERRENO,    22000, 4},
    {495, "Sandking",              CAT_TODOTERRENO,    35000, 2},
    {579, "Huntley",               CAT_TODOTERRENO,    45000, 4},
    {470, "Patriot",               CAT_TODOTERRENO,    52000, 4},

    // --- Furgonetas -----------------------------------------------------
    {418, "Moonbeam",              CAT_FURGONETA,       5000, 4},
    {413, "Pony",                  CAT_FURGONETA,       8000, 4},
    {459, "Berkley's RC Van",      CAT_FURGONETA,       9000, 4},
    {440, "Rumpo",                 CAT_FURGONETA,      11000, 4},
    {482, "Burrito",               CAT_FURGONETA,      12000, 4},
    {552, "Utility Van",           CAT_FURGONETA,      16000, 2},
    {483, "Camper",                CAT_FURGONETA,      24000, 4},
    {508, "Journey",               CAT_FURGONETA,      28000, 2},
    {582, "Newsvan",               CAT_FURGONETA,      30000, 2},

    // --- Pick-ups --------------------------------------------------------
    {605, "Sadler destartalado",   CAT_PICKUP,           900, 2},
    {478, "Walton",                CAT_PICKUP,          3500, 2},
    {600, "Picador",               CAT_PICKUP,          5500, 2},
    {554, "Yosemite",              CAT_PICKUP,          6500, 2},
    {422, "Bobcat",                CAT_PICKUP,          8500, 2},
    {543, "Sadler",                CAT_PICKUP,          9000, 2},

    // --- Camiones y autobuses --------------------------------------------
    {573, "Duneride",              CAT_CAMION,          9000, 2},
    {498, "Boxville",              CAT_CAMION,         18000, 2},
    {609, "Boxville de reparto",   CAT_CAMION,         18000, 2},
    {414, "Mule",                  CAT_CAMION,         20000, 2},
    {499, "Benson",                CAT_CAMION,         22000, 2},
    {456, "Yankee",                CAT_CAMION,         25000, 2},
    {578, "DFT-30",                CAT_CAMION,         28000, 2},
    {455, "Flatbed",               CAT_CAMION,         45000, 2},
    {403, "Linerunner",            CAT_CAMION,         65000, 2},
    {515, "Roadtrain",             CAT_CAMION,         70000, 2},
    {514, "Tanker",                CAT_CAMION,         75000, 2},
    {431, "Autobus",               CAT_CAMION,         80000, 8},
    {443, "Packer",                CAT_CAMION,         80000, 2},
    {437, "Autocar Coach",         CAT_CAMION,        120000, 8},

    // --- Motocicletas -----------------------------------------------------
    {462, "Faggio",                CAT_MOTO,            1500, 2},
    {448, "Pizzaboy",              CAT_MOTO,            1800, 1},
    {468, "Sanchez",               CAT_MOTO,            3500, 2},
    {581, "BF-400",                CAT_MOTO,            4000, 2},
    {471, "Quad",                  CAT_MOTO,            4500, 2},
    {461, "PCJ-600",               CAT_MOTO,            6000, 2},
    {521, "FCR-900",               CAT_MOTO,            7500, 2},
    {586, "Wayfarer",              CAT_MOTO,            9000, 2},
    {463, "Freeway",               CAT_MOTO,           11000, 2},
    {522, "NRG-500",               CAT_MOTO,           18000, 2},

    // --- Bicicletas --------------------------------------------------------
    {481, "BMX",                   CAT_BICICLETA,        200, 1},
    {509, "Bicicleta",             CAT_BICICLETA,        350, 1},
    {510, "Bicicleta de montana",  CAT_BICICLETA,        420, 1},

    // --- Industrial y agricola ---------------------------------------------
    {572, "Cortacesped",           CAT_INDUSTRIAL,      1200, 1},
    {457, "Caddy",                 CAT_INDUSTRIAL,      6000, 2},
    {485, "Baggage",               CAT_INDUSTRIAL,     12000, 1},
    {583, "Tug",                   CAT_INDUSTRIAL,     14000, 1},
    {530, "Forklift",              CAT_INDUSTRIAL,     18000, 1},
    {423, "Mr Whoopee",            CAT_INDUSTRIAL,     19000, 2},
    {588, "Hotdog",                CAT_INDUSTRIAL,     21000, 2},
    {574, "Barredora",             CAT_INDUSTRIAL,     30000, 1},
    {525, "Grua",                  CAT_INDUSTRIAL,     32000, 2},
    {428, "Securicar",             CAT_INDUSTRIAL,     55000, 4},
    {524, "Hormigonera",           CAT_INDUSTRIAL,     65000, 2},
    {408, "Trashmaster",           CAT_INDUSTRIAL,     70000, 2},
    {532, "Cosechadora",           CAT_INDUSTRIAL,     85000, 1},
    {486, "Dozer",                 CAT_INDUSTRIAL,     95000, 1},
    {406, "Dumper",                CAT_INDUSTRIAL,    150000, 1},

    // --- Competicion --------------------------------------------------------
    {571, "Kart",                  CAT_COMPETICION,     2500, 1},
    {504, "Bloodring Banger",      CAT_COMPETICION,     5000, 2},
    {556, "Monster A",             CAT_COMPETICION,    60000, 2},
    {557, "Monster B",             CAT_COMPETICION,    60000, 2},
    {444, "Monster",               CAT_COMPETICION,    65000, 2},
    {494, "Hotring Racer",         CAT_COMPETICION,    45000, 2},
    {502, "Hotring Racer A",       CAT_COMPETICION,    45000, 2},
    {503, "Hotring Racer B",       CAT_COMPETICION,    45000, 2},

    // --- Nautica --------------------------------------------------------------
    {473, "Dinghy",                CAT_NAUTICA,         8000, 1},
    {453, "Reefer",                CAT_NAUTICA,        28000, 1},
    {446, "Squalo",                CAT_NAUTICA,        38000, 1},
    {539, "Vortex",                CAT_NAUTICA,        40000, 2},
    {472, "Coastguard",            CAT_NAUTICA,        42000, 1},
    {452, "Speeder",               CAT_NAUTICA,        45000, 1},
    {454, "Tropic",                CAT_NAUTICA,        55000, 1},
    {595, "Launch",                CAT_NAUTICA,        65000, 1},
    {493, "Jetmax",                CAT_NAUTICA,        75000, 1},
    {484, "Marquis",               CAT_NAUTICA,        95000, 1},

    // --- Aviacion ----------------------------------------------------------------
    {593, "Dodo",                  CAT_AVIACION,       65000, 2},
    {460, "Skimmer",               CAT_AVIACION,       85000, 2},
    {512, "Cropduster",            CAT_AVIACION,       95000, 1},
    {513, "Stuntplane",            CAT_AVIACION,      120000, 1},
    {511, "Beagle",                CAT_AVIACION,      180000, 2},
    {469, "Sparrow",               CAT_AVIACION,      280000, 2},
    {487, "Maverick",              CAT_AVIACION,      450000, 4},
    {488, "News Chopper",          CAT_AVIACION,      520000, 2},
    {563, "Raindance",             CAT_AVIACION,      700000, 2},
    {553, "Nevada",                CAT_AVIACION,      850000, 2},
    {417, "Leviathan",             CAT_AVIACION,      950000, 2},
    {548, "Cargobob",              CAT_AVIACION,     1200000, 2},
    {519, "Shamal",                CAT_AVIACION,     2400000, 2},
    {592, "Andromada",             CAT_AVIACION,    28000000, 2},
    {577, "AT-400",                CAT_AVIACION,    35000000, 2}
};
#define TOTAL_CATALOGO          sizeof(g_Catalogo)

public Conce_NombreModelo(modelo, destino[], tam)
{
    for (new i = 0; i < TOTAL_CATALOGO; i++)
    {
        if (g_Catalogo[i][coModelo] != modelo) continue;
        format(destino, tam, "%s", g_Catalogo[i][coNombre]);
        return 1;
    }
    format(destino, tam, "Vehiculo %d", modelo);
    return 0;
}

stock Conce_PrecioModelo(modelo)
{
    for (new i = 0; i < TOTAL_CATALOGO; i++)
    {
        if (g_Catalogo[i][coModelo] == modelo) return g_Catalogo[i][coPrecio];
    }
    return 0;
}

stock Conce_CategoriaModelo(modelo)
{
    for (new i = 0; i < TOTAL_CATALOGO; i++)
    {
        if (g_Catalogo[i][coModelo] == modelo) return g_Catalogo[i][coCategoria];
    }
    return -1;
}

stock Veh_NombreModelo(modelo, destino[], tam)
{
    // Los nombres viven en el catalogo de arriba; si el modelo no esta,
    // se devuelve el numero.
    format(destino, tam, "Modelo %d", modelo);
    return Conce_NombreModelo(modelo, destino, tam);
}

// ---------------------------------------------------------------------
//  CREAR Y BORRAR
// ---------------------------------------------------------------------
stock Veh_ProximoIndice()
{
    for (new i = 0; i < MAX_VEHICULOS; i++)
    {
        if (!g_Vehiculos[i][veUsado]) return i;
    }
    return -1;
}

// ---------------------------------------------------------------------
//  CIERRE DE PUERTAS  ·  SIEMPRE POR JUGADOR
//  ---------------------------------------------------------------------
//  Las puertas NUNCA se cierran de forma global con SetVehicleParamsEx:
//  eso dejaria el vehiculo cerrado tambien para quien si tiene derecho a
//  usarlo (el taxista no podria entrar en su propio taxi).
//
//  En su lugar se cierra jugador por jugador segun lo que diga
//  Veh_PuedeUsar(): abierto para quien tiene el trabajo, la faccion o la
//  llave, y cerrado para el resto.
//
//  IMPORTANTE: cualquier llamada a SetVehicleParamsEx borra el estado
//  por jugador, asi que despues de tocar motor o luces hay que volver a
//  llamar a Veh_ActualizarCierreTodos().
// ---------------------------------------------------------------------
stock Veh_ActualizarCierrePara(playerid, indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (!IsPlayerConnected(playerid)) return 0;

    new bool:puede = Veh_PuedeUsar(playerid, indice);

    SetVehicleParamsForPlayer(g_Vehiculos[indice][veObjeto], playerid,
        VEHICLE_PARAMS_OFF,
        (puede) ? (VEHICLE_PARAMS_OFF) : (VEHICLE_PARAMS_ON));
    return 1;
}

// Un vehiculo, para todos los conectados. Tras crearlo o cambiar su estado.
stock Veh_ActualizarCierreTodos(indice)
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        Veh_ActualizarCierrePara(i, indice);
    }
    return 1;
}

// Todos los vehiculos, para un jugador. Al entrar, al spawnear y cada vez
// que cambia de trabajo o de faccion.
public Veh_ActualizarCierreJugador(playerid)
{
    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        Veh_ActualizarCierrePara(playerid, i);
    }
    return 1;
}

stock Veh_Aparecer(indice)
{
    if (!VehiculoValido(indice)) return 0;

    g_Vehiculos[indice][veObjeto] = CreateVehicle(g_Vehiculos[indice][veModelo],
        g_Vehiculos[indice][veX], g_Vehiculos[indice][veY], g_Vehiculos[indice][veZ],
        g_Vehiculos[indice][veA], g_Vehiculos[indice][veColor1], g_Vehiculos[indice][veColor2], -1);

    LinkVehicleToInterior(g_Vehiculos[indice][veObjeto], g_Vehiculos[indice][veInterior]);
    SetVehicleVirtualWorld(g_Vehiculos[indice][veObjeto], g_Vehiculos[indice][veMundo]);
    SetVehicleHealth(g_Vehiculos[indice][veObjeto], g_Vehiculos[indice][veSalud]);
    SetVehicleNumberPlate(g_Vehiculos[indice][veObjeto], g_Vehiculos[indice][veMatricula]);
    SetVehicleToRespawn(g_Vehiculos[indice][veObjeto]);

    // Puertas abiertas a nivel global: el cierre real se aplica despues
    // jugador por jugador. Ver Veh_ActualizarCierrePara().
    //
    // TODOS los vehiculos nacen con el motor apagado, incluidos los
    // coches libres del sistema de dropeo: para poder arrancarlos hay
    // que comprarlos primero con /comprarcoche (2026-08-12).
    SetVehicleParamsEx(g_Vehiculos[indice][veObjeto], VEHICLE_PARAMS_OFF,
        VEHICLE_PARAMS_OFF, VEHICLE_PARAMS_OFF, VEHICLE_PARAMS_OFF,
        VEHICLE_PARAMS_OFF, VEHICLE_PARAMS_OFF, VEHICLE_PARAMS_OFF);

    if (indice >= g_UltimoVehiculo) g_UltimoVehiculo = indice + 1;

    Veh_ActualizarCierreTodos(indice);

    // Repone las piezas y la pintura guardadas la ultima vez que se
    // tuneo este coche (14_tuning.inc). Async: si el coche es nuevo de
    // verdad (veID todavia a 0) simplemente no encuentra fila.
    Tuning_Aplicar(indice);
    return 1;
}

public Veh_Crear(modelo, propietario, tipo, tipoid, rangomin, Float:x, Float:y, Float:z, Float:a, color1, color2, interior, mundo, precio)
{
    new indice = Veh_ProximoIndice();
    if (indice == -1) return -1;

    // -1 significa "aleatorio" para CreateVehicle, pero si lo guardamos
    // tal cual en la base de datos el coche se repinta solo cada vez que
    // se recrea (cada reinicio del servidor). Tiramos el dado UNA vez
    // aqui y guardamos el resultado real: asi el color es de verdad
    // persistente.
    if (color1 == -1) color1 = random(126);
    if (color2 == -1) color2 = random(126);

    g_Vehiculos[indice][veUsado]        = 1;
    g_Vehiculos[indice][veID]           = 0;
    g_Vehiculos[indice][veModelo]       = modelo;
    g_Vehiculos[indice][vePropietario]  = propietario;
    g_Vehiculos[indice][veTipo]         = tipo;
    g_Vehiculos[indice][veTipoID]       = tipoid;
    g_Vehiculos[indice][veRangoMinimo]  = rangomin;
    g_Vehiculos[indice][veColor1]       = color1;
    g_Vehiculos[indice][veColor2]       = color2;
    g_Vehiculos[indice][veX]            = x;
    g_Vehiculos[indice][veY]            = y;
    g_Vehiculos[indice][veZ]            = z;
    g_Vehiculos[indice][veA]            = a;
    g_Vehiculos[indice][veInterior]     = interior;
    g_Vehiculos[indice][veMundo]        = mundo;
    g_Vehiculos[indice][veCombustible]  = 100.0;
    g_Vehiculos[indice][veSalud]        = 1000.0;
    // Solo los vehiculos personales se cierran con llave. Los de trabajo,
    // faccion y servidor no llevan llave: su acceso lo decide el puesto,
    // asi que nacen "abiertos" y el filtro lo hace Veh_PuedeUsar().
    g_Vehiculos[indice][veCerrado]      = (tipo == VEH_PERSONAL);
    g_Vehiculos[indice][vePrecio]       = precio;
    // Nace con precio automatico; solo /cambiarprecio lo pone a true.
    g_Vehiculos[indice][vePrecioManual] = false;

    Veh_GenerarMatricula(g_Vehiculos[indice][veMatricula], 12);
    Veh_Aparecer(indice);

    // Alta en la base de datos; el id real llega en la respuesta.
    new consulta[768];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO vehiculos (modelo, propietario, tipo, tipo_id, rango_minimo, matricula, \
                                color1, color2, pos_x, pos_y, pos_z, pos_a, interior, mundo, \
                                combustible, salud, cerrado, precio) \
         VALUES (%d, %d, %d, %d, %d, '%e', %d, %d, %f, %f, %f, %f, %d, %d, %f, %f, %d, %d)",
        modelo, propietario, tipo, tipoid, rangomin, g_Vehiculos[indice][veMatricula],
        color1, color2, x, y, z, a, interior, mundo, 100.0, 1000.0,
        _:g_Vehiculos[indice][veCerrado], precio);

    mysql_pquery(g_BD, consulta, "AlCrearVehiculo", "d", indice);
    return indice;
}

forward AlCrearVehiculo(indice);
public AlCrearVehiculo(indice)
{
    if (!VehiculoValido(indice)) return 0;
    g_Vehiculos[indice][veID] = cache_insert_id();
    return 1;
}

stock Veh_Borrar(indice)
{
    if (!VehiculoValido(indice)) return 0;

    if (g_Vehiculos[indice][veID] > 0)
    {
        new consulta[96];
        format(consulta, sizeof consulta, "DELETE FROM vehiculos WHERE id = %d", g_Vehiculos[indice][veID]);
        mysql_pquery(g_BD, consulta);
    }

    DestroyVehicle(g_Vehiculos[indice][veObjeto]);

    // Limpia flags de sesion del sistema de dropeo si los tuviera.
    Dropeo_AlBorrarVehiculo(indice);

    // Borra su tuning guardado, si tenia (14_tuning.inc). Tiene que ir
    // antes de vaciar g_Vehiculos: necesita leer veID.
    Tuning_AlBorrarVehiculo(indice);

    // Limpia el maletero en memoria (45_maletero.inc) -- la fila en la
    // tabla `maletero` ya cae sola por el ON DELETE CASCADE del DELETE
    // de arriba, esto es solo limpieza de RAM.
    Maletero_AlBorrarVehiculo(indice);

    new vacio[E_VEHICULO];
    g_Vehiculos[indice] = vacio;
    return 1;
}

// ---------------------------------------------------------------------
//  CARGA DESDE LA BASE DE DATOS
// ---------------------------------------------------------------------
stock Veh_CargarTodos()
{
    mysql_pquery(g_BD, "SELECT *, COALESCE(UNIX_TIMESTAMP(dropeo_expira), 0) AS expira_unix, COALESCE(UNIX_TIMESTAMP(federico_until), 0) AS federico_unix FROM vehiculos ORDER BY id ASC", "AlCargarVehiculos");
    return 1;
}

forward AlCargarVehiculos();
public AlCargarVehiculos()
{
    new filas;
    cache_get_row_count(filas);

    new cargados = 0, temporal;

    for (new i = 0; i < filas && cargados < MAX_VEHICULOS; i++)
    {
        new indice = cargados;

        g_Vehiculos[indice][veUsado] = 1;

        cache_get_value_name_int(i, "id",           g_Vehiculos[indice][veID]);
        cache_get_value_name_int(i, "modelo",       g_Vehiculos[indice][veModelo]);
        cache_get_value_name_int(i, "propietario",  g_Vehiculos[indice][vePropietario]);
        cache_get_value_name_int(i, "tipo",         g_Vehiculos[indice][veTipo]);
        cache_get_value_name_int(i, "tipo_id",      g_Vehiculos[indice][veTipoID]);
        cache_get_value_name_int(i, "rango_minimo", g_Vehiculos[indice][veRangoMinimo]);
        cache_get_value_name_int(i, "color1",       g_Vehiculos[indice][veColor1]);
        cache_get_value_name_int(i, "color2",       g_Vehiculos[indice][veColor2]);
        cache_get_value_name_int(i, "interior",     g_Vehiculos[indice][veInterior]);
        cache_get_value_name_int(i, "mundo",        g_Vehiculos[indice][veMundo]);
        cache_get_value_name_int(i, "precio",       g_Vehiculos[indice][vePrecio]);

        new precioManual = 0;
        cache_get_value_name_int(i, "precio_manual", precioManual);
        g_Vehiculos[indice][vePrecioManual] = (precioManual != 0);

        cache_get_value_name(i, "matricula", g_Vehiculos[indice][veMatricula], 12);

        cache_get_value_name_float(i, "pos_x",       g_Vehiculos[indice][veX]);
        cache_get_value_name_float(i, "pos_y",       g_Vehiculos[indice][veY]);
        cache_get_value_name_float(i, "pos_z",       g_Vehiculos[indice][veZ]);
        cache_get_value_name_float(i, "pos_a",       g_Vehiculos[indice][veA]);
        cache_get_value_name_float(i, "combustible", g_Vehiculos[indice][veCombustible]);
        // 10/09/2026 (peticion expresa de Vlad): al reiniciar el servidor
        // TODOS los vehiculos aparecen con el deposito lleno, sea cual
        // sea el nivel que tenian guardado -- se sobreescribe aqui mismo,
        // justo despues de leer el valor persistido, solo en la carga de
        // arranque (Veh_Crear ya ponia 100.0 para vehiculos nuevos, esto
        // solo faltaba para los que YA existian). Se persiste solo cuando
        // el propio vehiculo se guarde mas adelante (Veh_Guardar), no aqui.
        g_Vehiculos[indice][veCombustible] = 100.0;
        cache_get_value_name_float(i, "kilometros",  g_Vehiculos[indice][veKilometros]);
        cache_get_value_name_float(i, "salud",       g_Vehiculos[indice][veSalud]);

        cache_get_value_name_int(i, "cerrado", temporal);
        g_Vehiculos[indice][veCerrado] = bool:(temporal != 0);

        cache_get_value_name(i, "propietario_nombre", g_Vehiculos[indice][vePropietarioNombre], MAX_PLAYER_NAME + 1);
        cache_get_value_name_int(i, "expira_unix", g_Vehiculos[indice][veDropeoExpira]);
        cache_get_value_name_int(i, "federico_unix", g_Vehiculos[indice][veFedericoUntil]);

        cache_get_value_name_int(i, "es_nfs", temporal);
        g_Vehiculos[indice][veEsNFS] = bool:(temporal != 0);

        cache_get_value_name_int(i, "en_venta", temporal);
        g_Vehiculos[indice][veEnVenta] = bool:(temporal != 0);

        cache_get_value_name_int(i, "tiene_rastreador", temporal);
        g_Vehiculos[indice][veTieneRastreador] = bool:(temporal != 0);

        cache_get_value_name_int(i, "tiene_alarma", temporal);
        g_Vehiculos[indice][veTieneAlarma] = bool:(temporal != 0);

        g_Vehiculos[indice][veEtiqueta] = STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID;

        if (g_Vehiculos[indice][veSalud] < 300.0) g_Vehiculos[indice][veSalud] = 300.0;

        if (g_Vehiculos[indice][veFedericoUntil] > gettime())
        {
            // Retirado por Federico (43_federico.inc): no se crea el
            // vehiculo fisico ni su cartel hasta que expire el plazo.
            // El registro (posicion, modelo, matricula...) sigue vivo
            // en memoria para poder devolverlo tal cual.
            g_Vehiculos[indice][veObjeto] = INVALID_VEHICLE_ID;
        }
        else
        {
            Veh_Aparecer(indice);
            // Dropeo_CrearCartel ya deja la linea "A LA VENTA" puesta si
            // veEnVenta viene a true de la base de datos (Dropeo_ActualizarCartel
            // la anade sola, ver 22_dropeo.inc) -- no hace falta nada aparte.
            if (g_Vehiculos[indice][veTipo] == VEH_DROPEO) Dropeo_CrearCartel(indice);
        }
        cargados++;
    }

    printf("  [VEHICULOS] %d vehiculos cargados desde la base de datos.", cargados);

    // Federico necesita el pool ya en memoria para contar modelos --
    // se dispara aqui, no antes (43_federico.inc).
    Federico_Iniciar();
    Federico_SecretoIniciar();
    Federico_MotoIniciar();
    Federico_ChatarraIniciar();
    Federico_HistoricoIniciar();
    FedJugadores_Iniciar();

    // Maletero (45_maletero.inc) necesita el pool ya en memoria para
    // mapear vehiculo_id -> indice, mismo motivo que Federico arriba.
    Maletero_CargarTodos();

    return 1;
}

// ---------------------------------------------------------------------
//  GUARDADO
// ---------------------------------------------------------------------
public Veh_Guardar(indice)
{
    if (!VehiculoValido(indice) || g_Vehiculos[indice][veID] == 0) return 0;

    new consulta[650];
    if (g_Vehiculos[indice][veDropeoExpira] > 0)
    {
        mysql_format(g_BD, consulta, sizeof consulta,
            "UPDATE vehiculos SET pos_x = %f, pos_y = %f, pos_z = %f, pos_a = %f, \
                                  interior = %d, mundo = %d, combustible = %f, kilometros = %f, salud = %f, \
                                  cerrado = %d, color1 = %d, color2 = %d, propietario = %d, \
                                  propietario_nombre = '%e', dropeo_expira = FROM_UNIXTIME(%d), es_nfs = %d, \
                                  en_venta = %d \
             WHERE id = %d",
            g_Vehiculos[indice][veX], g_Vehiculos[indice][veY], g_Vehiculos[indice][veZ], g_Vehiculos[indice][veA],
            g_Vehiculos[indice][veInterior], g_Vehiculos[indice][veMundo],
            g_Vehiculos[indice][veCombustible], g_Vehiculos[indice][veKilometros], g_Vehiculos[indice][veSalud],
            _:g_Vehiculos[indice][veCerrado], g_Vehiculos[indice][veColor1], g_Vehiculos[indice][veColor2],
            g_Vehiculos[indice][vePropietario], g_Vehiculos[indice][vePropietarioNombre],
            g_Vehiculos[indice][veDropeoExpira], _:g_Vehiculos[indice][veEsNFS], _:g_Vehiculos[indice][veEnVenta], g_Vehiculos[indice][veID]);
    }
    else
    {
        mysql_format(g_BD, consulta, sizeof consulta,
            "UPDATE vehiculos SET pos_x = %f, pos_y = %f, pos_z = %f, pos_a = %f, \
                                  interior = %d, mundo = %d, combustible = %f, kilometros = %f, salud = %f, \
                                  cerrado = %d, color1 = %d, color2 = %d, propietario = %d, \
                                  propietario_nombre = '%e', dropeo_expira = NULL, es_nfs = %d, \
                                  en_venta = %d \
             WHERE id = %d",
            g_Vehiculos[indice][veX], g_Vehiculos[indice][veY], g_Vehiculos[indice][veZ], g_Vehiculos[indice][veA],
            g_Vehiculos[indice][veInterior], g_Vehiculos[indice][veMundo],
            g_Vehiculos[indice][veCombustible], g_Vehiculos[indice][veKilometros], g_Vehiculos[indice][veSalud],
            _:g_Vehiculos[indice][veCerrado], g_Vehiculos[indice][veColor1], g_Vehiculos[indice][veColor2],
            g_Vehiculos[indice][vePropietario], g_Vehiculos[indice][vePropietarioNombre],
            _:g_Vehiculos[indice][veEsNFS], _:g_Vehiculos[indice][veEnVenta], g_Vehiculos[indice][veID]);
    }

    mysql_pquery(g_BD, consulta);
    return 1;
}

public Veh_GuardarTodos()
{
    new guardados = 0;

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;

        // Si alguien lo esta conduciendo, actualiza la posicion real.
        if (g_Vehiculos[i][veObjeto] != INVALID_VEHICLE_ID)
        {
            new Float:vida;
            GetVehicleHealth(g_Vehiculos[i][veObjeto], vida);
            g_Vehiculos[i][veSalud] = vida;
        }

        Veh_Guardar(i);
        guardados++;
    }
    return guardados;
}

// ---------------------------------------------------------------------
//  Fix vehiculos ardiendo vacios / loop de muerte (12/09/2026)
//  ---------------------------------------------------------------------
//  El motor nativo de SA-MP no avanza el daño de un vehiculo mientras no
//  tiene a nadie dentro: uno que se queda critico/ardiendo cuando el
//  jugador se baja se congela asi, en llamas, para siempre -- hasta que
//  alguien vuelve a subirse y entonces si que revienta, con esa persona
//  dentro (el "loop de muerte"). Estas cuatro funciones se limitan a
//  detectar ese estado y, si el vehiculo esta de verdad vacio, forzar la
//  muerte nativa ya (SetVehicleHealth a 0.0), que dispara OnVehicleDeath
//  de verdad y reutiliza TODO lo que ya hace esa funcion (Teletipo,
//  Veh_Guardar, y el respawn nativo que sigue ya devuelve el vehiculo a
//  veX/veY/veZ/veA -- su /park o su posicion original de ficha, ver
//  OnVehicleSpawn). No se duplica esa logica aqui.
// ---------------------------------------------------------------------

// Veh_Guardar nunca lee el vehiculo real por si solo -- persiste lo que
// ya haya en g_Vehiculos[indice][veSalud]. Esto refresca ese valor desde
// el motor nativo antes de decidir nada o de guardar.
stock Veh_RefrescarSalud(indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (g_Vehiculos[indice][veObjeto] == INVALID_VEHICLE_ID) return 0;

    new Float:vida;
    GetVehicleHealth(g_Vehiculos[indice][veObjeto], vida);
    g_Vehiculos[indice][veSalud] = vida;
    return 1;
}

stock bool:Veh_EsCritico(indice)
{
    if (!VehiculoValido(indice)) return false;
    return (g_Vehiculos[indice][veSalud] <= VEHICULO_SALUD_CRITICA);
}

// true si hay algun jugador conectado dentro de este vehiculo ahora
// mismo, como conductor o como pasajero.
stock bool:Veh_TieneOcupantes(indice)
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        if (Jugador[i][jVehiculoDentro] == indice) return true;
        if (Jugador[i][jVehiculoPasajero] == indice) return true;
    }
    return false;
}

// Si el vehiculo esta de verdad vacio Y critico, lo resuelve ya en vez de
// dejarlo ardiendo congelado a la espera de la proxima victima.
stock Veh_ResolverCriticoVacio(indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (g_Vehiculos[indice][veObjeto] == INVALID_VEHICLE_ID) return 0;
    if (Veh_TieneOcupantes(indice)) return 0;
    if (!Veh_EsCritico(indice)) return 0;

    SetVehicleHealth(g_Vehiculos[indice][veObjeto], 0.0);
    return 1;
}

// Guarda la posicion actual de un vehiculo como su sitio de aparcamiento.
stock Veh_Aparcar(indice)
{
    if (!VehiculoValido(indice)) return 0;

    new Float:x, Float:y, Float:z, Float:a;
    GetVehiclePos(g_Vehiculos[indice][veObjeto], x, y, z);
    GetVehicleZAngle(g_Vehiculos[indice][veObjeto], a);

    g_Vehiculos[indice][veX] = x;
    g_Vehiculos[indice][veY] = y;
    g_Vehiculos[indice][veZ] = z;
    g_Vehiculos[indice][veA] = a;

    Veh_Guardar(indice);
    return 1;
}

// ---------------------------------------------------------------------
//  ELEGIR "EL VEHICULO DE AQUI"
//  ---------------------------------------------------------------------
//  El que llevas montado si vas al volante; si no, el registrado mas
//  cercano dentro del radio. Devuelve -1 si no hay ninguno.
//
//  Existe porque este mismo bucle estaba copiado en cuatro comandos de
//  admin (/adminlock, /adminpark, /borrardropeo, /cambiarprecio) y el
//  quinto iba a ser /borrarvehiculo. Con `tipo` a -1 vale cualquier
//  vehiculo; con un VEH_* concreto, solo los de ese tipo.
//
//  Poder usarlo DE PIE no es comodidad: un coche de faccion echa del
//  asiento a quien no sea de esa faccion (Veh_PuedeUsar), asi que un
//  admin que no sea policia no puede montarse en un coche patrulla sin
//  pasar antes por /adminlock. De pie no hace falta ese rodeo.
// ---------------------------------------------------------------------
stock Veh_BuscarCercano(playerid, Float:radio, tipo = -1)
{
    if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
    {
        new montado = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
        if (montado != -1 && (tipo == -1 || g_Vehiculos[montado][veTipo] == tipo)) return montado;
    }

    new indice = -1, Float:mejor = radio, Float:vx, Float:vy, Float:vz, Float:distancia;

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (tipo != -1 && g_Vehiculos[i][veTipo] != tipo) continue;

        GetVehiclePos(g_Vehiculos[i][veObjeto], vx, vy, vz);
        distancia = GetPlayerDistanceFromPoint(playerid, vx, vy, vz);

        if (distancia < mejor) { mejor = distancia; indice = i; }
    }
    return indice;
}

// Nombre legible del tipo de un vehiculo, para los mensajes de admin:
// borrar un coche patrulla y borrar el coche de un jugador no son lo
// mismo, y conviene que se lea antes de darle a la tecla.
stock Veh_NombreTipo(tipo, destino[], tam)
{
    switch (tipo)
    {
        case VEH_PERSONAL: format(destino, tam, "personal");
        case VEH_TRABAJO:  format(destino, tam, "de trabajo");
        case VEH_FACCION:  format(destino, tam, "de faccion");
        case VEH_SERVIDOR: format(destino, tam, "del servidor");
        case VEH_DROPEO:   format(destino, tam, "libre");
        default:           format(destino, tam, "desconocido");
    }
    return 1;
}

// ---------------------------------------------------------------------
//  CONTROL DE ACCESO
//  Esta es la funcion que hace cumplir la norma de los vehiculos de
//  trabajo y de faccion.
// ---------------------------------------------------------------------
public bool:Veh_PuedeUsar(playerid, indice)
{
    if (!VehiculoValido(indice)) return true;

    // Mercado Libre (34_mercado.inc): congelado para TODOS, incluido el
    // dueno, mientras esta publicado en un anuncio. Se comprueba antes
    // que ninguna otra regla (un admin con /adminlock si puede saltarselo,
    // igual que con cualquier otro coche cerrado -- ver Veh_AlConducir).
    if (g_Vehiculos[indice][veEnVenta]) return false;

    // Los admins juegan con las mismas reglas que cualquiera: si quieren
    // entrar en un vehiculo cerrado o de otro, usan /adminlock. Este
    // filtro ya NO da barra libre por rango (2026-08-12).

    switch (g_Vehiculos[indice][veTipo])
    {
        case VEH_PERSONAL:
        {
            // Cerrado de verdad: NADIE entra, ni siquiera el dueno, hasta
            // que alguien con llave (el dueno) vuelva a hacer /lock desde
            // fuera para abrirlo. Si no esta cerrado, cualquiera puede
            // llevarselo: robar coches tambien es rol.
            return !g_Vehiculos[indice][veCerrado];
        }

        case VEH_TRABAJO:
        {
            if (!Trabajo_Tiene(playerid, g_Vehiculos[indice][veTipoID])) return false;
            return (Trabajo_RangoDe(playerid, g_Vehiculos[indice][veTipoID]) >= g_Vehiculos[indice][veRangoMinimo]);
        }

        case VEH_FACCION:
        {
            if (Jugador[playerid][jFaccion] != g_Vehiculos[indice][veTipoID]) return false;
            return (Jugador[playerid][jFaccionRango] >= g_Vehiculos[indice][veRangoMinimo]);
        }

        case VEH_SERVIDOR:
        {
            return EsAdmin(playerid, ADMIN_MODERADOR);
        }

        case VEH_DROPEO:
        {
            // Sin dueno: siempre abierto para todos, es de la calle.
            // Con dueno: igual que VEH_PERSONAL, cerrado de verdad 
            // nadie entra hasta que el dueno lo abra con /lock desde fuera.
            if (g_Vehiculos[indice][vePropietario] == 0) return true;
            return !g_Vehiculos[indice][veCerrado];
        }
    }
    return true;
}

// Explica al jugador por que no puede conducirlo.
stock Veh_ExplicarRechazo(playerid, indice)
{
    new nombre[48];

    if (g_Vehiculos[indice][veEnVenta])
    {
        Error(playerid, "Este vehiculo esta publicado en el Mercado Libre. No se puede usar hasta que se venda, se cancele el anuncio o caduque.");
        return 1;
    }

    switch (g_Vehiculos[indice][veTipo])
    {
        case VEH_PERSONAL:
        {
            Error(playerid, "Este vehiculo esta cerrado con llave.");
        }
        case VEH_TRABAJO:
        {
            Trabajo_Nombre(g_Vehiculos[indice][veTipoID], nombre, sizeof nombre);
            format(g_Buffer, sizeof g_Buffer, "Este vehiculo es de %s. Necesitas ese trabajo para usarlo.", nombre);
            Error(playerid, g_Buffer);
        }
        case VEH_FACCION:
        {
            Faccion_Nombre(g_Vehiculos[indice][veTipoID], nombre, sizeof nombre);
            format(g_Buffer, sizeof g_Buffer, "Este vehiculo pertenece a %s. No eres miembro con rango suficiente.", nombre);
            Error(playerid, g_Buffer);
        }
        case VEH_SERVIDOR:
        {
            Error(playerid, "Este vehiculo es de la administracion del servidor.");
        }
        case VEH_DROPEO:
        {
            Error(playerid, "Este coche esta cerrado con llave. No es tuyo.");
        }
    }
    return 1;
}

// Se llama desde OnPlayerStateChange cuando alguien se pone al volante.
stock Veh_AlConducir(playerid, vehicleid)
{
    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (indice == -1) return 1;

    Jugador[playerid][jVehiculoDentro] = indice;

    // Fix vehiculos ardiendo vacios (12/09/2026): si este vehiculo se
    // quedo critico/ardiendo mientras estaba vacio, se saca al jugador
    // antes de que le pase nada y se resuelve el vehiculo de verdad (ver
    // Veh_ResolverCriticoVacio, mas arriba en este archivo). Va antes de
    // Veh_PuedeUsar a proposito: da igual si el coche es suyo, si esta
    // ardiendo no se entra.
    Veh_RefrescarSalud(indice);
    if (Veh_EsCritico(indice))
    {
        RemovePlayerFromVehicle(playerid);
        Jugador[playerid][jVehiculoDentro] = -1;
        Aviso(playerid, "Ese vehiculo esta demasiado dañado, se ha terminado de estropear. Prueba con otro.");
        Veh_ResolverCriticoVacio(indice);
        return 0;
    }

    if (g_AdminLockOverride[playerid])
    {
        // /adminlock ya autorizo esta entrada en concreto; se consume
        // aqui para que no quede colgado y afecte a la siguiente vez
        // que el admin entre en un coche por las buenas.
        g_AdminLockOverride[playerid] = false;
    }
    else if (!Veh_PuedeUsar(playerid, indice))
    {
        Veh_ExplicarRechazo(playerid, indice);
        RemovePlayerFromVehicle(playerid);
        Jugador[playerid][jVehiculoDentro] = -1;
        return 0;
    }

    // Los coches del sistema de dropeo (VEH_DROPEO) ya llevan toda esta
    // informacion en el cartel flotante de encima del techo (estado,
    // propietario, precio...), asi que aqui no se repite por chat.
    if (g_Vehiculos[indice][veTipo] == VEH_DROPEO) return 1;

    new duenoTexto[64];

    if (g_Vehiculos[indice][veTipo] == VEH_PERSONAL && g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID])
    {
        format(duenoTexto, sizeof duenoTexto, "tuyo");
    }
    else format(duenoTexto, sizeof duenoTexto, "de otra persona");

    #if USAR_COMBUSTIBLE == 1
        format(g_Buffer, sizeof g_Buffer, "Matricula %s   ~   Deposito %.0f%%   ~   Vehiculo %s   ~   /motor",
            g_Vehiculos[indice][veMatricula], g_Vehiculos[indice][veCombustible], duenoTexto);
    #else
        format(g_Buffer, sizeof g_Buffer, "Matricula %s   ~   Vehiculo %s   ~   /motor",
            g_Vehiculos[indice][veMatricula], duenoTexto);
    #endif

    Info(playerid, g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  COMBUSTIBLE
// ---------------------------------------------------------------------
stock Veh_ConsumirCombustible()
{
    #if USAR_COMBUSTIBLE == 0
        return 0;
    #else

    new motor, luces, alarma, puertas, capo, maletero, objetivo;

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veCombustible] <= 0.0) continue;

        GetVehicleParamsEx(g_Vehiculos[i][veObjeto], motor, luces, alarma, puertas, capo, maletero, objetivo);
        if (motor != VEHICLE_PARAMS_ON) continue;

        g_Vehiculos[i][veCombustible] -= COMBUSTIBLE_GASTO;

        if (g_Vehiculos[i][veCombustible] <= 0.0)
        {
            g_Vehiculos[i][veCombustible] = 0.0;
            SetVehicleParamsEx(g_Vehiculos[i][veObjeto], VEHICLE_PARAMS_OFF, luces, alarma, VEHICLE_PARAMS_OFF, capo, maletero, objetivo);
            Veh_ActualizarCierreTodos(i);

            for (new p = 0; p < MAX_PLAYERS; p++)
            {
                if (!IsPlayerConnected(p) || GetPlayerVehicleID(p) != g_Vehiculos[i][veObjeto]) continue;
                Error(p, "Te has quedado sin gasolina. Busca una gasolinera.");
            }
        }
    }
    return 1;
    #endif
}

// ---------------------------------------------------------------------
//  BIDON DE GASOLINA (usado desde 10_inventario.inc, item "Bidon de
//  gasolina"). Todo el acceso a g_Vehiculos se queda aqui dentro para
//  que el inventario no tenga que conocer el array del modulo.
//  Resultado: 1 repostado, 0 no hay coche tuyo cerca, -1 motor encendido,
//  -2 deposito ya lleno. Si devuelve 1, pctFinal trae el % final.
// ---------------------------------------------------------------------
stock Veh_RepostarConBidon(playerid, Float:radio, Float:litros, &pctFinal)
{
    new indice = -1;
    new Float:mejor = radio, Float:vx, Float:vy, Float:vz, Float:distancia;

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][vePropietario] != Jugador[playerid][jID]) continue;

        GetVehiclePos(g_Vehiculos[i][veObjeto], vx, vy, vz);
        distancia = GetPlayerDistanceFromPoint(playerid, vx, vy, vz);

        if (distancia < mejor) { mejor = distancia; indice = i; }
    }

    if (indice == -1) return 0;

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(g_Vehiculos[indice][veObjeto], motor, luces, alarma, puertas, capo, maletero, objetivo);
    if (motor == VEHICLE_PARAMS_ON) return -1;

    if (g_Vehiculos[indice][veCombustible] >= 100.0) return -2;

    g_Vehiculos[indice][veCombustible] += litros;
    if (g_Vehiculos[indice][veCombustible] > 100.0) g_Vehiculos[indice][veCombustible] = 100.0;
    Veh_Guardar(indice);

    pctFinal = floatround(g_Vehiculos[indice][veCombustible]);
    return 1;
}

// ---------------------------------------------------------------------
//  CARTELES DE LA GASOLINERA DE LEGION SQUARE
//  La unica de las 12 con carteles flotantes en los surtidores. Puesta
//  a mano cerca de la comisaria de Los Santos (2026-08-13). El resto de
//  gasolineras del array siguen funcionando igual, solo que sin cartel.
// ---------------------------------------------------------------------
stock Veh_CrearCartelesGasolinera()
{
    new const Float:offsets[3][2] = {{0.0, 0.0}, {4.5, 1.5}, {-4.5, -1.5}};

    for (new i = 0; i < 3; i++)
    {
        CreateDynamic3DTextLabel(
            "{4FD1C5}Gasolinera Legion Square\n{FFFFFF}Usa {E8B33A}/repostar{FFFFFF} para llenar el deposito",
            0x4FD1C5FF,
            GASOLINERA_LEGION_X + offsets[i][0], GASOLINERA_LEGION_Y + offsets[i][1], GASOLINERA_LEGION_Z + 0.9,
            10.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 20.0);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  CARTELES DE LOS 3 PETROLPUMPS NUEVOS (07/09/2026)
//  Mismo texto y estilo que Veh_CrearCartelesGasolinera de arriba, pero
//  uno por coordenada real (no offsets alrededor de un unico punto,
//  estos 3 son posiciones distintas de verdad) y con la marca
//  "Gasolinera Los Santos" porque el dinero va a esa caja fuerte -- ver
//  Veh_RepostarCompletar.
// ---------------------------------------------------------------------
stock Veh_CrearCartelesPetrolpumpsLS()
{
    new const Float:coords[3][3] = {
        {PETROLPUMP1_X, PETROLPUMP1_Y, PETROLPUMP1_Z},
        {PETROLPUMP2_X, PETROLPUMP2_Y, PETROLPUMP2_Z},
        {PETROLPUMP3_X, PETROLPUMP3_Y, PETROLPUMP3_Z}
    };

    for (new i = 0; i < 3; i++)
    {
        CreateDynamic3DTextLabel(
            "{4FD1C5}Gasolinera Los Santos\n{FFFFFF}Usa {E8B33A}/repostar{FFFFFF} para llenar el deposito",
            0x4FD1C5FF, coords[i][0], coords[i][1], coords[i][2] + 0.9,
            10.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 20.0);
    }
    return 1;
}

// Icono de minimapa quitado el 11/09/2026 -- peticion expresa de Vlad
// de limpiar todos los iconos del mapa (la gente usa /gps). Se deja el
// stock vacio para no tocar el call site en OnPlayerConnect.
stock Veh_OnConectarPetrolpumpsLS(playerid)
{
    #pragma unused playerid
    return 1;
}

// ---------------------------------------------------------------------
//  WIDGET DE CONDUCCION  (velocimetro + salud + gasolina)   v2 - 18/09/2026
//  Vlad quiere el widget compacto de la referencia: esquina INFERIOR
//  IZQUIERDA, fondo negro, icono coche + NOMBRE MODELO arriba, "00 KM/H"
//  grande a la izquierda, barra horizontal (combustible) a la derecha y
//  fila inferior con corazon-%HP + bidon-L (litros = % de 0 a 100).
//
//  Reutiliza g_Vehiculos[indice][veCombustible] (0-100, sistema unico de
//  gasolina), GetVehicleHealth (0-1000 -> 0-100), Conce_NombreModelo, y
//  el tic Veh_HUD_Segundo llamado desde 99_callbacks.inc. Solo cambia la
//  presentacion; ninguna logica economica ni de guardado tocada.
//
//  Se retira tambien la URL "Gamerol" que aparecia encima
//  (GPS_MostrarURL/GPS_OcultarURL, 30_gps.inc); el widget nuevo no la
//  necesita porque va abajo a la izquierda, lejos del minimapa.
// ---------------------------------------------------------------------
static Text:g_HUDFondo;                                     // caja negra global (todos los conductores)
static Text:g_HUDBarraFondo;                                // barra de gasolina - carril gris
static PlayerText:g_HUDIcono[MAX_PLAYERS];                  // mini-preview del vehiculo (icono)
static PlayerText:g_HUDNombre[MAX_PLAYERS];                 // TRASHMASTER
static PlayerText:g_HUDVelocidad[MAX_PLAYERS];              // 00
static PlayerText:g_HUDUnidad[MAX_PLAYERS];                 // KM/H
static PlayerText:g_HUDBarraFill[MAX_PLAYERS];              // barra amarilla que crece con la gasolina
static PlayerText:g_HUDDatos[MAX_PLAYERS];                  // "~r~<3~w~ 100%   ~y~L~w~ 100L"
static bool:g_HUDVisible[MAX_PLAYERS];
static g_HUDModeloMostrado[MAX_PLAYERS];                    // modelo actualmente pintado en el icono (evita refrescos innecesarios)

// Aviso en pantalla de /repostar -- 07/09/2026, peticion expresa de Vlad
// para quitar el texto de chat y usar el MISMO estilo que /arreglarcoche
// (dos PlayerTextDraw, Pricedown, se ocultan solas con un timer). Aqui el
// subtitulo es dinamico (el coste varia con los litros), a diferencia
// del de arreglarcoche que es fijo.
enum { RTD_TITULO, RTD_SUBTITULO, RTD_TOTAL }
static PlayerText:g_RepostarTD[MAX_PLAYERS][RTD_TOTAL];
#define REPOSTAR_AVISO_MS 4000

stock Veh_HUD_Iniciar()
{
    // Fondo negro compacto de todo el widget. Esquina INFERIOR DERECHA
    // en resolucion 640x480 (SA:MP escala solo).  Ancho ~210, alto ~56,
    // pegado al borde derecho igual que el widget antiguo (Vlad: "estaba
    // en la parte derecha de la pantalla", correccion 18/09).
    g_HUDFondo = TextDrawCreate(418.0, 396.0, "_");
    TextDrawUseBox(g_HUDFondo, true);
    TextDrawBoxColour(g_HUDFondo, 0x1A1D25E6);
    TextDrawTextSize(g_HUDFondo, 628.0, 56.0);
    TextDrawLetterSize(g_HUDFondo, 0.0, 5.6);
    TextDrawSetShadow(g_HUDFondo, 0);
    TextDrawSetOutline(g_HUDFondo, 0);

    // Barra de gasolina - carril gris fijo. La barra "fill" amarilla es
    // por jugador porque cambia con su combustible.
    g_HUDBarraFondo = TextDrawCreate(478.0, 420.0, "_");
    TextDrawUseBox(g_HUDBarraFondo, true);
    TextDrawBoxColour(g_HUDBarraFondo, 0x3A3D46FF);
    TextDrawTextSize(g_HUDBarraFondo, 621.0, 8.0);
    TextDrawLetterSize(g_HUDBarraFondo, 0.0, 0.6);
    TextDrawSetShadow(g_HUDBarraFondo, 0);
    TextDrawSetOutline(g_HUDBarraFondo, 0);
    return 1;
}

stock Veh_HUD_Crear(playerid)
{
    g_HUDVisible[playerid] = false;
    g_HUDModeloMostrado[playerid] = 0;

    // Icono - preview 3D del modelo del vehiculo, arriba a la izq del
    // nombre (como el iconito coche de la referencia). Se ajusta en
    // Veh_HUD_Segundo cuando cambia el modelo. Coordenadas del widget
    // pegado al borde DERECHO (418 -> 628).
    g_HUDIcono[playerid] = CreatePlayerTextDraw(playerid, 423.0, 398.0, "_");
    PlayerTextDrawFont(playerid, g_HUDIcono[playerid], TEXT_DRAW_FONT_MODEL_PREVIEW);
    PlayerTextDrawTextSize(playerid, g_HUDIcono[playerid], 20.0, 14.0);
    PlayerTextDrawSetPreviewModel(playerid, g_HUDIcono[playerid], 411);   // Infernus como placeholder
    PlayerTextDrawSetPreviewRot(playerid, g_HUDIcono[playerid], 0.0, 0.0, -60.0, 0.7);
    PlayerTextDrawBackgroundColour(playerid, g_HUDIcono[playerid], 0x00000000);

    // Nombre del modelo en MAYUSCULAS (TRASHMASTER, SULTAN, INFERNUS...)
    g_HUDNombre[playerid] = CreatePlayerTextDraw(playerid, 448.0, 400.0, "VEHICULO");
    PlayerTextDrawFont(playerid, g_HUDNombre[playerid], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_HUDNombre[playerid], 0.24, 1.10);
    PlayerTextDrawColour(playerid, g_HUDNombre[playerid], 0xFFFFFFFF);
    PlayerTextDrawSetShadow(playerid, g_HUDNombre[playerid], 0);
    PlayerTextDrawSetOutline(playerid, g_HUDNombre[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_HUDNombre[playerid], true);

    // Numero grande de velocidad (dos-tres digitos).
    g_HUDVelocidad[playerid] = CreatePlayerTextDraw(playerid, 423.0, 415.0, "00");
    PlayerTextDrawFont(playerid, g_HUDVelocidad[playerid], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_HUDVelocidad[playerid], 0.55, 2.10);
    PlayerTextDrawColour(playerid, g_HUDVelocidad[playerid], 0xFFFFFFFF);
    PlayerTextDrawSetShadow(playerid, g_HUDVelocidad[playerid], 0);
    PlayerTextDrawSetOutline(playerid, g_HUDVelocidad[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_HUDVelocidad[playerid], true);

    // Etiqueta KM/H debajo del numero.
    g_HUDUnidad[playerid] = CreatePlayerTextDraw(playerid, 423.0, 439.0, "KM/H");
    PlayerTextDrawFont(playerid, g_HUDUnidad[playerid], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_HUDUnidad[playerid], 0.20, 0.85);
    PlayerTextDrawColour(playerid, g_HUDUnidad[playerid], 0xFFFFFFFF);
    PlayerTextDrawSetShadow(playerid, g_HUDUnidad[playerid], 0);
    PlayerTextDrawSetOutline(playerid, g_HUDUnidad[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_HUDUnidad[playerid], true);

    // Barra amarilla que crece con la gasolina. Se estira reajustando
    // TextSize con el % actual (0-100). Empieza en 0 (invisible). Carril:
    // 479 -> 621  (=142 px de recorrido util).
    g_HUDBarraFill[playerid] = CreatePlayerTextDraw(playerid, 479.0, 421.0, "_");
    PlayerTextDrawUseBox(playerid, g_HUDBarraFill[playerid], true);
    PlayerTextDrawBoxColour(playerid, g_HUDBarraFill[playerid], 0xF2C94CFF);
    PlayerTextDrawTextSize(playerid, g_HUDBarraFill[playerid], 480.0, 6.0);   // se recalcula en Segundo
    PlayerTextDrawLetterSize(playerid, g_HUDBarraFill[playerid], 0.0, 0.45);
    PlayerTextDrawSetShadow(playerid, g_HUDBarraFill[playerid], 0);
    PlayerTextDrawSetOutline(playerid, g_HUDBarraFill[playerid], 0);

    // Linea de datos inferior: HP (salud) y GAS (litros). Los iconos
    // corazon/bidon de la referencia no salen en SA:MP porque el pipeline
    // convierte UTF-8 a Windows-1252 y esos glifos no existen en 1252;
    // usamos texto corto coloreado (~r~ rojo salud, ~y~ amarillo gasolina)
    // para que sea inequivoco. Iconos reales requeririan sprites TXD.
    g_HUDDatos[playerid] = CreatePlayerTextDraw(playerid, 478.0, 438.0, "~r~HP~w~ 100%   ~y~GAS~w~ 100L");
    PlayerTextDrawFont(playerid, g_HUDDatos[playerid], TEXT_DRAW_FONT_AHARONI_BOLD);
    PlayerTextDrawLetterSize(playerid, g_HUDDatos[playerid], 0.22, 1.00);
    PlayerTextDrawColour(playerid, g_HUDDatos[playerid], 0xFFFFFFFF);
    PlayerTextDrawSetShadow(playerid, g_HUDDatos[playerid], 0);
    PlayerTextDrawSetOutline(playerid, g_HUDDatos[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_HUDDatos[playerid], true);

    g_RepostarTD[playerid][RTD_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 130.0, "Vehiculo repostado.");
    PlayerTextDrawAlignment(playerid, g_RepostarTD[playerid][RTD_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_RepostarTD[playerid][RTD_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_RepostarTD[playerid][RTD_TITULO], 0.48, 2.0);
    PlayerTextDrawColour(playerid, g_RepostarTD[playerid][RTD_TITULO], 0x4FD1C5FF);
    PlayerTextDrawSetOutline(playerid, g_RepostarTD[playerid][RTD_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_RepostarTD[playerid][RTD_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_RepostarTD[playerid][RTD_TITULO], true);

    g_RepostarTD[playerid][RTD_SUBTITULO] = CreatePlayerTextDraw(playerid, 320.0, 158.0, "Fuiste cobrado 0$");
    PlayerTextDrawAlignment(playerid, g_RepostarTD[playerid][RTD_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_RepostarTD[playerid][RTD_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_RepostarTD[playerid][RTD_SUBTITULO], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_RepostarTD[playerid][RTD_SUBTITULO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_RepostarTD[playerid][RTD_SUBTITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_RepostarTD[playerid][RTD_SUBTITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_RepostarTD[playerid][RTD_SUBTITULO], true);
    return 1;
}

// Texto dinamico (el coste cambia con los litros) + sonido 1133
// "BUY_CAR_MOD" -- el mismo que ya usa /arreglarcoche.
stock Veh_RepostarMostrarAviso(playerid, coste)
{
    new dinero[24], texto[32];
    FormatearDinero(coste, dinero, sizeof dinero);
    format(texto, sizeof texto, "Fuiste cobrado %s", dinero);
    PlayerTextDrawSetString(playerid, g_RepostarTD[playerid][RTD_SUBTITULO], texto);

    PlayerTextDrawShow(playerid, g_RepostarTD[playerid][RTD_TITULO]);
    PlayerTextDrawShow(playerid, g_RepostarTD[playerid][RTD_SUBTITULO]);
    PlayerPlaySound(playerid, 1133, 0.0, 0.0, 0.0);
    SetTimerEx("Veh_RepostarOcultarAviso", REPOSTAR_AVISO_MS, false, "d", playerid);
    return 1;
}

forward Veh_RepostarOcultarAviso(playerid);
public Veh_RepostarOcultarAviso(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    PlayerTextDrawHide(playerid, g_RepostarTD[playerid][RTD_TITULO]);
    PlayerTextDrawHide(playerid, g_RepostarTD[playerid][RTD_SUBTITULO]);
    return 1;
}

stock Veh_HUD_Ocultar(playerid)
{
    if (!g_HUDVisible[playerid]) return 0;

    TextDrawHideForPlayer(playerid, g_HUDFondo);
    TextDrawHideForPlayer(playerid, g_HUDBarraFondo);
    PlayerTextDrawHide(playerid, g_HUDIcono[playerid]);
    PlayerTextDrawHide(playerid, g_HUDNombre[playerid]);
    PlayerTextDrawHide(playerid, g_HUDVelocidad[playerid]);
    PlayerTextDrawHide(playerid, g_HUDUnidad[playerid]);
    PlayerTextDrawHide(playerid, g_HUDBarraFill[playerid]);
    PlayerTextDrawHide(playerid, g_HUDDatos[playerid]);
    // STEP 14 UI 18/09: la URL "Gamerol" que pintaba
    // GPS_MostrarURL sobre el HUD YA NO SE MUESTRA. La llamada queda
    // eliminada aqui. El stock sigue existiendo (30_gps.inc) por si algun
    // dia se quiere reactivar; hoy no dibuja nada.
    g_HUDVisible[playerid] = false;
    return 1;
}

// Se llama una vez por segundo (TicSegundo, 99_callbacks.inc). El
// widget nuevo del STEP 14 UI solo necesita este tic: velocidad, salud y
// combustible cambian sin brusquedad para el ojo, y evitamos meter un
// segundo timer por jugador -- la infraestructura ya existente vale.
stock Veh_HUD_Segundo(playerid)
{
    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Veh_HUD_Ocultar(playerid);
        return 0;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (indice == -1)
    {
        Veh_HUD_Ocultar(playerid);
        return 0;
    }

    new modelo = GetVehicleModel(vehicleid);

    if (!g_HUDVisible[playerid])
    {
        TextDrawShowForPlayer(playerid, g_HUDFondo);
        TextDrawShowForPlayer(playerid, g_HUDBarraFondo);
        PlayerTextDrawShow(playerid, g_HUDIcono[playerid]);
        PlayerTextDrawShow(playerid, g_HUDNombre[playerid]);
        PlayerTextDrawShow(playerid, g_HUDVelocidad[playerid]);
        PlayerTextDrawShow(playerid, g_HUDUnidad[playerid]);
        PlayerTextDrawShow(playerid, g_HUDBarraFill[playerid]);
        PlayerTextDrawShow(playerid, g_HUDDatos[playerid]);
        g_HUDVisible[playerid] = true;
        // Fuerza refresco del icono al mostrar (por si cambio de coche
        // mientras el HUD estaba oculto).
        g_HUDModeloMostrado[playerid] = 0;
    }

    // Nombre + icono se actualizan solo cuando cambia el modelo (evita
    // pintar 60 veces por minuto lo mismo).
    if (g_HUDModeloMostrado[playerid] != modelo)
    {
        new nombre[32];
        Conce_NombreModelo(modelo, nombre, sizeof nombre);
        // MAYUSCULAS como en la referencia
        for (new i = 0; nombre[i]; i++) if (nombre[i] >= 'a' && nombre[i] <= 'z') nombre[i] -= 32;
        PlayerTextDrawSetString(playerid, g_HUDNombre[playerid], nombre);
        // Preview 3D del modelo real -- se debe re-mostrar tras cambiar
        // modelo/rot para que el cliente lo repinte.
        PlayerTextDrawSetPreviewModel(playerid, g_HUDIcono[playerid], modelo);
        PlayerTextDrawSetPreviewRot(playerid, g_HUDIcono[playerid], 0.0, 0.0, -60.0, 0.7);
        PlayerTextDrawShow(playerid, g_HUDIcono[playerid]);
        PlayerTextDrawShow(playerid, g_HUDNombre[playerid]);
        g_HUDModeloMostrado[playerid] = modelo;
    }

    new Float:vx, Float:vy, Float:vz;
    GetVehicleVelocity(vehicleid, vx, vy, vz);
    new Float:velocidad = floatsqroot(vx * vx + vy * vy + vz * vz) * 178.0;
    new kmh = floatround(velocidad);
    if (kmh < 0) kmh = 0;
    if (kmh > 999) kmh = 999;

    // Odometro persistente: solo cuenta con motor arrancado (no cuenta
    // remolques ni empujones). Se conserva del widget anterior.
    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(vehicleid, motor, luces, alarma, puertas, capo, maletero, objetivo);
    if (motor == VEHICLE_PARAMS_ON) g_Vehiculos[indice][veKilometros] += (velocidad / 3600.0);

    // Velocidad grande: siempre 2 digitos como en la ref (00, 42, 120).
    format(g_Buffer, sizeof g_Buffer, "%02d", kmh);
    PlayerTextDrawSetString(playerid, g_HUDVelocidad[playerid], g_Buffer);

    // Barra amarilla: ancho proporcional al % de combustible.
    //   carril: 479 -> 621  (=142 px de recorrido util)
    //   fill  : 479 + 142 * (pct/100)
    new Float:pctGas = g_Vehiculos[indice][veCombustible];
    if (pctGas < 0.0)   pctGas = 0.0;
    if (pctGas > 100.0) pctGas = 100.0;
    new Float:finX = 479.0 + (142.0 * (pctGas / 100.0));
    PlayerTextDrawTextSize(playerid, g_HUDBarraFill[playerid], finX, 6.0);
    // Cambio de color por umbral, mismo criterio que antes (verde/ambar/rojo parpadeante)
    new colorBar;
    if (pctGas <= HUD_GAS_UMBRAL_ROJO)          colorBar = (gettime() % 2 == 0) ? 0xE05A5AFF : 0x7A2E2EFF;
    else if (pctGas <= HUD_GAS_UMBRAL_AMARILLO) colorBar = 0xF2C94CFF;
    else                                         colorBar = 0xF2C94CFF; // dorado como la ref
    PlayerTextDrawBoxColour(playerid, g_HUDBarraFill[playerid], colorBar);
    PlayerTextDrawShow(playerid, g_HUDBarraFill[playerid]);

    // Salud del vehiculo: 0-1000 -> 0-100 %.
    new Float:hp;
    GetVehicleHealth(vehicleid, hp);
    new pctHP = floatround(hp / 10.0);
    if (pctHP < 0)   pctHP = 0;
    if (pctHP > 100) pctHP = 100;

    // Litros = % (0-100), a peticion de Vlad. No hay conversion a un tope
    // por modelo; el numero es el mismo dato de combustible mostrado como
    // "L" para respetar la referencia visual.
    new litros = floatround(pctGas);

    format(g_Buffer, sizeof g_Buffer, "~r~HP~w~ %d%%   ~y~GAS~w~ %dL", pctHP, litros);
    PlayerTextDrawSetString(playerid, g_HUDDatos[playerid], g_Buffer);
    return 1;
}

// Devuelve el INDICE de la gasolinera donde esta el jugador, o -1 si no
// esta en ninguna -- hace falta saber CUAL para decidir si este
// repostaje concreto alimenta la Gasolinera Los Santos (indices >=
// TOTAL_GASOLINERAS_ORIGINALES) o se destruye como siempre.
stock Veh_IndiceGasolinera(playerid)
{
    for (new i = 0; i < sizeof(g_Gasolineras); i++)
    {
        if (EnPunto(playerid, GASOLINERA_RADIO, g_Gasolineras[i][0], g_Gasolineras[i][1], g_Gasolineras[i][2])) return i;
    }
    return -1;
}

stock bool:EnGasolinera(playerid)
{
    return Veh_IndiceGasolinera(playerid) != -1;
}

CMD:repostar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante de un vehiculo.");
        return 1;
    }

    new indiceGasolinera = Veh_IndiceGasolinera(playerid);
    if (indiceGasolinera == -1)
    {
        Error(playerid, "No estas en una gasolinera.");
        return 1;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(vehicleid, motor, luces, alarma, puertas, capo, maletero, objetivo);
    if (motor == VEHICLE_PARAMS_ON)
    {
        Error(playerid, "Apaga el motor antes de repostar.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (indice == -1)
    {
        Error(playerid, "Este vehiculo no admite repostaje.");
        return 1;
    }

    new litros = floatround(100.0 - g_Vehiculos[indice][veCombustible]);
    if (litros <= 0)
    {
        Error(playerid, "El deposito ya esta lleno.");
        return 1;
    }

    new coste = litros * COMBUSTIBLE_PRECIO;
    if (!TieneDinero(playerid, coste))
    {
        new dinero[24];
        FormatearDinero(coste, dinero, sizeof dinero);
        format(g_Buffer, sizeof g_Buffer, "Llenar el deposito cuesta %s y no lo llevas encima.", dinero);
        Error(playerid, g_Buffer);
        return 1;
    }

    SendClientMessage(playerid, COLOR_BLANCO, "Repostando...");
    TogglePlayerControllable(playerid, false);

    SetTimerEx("Veh_RepostarCompletar", 4000, false, "dddd", playerid, indice, coste, indiceGasolinera);
    return 1;
}
CMD:gasolina(playerid, params[]) return cmd_repostar(playerid, params);

// A los 4 segundos de /repostar: descongela y, si todo sigue en orden
// (sigue conectado, sigue en el mismo coche, le sigue alcanzando el
// dinero), cobra y llena el deposito. indiceGasolinera decide si el
// cobro va a la caja fuerte de la Gasolinera Los Santos (petrolpumps
// nuevos, 07/09/2026, indice >= TOTAL_GASOLINERAS_ORIGINALES) o se
// destruye como siempre (las 12 gasolineras genericas de toda la vida).
forward Veh_RepostarCompletar(playerid, indice, coste, indiceGasolinera);
public Veh_RepostarCompletar(playerid, indice, coste, indiceGasolinera)
{
    if (!IsPlayerConnected(playerid)) return 0;

    TogglePlayerControllable(playerid, true);

    if (!VehiculoValido(indice)) return 0;
    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER || Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid)) != indice)
    {
        Error(playerid, "Te has movido antes de terminar de repostar.");
        return 0;
    }

    if (!TieneDinero(playerid, coste))
    {
        Error(playerid, "Ya no tienes suficiente dinero para pagar el repostaje.");
        return 0;
    }

    DarDinero(playerid, -coste);
    g_Vehiculos[indice][veCombustible] = 100.0;
    Veh_Guardar(indice);

    // Petrolpumps nuevos (07/09/2026): TODO lo cobrado aqui, integro, a
    // la caja fuerte de la Gasolinera Los Santos -- si no tiene dueno,
    // Negocio_Ingresar ya lo ignora solo, igual que el resto de negocios.
    // Las 12 gasolineras genericas de siempre NO tocan esto: su cobro
    // sigue siendo un sumidero puro, sin cambios.
    if (indiceGasolinera >= TOTAL_GASOLINERAS_ORIGINALES)
    {
        Negocio_Ingresar(NEGOCIO_SLOT_TIENDA, coste);
    }

    // Sin texto de chat -- peticion expresa de Vlad 07/09/2026: el aviso
    // ahora es en pantalla (Veh_RepostarMostrarAviso), mismo estilo que
    // /arreglarcoche, con el mismo sonido 1133.
    Veh_RepostarMostrarAviso(playerid, coste);

    new nombre[MAX_PLAYER_NAME + 1], mensaje[160];
    NombreRP(playerid, nombre, sizeof nombre);
    format(mensaje, sizeof mensaje, "* %s reposta el vehiculo.", nombre);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDOS DEL CONDUCTOR
// ---------------------------------------------------------------------
CMD:motor(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante.");
        return 1;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    new indice = Veh_BuscarPorObjeto(vehicleid);

    #if USAR_COMBUSTIBLE == 1
        if (indice != -1 && g_Vehiculos[indice][veCombustible] <= 0.0)
        {
            Error(playerid, "El deposito esta vacio.");
            return 1;
        }
    #endif

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(vehicleid, motor, luces, alarma, puertas, capo, maletero, objetivo);

    new nombre[MAX_PLAYER_NAME + 1], mensaje[160];
    NombreRP(playerid, nombre, sizeof nombre);

    // Normativa: /motor solo funciona en un coche a tu nombre (personal
    // tuyo, o libre comprado con /comprarcoche), o de trabajo/faccion/
    // servidor si tienes el puesto. Los libres sin dueno NO llevan el
    // motor arrancado (desde el 12/08/2026 todos nacen apagados, ver
    // Veh_Aparecer) -- para arrancarlos hace falta /puente, no /motor;
    // los coches ajenos (robados/puenteados) tampoco se tocan con /motor.
    if (indice != -1)
    {
        new tipo = g_Vehiculos[indice][veTipo];
        new bool:esTuyo = false;

        if (tipo == VEH_PERSONAL || tipo == VEH_DROPEO)
        {
            esTuyo = (g_Vehiculos[indice][vePropietario] != 0 && g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID]);
        }
        else
        {
            esTuyo = true; // trabajo/faccion/servidor: ya filtro Veh_PuedeUsar quien entra
        }

        if (!esTuyo)
        {
            if (tipo == VEH_DROPEO && g_Vehiculos[indice][vePropietario] == 0)
                Error(playerid, "Este coche es libre. Compralo primero con /comprarcoche para poder arrancarlo.");
            else
                Error(playerid, "Solo puedes arrancar o parar el motor de un coche a tu nombre. (Admin: usa /adminmotor)");
            return 1;
        }
    }

    if (motor == VEHICLE_PARAMS_ON)
    {
        SetVehicleParamsEx(vehicleid, VEHICLE_PARAMS_OFF, luces, alarma, VEHICLE_PARAMS_OFF, capo, maletero, objetivo);
        format(mensaje, sizeof mensaje, "* %s apaga el motor del vehiculo.", nombre);
    }
    else
    {
        SetVehicleParamsEx(vehicleid, VEHICLE_PARAMS_ON, luces, alarma, VEHICLE_PARAMS_OFF, capo, maletero, objetivo);
        format(mensaje, sizeof mensaje, "* %s gira la llave y arranca el motor.", nombre);
    }

    // SetVehicleParamsEx acaba de borrar el cierre por jugador: lo repone.
    if (indice != -1) Veh_ActualizarCierreTodos(indice);

    MensajeCercano(playerid, 12.0, COLOR_ACCION, mensaje);
    return 1;
}
CMD:m(playerid, params[]) return cmd_motor(playerid, params);

// ---------------------------------------------------------------------
//  ADMIN · /adminmotor
//  ---------------------------------------------------------------------
//  Arranca o para el motor de CUALQUIER vehiculo saltandose la normativa
//  de propiedad (sin comprarlo, sin ser de tu trabajo/faccion...). Pensado
//  para probar coches recien creados con /creardropeo o reposicionados con
//  /adminpark. Si lo arranca asi, se apaga solo en cuanto el admin se baja.
// ---------------------------------------------------------------------
CMD:adminmotor(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;
    return Veh_ForzarMotor(playerid);
}

// Arranca o apaga a la fuerza el motor del vehiculo en el que vas
// sentado, sin necesitar llave ni comprarlo. Se apaga solo al bajarte
// (Veh_AdminMotorAlBajar). La usan /adminmotor y /nfsmotor.
stock Veh_ForzarMotor(playerid)
{
    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante.");
        return 1;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    new indice = Veh_BuscarPorObjeto(vehicleid);

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(vehicleid, motor, luces, alarma, puertas, capo, maletero, objetivo);

    if (motor == VEHICLE_PARAMS_ON)
    {
        SetVehicleParamsEx(vehicleid, VEHICLE_PARAMS_OFF, luces, alarma, VEHICLE_PARAMS_OFF, capo, maletero, objetivo);
        if (indice != -1) g_AdminMotorForzado[indice] = false;
        Info(playerid, "Motor apagado.");
    }
    else
    {
        SetVehicleParamsEx(vehicleid, VEHICLE_PARAMS_ON, luces, alarma, VEHICLE_PARAMS_OFF, capo, maletero, objetivo);
        if (indice != -1) g_AdminMotorForzado[indice] = true;

        // El aviso completo solo la primera vez por sesion; luego, uno corto.
        if (!Jugador[playerid][jAvisoMotorLibre])
        {
            Info(playerid, "Motor arrancado sin comprarlo. Se apagara solo en cuanto te bajes.");
            Jugador[playerid][jAvisoMotorLibre] = true;
        }
        else
        {
            Info(playerid, "Motor arrancado.");
        }
    }

    if (indice != -1) Veh_ActualizarCierreTodos(indice);
    return 1;
}

// Llamada desde OnPlayerStateChange al bajarse del vehiculo (99_callbacks.inc).
// Si el motor se arranco con /adminmotor, se apaga automaticamente.
stock Veh_AdminMotorAlBajar(indice)
{
    if (!VehiculoValido(indice)) return 0;
    if (!g_AdminMotorForzado[indice]) return 0;

    g_AdminMotorForzado[indice] = false;

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(g_Vehiculos[indice][veObjeto], motor, luces, alarma, puertas, capo, maletero, objetivo);
    SetVehicleParamsEx(g_Vehiculos[indice][veObjeto], VEHICLE_PARAMS_OFF, luces, alarma, VEHICLE_PARAMS_OFF, capo, maletero, objetivo);
    Veh_ActualizarCierreTodos(indice);
    return 1;
}

// ---------------------------------------------------------------------
//  ADMIN · /adminlock
//  ---------------------------------------------------------------------
//  Fuerza la entrada al vehiculo mas cercano (cerrado o no, tuyo o
//  ajeno), saltandose por completo Veh_PuedeUsar. Es la UNICA puerta de
//  entrada que tienen los admins a un coche cerrado ajeno: desde
//  2026-08-12 Veh_PuedeUsar ya no da barra libre por rango, asi que sin
//  este comando un admin esta tan bloqueado como cualquiera.
// ---------------------------------------------------------------------
CMD:adminlock(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

    new indice = -1, Float:mejor = 6.0, Float:vx, Float:vy, Float:vz, Float:distancia;

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;

        GetVehiclePos(g_Vehiculos[i][veObjeto], vx, vy, vz);
        distancia = GetPlayerDistanceFromPoint(playerid, vx, vy, vz);

        if (distancia < mejor) { mejor = distancia; indice = i; }
    }

    if (indice == -1)
    {
        Error(playerid, "No hay ningun vehiculo cerca.");
        return 1;
    }

    g_AdminLockOverride[playerid] = true;
    PutPlayerInVehicle(playerid, g_Vehiculos[indice][veObjeto], 0);

    format(g_Buffer, sizeof g_Buffer, "Entrada forzada al vehiculo (matricula %s), saltandote el candado.",
        g_Vehiculos[indice][veMatricula]);
    Info(playerid, g_Buffer);

    format(g_Buffer, sizeof g_Buffer, "ADMINLOCK  admin=%d  matricula=%s", Jugador[playerid][jID], g_Vehiculos[indice][veMatricula]);
    EscribirLog("administracion", g_Buffer);
    return 1;
}

CMD:luces(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante.");
        return 1;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    new indice = Veh_BuscarPorObjeto(vehicleid);

    // Misma normativa que /motor: solo en un coche a tu nombre (personal
    // tuyo, o libre comprado con /comprarcoche), o de trabajo/faccion/
    // servidor si tienes el puesto.
    if (indice != -1)
    {
        new tipo = g_Vehiculos[indice][veTipo];
        new bool:esTuyo = false;

        if (tipo == VEH_PERSONAL || tipo == VEH_DROPEO)
        {
            esTuyo = (g_Vehiculos[indice][vePropietario] != 0 && g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID]);
        }
        else
        {
            esTuyo = true; // trabajo/faccion/servidor: ya filtro Veh_PuedeUsar quien entra
        }

        if (!esTuyo)
        {
            if (tipo == VEH_DROPEO && g_Vehiculos[indice][vePropietario] == 0)
                Error(playerid, "Este coche es libre. Compralo primero con /comprarcoche.");
            else
                Error(playerid, "Solo puedes tocar las luces de un coche a tu nombre.");
            return 1;
        }
    }

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(vehicleid, motor, luces, alarma, puertas, capo, maletero, objetivo);

    new nombre[MAX_PLAYER_NAME + 1], mensaje[160];
    NombreRP(playerid, nombre, sizeof nombre);

    if (luces == VEHICLE_PARAMS_ON)
    {
        SetVehicleParamsEx(vehicleid, motor, VEHICLE_PARAMS_OFF, alarma, VEHICLE_PARAMS_OFF, capo, maletero, objetivo);
        format(mensaje, sizeof mensaje, "* %s apaga las luces del vehiculo.", nombre);
    }
    else
    {
        SetVehicleParamsEx(vehicleid, motor, VEHICLE_PARAMS_ON, alarma, VEHICLE_PARAMS_OFF, capo, maletero, objetivo);
        format(mensaje, sizeof mensaje, "* %s enciende las luces del vehiculo.", nombre);
    }

    if (indice != -1) Veh_ActualizarCierreTodos(indice);

    MensajeCercano(playerid, 12.0, COLOR_ACCION, mensaje);
    return 1;
}

CMD:lock(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new indice = -1, Float:mejor = 6.0, Float:distancia;

    // Busca el vehiculo propio mas cercano (o el que este conduciendo).
    if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
    {
        indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    }
    else
    {
        for (new i = 0; i < g_UltimoVehiculo; i++)
        {
            if (!g_Vehiculos[i][veUsado]) continue;
            if (g_Vehiculos[i][veTipo] != VEH_PERSONAL && g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
            if (g_Vehiculos[i][vePropietario] != Jugador[playerid][jID]) continue;

            new Float:x, Float:y, Float:z;
            GetVehiclePos(g_Vehiculos[i][veObjeto], x, y, z);
            distancia = GetPlayerDistanceFromPoint(playerid, x, y, z);

            if (distancia < mejor) { mejor = distancia; indice = i; }
        }
    }

    if (indice == -1)
    {
        Error(playerid, "No tienes ningun vehiculo tuyo cerca.");
        return 1;
    }

    new bool:tieneLlave = (g_Vehiculos[indice][veTipo] == VEH_PERSONAL)
        || (g_Vehiculos[indice][veTipo] == VEH_DROPEO && g_Vehiculos[indice][vePropietario] != 0);

    if (tieneLlave && g_Vehiculos[indice][vePropietario] != Jugador[playerid][jID])
    {
        Error(playerid, "Ese vehiculo no es tuyo.");
        return 1;
    }

    // Los de trabajo y faccion no tienen llave: quien puede usarlos lo
    // decide el puesto, no un cierre manual. Los libres sin dueno tampoco:
    // son de la calle, no hay nada que cerrar hasta que alguien los compre.
    if (!tieneLlave)
    {
        Error(playerid, "Este vehiculo no lleva llave.");
        return 1;
    }

    g_Vehiculos[indice][veCerrado] = !g_Vehiculos[indice][veCerrado];

    // El cierre se aplica jugador por jugador, nunca de forma global.
    Veh_ActualizarCierreTodos(indice);
    Veh_Guardar(indice);

    // Refresca el cartel [ABIERTO]/[CERRADO] al instante, no solo cuando
    // alguien se baja del coche. Si vas dentro (el cartel esta oculto
    // por estar conduciendo), lo saca, lo actualiza y programa que
    // vuelva a desaparecer solo (2s + parpadeo), como al subirte.
    Dropeo_RefrescarCartelForzado(indice);

    new nombre[MAX_PLAYER_NAME + 1], mensaje[160];
    NombreRP(playerid, nombre, sizeof nombre);

    // Sin aviso [Gamerol 2.0] aparte: el /me de abajo ya se lo dice a
    // todo el mundo, incluido tu.
    if (g_Vehiculos[indice][veCerrado])
    {
        format(mensaje, sizeof mensaje, "* %s cierra el vehiculo con llave.", nombre);
    }
    else
    {
        format(mensaje, sizeof mensaje, "* %s abre el vehiculo con la llave.", nombre);
    }

    MensajeCercano(playerid, 12.0, COLOR_ACCION, mensaje);
    return 1;
}


// ---------------------------------------------------------------------
//  /PARK  ·  Fija el sitio de aparcamiento de un coche que has comprado.
//  ---------------------------------------------------------------------
//  Solo funciona en coches a tu nombre (personal tuyo, o libre comprado
//  con /comprarcoche): si se te caducan los papeles, el coche vuelve
//  justo aqui en vez de quedarse tirado donde lo dejaste.
// ---------------------------------------------------------------------
CMD:park(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del vehiculo que quieres parquear.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (indice == -1 || (g_Vehiculos[indice][veTipo] != VEH_PERSONAL && g_Vehiculos[indice][veTipo] != VEH_DROPEO))
    {
        Error(playerid, "Este vehiculo no se puede parquear.");
        return 1;
    }

    if (g_Vehiculos[indice][vePropietario] != Jugador[playerid][jID])
    {
        Error(playerid, "Solo puedes parquear un vehiculo que hayas comprado. (Admin: usa /adminpark)");
        return 1;
    }

    Veh_Aparcar(indice);
    Exito(playerid, "PARK!");
    return 1;
}

// Texto "3 dia(s) 4h" / "Caducados" / "Sin caducidad" para el tiempo que
// le queda a los papeles de un vehiculo. Reutilizable donde haga falta
// (por ahora, /misveh).
stock Veh_TiempoRestanteTexto(indice, destino[], tam)
{
    if (g_Vehiculos[indice][veDropeoExpira] <= 0)
    {
        format(destino, tam, "Sin caducidad");
        return 1;
    }

    new restante = g_Vehiculos[indice][veDropeoExpira] - gettime();
    if (restante <= 0)
    {
        format(destino, tam, "Caducados");
        return 1;
    }

    new dias = restante / 86400;
    new horas = (restante % 86400) / 3600;
    format(destino, tam, "%d dia(s) %dh", dias, horas);
    return 1;
}

// /misveh: solo modelo (rojo) y tiempo restante de papeles (verde).
// Sin matricula, sin estado, sin posicion ni /localizar -- a proposito.
CMD:misveh(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new lista[1600], linea[112], total = 0, modelo[48], tiempo[32];

    format(lista, sizeof lista, "{E8B33A}Vehiculo\tPapeles\n");

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (g_Vehiculos[i][veTipo] != VEH_PERSONAL && g_Vehiculos[i][veTipo] != VEH_DROPEO) continue;
        if (g_Vehiculos[i][vePropietario] != Jugador[playerid][jID]) continue;

        Veh_NombreModelo(g_Vehiculos[i][veModelo], modelo, sizeof modelo);
        Veh_TiempoRestanteTexto(i, tiempo, sizeof tiempo);

        format(linea, sizeof linea, "{FF0000}%s\t{32FF32}%s\n", modelo, tiempo);
        strcat(lista, linea, sizeof lista);
        total++;
    }

    if (total == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_NADA, DIALOG_STYLE_MSGBOX,
            "{E8B33A}Mis vehiculos",
            "{B4B5B7}No tienes ningun vehiculo a tu nombre.\n\nHay coches libres por la calle: subete a uno y usa {FFFFFF}/comprarcoche{B4B5B7}.",
            "Cerrar", "");
        return 1;
    }

    new titulo[64];
    format(titulo, sizeof titulo, "{E8B33A}Mis vehiculos (%d)", total);

    ShowPlayerDialog(playerid, DIALOGO_NADA, DIALOG_STYLE_TABLIST_HEADERS, titulo, lista, "Cerrar", "");
    return 1;
}

CMD:vendervehiculo(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del vehiculo que quieres vender.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_PERSONAL || g_Vehiculos[indice][vePropietario] != Jugador[playerid][jID])
    {
        Error(playerid, "Este vehiculo no es tuyo.");
        return 1;
    }

    if (GetPlayerDistanceFromPoint(playerid, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z) > 30.0)
    {
        Error(playerid, "Tienes que llevar el vehiculo al taller de la NFS para venderlo.");
        return 1;
    }

    Jugador[playerid][jTemporal] = indice;

    new precio = (g_Vehiculos[indice][vePrecio] * CONCE_PORCENTAJE_VENTA) / 100;
    new dinero[24], modelo[48];
    FormatearDinero(precio, dinero, sizeof dinero);
    Veh_NombreModelo(g_Vehiculos[indice][veModelo], modelo, sizeof modelo);

    new cuerpo[512];
    format(cuerpo, sizeof cuerpo,
        "{FFFFFF}Vas a vender tu {E8B33A}%s{FFFFFF} (matricula %s).\n\n\
        {B4B5B7}El taller te ofrece el {FFFFFF}%d%%{B4B5B7} del precio de catalogo:\n\n\
        {3D9970}%s\n\n{E05A5A}Esta operacion no se puede deshacer.",
        modelo, g_Vehiculos[indice][veMatricula], CONCE_PORCENTAJE_VENTA, dinero);

    ShowPlayerDialog(playerid, DIALOGO_VEHICULO_VENDER, DIALOG_STYLE_MSGBOX,
        "{E8B33A}Vender vehiculo", cuerpo, "Vender", "Cancelar");
    return 1;
}

// ---------------------------------------------------------------------
//  DIÁLOGOS
// ---------------------------------------------------------------------
stock Veh_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext, listitem

    switch (dialogid)
    {
        case DIALOGO_VEHICULO_VENDER:
        {
            if (!response) return 1;

            new indice = Jugador[playerid][jTemporal];
            if (!VehiculoValido(indice)) return 1;
            if (g_Vehiculos[indice][vePropietario] != Jugador[playerid][jID]) return 1;

            new precio = (g_Vehiculos[indice][vePrecio] * CONCE_PORCENTAJE_VENTA) / 100;
            new modelo[48], dinero[24];
            Veh_NombreModelo(g_Vehiculos[indice][veModelo], modelo, sizeof modelo);
            FormatearDinero(precio, dinero, sizeof dinero);

            RemovePlayerFromVehicle(playerid);
            Veh_Borrar(indice);

            Jugador[playerid][jBanco] += precio;
            BD_GuardarCuenta(playerid);

            format(g_Buffer, sizeof g_Buffer, "Has vendido tu %s. Te han ingresado %s en el banco.", modelo, dinero);
            Exito(playerid, g_Buffer);

            new nombre[MAX_PLAYER_NAME + 1];
            NombreRP(playerid, nombre, sizeof nombre);
            format(g_Buffer, sizeof g_Buffer, "%s vendio un %s por %d", nombre, modelo, precio);
            EscribirLog("economia", g_Buffer);
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------
//  RASTREADOR GPS -- extra de un vehiculo propio de verdad (VEH_PERSONAL
//  o VEH_DROPEO ya legalizado -- no hay concesionario en este servidor,
//  la inmensa mayoria de coches de jugador son VEH_DROPEO), comprado via
//  web (31_web.inc, comando de cola 'comprar_rastreador'). Mismo patron
//  de dos pasos que Camara_Comprar (35_cancelas.inc): la web solo pide,
//  aqui se revalida el banco y la propiedad de verdad.
// ---------------------------------------------------------------------
stock Rastreador_Comprar(colaId, cuentaId, vehiculoId)
{
    new consulta[450];
    format(consulta, sizeof consulta,
        "SELECT cu.banco, \
         (SELECT id FROM vehiculos WHERE id = %d AND propietario = %d AND tipo IN (%d, %d) LIMIT 1) AS vehiculo_id, \
         (SELECT tiene_rastreador FROM vehiculos WHERE id = %d AND propietario = %d LIMIT 1) AS ya_tiene \
         FROM cuentas cu WHERE cu.id = %d LIMIT 1",
        vehiculoId, cuentaId, VEH_PERSONAL, VEH_DROPEO, vehiculoId, cuentaId, cuentaId);
    mysql_pquery(g_BD, consulta, "Rastreador_AlComprobarCompra", "ddd", colaId, cuentaId, vehiculoId);
    return 1;
}

forward Rastreador_AlComprobarCompra(colaId, cuentaId, vehiculoId);
public Rastreador_AlComprobarCompra(colaId, cuentaId, vehiculoId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Cuenta no encontrada.");
        return 0;
    }

    new banco, vehiculoValido, yaTiene;
    cache_get_value_name_int(0, "banco", banco);
    cache_get_value_name_int(0, "vehiculo_id", vehiculoValido);
    cache_get_value_name_int(0, "ya_tiene", yaTiene);

    if (vehiculoValido == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Ese vehiculo no es tuyo.");
        return 0;
    }
    if (yaTiene != 0)
    {
        Web_MarcarTerminado(colaId, "error", "Ese vehiculo ya tiene rastreador.");
        return 0;
    }
    if (banco < RASTREADOR_PRECIO)
    {
        Web_MarcarTerminado(colaId, "error", "Saldo insuficiente en el banco.");
        return 0;
    }

    new consultaCobro[120];
    format(consultaCobro, sizeof consultaCobro, "UPDATE cuentas SET banco = banco - %d WHERE id = %d", RASTREADOR_PRECIO, cuentaId);
    mysql_pquery(g_BD, consultaCobro);

    new consultaInstalar[100];
    format(consultaInstalar, sizeof consultaInstalar, "UPDATE vehiculos SET tiene_rastreador = 1 WHERE id = %d", vehiculoId);
    mysql_pquery(g_BD, consultaInstalar);

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado] || g_Vehiculos[i][veID] != vehiculoId) continue;
        g_Vehiculos[i][veTieneRastreador] = true;
        // Si es VEH_DROPEO con cartel flotante, que se le vea "Localizador
        // ON" ya mismo, sin esperar a que algo mas lo refresque.
        if (g_Vehiculos[i][veTipo] == VEH_DROPEO) Dropeo_ActualizarCartel(i);
        break;
    }

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
        Jugador[conectado][jBanco] -= RASTREADOR_PRECIO;
        Banco_Movimiento(cuentaId, 2, -RASTREADOR_PRECIO, Jugador[conectado][jBanco], "", "Rastreador GPS via web");
    }
    else
    {
        Banco_Movimiento(cuentaId, 2, -RASTREADOR_PRECIO, banco - RASTREADOR_PRECIO, "", "Rastreador GPS via web");
    }

    format(g_Buffer, sizeof g_Buffer, "Rastreador instalado via web: cuenta %d, vehiculo %d, -%d$", cuentaId, vehiculoId, RASTREADOR_PRECIO);
    EscribirLog("economia", g_Buffer);
    Web_MarcarTerminado(colaId, "ok", "Rastreador instalado. La posicion se actualiza cada 10 segundos en el panel.");
    return 1;
}

// ---------------------------------------------------------------------
//  TICK -- llamado una vez cada RASTREADOR_INTERVALO_SEGUNDOS desde
//  TicSegundo (99_callbacks.inc). Recorre solo los vehiculos con
//  rastreador y escribe su posicion actual para que el panel la lea.
//  No toca pos_x/pos_y (eso es la posicion de reaparicion, otro
//  concepto) -- columnas propias rastreador_x/rastreador_y.
// ---------------------------------------------------------------------
stock Rastreador_Segundo()
{
    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado] || !g_Vehiculos[i][veTieneRastreador]) continue;
        if (!VehiculoValido(i) || g_Vehiculos[i][veObjeto] == INVALID_VEHICLE_ID) continue;

        new Float:x, Float:y, Float:z;
        GetVehiclePos(g_Vehiculos[i][veObjeto], x, y, z);

        new consulta[150];
        mysql_format(g_BD, consulta, sizeof consulta,
            "UPDATE vehiculos SET rastreador_x = %f, rastreador_y = %f, rastreador_actualizado_en = NOW() WHERE id = %d",
            x, y, g_Vehiculos[i][veID]);
        mysql_pquery(g_BD, consulta);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  AVISO WHATSAPP DE CADUCIDAD -- llamado una vez por segundo desde
//  TicSegundo (99_callbacks.inc). Recorre los vehiculos con papeles
//  (veDropeoExpira != 0) y encola un aviso via Web_EncolarAviso
//  (31_web.inc) la primera vez que quedan <=24h. veAvisado24h evita
//  reencolarlo cada segundo mientras dura la ventana -- se resetea a
//  false en cada renovacion de papeles (22_dropeo.inc, 26_nfs.inc,
//  31_web.inc), asi que solo avisa una vez por ciclo de 24h real.
// ---------------------------------------------------------------------
#define VEH_AVISO_CADUCIDAD_SEGUNDOS (24 * 3600)

stock Veh_AvisoCaducidad_Segundo()
{
    new ahora = gettime();
    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado] || g_Vehiculos[i][vePropietario] == 0) continue;
        if (g_Vehiculos[i][veDropeoExpira] == 0 || g_Vehiculos[i][veAvisado24h]) continue;

        new restante = g_Vehiculos[i][veDropeoExpira] - ahora;
        if (restante <= 0 || restante > VEH_AVISO_CADUCIDAD_SEGUNDOS) continue;

        g_Vehiculos[i][veAvisado24h] = true;
        Web_EncolarAviso(g_Vehiculos[i][vePropietario], "vehiculo",
            "Tu vehiculo caduca en menos de 24 horas. Renueva los papeles para no perderlo.");
    }
    return 1;
}

// ---------------------------------------------------------------------
//  ALARMA DE COCHE -- extra de un vehiculo propio (VEH_PERSONAL o
//  VEH_DROPEO), comprado via web (31_web.inc, comando de cola
//  'comprar_alarma'). Mismo patron de dos pasos que Rastreador_Comprar.
// ---------------------------------------------------------------------
stock Alarma_Comprar(colaId, cuentaId, vehiculoId)
{
    new consulta[450];
    format(consulta, sizeof consulta,
        "SELECT cu.banco, \
         (SELECT id FROM vehiculos WHERE id = %d AND propietario = %d AND tipo IN (%d, %d) LIMIT 1) AS vehiculo_id, \
         (SELECT tiene_alarma FROM vehiculos WHERE id = %d AND propietario = %d LIMIT 1) AS ya_tiene \
         FROM cuentas cu WHERE cu.id = %d LIMIT 1",
        vehiculoId, cuentaId, VEH_PERSONAL, VEH_DROPEO, vehiculoId, cuentaId, cuentaId);
    mysql_pquery(g_BD, consulta, "Alarma_AlComprobarCompra", "ddd", colaId, cuentaId, vehiculoId);
    return 1;
}

forward Alarma_AlComprobarCompra(colaId, cuentaId, vehiculoId);
public Alarma_AlComprobarCompra(colaId, cuentaId, vehiculoId)
{
    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Cuenta no encontrada.");
        return 0;
    }

    new banco, vehiculoValido, yaTiene;
    cache_get_value_name_int(0, "banco", banco);
    cache_get_value_name_int(0, "vehiculo_id", vehiculoValido);
    cache_get_value_name_int(0, "ya_tiene", yaTiene);

    if (vehiculoValido == 0)
    {
        Web_MarcarTerminado(colaId, "error", "Ese vehiculo no es tuyo.");
        return 0;
    }
    if (yaTiene != 0)
    {
        Web_MarcarTerminado(colaId, "error", "Ese vehiculo ya tiene alarma.");
        return 0;
    }
    if (banco < ALARMA_PRECIO)
    {
        Web_MarcarTerminado(colaId, "error", "Saldo insuficiente en el banco.");
        return 0;
    }

    new consultaCobro[120];
    format(consultaCobro, sizeof consultaCobro, "UPDATE cuentas SET banco = banco - %d WHERE id = %d", ALARMA_PRECIO, cuentaId);
    mysql_pquery(g_BD, consultaCobro);

    new consultaInstalar[100];
    format(consultaInstalar, sizeof consultaInstalar, "UPDATE vehiculos SET tiene_alarma = 1 WHERE id = %d", vehiculoId);
    mysql_pquery(g_BD, consultaInstalar);

    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado] || g_Vehiculos[i][veID] != vehiculoId) continue;
        g_Vehiculos[i][veTieneAlarma] = true;
        break;
    }

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
        Jugador[conectado][jBanco] -= ALARMA_PRECIO;
        Banco_Movimiento(cuentaId, 2, -ALARMA_PRECIO, Jugador[conectado][jBanco], "", "Alarma de coche via web");
    }
    else
    {
        Banco_Movimiento(cuentaId, 2, -ALARMA_PRECIO, banco - ALARMA_PRECIO, "", "Alarma de coche via web");
    }

    format(g_Buffer, sizeof g_Buffer, "Alarma instalada via web: cuenta %d, vehiculo %d, -%d$", cuentaId, vehiculoId, ALARMA_PRECIO);
    EscribirLog("economia", g_Buffer);
    Web_MarcarTerminado(colaId, "ok", "Alarma instalada.");
    return 1;
}

// ---------------------------------------------------------------------
//  DISPARO -- llamado desde 22_dropeo.inc (DropeoPuenteFin) en el
//  instante exacto en que alguien te puentea con exito. Activa la
//  alarma NATIVA de SA-MP (bocina + luces, no un sonido inventado) y
//  avisa al dueno si esta conectado. Se apaga sola a los
//  ALARMA_DURACION_SEGUNDOS via AlarmaApagar().
// ---------------------------------------------------------------------
stock Alarma_Disparar(indice)
{
    if (!VehiculoValido(indice) || !g_Vehiculos[indice][veTieneAlarma]) return 0;

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(g_Vehiculos[indice][veObjeto], motor, luces, alarma, puertas, capo, maletero, objetivo);
    SetVehicleParamsEx(g_Vehiculos[indice][veObjeto],
        motor, luces, VEHICLE_PARAMS_ON, puertas, capo, maletero, objetivo);

    SetTimerEx("AlarmaApagar", ALARMA_DURACION_SEGUNDOS * 1000, false, "i", indice);

    if (g_Vehiculos[indice][vePropietario] != 0)
    {
        new modelo[32];
        Veh_NombreModelo(g_Vehiculos[indice][veModelo], modelo, sizeof modelo);

        for (new p = 0; p < MAX_PLAYERS; p++)
        {
            if (!IsPlayerConnected(p) || !Jugador[p][jLogueado]) continue;
            if (Jugador[p][jID] != g_Vehiculos[indice][vePropietario]) continue;

            format(g_Buffer, sizeof g_Buffer,
                "Alguien esta puenteando tu %s (matricula %s). La alarma esta sonando.",
                modelo, g_Vehiculos[indice][veMatricula]);
            Aviso(p, g_Buffer);
            break;
        }
    }
    return 1;
}

forward AlarmaApagar(indice);
public AlarmaApagar(indice)
{
    if (!VehiculoValido(indice)) return 0;

    new motor, luces, alarma, puertas, capo, maletero, objetivo;
    GetVehicleParamsEx(g_Vehiculos[indice][veObjeto], motor, luces, alarma, puertas, capo, maletero, objetivo);
    SetVehicleParamsEx(g_Vehiculos[indice][veObjeto],
        motor, luces, VEHICLE_PARAMS_OFF, puertas, capo, maletero, objetivo);
    return 1;
}
