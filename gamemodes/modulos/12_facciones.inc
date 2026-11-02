/* =====================================================================
 *  12_facciones.inc
 *  ---------------------------------------------------------------------
 *  Seis ranuras de faccion. La 1 es la POLICIA NACIONAL (activa desde el
 *  20/08/2026, primera faccion del servidor); las otras cinco siguen
 *  libres y listas para rellenar.
 *
 *  ------------------------ COMO CREAR UNA FACCION ------------------------
 *  1. Elige una ranura libre en el array g_Facciones (de la 1 a la 6).
 *  2. Cambia nombre, siglas, descripcion, tipo y color.
 *  3. Pon las coordenadas de la sede (con /pos dentro del juego).
 *  4. Cambia faActiva de false a true.
 *  5. Escribe los diez rangos en g_RangosFaccion, misma posicion.
 *  6. Recompila y nombra a un lider con /darfaccion y /liderfaccion.
 *  ------------------------------------------------------------------------
 *
 *  Normas del sistema (ya implementadas):
 *    · Un jugador solo puede pertenecer a UNA faccion.
 *    · Los vehiculos de faccion solo los conduce quien pertenece a ella,
 *      y ademas se puede exigir un rango minimo por vehiculo.
 *    · Pertenecer a una faccion sube la nomina a 1200$ por hora.
 *
 *  Tipos de faccion:
 *    FAC_GOBIERNO  policia, ayuntamiento, justicia (acceso a canal departamental)
 *    FAC_SANIDAD   hospital, bomberos
 *    FAC_PRENSA    medios de comunicacion
 *    FAC_CRIMINAL  bandas, mafias
 *    FAC_EMPRESA   negocios legales grandes
 *    FAC_OTRA      cualquier otra cosa
 * ===================================================================== */

#define FAC_GOBIERNO            1
#define FAC_SANIDAD             2
#define FAC_PRENSA              3
#define FAC_CRIMINAL            4
#define FAC_EMPRESA             5
#define FAC_OTRA                6

enum E_FACCION
{
    faNombre[48],
    faSiglas[8],
    bool:faActiva,
    faDescripcion[144],
    faTipo,
    faColor,
    Float:faX,
    Float:faY,
    Float:faZ,
    faInterior,
    faMundo,
    faMaxMiembros,

    // Linea extra opcional para la etiqueta de la sede. Existe porque
    // algunas sedes son ademas una PUERTA por la que se entra (la
    // comisaria de la ranura 1 es el interior 6, ver 37_policia.inc), y
    // dos etiquetas 3D en la misma coordenada se solapan y no hay quien
    // las lea. Una sola etiqueta lo dice todo. Vacio = no se añade nada.
    faEntrada[72]
}

