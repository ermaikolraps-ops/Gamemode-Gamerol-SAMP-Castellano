/* =====================================================================
 *  36_medicos.inc
 *  ---------------------------------------------------------------------
 *  Trabajo de Medicos Los Santos (ranura 5). Retirado el 07/09/2026,
 *  peticion expresa de Vlad: el sistema de heridos (caida congelada,
 *  animacion, texto en pantalla, /automatico, /llamadas -> /asistir ->
 *  /tratar -> /trasladar) desaparece entero. Morir vuelve a ser lo de
 *  serie de SA-MP (OnPlayerDeath/OnPlayerSpawn en 99_callbacks.inc): al
 *  llegar a 0 de vida, reapareces normal en tu posicion guardada, con tu
 *  skin, sin perder armas ni inventario.
 *
 *  El trabajo de Medicos ya no tiene NADA que ver con morir. Lo unico
 *  que le queda es un servicio de pago entre jugadores:
 *
 *    /tratar [id] [precio 1000-2000] -- el medico se lo propone a
 *    cualquier jugador vivo con menos de 100 de vida, cerca fisicamente
 *    (MEDICOS_RADIO_ATENDER). El paciente acepta con /aceptarcura: sube
 *    a vida completa y paga el precio acordado.
 *
 *  Sin llamadas, sin cola de avisos, sin estado que sobreviva a una
 *  desconexion -- una oferta pendiente que se cancela sola si caduca o
 *  si cualquiera de los dos deja de estar disponible.
 *
 *  Aviso a la Policia cuando un jugador mata a otro: vive en
 *  OnPlayerDeath (99_callbacks.inc), no aqui -- no tiene relacion con el
 *  trabajo de Medicos.
 * ===================================================================== */

enum E_CURA_OFERTA
{
    bool:cuActiva,
    cuMedico,
    cuPrecio,
    cuHora
}
new g_CuraOferta[MAX_PLAYERS][E_CURA_OFERTA];   // indexado por el PACIENTE

#define CURAR_OFERTA_CADUCA 60   // segundos que sigue en pie una oferta de /curar

// ---------------------------------------------------------------------
//  CICLO DE VIDA
// ---------------------------------------------------------------------
stock Medicos_OnConectar(playerid)
{
    g_CuraOferta[playerid][cuActiva] = false;
    return 1;
}

// Llamado desde Trabajo_Dialogo (11_trabajos.inc) justo al firmar.
public Medicos_TrasContratar(playerid)
{
    Info(playerid, "Ya eres Medico. Usa /tratar [id] [precio 1000-2000] junto a un paciente.");
    return 1;
}

// Llamado desde Trabajo_Quitar (11_trabajos.inc) al dejar el trabajo.
// Ya no hay nada que liberar (no hay llamadas ni cola), se queda como
// punto de enganche por si algun dia hace falta.
public Medicos_TrabajoRetirado(playerid)
{
    #pragma unused playerid
    return 1;
}

// Llamado desde OnPlayerDisconnect (99_callbacks.inc). Ya no hay estado
// de caida que resolver ni llamada que liberar.
public Medicos_OnDisconnect(playerid)
{
    #pragma unused playerid
    return 1;
}

// Llamado desde el dispatcher de dialogos (99_callbacks.inc). Ya no
// queda ningun dialogo propio de Medicos (el de /llamadas se borro con
// el comando) -- se deja el enganche para no tocar 99_callbacks.inc.
public Medicos_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    #pragma unused playerid, dialogid, response, listitem, inputtext
    return 0;
}

// Llamado desde 16_administracion.inc / 37_policia.inc antes de meter a
// alguien en la carcel o similar: red de seguridad que asumia que podia
// estar "herido". Ya no existe ese estado -- no hay nada que resolver,
// se deja el enganche para no tocar esos dos archivos.
public Medicos_ResolverAdmin(playerid)
{
    #pragma unused playerid
    return 1;
}

