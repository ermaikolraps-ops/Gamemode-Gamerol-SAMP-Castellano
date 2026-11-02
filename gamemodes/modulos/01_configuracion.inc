/* =====================================================================
 *  01_configuracion.inc
 *  ---------------------------------------------------------------------
 *  Todo lo que vas a querer ajustar del servidor esta en este archivo.
 *  Si algo se puede configurar, se configura aqui y no repartido por el
 *  codigo. Cambia un valor, recompila y listo.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  IDENTIDAD DEL SERVIDOR
// ---------------------------------------------------------------------
// Rebautizado el 07/09/2026 (peticion expresa de Vlad): "Gamerol 2.0" ->
// "Gamerol 2026 - Encuentralos todos!". Cambiar aqui basta para el
// nombre del ban, el titulo de /ayuda, las normas y el prefijo de chat
// -- el Mode del server browser y el logo grande de la pantalla de login
// NO leen esto (son literales sueltos, ver SetGameModeText en
// 99_callbacks.inc y TD_LOGO/TD_SUB en 06_login.inc).
#define SERVIDOR_NOMBRE         "Gamerol 2026 - Encuentralos todos!"
#define SERVIDOR_NOMBRE_CORTO   "Gamerol 2026"
#define SERVIDOR_WEB            "www.gamerol.net"
#define SERVIDOR_DISCORD        "discord.gg/malagacity"
#define SERVIDOR_ANO            1992

// Prefijo que aparece delante de los mensajes del sistema.
#define PREFIJO                 "{E8B33A}[Gamerol 2026]{FFFFFF} "

// ---------------------------------------------------------------------
//  CONEXION A LA BASE DE DATOS
//  22/10/2026: usuario y contrasena YA NO van en el .amx. Los lee
//  mysql_connect_file() de /opt/gamerol/samp/mysql.ini (600, gamerol):
//  hostname, username, password, database. BD_NOMBRE solo se usa en logs.
// ---------------------------------------------------------------------
#define BD_NOMBRE               "malagacity"

// ---------------------------------------------------------------------
//  LIMITES GENERALES
// ---------------------------------------------------------------------
#define MAX_TRABAJOS            6       // Ranuras de trabajo disponibles
#define MAX_FACCIONES           6       // Ranuras de faccion disponibles
#define MAX_TRABAJOS_JUGADOR    2       // Trabajos simultaneos por jugador
#define MAX_RANGOS              10      // Rangos por faccion
#define MAX_VEHICULOS           1000    // Vehiculos persistentes totales
#define VEHICULO_SALUD_CRITICA  400.0   // por debajo de esto se considera "ardiendo" (fix coches ardiendo vacios, 12/09/2026)
#define MAX_SLOTS_INVENTARIO    20      // Ranuras del inventario
#define MAX_OBJETOS_SUELO       200     // Objetos tirados simultaneos

// ---------------------------------------------------------------------
//  ECONOMIA
// ---------------------------------------------------------------------
#define DINERO_INICIAL          10000   // Dinero en mano al crear personaje
#define BANCO_INICIAL           0       // Saldo bancario al crear personaje

// Nomina (payday). Ayuda GOV por falta de trabajo: subvencion fija para
// todo el mundo, tengas o no trabajo o faccion.
#define PAYDAY_SUBVENCION       3350    // Pago unico y fijo, sin suplementos de trabajo

#define PAYDAY_MINUTOS          20      // Minutos de juego entre payday y payday
#define PAYDAY_MINUTOS_PRIMERO  2       // El primer payday de una cuenta nueva es casi inmediato
#define PAYDAY_AFK_SEGUNDOS     120     // Sin actividad = el minuto no cuenta

// Banco
#define BANCO_COMISION          1       // % de comision por transferencia
#define BANCO_TRANSFER_MINIMA   100
#define BANCO_TRANSFER_MAXIMA   1000000

// Concesionario
#define CONCE_IMPUESTO_COMPRA   0       // % extra sobre el precio de catalogo
#define CONCE_PORCENTAJE_VENTA  60      // % del precio que se devuelve al vender

// ---------------------------------------------------------------------
//  DROPEO  (coches libres por el mapa + robo de coches ajenos abiertos)
//  Rediseno 2026-08-12. Ver modulos/22_dropeo.inc.
// ---------------------------------------------------------------------
#define DROPEO_DIAS_PROPIEDAD   7        // dias que duran los papeles de un coche libre comprado
#define DROPEO_RENOVAR_DIAS_ANTES 2      // solo se puede renovar cuando queden esto o menos dias (a partir del 6 de 7)
#define DROPEO_PREMIO_DROPEO    1000     // $ que se lleva quien dropea (vende robado) un coche ajeno
#define ABANDONO_PRECIO         1        // $ simbolico que se paga por /abandonarcoche

// Negocios (32_negocios.inc). Aqui y no alli porque el texto de /ayuda
// (09_cuenta.inc) los usa, y ese archivo se incluye antes.
#define NEGOCIO_PORCENTAJE_CAJA   60     // % de cada venta de la tienda que va a la caja fuerte del dueno
#define NEGOCIO_PORCENTAJE_SOLTAR 50     // % del precio que se devuelve al soltar el negocio (/soltarnegocio)
#define NEGOCIO_PORCENTAJE_NOMINA 10     // % de PAYDAY_SUBVENCION que gana "Bonos del Estado" en cada nomina (aparte, no se lo resta al jugador)
#define NEGOCIO_IMPUESTO_VEHICULO 25     // $ que SI se resta de la nomina del jugador, por cada vehiculo a su nombre -- va a "Bonos del Estado"

// Puente: arrancar sin llave un coche ya abierto (libre o ajeno sin cerrar).
#define DROPEO_PUENTE_FREEZE_MS 4000    // ms que el jugador queda congelado
#define DROPEO_PUENTE_EXITO     60      // % de exito por intento -- igualado a FORZAR_MALETERO_EXITO, subido al 60% el 08/09/2026 (antes 42%)
#define DROPEO_PUENTE_FALLOS    10      // fallos seguidos que activan el cooldown
#define DROPEO_PUENTE_ESPERA    900     // segundos de cooldown al llegar al tope

// Iconos del minimapa (ver documentacion de open.mp para el listado).
#define DROPEO_ICONO_COCHE      55      // coche rojo (Import/Export)
#define DROPEO_ICONO_TALLER     50      // llave inglesa / taller mecanico

// Parquin comunitario: aparcamiento fijo lleno de coches libres del
// sistema de dropeo. Coordenada guardada desde /panel > Coordenadas
// (clave "blib", 2026-08-12). Lleva un blip de coche rojo permanente.
#define PARQUIN_X                1709.57
#define PARQUIN_Y                -1155.29
#define PARQUIN_Z                23.8281

// ---------------------------------------------------------------------
//  DISTANCIAS DE CHAT (en metros de juego)
// ---------------------------------------------------------------------
#define RADIO_SUSURRO           3.0
#define RADIO_NORMAL            20.0
#define RADIO_GRITO             40.0
#define RADIO_ACCION            25.0    // /me y /do
#define RADIO_OOC_LOCAL         20.0    // /b
#define RADIO_INTERACCION       3.0     // dar objetos, pagar, etc.

// ---------------------------------------------------------------------
//  SEGURIDAD DE CUENTAS
// ---------------------------------------------------------------------
#define LOGIN_INTENTOS_MAX      3
#define LOGIN_SEGUNDOS_MAX      120     // Tiempo para loguearse antes del kick
#define PASSWORD_MIN            6
#define PASSWORD_MAX            32
#define COSTE_BCRYPT            12      // Coste del hash (12 = ~250 ms)

// ---------------------------------------------------------------------
//  SCORE MIENTRAS NO SE HA ENTRADO A LA CUENTA
//  El listado de jugadores del SA-MP (tecla TAB y el navegador de
//  servidores) ensena el score de todo el mundo, tambien de quien
//  todavia esta en la pantalla de entrada. Sin esto salen todos con un
//  0 al lado. Se les pone un numero bajo al azar hasta que se loguean;
//  en cuanto entran, AplicarDatosJugador lo pisa con su nivel real.
//  Pon los dos valores al mismo numero si quieres uno fijo.
// ---------------------------------------------------------------------
#define SCORE_SIN_LOGUEAR_MIN   1
#define SCORE_SIN_LOGUEAR_MAX   5

// ---------------------------------------------------------------------
//  NIVELES DE ADMINISTRACION
// ---------------------------------------------------------------------
#define ADMIN_NINGUNO           0
#define ADMIN_SOPORTE           1
#define ADMIN_MODERADOR         2
#define ADMIN_ADMIN             3
#define ADMIN_JEFE              4
#define ADMIN_GESTION           5
#define ADMIN_FUNDADOR          6

// ---------------------------------------------------------------------
//  PALETA DE COLORES
//  Formato RGBA. Los codigos {RRGGBB} son para incrustar dentro de texto.
// ---------------------------------------------------------------------

// Marca
#define COLOR_MARCA             0xE8B33AFF      // Dorado Gamerol
#define COLOR_MARCA_AZUL        0x2F80EDFF
#define HEX_MARCA               "{E8B33A}"
#define HEX_AZUL                "{5A9BD5}"

// Basicos
#define COLOR_BLANCO            0xFFFFFFFF
#define COLOR_NEGRO             0x000000FF
#define COLOR_GRIS              0xB4B5B7FF
#define COLOR_GRIS_OSCURO       0x8A8A8AFF
#define COLOR_ROJO              0xE05A5AFF
#define COLOR_ROJO_FUERTE       0xC0392BFF
#define COLOR_VERDE             0x53C168FF
#define COLOR_AZUL              0x5A9BD5FF
#define COLOR_AMARILLO          0xF2C94CFF
#define COLOR_NARANJA           0xE8973AFF
#define COLOR_MORADO            0xC2A2DAFF
#define COLOR_ROSA              0xE07AC4FF
#define COLOR_TURQUESA          0x4FD1C5FF
#define COLOR_MARRON            0x8B5A2BFF      // /verlista: fila de Chatarra
#define HEX_MARRON              "{8B5A2B}"

#define HEX_BLANCO              "{FFFFFF}"
#define HEX_GRIS                "{B4B5B7}"
#define HEX_ROJO                "{E05A5A}"
#define HEX_VERDE               "{53C168}"
#define HEX_AMARILLO            "{F2C94C}"
#define HEX_NARANJA             "{E8973A}"
#define HEX_MORADO              "{C2A2DA}"
#define HEX_DINERO              "{3D9970}"

// Mensajes del sistema
#define COLOR_ERROR             0xE05A5AFF
#define COLOR_EXITO             0x53C168FF
#define COLOR_INFO              0x8BC4E8FF
#define COLOR_AVISO             0xF2C94CFF
#define COLOR_DINERO            0x3D9970FF

// "[Server IA]" (58_ascensos.inc, 08/09/2026) -- voz institucional del
// servidor, distinta de "Info:" (COLOR_INFO, ayuda practica individual
// de 56_consejos.inc). Azul-violeta nuevo, no reutiliza ningun color ya
// asignado a otro sistema de esta paleta.
#define COLOR_SERVERIA          0x8E7CFCFF
#define HEX_SERVERIA            "{8E7CFC}"

// Chat de rol (colores clasicos de servidor de rol)
#define COLOR_IC                0xFFFFFFFF      // Hablar normal
#define COLOR_SUSURRO           0xD8D8A0FF      // /susurrar /s
#define COLOR_GRITO             0xFFFFFFFF      // /gritar /g
#define COLOR_ACCION            0xC2A2DAFF      // /me
#define COLOR_DESCRIPCION       0xC2A2DAFF      // /do
#define COLOR_OOC_LOCAL         0xA9A9A9FF      // /b
#define COLOR_OOC_GLOBAL        0xCFCFCFFF      // /ooc
#define COLOR_MP                0xFFDD70FF      // Mensajes privados
#define COLOR_RADIO             0x8AD1E8FF      // Radio de faccion
#define COLOR_DEPARTAMENTO      0x6FB7D8FF      // Canal departamental
#define COLOR_ADMIN_CHAT        0xFFB870FF      // /a
#define COLOR_ADMIN_ACCION      0xFFA645FF      // Avisos de admin
#define COLOR_ANUNCIO           0xFFD24DFF      // /anuncio
#define COLOR_REPORTE           0xE8973AFF      // /reportar
#define COLOR_NEWS              0xF08080FF      // El Teletipo (11/09/2026) -- ya estaba reservado para "Prensa" y sin usar, coral suave, no se confunde con ningun otro color activo de la paleta
#define HEX_NEWS                "{F08080}"

// ---------------------------------------------------------------------
//  IDENTIFICADORES DE DIALOGO
//  Nunca repitas un numero: cada dialogo del servidor tiene el suyo.
// ---------------------------------------------------------------------
enum
{
    DIALOGO_NADA = 0,

    // Login y registro
    DIALOGO_LOGIN = 1000,
    DIALOGO_REGISTRO_PASS,
    DIALOGO_REGISTRO_CONFIRMAR,
    DIALOGO_REGISTRO_EMAIL,
    DIALOGO_REGISTRO_SEXO,
    DIALOGO_REGISTRO_EDAD,
    DIALOGO_REGISTRO_ORIGEN,
    DIALOGO_NORMAS,

    // Cuenta
    DIALOGO_CUENTA = 1100,
    DIALOGO_CUENTA_ESTADISTICAS,
    DIALOGO_CUENTA_CAMBIAR_PASS,
    DIALOGO_CUENTA_CAMBIAR_PASS2,
    DIALOGO_CUENTA_EMAIL,
    DIALOGO_CUENTA_AJUSTES,
    DIALOGO_CUENTA_SANCIONES,

    // Inventario
    DIALOGO_INVENTARIO = 1200,
    DIALOGO_INVENTARIO_OPCIONES,
    DIALOGO_INVENTARIO_CANTIDAD,
    DIALOGO_INVENTARIO_DAR,
    DIALOGO_INVENTARIO_TIENDA,

    // Trabajos y facciones
    DIALOGO_TRABAJOS = 1300,
    DIALOGO_TRABAJO_ACEPTAR,
    DIALOGO_MIS_TRABAJOS,
    DIALOGO_TRABAJO_DEJAR,
    DIALOGO_TRABAJO_REEMPLAZAR,
    DIALOGO_FACCION_MIEMBROS = 1350,
    DIALOGO_FACCION_RANGOS,
    DIALOGO_FACCION_GESTION,

    // Vehiculos y concesionario
    DIALOGO_CONCE_CATEGORIAS = 1400,
    DIALOGO_CONCE_LISTA,
    DIALOGO_CONCE_CONFIRMAR,
    DIALOGO_CONCE_COLOR,
    DIALOGO_CONCE_COLOR_MANUAL,
    DIALOGO_VEHICULO_VENDER,
    DIALOGO_TALLER,

    // Banco
    DIALOGO_BANCO = 1500,
    DIALOGO_BANCO_INGRESAR,
    DIALOGO_BANCO_RETIRAR,
    DIALOGO_BANCO_TRANSFERIR_DESTINO,
    DIALOGO_BANCO_TRANSFERIR_CANTIDAD,
    DIALOGO_BANCO_TRANSFERIR_CONCEPTO,
    DIALOGO_BANCO_TRANSFERIR_CONFIRMAR,
    DIALOGO_BANCO_HISTORIAL,

    // Administracion
    DIALOGO_ADMIN = 1600,
    DIALOGO_ADMIN_JUGADORES,
    DIALOGO_ADMIN_FICHA,
    DIALOGO_ADMIN_DARADMIN,
    DIALOGO_ADMIN_LOGS,
    DIALOGO_ADMIN_REPORTES,

    // Ayuda
    DIALOGO_AYUDA = 1700,
    DIALOGO_AYUDA_SECCION,
    DIALOGO_PAYDAY,
    DIALOGO_SISTEMAS,
    DIALOGO_SISTEMAS_DETALLE,
    DIALOGO_ACTIVIDADES,
    DIALOGO_ACTIVIDADES_DETALLE,

    // Taxi (18_taxi.inc)
    DIALOGO_TAXI = 1800,
    DIALOGO_TAXI_PANEL,
    DIALOGO_TAXI_STATS,
    DIALOGO_TAXI_LOGROS,
    DIALOGO_TAXI_RANKING,
    DIALOGO_TAXI_AYUDA,

    // Basurero (19_basurero.inc)
    DIALOGO_BASURERO = 1900,
    DIALOGO_BASURERO_PANEL,
    DIALOGO_BASURERO_LOGROS,
    DIALOGO_BASURERO_RANKING,
    DIALOGO_BASURERO_AYUDA,

    // Desguace clandestino (20_desguace.inc)
    DIALOGO_DESGUACE = 2000,
    DIALOGO_DESGUACE_PANEL,
    DIALOGO_DESGUACE_STATS,
    DIALOGO_DESGUACE_LOGROS,
    DIALOGO_DESGUACE_RANKING,
    DIALOGO_DESGUACE_AYUDA,

    // Camionero (21_camionero.inc)
    DIALOGO_CAMIONERO = 2100,
    DIALOGO_CAMIONERO_PANEL,
    DIALOGO_CAMIONERO_CARGAS,
    DIALOGO_CAMIONERO_STATS,
    DIALOGO_CAMIONERO_LOGROS,
    DIALOGO_CAMIONERO_RANKING,
    DIALOGO_CAMIONERO_AYUDA,

    // Medicos Los Santos (36_medicos.inc)
    DIALOGO_MEDICOS_LLAMADAS = 2200,

    // Panel de administracion (23_panel.inc)
    DIALOGO_PANEL = 2300,
    DIALOGO_PANEL_JUGADORES,
    DIALOGO_PANEL_JUGADOR_ACCIONES,
    DIALOGO_PANEL_JUGADOR_MOTIVO,
    DIALOGO_PANEL_JUGADOR_DURACION,
    DIALOGO_PANEL_JUGADOR_LISTA,
    DIALOGO_PANEL_REPORTES,
    DIALOGO_PANEL_SANCIONES,
    DIALOGO_PANEL_CUENTA_BUSCAR,
    DIALOGO_PANEL_CUENTA_RESULTADOS,
    DIALOGO_PANEL_CUENTA_FICHA,
    DIALOGO_PANEL_CUENTA_ACCIONES,
    DIALOGO_PANEL_CUENTA_VALOR,
    DIALOGO_PANEL_CUENTA_MOTIVO,
    DIALOGO_PANEL_COORDS,
    DIALOGO_PANEL_COORD_ACCIONES,
    DIALOGO_PANEL_COORD_GUARDAR,
    DIALOGO_PANEL_CASAS,
    DIALOGO_PANEL_CASA_FICHA,
    DIALOGO_PANEL_CASA_ACCIONES,
    DIALOGO_PANEL_CASA_PRECIO,
    DIALOGO_PANEL_CASA_TRANSFERIR,
    DIALOGO_PANEL_CASA_BORRAR_CONFIRMAR,
    DIALOGO_PANEL_STAFF,
    DIALOGO_PANEL_STAFF_ACCIONES,
    DIALOGO_PANEL_VEHICULOS,
    DIALOGO_PANEL_VEHICULO_FICHA,
    DIALOGO_PANEL_VEHICULO_ACCIONES,
    DIALOGO_PANEL_VEHICULO_BORRAR_CONFIRMAR,

    // Casas (24_casas.inc)
    DIALOGO_CASAS = 2400,
    DIALOGO_CASA_COMPRAR_CONFIRMAR,
    DIALOGO_CASA_VENDER_CONFIRMAR,
    DIALOGO_CASA_GUARDAR_CANTIDAD,
    DIALOGO_CASA_SACAR_CANTIDAD,
    DIALOGO_CASA_CREAR_INTERIOR,
    DIALOGO_CASA_CREAR_NOMBRE,
    DIALOGO_CASA_CREAR_PRECIO,
    DIALOGO_CASA_ARMARIO,

    // Gasolinera / tienda de 24h (27_tienda.inc)
    DIALOGO_TIENDA = 2700,

    // GPS (30_gps.inc)
    DIALOGO_GPS = 2800,
    DIALOGO_GPS_DESTINO,

    // Negocios (32_negocios.inc)
    DIALOGO_NEGOCIO_COMPRAR_CONFIRMAR = 3200,
    DIALOGO_NEGOCIO_CAJAFUERTE,
    DIALOGO_NEGOCIO_CAJAFUERTE_SACAR,

    // Policia Nacional (37_policia.inc). Bloque 2500: estaba libre --
    // los usados eran 1000-2400, 2700, 2800 y 3200.
    DIALOGO_POLICIA_CODIGO = 2500,
    DIALOGO_POLICIA_MULTA,
    DIALOGO_POLICIA_ANTECEDENTES,
    DIALOGO_POLICIA_BUSCADOS,
    DIALOGO_POLICIA_AVISOS,

    // Comida rapida (42_comida.inc)
    DIALOGO_COMIDA_MENU = 4200,

    // Los coches de Federico (43_federico.inc)
    DIALOGO_FEDERICO = 4300,
    DIALOGO_FEDERICO_HISTORICO, // menu de /crearhistorico, 05/09/2026

    // Maletero de vehiculo (45_maletero.inc)
    DIALOGO_MALETERO = 4400,
    DIALOGO_MALETERO_ACCION,
    DIALOGO_MALETERO_CANTIDAD,

    // Guia ingame (48_guia.inc)
    DIALOGO_GUIA = 4800,

    // Guia de vehiculos (49_guiavehiculos.inc)
    DIALOGO_GUIA_VEHICULOS = 4900,

    // Casa de Empeños / Pawnshop (50_pawnshop.inc)
    DIALOGO_EMPENOS = 5000,
    DIALOGO_EMPENOS_ACCION,
    DIALOGO_EMPENOS_CANTIDAD,
    DIALOGO_EMPENOS_CONFIRMAR,

    // Gamerol Aventuras: DLC Pesca (51_pesca.inc, 06/09/2026)
    DIALOGO_PESCA_COCINAR = 5100,
    DIALOGO_PESCA_COCINAR_RECETA,
    DIALOGO_PESCA_VENDER,
    DIALOGO_PESCA_VENDER_CONFIRMAR,
    DIALOGO_PESCA_STATS,
    DIALOGO_PESCA_RANKING,

    // Perrito Caliente (52_perritos.inc, 07/09/2026)
    DIALOGO_PERRITO,

    // Ascensos / Server IA (58_ascensos.inc, 08/09/2026)
    DIALOGO_ASCENSOS = 5800,

    // /verlista (43_federico.inc, 08/09/2026)
    DIALOGO_FEDERICO_VERLISTA = 5900,

    // Hospital Interior - /tratamiento medico (64_hospitalinterior.inc, 14/09/2026)
    DIALOGO_HOSPITAL_TRATAMIENTO = 6400,

    // CRONOS GAMEROL -- desafios contrarreloj (65_cronos.inc, 14/09/2026)
    DIALOGO_CRONOS_LISTA = 6500,
    DIALOGO_CRONOS_RANKING
};

// ---------------------------------------------------------------------
//  MUNDOS VIRTUALES RESERVADOS
// ---------------------------------------------------------------------
#define MUNDO_NORMAL            0
#define MUNDO_LOGIN             30000   // Camara de la pantalla de inicio
#define MUNDO_CREACION          30001   // Sala de creacion de personaje
#define MUNDO_BANCO             500
#define MUNDO_TIENDA            501
#define MUNDO_BURGERSHOT        502
#define MUNDO_CLUCKINBELL       503
#define MUNDO_PIZZA             504
#define MUNDO_EMPENOS           505
// Interior del Hospital (64_hospitalinterior.inc) -- movido aqui desde el
// propio modulo el 13/09/2026: GPS (30_gps.inc) y el trabajo de Medicos
// (11_trabajos.inc) lo necesitan y ambos se incluyen ANTES que
// 64_hospitalinterior.inc en malagacity.pwn (un #define, a diferencia de
// una funcion, solo esta disponible DESPUES de su propia linea).
#define MUNDO_HOSPITAL_INTERIOR 507
// Mundo virtual PROPIO de cada sucursal de comida rapida (una por
// sucursal, no por cadena) -- arreglado el 07/09/2026: antes todas las
// sucursales de una misma cadena compartian g_ComidaMundo[cadena], asi
// que /salir en Clucking Bell 2 te sacaba por la puerta de Clucking
// Bell 1. Rango disjunto de cualquier otro mundo del proyecto, hueco
// para MAX_SUCURSALES_COMIDA (40) sucursales.
#define MUNDO_COMIDA_SUCURSAL_BASE 10000
// Los taxis se mueven por el mundo normal. No reservamos mundo virtual
// aparte porque el jugador convive con el resto de trafico.

// ---------------------------------------------------------------------
//  TRABAJO DE VENDEDOR DE ARMAS (25_vendedorarmas.inc)
//  ---------------------------------------------------------------------
//  Sistema copiado 1:1 de NG-RP (Next Generation Roleplay), traducido.
//  No es un trabajo de punto unico: son tres coordenadas, todas puestas
//  a mano desde /panel > Coordenadas por Vlad Jailovic el 2026-08-13.
//
//    1. Punto de contratacion: icono "i", cartel "Vendedor de Armas".
//    2. Punto de pedir materiales (/pedirmats): pagas y te dan checkpoint.
//    3. Punto de recepcion: al llegar, los materiales se acreditan solos.
//
//  Las armas se dan directas a la mano (GivePlayerWeapon), no pasan por
//  el inventario. El "material" si es un item normal (ITEM_MATERIAL,
//  fila "Materiales" en 10_inventario.inc), asi que se puede robar,
//  tirar o transferir como cualquier otro objeto.
// ---------------------------------------------------------------------
#define TRABAJO_VENDEDOR_ARMAS   1       // Ranura en g_Trabajos

// 1. Contratacion (icono "i" + cartel rojo "Vendedor de Armas" / /aceptarjob)
#define ARMAS_JOB_X              2076.96
#define ARMAS_JOB_Y              -2046.82
#define ARMAS_JOB_Z              13.5469
#define ARMAS_JOB_A              70.4575

// 2. Pedir materiales (/pedirmats)
#define ARMAS_MATS_PEDIR_X       1423.71
#define ARMAS_MATS_PEDIR_Y       -1306.32
#define ARMAS_MATS_PEDIR_Z       13.2818
#define ARMAS_MATS_RADIO_PEDIR   3.0
#define ARMAS_MATS_PRECIO        500     // $ por cada mat run
#define ARMAS_MATS_CANTIDAD      1000    // Materiales que da cada run

// 3. Recepcion automatica al llegar
#define ARMAS_MATS_RECIBIR_X     2168.72
#define ARMAS_MATS_RECIBIR_Y     -2274.94
#define ARMAS_MATS_RECIBIR_Z     13.1276
#define ARMAS_MATS_RADIO_RECIBIR 6.0

// Venta de armas
#define ARMAS_VENTA_ESPERA       10      // Segundos de espera entre ventas
#define ARMAS_OFERTA_RADIO       8.0     // Distancia maxima para ofrecer/aceptar
// FIX 06/09/2026 (encontrado por Vlad: cogia Materiales de un maletero y
// /venderarma decia que no tenia): este numero estaba mal, apuntaba a la
// fila 29 de g_Items, que es "Bote de spray", no "Materiales" (fila 31 de
// verdad). Asi /pedirmats, /venderarma y el botin de maleteros
// (45_maletero.inc, que ya usaba Item_Buscar("Materiales") y por eso
// nunca tuvo el bug) quedan los tres apuntando al mismo item real.
#define ARMAS_ITEM_MATERIALES    31      // Fila de "Materiales" en g_Items (10_inventario.inc)

// ---------------------------------------------------------------------
//  TRABAJO NFS (26_nfs.inc)
//  ---------------------------------------------------------------------
//  Trafico de coches libres. Un NFS marca un coche libre SIN dueno con
//  /nfscoche (solo en el taller NFS, sentado dentro) y a partir de ahi
//  solo se puede comprar a traves de un NFS con /nfsvendercoche. Un NFS
//  jamas puede anular los papeles de un coche que ya tiene dueno.
//
//  Coordenadas puestas a mano desde /panel > Coordenadas el 2026-08-13.
// ---------------------------------------------------------------------
#define TRABAJO_NFS              2       // Ranura en g_Trabajos
#define NFS_SKIN                 29

// Contratacion (icono + cartel "NFS" / /aceptarjob)
#define NFS_JOB_X                559.215
#define NFS_JOB_Y                -1288.8
#define NFS_JOB_Z                17.2482
#define NFS_JOB_A                178.75

// Recinto NFS -- 11/09/2026, punto distinto de la contratacion de arriba
// (tabla `coordenadas`, "recinto NFS"), solo para /gps (seccion Trabajos,
// 30_gps.inc). Peticion expresa de Vlad.
#define NFS_RECINTO_X            558.895
#define NFS_RECINTO_Y            -1256.38
#define NFS_RECINTO_Z            16.8039

// Taller NFS: unico taller del sistema de dropeo (el taller clandestino
// original se elimino, todo pasa por aqui). /nfscoche es solo para
// miembros NFS; /taller, /dropearcoche y /renovarpapeles son para
// cualquiera, ver 22_dropeo.inc.
#define NFS_TALLER_X             529.244
#define NFS_TALLER_Y             -1287.2
#define NFS_TALLER_Z             17.2422
#define NFS_TALLER_A             183.137
#define NFS_TALLER_RADIO         10.0
#define NFS_RENOVAR_PRECIO       5000

// Venta de coches marcados NFS (/nfsvendercoche ID PRECIO + /aceptarnfs)
#define NFS_VENTA_MINIMA         30000   // Precio minimo que puede pedir un NFS
#define NFS_VENTA_PAGO_NFS       5000    // Fijo, sea cual sea el precio de venta
#define NFS_OFERTA_RADIO         8.0     // Ambos tienen que estar en el mismo coche, esto es solo cinturon de seguridad

// Papeles de 14 dias vendidos por un NFS (/nfspapeles ID + /aceptarpapeles)
#define NFS_PAPELES_PRECIO       3000    // Fijo, va integro al NFS
#define NFS_PAPELES_DIAS         14
#define NFS_PAPELES_HORAS_MAX    24      // Solo si le quedan menos de 24h de papeles

// ---------------------------------------------------------------------
//  TRABAJO VENDEDOR DE MARIHUANA (29_vendedormarihuana.inc)
//  ---------------------------------------------------------------------
//  Reparto a clientes NPC por la ciudad. No tiene nada que ver con el
//  cultivo libre de 28_marihuana.inc (ese sigue igual): este trabajo
//  vende la cosecha a clientes generados por el servidor en vez de a
//  otros jugadores.
//
//  Coordenadas puestas a mano desde /panel > Coordenadas:
//    - Contratacion: clave "posventaherb".
//    - Cabina telefonica: clave "posempezarventasherb".
// ---------------------------------------------------------------------
#define TRABAJO_VENDEDOR_MARIHUANA   3       // Ranura en g_Trabajos

// 1. Contratacion (icono "i" + cartel / /aceptarjob)
#define MVEND_JOB_X              2166.6
#define MVEND_JOB_Y              -1671.94
#define MVEND_JOB_Z              15.0745
#define MVEND_JOB_A              42.5682

// 2. Cabina telefonica: aqui se activa/desactiva el modo vendedor con /vendedor
#define MVEND_CABINA_X           1807.96
#define MVEND_CABINA_Y           -1599.24
#define MVEND_CABINA_Z           13.5469
#define MVEND_CABINA_A           39.5891
#define MVEND_CABINA_RADIO       3.0

// Ritmo de las ofertas y venta
#define MVEND_OFERTA_PRIMERA     15      // segundos hasta la primera oferta al activar /vendedor
#define MVEND_OFERTA_SIGUIENTE   60      // segundos hasta la siguiente oferta tras resolver una (completar, cancelar o que se acabe el tiempo)
#define MVEND_OFERTA_TIEMPO      90      // segundos para llegar al cliente tras aceptar
#define MVEND_GRAMOS_MIN         20
#define MVEND_GRAMOS_MAX         100
#define MVEND_PRECIO_GRAMO       16      // $ por gramo vendido al cliente NPC (doblado, +40%, y +15% mas a peticion expresa 2026-08-16; 14 * 1.15 = 16.1 -> 16)

// ---------------------------------------------------------------------
//  TRABAJO MEDICOS LOS SANTOS (36_medicos.inc)
//  ---------------------------------------------------------------------
//  RETIRADO el 07/09/2026 (peticion expresa de Vlad) el sistema de
//  heridos que vivia aqui: ya no hay caida congelada, animacion, texto
//  en pantalla, /automatico ni /llamadas. Morir es de serie SA-MP otra
//  vez (OnPlayerDeath/OnPlayerSpawn, 99_callbacks.inc). El trabajo de
//  Medicos ya no tiene relacion con morir -- lo unico que hace es
//  /curar [id] [precio], ver 36_medicos.inc. Los defines de abajo que ya
//  no usa nadie se quedan sin tocar (deuda inofensiva, sin warnings) por
//  no ampliar el alcance de este cambio.
// ---------------------------------------------------------------------
#define TRABAJO_MEDICOS          5       // Ranura en g_Trabajos

#define MEDICOS_RADIO_ATENDER    4.0     // distancia maxima para /curar
#define MEDICOS_AUTOMATICO_ESPERA 5      // ya sin uso, ver nota de arriba
#define MEDICOS_SALUD_TRATADO    70.0    // ya sin uso, ver nota de arriba
#define MEDICOS_SALUD_AUTOMATICO 45.0    // ya sin uso, ver nota de arriba
#define MEDICOS_SALUD_MEDICO     70.0    // ya sin uso, ver nota de arriba
#define MEDICOS_FACTURA          500     // ya sin uso, ver nota de arriba

#define CURAR_PRECIO_MIN         1000    // $ minimo que puede pedir un Medico por /curar
#define CURAR_PRECIO_MAX         2000    // $ maximo que puede pedir un Medico por /curar

// Texto grande en pantalla, ya sin uso (ver nota de arriba).
#define MEDICOS_TEXTO_PANTALLA  "~r~HERIDO~n~~w~/automatico~n~~w~o espera a los ~g~MEDICOS"
#define MEDICOS_TEXTO_TIEMPO    1200

// ---------------------------------------------------------------------
//  PANEL WEB (31_web.inc)
//  ---------------------------------------------------------------------
//  Acciones de pago que un jugador puede pedir desde la web. Se cobran
//  siempre de la cuenta dentro del juego (dinero en mano), en caliente
//  si esta conectado.
// ---------------------------------------------------------------------
// Renovar por web cuesta mas que ir al taller de la NFS (NFS_RENOVAR_PRECIO,
// 5.000 $) porque te ahorras el viaje, pero 50.000 $ era un 10x -- un peaje,
// no una comodidad. Bajado a 12.000 $ (2,4x) el 2026-08-19, ver ECONOMIA.md.
// Duplicado en "web servidor/server.js" (PRECIO_RENOVAR): cambia los dos.
#define WEB_RENOVAR_PRECIO      12000   // $ por renovar papeles desde la web
#define WEB_RENOVAR_DIAS        20      // dias que se añaden
#define WEB_COSECHA_PRECIO      1000    // $ por cosechar una planta a distancia desde la web

// Avisos de WhatsApp para el dueno del servidor (09/09/2026, encargo
// directo de Vlad): cuenta nueva registrada y jugador conectado. SIEMPRE
// van a esta cuenta -- es un aviso de administracion, no una preferencia
// por jugador como el resto de wa_avisos_cola. cuenta_id 1 = Vlad_Jailovic,
// confirmado contra la base de datos real el mismo dia.
#define CUENTA_DUENO_AVISOS     1

// Portatil Hacker (45_maletero.inc / 31_web.inc, 05/09/2026). Se cobra
// del BANCO, no del efectivo en mano -- mismo criterio que renovar/
// cosechar (31_web.inc, cabecera del archivo): son acciones pensadas
// para hacerse offline o lejos del teclado, y el efectivo en mano no es
// seguro de tocar sin que el jugador este conectado. El master prompt
// decia solo "paga 5.000$" sin especificar banco o mano -- incompatibilidad
// documentada aqui, se resuelve con el mismo criterio ya establecido.
#define HACK_PRECIO             5000    // $ por cada ejecucion de hackeo, cobrado del banco
#define HACK_COOLDOWN_SEGUNDOS  3600    // 1 hora, independiente por tipo
#define CANCELA_PRECIO_COMPRA   200000  // $ de la PRIMERA cancela, comprada desde la web
#define CANCELA_INCREMENTO      100000  // $ de mas por cada cancela extra (2a = +100k, 3a = +200k...) -- sin limite de cuantas
#define CANCELA_PRECIO_MOVER    150000  // $ por pedir el traslado de una cancela ya instalada
#define CAMARA_PRECIO            50000  // $ de la camara de seguridad, extra de UNA cancela ya instalada
#define CAMARA_RADIO_AVISO       10.0   // metros: aviso IC de que la cancela esta vigilada
#define CAMARA_RADIO_DETECCION   20.0   // metros: registra nombre + matricula de quien entra, sin distinguir disfraces
#define CAMARA_COOLDOWN_SEGUNDOS 120    // segundos antes de repetir aviso/registro de la misma persona en la misma camara
#define RASTREADOR_PRECIO           50000  // $ del rastreador GPS, extra de UN vehiculo VEH_PERSONAL
#define RASTREADOR_INTERVALO_SEGUNDOS 10   // cada cuantos segundos se actualiza la posicion en el mapa web
#define ALARMA_PRECIO            20000  // $ de la alarma, extra de UN vehiculo propio (VEH_PERSONAL o VEH_DROPEO)
#define ALARMA_DURACION_SEGUNDOS 25     // cuanto suena la alarma nativa antes de apagarse sola

// ---------------------------------------------------------------------
//  PUNTOS DEL MAPA
//  Se pueden mover en caliente con los comandos de administracion,
//  estos son solo los valores por defecto de la primera ejecucion.
// ---------------------------------------------------------------------

// Spawn de personaje nuevo. Puesto a mano desde /panel > Coordenadas
// (clave "spawn") el 2026-08-13.
#define SPAWN_X                 1639.67
#define SPAWN_Y                 -1066.56
#define SPAWN_Z                 23.8984
#define SPAWN_A                 175.851

// Hospital al que se reaparece tras morir (All Saints General).
// Coordenada recalibrada por Vlad el 11/09/2026 via /panel ("respawn al
// morir") -- ahora SI esta conectada de verdad al respawn tras morir,
// ver Spawn_RefrescarDatosMuerte (03_utiles.inc) y OnPlayerDeath.
#define HOSPITAL_X              1183.27
#define HOSPITAL_Y              -1323.69
#define HOSPITAL_Z              13.5774
#define HOSPITAL_A              89.335

// Puerta de calle del interior nuevo del hospital (64_hospitalinterior.inc)
// -- mismo edificio que HOSPITAL_X/Y/Z de arriba, pero el punto exacto del
// /entrar. Vive aqui (no en el propio modulo) por el mismo motivo que
// MUNDO_HOSPITAL_INTERIOR arriba: GPS necesita llevar hasta aqui a
// Medicos/Paramedico. Marcada por Vlad el 13/09/2026 en /panel >
// Coordenadas (clave "ppppositionnn").
#define HOSPITAL_ENTRADA_X       1172.8
#define HOSPITAL_ENTRADA_Y       -1323.48
#define HOSPITAL_ENTRADA_Z       15.3993
#define HOSPITAL_ENTRADA_A       80.4394

// Punto de contratacion de Medicos Los Santos (11_trabajos.inc), ahora
// DENTRO del interior del hospital -- antes era HOSPITAL_X/Y/Z, en la
// calle. Pedido expreso de Vlad 13/09/2026, coordenada marcada en /panel
// (clave "positiom").
#define MEDICOS_JOB_X            2571.67
#define MEDICOS_JOB_Y            1292.88
#define MEDICOS_JOB_Z            866.276

// Carcel OOC para sanciones administrativas.
#define CARCEL_X                264.5
#define CARCEL_Y                77.5
#define CARCEL_Z                1001.0
#define CARCEL_INTERIOR         6

// ---------------------------------------------------------------------
//  POLICIA NACIONAL (37_policia.inc)
//  ---------------------------------------------------------------------
//  La facción vive en la ranura 1 de 12_facciones.inc. Estas constantes
//  son solo de la MECANICA: buscados, multas y carcel de rol.
// ---------------------------------------------------------------------
#define POLICIA_FACCION          1      // Ranura de faccion de la Policia Nacional

// Rangos minimos. El Aspirante (rango 1) acompaña pero no actua: no
// puede imponer delitos, multar ni detener.
#define POLICIA_RANGO_AGENTE     2      // Policia: ya puede actuar
#define POLICIA_RANGO_MANDO      8      // Comisario: puede perdonar

#define POLICIA_DISTANCIA        5.0    // Metros para esposar, registrar, detener

// Caducidad de los puntos de busqueda: se baja uno cada X minutos de
// juego. Si te escondes y no reincides, dejas de estar buscado solo.
// Los ANTECEDENTES no caducan nunca -- son otra cosa.
#define POLICIA_MINUTOS_CADUCA   30

// Multas. El importe sale del codigo penal, no lo elige el agente.
#define POLICIA_MULTA_ESPERA     90     // Segundos que aguanta la oferta
// Tope para cuentas nuevas: un novato no puede llevarse una multa que
// le arruine el primer dia. Idea tomada de NG-RP.
#define POLICIA_NOVATO_MINUTOS   300    // 5 horas de juego
#define POLICIA_NOVATO_MAX       5000   // $ maximo de multa a un novato

// Tope de puntos, por si alguien acumula media docena de delitos.
#define POLICIA_MAX_PUNTOS       30

// ---------------------------------------------------------------------
//  AVISOS A LA CENTRAL
//  ---------------------------------------------------------------------
//  Los sistemas ilegales que ya existen (cultivo, NFS, dropeo, armas y
//  el caido de Medicos) avisan a la policia. Es lo que le da trabajo a
//  la faccion sin inventar contenido nuevo.
//
//  DOS FRENOS CONTRA EL RUIDO, desde el primer dia:
//    1. PROBABILIDAD -- no todos los delitos se detectan.
//    2. AGRUPACION   -- si ya hay un aviso del mismo tipo cerca y
//       reciente, no se crea otro: se le suma un caso al que ya habia.
//  Sin esto, quien riegue diez plantas llena la central de spam y los
//  agentes dejan de mirarla. Es la misma leccion del centro de alertas
//  del panel web.
// ---------------------------------------------------------------------
#define MAX_AVISOS_POLICIA       24

#define AVISO_CULTIVO            1      // Cosechar marihuana (28_marihuana)
#define AVISO_ROBO_COCHE         2      // Robar un coche ajeno (22_dropeo)
#define AVISO_NFS                3      // Forzar un coche libre (26_nfs)
#define AVISO_ARMAS              4      // Entregar un arma (25_vendedorarmas)
#define AVISO_TIROTEO            5      // Alguien cae herido (36_medicos)

// Probabilidad de que cada cosa se detecte, en porcentaje. Un tiroteo
// se oye casi siempre; regar una planta en un descampado, casi nunca.
#define PROB_AVISO_CULTIVO       35
#define PROB_AVISO_ROBO_COCHE    55
#define PROB_AVISO_NFS           45
#define PROB_AVISO_ARMAS         40
#define PROB_AVISO_TIROTEO       80

// Un aviso sin resolver se cae solo pasados estos minutos: la central
// no puede acumular trabajo de hace media hora.
#define POLICIA_AVISO_CADUCA     10

// Radio en metros dentro del cual dos avisos del mismo tipo se
// consideran el mismo suceso.
#define POLICIA_AVISO_AGRUPA     220.0

// Aviso de un ciudadano llamando al 112 (/911).
#define AVISO_LLAMADA            6
#define POLICIA_911_ESPERA       60     // Segundos entre llamada y llamada

// Atraco a un local (39_atracos.inc). Se ve casi siempre: hay testigos.
#define AVISO_ATRACO             7
#define PROB_AVISO_ATRACO        65

// Forzar el maletero de un vehiculo ajeno (45_maletero.inc, 05/09/2026).
// La probabilidad esta fijada a 60 a proposito por el encargo -- no es
// una estimacion de "que tan visible es la accion" como las de arriba,
// es LA cifra exacta pedida: "60% de probabilidad de alertar a la
// policia... independiente de si el forzado termina teniendo exito".
// Se tira una vez por cada /forzarmaletero, siempre, aparte del 33% de
// exito del forzado en si.
#define AVISO_MALETERO           8
#define PROB_AVISO_MALETERO      60

// Puentear con exito un coche LIBRE sin dueno (22_dropeo.inc, 05/09/2026,
// peticion expresa de Vlad). A diferencia de arriba, esta SOLO se tira
// si el puente sale bien -- un intento fallido no alerta a nadie.
#define AVISO_PUENTE_LIBRE       9
#define PROB_AVISO_PUENTE_LIBRE  60

// Forzar la cerradura de un Coche de Jugador buscado por Federico
// (59_federicojugadores.inc, 08/09/2026). Mismo criterio que
// AVISO_MALETERO: se tira siempre al resolver el intento, exito o
// fallo, independiente del 60% de exito del forzado en si.
#define AVISO_CERRADURA          10
#define PROB_AVISO_CERRADURA     60

// Constantes de /forzarmaletero -- vuelto a 4s el 06/09/2026 (habia
// subido a 8s antes, Vlad prefirio revertirlo), con cuenta atras en
// pantalla igual (Maletero_ForzarTick, 45_maletero.inc). Probabilidad
// igualada a /puente (DROPEO_PUENTE_EXITO), aunque siguen siendo dos
// constantes independientes por si un dia se separan otra vez. Subida al
// 60% el 08/09/2026 (antes 42%).
#define FORZAR_MALETERO_FREEZE_MS   4000    // 4s, con cuenta atras en pantalla
#define FORZAR_MALETERO_EXITO       60      // % de exito por intento
#define FORZAR_MALETERO_VENTANA     60      // segundos de acceso tras forzarlo con exito

// ---------------------------------------------------------------------
//  CARNET DE CONDUCIR POR PUNTOS
//  ---------------------------------------------------------------------
//  Lo expide la DGT (ranura 1 de 32_negocios.inc), que hasta ahora solo
//  cobraba de /comprarcoche. Como en España: 12 puntos, los delitos de
//  trafico (codigos 1xx) los van restando, y a 0 se retira el carnet.
// ---------------------------------------------------------------------
#define CARNET_PUNTOS_INICIALES  12
#define CARNET_PRECIO            8000    // $ que cuesta sacarselo en la DGT
#define CARNET_DISTANCIA         3.0     // Metros al mostrador de la DGT

// Radar de velocidad de la policia.
#define RADAR_LIMITE             80      // km/h a partir de los cuales hay multa
#define RADAR_ALCANCE            60.0    // Metros que abarca el radar

// ---------------------------------------------------------------------
//  LA COMISARIA (37_policia.inc)
//  ---------------------------------------------------------------------
//  Es el interior 6 del juego, la comisaria de Los Santos de toda la
//  vida. NO es un mapeo de objetos: no hace falta, el interior ya
//  existe y no cuesta ni un objeto al streamer.
//
//  Y se eligio ese y no otro por un motivo concreto: LAS CELDAS YA
//  ESTAN AHI. CARCEL_X/Y/Z apuntan a 264.5, 77.5, 1001.0 con
//  CARCEL_INTERIOR 6 -- son los calabozos de esta misma comisaria. Asi
//  que la detencion deja de ser un teletransporte a ninguna parte: el
//  agente entra por la puerta de Pershing Square, cruza el vestibulo y
//  mete al detenido en una celda que esta treinta metros mas alla.
//
//  MUNDO VIRTUAL 0 (MUNDO_NORMAL), a proposito. El banco y la tienda
//  tienen mundo propio porque son sitios de tramite donde da igual
//  quien mas haya; una comisaria es justo lo contrario: si cada uno
//  entrara a su propia copia no habria denuncias, ni esperas, ni nadie
//  a quien pedirle explicaciones en el mostrador.
// ---------------------------------------------------------------------

// Puerta de la calle. Es el MISMO punto que la sede de la faccion en
// 12_facciones.inc: el marcador de /facciones y la puerta son la misma
// cosa. Si mueves uno, mueve el otro.
#define COMISARIA_EXT_X          1554.9
#define COMISARIA_EXT_Y          -1675.6
#define COMISARIA_EXT_Z          16.2
#define COMISARIA_EXT_A          90.0    // Mirando a la calle al salir

// Punto de llegada dentro. Es la entrada estandar del interior 6.
#define COMISARIA_INTERIOR       6
#define COMISARIA_INT_X          246.7818
#define COMISARIA_INT_Y          62.8978
#define COMISARIA_INT_Z          1003.6406
#define COMISARIA_INT_A          0.0     // Mirando al mostrador al entrar

// ---------------------------------------------------------------------
//  LA ENTRADA TRASERA
//  ---------------------------------------------------------------------
//  El garaje de atras. SOLO para gente de la faccion -- Aspirantes
//  incluidos, que para entrar a trabajar no hace falta rango.
//
//  Es la puerta por la que se mete a un detenido: te ahorra cruzar el
//  vestibulo publico esposando a alguien delante de quien haya venido a
//  denunciar, y deja al coche patrulla aparcado donde va, detras.
//
//  La coordenada de la calle la midio el dueño del servidor dentro del
//  juego el 20/08/2026 y la guardo como "pos1 -- entrada trasera
//  policias. /entrar". El punto de llegada de dentro es uno de los
//  puntos despejados de la zona interna (pasado el control, entre la
//  armeria y el pasillo de celdas); si al probarlo se cae mal, se saca
//  otro con /pos y se cambia aqui.
// ---------------------------------------------------------------------
#define COMISARIA_TRAS_X         1524.74
#define COMISARIA_TRAS_Y         -1677.84
#define COMISARIA_TRAS_Z         5.89062
#define COMISARIA_TRAS_A         92.2151

#define COMISARIA_TRASINT_X      258.6202
#define COMISARIA_TRASINT_Y      79.8815
#define COMISARIA_TRASINT_Z      1003.6406
#define COMISARIA_TRASINT_A      180.0

// ---------------------------------------------------------------------
//  LAS PUERTAS DEL CONTROL DE ACCESO
//  ---------------------------------------------------------------------
//  Dos hojas que separan el vestibulo publico de la zona interna. El
//  vestibulo lo pisa CUALQUIERA -- una comisaria donde no puedes entrar
//  a denunciar no sirve de nada. Lo que hace falta ser agente es para
//  pasar de ahi.
//
//  Las coordenadas salen del SpaceRol de 2010, que ya las tenia
//  medidas. Alli estaban creadas con CreateDynamicObject SIN mundo ni
//  interior (las lineas buenas estaban comentadas justo encima), asi
//  que las puertas aparecian flotando en TODOS los interiores del
//  servidor. Aqui van con su mundo y su interior, como los pickups del
//  banco.
// ---------------------------------------------------------------------
#define COMISARIA_PUERTA_MODELO  1495

// Hoja izquierda: cerrada y abierta.
#define COMISARIA_PI_CX          244.813507
#define COMISARIA_PI_CY          72.698638
#define COMISARIA_PI_AX          243.743743
#define COMISARIA_PI_AY          72.730651

// Hoja derecha: cerrada y abierta.
#define COMISARIA_PD_CX          246.333999
#define COMISARIA_PD_CY          72.709732
#define COMISARIA_PD_AX          247.347640
#define COMISARIA_PD_AY          72.716026

#define COMISARIA_PUERTA_Z       1002.640747
#define COMISARIA_PUERTA_VEL     2.5     // Velocidad de apertura
#define COMISARIA_PUERTA_DIST    5.0     // Metros para poder accionarlas

// Se cierran solas pasado este tiempo. Una puerta de control que se
// queda abierta porque a alguien se le olvido deja de ser un control.
#define COMISARIA_PUERTA_CIERRE  20      // Segundos

// ---------------------------------------------------------------------
//  LA ARMERIA
//  ---------------------------------------------------------------------
//  El armero de la comisaria (el mismo punto que usaba SpaceRol). Da
//  el equipo REGLAMENTARIO, y lo da segun el rango: es lo unico del
//  escalafon que se nota en la mano, no en un titulo.
//
//  No cura. El chaleco es equipo y sale de aqui; la salud es sanidad y
//  sale de los Medicos (36_medicos.inc), que para eso estan y para eso
//  cobran. Si la comisaria curase gratis, el trabajo de medico se
//  quedaria sin la mitad de sus clientes.
// ---------------------------------------------------------------------
#define ARMERIA_X                255.2966
#define ARMERIA_Y                77.1689
#define ARMERIA_Z                1003.6406
#define ARMERIA_DIST             2.5     // Metros al armario

// Espera entre equipamientos. No es por el paseo hasta aqui: es para
// que nadie se plante dentro y se repita el chaleco cada segundo.
#define ARMERIA_ESPERA           90      // Segundos

// Rangos a partir de los cuales se abre cada cajon.
#define ARMERIA_RANGO_ESCOPETA   4       // Oficial
#define ARMERIA_RANGO_SUBFUSIL   6       // Inspector

// Municion de cada arma reglamentaria.
#define ARMERIA_BALAS_PISTOLA    60
#define ARMERIA_BALAS_ESCOPETA   30
#define ARMERIA_BALAS_SUBFUSIL   100

// ---------------------------------------------------------------------
//  TIEMPOS DE LOS TEMPORIZADORES (milisegundos)
// ---------------------------------------------------------------------
#define TIEMPO_SEGUNDO          1000
#define TIEMPO_MINUTO           60000
#define TIEMPO_GUARDADO         300000  // Guardado automatico: 5 minutos

// ---------------------------------------------------------------------
//  DERRAPE  (38_drift.inc)
//  ---------------------------------------------------------------------
//  Portado del filterscript de derrape del catalogo antiguo. Premia el
//  derrape controlado: cuanto mas angulo y velocidad mantengas dentro
//  de rango, mas puntos se acumulan, y se cobran en cuanto el derrape
//  se corta -- si el coche se ha danado por el camino no se paga nada,
//  para no premiar un choque disfrazado de derrape.
// ---------------------------------------------------------------------
#define DRIFT_INTERVALO_MS       200      // cada cuanto se mide el angulo
#define DRIFT_ANGULO_MINIMO      10.0     // grados minimos de deriva para que cuente
#define DRIFT_ANGULO_MAXIMO      90.0     // por encima de esto ya es un trompo, no cuenta
#define DRIFT_VELOCIDAD_MINIMA   0.4      // magnitud minima del vector de velocidad del vehiculo (unidades de juego, no km/h)
#define DRIFT_SALUD_TOLERANCIA   50.0     // si la salud del coche baja mas que esto durante el derrape, se considera golpe y no se paga
#define DRIFT_DINERO_POR_PUNTO   1        // $ que vale cada punto de derrape acumulado. A 1:1 a proposito: el numero que ves subir en el HUD ("DRIFT $X") es literalmente lo que cobras
#define DRIFT_PUNTOS_MINIMOS     15       // por debajo de esto no se paga nada al cortar (un derrape corto de 1-2s ya llega)
#define DRIFT_PUNTOS_MAXIMOS     4000     // tope de puntos por derrape (evita numeros absurdos)
#define DRIFT_FLASH_PARPADEOS    2        // veces que el "+X$" parpadea en verde al cobrar
#define DRIFT_FLASH_INTERVALO_MS 180       // velocidad del parpadeo

// ---------------------------------------------------------------------
//  TALLER DE TUNING PERSISTENTE  (14_tuning.inc)
//  ---------------------------------------------------------------------
//  No hay tienda propia: se apoya en los talleres nativos de GTA:SA
//  (Transfender, Wheel Arch Angels, Loco Low Co...), que ya estan en el
//  mapa y abren su menu de piezas solos al entrar en el interior. El
//  gamemode solo escucha OnVehicleMod/OnVehiclePaintjob y guarda lo que
//  el jugador ha elegido, para que siga puesto la proxima vez que el
//  coche aparezca. Se guarda para cualquier tipo de vehiculo, dropeo
//  incluido -- ver la cabecera de 14_tuning.inc para el porque.
// ---------------------------------------------------------------------
#define TUNING_SLOTS             16       // Slots 0-15 de GetVehicleComponentInSlot (CARMODTYPE)

// ---------------------------------------------------------------------
//  ATRACOS A LOCALES  (39_atracos.inc)
//  ---------------------------------------------------------------------
//  Portado del filterscript de robos del catalogo antiguo, adaptado al
//  patron de este servidor: los locales los crea un admin de pie en el
//  sitio con /crearatraco (igual que /creardropeo), no hay coordenadas
//  fijas en el codigo. El resultado avisa a la Policia Nacional con el
//  mismo sistema que cultivo, NFS y robo de coches (37_policia.inc) --
//  no se toca jBuscado directamente, es el agente quien te para y te
//  imputa el codigo si te pilla.
// ---------------------------------------------------------------------
#define MAX_ATRACOS              50
#define ATRACO_RADIO             4.0      // metros a los que hay que quedarse quieto
#define ATRACO_DURACION_SEGUNDOS 25       // tiempo que tarda el atraco completo
#define ATRACO_COOLDOWN_MINUTOS  8        // minutos hasta que ese mismo local se pueda volver a atracar

// ---------------------------------------------------------------------
//  COMIDA RAPIDA  (42_comida.inc)
//  ---------------------------------------------------------------------
//  Las tres cadenas de comida rapida del San Andreas original -- Burger
//  Shot, Cluckin' Bell y Well Stacked Pizza Co. -- como negocios
//  comprables, ranuras 3/4/5 de 32_negocios.inc (las tres primeras de
//  las 7 que estaban libres).
//
//  MODELO DE FRANQUICIA, a proposito: cada cadena es UN solo negocio,
//  no uno por sucursal fisica. El juego reutiliza el mismo interior en
//  las ~30 sucursales de las tres cadenas repartidas por el mapa (igual
//  que ya haceis con la gasolinera en 27_tienda.inc); comprar la
//  franquicia da la caja fuerte de TODAS sus sucursales a la vez, y
//  /comer funciona en cualquiera de ellas tenga dueno o no. Menos
//  mantenimiento (3 interiores en vez de 30 filas de negocio con dueno
//  cada una) y mejor rol ("soy dueno de Burger Shot en Los Santos").
//
//  Interior y coordenadas de mostrador confirmados contra
//  open.mp/docs/scripting/resources/interiorids (categoria
//  "Restaurants, bars"): Burger Shot interior 10, Cluckin' Bell interior
//  9, Well Stacked Pizza interior 5. Ninguno de los tres estaba en uso
//  en este gamemode.
//
//  Las puertas EXTERIORES (las ~30 sucursales reales del mapa) todavia
//  no estan puestas -- Vlad las marca con /marcarcoord. El sistema entero
//  (comprar, caja fuerte, menu, animacion de comer, ingreso diario)
//  funciona ya desde hoy; anadir una sucursal manana es solo una fila
//  nueva en g_SucursalesComida, cero codigo nuevo.
// ---------------------------------------------------------------------
#define COMIDA_INTERIOR_BURGERSHOT   10
#define COMIDA_INTERIOR_CLUCKINBELL  9
#define COMIDA_INTERIOR_PIZZA        5

#define COMIDA_INT_BURGERSHOT_X      366.0248
#define COMIDA_INT_BURGERSHOT_Y      -73.3478
#define COMIDA_INT_BURGERSHOT_Z      1001.5078
#define COMIDA_INT_BURGERSHOT_A      90.0

#define COMIDA_INT_CLUCKINBELL_X     366.0002
#define COMIDA_INT_CLUCKINBELL_Y     -9.4338
#define COMIDA_INT_CLUCKINBELL_Z     1001.8516
#define COMIDA_INT_CLUCKINBELL_A     90.0

#define COMIDA_INT_PIZZA_X           372.5565
#define COMIDA_INT_PIZZA_Y           -131.3607
#define COMIDA_INT_PIZZA_Z           1001.4922
#define COMIDA_INT_PIZZA_A           90.0

// Icono "i" (1239) de /comer DENTRO del interior, uno por sucursal --
// pedido expreso de Vlad 13/09/2026, coordenadas marcadas en /panel >
// Coordenadas. Distinto del punto de llegada (COMIDA_INT_*, arriba): es
// solo donde flota el letrero/pickup, no donde aparece el jugador al
// entrar. Nombrado "ICONOCARTA" y NO "COMIDA_ICONO_*" a proposito: ese
// nombre ya lo usa el punto de COMPRA DEL NEGOCIO (32_negocios.inc, mas
// abajo en este mismo archivo) -- casi se pisan (mismo prefijo, "X/Y/Z"
// sin "A"), lo que habria hecho que estas coordenadas nuevas quedaran
// silenciosamente sobrescritas por esas otras (ultima definicion textual
// gana en una redefinicion de #define).
#define COMIDA_ICONOCARTA_BURGERSHOT_X    376.707
#define COMIDA_ICONOCARTA_BURGERSHOT_Y    -67.5461
#define COMIDA_ICONOCARTA_BURGERSHOT_Z    1001.51

#define COMIDA_ICONOCARTA_CLUCKINBELL_X   369.457
#define COMIDA_ICONOCARTA_CLUCKINBELL_Y   -6.37848
#define COMIDA_ICONOCARTA_CLUCKINBELL_Z   1001.85

#define COMIDA_ICONOCARTA_PIZZA_X         374.972
#define COMIDA_ICONOCARTA_PIZZA_Y         -119.401
#define COMIDA_ICONOCARTA_PIZZA_Z         1001.49

// Cluckin' Bell 2 (Angel Pine) -- marcada aparte porque es una sucursal
// distinta (mundo propio), aunque cae a centimetros de la de arriba por
// ser el mismo interior reutilizado.
#define COMIDA_ICONOCARTA_CB2_X           369.502
#define COMIDA_ICONOCARTA_CB2_Y           -6.4153
#define COMIDA_ICONOCARTA_CB2_Z           1001.85

// Cadenas, para indexar g_ComidaMenu y g_SucursalesComida.
#define COMIDA_CADENA_BURGERSHOT     0
#define COMIDA_CADENA_CLUCKINBELL    1
#define COMIDA_CADENA_PIZZA          2
#define COMIDA_TOTAL_CADENAS         3
// 3 platos propios de cada cadena + Te Helado y Gaseosa (04/09/2026,
// las mismas 2 bebidas en las 3 cadenas y tambien en la tienda).
#define COMIDA_PRODUCTOS_POR_CADENA  5

// Ingreso pasivo, una vez al dia real (86400 seg de uptime del server,
// ver TicSegundo), ADEMAS del 100% de cada venta de /comer que ya entra
// por Negocio_Ingresar -- las dos cosas se suman, no se sustituyen.
#define COMIDA_INGRESO_DIARIO        5000
#define COMIDA_SEGUNDOS_DIA          86400

// Puertas exteriores (sucursales reales). Vacio a proposito -- Vlad las
// rellena con /marcarcoord. Sube este numero si algun dia haceis mas de
// 40 sucursales activas entre las tres cadenas (hay unas 30 en total en
// el mapa original).
#define MAX_SUCURSALES_COMIDA        40
#define COMIDA_RADIO_PUERTA          2.5

// Primera sucursal real de cada cadena (Los Santos), marcada por Vlad con
// /marcarcoord el 03/09/2026 (tabla `coordenadas`, claves "negociosdos",
// "negociotres" y "NEGOCIOS"). Es la puerta exterior: donde se entra con
// Comida_IntentarEntrar y donde se vuelve a salir a la calle.
#define COMIDA_LS_BURGERSHOT_X       1199.4
#define COMIDA_LS_BURGERSHOT_Y       -918.685
#define COMIDA_LS_BURGERSHOT_Z       43.1182
#define COMIDA_LS_BURGERSHOT_A       12.9566

#define COMIDA_LS_CLUCKINBELL_X      927.257
#define COMIDA_LS_CLUCKINBELL_Y      -1352.93
#define COMIDA_LS_CLUCKINBELL_Z      13.3767
#define COMIDA_LS_CLUCKINBELL_A      268.293

// Segunda sucursal de Cluckin' Bell, "Clucking Bell 2" -- 07/09/2026,
// tabla `coordenadas` id 66. Misma cadena, mismo dueno, otra puerta
// (modelo de franquicia, ver cabecera de 42_comida.inc).
#define COMIDA_CB2_X                -2154.41
#define COMIDA_CB2_Y                -2460.96
#define COMIDA_CB2_Z                  30.8516
#define COMIDA_CB2_A                  51.244

#define COMIDA_LS_PIZZA_X            2104.51
#define COMIDA_LS_PIZZA_Y            -1806.36
#define COMIDA_LS_PIZZA_Z            13.5547
#define COMIDA_LS_PIZZA_A            269.655

// SUPERSEDIDO el 03/09/2026: Vlad revirtio a un solo icono en la calle
// para /comprarnegocio y /cajafuerte a la vez (igual que Gasolinera/DGT/
// Bonos), con coordenadas propias marcadas por /marcarcoord (tabla
// `coordenadas`, claves "posss"/"pozicion"/"pozziocion", 03/09/2026
// 01:20-01:23). Estas constantes COMIDA_COMPRA_* ya NO se usan en
// g_NegociosDef -- se conservan sin borrar por si hace falta reconstruir
// la secuencia. Las vigentes son COMIDA_ICONO_*, justo debajo.
#define COMIDA_COMPRA_BURGERSHOT_X   1201.4
#define COMIDA_COMPRA_BURGERSHOT_Y   -916.685
#define COMIDA_COMPRA_BURGERSHOT_Z   43.1182
#define COMIDA_COMPRA_BURGERSHOT_A   12.9566

#define COMIDA_COMPRA_CLUCKINBELL_X  929.257
#define COMIDA_COMPRA_CLUCKINBELL_Y  -1350.93
#define COMIDA_COMPRA_CLUCKINBELL_Z  13.3767
#define COMIDA_COMPRA_CLUCKINBELL_A  268.293

#define COMIDA_COMPRA_PIZZA_X        2106.51
#define COMIDA_COMPRA_PIZZA_Y        -1804.36
#define COMIDA_COMPRA_PIZZA_Z        13.5547
#define COMIDA_COMPRA_PIZZA_A        269.655

// Icono UNICO vigente de cada franquicia (/comprarnegocio + /cajafuerte
// a la vez), en la calle. Coordenadas marcadas por Vlad con /marcarcoord
// el 03/09/2026.
#define COMIDA_ICONO_BURGERSHOT_X    1205.35
#define COMIDA_ICONO_BURGERSHOT_Y    -921.14
#define COMIDA_ICONO_BURGERSHOT_Z    43.0665
#define COMIDA_ICONO_BURGERSHOT_A    6.68982

#define COMIDA_ICONO_CLUCKINBELL_X   926.343
#define COMIDA_ICONO_CLUCKINBELL_Y   -1348.0
#define COMIDA_ICONO_CLUCKINBELL_Z   13.4031
#define COMIDA_ICONO_CLUCKINBELL_A   268.293

#define COMIDA_ICONO_PIZZA_X         2099.66
#define COMIDA_ICONO_PIZZA_Y         -1802.98
#define COMIDA_ICONO_PIZZA_Z         13.5547
#define COMIDA_ICONO_PIZZA_A         268.692

// ---------------------------------------------------------------------
//  LOS COCHES DE FEDERICO  (43_federico.inc)
//  ---------------------------------------------------------------------
//  Federico busca 3 modelos del pool de vehiculos libres (VEH_DROPEO,
//  sin dueno) cada FEDERICO_DURACION_RONDA segundos. Ubicacion: puesta
//  por Vlad en /panel admin con /marcarcoord (clave "Posicion", tabla
//  `coordenadas`, id=47, nombre "/Entregarcoche FEDERICO") el
//  04/09/2026 -- sustituye a la de id=46 (03/09/2026, sin validar) de
//  la v1/v2.
// ---------------------------------------------------------------------
#define FEDERICO_DURACION_RONDA      86400   // 24 horas reales -- subido de 2h el 07/09/2026, peticion expresa de Vlad
#define FEDERICO_COOLDOWN           86400   // 24 horas reales
#define FEDERICO_RADIO_ENTREGA          6.0
#define FEDERICO_TOTAL_SLOTS            3
#define FEDERICO_AVISO_COOLDOWN       120    // segundos entre avisos repetidos al mismo jugador (antispam)

#define FEDERICO_X                  2461.43
#define FEDERICO_Y                 -2628.34
#define FEDERICO_Z                    13.6628

// Segundo punto de entrega, Angel Pine (07/09/2026, tabla `coordenadas`
// id 82, "/entregarcoche 2") -- mismo Federico, misma logica de entrega
// (Federico_SlotDeModelo etc. no dependen del punto), solo un segundo
// sitio fisico donde dejar el coche. Icono de mapa propio, slot 10
// (0-99 valido -- ver la nota junto a PESCAPUNTO_ICON_SLOT_BASE sobre el
// bug de slots fuera de rango).
#define FEDERICO_ANGELPINE_X       -2094.41
#define FEDERICO_ANGELPINE_Y       -2279.83
#define FEDERICO_ANGELPINE_Z          30.625
#define FEDERICO_ANGELPINE_A         143.555
#define FEDERICO_ANGELPINE_ICON_SLOT   10
#define FEDERICO_A                   356.653

// Maletero de vehiculo (45_maletero.inc), Fase 2 -- radio para /maletero.
// El de /forzarmaletero (Fase 3) y sus propias constantes de
// probabilidad/aviso/ventana llegan con esa fase.
#define MALETERO_RADIO 6.0

// Coche Secreto (05/09/2026) -- cuarto "coche buscado", aparte de los 3
// de siempre: un modelo del que solo quede 1 unidad libre en todo el
// pool (mismo criterio que la rareza SUPER RARE), con color reconocible.
// Rota cada 24h, igual que el cooldown de un vehiculo retirado. Se
// entrega igual que los otros tres, mismo /entregarcoche.
#define FEDERICO_SECRETO_DURACION   86400   // 24 horas reales
#define FEDERICO_SECRETO_RECOMPENSA 300000

// "La Moto del Dia" (07/09/2026) -- 5o coche buscado, aparte del Secreto,
// los 3 normales, el Historico y la mini lista chatarra. Se revela
// directamente (nombre visible, sin acertijo), rota cada 24h igual que
// el Secreto. Lista de modelos validos (motos de verdad, sin bicicletas
// ni quads) en g_FedericoMotos, 43_federico.inc.
#define FEDERICO_MOTO_DURACION      86400   // 24 horas reales
#define FEDERICO_MOTO_RECOMPENSA    50000

// Robo Express (08/09/2026) -- logica completa en 57_roboexpress.inc.
// Esta unica constante vive aqui (y no alli) porque 43_federico.inc la
// usa y se incluye ANTES que 57_roboexpress.inc en malagacity.pwn.
#define ROBOEXPRESS_RECOMPENSA      1000000

// Mini lista de "chatarra" (05/09/2026) -- coches clase CHATARRA aparte
// de los 3 normales y el Secreto, que NO pagan dinero: pagan materiales,
// marihuana o un arma (ciclo de 3 en 3 por posicion de ranura, ver el
// switch en CMD:entregarcoche). Solo se veian en la web (/federico) hasta
// el 08/09/2026, ahora tambien en /verlista ingame. Duracion IGUALADA al
// resto de Federico el 08/09/2026 (antes 6h, unica que no estaba a 24h) y
// ampliada de 3 a 5 coches el mismo dia, peticion expresa de Vlad.
// Pagos subidos un 40% el 07/09/2026 (peticion expresa de Vlad):
// materiales 2.000->2.800, marihuana 200->280, municion 200->280 (el
// arma en si sigue siendo una sola Uzi, no admite "40% mas arma").
// Pagos subidos un 500% el 13/09/2026 (peticion expresa de Vlad), MENOS
// el del arma: materiales 2.800->16.800, marihuana 280->1.680; el premio
// de arma cambia de Uzi a Spaz 12 con una cantidad fija de 200 balas (no
// escalada por el 500%, ver g_Items en 10_inventario.inc y el switch de
// 43_federico.inc).
#define FEDERICO_CHATARRA_TOTAL_SLOTS   5
#define FEDERICO_CHATARRA_DURACION      86400   // 24 horas reales -- antes 21600 (6h)
#define FEDERICO_CHATARRA_MATERIALES    16800
#define FEDERICO_CHATARRA_MARIHUANA     1680
#define FEDERICO_CHATARRA_ARMA          WEAPON_SHOTGSPA
#define FEDERICO_CHATARRA_ARMA_MUNICION 200

// "Coche de Personajes Historicos" (05/09/2026, renombrado el mismo dia
// desde "El Coche de la Droga" -- ahora son 4 posibles, uno a la vez).
// Un Fundador elige con /crearhistorico uno de los 4 personajes de un
// menu; se crea su vehiculo con su maletero cargado segun el personaje
// (ver g_Historicos, 43_federico.inc). Coche libre normal (se puede
// forzar el maletero como a cualquier otro) -- lo unico especial es que
// /entregarcoche lo reconoce por su vehiculo_id exacto (no por modelo,
// puede haber otros del mismo modelo sueltos por el mapa que NO sean
// este) y, al entregarlo, desaparece PARA SIEMPRE (no vuelve a los 24h
// como el resto de Federico) hasta que un Fundador cree otro.
// Recompensa subida 100.000->150.000 el 07/09/2026 -- el maletero propio
// de cada personaje (g_Historicos, dinero/materiales/marihuana/
// portatiles) es APARTE y se queda igual, se suma a esto.
#define FEDERICO_HISTORICO_RECOMPENSA  150000
#define FEDERICO_HISTORICO_TOTAL       4

// ---------------------------------------------------------------------
//  COCHES DE JUGADORES  (59_federicojugadores.inc, 08/09/2026)
//  ---------------------------------------------------------------------
//  Unica categoria de Federico que acepta vehiculos CON dueno real.
//  Comparte la rotacion de 24h del resto de Federico (mismo tick,
//  Federico_RondaGeneralSincronizar), pero NO su retirada de 24h: cada
//  vehiculo entregado tiene su propio plazo INDEPENDIENTE de 48h reales,
//  con persistencia propia en la tabla `federico_jugadores` -- nunca
//  reutiliza `vehiculos.federico_until`, que es la contabilidad generica
//  del resto de Federico. Ver informe tecnico PASO A / cabecera de
//  59_federicojugadores.inc para el porque de mantenerlo aislado.
// ---------------------------------------------------------------------
#define FEDJUGADORES_TOTAL_SLOTS         3
#define FEDJUGADORES_RETORNO_SEGUNDOS    172800   // 48 horas reales, INDEPENDIENTE por vehiculo
#define FEDJUGADORES_MAX_PENDIENTES      12       // vehiculos escondidos a la vez esperando sus 48h (margen generoso)
// FEDJUGADORES_MAX_ADMIN_CACHE existio hasta el 08/09/2026: el encargo
// original excluia vehiculos de propietarios admin>0, revertido en
// directo por el jefe ("los admins jugamos tambien!") antes de terminar
// la implementacion -- ver FedJugadores_VehiculosElegibles.

// Recompensas fijas por rareza -- decision explicita del jefe, DISTINTAS
// de las de Federico_Rareza (mas arriba en este archivo): solo se
// reutiliza la CLASIFICACION de Federico_Rareza, nunca su importe.
#define FEDJUGADORES_RECOMPENSA_SUPERRARE   130000
#define FEDJUGADORES_RECOMPENSA_RARE        100000
#define FEDJUGADORES_RECOMPENSA_DELMONTON   75000
#define FEDJUGADORES_RECOMPENSA_CHATARRA    50000

// /forzarcerradura -- referencia estructural /forzarmaletero
// (45_maletero.inc), constantes PROPIAS a proposito (encargo explicito:
// no acoplar el balance de las dos mecanicas). 60% de exito es cifra
// pedida expresamente por el jefe, no una estimacion.
#define FORZARCERRADURA_RADIO        6.0
#define FORZARCERRADURA_FREEZE_MS    4000
#define FORZARCERRADURA_EXITO        60

// ---------------------------------------------------------------------
//  HORA REAL + CLIMA DINAMICO  (60_climahora.inc, 10/09/2026)
//  ---------------------------------------------------------------------
//  Sustituye SetWeather(2)/SetWorldTime(12) fijos por hora real de
//  Europe/Madrid (calculada dentro del propio Pawn, ver cabecera de
//  60_climahora.inc) y clima dinamico con maquina de estados. Capa
//  ambiental global, no toca ningun otro sistema.
// ---------------------------------------------------------------------
#define CLIMA_DESPEJADO             0
#define CLIMA_SOLEADO               1
#define CLIMA_PARCIAL_NUBES         2
#define CLIMA_NUBLADO               3
#define CLIMA_LLUVIA                4
#define CLIMA_TORMENTA              5
#define CLIMA_NIEBLA                6
#define CLIMA_TOTAL_ESTADOS         7

#define CLIMA_HORA_NIEBLA_INICIO    4    // ventana de madrugada/amanecer en la que puede aparecer niebla
#define CLIMA_HORA_NIEBLA_FIN       7
#define CLIMA_PROB_NIEBLA           15   // % de probabilidad de niebla dentro de esa ventana

// ---------------------------------------------------------------------
//  HAMBRE Y SED  (44_hambre_sed.inc)
//  ---------------------------------------------------------------------
//  100 -> 0 en el tiempo indicado de juego EFECTIVO (conectado, logueado,
//  ya en la calle -- jSpawneado -- y sin estar AFK). Persisten en
//  `cuentas.hambre` / `cuentas.sed`, igual que salud o armadura.
// ---------------------------------------------------------------------
#define HAMBRE_SEGUNDOS_VACIO          3600   // 60 min reales de 100 a 0
#define SED_SEGUNDOS_VACIO             4800   // 80 min reales de 100 a 0

#define HAMBRESED_UMBRAL_AVISO         25.0
#define HAMBRESED_UMBRAL_CRITICO       10.0
#define HAMBRESED_AVISO_COOLDOWN_ALTO   300   // segundos entre avisos con <=25%
#define HAMBRESED_AVISO_COOLDOWN_BAJO    90   // segundos entre avisos con <=10%

// Penalizacion a 0%: segunda vuelta de tuerca, encargo de Vlad el
// 06/09/2026 -- ahora SI mata. Sin suelo: una barra entera (100) se
// vacia en 90 segundos reales de estar a hambre o sed 0, asi que hay que
// comer o beber a tiempo. Al llegar a 0 de vida, el motor de SA-MP
// dispara OnPlayerDeath igual que un balazo -- cae "herido" como
// cualquier otra muerte, no hace falta tratamiento especial.
#define HAMBRESED_PENALIZACION_SEGUNDOS  1        // cada cuanto se aplica
#define HAMBRESED_PENALIZACION_VIDA      1.1111   // vida que quita cada vez (100 / 90s)

// ---------------------------------------------------------------------
//  GUARDIAS ARMADOS  (46_guardias.inc)
//  ---------------------------------------------------------------------
//  NPCs estaticos (API nativa NPC_* de open.mp, sin FCNPC) creados a mano
//  por un admin con /crearguardia -- encargo de Vlad del 05/09/2026,
//  pensados para defender un punto en misiones o golpes. No se mueven,
//  solo apuntan y disparan a cualquier jugador (nunca a otro NPC) que
//  entre en rango, sin distincion de bando ni admin.
// ---------------------------------------------------------------------
#define MAX_GUARDIAS             20
#define GUARDIA_RANGO            15.0   // metros a los que empieza a disparar
#define GUARDIA_GRACIA_MS        10000  // pasivo estos ms tras crearse o revivir
#define GUARDIA_RESPAWN_MS       300000 // 5 min muerto antes de reaparecer en su sitio
#define GUARDIA_DISTANCIA_SPAWN  1.5    // metros por delante del admin al crearlo
#define GUARDIA_DISPARO_DELAY_MS 800    // cadencia de disparo mientras apunta

// ---------------------------------------------------------------------
//  ICONO DE BIENVENIDA  (47_bienvenida.inc)
//  ---------------------------------------------------------------------
//  Coordenada marcada por Vlad en el panel el 05/09/2026 (tabla
//  `coordenadas`, id 49, nota "icono info Bienvenido a Gamerol").
//  Un diamante (modelo 1239, el mismo pickup que ya usa el vendedor de
//  armas) en vez de la "i" que usa todo el mundo. Da 15.000$ una unica
//  vez por cuenta (flag en `cuentas`); el texto grande se puede volver
//  a ver, pero con cooldown silencioso de 1h -- sin aviso al jugador,
//  simplemente no vuelve a salir hasta que pase la hora.
// ---------------------------------------------------------------------
#define BIENVENIDA_X              1219.1200
#define BIENVENIDA_Y             -1306.6200
#define BIENVENIDA_Z                13.4791
#define BIENVENIDA_ANGULO           88.9173
#define BIENVENIDA_RADIO             3.0
#define BIENVENIDA_ICONO_MODELO   19055       // caja de regalo
#define BIENVENIDA_RECOMPENSA     15000
#define BIENVENIDA_TEXTO_COOLDOWN 3600         // 1h, en segundos (gettime())
#define BIENVENIDA_TEXTO_DURACION_MS 6000      // cuanto se queda el texto en pantalla

// ---------------------------------------------------------------------
//  GUIA INGAME  (48_guia.inc)
//  ---------------------------------------------------------------------
//  Coordenada marcada por Vlad en el panel el 05/09/2026 (tabla
//  `coordenadas`, id 50, nota "GUIA INGAME"). Mismo mecanismo que el
//  icono de bienvenida (acercarse activa algo), pero aqui abre un
//  dialogo con paginas en vez de dar dinero -- disparado al ENTRAR en
//  el radio (no cada segundo mientras estas dentro), para no reabrir
//  la ventana en bucle si te quedas parado encima.
// ---------------------------------------------------------------------
#define GUIA_X                    1218.8500
#define GUIA_Y                   -1295.1300
#define GUIA_Z                      13.4705
#define GUIA_ANGULO                270.0980
#define GUIA_RADIO                   3.0
#define GUIA_ICONO_MODELO         1274        // bolsa de dinero, la misma que usa el banco

// ---------------------------------------------------------------------
//  GUIA DE VEHICULOS  (49_guiavehiculos.inc)
//  ---------------------------------------------------------------------
//  Mismo mecanismo y estilo que la guia general (48_guia.inc), en la
//  coordenada que Vlad marco el 05/09/2026 (tabla `coordenadas`, id 51,
//  nota "sistema de vehiculos info").
// ---------------------------------------------------------------------
#define GUIAVEH_X                 1218.7000
#define GUIAVEH_Y                -1317.6100
#define GUIAVEH_Z                   13.4794
#define GUIAVEH_ANGULO              87.1583
#define GUIAVEH_RADIO                3.0
#define GUIAVEH_ICONO_MODELO      19130

// ---------------------------------------------------------------------
//  CASA DE EMPEÑOS / PAWNSHOP  (50_pawnshop.inc, 06/09/2026)
//  ---------------------------------------------------------------------
//  Exterior: coordenada que Vlad ya tenia guardada en el panel (tabla
//  `coordenadas`, id 56, clave "pozzzzzision", nombre "casa venta").
//  Interior: la coordenada "Zero's RC Shop" que dio Vlad de memoria no
//  cargaba nada visible (ni interior ni texto) -- descartada el
//  06/09/2026. En su lugar se reutiliza el interior 17, el MISMO que ya
//  usa la tienda de la gasolinera (27_tienda.inc, TIENDA_INT_*) y que
//  llevamos viendo funcionar en produccion: mismo interior de GTA, pero
//  MUNDO_EMPENOS es un mundo virtual distinto, asi que nadie se cruza
//  entre la tienda y la Casa de Empeños aunque compartan el mismo mesh.
//  Si algun dia se quiere un local distinto, /setempenos
//  (16_administracion.inc, mismo patron que /setbanco) lo cambia en
//  caliente sin tocar codigo.
// ---------------------------------------------------------------------
#define EMPENOS_EXT_X              2422.36
#define EMPENOS_EXT_Y             -1954.96
#define EMPENOS_EXT_Z                13.5469
#define EMPENOS_EXT_ANGULO          272.901
#define EMPENOS_EXT_RADIO             2.5
#define EMPENOS_PICKUP_MODELO      1212     // fajo de billetes

#define EMPENOS_INT_INTERIOR         17     // mismo interior que la tienda (27_tienda.inc), ya verificado en produccion
#define EMPENOS_INT_X               -25.7220
#define EMPENOS_INT_Y              -183.8216   // separado de TIENDA_INT_Y para no aparecer pegado a su mostrador
#define EMPENOS_INT_Z              1003.5469
#define EMPENOS_INT_ANGULO            0.0

// Precio de la marihuana: NO es un numero fijo. Se calcula en
// Empenos_PrecioMarihuana() (50_pawnshop.inc) a partir del precio REAL
// de la semilla en la tienda (27_tienda.inc) y el rendimiento de cultivo
// (MARIA_RENDIMIENTO, 28_marihuana.inc) -- si cambia el precio de la
// semilla, este numero se mueve solo. Formula documentada alli mismo.
#define EMPENOS_MARIHUANA_MULTIPLICADOR  2   // 100% de beneficio sobre el coste real
#define EMPENOS_MATERIALES_PRECIO        1   // $ por unidad, fijo (encargo expreso)

#define TIEMPO_CAMARA_LOGIN     13000   // Duracion de cada plano de la intro

// ---------------------------------------------------------------------
//  GAMEROL AVENTURAS: DLC PESCA  (51_pesca.inc, 06/09/2026)
//  ---------------------------------------------------------------------
//  Los 6 puntos de pesca y los 2 puestos de venta son CONFIGURABLES en
//  caliente con /setpesca [1-6] y /setventapescado [1-2]
//  (16_administracion.inc, mismo patron que /setbanco y /setempeños) --
//  estas constantes son solo el valor de PARTIDA que carga
//  Pesca_CargarConfiguracion() la primera vez, via Config_Leer.
//
//  Puntos 1-3: coordenadas que Vlad marco el 06/09/2026 con /marcarcoord
//  (tabla `coordenadas`, ids 57-59, "/pescar 1" / "/pescar 2" / "/pesca
//  3"). Puntos 4-6: coordenadas de Angel Pine desde el 07/09/2026 (ver
//  el bloque de mas abajo) -- los 6 puntos estan activos. Un punto solo
//  vuelve a quedar inactivo (sin etiqueta, sin /pescar) si su X e Y
//  llegan a 0,0 -- entonces hace falta que un Gestion+ lo marque de
//  nuevo de pie en el sitio con /setpesca.
// ---------------------------------------------------------------------
#define PESCA1_X                   373.636
#define PESCA1_Y                 -2088.44
#define PESCA1_Z                     7.83594
#define PESCA1_A                   177.396

#define PESCA2_X                   361.761
#define PESCA2_Y                 -2088.8
#define PESCA2_Z                     7.83594
#define PESCA2_A                   182.973

#define PESCA3_X                   397.798
#define PESCA3_Y                 -2088.6
#define PESCA3_Z                     7.83594
#define PESCA3_A                   185.874

// Angel Pine (07/09/2026), tabla `coordenadas` ids 77-79. Activan solos
// en Pesca_CargarConfiguracion (g_PescaPuntoActivo = X!=0 || Y!=0), sin
// tocar nada mas -- eran los 3 unicos huecos [1-6] que seguian a 0.0.
#define PESCA4_X                 -2694.35
#define PESCA4_Y                 -2215.23
#define PESCA4_Z                     1.06981
#define PESCA4_A                    49.2429

#define PESCA5_X                 -2702.12
#define PESCA5_Y                 -2222.37
#define PESCA5_Z                     1.38279
#define PESCA5_A                    56.763

#define PESCA6_X                 -2679.32
#define PESCA6_Y                 -2213.3
#define PESCA6_Z                     1.00716
#define PESCA6_A                    30.1294

#define PESCA_RADIO                  4.0
#define PESCA_DURACION_SEGUNDOS     10
#define PESCA_PROBABILIDAD_EXITO   60   // % de intentos que sacan un pez

// Puestos de venta -- coordenadas dadas directamente por Vlad el
// 06/09/2026 (Tierra Robada Bait Shop y Angel Pine Bait Shop, exteriores
// puros, sin interior). Igual que los puntos de pesca, son solo el valor
// de partida: /setventapescado [1-2] los corrige en caliente si hace falta.
#define VENTAPESCA1_X            -1125.12
#define VENTAPESCA1_Y             2696.53
#define VENTAPESCA1_Z               50.26
// Angel Pine: corregida DOS VECES. Original del 06/09 (por chat, sin
// marcar): -2144.15, -2437.28, 30.62. Primera correccion 07/09 (id 62,
// "/Venderpescado ANGELPINE") tambien resulto equivocada. Corregida de
// nuevo el 07/09/2026 con la coordenada definitiva (tabla `coordenadas`,
// id 71, "posicion /venderpescado angelpine reemplazar").
#define VENTAPESCA2_X            -2058.34
#define VENTAPESCA2_Y            -2465.39
#define VENTAPESCA2_Z               31.1797
// Tercer puesto, nuevo el 07/09/2026 (tabla `coordenadas`, id 61, "venta
// pescado 3") -- junto a los puntos de pesca, no en Angel Pine/Tierra Robada.
#define VENTAPESCA3_X              376.094
#define VENTAPESCA3_Y            -2068.27
#define VENTAPESCA3_Z                7.83594
#define VENTAPESCA_RADIO             3.0
#define VENTAPESCA_PICKUP_MODELO  1212   // fajo de billetes, mismo modelo que la Casa de Empeños
// Iconos de minimapa de los 3 puestos -- CAMBIADO el 07/09/2026 a
// peticion de Vlad: 36 "School" pasa a ser el icono estandar de "sitio
// para ganar dinero" en todo el servidor (antes 17 "Diner"). El mismo
// sprite se usa tambien en la Casa de Empeños (EMPENOS_ICON_SLOT,
// 50_pawnshop.inc) y en /entregarcoche (FEDERICO_ICON_SLOT, mas abajo).
#define VENTAPESCA_ICON_SPRITE       36
#define VENTAPESCA1_ICON_SLOT        93
#define VENTAPESCA2_ICON_SLOT        94
#define VENTAPESCA3_ICON_SLOT        95

// Icono de /entregarcoche (43_federico.inc) -- no tenia ninguno hasta
// ahora. Mismo sprite 36 "School" que Casa de Empeños/venderpescado,
// pedido expreso de Vlad ("icono estandar para ganar dinero").
#define FEDERICO_ICON_SLOT           99

// Iconos de los puntos de pesca (51_pesca.inc) -- no tenian ninguno
// hasta ahora, solo etiqueta 3D. Sprite 9 "Boat Yard", pedido expreso.
//
// BUG REAL corregido 07/09/2026 (madrugada): estos y los 4 siguientes
// (MEDICOS/ANGELPINE_BANCO/MOTEL_COMPRAR/TIENDA_ANGELPINE) estaban en el
// rango 100-109. SetPlayerMapIcon SOLO admite iconid 0-99 (100 iconos por
// jugador, verificado contra open.mp/docs/scripting/functions/
// SetPlayerMapIcon) -- por encima de 99 el juego simplemente NO lo
// muestra, sin error ni warning de compilacion. Los 10 iconos de esta
// tanda (6 puntos de pesca, Medicos, banco/motel/tienda de Angel Pine)
// llevaban toda la noche sin aparecer por esto. Reubicados al 0-9, que
// estaba libre de verdad (0-38 no lo usa nadie; 40-79 esta RESERVADO por
// COMIDA_ICON_SLOT_BASE aunque hoy solo usa 4; 80-87 reservado por
// TUNING_ICON_SLOT_BASE aunque hoy usa 0 -- ver sus propios modulos).
#define PESCAPUNTO_ICON_SPRITE        9
#define PESCAPUNTO_ICON_SLOT_BASE     0   // + indice (0-5), 6 huecos: 0-5

// Icono de /aceptarjob de Medicos Los Santos (11_trabajos.inc) --
// EXCEPCION deliberada a la regla "los trabajos no llevan blip, se usa
// /gps" (Trabajo_CrearMarcadores): pedido expreso de Vlad, coincide con
// el hospital real. Sprite 22 "Hospital".
#define MEDICOS_ICON_SPRITE          22
#define MEDICOS_ICON_SLOT             6

// Slots de negocio que 13_vehiculos.inc necesita ANTES de que
// 32_negocios.inc los defina (un #define exige orden textual, a
// diferencia de una funcion) -- movidos aqui el 07/09/2026, el resto de
// NEGOCIO_SLOT_* se quedan en 32_negocios.inc, donde nunca hizo falta
// moverlos porque nadie los usa antes.
#define NEGOCIO_SLOT_TIENDA      0        // "Gasolinera Los Santos"
#define NEGOCIO_SLOT_DGT         1        // "DGT"
// Igual de motivo, 07/09/2026: 27_tienda.inc (modulo 27, antes que 32)
// necesita esta ranura para la segunda sucursal (Tienda Angel Pine).
#define NEGOCIO_SLOT_TIENDA_ANGELPINE 7   // "Tienda Angel Pine"

// ---------------------------------------------------------------------
//  TRES SURTIDORES NUEVOS -- 07/09/2026, tabla `coordenadas` ids 63-65
//  ("petrolpump nuevo1/2/3"). Se anaden al final de g_Gasolineras
//  (13_vehiculos.inc) para que funcione /repostar ahi tambien -- a
//  diferencia de las 12 gasolineras genericas (que siguen siendo un
//  sumidero puro), lo cobrado en ESTAS TRES va integro a la caja fuerte
//  de la Gasolinera Los Santos (NEGOCIO_SLOT_TIENDA), peticion expresa
//  de Vlad. Ver Veh_IndiceGasolinera/Veh_RepostarCompletar.
// ---------------------------------------------------------------------
#define PETROLPUMP1_X            -2248.75
#define PETROLPUMP1_Y            -2558.23
#define PETROLPUMP1_Z               31.9219
#define PETROLPUMP2_X            -2243.95
#define PETROLPUMP2_Y            -2560.94
#define PETROLPUMP2_Z               31.9219
#define PETROLPUMP3_X            -2238.82
#define PETROLPUMP3_Y            -2563.79
#define PETROLPUMP3_Z               31.9219
// Sprite 19 "Enemy Attack", eleccion expresa de Vlad el 07/09/2026 para
// las DOS gasolineras (esta y la Gasolinera Los Santos original de
// 27_tienda.inc, TIENDA_ICON_SLOT) -- antes las dos usaban 17 "Diner".
// OJO: el 56 que se pidio primero queda descartado a proposito: la wiki
// oficial (sampwiki.blast.hk/wiki/MapIcons) lo marca como uno de los 4
// IDs (junto a 1, 2 y 4) que CRASHEAN el cliente si el jugador tiene la
// leyenda del mapa activada al abrirlo -- mismo tipo de riesgo que el
// CreateExplosion de la barbacoa, no se aplica sin verificar. Un solo
// icono para los tres pumps, anclado en "petrolpump nuevo2" (peticion
// expresa).
#define PETROLPUMP_ICON_SPRITE       19
#define PETROLPUMP_ICON_SLOT         96

// Gasolinera de la carretera LS-SF -- 11/09/2026, tabla `coordenadas`
// ("petrolpump lssf"). A diferencia del trio de arriba (PETROLPUMP1/2/3,
// en g_Gasolineras, funcionales para /repostar), esta SOLO es un destino
// de /gps (seccion Vehiculos, 30_gps.inc) -- Vlad no pidio hacerla
// surtidor de verdad, solo que aparezca en el GPS. Vlad marco dos puntos
// muy cercanos entre si al principio ("lssf"/"lssf2") y pidio quedarse
// solo con uno.
#define PETROLPUMP_CARRETERA_LSSF_X  -93.6213
#define PETROLPUMP_CARRETERA_LSSF_Y  -1175.09
#define PETROLPUMP_CARRETERA_LSSF_Z     1.88909

// ---------------------------------------------------------------------
//  PERRITO CALIENTE -- 07/09/2026, puesto nuevo, tabla `coordenadas` id
//  67. Sigue el patron de /comer (42_comida.inc): menu, cobro, vida,
//  hambre/sed, animacion -- pero SIN negocio ni dueno (puesto publico,
//  el cobro es un sumidero simple, decision de Vlad, 07/09/2026).
//  Precios y vida propuestos por Claudia siguiendo la misma proporcion
//  que el resto del menu de comida rapida (~5$ por punto de hambre en
//  los "menu con bebida", vida ~0.6x del hambre) -- avisar si se
//  quieren otros numeros, es un solo array a tocar.
// ---------------------------------------------------------------------
#define PERRITO_X                -2197.9
#define PERRITO_Y                -2387.63
#define PERRITO_Z                  30.625
#define PERRITO_A                  73.1397
#define PERRITO_RADIO               3.0
#define PERRITO_ICON_SPRITE        50   // "Restaurant" -- cambiado el 07/09/2026 a peticion de Vlad (antes 10 "Burger Shot")
#define PERRITO_ICON_SLOT          97

// ---------------------------------------------------------------------
//  PAY 'N' SPRAY CASERO -- 07/09/2026, tabla `coordenadas` id 69.
//  /arreglarcoche repara el vehiculo al completo por un precio fijo.
//  El cobro va integro a la caja fuerte de la DGT (decision de Vlad,
//  07/09/2026) -- igual que ya hace /comprarcoche.
// ---------------------------------------------------------------------
#define ARREGLARCOCHE_RADIO         3.0
#define ARREGLARCOCHE_PRECIO     1000
#define ARREGLARCOCHE_ICON_SPRITE  63   // "Pay 'n' Spray", verificado contra sampwiki.blast.hk/wiki/MapIcons

// Cuatro puntos desde el 07/09/2026 (noche): Angel Pine (el original) +
// Grove Street, NFS y Policia Nacional -- coordenadas marcadas por Vlad
// en /panel (tabla `coordenadas`, ids 83-85).
#define ARREGLARCOCHE_TOTAL_PUNTOS  4
new Float:g_ArreglarcocheX[ARREGLARCOCHE_TOTAL_PUNTOS] = {-2202.01,  2375.41,   555.735, 1560.13};
new Float:g_ArreglarcocheY[ARREGLARCOCHE_TOTAL_PUNTOS] = {-2431.58, -1643.38, -1275.31, -1695.87};
new Float:g_ArreglarcocheZ[ARREGLARCOCHE_TOTAL_PUNTOS] = {  30.6255,   13.5212,   17.2422,    5.89062};
new g_ArreglarcocheNombre[ARREGLARCOCHE_TOTAL_PUNTOS][24] = {"Angel Pine", "Grove Street", "NFS", "Policia Nacional"};

// g_ArreglarcocheIconSlot (11-13, mas el 98 de Angel Pine) se quito el
// 11/09/2026 junto con el resto de iconos de minimapa del gamemode --
// peticion expresa de Vlad, la gente usa /gps.

// ---------------------------------------------------------------------
//  /BANCO EN ANGEL PINE -- 07/09/2026, tabla `coordenadas` id 73. Punto
//  exterior SIN interior propio: usar /banco ahi abre el mismo menu de
//  siempre (ingresar/retirar/transferir), no es una sucursal nueva con
//  puerta. Ver Banco_EstaDentro en 15_banco.inc, que ahora acepta ESTE
//  punto ademas del mostrador de dentro del Ayuntamiento de Los Santos.
// ---------------------------------------------------------------------
#define ANGELPINE_BANCO_X        -2127.08
#define ANGELPINE_BANCO_Y        -2347.9
#define ANGELPINE_BANCO_Z          30.625
#define ANGELPINE_BANCO_A         141.259
#define ANGELPINE_BANCO_RADIO       4.0
// Mismo icono que ya usa el banco de siempre (BANCO_ICON_SLOT, 15_banco.inc):
// 52 "Robbery", el que usa la comunidad para bancos, en verde
// (COLOR_VERDE). Slot corregido 07/09/2026 -- ver la nota junto a
// PESCAPUNTO_ICON_SLOT_BASE de mas arriba (estaba en 107, fuera del
// rango valido 0-99, por eso nunca aparecio).
#define ANGELPINE_BANCO_ICON_SLOT     7

// ---------------------------------------------------------------------
//  MOTEL ANGEL PINE -- 07/09/2026. Negocio nuevo (32_negocios.inc,
//  NEGOCIO_SLOT_MOTEL) que sigue la misma filosofia que Gasolinera/DGT/
//  Bonos (un solo punto para /comprarnegocio y /cajafuerte, sin compra
//  exterior aparte), mas dos habitaciones de alquiler por tiempo real
//  (54_motel.inc, sistema propio, no forma parte del negocio en si).
//  Coordenadas de /panel: id 76 (comprar), 74 y 75 (habitacion 1 y 2).
// ---------------------------------------------------------------------
#define MOTEL_COMPRAR_X          -2190.02
#define MOTEL_COMPRAR_Y          -2267.13
#define MOTEL_COMPRAR_Z            30.625
#define MOTEL_COMPRAR_A           230.874
#define MOTEL_PRECIO             150000
#define MOTEL_INGRESO_DIARIO       3500   // cada 24h reales de uptime, mismo tick que Comida_IngresoDiario (TicSegundo)
// Icono de mapa del /comprarnegocio, peticion expresa de Vlad 07/09/2026:
// 32 "Property" en rojo (COLOR_ROJO_FUERTE) -- verificado contra
// sampwiki.blast.hk/wiki/MapIcons.
#define MOTEL_COMPRAR_ICON_SPRITE   32
// Slot corregido 07/09/2026 -- estaba en 108, fuera del rango valido
// 0-99 (ver nota junto a PESCAPUNTO_ICON_SLOT_BASE).
#define MOTEL_COMPRAR_ICON_SLOT       8

// Las dos habitaciones. Interior "Random House" (g_InterioresCasa,
// 24_casas.inc, interior nativo 2) reutilizado para las dos -- cada una
// con su PROPIO mundo virtual (MOTEL_MUNDO_BASE + indice, ver
// 54_motel.inc) para no repetir el fallo de Cluckin' Bell 2 del
// 07/09/2026 (un solo /salir compartido volviendo siempre a la primera
// puerta). Asi cada /salir vuelve exactamente al /entrar de su propia
// habitacion.
#define MOTEL_INTERIOR_ID          2
#define MOTEL_INT_X              2236.6997
#define MOTEL_INT_Y             -1078.9478
#define MOTEL_INT_Z              1049.0234
#define MOTEL_HAB1_X             -2192.21
#define MOTEL_HAB1_Y             -2255.42
#define MOTEL_HAB1_Z               30.6931
#define MOTEL_HAB1_A              320.151
#define MOTEL_HAB2_X             -2193.51
#define MOTEL_HAB2_Y             -2254.28
#define MOTEL_HAB2_Z               30.7018
#define MOTEL_HAB2_A              322.971
#define MOTEL_HAB_RADIO             3.0
#define MOTEL_ALQUILER_PRECIO    1000
#define MOTEL_ALQUILER_SEGUNDOS  86400   // 24h reales, mismo criterio que COMIDA_SEGUNDOS_DIA

// ---------------------------------------------------------------------
//  TIENDA ANGEL PINE -- 07/09/2026, segunda sucursal de la Gasolinera de
//  27_tienda.inc: MISMO interior nativo (TIENDA_INTERIOR) y MISMO
//  catalogo (g_TiendaCatalogo), pero con su propio mundo virtual
//  (MUNDO_TIENDA_ANGELPINE) para no repetir el fallo de Cluckin' Bell 2
//  del 07/09/2026 -- cada /salir vuelve a su propia puerta. Es un negocio
//  APARTE (NEGOCIO_SLOT_TIENDA_ANGELPINE, caja fuerte propia), no una
//  segunda puerta del mismo negocio. Coordenadas de /panel: id 80
//  (/entrar) e id 81 (/comprarnegocio + caja fuerte).
// ---------------------------------------------------------------------
#define TIENDA_ANGELPINE_EXT_X       -2177.92
#define TIENDA_ANGELPINE_EXT_Y       -2307.6
#define TIENDA_ANGELPINE_EXT_Z         30.625
#define TIENDA_ANGELPINE_EXT_A        141.049
#define TIENDA_ANGELPINE_COMPRAR_X   -2173.34
#define TIENDA_ANGELPINE_COMPRAR_Y   -2310.33
#define TIENDA_ANGELPINE_COMPRAR_Z     30.625
#define TIENDA_ANGELPINE_COMPRAR_A    140.109
#define TIENDA_ANGELPINE_PRECIO     175000
#define MUNDO_TIENDA_ANGELPINE        506
// Mismo icono que la tienda original -- constante compartida para que
// las dos NUNCA puedan quedar desincronizadas (antes 19 estaba solo
// hardcodeado dentro de Tienda_OnConectar).
#define TIENDA_ICON_SPRITE             19
// Slot corregido 07/09/2026 -- estaba en 109, fuera del rango valido
// 0-99 (ver nota junto a PESCAPUNTO_ICON_SLOT_BASE).
#define TIENDA_ANGELPINE_ICON_SLOT     9
// Sprite propio (07/09/2026, mismo dia): deja de compartir el 19 con la
// original -- Vlad pidio volver al 17 "Diner" solo para esta sucursal.
#define TIENDA_ANGELPINE_ICON_SPRITE  17

// Cana de pescar y Barbacoa -- unico precio, unica variante (encargo
// expreso: "una cana y punto", nada de niveles ni mejoras).
#define PESCA_PRECIO_CANA         10000
#define PESCA_PRECIO_BARBACOA     10000
// Modelo 19831 "Barbeque1" -- cambiado el 06/09/2026 a peticion de Vlad
// (el 1481 original salia gris y sin gracia). Verificado como objeto
// real de GTA:SA/SA-MP contra gtastuff.com/dev.prineside.com.
#define PESCA_BARBACOA_MODELO     19831

// Cuanto se hunde el objeto respecto a la posicion logica (baZ) para que
// no quede flotando -- MISMO problema y MISMA solucion que
// MARIA_HUNDIDO_Z en 28_marihuana.inc (el pivote del modelo no esta en
// su base). 0.5 no bastó (Vlad lo probó y seguía flotando, 06/09/2026);
// subido a 0.9, el mismo valor que terminó funcionando para la planta.
// Si sigue flotando, subirlo mas; si se entierra, bajarlo. Es el unico
// numero a tocar.
#define PESCA_BARBACOA_HUNDIDO_Z   0.9

#define PESCA_COCINAR_SEGUNDOS      120
#define PESCA_QUEMAR_SEGUNDOS        60   // ventana tras terminar de cocinar antes de quemarse
#define PESCA_COMER_ESCUDO           10.0
#define PESCA_COMER_ESCUDO_TOPE      50.0
// Añadido el 07/09/2026: comer un plato de pesca TAMBIEN sube hambre y
// sed (encima de vida/escudo, arriba), a diferencia de como se entrego
// originalmente el DLC el 06/09/2026.
#define PESCA_COMER_HAMBRE           50.0
#define PESCA_COMER_SED              20.0

// Caña en la mano mientras se usa /pescar -- objeto real de GTA:SA,
// modelo 18632 "FishingRod" (verificado 06/09/2026 contra bases de datos
// de modelos de GTA:SA/SA-MP, gtastuff.com y dev.prineside.com). Ranura 0
// de SetPlayerAttachedObject -- libre hasta ahora (1 = porro, 2-9 = toys,
// ver 28_marihuana.inc / 41_toys.inc). Hueso 6 = mano derecha, mismo
// hueso ya verificado en produccion para el porro. Offset/rotacion a
// cero como punto de partida neutro -- SIN VERIFICAR visualmente
// todavia, avisar si no encaja para ajustarlo.
#define PESCA_CANA_SLOT           0
#define PESCA_CANA_MODELO     18632
#define PESCA_CANA_BONE           6
#define PESCA_CANA_OFFSET_X       0.0
#define PESCA_CANA_OFFSET_Y       0.0
#define PESCA_CANA_OFFSET_Z       0.0
#define PESCA_CANA_ROT_X          0.0
#define PESCA_CANA_ROT_Y          0.0
#define PESCA_CANA_ROT_Z          0.0

// Punto de referencia para /gps, "(Pescar) Punto pesca Los Santos" --
// coordenada que Vlad marco el 06/09/2026 con /marcarcoord (tabla
// `coordenadas`, id 60, nombre "/gps lugar de pesca 1"). Es solo un
// destino de ruta, no un punto interactivo -- los puntos reales donde
// funciona /pescar son los 6 de PESCA1_X.. mas arriba, configurables con
// /setpesca.
#define PESCAGPS_LS_X           374.276
#define PESCAGPS_LS_Y         -2084.17
#define PESCAGPS_LS_Z             7.83594

// Igual que arriba pero para la zona de Angel Pine (07/09/2026, puntos
// 4-6 de PESCA4_X.. mas abajo) -- referencia fija tomada de una de las
// tres coordenadas de /panel ("/pescar angelpine1", id 77).
#define PESCAGPS_ANGELPINE_X   -2694.35
#define PESCAGPS_ANGELPINE_Y   -2215.23
#define PESCAGPS_ANGELPINE_Z      1.06981

// Umbrales de rango de pescador -- SOLO por numero total de capturas,
// nunca por peso/dinero/ventas (encargo expreso). Cosmeticos: no dan
// ninguna ventaja de juego.
new const g_PescaRangoUmbral[] = { 0, 10, 25, 45, 75, 125, 200 };
new const g_PescaRangoNombre[][32] = {
    "Novato", "Aficionado", "Experimentado", "Experto",
    "Patron de Pesca", "Maestro Pescador", "Leyenda de la Pesca"
};
#define PESCA_TOTAL_RANGOS (sizeof g_PescaRangoUmbral)

// ---------------------------------------------------------------------
//  /DUDA -- 11/09/2026, encargo expreso de Vlad: sustituye por completo
//  al viejo /duda (que solo avisaba al staff en vivo, ver 08_chat.inc,
//  ahora quitado -- /reportar ya cubre esa necesidad). El jugador
//  pregunta y Gemini Flash responde solo, sin que el equipo tenga que
//  intervenir -- ver 61_duda.inc para el mecanismo completo (cola en
//  MySQL, tabla `duda_cola`, respondida por GamerolOnlineWeb/src/duda.js
//  igual que ya hace La Reinosa en la web, con el mismo tipo de
//  system prompt restringido a un documento).
// ---------------------------------------------------------------------
#define DUDA_COOLDOWN_SEGUNDOS     60   // mismo valor que tenia el /duda viejo
#define DUDA_PREGUNTA_MAX_LEN     200
#define DUDA_TIMEOUT_SEGUNDOS      20   // si no hay respuesta en este tiempo, se avisa al jugador y se da la duda por perdida
#define DUDA_TICK_MS             2000   // cada cuanto mira el juego si ya hay respuesta esperando

// ---------------------------------------------------------------------
//  EL TELETIPO -- 11/09/2026, canal ambiental de noticias de San Andreas.
//  Gamerol decide QUE ha pasado (triggers reales de abajo), Groq decide
//  COMO contarlo. Mismo esqueleto de cola en MySQL que /duda (ver
//  62_teletipo.inc y GamerolOnlineWeb/src/teletipo.js), pero de un solo
//  sentido y SIN backlog: cooldown GLOBAL de servidor, no por jugador.
// ---------------------------------------------------------------------
#define TELETIPO_COOLDOWN_SEGUNDOS   420   // 7 minutos, global (no por jugador ni por trigger)
#define TELETIPO_TICK_MS            2000   // cada cuanto mira el juego si ya hay noticia esperando
#define TELETIPO_TIMEOUT_SEGUNDOS     30   // si Groq/Node no contestan a tiempo, se descarta en silencio (sin cola persistente)
#define TELETIPO_MAX_NOTICIA_LEN     140   // limite seguro de chat SA:MP/open.mp contando prefijo y colores
#define TELETIPO_PUENTE_VENTANA     3600   // segundos (60 min)
#define TELETIPO_PUENTE_UMBRAL         3
#define TELETIPO_MALETERO_VENTANA   3600   // segundos (60 min)
#define TELETIPO_MALETERO_UMBRAL       3
#define TELETIPO_CANNABIS_UMBRAL      31   // 31 o mas plantas vivas en todo el servidor
#define TELETIPO_HORA_MANANA           6
#define TELETIPO_HORA_NOCHE           22
#define TELETIPO_HORA_CANNABIS_1       3
#define TELETIPO_HORA_CANNABIS_2      15

// ---------------------------------------------------------------------
//  SISTEMA AUTOMATICO DE ANUNCIOS V1 -- 12/09/2026, publicidad in-game.
//  Independiente de El Teletipo: GAMEROL decide el turno fijo (ciclo
//  1-16, sin random), GROQ decide la redaccion (ver 63_anuncios.inc y
//  GamerolOnlineWeb/src/anuncios.js). Reutiliza el mismo limite de
//  linea de chat que El Teletipo (ver TELETIPO_MAX_NOTICIA_LEN).
// ---------------------------------------------------------------------
#define ANUNCIOS_CICLO_MS          900000   // 15 minutos reales, turno fijo
#define ANUNCIOS_TICK_MS             2000   // cada cuanto mira el juego si ya hay anuncio esperando
#define ANUNCIOS_TIMEOUT_SEGUNDOS      30   // si Groq/Node no contestan a tiempo, se descarta en silencio
#define ANUNCIOS_MAX_TEXTO_LEN        140   // mismo limite seguro que El Teletipo
#define ANUNCIOS_TOTAL                 16   // Burger Shot .. REDFIRE Seeds, ciclo fijo, no random
#define ANUNCIOS_COLISION_VENTANA_MS 90000   // si coincide con El Teletipo en menos de esto, se retrasa una vez

// ---------------------------------------------------------------------
//  CRONOS GAMEROL V1 -- 14/09/2026, encargo expreso de Vlad (email
//  "Gamerol 2026 -- Sistema CRONOS GAMEROL V1"): desafios contrarreloj
//  repartidos por San Andreas, creados por admin in-game sin tocar
//  codigo (/crearcrono + /crearmeta), corridos por cualquier jugador con
//  CUALQUIER vehiculo (/crono), ranking persistente de mejor tiempo por
//  jugador (65_cronos.inc, tablas `cronos` + `cronos_ranking`).
// ---------------------------------------------------------------------
#define CRONOS_MAX                    64   // tracks simultaneos que caben en memoria, de sobra para V1
#define CRONOS_PICKUP_MODELO        1248   // GTA III checkpoint pickup -- sin uso previo en el proyecto (comprobado)
#define CRONOS_RADIO_SALIDA           3.0
#define CRONOS_RADIO_META             5.0  // igual al radio tipico de un checkpoint de SA-MP
#define CRONOS_CUENTA_ATRAS_SEGUNDOS   5
#define CRONOS_YA_DURACION_MS        900   // cuanto se queda "¡YA!" antes de ocultarse (el cronometro ya arranco debajo)
#define CRONOS_LIMITE_MS          300000   // 5 minutos, limite duro de un intento (item 6 del encargo)
#define CRONOS_TICK_CORRIENDO_MS     100   // refresco del cronometro visual mientras corres
#define CRONOS_AVISO_FINAL_MS       3000   // cuanto se queda en pantalla "CRONO FINALIZADA"/"TIEMPO AGOTADO"
#define CRONOS_ADMIN_NIVEL    ADMIN_ADMIN  // quien puede /crearcrono, /crearmeta, /borrarcrono

// Premio por RECORD MUNDIAL real (item 11 del encargo) -- nunca en el
// primerisimo tiempo de una crono (item 10), solo al SUPERAR una marca
// ya existente.
#define CRONOS_PREMIO_DINERO        2500
#define CRONOS_PREMIO_MATERIALES   1000
#define CRONOS_PREMIO_MARIHUANA     100
