/* =====================================================================
 *  37_policia.inc
 *  ---------------------------------------------------------------------
 *  Mecanica de la Policia Nacional (ranura 1 de 12_facciones.inc).
 *
 *  La faccion en si -- rangos, /f, /dep, /invitar, coches -- ya la da
 *  12_facciones.inc. Este archivo es SOLO lo que la convierte en policia:
 *  buscados, multas, esposas, detenciones y carcel de rol.
 *
 *  ------------------------------- LA IDEA -------------------------------
 *  Esto no es "una faccion con juguetes": es la capa de CONSECUENCIA de
 *  los sistemas ilegales que ya existen (cultivo, NFS, dropeo, vendedor
 *  de armas). Cada delito del codigo penal de abajo existe porque hay
 *  algo concreto en el servidor que castigar, no porque toque tenerlo.
 *  -----------------------------------------------------------------------
 *
 *  CODIGO PENAL FIJO. El agente NO escribe el delito ni elige la pena:
 *  elige un codigo de una tabla que cualquiera puede leer con
 *  /codigopenal. Los puntos, la multa y los minutos estan escritos. Es
 *  la diferencia grande con el sistema del que viene la idea (SpaceRol,
 *  2010), donde el policia escribia el delito en texto libre y decidia
 *  cuanto valia -- una invitacion al abuso y a la discusion.
 *
 *  DOS COSAS QUE NO SON LO MISMO:
 *
 *    BUSCADO (jBuscado)          puntos que CADUCAN solos, 1 cada
 *                                POLICIA_MINUTOS_CADUCA minutos de juego.
 *                                Son los que pintan las estrellas.
 *                                Si te escondes y no reincides, dejas de
 *                                estar buscado sin hacer nada.
 *
 *    ANTECEDENTES (tabla SQL)    NO caducan nunca. Es el historial que
 *                                la policia consulta con /antecedentes.
 *
 *  Estar buscado es un estado; tener antecedentes es una reputacion.
 *
 *  Y OTRAS DOS QUE TAMPOCO:
 *
 *    jCarcelSegundos     carcel ADMINISTRATIVA (/carcel de un admin). OOC.
 *    jCarcelRolSegundos  carcel de ROL (/detener de un policia). IC.
 *
 *  Se parecen mucho por dentro -- de hecho esta reutiliza el mismo patron
 *  de tic por segundo de 16_administracion.inc -- pero se guardan aparte
 *  a proposito: un ban por saltarse las normas y una detencion por robar
 *  un coche no pueden salir iguales en los logs.
 *
 *  LAS MULTAS SE ACEPTAN, no se imponen: el agente ofrece y el multado
 *  paga con /aceptarmulta. Si se niega, la negativa es delito. El dinero
 *  va integro a Bonos del Estado (32_negocios.inc), no a la policia.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  EL CODIGO PENAL
//
//  Para añadir un delito: una fila mas aqui y ya esta. No hay que tocar
//  ningun comando -- /delito, /multar y /codigopenal leen esta tabla.
//
//  Los codigos van por centenas segun el tipo, para que se lean solos:
//    1xx trafico  ·  2xx drogas  ·  3xx armas
//    4xx vehiculos  ·  5xx personas  ·  6xx contra la autoridad
// ---------------------------------------------------------------------
enum E_DELITO
{
    deCodigo,
    deNombre[52],
    dePuntos,
    deMulta,
    deMinutos                               // Minutos de carcel que suma
};

static const g_CodigoPenal[][E_DELITO] =
{
    // Trafico -- el pan de cada dia de un control
    {101, "Circular con los papeles caducados",      1,  2000,  0},
    {102, "Exceso de velocidad",                     1,  1500,  0},
    {103, "Conduccion temeraria",                    2,  4000,  2},
    {104, "Conducir sin carnet en vigor",            2,  6000,  5},

    // Drogas -- engancha con 28_marihuana y 29_vendedormarihuana
    {201, "Tenencia de marihuana",                   2,  5000,  5},
    {202, "Cultivo de marihuana",                    3, 10000, 10},
    {203, "Trafico de marihuana",                    4, 20000, 15},

    // Armas -- engancha con 25_vendedorarmas
    {301, "Tenencia ilicita de armas",               4, 15000, 15},
    {302, "Venta ilegal de armas",                   5, 30000, 25},

    // Vehiculos -- engancha con 22_dropeo
    {401, "Robo de vehiculo",                        4, 12000, 15},
    // 402 "Trafico de vehiculos" se quito el 25/08/2026: era el delito
    // que se imputaba por usar /nfsmotor, pero eso es el propio trabajo
    // de la NFS, no algo que la policia deba perseguir automaticamente
    // (ver 26_nfs.inc y el historial de cambios grandes de LEEME.md).
    {403, "Forzar el maletero de un vehiculo ajeno", 4, 12000, 15},
    {404, "Puentear un vehiculo libre sin permiso",  2,  4000,  5},

    // Personas
    {501, "Agresion",                                3,  8000, 10},
    {502, "Homicidio",                               8, 50000, 45},
    {503, "Atraco a un local",                       5, 25000, 20},

    // Hacking -- engancha con 45_maletero.inc / 31_web.inc. Delito grave
    // a proposito (encargo de Vlad, 06/09/2026): el Portatil Hacker da
    // acceso a plantaciones, maleteros cargados y al Coche Secreto de
    // Federico, asi que tenerlo encima pesa como un delito gordo de
    // verdad, no una multa de tramite.
    {701, "Tenencia de un Portatil Hacker",          5, 25000, 20},

    // Contra la autoridad
    {601, "Resistencia a la autoridad",              2,  5000,  5},
    {602, "Fuga",                                    3,     0, 10},
    {603, "Agresion a un agente",                    6, 40000, 35},
    {604, "Negarse a pagar una multa",               2,     0,  5},
    {605, "Denuncia falsa al 112",                   2,  3000,  5}
};

// ---------------------------------------------------------------------
//  CONSULTAS BASICAS
// ---------------------------------------------------------------------

// Indice en g_CodigoPenal a partir del codigo, o -1 si no existe.
stock Policia_BuscarDelito(codigo)
{
    for (new i = 0; i < sizeof g_CodigoPenal; i++)
    {
        if (g_CodigoPenal[i][deCodigo] == codigo) return i;
    }
    return -1;
}

// True si el jugador es policia con rango suficiente para actuar.
// El Aspirante (rango 1) acompaña pero no actua.
stock bool:Policia_EsAgente(playerid)
{
    if (!Jugador[playerid][jLogueado]) return false;
    if (Jugador[playerid][jFaccion] != POLICIA_FACCION) return false;
    return (Jugador[playerid][jFaccionRango] >= POLICIA_RANGO_AGENTE);
}

// Igual pero avisando, para no repetir el mismo if en cada comando.
stock bool:Policia_NoEsAgente(playerid)
{
    if (Policia_EsAgente(playerid)) return false;

    if (Jugador[playerid][jFaccion] == POLICIA_FACCION)
        Error(playerid, "Los Aspirantes no pueden actuar todavia. Acompaña y aprende.");
    else
        Error(playerid, "Ese comando es solo para la Policia Nacional.");

    return true;
}

// Puntos -> estrellas de busqueda (0 a 6, el maximo de SA-MP).
stock Policia_Estrellas(puntos)
{
    if (puntos <= 0)  return 0;
    if (puntos <= 2)  return 1;
    if (puntos <= 4)  return 2;
    if (puntos <= 6)  return 3;
    if (puntos <= 9)  return 4;
    if (puntos <= 13) return 5;
    return 6;
}

// Deja las estrellas del jugador en sintonia con sus puntos.
// Se llama siempre que jBuscado cambia.
stock Policia_AplicarBusqueda(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    if (Jugador[playerid][jBuscado] > POLICIA_MAX_PUNTOS)
        Jugador[playerid][jBuscado] = POLICIA_MAX_PUNTOS;
    if (Jugador[playerid][jBuscado] < 0)
        Jugador[playerid][jBuscado] = 0;

    SetPlayerWantedLevel(playerid, Policia_Estrellas(Jugador[playerid][jBuscado]));
    return 1;
}

// Avisa por el canal departamental a todos los policias conectados.
//
// OJO: usa un buffer PROPIO, no g_Buffer. Quien llama a esta funcion
// casi siempre acaba de montar el texto con Formato(...), que escribe
// justo en g_Buffer -- si aqui volvieramos a formatear sobre el mismo
// sitio usandolo ademas como origen, el texto se pisaria a si mismo.
stock Policia_AvisarCentral(const texto[])
{
    new linea[160];
    format(linea, sizeof linea, "[CENTRAL] %s", texto);

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jFaccion] != POLICIA_FACCION) continue;
        Linea(i, COLOR_AZUL, linea);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  LA COMISARIA
//  ---------------------------------------------------------------------
//  Interior 6 del juego, mundo 0. Ni un objeto de mapeo: el interior ya
//  existe, y ahi dentro estan tambien las celdas (CARCEL_X/Y/Z), que es
//  justo el motivo de haber elegido ese y no otro. Ver el bloque LA
//  COMISARIA de 01_configuracion.inc.
//
//  TRES ZONAS, y la gracia esta en la del medio:
//
//    LA CALLE         Dos puertas, las dos con /entrar: la principal
//                     en Pershing Square (publica) y la de personal en
//                     el garaje de atras (solo faccion, y te deja ya
//                     pasado el control).
//    EL VESTIBULO     Publico. Entra CUALQUIERA -- a denunciar, a
//                     preguntar o a que le den largas. Una comisaria en
//                     la que solo puede entrar la policia no es una
//                     comisaria, es un cuartel.
//    LA ZONA INTERNA  Detras de las puertas de control. Solo agentes.
//                     Ahi estan la armeria y los calabozos.
//
//  Las puertas son de SpaceRol (2010), que ya tenia las coordenadas
//  medidas. Lo unico que no se ha copiado de alli es el fallo: alli se
//  creaban sin mundo ni interior y salian flotando en todos los
//  interiores del servidor.
// ---------------------------------------------------------------------

static STREAMER_TAG_OBJECT:g_PuertaComisariaI;
static STREAMER_TAG_OBJECT:g_PuertaComisariaD;
static bool:g_PuertaComisariaAbierta = false;

// Segundos que le quedan abierta antes del cierre automatico. 0 = no
// hay cuenta atras en marcha.
static g_PuertaComisariaCierre = 0;

static STREAMER_TAG_PICKUP:g_PickupComisariaDentro;
static STREAMER_TAG_PICKUP:g_PickupTraseraFuera;
static STREAMER_TAG_PICKUP:g_PickupTraseraDentro;
static STREAMER_TAG_PICKUP:g_PickupArmeria;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaComisariaDentro;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaTraseraFuera;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaTraseraDentro;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaArmeria;

// Espera de la armeria, por jugador. En segundos, la baja Policia_Segundo.
static g_ArmeriaEspera[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  MARCADORES
//  Llamado desde OnGameModeInit (99_callbacks.inc), en el bloque de
//  contenido del mundo. Mismo patron que Banco_CrearMarcadores y
//  Tienda_Iniciar.
// ---------------------------------------------------------------------
public Policia_Iniciar()
{
    // La puerta de la calle NO lleva marcador propio: la sede de la
    // faccion (12_facciones.inc) ya tiene el suyo en esta misma
    // coordenada, y dos etiquetas 3D en el mismo punto se solapan y no
    // hay quien las lea. El "/entrar" lo dice esa etiqueta, via el
    // campo faEntrada de la ranura 1.

    // Puerta desde el vestibulo.
    g_PickupComisariaDentro = CreateDynamicPickup(1239, 23,
        COMISARIA_INT_X, COMISARIA_INT_Y, COMISARIA_INT_Z,
        MUNDO_NORMAL, COMISARIA_INTERIOR, -1, 100.0);

    g_EtiquetaComisariaDentro = CreateDynamic3DTextLabel(
        "{FFFFFF}Escribe {E8B33A}/salir{FFFFFF} para volver a la calle\n{FFFFFF}o {E8B33A}/911{FFFFFF} si vienes a denunciar",
        COLOR_BLANCO, COMISARIA_INT_X, COMISARIA_INT_Y, COMISARIA_INT_Z + 0.7, 8.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, MUNDO_NORMAL, COMISARIA_INTERIOR, -1, 25.0);

    // La entrada trasera, en el garaje. Esta si lleva marcador propio:
    // ahi no hay nada de la faccion que lo diga.
    g_PickupTraseraFuera = CreateDynamicPickup(1239, 23,
        COMISARIA_TRAS_X, COMISARIA_TRAS_Y, COMISARIA_TRAS_Z, 0, 0, -1, 100.0);

    g_EtiquetaTraseraFuera = CreateDynamic3DTextLabel(
        "ENTRADA DE PERSONAL\n{FFFFFF}Solo Policia Nacional. {E8B33A}/entrar",
        COLOR_AZUL, COMISARIA_TRAS_X, COMISARIA_TRAS_Y, COMISARIA_TRAS_Z + 0.7, 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    g_PickupTraseraDentro = CreateDynamicPickup(1239, 23,
        COMISARIA_TRASINT_X, COMISARIA_TRASINT_Y, COMISARIA_TRASINT_Z,
        MUNDO_NORMAL, COMISARIA_INTERIOR, -1, 100.0);

    g_EtiquetaTraseraDentro = CreateDynamic3DTextLabel(
        "{FFFFFF}Escribe {E8B33A}/salir{FFFFFF} para bajar al garaje",
        COLOR_BLANCO, COMISARIA_TRASINT_X, COMISARIA_TRASINT_Y, COMISARIA_TRASINT_Z + 0.7, 8.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, MUNDO_NORMAL, COMISARIA_INTERIOR, -1, 25.0);

    // Armeria, ya en la zona interna.
    g_PickupArmeria = CreateDynamicPickup(1242, 23, ARMERIA_X, ARMERIA_Y, ARMERIA_Z,
        MUNDO_NORMAL, COMISARIA_INTERIOR, -1, 100.0);

    g_EtiquetaArmeria = CreateDynamic3DTextLabel(
        "ARMERIA\n{FFFFFF}Escribe {E8B33A}/equipo{FFFFFF} para recoger el reglamentario",
        COLOR_AZUL, ARMERIA_X, ARMERIA_Y, ARMERIA_Z + 0.7, 8.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, MUNDO_NORMAL, COMISARIA_INTERIOR, -1, 25.0);

    // Las dos hojas del control de acceso, cerradas. Con su mundo y su
    // interior: sin eso saldrian en todos los interiores del servidor.
    g_PuertaComisariaI = CreateDynamicObject(COMISARIA_PUERTA_MODELO,
        COMISARIA_PI_CX, COMISARIA_PI_CY, COMISARIA_PUERTA_Z, 0.0, 0.0, 0.0,
        MUNDO_NORMAL, COMISARIA_INTERIOR, -1, 100.0);

    g_PuertaComisariaD = CreateDynamicObject(COMISARIA_PUERTA_MODELO,
        COMISARIA_PD_CX, COMISARIA_PD_CY, COMISARIA_PUERTA_Z, 0.0, 0.0, 0.0,
        MUNDO_NORMAL, COMISARIA_INTERIOR, -1, 100.0);

    g_PuertaComisariaAbierta = false;
    g_PuertaComisariaCierre = 0;

    print("  [POLICIA] Comisaria de Los Santos lista.");
    return 1;
}

// ---------------------------------------------------------------------
//  ENTRAR Y SALIR
//  Los comandos /entrar y /salir viven en 15_banco.inc (son el punto de
//  entrada generico a cualquier interior fijo del servidor); estas dos
//  funciones son las que llama desde alli. Igual que la gasolinera.
//
//  Sin filtro de faccion a proposito: el vestibulo es publico.
// ---------------------------------------------------------------------
public bool:Policia_IntentarEntrar(playerid)
{
    if (GetPlayerInterior(playerid) != 0) return false;

    // --- La trasera del garaje: solo gente de la faccion, y te deja ya
    //     dentro, pasado el control.
    if (EnPunto(playerid, 3.0, COMISARIA_TRAS_X, COMISARIA_TRAS_Y, COMISARIA_TRAS_Z))
    {
        if (Jugador[playerid][jFaccion] != POLICIA_FACCION)
        {
            Error(playerid, "Esta puerta es solo para el personal de la comisaria.");
            Info(playerid, "La entrada publica esta en la fachada, en Pershing Square.");
            return true;
        }

        SetPlayerPos(playerid, COMISARIA_TRASINT_X, COMISARIA_TRASINT_Y, COMISARIA_TRASINT_Z);
        SetPlayerFacingAngle(playerid, COMISARIA_TRASINT_A);
        SetPlayerInterior(playerid, COMISARIA_INTERIOR);
        SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
        SetCameraBehindPlayer(playerid);

        Info(playerid, "Entras por la puerta de personal. La armeria la tienes al lado.");
        return true;
    }

    // --- La principal: vestibulo publico, entra cualquiera.
    if (!EnPunto(playerid, 2.5, COMISARIA_EXT_X, COMISARIA_EXT_Y, COMISARIA_EXT_Z)) return false;

    SetPlayerPos(playerid, COMISARIA_INT_X, COMISARIA_INT_Y, COMISARIA_INT_Z);
    SetPlayerFacingAngle(playerid, COMISARIA_INT_A);
    SetPlayerInterior(playerid, COMISARIA_INTERIOR);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
    SetCameraBehindPlayer(playerid);

    if (Jugador[playerid][jFaccion] == POLICIA_FACCION)
        Info(playerid, "Comisaria de Los Santos. La armeria esta pasando el control.");
    else
        Info(playerid, "Comisaria de Los Santos. Del control de acceso no se pasa.");

    return true;
}

public bool:Policia_IntentarSalir(playerid)
{
    if (GetPlayerInterior(playerid) != COMISARIA_INTERIOR) return false;
    if (GetPlayerVirtualWorld(playerid) != MUNDO_NORMAL) return false;

    // Por la trasera sale quien haya entrado por ella: es la misma
    // puerta, y ya se comprobo la faccion al entrar. Sin filtro aqui a
    // proposito -- si un detenido se escapa por el garaje, eso es rol,
    // no un fallo.
    if (EnPunto(playerid, 3.0, COMISARIA_TRASINT_X, COMISARIA_TRASINT_Y, COMISARIA_TRASINT_Z))
    {
        SetPlayerPos(playerid, COMISARIA_TRAS_X, COMISARIA_TRAS_Y, COMISARIA_TRAS_Z);
        SetPlayerFacingAngle(playerid, COMISARIA_TRAS_A);
        SetPlayerInterior(playerid, 0);
        SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
        SetCameraBehindPlayer(playerid);
        return true;
    }

    if (!EnPunto(playerid, 3.0, COMISARIA_INT_X, COMISARIA_INT_Y, COMISARIA_INT_Z)) return false;

    SetPlayerPos(playerid, COMISARIA_EXT_X, COMISARIA_EXT_Y, COMISARIA_EXT_Z);
    SetPlayerFacingAngle(playerid, COMISARIA_EXT_A);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
    SetCameraBehindPlayer(playerid);
    return true;
}

// ---------------------------------------------------------------------
//  LAS PUERTAS DEL CONTROL
// ---------------------------------------------------------------------

stock Policia_MoverPuertas(bool:abrir)
{
    if (abrir)
    {
        MoveDynamicObject(g_PuertaComisariaI, COMISARIA_PI_AX, COMISARIA_PI_AY,
            COMISARIA_PUERTA_Z, COMISARIA_PUERTA_VEL);
        MoveDynamicObject(g_PuertaComisariaD, COMISARIA_PD_AX, COMISARIA_PD_AY,
            COMISARIA_PUERTA_Z, COMISARIA_PUERTA_VEL);

        g_PuertaComisariaAbierta = true;
        g_PuertaComisariaCierre = COMISARIA_PUERTA_CIERRE;
    }
    else
    {
        MoveDynamicObject(g_PuertaComisariaI, COMISARIA_PI_CX, COMISARIA_PI_CY,
            COMISARIA_PUERTA_Z, COMISARIA_PUERTA_VEL);
        MoveDynamicObject(g_PuertaComisariaD, COMISARIA_PD_CX, COMISARIA_PD_CY,
            COMISARIA_PUERTA_Z, COMISARIA_PUERTA_VEL);

        g_PuertaComisariaAbierta = false;
        g_PuertaComisariaCierre = 0;
    }
    return 1;
}

// Aviso a quien este dentro de la comisaria, sea o no policia: la puerta
// se oye. No usa Policia_AvisarCentral porque esto no es un aviso de la
// central, es un ruido en una habitacion.
stock Policia_AvisarPuerta(const texto[])
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (GetPlayerInterior(i) != COMISARIA_INTERIOR) continue;
        if (GetPlayerVirtualWorld(i) != MUNDO_NORMAL) continue;
        Linea(i, COLOR_ACCION, texto);
    }
    return 1;
}

CMD:puerta(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    if (GetPlayerInterior(playerid) != COMISARIA_INTERIOR
        || GetPlayerVirtualWorld(playerid) != MUNDO_NORMAL)
    {
        Error(playerid, "Tienes que estar dentro de la comisaria.");
        return 1;
    }

    // El punto de mando esta entre las dos hojas, asi que una sola
    // comprobacion de distancia vale para los dos lados de la puerta.
    if (!EnPunto(playerid, COMISARIA_PUERTA_DIST,
        (COMISARIA_PI_CX + COMISARIA_PD_CX) / 2.0,
        (COMISARIA_PI_CY + COMISARIA_PD_CY) / 2.0, COMISARIA_PUERTA_Z))
    {
        Error(playerid, "Acercate al control de acceso.");
        return 1;
    }

    new suyo[MAX_PLAYER_NAME + 1], linea[144];
    NombreRP(playerid, suyo, sizeof suyo);

    if (!g_PuertaComisariaAbierta)
    {
        Policia_MoverPuertas(true);

        format(linea, sizeof linea, "* %s pasa su placa por el lector y las puertas del control se abren.", suyo);
        Policia_AvisarPuerta(linea);

        Formato("Puertas abiertas. Se cerraran solas en %d segundos.", COMISARIA_PUERTA_CIERRE);
        Info(playerid, g_Buffer);
    }
    else
    {
        Policia_MoverPuertas(false);

        format(linea, sizeof linea, "* %s cierra las puertas del control de acceso.", suyo);
        Policia_AvisarPuerta(linea);
    }
    return 1;
}

// Cada segundo, una sola vez para todo el servidor (no por jugador): la
// puerta es una y su cuenta atras tambien. Se llama desde TicSegundo en
// 99_callbacks.inc, al lado de Policia_AvisosSegundo.
public Policia_ComisariaSegundo()
{
    if (g_PuertaComisariaCierre <= 0) return 1;

    g_PuertaComisariaCierre--;
    if (g_PuertaComisariaCierre > 0) return 1;

    Policia_MoverPuertas(false);
    Policia_AvisarPuerta("* Las puertas del control de acceso se cierran solas.");
    return 1;
}

// ---------------------------------------------------------------------
//  LA ARMERIA
//  ---------------------------------------------------------------------
//  Equipo reglamentario, escalonado por rango. Es lo unico del escalafon
//  que se nota en la mano y no en un titulo: ascender a Oficial deja de
//  ser una linea en /miembros y pasa a ser la escopeta.
//
//  NO CURA. El chaleco es equipo y sale de aqui; la salud es sanidad y
//  sale de los Medicos (36_medicos.inc), que para eso estan y para eso
//  cobran. Una comisaria que cura gratis deja al trabajo de medico sin
//  la mitad de sus clientes.
//
//  Y no lo coge el Aspirante: la regla del modulo es que el rango 1
//  acompaña y aprende, y un arma reglamentaria es justo lo contrario de
//  acompañar. Policia_NoEsAgente ya lo dice con esas palabras.
// ---------------------------------------------------------------------
CMD:equipo(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    if (GetPlayerInterior(playerid) != COMISARIA_INTERIOR
        || GetPlayerVirtualWorld(playerid) != MUNDO_NORMAL
        || !EnPunto(playerid, ARMERIA_DIST, ARMERIA_X, ARMERIA_Y, ARMERIA_Z))
    {
        Error(playerid, "Tienes que estar en la armeria de la comisaria.");
        Info(playerid, "Esta pasando el control de acceso. Se abre con /puerta.");
        return 1;
    }

    if (g_ArmeriaEspera[playerid] > 0)
    {
        new espera[32];
        FormatearTiempo(g_ArmeriaEspera[playerid], espera, sizeof espera);
        Formato("El armero acaba de darte lo tuyo. Vuelve en %s.", espera);
        Error(playerid, g_Buffer);
        return 1;
    }

    new rango = Jugador[playerid][jFaccionRango];

    // Reglamentario de todo agente.
    SetPlayerArmour(playerid, 100.0);
    GivePlayerWeapon(playerid, WEAPON_NITESTICK, 1);
    GivePlayerWeapon(playerid, WEAPON_COLT45, ARMERIA_BALAS_PISTOLA);

    new extra[64];
    extra[0] = EOS;

    if (rango >= ARMERIA_RANGO_ESCOPETA)
    {
        GivePlayerWeapon(playerid, WEAPON_SHOTGSPA, ARMERIA_BALAS_ESCOPETA);
        format(extra, sizeof extra, ", escopeta");
    }

    if (rango >= ARMERIA_RANGO_SUBFUSIL)
    {
        GivePlayerWeapon(playerid, WEAPON_MP5, ARMERIA_BALAS_SUBFUSIL);
        format(extra, sizeof extra, "%s y subfusil", extra);
    }

    g_ArmeriaEspera[playerid] = ARMERIA_ESPERA;

    Formato("Equipo reglamentario: chaleco, defensa y pistola%s.", extra);
    Exito(playerid, g_Buffer);

    new suyo[MAX_PLAYER_NAME + 1], linea[144];
    NombreRP(playerid, suyo, sizeof suyo);
    format(linea, sizeof linea, "* %s firma el registro de la armeria y recoge su equipo.", suyo);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, linea);
    return 1;
}

// Deja la espera de la armeria a cero. Se llama al conectar y al
// desconectar: los huecos de MAX_PLAYERS se reutilizan, y el que entre
// en el sitio de otro no tiene por que heredar su cuenta atras.
stock Policia_ReiniciarArmeria(playerid)
{
    g_ArmeriaEspera[playerid] = 0;
    return 1;
}

// ---------------------------------------------------------------------
//  CARCEL DE ROL
// ---------------------------------------------------------------------

stock Policia_MeterEnCarcel(playerid, segundos)
{
    // Si estaba caido se resuelve primero: si no, la carcel se pelea
    // contra el estado de herido (congelado + invulnerable). Mismo
    // cuidado que tiene /carcel en 16_administracion.inc.
    if (Jugador[playerid][jHerido]) Medicos_ResolverAdmin(playerid);

    Jugador[playerid][jCarcelRolSegundos] = segundos;

    // Al entrar en la celda se pagan las cuentas: los puntos y la
    // condena pendiente se saldan. Los antecedentes se quedan.
    Jugador[playerid][jBuscado] = 0;
    Jugador[playerid][jCondenaSegundos] = 0;
    Policia_AplicarBusqueda(playerid);

    Policia_Soltar(playerid);   // ya no hacen falta las esposas

    SetPlayerPos(playerid, CARCEL_X, CARCEL_Y, CARCEL_Z);
    SetPlayerInterior(playerid, CARCEL_INTERIOR);
    SetPlayerVirtualWorld(playerid, 1000 + playerid);
    ResetPlayerWeapons(playerid);
    SetCameraBehindPlayer(playerid);

    BD_GuardarCuenta(playerid);
    return 1;
}

stock Policia_SacarDeCarcel(playerid)
{
    Jugador[playerid][jCarcelRolSegundos] = 0;

    SetPlayerPos(playerid, CARCEL_X + 4.0, CARCEL_Y, CARCEL_Z);
    SetPlayerInterior(playerid, CARCEL_INTERIOR);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
    SetCameraBehindPlayer(playerid);

    Exito(playerid, "Has cumplido tu condena. Quedas en libertad.");
    Info(playerid, "Tus antecedentes siguen ahi: la policia puede consultarlos.");

    BD_GuardarCuenta(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  ESPOSAS
// ---------------------------------------------------------------------

stock Policia_Esposar(playerid, agente)
{
    Jugador[playerid][jEsposado] = true;
    Jugador[playerid][jEsposadoPor] = agente;
    SetPlayerSpecialAction(playerid, SPECIAL_ACTION_CUFFED);
    TogglePlayerControllable(playerid, false);
    return 1;
}

stock Policia_Soltar(playerid)
{
    if (!Jugador[playerid][jEsposado]) return 0;

    Jugador[playerid][jEsposado] = false;
    Jugador[playerid][jEsposadoPor] = INVALID_PLAYER_ID;
    SetPlayerSpecialAction(playerid, SPECIAL_ACTION_NONE);

    // Solo se descongela si no lo tiene congelado un admin por otra via.
    if (!Jugador[playerid][jCongelado]) TogglePlayerControllable(playerid, true);
    return 1;
}

// True (y avisa) si el jugador esta esposado. Los comandos que un
// detenido no deberia poder usar llaman a esto primero.
stock bool:Policia_EstaEsposado(playerid)
{
    if (!Jugador[playerid][jEsposado]) return false;
    Error(playerid, "Estas esposado. No puedes hacer eso.");
    return true;
}

// ---------------------------------------------------------------------
//  APLICAR UN DELITO
//
//  El corazon del modulo: suma puntos, suma condena y deja constancia
//  en los antecedentes. Lo llaman /delito, la negativa a pagar una
//  multa, y en el futuro los enganches automaticos con los sistemas
//  ilegales (cultivo, NFS, dropeo, armas).
// ---------------------------------------------------------------------
stock Policia_AplicarDelito(objetivo, indice, const agente[])
{
    if (indice < 0 || indice >= sizeof g_CodigoPenal) return 0;

    Jugador[objetivo][jBuscado] += g_CodigoPenal[indice][dePuntos];
    Jugador[objetivo][jCondenaSegundos] += g_CodigoPenal[indice][deMinutos] * 60;
    Policia_AplicarBusqueda(objetivo);

    new cuenta[MAX_PLAYER_NAME + 1];
    GetPlayerName(objetivo, cuenta, sizeof cuenta);

    new consulta[512];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO antecedentes (cuenta_id, nombre, codigo, delito, puntos, agente) VALUES (%d, '%e', %d, '%e', %d, '%e')",
        Jugador[objetivo][jID], cuenta, g_CodigoPenal[indice][deCodigo],
        g_CodigoPenal[indice][deNombre], g_CodigoPenal[indice][dePuntos], agente);
    mysql_pquery(g_BD, consulta);

    // Los delitos de TRAFICO (codigos 1xx) restan puntos del carnet,
    // como en España. Se quitan tantos puntos como puntos de busqueda
    // valga el delito: circular sin papeles 1, temeraria 2...
    if (g_CodigoPenal[indice][deCodigo] >= 100 && g_CodigoPenal[indice][deCodigo] < 200)
        Policia_QuitarPuntosCarnet(objetivo, g_CodigoPenal[indice][dePuntos], agente);

    BD_GuardarCuenta(objetivo);
    return 1;
}

// ---------------------------------------------------------------------
//  AVISOS A LA CENTRAL
//
//  Los sistemas ilegales que ya existen llaman a Policia_CrearAviso()
//  y la policia ve el trabajo en /avisos. Es la MISMA forma que usan
//  los Medicos en 36_medicos.inc (/llamadas -> /asistir -> /tratar),
//  con otros verbos: /avisos -> /asumir -> /resolver. Se copia a
//  proposito: el patron ya esta probado y los jugadores ya lo conocen
//  de otro sistema del servidor.
//
//  El estado vive en memoria y no se persiste, igual que las llamadas
//  de Medicos: un aviso de hace tres reinicios no le importa a nadie.
// ---------------------------------------------------------------------
enum E_AVISO_POL
{
    bool:avActivo,
    avTipo,
    Float:avX, Float:avY, Float:avZ,
    avInterior, avMundo,
    avHora,                                 // gettime() del suceso
    avAgente,                               // INVALID_PLAYER_ID si nadie lo asume
    avNombre[MAX_PLAYER_NAME + 1],          // sospechoso al generarse el aviso
    avCasos                                 // cuantos sucesos se han agrupado aqui
};
new g_Avisos[MAX_AVISOS_POLICIA][E_AVISO_POL];

stock Policia_NombreTipo(tipo, destino[], tam)
{
    switch (tipo)
    {
        case AVISO_CULTIVO:    format(destino, tam, "Cultivo de marihuana");
        case AVISO_ROBO_COCHE: format(destino, tam, "Robo de vehiculo");
        case AVISO_NFS:        format(destino, tam, "Vehiculo forzado");
        case AVISO_ARMAS:      format(destino, tam, "Trafico de armas");
        case AVISO_TIROTEO:    format(destino, tam, "Persona herida");
        case AVISO_LLAMADA:    format(destino, tam, "Llamada al 112");
        case AVISO_ATRACO:     format(destino, tam, "Atraco a un local");
        case AVISO_MALETERO:   format(destino, tam, "Maletero forzado");
        case AVISO_PUENTE_LIBRE: format(destino, tam, "Vehiculo libre puenteado");
        case AVISO_CERRADURA:  format(destino, tam, "Cerradura de vehiculo forzada");
        default:               format(destino, tam, "Suceso");
    }
    return 1;
}

// Delito del codigo penal que corresponde a cada tipo de aviso, para
// que el agente sepa de que acusar cuando llegue.
stock Policia_CodigoDeTipo(tipo)
{
    switch (tipo)
    {
        case AVISO_CULTIVO:    return 202;
        case AVISO_ROBO_COCHE: return 401;
        case AVISO_NFS:        return 402;
        case AVISO_ARMAS:      return 302;
        case AVISO_TIROTEO:    return 501;
        case AVISO_LLAMADA:    return 0;    // lo dira el que llamo
        case AVISO_ATRACO:     return 503;
        case AVISO_MALETERO:   return 403;
        case AVISO_PUENTE_LIBRE: return 404;
        case AVISO_CERRADURA:  return 405;
    }
    return 0;
}

// Cuantos policias hay conectados con rango para actuar.
stock Policia_AgentesConectados()
{
    new n = 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        if (Policia_EsAgente(i)) n++;
    }
    return n;
}

// Probabilidad de deteccion de cada tipo, en porcentaje.
stock Policia_ProbabilidadTipo(tipo)
{
    switch (tipo)
    {
        case AVISO_CULTIVO:    return PROB_AVISO_CULTIVO;
        case AVISO_ROBO_COCHE: return PROB_AVISO_ROBO_COCHE;
        case AVISO_NFS:        return PROB_AVISO_NFS;
        case AVISO_ARMAS:      return PROB_AVISO_ARMAS;
        case AVISO_TIROTEO:    return PROB_AVISO_TIROTEO;
        case AVISO_LLAMADA:    return 100;  // una denuncia siempre llega
        case AVISO_ATRACO:     return PROB_AVISO_ATRACO;
        case AVISO_MALETERO:   return PROB_AVISO_MALETERO;
        case AVISO_PUENTE_LIBRE: return PROB_AVISO_PUENTE_LIBRE;
        case AVISO_CERRADURA:  return PROB_AVISO_CERRADURA;
    }
    return 0;
}

// Aviso activo del mismo tipo a menos de POLICIA_AVISO_AGRUPA metros,
// o -1 si no hay ninguno. Es el freno contra el spam: diez plantas en
// el mismo descampado son UN aviso con diez casos, no diez avisos.
stock Policia_AvisoCercano(tipo, Float:x, Float:y, Float:z, interior, mundo)
{
    for (new i = 0; i < MAX_AVISOS_POLICIA; i++)
    {
        if (!g_Avisos[i][avActivo]) continue;
        if (g_Avisos[i][avTipo] != tipo) continue;
        if (g_Avisos[i][avInterior] != interior || g_Avisos[i][avMundo] != mundo) continue;

        new Float:dx = g_Avisos[i][avX] - x;
        new Float:dy = g_Avisos[i][avY] - y;
        new Float:dz = g_Avisos[i][avZ] - z;
        if (floatsqroot(dx * dx + dy * dy + dz * dz) <= POLICIA_AVISO_AGRUPA) return i;
    }
    return -1;
}

/*
 *  Punto de entrada para el resto del gamemode.
 *
 *  Lo llaman 28_marihuana, 22_dropeo, 26_nfs, 25_vendedorarmas y
 *  36_medicos cuando pasa algo que la policia deberia saber. Aqui se
 *  decide si el suceso se detecta y si merece un aviso nuevo.
 *
 *  Devuelve 1 si se ha creado o agrupado un aviso, 0 si no.
 */