// ---------------------------------------------------------------------
//  LAS SEIS RANURAS
//  La posicion 0 es "sin faccion", no la borres.
// ---------------------------------------------------------------------
new g_Facciones[MAX_FACCIONES + 1][E_FACCION] =
{
    {"Ninguna", "", false, "", 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, ""},

    // Nombre                Siglas  Activa  Descripcion
    //                       Tipo          Color        Coordenadas de la sede        Int Mundo  Max
    //
    // RANURA 1 -- POLICIA NACIONAL (activada el 20/08/2026, primera faccion
    // del servidor). Tipo FAC_GOBIERNO: eso es lo que le da acceso al canal
    // /dep, via Faccion_EsPolicial(). La sede esta en la puerta de la
    // comisaria de Pershing Square; si el marcador queda descolocado, se
    // corrige con /guardarpos dentro del juego y se cambian estas tres
    // coordenadas.
    {"Policía Nacional",     "CNP",  true,   "Cuerpo Nacional de Policía. Orden público, trafico y delitos en Los Santos.",
                             FAC_GOBIERNO, 0x5A9BD5FF,  1554.9, -1675.6, 16.2,         0,   0,    30,
                             "{FFFFFF}Escribe {E8B33A}/entrar{FFFFFF} para pasar a la comisaría"},

    // RANURA 2 -- RESERVADA PARA GROVE STREET (decidido el 20/08/2026).
    //
    // Sera la SEGUNDA faccion del servidor, y es a proposito una banda
    // criminal: la Policia Nacional (ranura 1) no tiene con quien rolear
    // mientras el unico "delincuente" sea un jugador suelto haciendo
    // cosas por su cuenta. Con Grove Street enfrente, el codigo penal y
    // los avisos de 37_policia.inc dejan de ser mecanica y pasan a ser
    // una historia entre dos grupos.
    //
    // Para activarla: tipo FAC_CRIMINAL, siglas "GSF", sede en Grove
    // Street (la coordenada se saca con /pos dentro del juego), rangos
    // propios de banda, y faActiva a true.
    //
    // Ojo con una cosa al hacerlo: el barrio de Grove Street ya es el
    // sitio donde mas cosas pasan del servidor (la casa de ejemplo, la
    // gasolinera y la Tienda Grove estan ahi). Conviene mirar que la
    // sede no caiga encima de un negocio o de la puerta de una casa.
    {"Faccion libre 2",      "F2",   false,  "Reservada para Grove Street. Ver 12_facciones.inc.",
                             FAC_CRIMINAL, 0x53C168FF,  1481.0, -1771.0, 18.8,         0,   0,    30, ""},

    {"Faccion libre 3",      "F3",   false,  "Ranura libre. Ver instrucciones en 12_facciones.inc.",
                             FAC_OTRA,     0x53C168FF,  1481.0, -1771.0, 18.8,         0,   0,    30, ""},

    {"Faccion libre 4",      "F4",   false,  "Ranura libre. Ver instrucciones en 12_facciones.inc.",
                             FAC_OTRA,     0xE8973AFF,  1481.0, -1771.0, 18.8,         0,   0,    30, ""},

    {"Faccion libre 5",      "F5",   false,  "Ranura libre. Ver instrucciones en 12_facciones.inc.",
                             FAC_OTRA,     0xC2A2DAFF,  1481.0, -1771.0, 18.8,         0,   0,    30, ""},

    {"Faccion libre 6",      "F6",   false,  "Ranura libre. Ver instrucciones en 12_facciones.inc.",
                             FAC_OTRA,     0x4FD1C5FF,  1481.0, -1771.0, 18.8,         0,   0,    30, ""}
};

// ---------------------------------------------------------------------
//  RANGOS DE CADA FACCION (del 1 al 10)
// ---------------------------------------------------------------------
static const g_RangosFaccion[MAX_FACCIONES + 1][MAX_RANGOS][32] =
{
    {"", "", "", "", "", "", "", "", "", ""},
    // Ranura 1 -- Policia Nacional. Escalafon real del CNP español, de
    // menor a mayor. El rango 10 es el jefe de la faccion.
    {"Aspirante", "Policía", "Policía Primera", "Oficial", "Subinspector",
     "Inspector", "Inspector Jefe", "Comisario", "Comisario Principal", "Comisario General"},
    {"Rango 1", "Rango 2", "Rango 3", "Rango 4", "Rango 5", "Rango 6", "Rango 7", "Rango 8", "Subdirector", "Director"},
    {"Rango 1", "Rango 2", "Rango 3", "Rango 4", "Rango 5", "Rango 6", "Rango 7", "Rango 8", "Subdirector", "Director"},
    {"Rango 1", "Rango 2", "Rango 3", "Rango 4", "Rango 5", "Rango 6", "Rango 7", "Rango 8", "Subdirector", "Director"},
    {"Rango 1", "Rango 2", "Rango 3", "Rango 4", "Rango 5", "Rango 6", "Rango 7", "Rango 8", "Subdirector", "Director"},
    {"Rango 1", "Rango 2", "Rango 3", "Rango 4", "Rango 5", "Rango 6", "Rango 7", "Rango 8", "Subdirector", "Director"}
};

