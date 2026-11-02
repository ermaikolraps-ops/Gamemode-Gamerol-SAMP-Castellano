/* =====================================================================
 *  11_trabajos.inc
 *  ---------------------------------------------------------------------
 *  Seis ranuras de trabajo libres, listas para rellenar.
 *
 *  ------------------------- COMO CREAR UN TRABAJO -------------------------
 *  1. Baja al array g_Trabajos y elige una ranura libre (de la 1 a la 6).
 *  2. Cambia el nombre, la descripcion y el color.
 *  3. Pon las coordenadas del punto donde se contrata. Si no las sabes,
 *     entra al juego, ponte donde quieras y escribe /pos: te las da.
 *  4. Cambia trActivo de false a true.
 *  5. Escribe los seis rangos en g_RangosTrabajo, en la misma posicion.
 *  6. Recompila. Ya aparece en /ayuda > Trabajos, tiene su marcador en
 *     el mapa y sus vehiculos solo los pueden conducir sus trabajadores.
 *  -------------------------------------------------------------------------
 *
 *  Normas del sistema (no hay que tocar nada para que se cumplan):
 *    · Un jugador puede tener como maximo MAX_TRABAJOS_JUGADOR (2).
 *    · Los vehiculos de trabajo solo los conduce quien tiene ese trabajo.
 *    · Tener al menos un trabajo sube la nomina de 800$ a 1000$.
 * ===================================================================== */

#define RANGOS_TRABAJO          6

enum E_TRABAJO
{
    trNombre[32],
    bool:trActivo,
    trDescripcion[144],
    Float:trX,
    Float:trY,
    Float:trZ,
    trInterior,
    trMundo,
    trNivelMinimo,
    trColor,
    trPagoExtra,                 // Suplemento por nomina, ademas del base
    trTextoCartel[144],          // Vacio = usa el cartel generico de abajo
    trComandos[650]              // Lista de comandos, se muestra en la ficha del trabajo
}