public Policia_CrearAviso(tipo, sospechoso, Float:x, Float:y, Float:z, interior, mundo)
{
    // Sin agentes conectados no se genera nada: la cola se llenaria de
    // trabajo que nadie va a mirar y llegaria caducado.
    if (Policia_AgentesConectados() == 0) return 0;

    // Un policia cometiendo el delito no se avisa a si mismo.
    if (IsPlayerConnected(sospechoso) && Jugador[sospechoso][jFaccion] == POLICIA_FACCION) return 0;

    // 1er freno: no todo se detecta.
    if (random(100) >= Policia_ProbabilidadTipo(tipo)) return 0;

    new nombre[MAX_PLAYER_NAME + 1];
    if (IsPlayerConnected(sospechoso)) NombreRP(sospechoso, nombre, sizeof nombre);
    else                               format(nombre, sizeof nombre, "Desconocido");

    // 2o freno: si ya hay uno igual al lado, se le suma un caso.
    new cercano = Policia_AvisoCercano(tipo, x, y, z, interior, mundo);
    if (cercano != -1)
    {
        g_Avisos[cercano][avCasos]++;
        g_Avisos[cercano][avHora] = gettime();   // sigue vivo
        return 1;
    }

    // Hueco libre. Si no queda ninguno, se pisa el mas viejo sin asumir.
    new hueco = -1;
    for (new i = 0; i < MAX_AVISOS_POLICIA; i++)
    {
        if (!g_Avisos[i][avActivo]) { hueco = i; break; }
    }
    if (hueco == -1)
    {
        new masViejo = -1, horaVieja = 0;
        for (new i = 0; i < MAX_AVISOS_POLICIA; i++)
        {
            if (g_Avisos[i][avAgente] != INVALID_PLAYER_ID) continue;   // ese lo lleva alguien
            if (masViejo == -1 || g_Avisos[i][avHora] < horaVieja)
            {
                masViejo = i;
                horaVieja = g_Avisos[i][avHora];
            }
        }
        if (masViejo == -1) return 0;   // todos asumidos: no se pisa ninguno
        hueco = masViejo;
    }

    g_Avisos[hueco][avActivo]   = true;
    g_Avisos[hueco][avTipo]     = tipo;
    g_Avisos[hueco][avX]        = x;
    g_Avisos[hueco][avY]        = y;
    g_Avisos[hueco][avZ]        = z;
    g_Avisos[hueco][avInterior] = interior;
    g_Avisos[hueco][avMundo]    = mundo;
    g_Avisos[hueco][avHora]     = gettime();
    g_Avisos[hueco][avAgente]   = INVALID_PLAYER_ID;
    g_Avisos[hueco][avCasos]    = 1;
    format(g_Avisos[hueco][avNombre], MAX_PLAYER_NAME + 1, "%s", nombre);

    new tipoTexto[32];
    Policia_NombreTipo(tipo, tipoTexto, sizeof tipoTexto);

    new linea[144];
    format(linea, sizeof linea, "Aviso #%d - %s. Escribe /avisos para verlo.", hueco + 1, tipoTexto);
    Policia_AvisarCentral(linea);
    return 1;
}

