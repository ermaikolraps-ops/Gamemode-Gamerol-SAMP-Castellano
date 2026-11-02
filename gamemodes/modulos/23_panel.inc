/* =====================================================================
 *  23_panel.inc
 *  ---------------------------------------------------------------------
 *  Panel de administracion por dialogos: /panel
 *
 *  No inventa moderacion nueva. Es una capa de menus por encima de los
 *  comandos que ya existen en 16_administracion.inc (cmd_ir, cmd_kick,
 *  cmd_banear...): arma los parametros de texto que esos comandos ya
 *  esperan y los llama directamente, asi que hereda sus mismos permisos,
 *  mensajes y registro en la tabla `sanciones` / `admin_logs`.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  ACCIONES DISPONIBLES SOBRE UN JUGADOR
// ---------------------------------------------------------------------
enum
{
    PACC_FICHA,
    PACC_IR,
    PACC_TRAER,
    PACC_ESPECTAR,
    PACC_CURAR,
    PACC_CONGELAR,
    PACC_AVISAR,
    PACC_MUTEAR,
    PACC_KICK,
    PACC_BANEAR,
    PACC_TRABAJO_DAR,
    PACC_TRABAJO_QUITAR,
    PACC_FACCION_DAR,
    PACC_FACCION_QUITAR,
    PACC_FACCION_LIDER,
    PACC_MAX
};

// Acciones sobre CUENTAS (buscadas por nombre, jugador conectado o no).
enum
{
    CACC_DINERO,
    CACC_NIVEL,
    CACC_BAN,
    CACC_DESBAN
};

static g_PanelObjetivo[MAX_PLAYERS];
static g_PanelAccion[MAX_PLAYERS];
static g_PanelValor[MAX_PLAYERS];
static g_PanelAccionMostrada[MAX_PLAYERS][PACC_MAX];
static g_PanelNumAcciones[MAX_PLAYERS];
static g_PanelListaValores[MAX_PLAYERS][8];
static g_PanelListaNum[MAX_PLAYERS];

// Estado de la seccion Cuentas.
static g_PanelCuentaID[MAX_PLAYERS];
static g_PanelCuentaNombre[MAX_PLAYERS][MAX_PLAYER_NAME + 1];
static g_PanelCuentaAccion[MAX_PLAYERS];
static g_PanelCuentaBaneada[MAX_PLAYERS];
static g_PanelCuentaResultID[MAX_PLAYERS][20];
static g_PanelCuentaResultNombre[MAX_PLAYERS][20][MAX_PLAYER_NAME + 1];
static g_PanelCuentaResultNum[MAX_PLAYERS];

// Estado de la seccion Coordenadas.
static g_PanelCoordSeleccionada[MAX_PLAYERS];

// Estado de la seccion Casas.
static g_PanelCasaSeleccionada[MAX_PLAYERS];

// Estado de la seccion Staff.
static g_PanelStaffObjetivo[MAX_PLAYERS];

// Estado de la seccion Vehiculos.
static g_PanelVehiculoSeleccionado[MAX_PLAYERS];

// ---------------------------------------------------------------------
//  COMANDO
// ---------------------------------------------------------------------
CMD:panel(playerid, params[])
{
    #pragma unused params
    if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

    Panel_AbrirPrincipal(playerid);
    return 1;
}

stock Panel_AbrirPrincipal(playerid)
{
    ShowPlayerDialog(playerid, DIALOGO_PANEL, DIALOG_STYLE_LIST,
        "{E8973A}Panel de administracion",
        "Jugadores\nReportes\nSanciones\nCuentas\nCoordenadas\nCasas\nVehiculos\nStaff\n{4FD1C5}Plantas: cosechar todas\n", "Elegir", "Cerrar");
}

// ---------------------------------------------------------------------
//  SECCION: JUGADORES
// ---------------------------------------------------------------------
stock Panel_JugadorFila(fila)
{
    new indice = 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (indice == fila) return i;
        indice++;
    }
    return INVALID_PLAYER_ID;
}

stock Panel_AbrirJugadores(playerid)
{
    new lista[2500], linea[128], nombre[MAX_PLAYER_NAME + 1], rango[32], estado[16];

    format(lista, sizeof lista, "{E8B33A}ID\tNombre\tRango\tEstado\n");

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;

        NombreRP(i, nombre, sizeof nombre);
        Admin_NombreNivel(Jugador[i][jAdmin], rango, sizeof rango);

        if (Jugador[i][jCarcelSegundos] > 0)      format(estado, sizeof estado, "Carcel");
        else if (Jugador[i][jMuteSegundos] > 0)   format(estado, sizeof estado, "Muteado");
        else if (Jugador[i][jCongelado])          format(estado, sizeof estado, "Congelado");
        else if (Jugador[i][jAFK])                format(estado, sizeof estado, "AFK");
        else                                       format(estado, sizeof estado, "En juego");

        format(linea, sizeof linea, "{FFFFFF}%d\t%s\t{B4B5B7}%s\t%s\n", i, nombre, rango, estado);

        if (strlen(lista) + strlen(linea) >= sizeof(lista) - 8) break;
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_JUGADORES, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8973A}Jugadores conectados", lista, "Elegir", "Volver");
}

stock Panel_AccionMinNivel(accion)
{
    switch (accion)
    {
        case PACC_FICHA:    return ADMIN_SOPORTE;
        case PACC_IR:       return ADMIN_SOPORTE;
        case PACC_TRAER:    return ADMIN_MODERADOR;
        case PACC_ESPECTAR: return ADMIN_MODERADOR;
        case PACC_CURAR:    return ADMIN_ADMIN;
        case PACC_CONGELAR: return ADMIN_MODERADOR;
        case PACC_AVISAR:   return ADMIN_MODERADOR;
        case PACC_MUTEAR:   return ADMIN_MODERADOR;
        case PACC_KICK:     return ADMIN_MODERADOR;
        case PACC_BANEAR:   return ADMIN_ADMIN;
        case PACC_TRABAJO_DAR:     return ADMIN_JEFE;
        case PACC_TRABAJO_QUITAR:  return ADMIN_JEFE;
        case PACC_FACCION_DAR:     return ADMIN_JEFE;
        case PACC_FACCION_QUITAR:  return ADMIN_JEFE;
        case PACC_FACCION_LIDER:   return ADMIN_GESTION;
    }
    return ADMIN_FUNDADOR;
}

stock Panel_EtiquetaAccion(objetivo, accion, destino[], tam)
{
    switch (accion)
    {
        case PACC_FICHA:    format(destino, tam, "Ver ficha");
        case PACC_IR:       format(destino, tam, "Ir hasta el");
        case PACC_TRAER:    format(destino, tam, "Traer hasta mi");
        case PACC_ESPECTAR: format(destino, tam, "Observar (espectar)");
        case PACC_CURAR:    format(destino, tam, "Curar");
        case PACC_CONGELAR: format(destino, tam, Jugador[objetivo][jCongelado] ? "Descongelar" : "Congelar");
        case PACC_AVISAR:   format(destino, tam, "Avisar (%d de 3)", Jugador[objetivo][jAvisos]);
        case PACC_MUTEAR:   format(destino, tam, Jugador[objetivo][jMuteSegundos] > 0 ? "Quitar silencio" : "Silenciar (mutear)");
        case PACC_KICK:     format(destino, tam, "Expulsar (kick)");
        case PACC_BANEAR:   format(destino, tam, "Banear");
        case PACC_TRABAJO_DAR:     format(destino, tam, "Asignar trabajo");
        case PACC_TRABAJO_QUITAR:  format(destino, tam, "Quitar un trabajo");
        case PACC_FACCION_DAR:     format(destino, tam, "Meter en una faccion");
        case PACC_FACCION_QUITAR:  format(destino, tam, "Sacar de la faccion");
        case PACC_FACCION_LIDER:   format(destino, tam, "Nombrar lider de faccion");
        default:            format(destino, tam, "?");
    }
}

stock bool:Panel_JugadorTieneTrabajo(objetivo)
{
    return (Jugador[objetivo][jTrabajo][0] != 0 || Jugador[objetivo][jTrabajo][1] != 0);
}

stock Panel_AbrirAccionesJugador(playerid, objetivo)
{
    if (!IsPlayerConnected(objetivo) || !Jugador[objetivo][jLogueado])
    {
        Error(playerid, "Ese jugador ya no esta conectado.");
        Panel_AbrirJugadores(playerid);
        return;
    }

    g_PanelObjetivo[playerid] = objetivo;

    new nivel = Jugador[playerid][jAdmin];
    new nombre[MAX_PLAYER_NAME + 1], lista[512], etiqueta[40], titulo[64];
    NombreRP(objetivo, nombre, sizeof nombre);

    g_PanelNumAcciones[playerid] = 0;
    lista[0] = EOS;

    for (new accion = 0; accion < PACC_MAX; accion++)
    {
        if (nivel < Panel_AccionMinNivel(accion)) continue;
        if (objetivo == playerid && (accion == PACC_IR || accion == PACC_TRAER || accion == PACC_ESPECTAR)) continue;
        if (accion == PACC_TRABAJO_QUITAR && !Panel_JugadorTieneTrabajo(objetivo)) continue;
        if ((accion == PACC_FACCION_QUITAR || accion == PACC_FACCION_LIDER) && Jugador[objetivo][jFaccion] == 0) continue;

        Panel_EtiquetaAccion(objetivo, accion, etiqueta, sizeof etiqueta);
        strcat(lista, etiqueta, sizeof lista);
        strcat(lista, "\n", sizeof lista);

        g_PanelAccionMostrada[playerid][g_PanelNumAcciones[playerid]] = accion;
        g_PanelNumAcciones[playerid]++;
    }

    format(titulo, sizeof titulo, "{E8973A}%s{FFFFFF}  (ID %d)", nombre, objetivo);
    ShowPlayerDialog(playerid, DIALOGO_PANEL_JUGADOR_ACCIONES, DIALOG_STYLE_LIST, titulo, lista, "Elegir", "Volver");
}

stock Panel_PedirDuracion(playerid, const texto[])
{
    ShowPlayerDialog(playerid, DIALOGO_PANEL_JUGADOR_DURACION, DIALOG_STYLE_INPUT,
        "{E8973A}Panel de administracion", texto, "Continuar", "Cancelar");
}

stock Panel_PedirMotivo(playerid, const texto[])
{
    ShowPlayerDialog(playerid, DIALOGO_PANEL_JUGADOR_MOTIVO, DIALOG_STYLE_INPUT,
        "{E8973A}Panel de administracion", texto, "Confirmar", "Cancelar");
}

stock Panel_EjecutarAccion(playerid, objetivo, accion)
{
    new buf[16];
    format(buf, sizeof buf, "%d", objetivo);

    switch (accion)
    {
        case PACC_FICHA:    cmd_revisar(playerid, buf);
        case PACC_IR:       cmd_ir(playerid, buf);
        case PACC_TRAER:    cmd_traer(playerid, buf);
        case PACC_ESPECTAR: cmd_espectar(playerid, buf);
        case PACC_CURAR:    cmd_curar(playerid, buf);

        case PACC_CONGELAR:
        {
            if (Jugador[objetivo][jCongelado]) cmd_descongelar(playerid, buf);
            else cmd_congelar(playerid, buf);
        }

        case PACC_KICK:
        {
            g_PanelAccion[playerid] = accion;
            Panel_PedirMotivo(playerid, "Motivo de la expulsion:");
        }

        case PACC_AVISAR:
        {
            g_PanelAccion[playerid] = accion;
            Panel_PedirMotivo(playerid, "Motivo del aviso (al 3er aviso, ban automatico):");
        }

        case PACC_MUTEAR:
        {
            if (Jugador[objetivo][jMuteSegundos] > 0)
            {
                cmd_desmutear(playerid, buf);
            }
            else
            {
                g_PanelAccion[playerid] = accion;
                Panel_PedirDuracion(playerid, "Minutos de silencio (1 a 1440):");
            }
        }

        case PACC_BANEAR:
        {
            g_PanelAccion[playerid] = accion;
            Panel_PedirDuracion(playerid, "Dias de baneo (0 = permanente):");
        }

        case PACC_TRABAJO_DAR:
        {
            g_PanelAccion[playerid] = accion;
            Panel_AbrirListaTrabajos(playerid);
        }

        case PACC_TRABAJO_QUITAR:
        {
            g_PanelAccion[playerid] = accion;
            Panel_AbrirListaTrabajosDelJugador(playerid, objetivo);
        }

        case PACC_FACCION_DAR:
        {
            g_PanelAccion[playerid] = accion;
            Panel_AbrirListaFacciones(playerid);
        }

        case PACC_FACCION_QUITAR: cmd_quitarfaccion(playerid, buf);
        case PACC_FACCION_LIDER:  cmd_liderfaccion(playerid, buf);
    }
}

// ---------------------------------------------------------------------
//  TRABAJOS Y FACCIONES  (listas de eleccion)
//  g_Trabajos[] y g_Facciones[] no son "static": se declaran en
//  11_trabajos.inc / 12_facciones.inc y se leen aqui tal cual.
// ---------------------------------------------------------------------
stock Panel_AbrirListaTrabajos(playerid)
{
    new lista[256];
    g_PanelListaNum[playerid] = 0;
    lista[0] = EOS;

    for (new t = 1; t <= MAX_TRABAJOS; t++)
    {
        if (!g_Trabajos[t][trActivo]) continue;
        strcat(lista, g_Trabajos[t][trNombre], sizeof lista);
        strcat(lista, "\n", sizeof lista);
        g_PanelListaValores[playerid][g_PanelListaNum[playerid]] = t;
        g_PanelListaNum[playerid]++;
    }

    if (g_PanelListaNum[playerid] == 0)
    {
        Error(playerid, "No hay ningun trabajo activo todavia.");
        Panel_AbrirAccionesJugador(playerid, g_PanelObjetivo[playerid]);
        return;
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_JUGADOR_LISTA, DIALOG_STYLE_LIST,
        "{E8973A}Elige un trabajo", lista, "Elegir", "Cancelar");
}

stock Panel_AbrirListaTrabajosDelJugador(playerid, objetivo)
{
    new lista[128], nombre[32];
    g_PanelListaNum[playerid] = 0;
    lista[0] = EOS;

    for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
    {
        new trabajo = Jugador[objetivo][jTrabajo][i];
        if (trabajo == 0) continue;

        Trabajo_Nombre(trabajo, nombre, sizeof nombre);
        strcat(lista, nombre, sizeof lista);
        strcat(lista, "\n", sizeof lista);
        g_PanelListaValores[playerid][g_PanelListaNum[playerid]] = trabajo;
        g_PanelListaNum[playerid]++;
    }

    if (g_PanelListaNum[playerid] == 0)
    {
        Error(playerid, "Ese jugador no tiene ningun trabajo.");
        Panel_AbrirAccionesJugador(playerid, objetivo);
        return;
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_JUGADOR_LISTA, DIALOG_STYLE_LIST,
        "{E8973A}Quitar cual trabajo", lista, "Elegir", "Cancelar");
}

stock Panel_AbrirListaFacciones(playerid)
{
    new lista[256];
    g_PanelListaNum[playerid] = 0;
    lista[0] = EOS;

    for (new f = 1; f <= MAX_FACCIONES; f++)
    {
        if (!g_Facciones[f][faActiva]) continue;
        strcat(lista, g_Facciones[f][faNombre], sizeof lista);
        strcat(lista, "\n", sizeof lista);
        g_PanelListaValores[playerid][g_PanelListaNum[playerid]] = f;
        g_PanelListaNum[playerid]++;
    }

    if (g_PanelListaNum[playerid] == 0)
    {
        Error(playerid, "No hay ninguna faccion activa todavia.");
        Panel_AbrirAccionesJugador(playerid, g_PanelObjetivo[playerid]);
        return;
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_JUGADOR_LISTA, DIALOG_STYLE_LIST,
        "{E8973A}Elige una faccion", lista, "Elegir", "Cancelar");
}

// ---------------------------------------------------------------------
//  SECCION: REPORTES
//  La lista y su acceso por fila viven en 16_administracion.inc, unico
//  duenio del array (privado) de reportes.
// ---------------------------------------------------------------------
stock Panel_AbrirReportes(playerid)
{
    new lista[1200];
    new abiertos = Reportes_ConstruirLista(lista, sizeof lista);

    if (abiertos == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_PANEL_REPORTES, DIALOG_STYLE_MSGBOX,
            "{E8973A}Reportes", "{B4B5B7}No hay ningun reporte pendiente.", "Volver", "");
        return;
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_REPORTES, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8973A}Reportes pendientes", lista, "Atender", "Volver");
}

// ---------------------------------------------------------------------
//  SECCION: SANCIONES  (historial de la tabla `sanciones`)
// ---------------------------------------------------------------------
stock Panel_AbrirSanciones(playerid)
{
    mysql_pquery(g_BD,
        "SELECT nombre, tipo, admin, motivo, DATE_FORMAT(fecha, '%d/%m %H:%i') AS cuando \
         FROM sanciones ORDER BY id DESC LIMIT 40", "Panel_AlCargarSanciones", "d", playerid);
}

public Panel_AlCargarSanciones(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_PANEL_SANCIONES, DIALOG_STYLE_MSGBOX,
            "{E8973A}Sanciones", "{B4B5B7}Todavia no hay ninguna sancion registrada.", "Volver", "");
        return 1;
    }

    new lista[3000], linea[220], nombre[24], admin[24], motivo[128], cuando[16], tipo, tipoTxt[10];

    format(lista, sizeof lista, "{E8B33A}Fecha\tTipo\tJugador\tAdmin\tMotivo\n");

    for (new i = 0; i < filas; i++)
    {
        cache_get_value_name(i, "nombre", nombre, sizeof nombre);
        cache_get_value_name(i, "admin", admin, sizeof admin);
        cache_get_value_name(i, "motivo", motivo, sizeof motivo);
        cache_get_value_name(i, "cuando", cuando, sizeof cuando);
        cache_get_value_name_int(i, "tipo", tipo);

        SubrayadoAEspacio(nombre);
        SubrayadoAEspacio(admin);

        switch (tipo)
        {
            case SANCION_AVISO:  format(tipoTxt, sizeof tipoTxt, "Aviso");
            case SANCION_KICK:   format(tipoTxt, sizeof tipoTxt, "Kick");
            case SANCION_MUTE:   format(tipoTxt, sizeof tipoTxt, "Mute");
            case SANCION_CARCEL: format(tipoTxt, sizeof tipoTxt, "Carcel");
            case SANCION_BAN:    format(tipoTxt, sizeof tipoTxt, "Ban");
            default:             format(tipoTxt, sizeof tipoTxt, "?");
        }

        format(linea, sizeof linea, "{B4B5B7}%s\t{E8973A}%s\t{FFFFFF}%s\t{B4B5B7}%s\t%s\n",
            cuando, tipoTxt, nombre, admin, motivo);

        if (strlen(lista) + strlen(linea) >= sizeof(lista) - 8) break;
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_SANCIONES, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8973A}Ultimas sanciones", lista, "Volver", "");
    return 1;
}

// ---------------------------------------------------------------------
//  SECCION: CUENTAS
//  Busca por nombre en la tabla `cuentas`. Funciona con el jugador
//  conectado o no: si esta conectado, reutiliza los mismos comandos que
//  la seccion Jugadores (mantiene sincronizados memoria y BD); si no,
//  actualiza la fila directamente con los helpers de 16_administracion.inc.
// ---------------------------------------------------------------------
stock Panel_PedirBusquedaCuenta(playerid)
{
    ShowPlayerDialog(playerid, DIALOGO_PANEL_CUENTA_BUSCAR, DIALOG_STYLE_INPUT,
        "{E8973A}Buscar cuenta", "Nombre completo o parte de el:", "Buscar", "Cancelar");
}

public Panel_AlBuscarCuentas(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_PANEL_CUENTA_RESULTADOS, DIALOG_STYLE_MSGBOX,
            "{E8973A}Cuentas", "{B4B5B7}No hay ninguna cuenta con ese nombre.", "Volver", "");
        return 1;
    }

    if (filas > 20) filas = 20;
    g_PanelCuentaResultNum[playerid] = filas;

    new lista[1200], linea[64], nombre[MAX_PLAYER_NAME + 1], nivel, admin, baneado;
    format(lista, sizeof lista, "{E8B33A}Nombre\tNivel\tAdmin\tBaneada\n");

    for (new i = 0; i < filas; i++)
    {
        cache_get_value_name(i, "nombre", nombre, sizeof nombre);
        cache_get_value_name_int(i, "nivel", nivel);
        cache_get_value_name_int(i, "admin", admin);
        cache_get_value_name_int(i, "baneado", baneado);
        cache_get_value_name_int(i, "id", g_PanelCuentaResultID[playerid][i]);
        format(g_PanelCuentaResultNombre[playerid][i], MAX_PLAYER_NAME + 1, "%s", nombre);

        format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%d\t%d\t%s\n",
            nombre, nivel, admin, baneado ? ("{E85A3A}si") : ("no"));
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_CUENTA_RESULTADOS, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8973A}Resultados", lista, "Elegir", "Volver");
    return 1;
}

stock Panel_AbrirFichaCuenta(playerid, cuentaid)
{
    mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
        "SELECT nombre, nivel, admin, dinero, banco, avisos, baneado, ban_motivo, ban_admin, \
         DATE_FORMAT(ban_expira, '%%d/%%m/%%Y') AS expira, minutos_totales FROM cuentas WHERE id = %d",
        cuentaid);
    mysql_pquery(g_BD, g_Buffer, "Panel_AlCargarFichaCuenta", "dd", playerid, cuentaid);
}

public Panel_AlCargarFichaCuenta(playerid, cuentaid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);
    if (filas == 0)
    {
        Error(playerid, "Esa cuenta ya no existe.");
        Panel_AbrirPrincipal(playerid);
        return 1;
    }

    new nombre[MAX_PLAYER_NAME + 1], nivel, admin, dinero, banco, avisos, baneado,
        motivo[128], banAdmin[24], expira[16], minutos;

    cache_get_value_name(0, "nombre", nombre, sizeof nombre);
    cache_get_value_name_int(0, "nivel", nivel);
    cache_get_value_name_int(0, "admin", admin);
    cache_get_value_name_int(0, "dinero", dinero);
    cache_get_value_name_int(0, "banco", banco);
    cache_get_value_name_int(0, "avisos", avisos);
    cache_get_value_name_int(0, "baneado", baneado);
    cache_get_value_name(0, "ban_motivo", motivo, sizeof motivo);
    cache_get_value_name(0, "ban_admin", banAdmin, sizeof banAdmin);
    cache_get_value_name(0, "expira", expira, sizeof expira);
    cache_get_value_name_int(0, "minutos_totales", minutos);

    g_PanelCuentaID[playerid] = cuentaid;
    format(g_PanelCuentaNombre[playerid], MAX_PLAYER_NAME + 1, "%s", nombre);
    g_PanelCuentaBaneada[playerid] = baneado;

    new tiempo[48];
    FormatearMinutos(minutos, tiempo, sizeof tiempo);

    new cuerpo[700], dineroTxt[24], bancoTxt[24];
    FormatearDinero(dinero, dineroTxt, sizeof dineroTxt);
    FormatearDinero(banco, bancoTxt, sizeof bancoTxt);

    format(cuerpo, sizeof cuerpo,
        "{E8B33A}%s{B4B5B7}   id de BD {FFFFFF}%d\n\n\
        {B4B5B7}Nivel        {FFFFFF}%d\n\
        {B4B5B7}Rango staff  {FFFFFF}%d\n\
        {B4B5B7}Efectivo     {3D9970}%s{B4B5B7}      Banco {3D9970}%s\n\
        {B4B5B7}Tiempo jugado {FFFFFF}%s\n\
        {B4B5B7}Avisos       {FFFFFF}%d{B4B5B7} de 3\n\n",
        nombre, cuentaid, nivel, admin, dineroTxt, bancoTxt, tiempo, avisos);

    if (baneado)
    {
        format(g_Buffer, sizeof g_Buffer,
            "{E85A3A}BANEADA{B4B5B7}   por %s\n{B4B5B7}Motivo: {FFFFFF}%s\n{B4B5B7}Expira: {FFFFFF}%s",
            banAdmin, motivo, strlen(expira) ? expira : "permanente");
        strcat(cuerpo, g_Buffer, sizeof cuerpo);
    }
    else
    {
        strcat(cuerpo, "{53C168}No esta baneada.", sizeof cuerpo);
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_CUENTA_FICHA, DIALOG_STYLE_MSGBOX,
        "{E8973A}Ficha de cuenta", cuerpo, "Acciones", "Volver");
    return 1;
}

stock Panel_AbrirAccionesCuenta(playerid)
{
    new nivel = Jugador[playerid][jAdmin];
    new lista[200];
    lista[0] = EOS;

    new acciones[4], num = 0;

    // Restringido a ADMIN_FUNDADOR (05/09/2026, peticion expresa de
    // Vlad): antes era ADMIN_JEFE, y Dani (nivel 5, ADMIN_GESTION) ya
    // veia este boton aunque no supiera el nombre de ningun comando de
    // texto -- el nombre del comando nunca fue el problema real. Igual
    // en DIALOGO_PANEL_CUENTA_VALOR mas abajo, que es quien de verdad
    // ejecuta el cambio.
    if (nivel >= ADMIN_FUNDADOR)
    {
        strcat(lista, "Dar / quitar dinero\n", sizeof lista);
        acciones[num++] = CACC_DINERO;
    }
    if (nivel >= ADMIN_GESTION)
    {
        strcat(lista, "Cambiar nivel de personaje\n", sizeof lista);
        acciones[num++] = CACC_NIVEL;
    }
    if (nivel >= ADMIN_ADMIN)
    {
        if (g_PanelCuentaBaneada[playerid])
        {
            strcat(lista, "Desbanear\n", sizeof lista);
            acciones[num++] = CACC_DESBAN;
        }
        else
        {
            strcat(lista, "Banear\n", sizeof lista);
            acciones[num++] = CACC_BAN;
        }
    }

    for (new i = 0; i < num; i++) g_PanelListaValores[playerid][i] = acciones[i];
    g_PanelListaNum[playerid] = num;

    if (num == 0)
    {
        Error(playerid, "No tienes permiso para actuar sobre cuentas.");
        Panel_AbrirPrincipal(playerid);
        return;
    }

    new titulo[64];
    format(titulo, sizeof titulo, "{E8973A}%s", g_PanelCuentaNombre[playerid]);
    ShowPlayerDialog(playerid, DIALOGO_PANEL_CUENTA_ACCIONES, DIALOG_STYLE_LIST, titulo, lista, "Elegir", "Volver");
}

stock Panel_PedirValorCuenta(playerid, const texto[])
{
    ShowPlayerDialog(playerid, DIALOGO_PANEL_CUENTA_VALOR, DIALOG_STYLE_INPUT,
        "{E8973A}Panel de administracion", texto, "Continuar", "Cancelar");
}

stock Panel_PedirMotivoCuenta(playerid, const texto[])
{
    ShowPlayerDialog(playerid, DIALOGO_PANEL_CUENTA_MOTIVO, DIALOG_STYLE_INPUT,
        "{E8973A}Panel de administracion", texto, "Confirmar", "Cancelar");
}

// Si la cuenta esta conectada ahora mismo, delega en los mismos cmd_ que
// usa la seccion Jugadores; si no, usa los helpers offline de
// 16_administracion.inc. Asi nunca se pisa el struct en memoria de un
// jugador que sigue conectado.
stock Panel_EjecutarAccionCuenta(playerid, accion)
{
    new cuentaid = g_PanelCuentaID[playerid];

    switch (accion)
    {
        case CACC_DINERO:
        {
            g_PanelCuentaAccion[playerid] = accion;
            Panel_PedirValorCuenta(playerid, "Cantidad (puede ser negativa):");
        }
        case CACC_NIVEL:
        {
            g_PanelCuentaAccion[playerid] = accion;
            Panel_PedirValorCuenta(playerid, "Nuevo nivel de personaje (1-100):");
        }
        case CACC_BAN:
        {
            g_PanelCuentaAccion[playerid] = accion;
            Panel_PedirValorCuenta(playerid, "Dias de baneo (0 = permanente):");
        }
        case CACC_DESBAN:
        {
            new buf[32];
            format(buf, sizeof buf, "%s", g_PanelCuentaNombre[playerid]);
            cmd_desbanear(playerid, buf);
            g_PanelCuentaBaneada[playerid] = 0;
            Panel_AbrirFichaCuenta(playerid, cuentaid);
        }
    }
}

// ---------------------------------------------------------------------
//  SECCION: COORDENADAS
//  g_Coords[] / g_TotalCoords viven en 16_administracion.inc (no static),
//  compartidos con /goto.
// ---------------------------------------------------------------------
stock Panel_AbrirCoords(playerid)
{
    new lista[600];
    format(lista, sizeof lista, "{53C168}+ Guardar mi posicion aqui\n");

    for (new i = 0; i < g_TotalCoords; i++)
    {
        format(g_Buffer, sizeof g_Buffer, "{FFFFFF}%s{B4B5B7} (%s)\n", g_Coords[i][goNombre], g_Coords[i][goClave]);
        strcat(lista, g_Buffer, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_COORDS, DIALOG_STYLE_LIST, "{E8973A}Coordenadas guardadas", lista, "Elegir", "Volver");
}

stock Panel_AbrirAccionesCoord(playerid, indice)
{
    g_PanelCoordSeleccionada[playerid] = indice;

    new titulo[64];
    format(titulo, sizeof titulo, "{E8973A}%s", g_Coords[indice][goNombre]);
    ShowPlayerDialog(playerid, DIALOGO_PANEL_COORD_ACCIONES, DIALOG_STYLE_LIST, titulo, "Ir\nBorrar\n", "Elegir", "Volver");
}

// ---------------------------------------------------------------------
//  SECCION: CASAS
//  g_Casas[] / g_TotalCasas viven en 24_casas.inc (no static).
// ---------------------------------------------------------------------
stock Panel_AbrirCasas(playerid)
{
    new lista[2500], linea[128], precio[24], estado[48];
    format(lista, sizeof lista, "{E8B33A}Casa\tPrecio\tEstado\n");

    for (new i = 0; i < g_TotalCasas; i++)
    {
        FormatearDinero(g_Casas[i][caPrecio], precio, sizeof precio);

        if (g_Casas[i][caPropietario] == 0) format(estado, sizeof estado, "{3D9970}Libre");
        else
        {
            new dueno[MAX_PLAYER_NAME + 1];
            format(dueno, sizeof dueno, "%s", g_Casas[i][caPropietarioNombre]);
            SubrayadoAEspacio(dueno);
            format(estado, sizeof estado, "{B4B5B7}%s", dueno);
        }

        format(linea, sizeof linea, "{FFFFFF}%s\t%s\t%s\n", g_Casas[i][caNombre], precio, estado);
        strcat(lista, linea, sizeof lista);
    }

    if (g_TotalCasas == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_PANEL_CASAS, DIALOG_STYLE_MSGBOX,
            "{E8973A}Casas", "{B4B5B7}Todavia no hay ninguna casa. Usa {FFFFFF}/crearcasa{B4B5B7} en el juego.", "Volver", "");
        return;
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_CASAS, DIALOG_STYLE_TABLIST_HEADERS, "{E8973A}Casas", lista, "Elegir", "Volver");
}

stock Panel_AbrirFichaCasa(playerid, indice)
{
    g_PanelCasaSeleccionada[playerid] = indice;

    new precio[24], cuerpo[400];
    FormatearDinero(g_Casas[indice][caPrecio], precio, sizeof precio);

    format(cuerpo, sizeof cuerpo,
        "{E8B33A}%s{B4B5B7}   id %d\n\n\
        {B4B5B7}Precio        {3D9970}%s\n\
        {B4B5B7}Interior      {FFFFFF}%d{B4B5B7}   Mundo {FFFFFF}%d\n\
        {B4B5B7}Cerrada       {FFFFFF}%s\n",
        g_Casas[indice][caNombre], g_Casas[indice][caID], precio,
        g_Casas[indice][caInteriorID], g_Casas[indice][caMundoVirtual],
        g_Casas[indice][caCerrada] ? "si" : "no");

    if (g_Casas[indice][caPropietario] == 0)
    {
        strcat(cuerpo, "{B4B5B7}Propietario   {3D9970}libre", sizeof cuerpo);
    }
    else
    {
        new dueno[MAX_PLAYER_NAME + 1];
        format(dueno, sizeof dueno, "%s", g_Casas[indice][caPropietarioNombre]);
        SubrayadoAEspacio(dueno);
        format(g_Buffer, sizeof g_Buffer, "{B4B5B7}Propietario   {FFFFFF}%s", dueno);
        strcat(cuerpo, g_Buffer, sizeof cuerpo);
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_CASA_FICHA, DIALOG_STYLE_MSGBOX, "{E8973A}Ficha de la casa", cuerpo, "Acciones", "Volver");
}

stock Panel_AbrirAccionesCasa(playerid, indice)
{
    new lista[200];
    format(lista, sizeof lista, "Ir a la puerta\nCambiar precio\nTransferir a un jugador conectado\n");

    if (g_Casas[indice][caPropietario] != 0) strcat(lista, "Liberar (quitar propietario)\n", sizeof lista);
    else strcat(lista, "Borrar casa\n", sizeof lista);

    new titulo[64];
    format(titulo, sizeof titulo, "{E8973A}%s", g_Casas[indice][caNombre]);
    ShowPlayerDialog(playerid, DIALOGO_PANEL_CASA_ACCIONES, DIALOG_STYLE_LIST, titulo, lista, "Elegir", "Volver");
}

// ---------------------------------------------------------------------
//  SECCION: VEHICULOS  (toda la flota persistente, g_Vehiculos[] vive en
//  13_vehiculos.inc). Solo se listan los LEGALIZADOS: personales o
//  libres de dropeo con propietario (con papeles), mas los de
//  trabajo/faccion/servidor (esos no tienen concepto de "libre").  Los
//  coches libres de la calle SIN comprar no salen aqui  son cientos y
//  no aportan nada que gestionar; para esos usa /borrardropeo o el
//  cartel flotante de cada uno.
// ---------------------------------------------------------------------
stock bool:Panel_VehiculoEsLegalizado(indice)
{
    if (g_Vehiculos[indice][veTipo] == VEH_PERSONAL || g_Vehiculos[indice][veTipo] == VEH_DROPEO)
    {
        return (g_Vehiculos[indice][vePropietario] != 0);
    }
    return true; // trabajo / faccion / servidor: siempre se listan
}

stock Panel_VehiculoFila(fila)
{
    new indice = 0;
    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (!Panel_VehiculoEsLegalizado(i)) continue;
        if (indice == fila) return i;
        indice++;
    }
    return -1;
}

stock Panel_VehiculoEstadoTxt(indice, destino[], tam)
{
    if (g_Vehiculos[indice][veTipo] == VEH_PERSONAL || g_Vehiculos[indice][veTipo] == VEH_DROPEO)
    {
        // Solo llegan aqui los legalizados (con propietario); ver
        // Panel_VehiculoEsLegalizado.
        format(destino, tam, "{FF3232}CON PROPIETARIO");
        return;
    }

    switch (g_Vehiculos[indice][veTipo])
    {
        case VEH_TRABAJO:  format(destino, tam, "{8BC4E8}Trabajo");
        case VEH_FACCION:  format(destino, tam, "{8BC4E8}Faccion");
        case VEH_SERVIDOR: format(destino, tam, "{8BC4E8}Servidor");
        default:            format(destino, tam, "?");
    }
}

// El buffer se quedo pensado para toda la flota (con los cientos de
// libres sin comprar de /creardropeo incluidos); ahora que la lista solo
// ensena legalizados, 20000 sobra de largo, pero se deja igual por si
// crece el numero de coches con papeles.
stock Panel_AbrirVehiculos(playerid)
{
    static lista[20000];
    new linea[160], modelo[32], estado[48];
    format(lista, sizeof lista, "{E8B33A}Matricula\tModelo\tEstado\n");

    new mostrados = 0, total = 0;
    for (new i = 0; i < g_UltimoVehiculo; i++)
    {
        if (!g_Vehiculos[i][veUsado]) continue;
        if (!Panel_VehiculoEsLegalizado(i)) continue;
        total++;

        Veh_NombreModelo(g_Vehiculos[i][veModelo], modelo, sizeof modelo);
        Panel_VehiculoEstadoTxt(i, estado, sizeof estado);

        format(linea, sizeof linea, "{FFFFFF}%s\t%s\t%s\n", g_Vehiculos[i][veMatricula], modelo, estado);

        if (strlen(lista) + strlen(linea) >= sizeof(lista) - 8) break;
        strcat(lista, linea, sizeof lista);
        mostrados++;
    }

    if (mostrados == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_PANEL_VEHICULOS, DIALOG_STYLE_MSGBOX,
            "{E8973A}Vehiculos", "{B4B5B7}No hay ningun vehiculo legalizado todavia (los libres sin comprar no se listan aqui).", "Volver", "");
        return;
    }

    new titulo[80];
    if (mostrados < total)
        format(titulo, sizeof titulo, "{E8973A}Vehiculos   %d de %d (lista cortada, hay demasiados)", mostrados, total);
    else
        format(titulo, sizeof titulo, "{E8973A}Vehiculos   %d", total);

    ShowPlayerDialog(playerid, DIALOGO_PANEL_VEHICULOS, DIALOG_STYLE_TABLIST_HEADERS, titulo, lista, "Elegir", "Volver");
}

stock Panel_AbrirFichaVehiculo(playerid, indice)
{
    g_PanelVehiculoSeleccionado[playerid] = indice;

    new modelo[32], tipoTxt[20], estado[80], cuerpo[600];
    Veh_NombreModelo(g_Vehiculos[indice][veModelo], modelo, sizeof modelo);

    switch (g_Vehiculos[indice][veTipo])
    {
        case VEH_PERSONAL: format(tipoTxt, sizeof tipoTxt, "Personal");
        case VEH_TRABAJO:  format(tipoTxt, sizeof tipoTxt, "Trabajo");
        case VEH_FACCION:  format(tipoTxt, sizeof tipoTxt, "Faccion");
        case VEH_SERVIDOR: format(tipoTxt, sizeof tipoTxt, "Servidor");
        case VEH_DROPEO:   format(tipoTxt, sizeof tipoTxt, "Comprado (dropeo)");
        default:            format(tipoTxt, sizeof tipoTxt, "?");
    }

    if (g_Vehiculos[indice][veTipo] == VEH_PERSONAL || g_Vehiculos[indice][veTipo] == VEH_DROPEO)
    {
        if (g_Vehiculos[indice][vePropietario] == 0)
        {
            format(estado, sizeof estado, "{32FF32}LIBRE");
        }
        else
        {
            new dueno[MAX_PLAYER_NAME + 1];
            format(dueno, sizeof dueno, "%s", g_Vehiculos[indice][vePropietarioNombre]);
            SubrayadoAEspacio(dueno);
            format(estado, sizeof estado, "{FF3232}CON PROPIETARIO{FFFFFF}: %s", dueno);
        }
    }
    else
    {
        format(estado, sizeof estado, "{8BC4E8}No aplica (%s)", tipoTxt);
    }

    format(cuerpo, sizeof cuerpo,
        "{E8B33A}%s{B4B5B7}   id BD %d\n\n\
        {B4B5B7}Matricula     {FFFFFF}%s\n\
        {B4B5B7}Tipo          {FFFFFF}%s\n\
        {B4B5B7}Propiedad     %s\n\
        {B4B5B7}Posicion      {FFFFFF}%.1f, %.1f, %.1f\n",
        modelo, g_Vehiculos[indice][veID], g_Vehiculos[indice][veMatricula], tipoTxt, estado,
        g_Vehiculos[indice][veX], g_Vehiculos[indice][veY], g_Vehiculos[indice][veZ]);

    ShowPlayerDialog(playerid, DIALOGO_PANEL_VEHICULO_FICHA, DIALOG_STYLE_MSGBOX,
        "{E8973A}Ficha del vehiculo", cuerpo, "Acciones", "Volver");
}

stock Panel_AbrirAccionesVehiculo(playerid, indice)
{
    g_PanelVehiculoSeleccionado[playerid] = indice;

    new titulo[48];
    format(titulo, sizeof titulo, "{E8973A}%s", g_Vehiculos[indice][veMatricula]);
    ShowPlayerDialog(playerid, DIALOGO_PANEL_VEHICULO_ACCIONES, DIALOG_STYLE_LIST, titulo,
        "Teletransportarme a el\nBorrar vehiculo\n", "Elegir", "Volver");
}

// ---------------------------------------------------------------------
//  SECCION: STAFF  (gestion de rangos del equipo conectado)
// ---------------------------------------------------------------------
stock Panel_AbrirStaff(playerid)
{
    new lista[1200], linea[96], nombre[MAX_PLAYER_NAME + 1], rango[32], conectados = 0;
    format(lista, sizeof lista, "{E8B33A}Nombre\tRango\tID\n");

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jAdmin] < ADMIN_SOPORTE) continue;

        NombreRP(i, nombre, sizeof nombre);
        Admin_NombreNivel(Jugador[i][jAdmin], rango, sizeof rango);

        format(linea, sizeof linea, "{FFFFFF}%s\t{E8973A}%s\t{B4B5B7}%d\n", nombre, rango, i);
        strcat(lista, linea, sizeof lista);
        conectados++;
    }

    if (conectados == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_PANEL_STAFF, DIALOG_STYLE_MSGBOX,
            "{E8973A}Staff", "{B4B5B7}No hay ningun otro miembro del equipo conectado.", "Volver", "");
        return;
    }

    ShowPlayerDialog(playerid, DIALOGO_PANEL_STAFF, DIALOG_STYLE_TABLIST_HEADERS, "{E8973A}Staff conectado", lista, "Elegir", "Volver");
}

// Reconstruye la misma lista para traducir fila -> playerid (staff conectado).
stock Panel_StaffFila(fila)
{
    new indice = 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jAdmin] < ADMIN_SOPORTE) continue;
        if (indice == fila) return i;
        indice++;
    }
    return INVALID_PLAYER_ID;
}

stock Panel_AbrirAccionesStaff(playerid, objetivo)
{
    if (objetivo == playerid)
    {
        Error(playerid, "No puedes gestionar tu propio rango.");
        Panel_AbrirStaff(playerid);
        return;
    }

    if (Jugador[objetivo][jAdmin] >= Jugador[playerid][jAdmin])
    {
        Error(playerid, "Ese admin tiene tu mismo nivel o superior.");
        Panel_AbrirStaff(playerid);
        return;
    }

    g_PanelStaffObjetivo[playerid] = objetivo;

    new nombre[MAX_PLAYER_NAME + 1], titulo[64];
    NombreRP(objetivo, nombre, sizeof nombre);
    format(titulo, sizeof titulo, "{E8973A}%s", nombre);

    // Filas fijas: 0 Subir, 1 Bajar, 2 Quitar. Si el resultado no es
    // valido (p.ej. subir mas alla de tu propio nivel), cmd_daradmin
    // ya lo rechaza con su propio mensaje de error.
    ShowPlayerDialog(playerid, DIALOGO_PANEL_STAFF_ACCIONES, DIALOG_STYLE_LIST, titulo,
        "Subir un nivel\nBajar un nivel\nQuitar del equipo\n", "Elegir", "Volver");
}

// ---------------------------------------------------------------------
//  DIALOGOS
// ---------------------------------------------------------------------
public Panel_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    switch (dialogid)
    {
        case DIALOGO_PANEL:
        {
            if (!response) return 1;

            // Nivel base para entrar al panel. Cada opcion del menu aplica
            // ademas su propio nivel mas estricto justo debajo.
            if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

            switch (listitem)
            {
                case 0: Panel_AbrirJugadores(playerid);
                case 1: Panel_AbrirReportes(playerid);
                case 2:
                {
                    if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;
                    Panel_AbrirSanciones(playerid);
                }
                case 3:
                {
                    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;
                    Panel_PedirBusquedaCuenta(playerid);
                }
                case 4:
                {
                    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;
                    Panel_AbrirCoords(playerid);
                }
                case 5:
                {
                    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;
                    Panel_AbrirCasas(playerid);
                }
                case 6:
                {
                    if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;
                    Panel_AbrirVehiculos(playerid);
                }
                case 7:
                {
                    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;
                    Panel_AbrirStaff(playerid);
                }
                case 8:
                {
                    if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

                    new total = Maria_ForzarTodasListas();
                    new mensaje[64];
                    format(mensaje, sizeof mensaje, "%d planta(s) puestas como listas para cosechar.", total);
                    Exito(playerid, mensaje);
                    Panel_AbrirPrincipal(playerid);
                }
            }
            return 1;
        }

        case DIALOGO_PANEL_JUGADORES:
        {
            if (!response) { Panel_AbrirPrincipal(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

            new objetivo = Panel_JugadorFila(listitem);
            if (objetivo == INVALID_PLAYER_ID)
            {
                Error(playerid, "Ese jugador ya no esta conectado.");
                Panel_AbrirJugadores(playerid);
                return 1;
            }

            Panel_AbrirAccionesJugador(playerid, objetivo);
            return 1;
        }

        case DIALOGO_PANEL_JUGADOR_ACCIONES:
        {
            if (!response) { Panel_AbrirJugadores(playerid); return 1; }

            new objetivo = g_PanelObjetivo[playerid];
            if (!IsPlayerConnected(objetivo) || !Jugador[objetivo][jLogueado])
            {
                Error(playerid, "Ese jugador ya no esta conectado.");
                Panel_AbrirJugadores(playerid);
                return 1;
            }

            if (listitem < 0 || listitem >= g_PanelNumAcciones[playerid]) return 1;

            new accion = g_PanelAccionMostrada[playerid][listitem];
            if (SinPermiso(playerid, Panel_AccionMinNivel(accion))) return 1;

            Panel_EjecutarAccion(playerid, objetivo, accion);
            return 1;
        }

        case DIALOGO_PANEL_JUGADOR_DURACION:
        {
            new objetivo = g_PanelObjetivo[playerid];

            if (!response) { Panel_AbrirAccionesJugador(playerid, objetivo); return 1; }
            if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;
            if (!IsPlayerConnected(objetivo) || !Jugador[objetivo][jLogueado])
            {
                Error(playerid, "Ese jugador ya no esta conectado.");
                Panel_AbrirJugadores(playerid);
                return 1;
            }

            if (!EsNumeroEntero(inputtext))
            {
                Error(playerid, "Escribe solo un numero.");
                switch (g_PanelAccion[playerid])
                {
                    case PACC_BANEAR:       Panel_PedirDuracion(playerid, "Dias de baneo (0 = permanente):");
                    case PACC_TRABAJO_DAR:  Panel_PedirDuracion(playerid, "Rango del trabajo (1-6):");
                    case PACC_FACCION_DAR:  Panel_PedirDuracion(playerid, "Rango en la faccion (1-10):");
                    default:                Panel_PedirDuracion(playerid, "Minutos de silencio (1 a 1440):");
                }
                return 1;
            }

            new valorNum = strval(inputtext);

            switch (g_PanelAccion[playerid])
            {
                case PACC_BANEAR:
                {
                    g_PanelValor[playerid] = valorNum;
                    Panel_PedirMotivo(playerid, "Motivo del baneo:");
                }
                case PACC_TRABAJO_DAR:
                {
                    new buf[24];
                    format(buf, sizeof buf, "%d %d %d", objetivo, g_PanelValor[playerid], valorNum);
                    cmd_darjob(playerid, buf);
                }
                case PACC_FACCION_DAR:
                {
                    new buf[24];
                    format(buf, sizeof buf, "%d %d %d", objetivo, g_PanelValor[playerid], valorNum);
                    cmd_darfaccion(playerid, buf);
                }
                default:
                {
                    g_PanelValor[playerid] = valorNum;
                    Panel_PedirMotivo(playerid, "Motivo del silencio:");
                }
            }
            return 1;
        }

        case DIALOGO_PANEL_JUGADOR_LISTA:
        {
            new objetivo = g_PanelObjetivo[playerid];

            if (!response) { Panel_AbrirAccionesJugador(playerid, objetivo); return 1; }
            if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;
            if (!IsPlayerConnected(objetivo) || !Jugador[objetivo][jLogueado])
            {
                Error(playerid, "Ese jugador ya no esta conectado.");
                Panel_AbrirJugadores(playerid);
                return 1;
            }
            if (listitem < 0 || listitem >= g_PanelListaNum[playerid]) return 1;

            new valor = g_PanelListaValores[playerid][listitem];

            switch (g_PanelAccion[playerid])
            {
                case PACC_TRABAJO_DAR:
                {
                    g_PanelValor[playerid] = valor;
                    Panel_PedirDuracion(playerid, "Rango del trabajo (1-6):");
                }
                case PACC_FACCION_DAR:
                {
                    g_PanelValor[playerid] = valor;
                    Panel_PedirDuracion(playerid, "Rango en la faccion (1-10):");
                }
                case PACC_TRABAJO_QUITAR:
                {
                    new buf[24];
                    format(buf, sizeof buf, "%d %d", objetivo, valor);
                    cmd_quitarjob(playerid, buf);
                }
            }
            return 1;
        }

        case DIALOGO_PANEL_JUGADOR_MOTIVO:
        {
            new objetivo = g_PanelObjetivo[playerid];

            if (!response) { Panel_AbrirAccionesJugador(playerid, objetivo); return 1; }
            if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;
            if (!IsPlayerConnected(objetivo) || !Jugador[objetivo][jLogueado])
            {
                Error(playerid, "Ese jugador ya no esta conectado.");
                Panel_AbrirJugadores(playerid);
                return 1;
            }

            if (EstaVacio(inputtext))
            {
                Error(playerid, "Tienes que escribir un motivo.");
                switch (g_PanelAccion[playerid])
                {
                    case PACC_KICK:   Panel_PedirMotivo(playerid, "Motivo de la expulsion:");
                    case PACC_AVISAR: Panel_PedirMotivo(playerid, "Motivo del aviso:");
                    case PACC_MUTEAR: Panel_PedirMotivo(playerid, "Motivo del silencio:");
                    case PACC_BANEAR: Panel_PedirMotivo(playerid, "Motivo del baneo:");
                }
                return 1;
            }

            new buf[200];
            switch (g_PanelAccion[playerid])
            {
                case PACC_KICK:   { format(buf, sizeof buf, "%d %s", objetivo, inputtext); cmd_expulsar_jugador(playerid, buf); }
                case PACC_AVISAR: { format(buf, sizeof buf, "%d %s", objetivo, inputtext); cmd_avisar(playerid, buf); }
                case PACC_MUTEAR: { format(buf, sizeof buf, "%d %d %s", objetivo, g_PanelValor[playerid], inputtext); cmd_mutear(playerid, buf); }
                case PACC_BANEAR: { format(buf, sizeof buf, "%d %d %s", objetivo, g_PanelValor[playerid], inputtext); cmd_banear(playerid, buf); }
            }
            return 1;
        }

        case DIALOGO_PANEL_REPORTES:
        {
            if (!response) { Panel_AbrirPrincipal(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_SOPORTE)) return 1;

            new numero = Reportes_NumeroPorFila(listitem);
            if (numero == 0) { Panel_AbrirPrincipal(playerid); return 1; }

            new buf[8];
            format(buf, sizeof buf, "%d", numero);
            cmd_atender(playerid, buf);
            return 1;
        }

        case DIALOGO_PANEL_SANCIONES:
        {
            if (SinPermiso(playerid, ADMIN_MODERADOR)) return 1;
            if (response) { }
            Panel_AbrirPrincipal(playerid);
            return 1;
        }

        case DIALOGO_PANEL_CUENTA_BUSCAR:
        {
            if (!response) { Panel_AbrirPrincipal(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

            if (EstaVacio(inputtext))
            {
                Error(playerid, "Escribe al menos una letra.");
                Panel_PedirBusquedaCuenta(playerid);
                return 1;
            }

            mysql_format(g_BD, g_Buffer, sizeof g_Buffer,
                "SELECT id, nombre, nivel, admin, baneado FROM cuentas WHERE nombre LIKE '%%%e%%' ORDER BY nombre LIMIT 20",
                inputtext);
            mysql_pquery(g_BD, g_Buffer, "Panel_AlBuscarCuentas", "d", playerid);
            return 1;
        }

        case DIALOGO_PANEL_CUENTA_RESULTADOS:
        {
            if (!response) { Panel_AbrirPrincipal(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;
            if (listitem < 0 || listitem >= g_PanelCuentaResultNum[playerid]) return 1;

            Panel_AbrirFichaCuenta(playerid, g_PanelCuentaResultID[playerid][listitem]);
            return 1;
        }

        case DIALOGO_PANEL_CUENTA_FICHA:
        {
            if (!response) { Panel_AbrirPrincipal(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;
            Panel_AbrirAccionesCuenta(playerid);
            return 1;
        }

        case DIALOGO_PANEL_CUENTA_ACCIONES:
        {
            if (!response) { Panel_AbrirFichaCuenta(playerid, g_PanelCuentaID[playerid]); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;
            if (listitem < 0 || listitem >= g_PanelListaNum[playerid]) return 1;

            new accion = g_PanelListaValores[playerid][listitem];
            Panel_EjecutarAccionCuenta(playerid, accion);
            return 1;
        }

        case DIALOGO_PANEL_CUENTA_VALOR:
        {
            if (!response) { Panel_AbrirAccionesCuenta(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

            // Dinero exige ADMIN_FUNDADOR, aparte del ADMIN_JEFE general
            // de arriba -- las demas acciones de este dialogo (nivel,
            // baneo) se quedan en su nivel de siempre, esto es solo para
            // dinero (05/09/2026, peticion expresa de Vlad).
            if (g_PanelCuentaAccion[playerid] == CACC_DINERO && SinPermiso(playerid, ADMIN_FUNDADOR)) return 1;

            if (!EsNumeroEntero(inputtext))
            {
                Error(playerid, "Escribe solo un numero.");
                switch (g_PanelCuentaAccion[playerid])
                {
                    case CACC_DINERO: Panel_PedirValorCuenta(playerid, "Cantidad (puede ser negativa):");
                    case CACC_NIVEL:  Panel_PedirValorCuenta(playerid, "Nuevo nivel de personaje (1-100):");
                    case CACC_BAN:    Panel_PedirValorCuenta(playerid, "Dias de baneo (0 = permanente):");
                }
                return 1;
            }

            g_PanelValor[playerid] = strval(inputtext);

            new objetivo = BuscarJugador(g_PanelCuentaNombre[playerid]);
            new bool:conectado = (objetivo != INVALID_PLAYER_ID && objetivo != JUGADOR_AMBIGUO);

            switch (g_PanelCuentaAccion[playerid])
            {
                case CACC_DINERO:
                {
                    if (conectado)
                    {
                        new buf[24];
                        format(buf, sizeof buf, "%d %d", objetivo, g_PanelValor[playerid]);
                        cmd_jailovicdardinero(playerid, buf);
                    }
                    else
                    {
                        Cuenta_DarDineroOffline(playerid, g_PanelCuentaID[playerid], g_PanelCuentaNombre[playerid], g_PanelValor[playerid]);
                    }
                    Panel_AbrirFichaCuenta(playerid, g_PanelCuentaID[playerid]);
                }

                case CACC_NIVEL:
                {
                    if (g_PanelValor[playerid] < 1 || g_PanelValor[playerid] > 100)
                    {
                        Error(playerid, "El nivel va de 1 a 100.");
                        Panel_PedirValorCuenta(playerid, "Nuevo nivel de personaje (1-100):");
                        return 1;
                    }

                    if (conectado)
                    {
                        new buf[24];
                        format(buf, sizeof buf, "%d %d", objetivo, g_PanelValor[playerid]);
                        cmd_darnivel(playerid, buf);
                    }
                    else
                    {
                        Cuenta_CambiarNivelOffline(playerid, g_PanelCuentaID[playerid], g_PanelCuentaNombre[playerid], g_PanelValor[playerid]);
                    }
                    Panel_AbrirFichaCuenta(playerid, g_PanelCuentaID[playerid]);
                }

                case CACC_BAN: Panel_PedirMotivoCuenta(playerid, "Motivo del baneo:");
            }
            return 1;
        }

        case DIALOGO_PANEL_CUENTA_MOTIVO:
        {
            if (!response) { Panel_AbrirAccionesCuenta(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

            if (EstaVacio(inputtext))
            {
                Error(playerid, "Tienes que escribir un motivo.");
                Panel_PedirMotivoCuenta(playerid, "Motivo del baneo:");
                return 1;
            }

            new objetivo = BuscarJugador(g_PanelCuentaNombre[playerid]);
            if (objetivo != INVALID_PLAYER_ID && objetivo != JUGADOR_AMBIGUO)
            {
                new buf[200];
                format(buf, sizeof buf, "%d %d %s", objetivo, g_PanelValor[playerid], inputtext);
                cmd_banear(playerid, buf);
            }
            else
            {
                Cuenta_BanearOffline(playerid, g_PanelCuentaID[playerid], g_PanelCuentaNombre[playerid], g_PanelValor[playerid], inputtext);
            }

            g_PanelCuentaBaneada[playerid] = 1;
            Panel_AbrirFichaCuenta(playerid, g_PanelCuentaID[playerid]);
            return 1;
        }

        case DIALOGO_PANEL_COORDS:
        {
            if (!response) { Panel_AbrirPrincipal(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

            if (listitem == 0)
            {
                ShowPlayerDialog(playerid, DIALOGO_PANEL_COORD_GUARDAR, DIALOG_STYLE_INPUT,
                    "{E8973A}Guardar coordenada", "Clave (sin espacios) y nombre, separados por espacio:\nEjemplo: hospital Hospital de Los Santos",
                    "Guardar", "Cancelar");
                return 1;
            }

            new indice = listitem - 1;
            if (indice < 0 || indice >= g_TotalCoords) return 1;

            Panel_AbrirAccionesCoord(playerid, indice);
            return 1;
        }

        case DIALOGO_PANEL_COORD_GUARDAR:
        {
            if (!response) { Panel_AbrirCoords(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

            new clave[24], nombre[48];
            if (sscanf(inputtext, "s[24]s[48]", clave, nombre))
            {
                Error(playerid, "Formato: clave nombre (ej: hospital Hospital de Los Santos)");
                return 1;
            }

            new buf[200];
            format(buf, sizeof buf, "%s %s", clave, nombre);
            cmd_marcarcoord(playerid, buf);
            Panel_AbrirCoords(playerid);
            return 1;
        }

        case DIALOGO_PANEL_COORD_ACCIONES:
        {
            if (!response) { Panel_AbrirCoords(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

            new indice = g_PanelCoordSeleccionada[playerid];
            if (indice < 0 || indice >= g_TotalCoords) { Panel_AbrirCoords(playerid); return 1; }

            if (listitem == 0)
            {
                Coord_Teletransportar(playerid, indice);
            }
            else if (listitem == 1)
            {
                new buf[24];
                format(buf, sizeof buf, "%s", g_Coords[indice][goClave]);
                cmd_borrarcoord(playerid, buf);
                Panel_AbrirCoords(playerid);
            }
            return 1;
        }

        case DIALOGO_PANEL_CASAS:
        {
            if (!response) { Panel_AbrirPrincipal(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;
            if (listitem < 0 || listitem >= g_TotalCasas) return 1;

            Panel_AbrirFichaCasa(playerid, listitem);
            return 1;
        }

        case DIALOGO_PANEL_CASA_FICHA:
        {
            if (!response) { Panel_AbrirCasas(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

            new indice = g_PanelCasaSeleccionada[playerid];
            if (indice < 0 || indice >= g_TotalCasas) { Panel_AbrirCasas(playerid); return 1; }

            Panel_AbrirAccionesCasa(playerid, indice);
            return 1;
        }

        case DIALOGO_PANEL_CASA_ACCIONES:
        {
            if (!response) { Panel_AbrirFichaCasa(playerid, g_PanelCasaSeleccionada[playerid]); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

            new indice = g_PanelCasaSeleccionada[playerid];
            if (indice < 0 || indice >= g_TotalCasas) { Panel_AbrirCasas(playerid); return 1; }

            switch (listitem)
            {
                case 0: // Ir a la puerta
                {
                    SetPlayerPos(playerid, g_Casas[indice][caExtX], g_Casas[indice][caExtY], g_Casas[indice][caExtZ]);
                    SetPlayerInterior(playerid, 0);
                    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
                    SetCameraBehindPlayer(playerid);
                }
                case 1: // Cambiar precio
                {
                    ShowPlayerDialog(playerid, DIALOGO_PANEL_CASA_PRECIO, DIALOG_STYLE_INPUT,
                        "{E8973A}Cambiar precio", "Nuevo precio (numero):", "Cambiar", "Cancelar");
                }
                case 2: // Transferir
                {
                    ShowPlayerDialog(playerid, DIALOGO_PANEL_CASA_TRANSFERIR, DIALOG_STYLE_INPUT,
                        "{E8973A}Transferir casa", "ID o nombre del jugador conectado:", "Transferir", "Cancelar");
                }
                case 3: // Liberar o borrar, segun estado
                {
                    if (g_Casas[indice][caPropietario] != 0)
                    {
                        Casa_LiberarPropietario(indice);
                        Exito(playerid, "Casa liberada. Ya no tiene propietario.");
                        Panel_AbrirFichaCasa(playerid, indice);
                    }
                    else
                    {
                        ShowPlayerDialog(playerid, DIALOGO_PANEL_CASA_BORRAR_CONFIRMAR, DIALOG_STYLE_MSGBOX,
                            "{E8973A}Borrar casa",
                            "{B4B5B7}Esto es definitivo.\n\n{FFFFFF}Confirmas?", "Borrar", "Cancelar");
                    }
                }
            }
            return 1;
        }

        case DIALOGO_PANEL_CASA_PRECIO:
        {
            if (!response) { Panel_AbrirAccionesCasa(playerid, g_PanelCasaSeleccionada[playerid]); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

            if (!EsNumeroEntero(inputtext) || strval(inputtext) < 1)
            {
                Error(playerid, "Precio invalido.");
                return 1;
            }

            new indice = g_PanelCasaSeleccionada[playerid];
            if (indice < 0 || indice >= g_TotalCasas) { Panel_AbrirCasas(playerid); return 1; }

            Casa_CambiarPrecio(indice, strval(inputtext));
            Exito(playerid, "Precio actualizado.");
            Panel_AbrirFichaCasa(playerid, indice);
            return 1;
        }

        case DIALOGO_PANEL_CASA_TRANSFERIR:
        {
            if (!response) { Panel_AbrirAccionesCasa(playerid, g_PanelCasaSeleccionada[playerid]); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

            new indice = g_PanelCasaSeleccionada[playerid];
            if (indice < 0 || indice >= g_TotalCasas) { Panel_AbrirCasas(playerid); return 1; }

            new objetivo = BuscarJugador(inputtext);
            if (JugadorInvalido(playerid, objetivo)) { Panel_AbrirAccionesCasa(playerid, indice); return 1; }

            new cuenta[MAX_PLAYER_NAME + 1];
            GetPlayerName(objetivo, cuenta, sizeof cuenta);
            Casa_TransferirPropietario(indice, Jugador[objetivo][jID], cuenta);

            format(g_Buffer, sizeof g_Buffer, "Casa transferida a %s.", cuenta);
            Exito(playerid, g_Buffer);
            Panel_AbrirFichaCasa(playerid, indice);
            return 1;
        }

        case DIALOGO_PANEL_CASA_BORRAR_CONFIRMAR:
        {
            if (!response) { Panel_AbrirCasas(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

            new indice = g_PanelCasaSeleccionada[playerid];
            if (indice < 0 || indice >= g_TotalCasas) { Panel_AbrirCasas(playerid); return 1; }

            Casa_Borrar(indice);
            Exito(playerid, "Casa borrada.");
            Panel_AbrirCasas(playerid);
            return 1;
        }

        case DIALOGO_PANEL_STAFF:
        {
            if (!response) { Panel_AbrirPrincipal(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

            new objetivo = Panel_StaffFila(listitem);
            if (objetivo == INVALID_PLAYER_ID) { Panel_AbrirStaff(playerid); return 1; }

            Panel_AbrirAccionesStaff(playerid, objetivo);
            return 1;
        }

        case DIALOGO_PANEL_STAFF_ACCIONES:
        {
            if (!response) { Panel_AbrirStaff(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_GESTION)) return 1;

            new objetivo = g_PanelStaffObjetivo[playerid];
            if (!IsPlayerConnected(objetivo) || !Jugador[objetivo][jLogueado]) { Panel_AbrirStaff(playerid); return 1; }

            new buf[16], nivel;
            switch (listitem)
            {
                case 0: nivel = Jugador[objetivo][jAdmin] + 1;
                case 1: nivel = Jugador[objetivo][jAdmin] - 1;
                case 2: nivel = 0;
                default: return 1;
            }

            format(buf, sizeof buf, "%d %d", objetivo, nivel);
            cmd_daradmin(playerid, buf);
            Panel_AbrirStaff(playerid);
            return 1;
        }

        case DIALOGO_PANEL_VEHICULOS:
        {
            if (!response) { Panel_AbrirPrincipal(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

            new indice = Panel_VehiculoFila(listitem);
            if (indice == -1) { Panel_AbrirVehiculos(playerid); return 1; }

            Panel_AbrirFichaVehiculo(playerid, indice);
            return 1;
        }

        case DIALOGO_PANEL_VEHICULO_FICHA:
        {
            if (!response) { Panel_AbrirVehiculos(playerid); return 1; }
            if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;

            new indice = g_PanelVehiculoSeleccionado[playerid];
            if (!VehiculoValido(indice)) { Panel_AbrirVehiculos(playerid); return 1; }

            Panel_AbrirAccionesVehiculo(playerid, indice);
            return 1;
        }

        case DIALOGO_PANEL_VEHICULO_ACCIONES:
        {
            new indice = g_PanelVehiculoSeleccionado[playerid];

            if (!response) { Panel_AbrirFichaVehiculo(playerid, indice); return 1; }
            if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;
            if (!VehiculoValido(indice)) { Panel_AbrirVehiculos(playerid); return 1; }

            switch (listitem)
            {
                case 0: // Teletransportarme a el
                {
                    new Float:x, Float:y, Float:z;
                    GetVehiclePos(g_Vehiculos[indice][veObjeto], x, y, z);

                    SetPlayerPos(playerid, x + 1.5, y, z + 0.5);
                    SetPlayerInterior(playerid, g_Vehiculos[indice][veInterior]);
                    SetPlayerVirtualWorld(playerid, g_Vehiculos[indice][veMundo]);

                    Exito(playerid, "Te has teletransportado junto al vehiculo.");
                    Panel_AbrirFichaVehiculo(playerid, indice);
                }
                case 1: // Borrar vehiculo
                {
                    new modelo[32], cuerpo[300];
                    Veh_NombreModelo(g_Vehiculos[indice][veModelo], modelo, sizeof modelo);

                    format(cuerpo, sizeof cuerpo,
                        "{FFFFFF}Vas a borrar el %s (matricula %s).\n\n{E05A5A}Esta operacion no se puede deshacer.",
                        modelo, g_Vehiculos[indice][veMatricula]);

                    ShowPlayerDialog(playerid, DIALOGO_PANEL_VEHICULO_BORRAR_CONFIRMAR, DIALOG_STYLE_MSGBOX,
                        "{E8973A}Borrar vehiculo", cuerpo, "Borrar", "Cancelar");
                }
            }
            return 1;
        }

        case DIALOGO_PANEL_VEHICULO_BORRAR_CONFIRMAR:
        {
            new indice = g_PanelVehiculoSeleccionado[playerid];

            if (!response) { Panel_AbrirFichaVehiculo(playerid, indice); return 1; }
            if (SinPermiso(playerid, ADMIN_ADMIN)) return 1;
            if (!VehiculoValido(indice)) { Panel_AbrirVehiculos(playerid); return 1; }

            new matricula[12];
            strmid(matricula, g_Vehiculos[indice][veMatricula], 0, sizeof matricula, sizeof matricula);

            Veh_Borrar(indice);

            format(g_Buffer, sizeof g_Buffer, "Vehiculo %s borrado.", matricula);
            Exito(playerid, g_Buffer);

            format(g_Buffer, sizeof g_Buffer, "PANEL_BORRAR_VEHICULO  admin=%d  matricula=%s",
                Jugador[playerid][jID], matricula);
            EscribirLog("administracion", g_Buffer);

            Panel_AbrirVehiculos(playerid);
            return 1;
        }
    }
    return 0;
}