// ---------------------------------------------------------------------
//  UNIFORMES POR RANGO
//  ---------------------------------------------------------------------
//  Mismo formato que g_RangosFaccion: una fila por faccion, un skin por
//  rango, en la misma posicion. Un 0 significa "este rango no lleva
//  uniforme" -- y una fila entera de ceros, "esta faccion no tiene".
//
//  Quien decide que aspecto lleva puesto un jugador NO es este array, es
//  Skin_Actualizar() (03_utiles.inc). Aqui solo esta el dato.
//
//  RANURA 1 -- POLICIA NACIONAL. La idea es la del CNP de verdad: la
//  escala basica va de uniforme y la ejecutiva, de Inspector para
//  arriba, va de PAISANO. Que un Comisario se distinga de un Policia sin
//  tener que mirarle el /mifaccion es justo lo que hace que el escalafon
//  se note dentro del juego, igual que la armeria.
//
//    280  uniforme de calle       Aspirante, Policia, Policia Primera
//    281  uniforme, otra variante Oficial, Subinspector
//    286  traje (de paisano)      Inspector e Inspector Jefe
//    285  traje de mando          Comisario, Principal y General
//
//  Los cuatro son de la familia policial de GTA:SA. Si alguno no
//  convence al verlo en el juego, se cambia el numero de esta tabla y ya
//  esta: no hay ningun comando que dependa de un skin concreto.
// ---------------------------------------------------------------------
static const g_SkinsFaccion[MAX_FACCIONES + 1][MAX_RANGOS] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    // Ranura 1 -- Policia Nacional
    //  1    2    3    4    5    6    7    8    9   10
    {280, 280, 280, 281, 281, 286, 286, 285, 285, 285},

    // Ranura 2 -- reservada para Grove Street. Cuando se active, aqui
    // van sus skins (los de la banda de GTA:SA son 105, 106 y 107).
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


static STREAMER_TAG_PICKUP:g_PickupFaccion[MAX_FACCIONES + 1];
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaFaccion[MAX_FACCIONES + 1];

// ---------------------------------------------------------------------
//  MARCADORES DE LAS SEDES
// ---------------------------------------------------------------------
stock Faccion_CrearMarcadores()
{
    new activas = 0, etiqueta[224];

    for (new f = 1; f <= MAX_FACCIONES; f++)
    {
        if (!g_Facciones[f][faActiva]) continue;

        g_PickupFaccion[f] = CreateDynamicPickup(1247, 23,
            g_Facciones[f][faX], g_Facciones[f][faY], g_Facciones[f][faZ],
            g_Facciones[f][faMundo], g_Facciones[f][faInterior], -1, 100.0);

        format(etiqueta, sizeof etiqueta, "%s\n{FFFFFF}Sede oficial", g_Facciones[f][faNombre]);

        // Sedes que ademas son una puerta (la comisaria, de momento):
        // lo dice esta misma etiqueta en vez de plantar otra encima.
        if (g_Facciones[f][faEntrada][0] != EOS)
        {
            format(etiqueta, sizeof etiqueta, "%s\n%s", etiqueta, g_Facciones[f][faEntrada]);
        }

        g_EtiquetaFaccion[f] = CreateDynamic3DTextLabel(etiqueta, g_Facciones[f][faColor],
            g_Facciones[f][faX], g_Facciones[f][faY], g_Facciones[f][faZ] + 0.6, 12.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0,
            g_Facciones[f][faMundo], g_Facciones[f][faInterior], -1, 30.0);

        activas++;
    }

    printf("  [FACCIONES] %d de %d ranuras activas.", activas, MAX_FACCIONES);
    return activas;
}

// ---------------------------------------------------------------------
//  CONSULTAS
// ---------------------------------------------------------------------
stock bool:FaccionValida(faccion)
{
    return (faccion > 0 && faccion <= MAX_FACCIONES);
}