stock Policia_CerrarAviso(indice)
{
    if (indice < 0 || indice >= MAX_AVISOS_POLICIA) return 0;

    new agente = g_Avisos[indice][avAgente];
    if (agente != INVALID_PLAYER_ID && IsPlayerConnected(agente))
        DisablePlayerCheckpoint(agente);

    g_Avisos[indice][avActivo] = false;
    g_Avisos[indice][avAgente] = INVALID_PLAYER_ID;
    g_Avisos[indice][avCasos]  = 0;
    return 1;
}

// Cada segundo, una sola vez para todo el servidor: caduca los avisos
// viejos. No hace falta mas resolucion que esta.
public Policia_AvisosSegundo()
{
    new ahora = gettime();

    for (new i = 0; i < MAX_AVISOS_POLICIA; i++)
    {
        if (!g_Avisos[i][avActivo]) continue;
        if (ahora - g_Avisos[i][avHora] < POLICIA_AVISO_CADUCA * 60) continue;

        new agente = g_Avisos[i][avAgente];
        if (agente != INVALID_PLAYER_ID && IsPlayerConnected(agente))
            Aviso(agente, "El aviso que llevabas ha caducado.");

        Policia_CerrarAviso(i);
    }
    return 1;
}

CMD:avisos(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new lista[1536], linea[224], activos = 0;
    format(lista, sizeof lista, "{B4B5B7}#\tSuceso\tSospechoso\tDistancia\tEstado\n");

    for (new i = 0; i < MAX_AVISOS_POLICIA; i++)
    {
        if (!g_Avisos[i][avActivo]) continue;

        new tipoTexto[32], estado[48], casos[16];
        Policia_NombreTipo(g_Avisos[i][avTipo], tipoTexto, sizeof tipoTexto);

        if (g_Avisos[i][avAgente] == INVALID_PLAYER_ID)   format(estado, sizeof estado, "{53C168}Libre");
        else if (g_Avisos[i][avAgente] == playerid)       format(estado, sizeof estado, "{FFFFFF}Lo llevas tu");
        else                                              format(estado, sizeof estado, "{E05A5A}Asumido");

        if (g_Avisos[i][avCasos] > 1) format(casos, sizeof casos, " (x%d)", g_Avisos[i][avCasos]);
        else                          format(casos, sizeof casos, "");

        // La distancia dice mucho mas que un nombre de barrio, y no
        // depende de ninguna tabla de zonas que haya que mantener.
        new metros = floatround(GetPlayerDistanceFromPoint(playerid, g_Avisos[i][avX], g_Avisos[i][avY], g_Avisos[i][avZ]));

        format(linea, sizeof linea, "{F2C94C}%d\t{FFFFFF}%s%s\t%s\t{B4B5B7}%d m\t%s\n",
            i + 1, tipoTexto, casos, g_Avisos[i][avNombre], metros, estado);
        strcat(lista, linea, sizeof lista);
        activos++;
    }

    if (activos == 0)
    {
        Info(playerid, "No hay ningun aviso pendiente ahora mismo.");
        return 1;
    }

    ShowPlayerDialog(playerid, DIALOGO_POLICIA_AVISOS, DIALOG_STYLE_TABLIST_HEADERS,
        "Avisos de la central", lista, "Cerrar", "");

    Linea(playerid, COLOR_GRIS, "Asume uno con /asumir [numero] y cierralo con /resolver [numero].");
    return 1;
}

