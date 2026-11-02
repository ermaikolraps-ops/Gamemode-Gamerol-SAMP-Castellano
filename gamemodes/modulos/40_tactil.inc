/* =====================================================================
 *  40_tactil.inc
 *  ---------------------------------------------------------------------
 *  /tactil activa un HUD de botones tactiles para SAMP, open.mp y Unity.
 *  V1: solo coches libres -- comprar, lock, motor, park y renovar
 *  papeles en el taller NFS. Reutiliza cmd_comprarcoche, cmd_lock,
 *  cmd_motor, cmd_park y cmd_renovarpapeles de 13_vehiculos.inc /
 *  22_dropeo.inc tal cual, sin duplicar su logica.
 *
 *  Preferencia de solo sesion (se resetea al reconectar).
 *
 *  SelectTextDraw es la unica forma que da el motor de SA-MP/open.mp
 *  para que un toque en pantalla llegue al servidor, y bloquea el
 *  control del jugador mientras esta activo (lo confirmamos a las
 *  bravas en Gamerol Unity). La abrimos solo en una ventana corta con
 *  caducidad automatica, cuando el coche esta parado: toque real, pero
 *  nunca se queda pillada pase lo que pase.
 * ===================================================================== */

new bool:g_Tactil[MAX_PLAYERS];

#define TACTIL_BTNA_OCULTO   0
#define TACTIL_BTNA_COMPRAR  1
#define TACTIL_BTNA_LOCK     2

#define TACTIL_VELOCIDAD_PARADO   0.05
#define TACTIL_VENTANA_MS         6000
#define TACTIL_REARME_MS          2000

new PlayerText:g_TactilBotonA[MAX_PLAYERS];   // COMPRAR / LOCK
new PlayerText:g_TactilBotonMotor[MAX_PLAYERS];
new PlayerText:g_TactilBotonPark[MAX_PLAYERS];
new PlayerText:g_TactilBotonB[MAX_PLAYERS];   // RENOVAR PAPELES

new g_TactilBotonAModo[MAX_PLAYERS];
new bool:g_TactilMotorParkVisible[MAX_PLAYERS];
new bool:g_TactilBotonBVisible[MAX_PLAYERS];
new g_TactilCooldown[MAX_PLAYERS];

new bool:g_TactilSeleccionando[MAX_PLAYERS];
new g_TactilSeleccionExpira[MAX_PLAYERS];
new g_TactilProximaVentana[MAX_PLAYERS];

// OJO: con alineacion CENTER, TextDrawTextSize no es ancho/alto -- hay
// que invertir los parametros y no cuadran con el area de toque real
// (asi es como MOTOR estaba pulsando LOCK). Con alineacion IZQUIERDA
// (la de por defecto) los dos numeros son la esquina inferior-derecha
// ABSOLUTA de la caja, sin ambiguedad.
stock Tactil_CrearBoton(playerid, &PlayerText:var, Float:y, const texto[], color)
{
    var = CreatePlayerTextDraw(playerid, 175.0, y, texto);
    PlayerTextDrawFont(playerid, var, TEXT_DRAW_FONT_2);
    PlayerTextDrawLetterSize(playerid, var, 0.25, 1.2);
    PlayerTextDrawColour(playerid, var, 0xFFFFFFFF);
    PlayerTextDrawUseBox(playerid, var, true);
    PlayerTextDrawBoxColour(playerid, var, color);
    PlayerTextDrawTextSize(playerid, var, 465.0, y + 45.0);
    PlayerTextDrawSetSelectable(playerid, var, true);
    return 1;
}

forward Tactil_OnConectar(playerid);
public Tactil_OnConectar(playerid)
{
    g_Tactil[playerid] = false;
    g_TactilBotonAModo[playerid] = TACTIL_BTNA_OCULTO;
    g_TactilMotorParkVisible[playerid] = false;
    g_TactilBotonBVisible[playerid] = false;
    g_TactilCooldown[playerid] = 0;
    g_TactilSeleccionando[playerid] = false;
    g_TactilSeleccionExpira[playerid] = 0;
    g_TactilProximaVentana[playerid] = 0;

    Tactil_CrearBoton(playerid, g_TactilBotonA[playerid],     330.0, "COMPRAR", 0x3D8B3DCC);
    Tactil_CrearBoton(playerid, g_TactilBotonMotor[playerid], 392.0, "MOTOR",   0x5A5A8BCC);
    Tactil_CrearBoton(playerid, g_TactilBotonPark[playerid],  454.0, "PARK",    0x5A5A8BCC);
    Tactil_CrearBoton(playerid, g_TactilBotonB[playerid],     516.0, "RENOVAR PAPELES", 0x5A9BD5CC);
    return 1;
}