// Skin que le toca a ese rango de esa faccion, o 0 si no lleva uniforme.
// Es la puerta de g_SkinsFaccion hacia fuera: ese array es static y no se
// ve desde otro modulo, asi que nadie lo lee directamente (misma regla
// que g_NegociosDef y Negocio_Posicion).
//
// Vive AQUI, despues de FaccionValida, y no arriba junto a la tabla: una
// funcion que usa otra con tag de retorno (bool:) antes de que este
// definida obliga al compilador a reparsear el archivo entero y suelta
// un warning 208.
public Faccion_SkinDe(faccion, rango)
{
    if (!FaccionValida(faccion)) return 0;
    if (rango < 1 || rango > MAX_RANGOS) return 0;

    return g_SkinsFaccion[faccion][rango - 1];
}

public Faccion_Nombre(faccion, destino[], tam)
{
    if (!FaccionValida(faccion)) { format(destino, tam, "Ninguna"); return 0; }
    format(destino, tam, "%s", g_Facciones[faccion][faNombre]);
    return 1;
}

public Faccion_NombreRango(faccion, rango, destino[], tam)
{
    if (!FaccionValida(faccion) || rango < 1 || rango > MAX_RANGOS)
    {
        format(destino, tam, "Sin rango");
        return 0;
    }
    format(destino, tam, "%s", g_RangosFaccion[faccion][rango - 1]);
    return 1;
}

public Faccion_Color(faccion)
{
    if (!FaccionValida(faccion)) return COLOR_BLANCO;
    return g_Facciones[faccion][faColor];
}

public bool:Faccion_EsPolicial(faccion)
{
    if (!FaccionValida(faccion)) return false;
    return (g_Facciones[faccion][faTipo] == FAC_GOBIERNO);
}

stock Faccion_ContarMiembros(faccion)
{
    new total = 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jFaccion] == faccion) total++;
    }
    return total;
}

// Envia un mensaje a todos los miembros conectados de una faccion.
stock Faccion_Mensaje(faccion, color, const texto[])
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jFaccion] != faccion) continue;
        SendClientMessage(i, color, texto);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  ALTA Y BAJA
// ---------------------------------------------------------------------
stock Faccion_Dar(playerid, faccion, rango)
{
    if (!FaccionValida(faccion)) return 0;

    if (rango < 1) rango = 1;
    if (rango > MAX_RANGOS) rango = MAX_RANGOS;

    Jugador[playerid][jFaccion]      = faccion;
    Jugador[playerid][jFaccionRango] = rango;
    Jugador[playerid][jFaccionLider] = false;

    // Los vehiculos de la faccion se le abren al instante.
    Veh_ActualizarCierreJugador(playerid);

    // Y se le pone el uniforme que le toque, si esa faccion tiene.
    Skin_Actualizar(playerid);
    return 1;
}

stock Faccion_Quitar(playerid)
{
    Jugador[playerid][jFaccion]      = 0;
    Jugador[playerid][jFaccionRango] = 0;
    Jugador[playerid][jFaccionLider] = false;

    // Y los vehiculos de la faccion se le cierran.
    Veh_ActualizarCierreJugador(playerid);

    // El uniforme se devuelve al salir. Si no le queda ningun otro
    // puesto con ropa de trabajo, vuelve a su aspecto de siempre.
    Skin_Actualizar(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDOS DE CONSULTA
// ---------------------------------------------------------------------
CMD:facciones(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new lista[2048], linea[256], activas = 0;

    static const nombresTipo[][] = { "", "Gobierno", "Sanidad", "Prensa", "Criminal", "Empresa", "Otra" };

    format(lista, sizeof lista, "{E8B33A}Faccion\tTipo\tMiembros\n");

    for (new f = 1; f <= MAX_FACCIONES; f++)
    {
        if (!g_Facciones[f][faActiva]) continue;

        format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%s\t{B4B5B7}%d de %d\n",
            g_Facciones[f][faNombre],
            nombresTipo[g_Facciones[f][faTipo]],
            Faccion_ContarMiembros(f),
            g_Facciones[f][faMaxMiembros]);

        strcat(lista, linea, sizeof lista);
        activas++;
    }

    if (activas == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_NADA, DIALOG_STYLE_MSGBOX,
            "{E8B33A}Facciones",
            "{B4B5B7}Todavia no hay ninguna faccion en marcha.\n\n\
            Las facciones se abren por fases y se entra por seleccion,\n\
            no apuntandose. El equipo del servidor las ira anunciando.",
            "Cerrar", "");
        return 1;
    }

    ShowPlayerDialog(playerid, DIALOGO_FACCION_RANGOS, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8B33A}Facciones de Los Santos", lista, "Ver ficha", "Cerrar");
    return 1;
}