CMD:asumir(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new numero;
    if (sscanf(params, "d", numero))
    {
        Uso(playerid, "/asumir [numero de aviso]");
        Linea(playerid, COLOR_GRIS, "Los numeros salen en /avisos.");
        return 1;
    }

    new indice = numero - 1;
    if (indice < 0 || indice >= MAX_AVISOS_POLICIA || !g_Avisos[indice][avActivo])
    {
        Error(playerid, "Ese aviso no existe o ya se ha cerrado.");
        return 1;
    }
    if (g_Avisos[indice][avAgente] != INVALID_PLAYER_ID)
    {
        if (g_Avisos[indice][avAgente] == playerid) Error(playerid, "Ya llevas ese aviso.");
        else                                        Error(playerid, "Ese aviso ya lo lleva otro agente.");
        return 1;
    }

    // Un agente, un aviso: si llevaba otro, lo suelta.
    for (new i = 0; i < MAX_AVISOS_POLICIA; i++)
    {
        if (g_Avisos[i][avActivo] && g_Avisos[i][avAgente] == playerid)
            g_Avisos[i][avAgente] = INVALID_PLAYER_ID;
    }

    g_Avisos[indice][avAgente] = playerid;
    SetPlayerCheckpoint(playerid, g_Avisos[indice][avX], g_Avisos[indice][avY], g_Avisos[indice][avZ], 8.0);

    new tipoTexto[32];
    Policia_NombreTipo(g_Avisos[indice][avTipo], tipoTexto, sizeof tipoTexto);

    Formato("Aviso #%d asumido: %s. Sigue el checkpoint.", numero, tipoTexto);
    Exito(playerid, g_Buffer);

    new codigo = Policia_CodigoDeTipo(g_Avisos[indice][avTipo]);
    if (codigo > 0)
    {
        Formato("Si le pillas, el delito que toca es el %d.", codigo);
        Info(playerid, g_Buffer);
    }

    new agente[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, agente, sizeof agente);
    Formato("%s asume el aviso #%d (%s).", agente, numero, tipoTexto);
    Policia_AvisarCentral(g_Buffer);
    return 1;
}