// ---------------------------------------------------------------------
//  LAS SEIS RANURAS
//  La posicion 0 es "sin trabajo", no la borres.
// ---------------------------------------------------------------------
new g_Trabajos[MAX_TRABAJOS + 1][E_TRABAJO] =
{
    {"Ninguno", false, "", 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, "", ""},

    // Vendedor de Armas. La logica completa vive en 25_vendedorarmas.inc
    // (sistema de NG-RP copiado 1:1). Punto de contratacion puesto a mano
    // desde /panel > Coordenadas (clave "posgetjobvendedorarmas").
    {"Vendedor de Armas",    true,   "Fabrica y vende armas con materiales.",
                             ARMAS_JOB_X, ARMAS_JOB_Y, ARMAS_JOB_Z,       0,   0,   0,  0xE05A5AFF,  0,
                             "Vendedor de Armas\n{E05A5A}/aceptarjob",
                             "{E8B33A}Comandos\n\
                             {FFFFFF}/aceptarjob{B4B5B7} - te haces miembro\n\
                             {FFFFFF}/pedirmats{B4B5B7} - pide un cargamento de materiales\n\
                             {FFFFFF}/venderarma{B4B5B7} id arma - fabrica y vende un arma\n\
                             {FFFFFF}/aceptararma{B4B5B7} - (comprador) acepta el arma ofrecida"},

    // NFS. La logica completa vive en 26_nfs.inc. Trafico de coches
    // libres: marcan un coche con /nfscoche y solo ellos lo pueden
    // vender. Coordenadas puestas a mano desde /panel (2026-08-13).
    {"NFS",                  true,   "Trafico de coches libres sin dueno.",
                             NFS_JOB_X, NFS_JOB_Y, NFS_JOB_Z,             0,   0,   0,  0x5A9BD5FF,  0,
                             "NFS\n{5A9BD5}/aceptarjob",
                             "{E8B33A}Comandos\n\
                             {FFFFFF}/aceptarjob{B4B5B7} - te haces miembro\n\
                             {FFFFFF}/nfsmotor{B4B5B7} - arranca/apaga un coche libre sin llave\n\
                             {FFFFFF}/nfscoche{B4B5B7} - marca un coche libre como NFS (en su taller)\n\
                             {FFFFFF}/nfspark{B4B5B7} - aparca un coche libre donde quieras\n\
                             {FFFFFF}/nfsvendercoche{B4B5B7} id precio - vende un coche NFS\n\
                             {FFFFFF}/aceptarnfs{B4B5B7} - (comprador) acepta la compra\n\
                             {FFFFFF}/nfspapeles{B4B5B7} id - vende 14 dias de papeles\n\
                             {FFFFFF}/aceptarpapeles{B4B5B7} - (dueno) acepta la renovacion"},

    // Vendedor de Marihuana. La logica completa vive en
    // 29_vendedormarihuana.inc. Punto de contratacion puesto a mano desde
    // /panel > Coordenadas (clave "posventaherb").
    {"Vendedor de Marihuana", true,  "Reparte marihuana a clientes por la ciudad.",
                             MVEND_JOB_X, MVEND_JOB_Y, MVEND_JOB_Z,       0,   0,   0,  0x53C168FF,  0,
                             "Vendedor de Marihuana\n{53C168}/aceptarjob",
                             "{E8B33A}Comandos\n\
                             {FFFFFF}/aceptarjob{B4B5B7} - te haces miembro\n\
                             {FFFFFF}/vendedor{B4B5B7} - activa o desactiva el telefono (en la cabina)\n\
                             {FFFFFF}/aceptarventa{B4B5B7} - acepta el encargo actual\n\
                             {FFFFFF}/entregarventa{B4B5B7} - entrega el genero al cliente\n\
                             {FFFFFF}/cancelarventa{B4B5B7} - cancela el encargo en curso"},

    {"Trabajo libre 4",      false,  "Ranura libre. Ver instrucciones en 11_trabajos.inc.",
                             1481.0, -1771.0, 18.8,                       0,   0,   1,  0xE8973AFF,  0, "", ""},

    // Medicos Los Santos. La logica completa vive en 36_medicos.inc.
    // Retirado el 07/09/2026 el sistema de heridos/caida -- ya no tiene
    // relacion con morir. Lo unico que hacen es curar a cualquier
    // jugador por un precio, en persona. Punto de contratacion movido el
    // 13/09/2026 DENTRO del interior del hospital (MEDICOS_JOB_X/Y/Z +
    // MUNDO_HOSPITAL_INTERIOR, 01_configuracion.inc; antes HOSPITAL_X/Y/Z
    // en la calle) -- pedido expreso de Vlad. Para llegar: GPS lleva a la
    // puerta (30_gps.inc) y desde ahi /entrar.
    // Es /tratar y no /curar porque ya existe un /curar de admin
    // (16_administracion.inc).
    {"Medicos Los Santos",   true,   "Cura a cualquier jugador por un precio, con /tratar.",
                             MEDICOS_JOB_X, MEDICOS_JOB_Y, MEDICOS_JOB_Z, 0,   MUNDO_HOSPITAL_INTERIOR,   0,  0x4FD1C5FF,  0,
                             "Medicos Los Santos\n{4FD1C5}/aceptarjob",
                             "{E8B33A}Comandos\n\
                             {FFFFFF}/aceptarjob{B4B5B7} - te haces miembro\n\
                             {FFFFFF}/tratar{B4B5B7} id precio - propon curar a alguien (1.000$-2.000$)\n\n\
                             {B4B5B7}El paciente acepta con {FFFFFF}/aceptarcura{B4B5B7}: sube a vida completa y paga el precio."},

    {"Trabajo libre 6",      false,  "Ranura libre. Ver instrucciones en 11_trabajos.inc.",
                             1481.0, -1771.0, 18.8,                       0,   0,   1,  0xF2C94CFF,  0, "", ""}
};

// ---------------------------------------------------------------------
//  RANGOS DE CADA TRABAJO (del 1 al 6; el 0 no se usa)
// ---------------------------------------------------------------------
static const g_RangosTrabajo[MAX_TRABAJOS + 1][RANGOS_TRABAJO][24] =
{
    {"", "", "", "", "", ""},
    // Vendedor de Armas: aqui solo se muestran 6 nombres de referencia;
    // el nivel real (0-1200+) lo gestiona 25_vendedorarmas.inc, igual que
    // en NGRP, y decide que armas puedes fabricar.
    {"Aprendiz", "Traficante", "Proveedor", "Armero", "Contactado", "Veterano"},
    {"Novato", "Recogecoches", "Tasador", "Vendedor", "Jefe de parking", "Veterano"},
    {"Camello novato", "Repartidor", "Trapicheo fijo", "Distribuidor", "Proveedor de barrio", "Capo de la calle"},
    {"Auxiliar", "Enfermero", "Paramedico", "Medico de guardia", "Medico adjunto", "Jefe de urgencias"},
    {"Aprendiz", "Operario", "Oficial", "Encargado", "Jefe de equipo", "Veterano"},
    {"Aprendiz", "Operario", "Oficial", "Encargado", "Jefe de equipo", "Veterano"}
};