CMD:mifaccion(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new f = Jugador[playerid][jFaccion];
    if (!FaccionValida(f))
    {
        Error(playerid, "No perteneces a ninguna faccion.");
        return 1;
    }

    new rango[32];
    Faccion_NombreRango(f, Jugador[playerid][jFaccionRango], rango, sizeof rango);

    new cuerpo[800];
    format(cuerpo, sizeof cuerpo,
        "{E8B33A}%s{B4B5B7}  (%s)\n\n%s\n\n\
        {B4B5B7}Tu rango       {FFFFFF}%d de %d  -  %s%s\n\
        {B4B5B7}Miembros       {FFFFFF}%d{B4B5B7} conectados de un maximo de %d\n\n\
        {E8B33A}Comandos\n\
        {FFFFFF}/f{B4B5B7} texto        chat interno\n\
        {FFFFFF}/radio{B4B5B7} texto    radio de servicio\n\
        {FFFFFF}/miembros{B4B5B7}       companeros conectados\n\
        {FFFFFF}/salirfaccion{B4B5B7}   abandonar la faccion",
        g_Facciones[f][faNombre], g_Facciones[f][faSiglas], g_Facciones[f][faDescripcion],
        Jugador[playerid][jFaccionRango], MAX_RANGOS, rango,
        (Jugador[playerid][jFaccionLider]) ? (" {E8B33A}[Lider]") : (""),
        Faccion_ContarMiembros(f), g_Facciones[f][faMaxMiembros]);

    ShowPlayerDialog(playerid, DIALOGO_NADA, DIALOG_STYLE_MSGBOX,
        "{E8B33A}Tu faccion", cuerpo, "Cerrar", "");
    return 1;
}

CMD:miembros(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new f = Jugador[playerid][jFaccion];
    if (!FaccionValida(f))
    {
        Error(playerid, "No perteneces a ninguna faccion.");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], rango[32], conectados = 0;

    format(g_Buffer, sizeof g_Buffer, "Miembros de %s conectados:", g_Facciones[f][faNombre]);
    Linea(playerid, Faccion_Color(f), g_Buffer);

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jFaccion] != f) continue;

        NombreRP(i, nombre, sizeof nombre);
        Faccion_NombreRango(f, Jugador[i][jFaccionRango], rango, sizeof rango);

        format(g_Buffer, sizeof g_Buffer, "  {FFFFFF}%s{B4B5B7} - %s%s - ID %d",
            nombre, rango, (Jugador[i][jFaccionLider]) ? (" [Lider]") : (""), i);
        SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
        conectados++;
    }

    if (conectados == 0) Linea(playerid, COLOR_GRIS, "  No hay nadie mas conectado.");
    return 1;
}

// ---------------------------------------------------------------------
//  CANALES DE COMUNICACIÓN
// ---------------------------------------------------------------------
CMD:f(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    new f = Jugador[playerid][jFaccion];
    if (!FaccionValida(f))
    {
        Error(playerid, "No perteneces a ninguna faccion.");
        return 1;
    }

    if (EstaVacio(params))
    {
        Uso(playerid, "/f [mensaje para tu faccion]");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], rango[32], mensaje[224];
    NombreRP(playerid, nombre, sizeof nombre);
    Faccion_NombreRango(f, Jugador[playerid][jFaccionRango], rango, sizeof rango);
    LimpiarTexto(params);

    format(mensaje, sizeof mensaje, "[%s] %s %s: %s", g_Facciones[f][faSiglas], rango, nombre, params);
    Faccion_Mensaje(f, Faccion_Color(f), mensaje);
    return 1;
}