CMD:resolver(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new numero;
    if (sscanf(params, "d", numero))
    {
        Uso(playerid, "/resolver [numero de aviso]");
        return 1;
    }

    new indice = numero - 1;
    if (indice < 0 || indice >= MAX_AVISOS_POLICIA || !g_Avisos[indice][avActivo])
    {
        Error(playerid, "Ese aviso no existe o ya se ha cerrado.");
        return 1;
    }
    if (g_Avisos[indice][avAgente] != playerid)
    {
        Error(playerid, "Ese aviso no lo llevas tu. Asumelo antes con /asumir.");
        return 1;
    }

    new tipoTexto[32];
    Policia_NombreTipo(g_Avisos[indice][avTipo], tipoTexto, sizeof tipoTexto);

    Policia_CerrarAviso(indice);

    Formato("Aviso #%d cerrado (%s).", numero, tipoTexto);
    Exito(playerid, g_Buffer);

    new agente[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, agente, sizeof agente);
    Formato("%s cierra el aviso #%d.", agente, numero);
    Policia_AvisarCentral(g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  TICS
// ---------------------------------------------------------------------

// Cada segundo, por jugador. Mismo patron que Admin_Segundo.
public Policia_Segundo(playerid)
{
    // La espera de la armeria corre aqui: es por jugador, como la condena.
    if (g_ArmeriaEspera[playerid] > 0) g_ArmeriaEspera[playerid]--;

    if (Jugador[playerid][jCarcelRolSegundos] <= 0) return 1;

    Jugador[playerid][jCarcelRolSegundos]--;

    if (Jugador[playerid][jCarcelRolSegundos] == 0)
    {
        Policia_SacarDeCarcel(playerid);
    }
    else if (Jugador[playerid][jCarcelRolSegundos] % 30 == 0)
    {
        new tiempo[32];
        FormatearTiempo(Jugador[playerid][jCarcelRolSegundos], tiempo, sizeof tiempo);
        Formato("Te quedan %s de condena.", tiempo);
        Aviso(playerid, g_Buffer);
    }
    return 1;
}

// Cada minuto, por jugador: caducidad de los puntos de busqueda.
public Policia_Minuto(playerid)
{
    if (Jugador[playerid][jBuscado] <= 0) return 1;

    // Dentro de la celda el tiempo de busqueda no corre: ya se estan
    // pagando las cuentas de otra forma.
    if (Jugador[playerid][jCarcelRolSegundos] > 0) return 1;

    Jugador[playerid][jMinutosCaduca]++;
    if (Jugador[playerid][jMinutosCaduca] < POLICIA_MINUTOS_CADUCA) return 1;

    Jugador[playerid][jMinutosCaduca] = 0;
    Jugador[playerid][jBuscado]--;

    if (Jugador[playerid][jBuscado] <= 0)
    {
        // Si dejas de estar buscado, la condena pendiente caduca con
        // los puntos. Los antecedentes NO: eso no se borra nunca.
        Jugador[playerid][jBuscado] = 0;
        Jugador[playerid][jCondenaSegundos] = 0;
        Info(playerid, "Ya no se te busca. Tus antecedentes siguen registrados.");
    }

    Policia_AplicarBusqueda(playerid);
    return 1;
}

public Policia_OnSpawn(playerid)
{
    // Las estrellas no sobreviven al respawn: se vuelven a poner.
    Policia_AplicarBusqueda(playerid);

    if (Jugador[playerid][jCarcelRolSegundos] > 0)
    {
        SetPlayerPos(playerid, CARCEL_X, CARCEL_Y, CARCEL_Z);
        SetPlayerInterior(playerid, CARCEL_INTERIOR);
        SetPlayerVirtualWorld(playerid, 1000 + playerid);
        Aviso(playerid, "Sigues cumpliendo condena.");
    }
    return 1;
}

public Policia_OnDisconnect(playerid)
{
    // Si se va esposado, que no arrastre el estado a la siguiente
    // sesion -- ni el, ni una multa a medias.
    Jugador[playerid][jEsposado] = false;
    Jugador[playerid][jEsposadoPor] = INVALID_PLAYER_ID;
    Jugador[playerid][jMultaImporte] = 0;
    Jugador[playerid][jMultaAgente] = INVALID_PLAYER_ID;
    Jugador[playerid][jMinutosCaduca] = 0;

    Policia_ReiniciarArmeria(playerid);

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jMultaAgente] == playerid) Jugador[i][jMultaAgente] = INVALID_PLAYER_ID;
        if (Jugador[i][jEsposadoPor] == playerid) Jugador[i][jEsposadoPor] = INVALID_PLAYER_ID;
    }

    // Si llevaba un aviso, vuelve a la cola para que lo coja otro. No se
    // cierra: el suceso sigue ahi aunque el agente se haya ido.
    for (new i = 0; i < MAX_AVISOS_POLICIA; i++)
    {
        if (g_Avisos[i][avActivo] && g_Avisos[i][avAgente] == playerid)
            g_Avisos[i][avAgente] = INVALID_PLAYER_ID;
    }
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDOS PARA TODO EL MUNDO
// ---------------------------------------------------------------------

CMD:codigopenal(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new lista[2048], fila[160];
    format(lista, sizeof lista, "{B4B5B7}Codigo\tDelito\tPuntos\tMulta\tCarcel\n");

    for (new i = 0; i < sizeof g_CodigoPenal; i++)
    {
        if (g_CodigoPenal[i][deMulta] > 0 && g_CodigoPenal[i][deMinutos] > 0)
            format(fila, sizeof fila, "{F2C94C}%d\t{FFFFFF}%s\t%d\t$%d\t%d min\n",
                g_CodigoPenal[i][deCodigo], g_CodigoPenal[i][deNombre],
                g_CodigoPenal[i][dePuntos], g_CodigoPenal[i][deMulta], g_CodigoPenal[i][deMinutos]);
        else if (g_CodigoPenal[i][deMulta] > 0)
            format(fila, sizeof fila, "{F2C94C}%d\t{FFFFFF}%s\t%d\t$%d\t{B4B5B7}-\n",
                g_CodigoPenal[i][deCodigo], g_CodigoPenal[i][deNombre],
                g_CodigoPenal[i][dePuntos], g_CodigoPenal[i][deMulta]);
        else
            format(fila, sizeof fila, "{F2C94C}%d\t{FFFFFF}%s\t%d\t{B4B5B7}-\t{FFFFFF}%d min\n",
                g_CodigoPenal[i][deCodigo], g_CodigoPenal[i][deNombre],
                g_CodigoPenal[i][dePuntos], g_CodigoPenal[i][deMinutos]);

        strcat(lista, fila);
    }

    ShowPlayerDialog(playerid, DIALOGO_POLICIA_CODIGO, DIALOG_STYLE_TABLIST_HEADERS,
        "Codigo penal de Los Santos", lista, "Cerrar", "");
    return 1;
}

