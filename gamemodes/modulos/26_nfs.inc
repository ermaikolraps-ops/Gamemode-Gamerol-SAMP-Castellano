/* =====================================================================
 *  26_nfs.inc
 *  ---------------------------------------------------------------------
 *  Trabajo NFS (ranura 2). Trafico de coches libres sin dueno.
 *
 *    1. /aceptarjob en el punto de contratacion: te haces miembro NFS,
 *       skin 29.
 *    2. /nfsmotor: arranca/apaga el motor de un coche libre sin llave
 *       (igual que /adminmotor), solo dentro del taller NFS.
 *    3. /nfscoche: sentado en un coche libre SIN DUENO y dentro del
 *       taller NFS, lo marca como "coche NFS". Un NFS JAMAS puede tocar
 *       un coche que ya tiene dueno  eso no cambia nunca.
 *    4. /nfspark: aparca un coche libre sin dueno donde quieras (llama
 *       a Veh_Aparcar, lo mismo que usa /adminpark).
 *    5. /nfsvendercoche [jugador] [precio] (ambos sentados en el coche
 *       marcado NFS, precio >= NFS_VENTA_MINIMA): el otro jugador
 *       confirma con /aceptarnfs, paga el precio integro, el NFS se
 *       lleva un fijo de NFS_VENTA_PAGO_NFS y el resto del dinero se
 *       destruye. El coche pasa a ser suyo 7 dias, como /comprarcoche.
 *    6. /nfspapeles [jugador] (mismo coche, precio fijo
 *       NFS_PAPELES_PRECIO): solo si al coche (que YA tiene dueno) le
 *       quedan menos de NFS_PAPELES_HORAS_MAX horas de papeles. El
 *       dueno confirma con /aceptarpapeles y se renueva 14 dias.
 *
 *  El taller NFS tambien vale para renovar (mas caro, NFS_RENOVAR_PRECIO)
 *  y dropear con el /taller de siempre  eso se resuelve en 22_dropeo.inc.
 * ===================================================================== */

enum E_NFS_JUGADOR
{
    naOfertaCocheIndice,        // Indice de vehiculo ofrecido en venta, -1 = nada
    naOfertaCochePrecio,
    naOfertaCocheDe,            // playerid del NFS que ofrece

    naOfertaPapelesIndice,      // Indice de vehiculo ofrecido en papeles, -1 = nada
    naOfertaPapelesDe
}
new gNFS[MAX_PLAYERS][E_NFS_JUGADOR];

static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaTallerNFS;

// ---------------------------------------------------------------------
//  ARRANQUE
// ---------------------------------------------------------------------
public NFS_Iniciar()
{
    g_EtiquetaTallerNFS = CreateDynamic3DTextLabel(
        "{5A9BD5}Taller NFS\n{FFFFFF}Escribe {E8B33A}/taller{FFFFFF} para renovar o dropear\n{B4B5B7}Miembros NFS: {E8B33A}/nfscoche {B4B5B7}sentados en un coche libre",
        0x5A9BD5FF, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z + 0.6, 14.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    return 1;
}

public NFS_OnConnect(playerid)
{
    new vacio[E_NFS_JUGADOR];
    gNFS[playerid] = vacio;
    gNFS[playerid][naOfertaCocheIndice]   = -1;
    gNFS[playerid][naOfertaPapelesIndice] = -1;
    return 1;
}

public NFS_OnDisconnect(playerid)
{
    #pragma unused playerid
    return 1;
}

public NFS_TrabajoRetirado(playerid)
{
    gNFS[playerid][naOfertaCocheIndice]   = -1;
    gNFS[playerid][naOfertaPapelesIndice] = -1;
    return 1;
}

public NFS_TrasContratar(playerid)
{
    // El uniforme NO se pone aqui. Lo pone Skin_Actualizar (03_utiles),
    // que ya ha corrido dentro de Trabajo_Dar leyendo g_SkinsTrabajo
    // (11_trabajos.inc). Ponerlo aqui a mano era justo el motivo de que
    // nadie lo quitara al dejar el trabajo: quien viste, desviste.
    Info(playerid, "Te han dado el uniforme de la NFS.");
    return 1;
}

// ---------------------------------------------------------------------
//  /NFSMOTOR  igual que /adminmotor, para miembros NFS y solo en
//  coches del sistema de dropeo.
// ---------------------------------------------------------------------
CMD:nfsmotor(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (!Trabajo_Tiene(playerid, TRABAJO_NFS))
    {
        Error(playerid, "No eres miembro de la NFS.");
        return 1;
    }

    if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
    {
        new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
        if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_DROPEO)
        {
            Error(playerid, "Solo puedes usar esto en coches libres.");
            return 1;
        }
    }

    // Sin aviso a la policia: usar /nfsmotor es el propio trabajo de la
    // NFS, no un delito que perseguir (quitado el 25/08/2026 junto con
    // el codigo penal 402 "Trafico de vehiculos" en 37_policia.inc).
    return Veh_ForzarMotor(playerid);
}