CMD:radio(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    new f = Jugador[playerid][jFaccion];
    if (!FaccionValida(f))
    {
        Error(playerid, "No perteneces a ninguna faccion.");
        return 1;
    }

    if (EstaVacio(params))
    {
        Uso(playerid, "/radio [mensaje por la emisora de servicio]");
        return 1;
    }

    if (!Inv_TieneItem(playerid, Item_Buscar("Radio portatil")))
    {
        Error(playerid, "No llevas encima una radio portatil.");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], rango[32], mensaje[224];
    NombreRP(playerid, nombre, sizeof nombre);
    Faccion_NombreRango(f, Jugador[playerid][jFaccionRango], rango, sizeof rango);
    LimpiarTexto(params);

    format(mensaje, sizeof mensaje, "** [%s] %s %s: %s **", g_Facciones[f][faSiglas], rango, nombre, params);
    Faccion_Mensaje(f, COLOR_RADIO, mensaje);

    // Quien esté al lado oye la voz saliendo del aparato.
    format(mensaje, sizeof mensaje, "* La radio de %s crepita mientras habla por ella.", nombre);
    MensajeCercano(playerid, 8.0, COLOR_ACCION, mensaje);
    return 1;
}

// Canal departamental: lo comparten todas las facciones de gobierno.
CMD:dep(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (EstaSilenciado(playerid)) return 1;

    new f = Jugador[playerid][jFaccion];
    if (!Faccion_EsPolicial(f))
    {
        Error(playerid, "Ese canal es solo para las facciones de gobierno.");
        return 1;
    }

    if (EstaVacio(params))
    {
        Uso(playerid, "/dep [mensaje al canal departamental]");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], rango[32], mensaje[224];
    NombreRP(playerid, nombre, sizeof nombre);
    Faccion_NombreRango(f, Jugador[playerid][jFaccionRango], rango, sizeof rango);
    LimpiarTexto(params);

    format(mensaje, sizeof mensaje, "[DEPARTAMENTAL] %s %s (%s): %s",
        rango, nombre, g_Facciones[f][faSiglas], params);

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (!Faccion_EsPolicial(Jugador[i][jFaccion])) continue;
        SendClientMessage(i, COLOR_DEPARTAMENTO, mensaje);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  MANDO DE LA FACCIÓN
//  Requiere ser lider, o tener el rango maximo.
// ---------------------------------------------------------------------
stock bool:Faccion_PuedeMandar(playerid)
{
    if (!FaccionValida(Jugador[playerid][jFaccion])) return false;
    if (Jugador[playerid][jFaccionLider]) return true;
    return (Jugador[playerid][jFaccionRango] >= MAX_RANGOS);
}

CMD:invitar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (!Faccion_PuedeMandar(playerid))
    {
        Error(playerid, "Solo el mando de la faccion puede invitar.");
        return 1;
    }

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/invitar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    new f = Jugador[playerid][jFaccion];

    if (objetivo == playerid) { Error(playerid, "No puedes invitarte a ti mismo."); return 1; }
    if (FaccionValida(Jugador[objetivo][jFaccion]))
    {
        Error(playerid, "Esa persona ya pertenece a una faccion.");
        return 1;
    }
    if (!EstaCerca(playerid, objetivo, RADIO_INTERACCION))
    {
        Error(playerid, "Tienes que estar al lado de esa persona.");
        return 1;
    }
    if (Faccion_ContarMiembros(f) >= g_Facciones[f][faMaxMiembros])
    {
        Error(playerid, "La faccion esta llena.");
        return 1;
    }

    Jugador[objetivo][jTemporal] = f;

    new mio[MAX_PLAYER_NAME + 1], suyo[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, mio, sizeof mio);
    NombreRP(objetivo, suyo, sizeof suyo);

    new cuerpo[512];
    format(cuerpo, sizeof cuerpo,
        "{FFFFFF}%s{B4B5B7} te ha invitado a entrar en:\n\n{E8B33A}%s\n\n{B4B5B7}%s\n\n\
        {B4B5B7}Solo puedes pertenecer a una faccion a la vez.",
        mio, g_Facciones[f][faNombre], g_Facciones[f][faDescripcion]);

    ShowPlayerDialog(objetivo, DIALOGO_FACCION_GESTION, DIALOG_STYLE_MSGBOX,
        "{E8B33A}Invitacion de faccion", cuerpo, "Aceptar", "Rechazar");

    format(g_Buffer, sizeof g_Buffer, "Has invitado a %s a la faccion.", suyo);
    Exito(playerid, g_Buffer);
    return 1;
}