forward Tactil_OnDesconectar(playerid);
public Tactil_OnDesconectar(playerid)
{
    PlayerTextDrawDestroy(playerid, g_TactilBotonA[playerid]);
    PlayerTextDrawDestroy(playerid, g_TactilBotonMotor[playerid]);
    PlayerTextDrawDestroy(playerid, g_TactilBotonPark[playerid]);
    PlayerTextDrawDestroy(playerid, g_TactilBotonB[playerid]);
    return 1;
}

stock PonerTactilBotonA(playerid, modo, indice = -1)
{
    if (g_TactilBotonAModo[playerid] == modo && modo != TACTIL_BTNA_LOCK) return 1;
    g_TactilBotonAModo[playerid] = modo;

    if (modo == TACTIL_BTNA_OCULTO)
    {
        PlayerTextDrawHide(playerid, g_TactilBotonA[playerid]);
        return 1;
    }

    if (modo == TACTIL_BTNA_COMPRAR)
    {
        new precioTxt[24], texto[48];
        FormatearDinero(g_Vehiculos[indice][vePrecio], precioTxt, sizeof precioTxt);
        format(texto, sizeof texto, "COMPRAR %s", precioTxt);
        PlayerTextDrawSetString(playerid, g_TactilBotonA[playerid], texto);
        PlayerTextDrawBoxColour(playerid, g_TactilBotonA[playerid], 0x3D8B3DCC);
    }
    else // TACTIL_BTNA_LOCK
    {
        PlayerTextDrawSetString(playerid, g_TactilBotonA[playerid], "LOCK");
        PlayerTextDrawBoxColour(playerid, g_TactilBotonA[playerid], 0x8B3D3DCC);
    }
    PlayerTextDrawShow(playerid, g_TactilBotonA[playerid]);
    return 1;
}

stock PonerTactilMotorPark(playerid, bool:visible)
{
    if (g_TactilMotorParkVisible[playerid] == visible) return 1;
    g_TactilMotorParkVisible[playerid] = visible;

    if (visible)
    {
        PlayerTextDrawShow(playerid, g_TactilBotonMotor[playerid]);
        PlayerTextDrawShow(playerid, g_TactilBotonPark[playerid]);
    }
    else
    {
        PlayerTextDrawHide(playerid, g_TactilBotonMotor[playerid]);
        PlayerTextDrawHide(playerid, g_TactilBotonPark[playerid]);
    }
    return 1;
}

stock PonerTactilBotonB(playerid, bool:visible)
{
    if (g_TactilBotonBVisible[playerid] == visible) return 1;
    g_TactilBotonBVisible[playerid] = visible;

    if (visible) PlayerTextDrawShow(playerid, g_TactilBotonB[playerid]);
    else PlayerTextDrawHide(playerid, g_TactilBotonB[playerid]);
    return 1;
}

// Ventana corta y con caducidad automatica para poder tocar los botones.
// Solo se abre con el coche parado, y se cierra sola al agotarse el
// tiempo, al tocar un boton, o al dejar de ser relevante.
stock Tactil_ActualizarVentana(playerid, bool:parado)
{
    new bool:hayBotones = (g_TactilBotonAModo[playerid] != TACTIL_BTNA_OCULTO
        || g_TactilMotorParkVisible[playerid] || g_TactilBotonBVisible[playerid]);
    new ahora = GetTickCount();

    if (g_TactilSeleccionando[playerid])
    {
        if (!hayBotones || ahora > g_TactilSeleccionExpira[playerid])
        {
            CancelSelectTextDraw(playerid);
            g_TactilSeleccionando[playerid] = false;
            g_TactilProximaVentana[playerid] = ahora + TACTIL_REARME_MS;
        }
        return 1;
    }

    if (hayBotones && parado && ahora > g_TactilProximaVentana[playerid])
    {
        SelectTextDraw(playerid, 0xFFFFFFAA);
        g_TactilSeleccionando[playerid] = true;
        g_TactilSeleccionExpira[playerid] = ahora + TACTIL_VENTANA_MS;
    }
    return 1;
}

stock Tactil_CerrarVentana(playerid)
{
    if (!g_TactilSeleccionando[playerid]) return 1;
    CancelSelectTextDraw(playerid);
    g_TactilSeleccionando[playerid] = false;
    g_TactilProximaVentana[playerid] = GetTickCount() + TACTIL_REARME_MS;
    return 1;
}

