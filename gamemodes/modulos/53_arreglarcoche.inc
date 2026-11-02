/* =====================================================================
 *  53_arreglarcoche.inc
 *  ---------------------------------------------------------------------
 *  Pay 'n' Spray casero, encargo de Vlad del 07/09/2026. Coordenada
 *  original marcada en /panel (tabla `coordenadas`, id 69, "/Arreglarcoche
 *  FIXCAR 1000$"). Ampliado a CUATRO puntos el mismo dia por la noche
 *  (Angel Pine + Grove Street, NFS y Policia Nacional -- ids 83-85 de la
 *  misma tabla), mismo comportamiento en los cuatro: no dependen de cual,
 *  solo hace falta estar junto a CUALQUIERA de ellos (g_ArreglarcocheX/Y/Z,
 *  01_configuracion.inc).
 *
 *  /arreglarcoche repara el vehiculo al 100% por un precio fijo
 *  (ARREGLARCOCHE_PRECIO, 01_configuracion.inc). El cobro va integro a
 *  la caja fuerte de la DGT (decision de Vlad, 07/09/2026) -- mismo
 *  negocio que ya se queda el 100% de cada /comprarcoche.
 *
 *  El aviso en pantalla reutiliza el MISMO estilo visual que "Estas
 *  muriendo de hambre" (44_hambre_sed.inc) / el cartel de bienvenida
 *  (47_bienvenida.inc): dos PlayerTextDraw, fuente Pricedown, se ocultan
 *  solos con un timer -- copia del patron, no una UI nueva.
 * ===================================================================== */

enum { ATD_TITULO, ATD_SUBTITULO, ATD_TOTAL }
static PlayerText:g_ArreglarTD[MAX_PLAYERS][ATD_TOTAL];

#define ARREGLARCOCHE_AVISO_MS 4000