CMD:ascender(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (!Faccion_PuedeMandar(playerid))
    {
        Error(playerid, "Solo el mando de la faccion puede ascender.");
        return 1;
    }

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/ascender [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (Jugador[objetivo][jFaccion] != Jugador[playerid][jFaccion])
    {
        Error(playerid, "Esa persona no es de tu faccion.");
        return 1;
    }
    if (Jugador[objetivo][jFaccionRango] >= MAX_RANGOS)
    {
        Error(playerid, "Ya tiene el rango mas alto.");
        return 1;
    }
    if (objetivo != playerid && Jugador[objetivo][jFaccionRango] + 1 >= Jugador[playerid][jFaccionRango] && !Jugador[playerid][jFaccionLider])
    {
        Error(playerid, "No puedes ascender a alguien a tu mismo rango o por encima.");
        return 1;
    }

    Jugador[objetivo][jFaccionRango]++;
    Skin_Actualizar(objetivo);      // el uniforme va por rango
    BD_GuardarCuenta(objetivo);

    new nombre[MAX_PLAYER_NAME + 1], rango[32], mensaje[192];
    NombreRP(objetivo, nombre, sizeof nombre);
    Faccion_NombreRango(Jugador[objetivo][jFaccion], Jugador[objetivo][jFaccionRango], rango, sizeof rango);

    format(mensaje, sizeof mensaje, "%s ha sido ascendido a %s.", nombre, rango);
    Faccion_Mensaje(Jugador[playerid][jFaccion], Faccion_Color(Jugador[playerid][jFaccion]), mensaje);
    return 1;
}

CMD:degradar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (!Faccion_PuedeMandar(playerid))
    {
        Error(playerid, "Solo el mando de la faccion puede degradar.");
        return 1;
    }

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/degradar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (Jugador[objetivo][jFaccion] != Jugador[playerid][jFaccion])
    {
        Error(playerid, "Esa persona no es de tu faccion.");
        return 1;
    }
    if (Jugador[objetivo][jFaccionRango] <= 1)
    {
        Error(playerid, "Ya tiene el rango mas bajo.");
        return 1;
    }
    if (Jugador[objetivo][jFaccionLider])
    {
        Error(playerid, "No puedes degradar al lider.");
        return 1;
    }

    Jugador[objetivo][jFaccionRango]--;
    Skin_Actualizar(objetivo);      // el uniforme va por rango
    BD_GuardarCuenta(objetivo);

    new nombre[MAX_PLAYER_NAME + 1], rango[32], mensaje[192];
    NombreRP(objetivo, nombre, sizeof nombre);
    Faccion_NombreRango(Jugador[objetivo][jFaccion], Jugador[objetivo][jFaccionRango], rango, sizeof rango);

    format(mensaje, sizeof mensaje, "%s ha sido degradado a %s.", nombre, rango);
    Faccion_Mensaje(Jugador[playerid][jFaccion], Faccion_Color(Jugador[playerid][jFaccion]), mensaje);
    return 1;
}

CMD:expulsar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (!Faccion_PuedeMandar(playerid))
    {
        Error(playerid, "Solo el mando de la faccion puede expulsar.");
        return 1;
    }

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/expulsar [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (Jugador[objetivo][jFaccion] != Jugador[playerid][jFaccion])
    {
        Error(playerid, "Esa persona no es de tu faccion.");
        return 1;
    }
    if (objetivo == playerid) { Error(playerid, "Para irte usa /salirfaccion."); return 1; }
    if (Jugador[objetivo][jFaccionLider])
    {
        Error(playerid, "No puedes expulsar al lider.");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192], f = Jugador[playerid][jFaccion];
    NombreRP(objetivo, nombre, sizeof nombre);

    Faccion_Quitar(objetivo);
    BD_GuardarCuenta(objetivo);

    format(mensaje, sizeof mensaje, "%s ha sido expulsado de la faccion.", nombre);
    Faccion_Mensaje(f, Faccion_Color(f), mensaje);

    Error(objetivo, "Te han expulsado de la faccion.");
    return 1;
}