static STREAMER_TAG_PICKUP:g_PickupTrabajo[MAX_TRABAJOS + 1];
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaTrabajo[MAX_TRABAJOS + 1];

// ---------------------------------------------------------------------
//  MARCADORES EN EL MAPA
// ---------------------------------------------------------------------
stock Trabajo_CrearMarcadores()
{
    new activos = 0, etiqueta[192];

    for (new t = 1; t <= MAX_TRABAJOS; t++)
    {
        if (!g_Trabajos[t][trActivo]) continue;

        g_PickupTrabajo[t] = CreateDynamicPickup(1239, 23,
            g_Trabajos[t][trX], g_Trabajos[t][trY], g_Trabajos[t][trZ],
            g_Trabajos[t][trMundo], g_Trabajos[t][trInterior], -1, 100.0);

        // Sin blip en el minimapa a proposito: para llegar aqui se usa
        // /gps, no el radar (ver 30_gps.inc).

        // Cartel: usa el texto personalizado del trabajo si lo tiene,
        // si no, el generico de siempre.
        if (g_Trabajos[t][trTextoCartel][0] != EOS)
        {
            format(etiqueta, sizeof etiqueta, "%s", g_Trabajos[t][trTextoCartel]);
        }
        else
        {
            format(etiqueta, sizeof etiqueta, "%s\n{FFFFFF}Escribe {E8B33A}/trabajo{FFFFFF} para presentar tu solicitud",
                g_Trabajos[t][trNombre]);
        }

        g_EtiquetaTrabajo[t] = CreateDynamic3DTextLabel(etiqueta, g_Trabajos[t][trColor],
            g_Trabajos[t][trX], g_Trabajos[t][trY], g_Trabajos[t][trZ] + 0.6, 12.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0,
            g_Trabajos[t][trMundo], g_Trabajos[t][trInterior], -1, 30.0);

        activos++;
    }

    printf("  [TRABAJOS] %d de %d ranuras activas.", activos, MAX_TRABAJOS);
    return activos;
}

// ---------------------------------------------------------------------
//  CONSULTAS
// ---------------------------------------------------------------------
stock bool:TrabajoValido(trabajo)
{
    return (trabajo > 0 && trabajo <= MAX_TRABAJOS);
}

public Trabajo_Nombre(trabajo, destino[], tam)
{
    if (!TrabajoValido(trabajo)) { format(destino, tam, "Ninguno"); return 0; }
    format(destino, tam, "%s", g_Trabajos[trabajo][trNombre]);
    return 1;
}

public Trabajo_NombreRango(trabajo, rango, destino[], tam)
{
    if (!TrabajoValido(trabajo) || rango < 1 || rango > RANGOS_TRABAJO)
    {
        format(destino, tam, "Sin rango");
        return 0;
    }
    format(destino, tam, "%s", g_RangosTrabajo[trabajo][rango - 1]);
    return 1;
}

public bool:Trabajo_Tiene(playerid, trabajo)
{
    for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
    {
        if (Jugador[playerid][jTrabajo][i] == trabajo && trabajo != 0) return true;
    }
    return false;
}

public Trabajo_Contar(playerid)
{
    new total = 0;
    for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
    {
        if (TrabajoValido(Jugador[playerid][jTrabajo][i])) total++;
    }
    return total;
}

// Devuelve el rango del jugador en ese trabajo, o 0 si no lo tiene.
stock Trabajo_RangoDe(playerid, trabajo)
{
    for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
    {
        if (Jugador[playerid][jTrabajo][i] == trabajo) return Jugador[playerid][jTrabajoRango][i];
    }
    return 0;
}