CMD:mibusqueda(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new estrellas = Policia_Estrellas(Jugador[playerid][jBuscado]);

    if (estrellas == 0 && Jugador[playerid][jCarcelRolSegundos] <= 0)
    {
        Info(playerid, "No se te busca por nada. Estas limpio.");
        return 1;
    }

    Linea(playerid, COLOR_MARCA, "--- Tu situacion con la justicia ---");

    if (estrellas > 0)
    {
        Formato("Nivel de busqueda: %d estrella(s)  ~  %d punto(s).", estrellas, Jugador[playerid][jBuscado]);
        Linea(playerid, COLOR_ROJO, g_Buffer);

        if (Jugador[playerid][jCondenaSegundos] > 0)
        {
            new tiempo[32];
            FormatearTiempo(Jugador[playerid][jCondenaSegundos], tiempo, sizeof tiempo);
            Formato("Si te detienen ahora: %s de carcel.", tiempo);
            Linea(playerid, COLOR_GRIS, g_Buffer);
        }

        new quedan = POLICIA_MINUTOS_CADUCA - Jugador[playerid][jMinutosCaduca];
        Formato("Pierdes un punto dentro de %d minuto(s) si no reincides.", quedan);
        Linea(playerid, COLOR_GRIS, g_Buffer);
    }

    if (Jugador[playerid][jCarcelRolSegundos] > 0)
    {
        new tiempo[32];
        FormatearTiempo(Jugador[playerid][jCarcelRolSegundos], tiempo, sizeof tiempo);
        Formato("Estas cumpliendo condena: te quedan %s.", tiempo);
        Linea(playerid, COLOR_AVISO, g_Buffer);
    }
    return 1;
}

CMD:aceptarmulta(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (Jugador[playerid][jMultaImporte] <= 0)
    {
        Error(playerid, "No tienes ninguna multa pendiente.");
        return 1;
    }

    if (gettime() > Jugador[playerid][jMultaExpira])
    {
        Jugador[playerid][jMultaImporte] = 0;
        Error(playerid, "Esa multa ha caducado. El agente tendra que ponerla otra vez.");
        return 1;
    }

    new importe = Jugador[playerid][jMultaImporte];

    // Se cobra del banco, y si no llega, del efectivo. Una multa que no
    // se puede pagar no puede quedarse colgada: si no hay dinero, se
    // convierte en delito, que es lo que pasaria de verdad.
    if (Jugador[playerid][jBanco] >= importe)
    {
        Jugador[playerid][jBanco] -= importe;
    }
    else if (Jugador[playerid][jDinero] + Jugador[playerid][jBanco] >= importe)
    {
        new resto = importe - Jugador[playerid][jBanco];
        Jugador[playerid][jBanco] = 0;
        Jugador[playerid][jDinero] -= resto;
        ActualizarDineroHUD(playerid);
    }
    else
    {
        Error(playerid, "No tienes suficiente dinero para pagar la multa.");
        Info(playerid, "Consigue el dinero o el agente te acusara de negarte a pagar.");
        return 1;
    }

    Negocio_IngresoMulta(importe);

    new agente = Jugador[playerid][jMultaAgente];
    Jugador[playerid][jMultaImporte] = 0;

    Formato("Has pagado una multa de $%d. Motivo: %s.", importe, Jugador[playerid][jMultaMotivo]);
    Exito(playerid, g_Buffer);

    if (agente != INVALID_PLAYER_ID && IsPlayerConnected(agente))
    {
        new nombre[MAX_PLAYER_NAME + 1];
        NombreRP(playerid, nombre, sizeof nombre);
        Formato("%s ha pagado la multa de $%d.", nombre, importe);
        Exito(agente, g_Buffer);
    }

    BD_GuardarCuenta(playerid);
    return 1;
}

CMD:rechazarmulta(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (Jugador[playerid][jMultaImporte] <= 0)
    {
        Error(playerid, "No tienes ninguna multa pendiente.");
        return 1;
    }

    new agente = Jugador[playerid][jMultaAgente];
    Jugador[playerid][jMultaImporte] = 0;

    new agenteNombre[MAX_PLAYER_NAME + 1];
    if (agente != INVALID_PLAYER_ID && IsPlayerConnected(agente))
        NombreRP(agente, agenteNombre, sizeof agenteNombre);
    else
        format(agenteNombre, sizeof agenteNombre, "un agente");

    // Negarse es delito 604. Se aplica solo, sin que el agente tenga que
    // escribir nada mas.
    new indice = Policia_BuscarDelito(604);
    Policia_AplicarDelito(playerid, indice, agenteNombre);

    Error(playerid, "Te has negado a pagar. Eso es delito: se te busca por ello.");

    if (agente != INVALID_PLAYER_ID && IsPlayerConnected(agente))
    {
        new nombre[MAX_PLAYER_NAME + 1];
        NombreRP(playerid, nombre, sizeof nombre);
        Formato("%s se ha negado a pagar la multa. Delito 604 aplicado.", nombre);
        Aviso(agente, g_Buffer);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDOS DE LA POLICIA
// ---------------------------------------------------------------------

CMD:delito(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new objetivoTexto[24], codigo;
    if (sscanf(params, "s[24]d", objetivoTexto, codigo))
    {
        Uso(playerid, "/delito [id o nombre] [codigo]");
        Linea(playerid, COLOR_GRIS, "Mira la lista con /codigopenal.");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (objetivo == playerid)
    {
        Error(playerid, "No puedes acusarte a ti mismo.");
        return 1;
    }
    if (Jugador[objetivo][jFaccion] == POLICIA_FACCION)
    {
        Error(playerid, "No puedes acusar a un compañero. Eso lo lleva el mando.");
        return 1;
    }
    if (!EstaCerca(playerid, objetivo, POLICIA_DISTANCIA))
    {
        Error(playerid, "Tienes que estar al lado para acusarle.");
        return 1;
    }

    new indice = Policia_BuscarDelito(codigo);
    if (indice == -1)
    {
        Error(playerid, "Ese codigo no existe en el codigo penal.");
        Linea(playerid, COLOR_GRIS, "Mira la lista con /codigopenal.");
        return 1;
    }

    new agente[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, agente, sizeof agente);
    NombreRP(objetivo, suyo, sizeof suyo);

    Policia_AplicarDelito(objetivo, indice, agente);

    Formato("%s queda acusado de: %s (codigo %d).", suyo, g_CodigoPenal[indice][deNombre], codigo);
    Exito(playerid, g_Buffer);

    Formato("El agente %s te acusa de: %s.", agente, g_CodigoPenal[indice][deNombre]);
    Error(objetivo, g_Buffer);

    new tiempo[32];
    FormatearTiempo(Jugador[objetivo][jCondenaSegundos], tiempo, sizeof tiempo);
    Formato("Nivel de busqueda: %d estrella(s). Si te detienen: %s de carcel.", Policia_Estrellas(Jugador[objetivo][jBuscado]), tiempo);
    Aviso(objetivo, g_Buffer);

    Formato("%s acusa a %s de %s.", agente, suyo, g_CodigoPenal[indice][deNombre]);
    Policia_AvisarCentral(g_Buffer);
    return 1;
}

CMD:multar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new objetivoTexto[24], codigo;
    if (sscanf(params, "s[24]d", objetivoTexto, codigo))
    {
        Uso(playerid, "/multar [id o nombre] [codigo]");
        Linea(playerid, COLOR_GRIS, "El importe lo fija el codigo penal, no tu. Mira /codigopenal.");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (objetivo == playerid)
    {
        Error(playerid, "No puedes multarte a ti mismo.");
        return 1;
    }
    if (!EstaCerca(playerid, objetivo, POLICIA_DISTANCIA))
    {
        Error(playerid, "Tienes que estar al lado para multarle.");
        return 1;
    }
    if (Jugador[objetivo][jMultaImporte] > 0 && gettime() <= Jugador[objetivo][jMultaExpira])
    {
        Error(playerid, "Ya tiene una multa pendiente de contestar.");
        return 1;
    }

    new indice = Policia_BuscarDelito(codigo);
    if (indice == -1)
    {
        Error(playerid, "Ese codigo no existe en el codigo penal.");
        return 1;
    }

    new importe = g_CodigoPenal[indice][deMulta];
    if (importe <= 0)
    {
        Error(playerid, "Ese delito no lleva multa: se resuelve deteniendo.");
        return 1;
    }

    // Tope para cuentas nuevas. Un novato no puede llevarse una multa
    // que le arruine el primer dia por no saber jugar todavia.
    if (Jugador[objetivo][jMinutosTotales] < POLICIA_NOVATO_MINUTOS && importe > POLICIA_NOVATO_MAX)
    {
        importe = POLICIA_NOVATO_MAX;
        Aviso(playerid, "Es una cuenta nueva: la multa se ha rebajado al tope de novato.");
    }

    Jugador[objetivo][jMultaImporte] = importe;
    Jugador[objetivo][jMultaAgente] = playerid;
    Jugador[objetivo][jMultaExpira] = gettime() + POLICIA_MULTA_ESPERA;
    format(Jugador[objetivo][jMultaMotivo], 64, "%s", g_CodigoPenal[indice][deNombre]);

    new agente[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, agente, sizeof agente);
    NombreRP(objetivo, suyo, sizeof suyo);

    Formato("Has multado a %s con $%d por: %s.", suyo, importe, g_CodigoPenal[indice][deNombre]);
    Exito(playerid, g_Buffer);
    Info(playerid, "Tiene que aceptarla. Si se niega, sera delito.");

    Formato("El agente %s te multa con $%d por: %s.", agente, importe, g_CodigoPenal[indice][deNombre]);
    Aviso(objetivo, g_Buffer);
    Linea(objetivo, COLOR_MARCA, "Escribe /aceptarmulta para pagarla, o /rechazarmulta si te niegas.");
    Linea(objetivo, COLOR_GRIS, "Negarte es delito (codigo 604). Tienes 90 segundos.");
    return 1;
}

CMD:esposar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/esposar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (objetivo == playerid)
    {
        Error(playerid, "No puedes esposarte a ti mismo.");
        return 1;
    }
    if (!EstaCerca(playerid, objetivo, POLICIA_DISTANCIA))
    {
        Error(playerid, "Tienes que estar al lado para esposarle.");
        return 1;
    }
    if (Jugador[objetivo][jEsposado])
    {
        Error(playerid, "Ya esta esposado.");
        return 1;
    }

    Policia_Esposar(objetivo, playerid);

    new agente[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, agente, sizeof agente);
    NombreRP(objetivo, suyo, sizeof suyo);

    Formato("* %s esposa a %s.", agente, suyo);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, g_Buffer);

    Aviso(objetivo, "Estas esposado. No puedes moverte hasta que te suelten.");
    return 1;
}

CMD:quitaresposas(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/quitaresposas [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (!Jugador[objetivo][jEsposado])
    {
        Error(playerid, "Ese jugador no esta esposado.");
        return 1;
    }
    if (!EstaCerca(playerid, objetivo, POLICIA_DISTANCIA))
    {
        Error(playerid, "Tienes que estar al lado para soltarle.");
        return 1;
    }

    Policia_Soltar(objetivo);

    new agente[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, agente, sizeof agente);
    NombreRP(objetivo, suyo, sizeof suyo);

    Formato("* %s le quita las esposas a %s.", agente, suyo);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, g_Buffer);

    Exito(objetivo, "Te han quitado las esposas.");
    return 1;
}

CMD:detener(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/detener [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (objetivo == playerid)
    {
        Error(playerid, "No puedes detenerte a ti mismo.");
        return 1;
    }
    if (!EstaCerca(playerid, objetivo, POLICIA_DISTANCIA))
    {
        Error(playerid, "Tienes que estar al lado para detenerle.");
        return 1;
    }
    // Esposar primero es obligatorio a proposito: obliga a rolear la
    // detencion en dos pasos en vez de teletransportar a alguien a la
    // celda desde la acera.
    if (!Jugador[objetivo][jEsposado])
    {
        Error(playerid, "Tienes que esposarle antes de detenerle.");
        return 1;
    }
    if (Jugador[objetivo][jCondenaSegundos] <= 0)
    {
        Error(playerid, "No tiene ninguna condena pendiente. Acusale antes con /delito.");
        return 1;
    }

    new segundos = Jugador[objetivo][jCondenaSegundos];
    new tiempo[32];
    FormatearTiempo(segundos, tiempo, sizeof tiempo);

    new agente[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, agente, sizeof agente);
    NombreRP(objetivo, suyo, sizeof suyo);

    Policia_MeterEnCarcel(objetivo, segundos);

    Formato("Has detenido a %s. Condena: %s.", suyo, tiempo);
    Exito(playerid, g_Buffer);

    Formato("El agente %s te ha detenido. Cumples %s de condena.", agente, tiempo);
    Error(objetivo, g_Buffer);
    Info(objetivo, "Mira cuanto te queda con /mibusqueda.");

    Formato("%s ha detenido a %s (%s).", agente, suyo, tiempo);
    Policia_AvisarCentral(g_Buffer);
    return 1;
}

CMD:buscados(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new lista[1536], fila[160], encontrados = 0;
    format(lista, sizeof lista, "{B4B5B7}ID\tNombre\tEstrellas\tCondena\n");

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jBuscado] <= 0) continue;

        new nombre[MAX_PLAYER_NAME + 1], tiempo[32];
        NombreRP(i, nombre, sizeof nombre);
        FormatearTiempo(Jugador[i][jCondenaSegundos], tiempo, sizeof tiempo);

        format(fila, sizeof fila, "{FFFFFF}%d\t%s\t{E05A5A}%d\t{FFFFFF}%s\n",
            i, nombre, Policia_Estrellas(Jugador[i][jBuscado]), tiempo);
        strcat(lista, fila);
        encontrados++;
    }

    if (encontrados == 0)
    {
        Info(playerid, "Ahora mismo no se busca a nadie conectado.");
        return 1;
    }

    ShowPlayerDialog(playerid, DIALOGO_POLICIA_BUSCADOS, DIALOG_STYLE_TABLIST_HEADERS,
        "Buscados ahora mismo", lista, "Cerrar", "");
    return 1;
}