CMD:salirfaccion(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new f = Jugador[playerid][jFaccion];
    if (!FaccionValida(f))
    {
        Error(playerid, "No perteneces a ninguna faccion.");
        return 1;
    }

    if (Jugador[playerid][jFaccionLider])
    {
        Error(playerid, "Eres el lider. Un administrador tiene que traspasar el mando antes.");
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
    NombreRP(playerid, nombre, sizeof nombre);

    Faccion_Quitar(playerid);
    BD_GuardarCuenta(playerid);

    format(mensaje, sizeof mensaje, "%s ha abandonado la faccion.", nombre);
    Faccion_Mensaje(f, Faccion_Color(f), mensaje);

    Exito(playerid, "Has abandonado la faccion. Tu nomina vuelve a la de un trabajador normal.");
    return 1;
}

// ---------------------------------------------------------------------
//  DIÁLOGOS
// ---------------------------------------------------------------------
stock Faccion_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext

    switch (dialogid)
    {
        case DIALOGO_FACCION_RANGOS:
        {
            if (!response) return 1;

            new contador = 0, elegida = 0;
            for (new f = 1; f <= MAX_FACCIONES; f++)
            {
                if (!g_Facciones[f][faActiva]) continue;
                if (contador == listitem) { elegida = f; break; }
                contador++;
            }
            if (elegida == 0) return 1;

            new cuerpo[900];
            format(cuerpo, sizeof cuerpo,
                "{E8B33A}%s{B4B5B7}  (%s)\n\n%s\n\n{E8B33A}Escalafon\n",
                g_Facciones[elegida][faNombre], g_Facciones[elegida][faSiglas], g_Facciones[elegida][faDescripcion]);

            new linea[64];
            for (new r = 0; r < MAX_RANGOS; r++)
            {
                format(linea, sizeof linea, "{B4B5B7}%d.  %s\n", r + 1, g_RangosFaccion[elegida][r]);
                strcat(cuerpo, linea, sizeof cuerpo);
            }

            strcat(cuerpo, "\n{B4B5B7}A las facciones se entra por invitacion de su mando.", sizeof cuerpo);

            ShowPlayerDialog(playerid, DIALOGO_NADA, DIALOG_STYLE_MSGBOX,
                "{E8B33A}Ficha de la faccion", cuerpo, "Cerrar", "");
            return 1;
        }

        case DIALOGO_FACCION_GESTION:
        {
            new f = Jugador[playerid][jTemporal];
            Jugador[playerid][jTemporal] = 0;

            if (!response) return 1;
            if (!FaccionValida(f)) return 1;

            if (FaccionValida(Jugador[playerid][jFaccion]))
            {
                Error(playerid, "Ya perteneces a una faccion.");
                return 1;
            }
            if (Faccion_ContarMiembros(f) >= g_Facciones[f][faMaxMiembros])
            {
                Error(playerid, "La faccion se ha llenado mientras lo pensabas.");
                return 1;
            }

            Faccion_Dar(playerid, f, 1);
            BD_GuardarCuenta(playerid);

            new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
            NombreRP(playerid, nombre, sizeof nombre);

            format(mensaje, sizeof mensaje, "%s se ha unido a la faccion.", nombre);
            Faccion_Mensaje(f, Faccion_Color(f), mensaje);

            format(mensaje, sizeof mensaje, "Ahora perteneces a %s. Escribe /mifaccion para ver los detalles.", g_Facciones[f][faNombre]);
            Exito(playerid, mensaje);
            return 1;
        }
    }
    return 0;
}