// ---------------------------------------------------------------------
//  CONTRATAR Y DESPEDIR
//  Devuelve  1 correcto · 0 ya lo tiene · -1 sin plazas libres
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
//  UNIFORMES DE TRABAJO
//  ---------------------------------------------------------------------
//  Un skin por ranura de trabajo; 0 = ese trabajo no tiene uniforme.
//  Solo la NFS lleva, y hasta hoy se lo ponia ella misma en
//  NFS_TrasContratar sin que nadie lo quitara al dejar el trabajo.
//
//  Quien decide el aspecto es Skin_Actualizar() (03_utiles.inc); aqui
//  solo esta el dato. Un uniforme de faccion gana a uno de trabajo.
// ---------------------------------------------------------------------
static const g_SkinsTrabajo[MAX_TRABAJOS + 1] =
{
    0,          // (sin trabajo)
    0,          // 1 Vendedor de Armas    -- de calle, es un trapicheo
    NFS_SKIN,   // 2 NFS                  -- mono de taller
    0,          // 3 Vendedor de Marihuana-- de calle, por lo mismo
    0,          // 4 (ranura libre)
    0,          // 5 Medicos Los Santos   -- pendiente, ver LEEME
    0           // 6 (ranura libre)
};

// Skin de ese trabajo, o 0 si no tiene uniforme.
public Trabajo_SkinDe(trabajo)
{
    if (!TrabajoValido(trabajo)) return 0;
    return g_SkinsTrabajo[trabajo];
}

stock Trabajo_Dar(playerid, trabajo, rango)
{
    if (!TrabajoValido(trabajo)) return 0;
    if (Trabajo_Tiene(playerid, trabajo)) return 0;

    if (rango < 1) rango = 1;
    if (rango > RANGOS_TRABAJO) rango = RANGOS_TRABAJO;

    for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
    {
        if (TrabajoValido(Jugador[playerid][jTrabajo][i])) continue;

        Jugador[playerid][jTrabajo][i]      = trabajo;
        Jugador[playerid][jTrabajoRango][i] = rango;

        // Los vehiculos del trabajo se le abren al instante.
        Veh_ActualizarCierreJugador(playerid);

        // Y el uniforme, si ese trabajo tiene.
        Skin_Actualizar(playerid);
        return 1;
    }
    return -1;
}

stock Trabajo_Quitar(playerid, trabajo)
{
    for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
    {
        if (Jugador[playerid][jTrabajo][i] != trabajo) continue;

        Jugador[playerid][jTrabajo][i]      = 0;
        Jugador[playerid][jTrabajoRango][i] = 0;

        // El modulo del trabajo tiene que enterarse para cerrar el turno:
        // si no, el jugador se queda cobrando carreras de un trabajo que
        // ya no tiene.
        switch (trabajo)
        {
            case TRABAJO_VENDEDOR_ARMAS:      VendedorArmas_TrabajoRetirado(playerid);
            case TRABAJO_NFS:                 NFS_TrabajoRetirado(playerid);
            case TRABAJO_VENDEDOR_MARIHUANA:  MariaVendedor_TrabajoRetirado(playerid);
            case TRABAJO_MEDICOS:             Medicos_TrabajoRetirado(playerid);
        }

        // Y los vehiculos del trabajo se le cierran.
        Veh_ActualizarCierreJugador(playerid);

        // El uniforme se devuelve con el puesto: si no le queda otro,
        // vuelve a su aspecto de siempre.
        Skin_Actualizar(playerid);
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------
//  COMANDOS
// ---------------------------------------------------------------------
// Vive dentro de /ayuda > Trabajos (09_cuenta.inc, Ayuda_MostrarSeccion
// caso 2) -- no hay comando /trabajos suelto, todo pasa por el menu de
// ayuda a proposito.
stock Trabajo_MostrarLista(playerid)
{
    new lista[2048], linea[256], activos = 0, nombre[32];

    format(lista, sizeof lista, "{E8B33A}Trabajo\tRequisito\tEstado\n");

    for (new t = 1; t <= MAX_TRABAJOS; t++)
    {
        if (!g_Trabajos[t][trActivo]) continue;

        new estado[48];
        if (Trabajo_Tiene(playerid, t))
        {
            Trabajo_NombreRango(t, Trabajo_RangoDe(playerid, t), nombre, sizeof nombre);
            format(estado, sizeof estado, "{53C168}%s", nombre);
        }
        else if (Jugador[playerid][jNivel] < g_Trabajos[t][trNivelMinimo])
        {
            format(estado, sizeof estado, "{E05A5A}Bloqueado");
        }
        else format(estado, sizeof estado, "{B4B5B7}Disponible");

        format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}Nivel %d\t%s\n",
            g_Trabajos[t][trNombre], g_Trabajos[t][trNivelMinimo], estado);

        strcat(lista, linea, sizeof lista);
        activos++;
    }

    if (activos == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_NADA, DIALOG_STYLE_MSGBOX,
            "{E8B33A}Trabajos",
            "{B4B5B7}Todavia no hay ningun trabajo abierto en la ciudad.\n\n\
            El equipo del servidor los ira anunciando conforme se vayan\n\
            poniendo en marcha. Mientras tanto cobras la nomina de\n\
            desempleado cada hora de juego.",
            "Cerrar", "");
        return 1;
    }

    ShowPlayerDialog(playerid, DIALOGO_TRABAJOS, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8B33A}Trabajos de Los Santos", lista, "Ver ficha", "Cerrar");
    return 1;
}

