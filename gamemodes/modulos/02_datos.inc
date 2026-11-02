/* =====================================================================
 *  02_datos.inc
 *  ---------------------------------------------------------------------
 *  Estructura de datos del jugador, variables globales compartidas y
 *  prototipos de las funciones que se usan entre modulos.
 *
 *  En Pawn una funcion tiene que estar declarada antes de usarse. Como
 *  los modulos se llaman entre si en los dos sentidos, aqui abajo hay
 *  una lista de prototipos que rompe esa dependencia circular.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  FICHA COMPLETA DEL JUGADOR
//  Los campos marcados como SESION no se guardan en la base de datos.
// ---------------------------------------------------------------------
enum E_JUGADOR
{
    // --- Identidad -------------------------------------------------
    jID,                                    // cuentas.id (0 = sin cuenta)
    jNombre[MAX_PLAYER_NAME + 1],
    jPassword[BCRYPT_HASH_LENGTH],
    jEmail[64],
    jOrigen[32],

    // --- Personaje -------------------------------------------------
    jSexo,                                  // 1 hombre, 2 mujer
    jEdad,
    jSkin,

    // El aspecto con el que se creo el personaje. jSkin es el de AHORA
    // (que puede ser un uniforme); este es el de siempre, y es al que
    // se vuelve al quedarse sin uniforme. Ver Skin_Actualizar().
    jSkinInicial,
    jNivel,
    jRespeto,
    jMinutosTotales,
    jMinutosPayday,
    jPaydays,

    // --- Economia --------------------------------------------------
    jDinero,
    jBanco,
    jCuentaBancaria,

    // --- Ocupacion -------------------------------------------------
    jTrabajo[MAX_TRABAJOS_JUGADOR],
    jTrabajoRango[MAX_TRABAJOS_JUGADOR],
    jFaccion,
    jFaccionRango,
    bool:jFaccionLider,

    // --- Staff autonomo / Server IA (58_ascensos.inc) ---------------
    //  jStaffRango es SOLO un titulo -- no desbloquea ningun comando
    //  todavia (ver el informe de aprobacion de permisos en el modulo).
    jStaffRango,                            // 0 = ninguno, 1..6 en V1
    jStaffOfertas,                          // bitmask: hitos ya informados/ofrecidos, ver ASCENSOS_BIT_*
    jPoliciaIngreso,                        // gettime() de cuando entro en la Policia via Server IA, 0 = nunca

    // --- Estado fisico ---------------------------------------------
    Float:jSalud,
    Float:jArmadura,
    Float:jPosX,
    Float:jPosY,
    Float:jPosZ,
    Float:jPosA,
    jInterior,
    jMundo,
    bool:jHerido,

    // --- Hambre y sed (44_hambre_sed.inc) ---------------------------
    Float:jHambre,                          // 100..0, ver HAMBRE_SEGUNDOS_VACIO
    Float:jSed,                             // 100..0, ver SED_SEGUNDOS_VACIO
    jHambreAvisoHasta,                      // GetTickCount hasta el que no se repite el aviso
    jSedAvisoHasta,                         // idem para sed
    jHambreSedPenalizacionHasta,            // gettime() hasta el que no se repite el drenado de vida a 0%

    // --- Gamerol Aventuras: DLC Pesca (51_pesca.inc) ----------------
    jPescaCapturas,                         // total de peces capturados -- unica base del rango de pescador
    jPescaComidos,                          // platos de pesca consumidos -- solo para el ranking de /pescas

    // --- Inventario ------------------------------------------------
    Float:jPesoExtra,                       // Capacidad extra por mochilas

    // --- Policia (37_policia.inc) ----------------------------------
    //  Ojo: jCarcelSegundos (mas abajo) es la carcel ADMINISTRATIVA que
    //  pone un admin con /carcel. jCarcelRolSegundos es la de ROL, que
    //  pone un policia con /detener. Son cosas distintas y no se mezclan.
    jTelefono,                              // Numero de movil (se genera al crear la cuenta)
    jCarnet,                                // 0 sin carnet, 1 con carnet en vigor
    jCarnetPuntos,                          // Los 12 puntos de toda la vida
    jBuscado,                               // Puntos de busqueda (caducan solos)
    jCondenaSegundos,                       // Carcel pendiente por delitos sin cumplir
    jCarcelRolSegundos,                     // Carcel de rol que se esta cumpliendo

    // --- Administracion --------------------------------------------
    jAdmin,
    jAvisos,
    jMuteSegundos,
    jCarcelSegundos,
    bool:jBaneado,

    // --- SESION ----------------------------------------------------
    bool:jLogueado,
    bool:jRegistrado,                       // Existe en la base de datos
    bool:jSpawneado,
    jIntentosLogin,
    jSegundosLogin,
    jSegundosSesion,
    jUltimaActividad,                       // GetTickCount de la ultima accion
    bool:jAFK,
    jTemporal,                              // Valor auxiliar entre dialogos
    jTemporalTexto[64],                     // Texto auxiliar entre dialogos
    jTemporal2,
    bool:jEnBanco,
    bool:jAdminOculto,
    jSpecObjetivo,
    Float:jSpecPos[4],
    jSpecInterior,
    jSpecMundo,
    bool:jCongelado,
    jVehiculoDentro,                        // ID persistente del vehiculo (como conductor)
    jVehiculoPasajero,                      // idem, pero como pasajero (cartel de dropeo)
    jCamaraAvisoHasta,                      // GetTickCount hasta el que no se repite el aviso de chat (global, cualquier camara)
    jCamaraRegistroCancela,                 // cancelas.id de la ultima camara que registro nombre+matricula
    jCamaraRegistroHasta,                   // GetTickCount hasta el que no se repite ese registro
    jAvisoMuerte,
    bool:jAvisoMotorLibre,                  // Ya vio el aviso de /adminmotor o /nfsmotor esta sesion

    // --- Policia: estado de sesion (37_policia.inc) -----------------
    //  Nada de esto se guarda: unas esposas o una multa a medias no
    //  tienen sentido despues de reconectar.
    bool:jEsposado,
    jEsposadoPor,                           // playerid del agente, para el mensaje al soltar
    jMultaImporte,                          // 0 = no hay multa pendiente
    jMultaAgente,                           // playerid del agente que la puso
    jMultaMotivo[64],
    jMultaExpira,                           // gettime() en que caduca la oferta
    jMinutosCaduca,                         // Cuenta atras interna para bajar un punto de busqueda
    j911Ultimo,                             // gettime() de la ultima llamada al 911 (antiflood)

    // --- Ascensos: oferta pendiente, solo sesion (58_ascensos.inc) --
    jOfertaTipo,                             // 0 = ninguna, ver OFERTA_* en 58_ascensos.inc
    jOfertaExpira,                           // gettime() hasta el que sigue vigente
    jOfertaDato,                             // rango que otorgaria (solo OFERTA_STAFF)
    bool:jAscensosReoferta,                  // true solo si el ultimo /ascensos mostrado tenia boton "Pedir info" -- ver Ascensos_Dialogo

    // --- Ascensos: cooldown/limite de /expulsarcomunidad, solo sesion --
    jStaffCooldown,                          // gettime() hasta el que no puede volver a usarlo
    jStaffHoraInicio,                        // gettime() de inicio de la ventana de 1h actual, 0 = sin abrir
    jStaffAccionesHora,                      // acciones punitivas usadas en la ventana actual

    // --- Caido (36_medicos.inc) -------------------------------------
    Float:jCaidaX,
    Float:jCaidaY,
    Float:jCaidaZ,
    jCaidaInterior,
    jCaidaMundo,
    jHeridoDesde                             // gettime() de cuando cayo (minimo antes de /automatico)
}
new Jugador[MAX_PLAYERS][E_JUGADOR];

// ---------------------------------------------------------------------
//  VARIABLES GLOBALES
// ---------------------------------------------------------------------
new MySQL:g_BD = MYSQL_INVALID_HANDLE;      // Conexion principal
new g_Conectados = 0;                       // Jugadores logueados ahora mismo
new g_SegundosServidor = 0;                 // Uptime en segundos

// Semana ISO en curso ("2026-31"), en el mismo formato que usan las vistas
// de ranking de MySQL. Se refresca sola cada minuto: los modulos la leen de
// aqui en vez de lanzar una consulta por cada carrera terminada.
new g_SemanaIso[10];

// ---------------------------------------------------------------------
//  UTILIDADES DE CADENAS DE FORMATO
// ---------------------------------------------------------------------
new g_Buffer[512];                          // Buffer de formato de uso general

// ---------------------------------------------------------------------
//  PROTOTIPOS ENTRE MODULOS
// ---------------------------------------------------------------------

// 03_utiles
forward SpawnJugadorEnMundo(playerid);
forward GuardarPosicionActual(playerid);

// 05_basedatos
forward BD_GuardarCuenta(playerid);
forward BD_RegistrarLog(const admin[], nivel, const accion[], const objetivo[], const detalles[]);
forward BD_RegistrarSancion(cuentaid, const nombre[], tipo, const admin[], const motivo[], duracion);

// 06_login
forward Login_TerminarSesion(playerid);
forward Login_MostrarPantalla(playerid);
forward Login_OcultarPantalla(playerid);
forward Login_IniciarCreacionPersonaje(playerid);

// 08_chat
forward MensajeCercano(playerid, Float:radio, color, const texto[]);
forward MensajeCercanoPos(Float:x, Float:y, Float:z, interior, mundo, Float:radio, color, const texto[]);

// 10_inventario
forward Inv_Cargar(playerid);
forward Inv_Guardar(playerid);

// 41_toys
forward Toys_Cargar(playerid);
forward Toys_Guardar(playerid);
forward Inv_DarItem(playerid, item, cantidad, datos);
forward Inv_QuitarItem(playerid, item, cantidad);
forward Inv_TieneItem(playerid, item);
forward Inv_Limpiar(playerid);
forward Inv_DarEquipoInicial(playerid);
forward Float:Inv_PesoActual(playerid);
forward Float:Inv_PesoMaximo(playerid);

// 11_trabajos
forward bool:Trabajo_Tiene(playerid, trabajo);
forward Trabajo_Contar(playerid);
forward Trabajo_Nombre(trabajo, destino[], tam);
forward Trabajo_NombreRango(trabajo, rango, destino[], tam);

// 12_facciones
forward Faccion_Nombre(faccion, destino[], tam);
forward Faccion_NombreRango(faccion, rango, destino[], tam);
forward bool:Faccion_EsPolicial(faccion);
forward Faccion_Color(faccion);

// 13_vehiculos
forward Veh_Guardar(indice);
forward Veh_GuardarTodos();
forward Veh_BuscarPorObjeto(vehicleid);
forward bool:Veh_PuedeUsar(playerid, indice);
forward Veh_Crear(modelo, propietario, tipo, tipoid, rangomin, Float:x, Float:y, Float:z, Float:a, color1, color2, interior, mundo, precio);
forward Veh_ContarDeJugador(playerid);
forward Veh_ActualizarCierreJugador(playerid);

forward Conce_NombreModelo(modelo, destino[], tam);

// 14_tuning -- llamados desde 13_vehiculos.inc (Veh_Aparecer/Veh_Borrar),
// que se incluye antes que este modulo. Mismo motivo que Dropeo_AlBorrarVehiculo.
forward Tuning_Aplicar(indice);
forward Tuning_AlBorrarVehiculo(indice);

// 15_banco
forward Banco_Movimiento(cuentaid, tipo, cantidad, saldo, const contraparte[], const concepto[]);
forward bool:Banco_EstaDentro(playerid);

// 42_comida -- llamados desde 15_banco.inc (/entrar, /salir), que se
// incluye antes que este modulo. Mismo motivo que Tienda_IntentarEntrar.
forward bool:Comida_IntentarEntrar(playerid);
forward bool:Comida_IntentarSalir(playerid);

// 16_administracion
forward Admin_Aviso(const texto[]);
forward Admin_NombreNivel(nivel, destino[], tam);
forward Admin_MostrarComandos(playerid);

// 17_payday
forward Payday_Entregar(playerid);

// 22_dropeo
forward Dropeo_OnConectar(playerid);
forward Dropeo_OnDesconectar(playerid);
forward Dropeo_OnSpawn(playerid);
forward bool:Dropeo_PuedeArrancar(playerid, indice);
forward Dropeo_AlBorrarVehiculo(indice);
forward Dropeo_IntentarForzar(playerid);
forward Dropeo_Dialogo(playerid, dialogid, response, listitem, const inputtext[]);

// 25_vendedorarmas
forward VendedorArmas_Iniciar();
forward VendedorArmas_OnConnect(playerid);
forward VendedorArmas_OnDisconnect(playerid);
forward VendedorArmas_OnSpawn(playerid);
forward VendedorArmas_OnEnterCheckpoint(playerid);
forward VendedorArmas_TrabajoRetirado(playerid);
forward VendedorArmas_TrasContratar(playerid);

// 26_nfs
forward NFS_Iniciar();
forward NFS_OnConnect(playerid);
forward NFS_OnDisconnect(playerid);
forward NFS_TrabajoRetirado(playerid);
forward NFS_TrasContratar(playerid);

// 28_marihuana
forward Porro_OnConectar(playerid);
forward Porro_OnDesconectar(playerid);
forward Porro_OnMuerte(playerid);

// 29_vendedormarihuana
forward MariaVendedor_OnConnect(playerid);
forward MariaVendedor_OnDisconnect(playerid);
forward MariaVendedor_OnDeath(playerid);
forward MariaVendedor_TrabajoRetirado(playerid);
forward MariaVendedor_TrasContratar(playerid);

// 30_gps -- llamados desde 13_vehiculos.inc, que se incluye antes que este modulo.
forward GPS_MostrarURL(playerid);
forward GPS_OcultarURL(playerid);

// 23_panel
forward Panel_Dialogo(playerid, dialogid, response, listitem, const inputtext[]);
forward Panel_AlCargarSanciones(playerid);
forward Panel_AlBuscarCuentas(playerid);
forward Panel_AlCargarFichaCuenta(playerid, cuentaid);

// 24_casas
forward Casa_Dialogo(playerid, dialogid, response, listitem, const inputtext[]);
forward AlCargarCasas();
forward AlCargarAlmacenCasas();

// 32_negocios
forward Negocio_Dialogo(playerid, dialogid, response, listitem, const inputtext[]);
forward Negocio_OnConectar(playerid);
forward Negocio_IngresoTienda(precio, slotTienda);
forward Negocio_IngresoComprarCoche(precio);
forward Negocio_IngresoNomina();
forward Negocio_IngresoImpuestoVehiculos(cantidad);
forward Negocio_IngresoNFS(remanente);

// 32_negocios (sigue)
forward Negocio_IngresoMulta(cantidad);
forward Negocio_Posicion(slot, &Float:x, &Float:y, &Float:z, &interior, &mundo);

// 37_policia
forward Policia_Segundo(playerid);
forward Policia_Minuto(playerid);
forward Policia_OnSpawn(playerid);
forward Policia_OnDisconnect(playerid);
forward Policia_Dialogo(playerid, dialogid, response, listitem, const inputtext[]);
forward bool:Policia_EsAgente(playerid);
forward Policia_CrearAviso(tipo, sospechoso, Float:x, Float:y, Float:z, interior, mundo);
forward Policia_AvisosSegundo();
forward Policia_QuitarPuntosCarnet(playerid, puntos, const agente[]);
forward Policia_Estrellas(puntos);
forward Policia_AplicarBusqueda(playerid);
forward AlCargarAntecedentes(playerid);

// Uniformes. Skin_Actualizar (03_utiles.inc) se incluye ANTES que los
// modulos de trabajos y facciones, asi que necesita sus prototipos.
forward Faccion_SkinDe(faccion, rango);
forward Trabajo_SkinDe(trabajo);
forward Skin_Actualizar(playerid);

// 37_policia -- la comisaria. Estas tres las llama 15_banco.inc (los
// comandos /entrar y /salir genericos) y 99_callbacks.inc, y los dos se
// incluyen ANTES que 37_policia.inc.
forward Policia_Iniciar();
forward bool:Policia_IntentarEntrar(playerid);
forward bool:Policia_IntentarSalir(playerid);
forward Policia_ComisariaSegundo();

// 36_medicos
forward Medicos_TrabajoRetirado(playerid);
forward Medicos_TrasContratar(playerid);
forward Medicos_ResolverAdmin(playerid);
forward Medicos_OnDisconnect(playerid);
forward Medicos_Dialogo(playerid, dialogid, response, listitem, const inputtext[]);