// ---------------------------------------------------------------------
//  /NFSCOCHE  marca un coche libre SIN DUENO como "coche NFS".
// ---------------------------------------------------------------------
CMD:nfscoche(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (!Trabajo_Tiene(playerid, TRABAJO_NFS))
    {
        Error(playerid, "No eres miembro de la NFS.");
        return 1;
    }

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del coche.");
        return 1;
    }

    if (!EnPunto(playerid, NFS_TALLER_RADIO, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z))
    {
        Error(playerid, "Esto solo se puede hacer en el taller NFS.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_DROPEO)
    {
        Error(playerid, "Este no es un coche libre del sistema de dropeo.");
        return 1;
    }

    if (g_Vehiculos[indice][vePropietario] != 0)
    {
        Error(playerid, "Este coche ya tiene dueno. La NFS no puede tocarlo.");
        return 1;
    }

    if (g_Vehiculos[indice][veEsNFS])
    {
        Error(playerid, "Este coche ya esta marcado como NFS.");
        return 1;
    }

    g_Vehiculos[indice][veEsNFS] = true;
    Veh_Guardar(indice);
    Dropeo_ActualizarCartel(indice);

    Exito(playerid, "Coche marcado como NFS. Solo se podra vender con /nfsvendercoche.");
    return 1;
}

// ---------------------------------------------------------------------
//  /NFSPARK  aparca un coche libre sin dueno donde quieras.
// ---------------------------------------------------------------------
CMD:nfspark(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (!Trabajo_Tiene(playerid, TRABAJO_NFS))
    {
        Error(playerid, "No eres miembro de la NFS.");
        return 1;
    }

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del coche.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(GetPlayerVehicleID(playerid));
    if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_DROPEO)
    {
        Error(playerid, "Este no es un coche libre del sistema de dropeo.");
        return 1;
    }

    if (g_Vehiculos[indice][vePropietario] != 0)
    {
        Error(playerid, "Este coche ya tiene dueno.");
        return 1;
    }

    Veh_Aparcar(indice);
    SendClientMessage(playerid, COLOR_BLANCO, "PARK!");
    return 1;
}