CMD:trabajo(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (VaConduciendo(playerid)) return 1;

    new encontrado = 0;

    for (new t = 1; t <= MAX_TRABAJOS; t++)
    {
        if (!g_Trabajos[t][trActivo]) continue;
        if (GetPlayerInterior(playerid) != g_Trabajos[t][trInterior]) continue;
        if (GetPlayerVirtualWorld(playerid) != g_Trabajos[t][trMundo]) continue;
        if (!EnPunto(playerid, 3.0, g_Trabajos[t][trX], g_Trabajos[t][trY], g_Trabajos[t][trZ])) continue;

        encontrado = t;
        break;
    }

    if (encontrado == 0)
    {
        Error(playerid, "No estas en ningun punto de contratacion. Mira /ayuda > Trabajos.");
        return 1;
    }

    if (Trabajo_Tiene(playerid, encontrado))
    {
        Error(playerid, "Ya trabajas aqui.");
        return 1;
    }

    if (Jugador[playerid][jNivel] < g_Trabajos[encontrado][trNivelMinimo])
    {
        format(g_Buffer, sizeof g_Buffer, "Necesitas nivel %d para este trabajo.", g_Trabajos[encontrado][trNivelMinimo]);
        Error(playerid, g_Buffer);
        return 1;
    }

    Jugador[playerid][jTemporal] = encontrado;

    // Ya tienes el maximo: en vez de bloquear, se ofrece reemplazar el
    // segundo trabajo por este, con confirmacion explicita.
    if (Trabajo_Contar(playerid) >= MAX_TRABAJOS_JUGADOR)
    {
        new segundo = Jugador[playerid][jTrabajo][1];
        new nombreSegundo[32];
        Trabajo_Nombre(segundo, nombreSegundo, sizeof nombreSegundo);

        format(g_Buffer, sizeof g_Buffer,
            "{E8B33A}%s\n\n{B4B5B7}Ya tienes el maximo de %d trabajos. Si aceptas este, dejaras\n\
            automaticamente {FFFFFF}%s{B4B5B7}.\n\n\
            Quieres continuar?",
            g_Trabajos[encontrado][trNombre], MAX_TRABAJOS_JUGADOR, nombreSegundo);

        ShowPlayerDialog(playerid, DIALOGO_TRABAJO_REEMPLAZAR, DIALOG_STYLE_MSGBOX,
            "{E8B33A}Cambiar de trabajo", g_Buffer, "Si, cambiar", "No");
        return 1;
    }

    new cuerpo[512];
    format(cuerpo, sizeof cuerpo,
        "{E8B33A}%s\n\n{B4B5B7}%s\n\n\
        {FFFFFF}Entrarias como {E8B33A}%s{FFFFFF}, el rango mas bajo.\n\n\
        Quieres firmar el contrato?",
        g_Trabajos[encontrado][trNombre],
        g_Trabajos[encontrado][trDescripcion],
        g_RangosTrabajo[encontrado][0]);

    ShowPlayerDialog(playerid, DIALOGO_TRABAJO_ACEPTAR, DIALOG_STYLE_MSGBOX,
        "{E8B33A}Contrato de trabajo", cuerpo, "Firmar", "No, gracias");
    return 1;
}