CMD:tactil(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    g_Tactil[playerid] = !g_Tactil[playerid];

    if (g_Tactil[playerid])
    {
        Exito(playerid, "Modo tactil activado. Parado en un coche libre veras botones en pantalla: tocalos con el dedo.");
    }
    else
    {
        Tactil_CerrarVentana(playerid);
        PonerTactilBotonA(playerid, TACTIL_BTNA_OCULTO);
        PonerTactilMotorPark(playerid, false);
        PonerTactilBotonB(playerid, false);
        Info(playerid, "Modo tactil desactivado.");
    }
    return 1;
}

forward Tactil_Timer();
public Tactil_Timer()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (!IsPlayerConnected(playerid) || !g_Tactil[playerid]) continue;
        if (!Jugador[playerid][jLogueado]) continue;

        new indice = -1;
        new bool:parado = false;

        if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
        {
            new vehiculoid = GetPlayerVehicleID(playerid);
            indice = Veh_BuscarPorObjeto(vehiculoid);

            new Float:vx, Float:vy, Float:vz;
            GetVehicleVelocity(vehiculoid, vx, vy, vz);
            parado = (floatabs(vx) + floatabs(vy) + floatabs(vz)) < TACTIL_VELOCIDAD_PARADO;
        }

        if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_DROPEO || g_Vehiculos[indice][veEsNFS])
        {
            PonerTactilBotonA(playerid, TACTIL_BTNA_OCULTO);
            PonerTactilMotorPark(playerid, false);
            PonerTactilBotonB(playerid, false);
        }
        else if (g_Vehiculos[indice][vePropietario] == 0)
        {
            PonerTactilBotonA(playerid, TACTIL_BTNA_COMPRAR, indice);
            PonerTactilMotorPark(playerid, false);
            PonerTactilBotonB(playerid, false);
        }
        else if (g_Vehiculos[indice][vePropietario] == Jugador[playerid][jID])
        {
            PonerTactilBotonA(playerid, TACTIL_BTNA_LOCK);
            PonerTactilMotorPark(playerid, true);

            new bool:enTaller = GetPlayerDistanceFromPoint(playerid, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z) <= NFS_TALLER_RADIO;
            new segundosRestantes = g_Vehiculos[indice][veDropeoExpira] - gettime();
            new bool:papelesPorCaducar = g_Vehiculos[indice][veDropeoExpira] > 0
                && segundosRestantes <= (DROPEO_RENOVAR_DIAS_ANTES * 86400);
            PonerTactilBotonB(playerid, enTaller && papelesPorCaducar);
        }
        else
        {
            PonerTactilBotonA(playerid, TACTIL_BTNA_OCULTO);
            PonerTactilMotorPark(playerid, false);
            PonerTactilBotonB(playerid, false);
        }

        Tactil_ActualizarVentana(playerid, parado);
    }
    return 1;
}

forward Tactil_OnClickPlayerTextDraw(playerid, PlayerText:playertextid);
public Tactil_OnClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
    if (!g_Tactil[playerid]) return 1;
    if (GetTickCount() < g_TactilCooldown[playerid]) return 1;
    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER) return 1;

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (indice == -1) return 1;

    new sinParams[2];
    sinParams[0] = 1;
    sinParams[1] = EOS;

    if (playertextid == g_TactilBotonA[playerid] && g_TactilBotonAModo[playerid] != TACTIL_BTNA_OCULTO)
    {
        g_TactilCooldown[playerid] = GetTickCount() + 2000;
        Tactil_CerrarVentana(playerid);

        if (g_TactilBotonAModo[playerid] == TACTIL_BTNA_COMPRAR) cmd_comprarcoche(playerid, sinParams);
        else cmd_lock(playerid, sinParams);
    }
    else if (playertextid == g_TactilBotonMotor[playerid] && g_TactilMotorParkVisible[playerid])
    {
        g_TactilCooldown[playerid] = GetTickCount() + 2000;
        Tactil_CerrarVentana(playerid);
        cmd_motor(playerid, sinParams);
    }
    else if (playertextid == g_TactilBotonPark[playerid] && g_TactilMotorParkVisible[playerid])
    {
        g_TactilCooldown[playerid] = GetTickCount() + 2000;
        Tactil_CerrarVentana(playerid);
        cmd_park(playerid, sinParams);
    }
    else if (playertextid == g_TactilBotonB[playerid] && g_TactilBotonBVisible[playerid])
    {
        g_TactilCooldown[playerid] = GetTickCount() + 2000;
        Tactil_CerrarVentana(playerid);
        cmd_renovarpapeles(playerid, sinParams);
    }
    return 1;
}