CMD:antecedentes(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/antecedentes [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    // El nombre se guarda en jTemporalTexto para poder titular el
    // dialogo cuando vuelva la consulta.
    NombreRP(objetivo, Jugador[playerid][jTemporalTexto], 64);

    new consulta[256];
    mysql_format(g_BD, consulta, sizeof consulta,
        "SELECT codigo, delito, agente, DATE_FORMAT(fecha, '%%d/%%m/%%Y') AS dia FROM antecedentes WHERE cuenta_id = %d ORDER BY id DESC LIMIT 30",
        Jugador[objetivo][jID]);
    mysql_pquery(g_BD, consulta, "AlCargarAntecedentes", "d", playerid);
    return 1;
}

public AlCargarAntecedentes(playerid)
{
    if (!IsPlayerConnected(playerid) || !Jugador[playerid][jLogueado]) return 1;

    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        Formato("%s no tiene antecedentes.", Jugador[playerid][jTemporalTexto]);
        Info(playerid, g_Buffer);
        return 1;
    }

    new lista[2048], fila[192];
    format(lista, sizeof lista, "{B4B5B7}Fecha\tCodigo\tDelito\tAgente\n");

    for (new i = 0; i < filas; i++)
    {
        new codigo, delito[64], agente[32], dia[16];
        cache_get_value_name_int(i, "codigo", codigo);
        cache_get_value_name(i, "delito", delito, sizeof delito);
        cache_get_value_name(i, "agente", agente, sizeof agente);
        cache_get_value_name(i, "dia", dia, sizeof dia);

        format(fila, sizeof fila, "{FFFFFF}%s\t{F2C94C}%d\t{FFFFFF}%s\t{B4B5B7}%s\n", dia, codigo, delito, agente);
        strcat(lista, fila);
    }

    Formato("Antecedentes de %s", Jugador[playerid][jTemporalTexto]);
    ShowPlayerDialog(playerid, DIALOGO_POLICIA_ANTECEDENTES, DIALOG_STYLE_TABLIST_HEADERS,
        g_Buffer, lista, "Cerrar", "");
    return 1;
}