CMD:mistrabajos(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (Trabajo_Contar(playerid) == 0)
    {
        Error(playerid, "No tienes ningun trabajo. Mira la lista en /ayuda > Trabajos.");
        return 1;
    }

    new nombre[32], rango[24];

    Linea(playerid, COLOR_MARCA, "Tus trabajos:");
    for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
    {
        new t = Jugador[playerid][jTrabajo][i];
        if (!TrabajoValido(t)) continue;

        Trabajo_Nombre(t, nombre, sizeof nombre);
        Trabajo_NombreRango(t, Jugador[playerid][jTrabajoRango][i], rango, sizeof rango);

        format(g_Buffer, sizeof g_Buffer, "  {FFFFFF}%s{B4B5B7} - rango %d de %d ({FFFFFF}%s{B4B5B7})",
            nombre, Jugador[playerid][jTrabajoRango][i], RANGOS_TRABAJO, rango);
        SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
    }
    return 1;
}

CMD:dejartrabajo(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (Trabajo_Contar(playerid) == 0)
    {
        Error(playerid, "No tienes ningun trabajo que dejar.");
        return 1;
    }

    new lista[256], nombre[32];
    lista[0] = EOS;

    for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
    {
        new t = Jugador[playerid][jTrabajo][i];
        if (!TrabajoValido(t)) continue;

        Trabajo_Nombre(t, nombre, sizeof nombre);
        format(lista, sizeof lista, "%s%s%s", lista, (lista[0] != EOS) ? ("\n") : (""), nombre);
    }

    ShowPlayerDialog(playerid, DIALOGO_TRABAJO_DEJAR, DIALOG_STYLE_LIST,
        "{E8B33A}Dejar un trabajo", lista, "Renunciar", "Cancelar");
    return 1;
}