// ---------------------------------------------------------------------
//  VENTA: /nfsvendercoche [jugador] [precio]  ->  /aceptarnfs
// ---------------------------------------------------------------------
CMD:nfsvendercoche(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (!Trabajo_Tiene(playerid, TRABAJO_NFS))
    {
        Error(playerid, "No eres miembro de la NFS.");
        return 1;
    }

    new id, precio;
    if (sscanf(params, "ud", id, precio))
    {
        Uso(playerid, "/nfsvendercoche [jugador] [precio]");
        return 1;
    }

    if (!IsPlayerConnected(id) || id == playerid)
    {
        Error(playerid, "Jugador no valido.");
        return 1;
    }

    if (precio < NFS_VENTA_MINIMA)
    {
        Formato("El precio minimo de venta NFS es %d$.", NFS_VENTA_MINIMA);
        Error(playerid, g_Buffer);
        return 1;
    }

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del coche.");
        return 1;
    }

    if (!EnPunto(playerid, NFS_TALLER_RADIO, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z))
    {
        Error(playerid, "Esto solo se puede hacer en el taller NFS.");
        return 1;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    if (GetPlayerVehicleID(id) != vehicleid || GetPlayerState(id) == PLAYER_STATE_NONE)
    {
        Error(playerid, "El comprador tiene que estar sentado en el mismo coche.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_DROPEO || !g_Vehiculos[indice][veEsNFS])
    {
        Error(playerid, "Este coche no esta marcado como NFS.");
        return 1;
    }

    if (g_Vehiculos[indice][vePropietario] != 0)
    {
        Error(playerid, "Este coche ya tiene dueno.");
        return 1;
    }

    gNFS[id][naOfertaCocheIndice] = indice;
    gNFS[id][naOfertaCochePrecio] = precio;
    gNFS[id][naOfertaCocheDe]     = playerid;

    new nombreComprador[MAX_PLAYER_NAME + 1], precioTxt[24], mensaje[160];
    NombreRP(id, nombreComprador, sizeof nombreComprador);
    FormatearDinero(precio, precioTxt, sizeof precioTxt);

    format(mensaje, sizeof mensaje, "Le has ofrecido este coche a %s por %s.", nombreComprador, precioTxt);
    Exito(playerid, mensaje);

    format(mensaje, sizeof mensaje, "Te ofrecen este coche por %s. Escribe /aceptarnfs para comprarlo.", precioTxt);
    Info(id, mensaje);
    return 1;
}

CMD:aceptarnfs(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new indice = gNFS[playerid][naOfertaCocheIndice];
    if (indice == -1)
    {
        Error(playerid, "No tienes ninguna oferta de coche NFS pendiente.");
        return 1;
    }

    new vendedor = gNFS[playerid][naOfertaCocheDe];
    new precio   = gNFS[playerid][naOfertaCochePrecio];

    if (!VehiculoValido(indice) || g_Vehiculos[indice][veTipo] != VEH_DROPEO
        || !g_Vehiculos[indice][veEsNFS] || g_Vehiculos[indice][vePropietario] != 0)
    {
        Error(playerid, "Esa oferta ya no es valida.");
        gNFS[playerid][naOfertaCocheIndice] = -1;
        return 1;
    }

    if (!TieneDinero(playerid, precio))
    {
        Error(playerid, "No tienes suficiente dinero.");
        return 1;
    }

    // El comprador paga el precio integro. El NFS solo se lleva un fijo:
    // el resto del dinero desaparece del servidor, no va a nadie.
    DarDinero(playerid, -precio);
    if (IsPlayerConnected(vendedor)) DarDinero(vendedor, NFS_VENTA_PAGO_NFS);
    Negocio_IngresoNFS(precio - NFS_VENTA_PAGO_NFS);   // antes se perdia entero

    new nombre[MAX_PLAYER_NAME + 1], cuenta[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);
    GetPlayerName(playerid, cuenta, sizeof cuenta);

    g_Vehiculos[indice][vePropietario] = Jugador[playerid][jID];
    format(g_Vehiculos[indice][vePropietarioNombre], MAX_PLAYER_NAME + 1, "%s", cuenta);
    g_Vehiculos[indice][veDropeoExpira] = gettime() + (DROPEO_DIAS_PROPIEDAD * 86400);
    g_Vehiculos[indice][veAvisado24h] = false;
    g_Vehiculos[indice][veEsNFS] = false;
    Dropeo_MarcarArrancable(indice);

    Veh_Guardar(indice);
    Dropeo_ActualizarCartel(indice);

    new precioTxt[24];
    FormatearDinero(precio, precioTxt, sizeof precioTxt);
    format(g_Buffer, sizeof g_Buffer, "Coche comprado por %s. Papeles validos %d dias.", precioTxt, DROPEO_DIAS_PROPIEDAD);
    Exito(playerid, g_Buffer);

    if (IsPlayerConnected(vendedor))
    {
        new pagoTxt[24];
        FormatearDinero(NFS_VENTA_PAGO_NFS, pagoTxt, sizeof pagoTxt);
        format(g_Buffer, sizeof g_Buffer, "%s ha aceptado el coche. Te llevas %s.", nombre, pagoTxt);
        Exito(vendedor, g_Buffer);

        // Contador persistente para el panel web.
        new consultaStat[100];
        format(consultaStat, sizeof consultaStat, "UPDATE cuentas SET nfs_coches_vendidos = nfs_coches_vendidos + 1 WHERE id = %d", Jugador[vendedor][jID]);
        mysql_pquery(g_BD, consultaStat);
    }

    gNFS[playerid][naOfertaCocheIndice] = -1;
    return 1;
}

// ---------------------------------------------------------------------
//  PAPELES: /nfspapeles [jugador]  ->  /aceptarpapeles
// ---------------------------------------------------------------------
CMD:nfspapeles(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (!Trabajo_Tiene(playerid, TRABAJO_NFS))
    {
        Error(playerid, "No eres miembro de la NFS.");
        return 1;
    }

    new id;
    if (sscanf(params, "u", id))
    {
        Uso(playerid, "/nfspapeles [jugador]");
        return 1;
    }

    if (!IsPlayerConnected(id) || id == playerid)
    {
        Error(playerid, "Jugador no valido.");
        return 1;
    }

    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        Error(playerid, "Tienes que estar al volante del coche.");
        return 1;
    }

    if (!EnPunto(playerid, NFS_TALLER_RADIO, NFS_TALLER_X, NFS_TALLER_Y, NFS_TALLER_Z))
    {
        Error(playerid, "Esto solo se puede hacer en el taller NFS.");
        return 1;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    if (GetPlayerVehicleID(id) != vehicleid || GetPlayerState(id) == PLAYER_STATE_NONE)
    {
        Error(playerid, "El dueno tiene que estar sentado en el mismo coche.");
        return 1;
    }

    new indice = Veh_BuscarPorObjeto(vehicleid);
    if (indice == -1 || g_Vehiculos[indice][veTipo] != VEH_DROPEO || g_Vehiculos[indice][vePropietario] == 0)
    {
        Error(playerid, "Este coche libre no tiene dueno todavia.");
        return 1;
    }

    if (g_Vehiculos[indice][vePropietario] != Jugador[id][jID])
    {
        Error(playerid, "Ese jugador no es el dueno de este coche.");
        return 1;
    }

    new restante = g_Vehiculos[indice][veDropeoExpira] - gettime();
    if (restante > NFS_PAPELES_HORAS_MAX * 3600)
    {
        Formato("A este coche le quedan mas de %d horas de papeles. Todavia no se le pueden vender papeles NFS.", NFS_PAPELES_HORAS_MAX);
        Error(playerid, g_Buffer);
        return 1;
    }

    gNFS[id][naOfertaPapelesIndice] = indice;
    gNFS[id][naOfertaPapelesDe]     = playerid;

    new nombreDueno[MAX_PLAYER_NAME + 1], precioTxt[24], mensaje[160];
    NombreRP(id, nombreDueno, sizeof nombreDueno);
    FormatearDinero(NFS_PAPELES_PRECIO, precioTxt, sizeof precioTxt);

    format(mensaje, sizeof mensaje, "Le has ofrecido %d dias de papeles a %s por %s.", NFS_PAPELES_DIAS, nombreDueno, precioTxt);
    Exito(playerid, mensaje);

    format(mensaje, sizeof mensaje, "Te ofrecen %d dias de papeles por %s. Escribe /aceptarpapeles para renovar.", NFS_PAPELES_DIAS, precioTxt);
    Info(id, mensaje);
    return 1;
}

CMD:aceptarpapeles(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    new indice = gNFS[playerid][naOfertaPapelesIndice];
    if (indice == -1)
    {
        Error(playerid, "No tienes ninguna oferta de papeles NFS pendiente.");
        return 1;
    }

    new vendedor = gNFS[playerid][naOfertaPapelesDe];

    if (!VehiculoValido(indice) || g_Vehiculos[indice][veTipo] != VEH_DROPEO
        || g_Vehiculos[indice][vePropietario] != Jugador[playerid][jID])
    {
        Error(playerid, "Esa oferta ya no es valida.");
        gNFS[playerid][naOfertaPapelesIndice] = -1;
        return 1;
    }

    if (!TieneDinero(playerid, NFS_PAPELES_PRECIO))
    {
        Error(playerid, "No tienes suficiente dinero.");
        return 1;
    }

    DarDinero(playerid, -NFS_PAPELES_PRECIO);
    if (IsPlayerConnected(vendedor)) DarDinero(vendedor, NFS_PAPELES_PRECIO);

    g_Vehiculos[indice][veDropeoExpira] = gettime() + (NFS_PAPELES_DIAS * 86400);
    g_Vehiculos[indice][veAvisado24h] = false;
    Veh_Guardar(indice);
    Dropeo_ActualizarCartel(indice);

    format(g_Buffer, sizeof g_Buffer, "Papeles renovados %d dias por la NFS.", NFS_PAPELES_DIAS);
    Exito(playerid, g_Buffer);

    if (IsPlayerConnected(vendedor))
    {
        new nombre[MAX_PLAYER_NAME + 1];
        NombreRP(playerid, nombre, sizeof nombre);
        format(g_Buffer, sizeof g_Buffer, "%s ha aceptado los papeles.", nombre);
        Exito(vendedor, g_Buffer);
    }

    gNFS[playerid][naOfertaPapelesIndice] = -1;
    return 1;
}