// ---------------------------------------------------------------------
//  MANDO: perdonar
// ---------------------------------------------------------------------
CMD:perdonar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    if (Jugador[playerid][jFaccionRango] < POLICIA_RANGO_MANDO && !Jugador[playerid][jFaccionLider])
    {
        Error(playerid, "Solo un Comisario o superior puede perdonar una busqueda.");
        return 1;
    }

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/perdonar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (Jugador[objetivo][jBuscado] <= 0 && Jugador[objetivo][jCarcelRolSegundos] <= 0)
    {
        Error(playerid, "Ese jugador no tiene nada pendiente.");
        return 1;
    }

    new estabaPreso = (Jugador[objetivo][jCarcelRolSegundos] > 0);

    Jugador[objetivo][jBuscado] = 0;
    Jugador[objetivo][jCondenaSegundos] = 0;
    Policia_AplicarBusqueda(objetivo);

    if (estabaPreso) Policia_SacarDeCarcel(objetivo);
    else             BD_GuardarCuenta(objetivo);

    new agente[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, agente, sizeof agente);
    NombreRP(objetivo, suyo, sizeof suyo);

    Formato("Has perdonado la busqueda de %s.", suyo);
    Exito(playerid, g_Buffer);

    Formato("El mando %s ha perdonado tu busqueda. Los antecedentes se quedan.", agente);
    Exito(objetivo, g_Buffer);

    Formato("%s ha perdonado la busqueda de %s.", agente, suyo);
    Policia_AvisarCentral(g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  EL CARNET DE CONDUCIR POR PUNTOS
//
//  Lo mas español que se le puede meter a esto, y de paso le da negocio
//  de verdad al dueño de la DGT: hasta ahora solo cobraba de
//  /comprarcoche.
//
//  12 puntos, los delitos de trafico (codigos 1xx) los van restando, y
//  a 0 puntos se retira el carnet. Conducir sin el es el delito 104.
// ---------------------------------------------------------------------

// Quita puntos del carnet. Lo llama Policia_AplicarDelito cuando el
// delito es de trafico. Si llega a 0, retira el carnet.
public Policia_QuitarPuntosCarnet(playerid, puntos, const agente[])
{
    if (!IsPlayerConnected(playerid)) return 0;
    if (!Jugador[playerid][jCarnet]) return 0;   // sin carnet no hay puntos que quitar
    if (puntos <= 0) return 0;

    Jugador[playerid][jCarnetPuntos] -= puntos;
    if (Jugador[playerid][jCarnetPuntos] < 0) Jugador[playerid][jCarnetPuntos] = 0;

    if (Jugador[playerid][jCarnetPuntos] == 0)
    {
        Jugador[playerid][jCarnet] = 0;
        Error(playerid, "Te has quedado sin puntos: la DGT te retira el carnet.");
        Info(playerid, "Puedes sacartelo otra vez en la DGT, pagando de nuevo.");

        new linea[144];
        format(linea, sizeof linea, "%s se queda sin puntos. Carnet retirado por %s.", Jugador[playerid][jNombre], agente);
        Policia_AvisarCentral(linea);
    }
    else
    {
        Formato("Te quitan %d punto(s) del carnet. Te quedan %d de %d.", puntos, Jugador[playerid][jCarnetPuntos], CARNET_PUNTOS_INICIALES);
        Aviso(playerid, g_Buffer);
    }
    return 1;
}

CMD:carnet(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (!Jugador[playerid][jCarnet])
    {
        Error(playerid, "No tienes carnet de conducir.");
        Formato("Sacatelo en la DGT por $%d. Escribe /sacarcarnet estando alli.", CARNET_PRECIO);
        Info(playerid, g_Buffer);
        return 1;
    }

    Linea(playerid, COLOR_MARCA, "--- Carnet de conducir ---");
    Formato("Titular: %s   ~   Telefono: %d", Jugador[playerid][jNombre], Jugador[playerid][jTelefono]);
    Linea(playerid, COLOR_BLANCO, g_Buffer);

    Formato("Puntos: %d de %d.", Jugador[playerid][jCarnetPuntos], CARNET_PUNTOS_INICIALES);
    Linea(playerid, (Jugador[playerid][jCarnetPuntos] <= 4) ? COLOR_ROJO : COLOR_VERDE, g_Buffer);

    if (Jugador[playerid][jCarnetPuntos] <= 4)
        Linea(playerid, COLOR_GRIS, "Cuidado: a 0 puntos la DGT te lo retira.");
    return 1;
}

CMD:sacarcarnet(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (Jugador[playerid][jCarnet])
    {
        Error(playerid, "Ya tienes carnet en vigor. Mira tus puntos con /carnet.");
        return 1;
    }

    // La DGT es la ranura 1 de 32_negocios.inc. Se usa su propia
    // coordenada en vez de repetirla aqui: si el negocio se mueve, esto
    // se mueve con el.
    new Float:dgtX, Float:dgtY, Float:dgtZ, dgtInt, dgtMundo;
    Negocio_Posicion(NEGOCIO_SLOT_DGT, dgtX, dgtY, dgtZ, dgtInt, dgtMundo);

    if (!EnPunto(playerid, CARNET_DISTANCIA, dgtX, dgtY, dgtZ)
        || GetPlayerInterior(playerid) != dgtInt
        || GetPlayerVirtualWorld(playerid) != dgtMundo)
    {
        Error(playerid, "Tienes que estar en el mostrador de la DGT.");
        return 1;
    }

    if (!TieneDinero(playerid, CARNET_PRECIO))
    {
        Formato("El carnet cuesta $%d y no lo llevas encima.", CARNET_PRECIO);
        Error(playerid, g_Buffer);
        return 1;
    }

    DarDinero(playerid, -CARNET_PRECIO);

    // Lo que paga el ciudadano se lo lleva el dueño de la DGT, como
    // cualquier otro tramite suyo.
    Negocio_IngresoComprarCoche(CARNET_PRECIO);

    Jugador[playerid][jCarnet] = 1;
    Jugador[playerid][jCarnetPuntos] = CARNET_PUNTOS_INICIALES;
    BD_GuardarCuenta(playerid);

    Formato("Carnet expedido. Tienes %d puntos.", CARNET_PUNTOS_INICIALES);
    Exito(playerid, g_Buffer);
    Info(playerid, "Los delitos de trafico te iran restando puntos. A 0, te lo retiran.");
    return 1;
}

// ---------------------------------------------------------------------
//  EL 112: un ciudadano avisa a la policia
//
//  Y por fin le damos uso a cuentas.telefono, que llevaba meses
//  generandose al crear la cuenta sin que lo usara nadie.
// ---------------------------------------------------------------------
CMD:911(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (EstaVacio(params))
    {
        Uso(playerid, "/911 [que esta pasando]");
        Linea(playerid, COLOR_GRIS, "La policia vera tu nombre, tu telefono y donde estas.");
        return 1;
    }

    if (gettime() - Jugador[playerid][j911Ultimo] < POLICIA_911_ESPERA)
    {
        new quedan = POLICIA_911_ESPERA - (gettime() - Jugador[playerid][j911Ultimo]);
        Formato("Espera %d segundos antes de volver a llamar.", quedan);
        Error(playerid, g_Buffer);
        return 1;
    }

    if (Policia_AgentesConectados() == 0)
    {
        Error(playerid, "No hay ninguna patrulla de servicio ahora mismo.");
        return 1;
    }

    Jugador[playerid][j911Ultimo] = gettime();
    LimpiarTexto(params);

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    // La llamada entra en la MISMA cola que el resto de avisos, pero sin
    // pasar por el filtro de probabilidad: una denuncia de un vecino
    // llega siempre. Por eso se crea a mano en vez de con
    // Policia_CrearAviso.
    new hueco = -1;
    for (new i = 0; i < MAX_AVISOS_POLICIA; i++)
    {
        if (!g_Avisos[i][avActivo]) { hueco = i; break; }
    }
    if (hueco == -1)
    {
        Error(playerid, "La centralita esta saturada. Prueba en un minuto.");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);

    g_Avisos[hueco][avActivo]   = true;
    g_Avisos[hueco][avTipo]     = AVISO_LLAMADA;
    g_Avisos[hueco][avX]        = x;
    g_Avisos[hueco][avY]        = y;
    g_Avisos[hueco][avZ]        = z;
    g_Avisos[hueco][avInterior] = GetPlayerInterior(playerid);
    g_Avisos[hueco][avMundo]    = GetPlayerVirtualWorld(playerid);
    g_Avisos[hueco][avHora]     = gettime();
    g_Avisos[hueco][avAgente]   = INVALID_PLAYER_ID;
    g_Avisos[hueco][avCasos]    = 1;
    format(g_Avisos[hueco][avNombre], MAX_PLAYER_NAME + 1, "%s", nombre);

    Exito(playerid, "Llamada registrada. Una patrulla va a verlo.");

    new linea[220];
    format(linea, sizeof linea, "112 - Aviso #%d de %s (tel. %d): %s", hueco + 1, nombre, Jugador[playerid][jTelefono], params);
    Policia_AvisarCentral(linea);
    Policia_AvisarCentral("Si la llamada era falsa, el delito es el 605.");
    return 1;
}

// ---------------------------------------------------------------------
//  CONTROL DE TRAFICO
// ---------------------------------------------------------------------

CMD:pedirpapeles(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/pedirpapeles [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;
    if (!EstaCerca(playerid, objetivo, POLICIA_DISTANCIA))
    {
        Error(playerid, "Tienes que estar al lado para pedirle la documentacion.");
        return 1;
    }

    new suyo[MAX_PLAYER_NAME + 1];
    NombreRP(objetivo, suyo, sizeof suyo);

    Formato("--- Documentacion de %s ---", suyo);
    Linea(playerid, COLOR_MARCA, g_Buffer);

    if (!Jugador[objetivo][jCarnet])
    {
        Linea(playerid, COLOR_ROJO, "Carnet de conducir: NO TIENE.");
        Linea(playerid, COLOR_GRIS, "Si va conduciendo, el delito es el 104.");
    }
    else
    {
        Formato("Carnet: en vigor, %d de %d puntos.", Jugador[objetivo][jCarnetPuntos], CARNET_PUNTOS_INICIALES);
        Linea(playerid, (Jugador[objetivo][jCarnetPuntos] <= 4) ? COLOR_AVISO : COLOR_VERDE, g_Buffer);
    }

    // Papeles del coche en el que va, si va en alguno.
    new vehiculoid = GetPlayerVehicleID(objetivo);
    if (vehiculoid == 0)
    {
        Linea(playerid, COLOR_GRIS, "No va en ningun vehiculo ahora mismo.");
    }
    else
    {
        new indice = Veh_BuscarPorObjeto(vehiculoid);
        if (indice == -1)
        {
            Linea(playerid, COLOR_GRIS, "El vehiculo no esta registrado.");
        }
        else if (g_Vehiculos[indice][veTipo] != VEH_DROPEO)
        {
            Linea(playerid, COLOR_VERDE, "El vehiculo no necesita papeles.");
        }
        else
        {
            new restante = g_Vehiculos[indice][veDropeoExpira] - gettime();
            if (g_Vehiculos[indice][vePropietario] == 0)
            {
                Linea(playerid, COLOR_ROJO, "Vehiculo SIN DUEÑO. Papeles: ninguno. Delito 101.");
            }
            else if (restante <= 0)
            {
                Linea(playerid, COLOR_ROJO, "Papeles CADUCADOS. Delito 101.");
            }
            else
            {
                Formato("Papeles en regla: le quedan %d dia(s).", restante / 86400);
                Linea(playerid, COLOR_VERDE, g_Buffer);
            }
        }
    }

    Formato("* %s le pide la documentacion a %s.", Jugador[playerid][jNombre], suyo);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, g_Buffer);
    return 1;
}

CMD:radar(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;
    if (Policia_NoEsAgente(playerid)) return 1;

    // El vehiculo con conductor mas cercano, dentro del alcance.
    new objetivo = INVALID_PLAYER_ID, Float:mejor = RADAR_ALCANCE;

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (i == playerid) continue;
        if (GetPlayerState(i) != PLAYER_STATE_DRIVER) continue;
        if (!EnMismaDimension(playerid, i)) continue;

        new Float:d = DistanciaEntreJugadores(playerid, i);
        if (d < mejor) { mejor = d; objetivo = i; }
    }

    if (objetivo == INVALID_PLAYER_ID)
    {
        Error(playerid, "El radar no coge ningun vehiculo en marcha cerca.");
        return 1;
    }

    // Mismo calculo que el velocimetro del HUD (13_vehiculos.inc).
    new Float:vx, Float:vy, Float:vz;
    GetVehicleVelocity(GetPlayerVehicleID(objetivo), vx, vy, vz);
    new velocidad = floatround(floatsqroot(vx * vx + vy * vy + vz * vz) * 178.0);

    new suyo[MAX_PLAYER_NAME + 1];
    NombreRP(objetivo, suyo, sizeof suyo);

    if (velocidad > RADAR_LIMITE)
    {
        Formato("RADAR: %s a %d km/h (limite %d). Delito 102.", suyo, velocidad, RADAR_LIMITE);
        Linea(playerid, COLOR_ROJO, g_Buffer);
    }
    else
    {
        Formato("RADAR: %s a %d km/h. Dentro del limite.", suyo, velocidad);
        Linea(playerid, COLOR_VERDE, g_Buffer);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  DIALOGOS
//  Todos los de este modulo son informativos: se cierran y ya.
// ---------------------------------------------------------------------
public Policia_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused response
    #pragma unused listitem
    #pragma unused inputtext

    switch (dialogid)
    {
        case DIALOGO_POLICIA_CODIGO,
             DIALOGO_POLICIA_BUSCADOS,
             DIALOGO_POLICIA_ANTECEDENTES,
             DIALOGO_POLICIA_MULTA,
             DIALOGO_POLICIA_AVISOS:
        {
            return 1;
        }
    }
    return 0;
}