// ---------------------------------------------------------------------
//  /TRATAR Y /ACEPTARCURA
//  OJO: se llama /tratar y no /curar -- ya existe un /curar de admin
//  (16_administracion.inc, ADMIN_ADMIN, cura gratis sin precio). Nombre
//  distinto para no chocar con el; /aceptarcura se queda tal cual, sin
//  colision.
// ---------------------------------------------------------------------
CMD:tratar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (!Trabajo_Tiene(playerid, TRABAJO_MEDICOS))
    {
        Error(playerid, "No eres Medico.");
        return 1;
    }

    new objetivoTexto[24], precio;
    if (sscanf(params, "s[24]d", objetivoTexto, precio))
    {
        Uso(playerid, "/tratar [id o nombre] [precio 1000-2000]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    if (objetivo == playerid)
    {
        Error(playerid, "No puedes curarte a ti mismo.");
        return 1;
    }

    if (!EstaCerca(playerid, objetivo, MEDICOS_RADIO_ATENDER))
    {
        Error(playerid, "Acercate mas al paciente.");
        return 1;
    }

    if (precio < CURAR_PRECIO_MIN || precio > CURAR_PRECIO_MAX)
    {
        new textoMin[24], textoMax[24];
        FormatearDinero(CURAR_PRECIO_MIN, textoMin, sizeof textoMin);
        FormatearDinero(CURAR_PRECIO_MAX, textoMax, sizeof textoMax);
        Formato("El precio tiene que estar entre %s y %s.", textoMin, textoMax);
        Error(playerid, g_Buffer);
        return 1;
    }

    new Float:vida;
    GetPlayerHealth(objetivo, vida);
    if (vida >= 100.0)
    {
        Error(playerid, "Ese jugador ya esta a vida completa.");
        return 1;
    }

    g_CuraOferta[objetivo][cuActiva] = true;
    g_CuraOferta[objetivo][cuMedico] = playerid;
    g_CuraOferta[objetivo][cuPrecio] = precio;
    g_CuraOferta[objetivo][cuHora]   = gettime();

    new textoPrecio[24];
    FormatearDinero(precio, textoPrecio, sizeof textoPrecio);

    Formato("Le has propuesto curarle por %s. Esperando a que acepte con /aceptarcura.", textoPrecio);
    Exito(playerid, g_Buffer);

    Formato("Un medico te propone curarte por %s. Escribe /aceptarcura para aceptar.", textoPrecio);
    Info(objetivo, g_Buffer);
    return 1;
}

CMD:aceptarcura(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (!g_CuraOferta[playerid][cuActiva])
    {
        Error(playerid, "No tienes ninguna oferta de cura pendiente.");
        return 1;
    }

    new medico = g_CuraOferta[playerid][cuMedico];
    new precio = g_CuraOferta[playerid][cuPrecio];

    if (gettime() - g_CuraOferta[playerid][cuHora] > CURAR_OFERTA_CADUCA)
    {
        g_CuraOferta[playerid][cuActiva] = false;
        Error(playerid, "La oferta ha caducado. Pidele al medico que te la vuelva a proponer.");
        return 1;
    }

    if (!IsPlayerConnected(medico) || !EstaCerca(playerid, medico, MEDICOS_RADIO_ATENDER))
    {
        g_CuraOferta[playerid][cuActiva] = false;
        Error(playerid, "El medico ya no esta cerca. La oferta se ha cancelado.");
        return 1;
    }

    if (!TieneDinero(playerid, precio))
    {
        Error(playerid, "No tienes dinero suficiente para pagar la cura.");
        return 1;
    }

    g_CuraOferta[playerid][cuActiva] = false;

    DarDinero(playerid, -precio);
    DarDinero(medico, precio);
    SetPlayerHealth(playerid, 100.0);

    new textoPrecio[24];
    FormatearDinero(precio, textoPrecio, sizeof textoPrecio);

    Formato("Te han curado del todo. Has pagado %s.", textoPrecio);
    Exito(playerid, g_Buffer);

    Formato("Has curado y cobrado %s.", textoPrecio);
    Exito(medico, g_Buffer);
    return 1;
}