stock Arreglarcoche_CrearTextdraws(playerid)
{
    g_ArreglarTD[playerid][ATD_TITULO] = CreatePlayerTextDraw(playerid, 320.0, 130.0, "Coche arreglado.");
    PlayerTextDrawAlignment(playerid, g_ArreglarTD[playerid][ATD_TITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_ArreglarTD[playerid][ATD_TITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_ArreglarTD[playerid][ATD_TITULO], 0.48, 2.0);
    PlayerTextDrawColour(playerid, g_ArreglarTD[playerid][ATD_TITULO], 0x4FD1C5FF);
    PlayerTextDrawSetOutline(playerid, g_ArreglarTD[playerid][ATD_TITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_ArreglarTD[playerid][ATD_TITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_ArreglarTD[playerid][ATD_TITULO], true);

    g_ArreglarTD[playerid][ATD_SUBTITULO] = CreatePlayerTextDraw(playerid, 320.0, 158.0, "Fuiste cobrado 1000$");
    PlayerTextDrawAlignment(playerid, g_ArreglarTD[playerid][ATD_SUBTITULO], TEXT_DRAW_ALIGN_CENTRE);
    PlayerTextDrawFont(playerid, g_ArreglarTD[playerid][ATD_SUBTITULO], TEXT_DRAW_FONT_PRICEDOWN);
    PlayerTextDrawLetterSize(playerid, g_ArreglarTD[playerid][ATD_SUBTITULO], 0.24, 1.2);
    PlayerTextDrawColour(playerid, g_ArreglarTD[playerid][ATD_SUBTITULO], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, g_ArreglarTD[playerid][ATD_SUBTITULO], 1);
    PlayerTextDrawSetShadow(playerid, g_ArreglarTD[playerid][ATD_SUBTITULO], 0);
    PlayerTextDrawSetProportional(playerid, g_ArreglarTD[playerid][ATD_SUBTITULO], true);
    return 1;
}

stock Arreglarcoche_MostrarAviso(playerid)
{
    PlayerTextDrawShow(playerid, g_ArreglarTD[playerid][ATD_TITULO]);
    PlayerTextDrawShow(playerid, g_ArreglarTD[playerid][ATD_SUBTITULO]);
    SetTimerEx("Arreglarcoche_OcultarAviso", ARREGLARCOCHE_AVISO_MS, false, "d", playerid);
    return 1;
}

forward Arreglarcoche_OcultarAviso(playerid);
public Arreglarcoche_OcultarAviso(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    PlayerTextDrawHide(playerid, g_ArreglarTD[playerid][ATD_TITULO]);
    PlayerTextDrawHide(playerid, g_ArreglarTD[playerid][ATD_SUBTITULO]);
    return 1;
}

stock Arreglarcoche_Iniciar()
{
    for (new p = 0; p < ARREGLARCOCHE_TOTAL_PUNTOS; p++)
    {
        CreateDynamic3DTextLabel(
            "{4FD1C5}Mecanico\n{FFFFFF}Usa {E8B33A}/arreglarcoche{FFFFFF} -- 1.000$",
            0x4FD1C5FF, g_ArreglarcocheX[p], g_ArreglarcocheY[p], g_ArreglarcocheZ[p] + 0.9, 12.0,
            INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 25.0);
    }
    return 1;
}

// Iconos de minimapa quitados el 11/09/2026 -- peticion expresa de Vlad
// de limpiar todos los iconos del mapa (la gente usa /gps). Se mantiene
// el textdraw, que no es un icono de mapa.
stock Arreglarcoche_OnConectar(playerid)
{
    Arreglarcoche_CrearTextdraws(playerid);
    return 1;
}

// Indice del punto en el que estas, o -1 si no estas junto a ninguno.
stock Arreglarcoche_PuntoCercano(playerid)
{
    for (new p = 0; p < ARREGLARCOCHE_TOTAL_PUNTOS; p++)
    {
        if (EnPunto(playerid, ARREGLARCOCHE_RADIO, g_ArreglarcocheX[p], g_ArreglarcocheY[p], g_ArreglarcocheZ[p]))
            return p;
    }
    return -1;
}

// Reparacion real, extraida de CMD:arreglarcoche el 14/09/2026 para que
// CRONOS GAMEROL (65_cronos.inc) pueda reparar GRATIS el vehiculo del
// premio de record mundial sin duplicar esta logica ni cobrar el precio
// normal -- el cobro (DarDinero/Negocio_Ingresar) se queda en el CMD,
// esto es solo el efecto mecanico sobre el vehiculo.
stock Arreglarcoche_Reparar(vehicleid, indice)
{
    // RepairVehicle repara salud Y daño visual (abolladuras, puertas,
    // cristales) de una vez -- SetVehicleHealth por si sola NO deshace
    // el daño visual, solo el numero. Mismo nativo que ya usa el
    // /reparar de admin en 16_administracion.inc.
    RepairVehicle(vehicleid);
    g_Vehiculos[indice][veSalud] = 1000.0;
    Veh_Guardar(indice);
    return 1;
}

CMD:arreglarcoche(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante de un vehiculo.");
        return 1;
    }
    new punto = Arreglarcoche_PuntoCercano(playerid);
    if (punto == -1)
    {
        Error(playerid, "Tienes que estar en el Pay 'n' Spray.");
        return 1;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (indice == -1)
    {
        Error(playerid, "Este vehiculo no se puede reparar aqui.");
        return 1;
    }

    if (!TieneDinero(playerid, ARREGLARCOCHE_PRECIO))
    {
        Error(playerid, "No te llegan los 1.000$ para arreglar el coche.");
        return 1;
    }

    DarDinero(playerid, -ARREGLARCOCHE_PRECIO);

    // Integro a la caja fuerte de la DGT (decision de Vlad, 07/09/2026)
    // -- si no tiene dueno, Negocio_Ingresar lo ignora solo.
    Negocio_Ingresar(NEGOCIO_SLOT_DGT, ARREGLARCOCHE_PRECIO);

    Arreglarcoche_Reparar(vehicleid, indice);

    // Sonido de "instalar pieza" del propio juego -- el mismo que suena
    // en Transfender/Wheel Arch Angels al elegir una modificacion
    // (verificado contra sampwiki.blast.hk/wiki/Sound_IDs, id 1133
    // "BUY_CAR_MOD"). Es lo mas parecido a un taladro mecanico que trae
    // el juego de fabrica.
    PlayerPlaySound(playerid, 1133, 0.0, 0.0, 0.0);

    Arreglarcoche_MostrarAviso(playerid);

    new nombre[MAX_PLAYER_NAME + 1], mensaje[144];
    NombreRP(playerid, nombre, sizeof nombre);
    format(mensaje, sizeof mensaje, "* %s repara su vehiculo en el Pay 'n' Spray (%s).", nombre, g_ArreglarcocheNombre[punto]);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

    Exito(playerid, "Coche arreglado. Fuiste cobrado 1.000$.");
    return 1;
}