// ---------------------------------------------------------------------
//  DIÁLOGOS
// ---------------------------------------------------------------------
stock Trabajo_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused inputtext

    switch (dialogid)
    {
        case DIALOGO_TRABAJOS:
        {
            if (!response) return 1;

            // Traduce la fila elegida al identificador real del trabajo.
            new contador = 0, elegido = 0;
            for (new t = 1; t <= MAX_TRABAJOS; t++)
            {
                if (!g_Trabajos[t][trActivo]) continue;
                if (contador == listitem) { elegido = t; break; }
                contador++;
            }
            if (elegido == 0) return 1;

            new cuerpo[1100];
            format(cuerpo, sizeof cuerpo,
                "{E8B33A}%s\n\n{B4B5B7}%s\n\n\
                {E8B33A}Rangos\n{B4B5B7}1. %s\n2. %s\n3. %s\n4. %s\n5. %s\n6. %s\n\n\
                {B4B5B7}Nivel minimo: {FFFFFF}%d{B4B5B7}   Suplemento de nomina: {3D9970}%d$\n\n\
                %s\n\n\
                {B4B5B7}Acercate al punto de contratacion y escribe {FFFFFF}/trabajo{B4B5B7} (o {FFFFFF}/aceptarjob{B4B5B7}).",
                g_Trabajos[elegido][trNombre],
                g_Trabajos[elegido][trDescripcion],
                g_RangosTrabajo[elegido][0], g_RangosTrabajo[elegido][1], g_RangosTrabajo[elegido][2],
                g_RangosTrabajo[elegido][3], g_RangosTrabajo[elegido][4], g_RangosTrabajo[elegido][5],
                g_Trabajos[elegido][trNivelMinimo], g_Trabajos[elegido][trPagoExtra],
                g_Trabajos[elegido][trComandos]);

            ShowPlayerDialog(playerid, DIALOGO_NADA, DIALOG_STYLE_MSGBOX,
                "{E8B33A}Ficha del trabajo", cuerpo, "Cerrar", "");

            SetPlayerCheckpoint(playerid, g_Trabajos[elegido][trX], g_Trabajos[elegido][trY], g_Trabajos[elegido][trZ], 3.0);
            Info(playerid, "Te he marcado el punto de contratacion en el mapa.");
            return 1;
        }

        case DIALOGO_TRABAJO_ACEPTAR:
        {
            if (!response) return 1;

            new trabajo = Jugador[playerid][jTemporal];
            if (!TrabajoValido(trabajo)) return 1;

            // Se vuelve a comprobar todo: pudo cambiar mientras leía.
            if (Trabajo_Tiene(playerid, trabajo)) { Error(playerid, "Ya trabajas aqui."); return 1; }
            if (Trabajo_Contar(playerid) >= MAX_TRABAJOS_JUGADOR)
            {
                Error(playerid, "Ya tienes el maximo de trabajos.");
                return 1;
            }

            if (Trabajo_Dar(playerid, trabajo, 1) != 1)
            {
                Error(playerid, "No se ha podido firmar el contrato.");
                return 1;
            }

            // Los ganchos de "tras contratar" (p.ej. la skin de la NFS)
            // van ANTES de guardar: si no, ese cambio solo vive en
            // memoria hasta el siguiente guardado periodico y se pierde
            // si el servidor se reinicia antes.
            if (trabajo == TRABAJO_VENDEDOR_ARMAS)     VendedorArmas_TrasContratar(playerid);
            if (trabajo == TRABAJO_NFS)                NFS_TrasContratar(playerid);
            if (trabajo == TRABAJO_VENDEDOR_MARIHUANA) MariaVendedor_TrasContratar(playerid);
            if (trabajo == TRABAJO_MEDICOS)            Medicos_TrasContratar(playerid);
            BD_GuardarCuenta(playerid);

            new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
            NombreRP(playerid, nombre, sizeof nombre);

            format(mensaje, sizeof mensaje, "Has firmado como %s en %s.",
                g_RangosTrabajo[trabajo][0], g_Trabajos[trabajo][trNombre]);
            Exito(playerid, mensaje);

            format(mensaje, sizeof mensaje, "* %s firma unos papeles y se los devuelve al encargado.", nombre);
            MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
            return 1;
        }

        // Ya tenia el maximo de trabajos: confirmo que quiere dejar el
        // segundo y coger este nuevo en su lugar.
        case DIALOGO_TRABAJO_REEMPLAZAR:
        {
            if (!response) { Info(playerid, "No has cambiado de trabajo."); return 1; }

            new trabajo = Jugador[playerid][jTemporal];
            if (!TrabajoValido(trabajo)) return 1;

            if (Trabajo_Tiene(playerid, trabajo)) { Error(playerid, "Ya trabajas aqui."); return 1; }
            if (Trabajo_Contar(playerid) < MAX_TRABAJOS_JUGADOR) return 1; // ya no hacia falta reemplazar

            new segundo = Jugador[playerid][jTrabajo][1];
            new nombreViejo[32], nombreNuevo[32];
            Trabajo_Nombre(segundo, nombreViejo, sizeof nombreViejo);
            Trabajo_Nombre(trabajo, nombreNuevo, sizeof nombreNuevo);

            Trabajo_Quitar(playerid, segundo);

            if (Trabajo_Dar(playerid, trabajo, 1) != 1)
            {
                Error(playerid, "No se ha podido firmar el contrato.");
                return 1;
            }

            if (trabajo == TRABAJO_VENDEDOR_ARMAS)     VendedorArmas_TrasContratar(playerid);
            if (trabajo == TRABAJO_NFS)                NFS_TrasContratar(playerid);
            if (trabajo == TRABAJO_VENDEDOR_MARIHUANA) MariaVendedor_TrasContratar(playerid);
            if (trabajo == TRABAJO_MEDICOS)            Medicos_TrasContratar(playerid);
            BD_GuardarCuenta(playerid);

            new nombre[MAX_PLAYER_NAME + 1], mensaje[192];
            NombreRP(playerid, nombre, sizeof nombre);

            format(mensaje, sizeof mensaje, "Has dejado %s y has firmado como %s en %s.",
                nombreViejo, g_RangosTrabajo[trabajo][0], nombreNuevo);
            Exito(playerid, mensaje);

            format(mensaje, sizeof mensaje, "* %s firma unos papeles y se los devuelve al encargado.", nombre);
            MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
            return 1;
        }

        case DIALOGO_TRABAJO_DEJAR:
        {
            if (!response) return 1;

            new contador = 0, elegido = 0;
            for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
            {
                if (!TrabajoValido(Jugador[playerid][jTrabajo][i])) continue;
                if (contador == listitem) { elegido = Jugador[playerid][jTrabajo][i]; break; }
                contador++;
            }
            if (elegido == 0) return 1;

            new nombre[32];
            Trabajo_Nombre(elegido, nombre, sizeof nombre);
            Trabajo_Quitar(playerid, elegido);
            BD_GuardarCuenta(playerid);

            format(g_Buffer, sizeof g_Buffer, "Has dejado el trabajo de %s.", nombre);
            Exito(playerid, g_Buffer);
            return 1;
        }
    }
    return 0;
}
