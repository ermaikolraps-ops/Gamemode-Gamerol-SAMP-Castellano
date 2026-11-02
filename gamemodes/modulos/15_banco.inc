/* =====================================================================
 *  15_banco.inc
 *  ---------------------------------------------------------------------
 *  Banco de Los Santos.
 *
 *  NORMA CENTRAL: para ingresar, retirar o transferir hay que estar
 *  FISICAMENTE DENTRO del banco. No hay banca a distancia en 1992.
 *  Lo unico que se puede hacer desde fuera es consultar el saldo, que
 *  es informacion que tu personaje ya conoce.
 *
 *  El destinatario de una transferencia NO necesita estar conectado:
 *  se busca por nombre en la base de datos y se le ingresa igualmente.
 * ===================================================================== */

// Puerta de la calle. Puesta a mano desde /panel > Coordenadas
// (clave "posbanco").
#define BANCO_ENTRADA_X         1456.8
#define BANCO_ENTRADA_Y         -1012.77
#define BANCO_ENTRADA_Z         26.8438

// Punto de llegada dentro del interior.
#define BANCO_SALIDA_X          384.8092
#define BANCO_SALIDA_Y          173.7472
#define BANCO_SALIDA_Z          1008.3893
#define BANCO_SALIDA_A          0.0

// Distancia maxima al mostrador para poder operar.
#define BANCO_RADIO_MOSTRADOR   12.0

// Icono de $ en el minimapa, siempre visible (peticion de Vlad,
// 05/09/2026 -- reemplaza la norma anterior de "sin blip a proposito,
// se llega con /gps").
#define BANCO_ICON_SLOT         89

static STREAMER_TAG_PICKUP:g_PickupBancoFuera;
static STREAMER_TAG_PICKUP:g_PickupBancoDentro;

// ---------------------------------------------------------------------
//  PUNTOS FIJOS DEL MAPA
//  Ayuntamiento (banco/spawn), hospital y carcel. Hospital y carcel
//  siguen sin blip a proposito -- para llegar se usa /gps (30_gps.inc).
//  El banco SI lleva icono fijo en el minimapa, ver Banco_OnConectar.
// ---------------------------------------------------------------------
stock PuntosInteres_CrearMarcadores()
{
    return 1;
}

// Iconos de minimapa quitados el 11/09/2026 -- peticion expresa de Vlad
// de limpiar todos los iconos del mapa (la gente usa /gps). Se deja el
// stock vacio para no tocar el call site en OnPlayerConnect.
stock Banco_OnConectar(playerid)
{
    #pragma unused playerid
    return 1;
}
static STREAMER_TAG_PICKUP:g_PickupMostrador;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaBancoFuera;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaBancoDentro;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaMostrador;
static STREAMER_TAG_3D_TEXT_LABEL:g_EtiquetaBancoAngelPine;

// Datos de la transferencia en curso de cada jugador.
static g_DestinoID[MAX_PLAYERS];
static g_DestinoNombre[MAX_PLAYERS][MAX_PLAYER_NAME + 1];
static g_DestinoCantidad[MAX_PLAYERS];
static g_DestinoConcepto[MAX_PLAYERS][48];

// Evita que un segundo /transferir lanzado mientras el primero sigue en
// vuelo (esperando la consulta async del destinatario) mezcle la cantidad
// de una operacion con el destinatario de otra.
static bool:g_TransferenciaPendiente[MAX_PLAYERS];

// Se llama desde OnPlayerConnect: si el hueco de playerid quedo con una
// transferencia marcada como pendiente (por ejemplo, el jugador anterior
// se desconecto a mitad de la operacion), se libera para el que entra.
stock Banco_ReiniciarEstado(playerid)
{
    g_TransferenciaPendiente[playerid] = false;
    return 1;
}

// ---------------------------------------------------------------------
//  MARCADORES
// ---------------------------------------------------------------------
stock Banco_CrearMarcadores()
{
    // Puerta desde la calle.
    g_PickupBancoFuera = CreateDynamicPickup(1274, 23, BANCO_ENTRADA_X, BANCO_ENTRADA_Y, BANCO_ENTRADA_Z, 0, 0, -1, 100.0);

    g_EtiquetaBancoFuera = CreateDynamic3DTextLabel(
        "BANCO DE LOS SANTOS\n{FFFFFF}Escribe {E8B33A}/entrar{FFFFFF} para pasar dentro",
        COLOR_MARCA, BANCO_ENTRADA_X, BANCO_ENTRADA_Y, BANCO_ENTRADA_Z + 0.7, 14.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    // Puerta desde dentro.
    g_PickupBancoDentro = CreateDynamicPickup(1318, 23, BANCO_SALIDA_X, BANCO_SALIDA_Y, BANCO_SALIDA_Z,
        MUNDO_BANCO, g_BancoInterior, -1, 100.0);

    g_EtiquetaBancoDentro = CreateDynamic3DTextLabel(
        "{FFFFFF}Escribe {E8B33A}/salir{FFFFFF} para volver a la calle",
        COLOR_BLANCO, BANCO_SALIDA_X, BANCO_SALIDA_Y, BANCO_SALIDA_Z + 0.7, 8.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, MUNDO_BANCO, g_BancoInterior, -1, 25.0);

    // Mostrador de operaciones.
    g_PickupMostrador = CreateDynamicPickup(1274, 23, g_BancoPos[0], g_BancoPos[1], g_BancoPos[2],
        MUNDO_BANCO, g_BancoInterior, -1, 100.0);

    g_EtiquetaMostrador = CreateDynamic3DTextLabel(
        "MOSTRADOR\n{FFFFFF}Escribe {E8B33A}/banco{FFFFFF} para operar con tu cuenta",
        COLOR_MARCA, g_BancoPos[0], g_BancoPos[1], g_BancoPos[2] + 0.7, 10.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, MUNDO_BANCO, g_BancoInterior, -1, 25.0);

    // Punto exterior de Angel Pine (07/09/2026), sin interior ni pickup:
    // solo el cartel, /banco ahi abre el mismo menu (Banco_EstaDentro,
    // mas abajo, acepta este punto ademas del mostrador de dentro).
    g_EtiquetaBancoAngelPine = CreateDynamic3DTextLabel(
        "{53C168}Banco Angel Pine\n{FFFFFF}/Banco",
        COLOR_VERDE, ANGELPINE_BANCO_X, ANGELPINE_BANCO_Y, ANGELPINE_BANCO_Z + 0.7, 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 25.0);

    print("  [BANCO] Sucursal preparada.");
    return 1;
}

// Retira los marcadores para poder recolocarlos con /setbanco.
stock Banco_BorrarMarcadores()
{
    DestroyDynamicPickup(g_PickupBancoFuera);
    DestroyDynamicPickup(g_PickupBancoDentro);
    DestroyDynamicPickup(g_PickupMostrador);
    DestroyDynamic3DTextLabel(g_EtiquetaBancoFuera);
    DestroyDynamic3DTextLabel(g_EtiquetaBancoDentro);
    DestroyDynamic3DTextLabel(g_EtiquetaMostrador);
    DestroyDynamic3DTextLabel(g_EtiquetaBancoAngelPine);
    return 1;
}

// ---------------------------------------------------------------------
//  ¿ESTA FISICAMENTE DENTRO DEL BANCO?
// ---------------------------------------------------------------------
public bool:Banco_EstaDentro(playerid)
{
    // Mostrador de dentro del banco, Ayuntamiento de Los Santos.
    if (GetPlayerInterior(playerid) == g_BancoInterior
        && GetPlayerVirtualWorld(playerid) == MUNDO_BANCO
        && GetPlayerDistanceFromPoint(playerid, g_BancoPos[0], g_BancoPos[1], g_BancoPos[2]) <= BANCO_RADIO_MOSTRADOR)
        return true;

    // Punto exterior de Angel Pine (07/09/2026, peticion de Vlad): sin
    // interior propio, /banco ahi abre el mismo menu de siempre.
    if (GetPlayerInterior(playerid) == 0
        && GetPlayerVirtualWorld(playerid) == MUNDO_NORMAL
        && GetPlayerDistanceFromPoint(playerid, ANGELPINE_BANCO_X, ANGELPINE_BANCO_Y, ANGELPINE_BANCO_Z) <= ANGELPINE_BANCO_RADIO)
        return true;

    return false;
}

stock bool:Banco_ExigirPresencia(playerid)
{
    if (Banco_EstaDentro(playerid)) return false;

    Error(playerid, "Tienes que estar dentro del banco, o en el punto de Angel Pine.");
    Info(playerid, "La sucursal esta en el ayuntamiento de Los Santos; tambien hay un punto en Angel Pine. Te marco el del ayuntamiento.");
    SetPlayerCheckpoint(playerid, BANCO_ENTRADA_X, BANCO_ENTRADA_Y, BANCO_ENTRADA_Z, 4.0);
    return true;
}

// ---------------------------------------------------------------------
//  ENTRAR Y SALIR
// ---------------------------------------------------------------------
// /entrar y /salir son el punto de entrada generico a cualquier interior
// fijo del servidor. El banco se comprueba aqui mismo; cualquier otra
// puerta (la gasolinera de 27_tienda.inc, y las que vengan detras) se
// delega a su propio modulo con una funcion Modulo_IntentarEntrar/Salir
// para no ir apilando comandos CMD: duplicados.
CMD:entrar(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerInterior(playerid) == 0
        && EnPunto(playerid, 2.5, BANCO_ENTRADA_X, BANCO_ENTRADA_Y, BANCO_ENTRADA_Z))
    {
        SetPlayerPos(playerid, BANCO_SALIDA_X, BANCO_SALIDA_Y, BANCO_SALIDA_Z);
        SetPlayerFacingAngle(playerid, BANCO_SALIDA_A);
        SetPlayerInterior(playerid, g_BancoInterior);
        SetPlayerVirtualWorld(playerid, MUNDO_BANCO);
        SetCameraBehindPlayer(playerid);

        Info(playerid, "Banco de Los Santos. Acercate al mostrador y escribe /banco.");
        return 1;
    }

    if (Tienda_IntentarEntrar(playerid)) return 1;
    if (Policia_IntentarEntrar(playerid)) return 1;
    if (Casa_IntentarEntrar(playerid)) return 1;
    if (Comida_IntentarEntrar(playerid)) return 1;
    if (Empenos_IntentarEntrar(playerid)) return 1;
    if (Motel_IntentarEntrar(playerid)) return 1;
    if (Hospital_IntentarEntrar(playerid)) return 1;

    Error(playerid, "Aqui no hay ninguna puerta.");
    return 1;
}

CMD:salir(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerVirtualWorld(playerid) == MUNDO_BANCO
        && EnPunto(playerid, 3.0, BANCO_SALIDA_X, BANCO_SALIDA_Y, BANCO_SALIDA_Z))
    {
        SetPlayerPos(playerid, BANCO_ENTRADA_X, BANCO_ENTRADA_Y, BANCO_ENTRADA_Z);
        SetPlayerInterior(playerid, 0);
        SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
        SetCameraBehindPlayer(playerid);
        return 1;
    }

    if (Tienda_IntentarSalir(playerid)) return 1;
    if (Policia_IntentarSalir(playerid)) return 1;
    if (Casa_IntentarSalir(playerid)) return 1;
    if (Comida_IntentarSalir(playerid)) return 1;
    if (Empenos_IntentarSalir(playerid)) return 1;
    if (Motel_IntentarSalir(playerid)) return 1;
    if (Hospital_IntentarSalir(playerid)) return 1;

    Error(playerid, "Aqui no hay ninguna salida.");
    return 1;
}

// ---------------------------------------------------------------------
//  MOVIMIENTOS
// ---------------------------------------------------------------------
public Banco_Movimiento(cuentaid, tipo, cantidad, saldo, const contraparte[], const concepto[])
{
    // "Ganancias ultima hora" (55_ganancias.inc): todo ingreso real de
    // banco cuenta, EXCEPTO "Ingreso en ventanilla" -- es el jugador
    // metiendo su propio efectivo en su propia cuenta, no dinero nuevo.
    // Analizado a fondo el 07/09/2026: es la UNICA cadena de concepto
    // que significa eso; el resto de positivos (nomina, transferencia
    // recibida, ventas, Ayudante de Vendedor...) son ingresos reales.
    if (cantidad > 0 && strcmp(concepto, "Ingreso en ventanilla") != 0)
    {
        for (new i = 0; i < MAX_PLAYERS; i++)
        {
            if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
            if (Jugador[i][jID] != cuentaid) continue;
            Ganancias_Registrar(i, cantidad);
            break;
        }
    }

    new consulta[512];
    mysql_format(g_BD, consulta, sizeof consulta,
        "INSERT INTO banco_movimientos (cuenta_id, tipo, cantidad, saldo, contraparte, concepto) \
         VALUES (%d, %d, %d, %d, '%e', '%e')", cuentaid, tipo, cantidad, saldo, contraparte, concepto);
    mysql_pquery(g_BD, consulta);
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDOS
// ---------------------------------------------------------------------
CMD:saldo(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new banco[24], mano[24];
    FormatearDinero(Jugador[playerid][jBanco], banco, sizeof banco);
    FormatearDinero(Jugador[playerid][jDinero], mano, sizeof mano);

    format(g_Buffer, sizeof g_Buffer, "Cuenta n%c %d   ~   Saldo: {FFFFFF}%s{8BC4E8}   ~   En efectivo: {FFFFFF}%s",
        186, Jugador[playerid][jCuentaBancaria], banco, mano);
    Info(playerid, g_Buffer);
    return 1;
}

CMD:banco(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Banco_ExigirPresencia(playerid)) return 1;

    Banco_MostrarMenu(playerid);
    return 1;
}

stock Banco_MostrarMenu(playerid)
{
    new banco[24], mano[24];
    FormatearDinero(Jugador[playerid][jBanco], banco, sizeof banco);
    FormatearDinero(Jugador[playerid][jDinero], mano, sizeof mano);

    new titulo[128];
    format(titulo, sizeof titulo, "{E8B33A}Banco de Los Santos   ~   Cuenta n%c %d", 186, Jugador[playerid][jCuentaBancaria]);

    format(g_Buffer, sizeof g_Buffer,
        "{FFFFFF}Ingresar dinero\n\
        {FFFFFF}Retirar dinero\n\
        {FFFFFF}Transferir a otra persona\n\
        {FFFFFF}Ver mis ultimos movimientos\n\
        {B4B5B7}Saldo: %s   Efectivo: %s", banco, mano);

    ShowPlayerDialog(playerid, DIALOGO_BANCO, DIALOG_STYLE_LIST, titulo, g_Buffer, "Elegir", "Salir");
    return 1;
}

CMD:transferir(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (Banco_ExigirPresencia(playerid)) return 1;

    new destino[MAX_PLAYER_NAME + 1], cantidad;
    if (sscanf(params, "s[25]d", destino, cantidad))
    {
        Uso(playerid, "/transferir [nombre completo de la cuenta] [cantidad]");
        Linea(playerid, COLOR_GRIS, "Ejemplo: /transferir Juan_Perez 5000");
        return 1;
    }

    Banco_PrepararTransferencia(playerid, destino, cantidad);
    return 1;
}

stock Banco_PrepararTransferencia(playerid, const destino[], cantidad)
{
    if (g_TransferenciaPendiente[playerid])
    {
        Error(playerid, "Ya tienes una transferencia en curso. Espera a que termine o cancelala.");
        return 0;
    }

    if (cantidad < BANCO_TRANSFER_MINIMA)
    {
        format(g_Buffer, sizeof g_Buffer, "La transferencia minima es de %d$.", BANCO_TRANSFER_MINIMA);
        Error(playerid, g_Buffer);
        return 0;
    }
    if (cantidad > BANCO_TRANSFER_MAXIMA)
    {
        format(g_Buffer, sizeof g_Buffer, "La transferencia maxima es de %d$ por operacion.", BANCO_TRANSFER_MAXIMA);
        Error(playerid, g_Buffer);
        return 0;
    }

    new comision = (cantidad * BANCO_COMISION) / 100;
    if (Jugador[playerid][jBanco] < cantidad + comision)
    {
        new falta[24];
        FormatearDinero((cantidad + comision) - Jugador[playerid][jBanco], falta, sizeof falta);
        format(g_Buffer, sizeof g_Buffer, "No tienes saldo suficiente. Te faltan %s (comision incluida).", falta);
        Error(playerid, g_Buffer);
        return 0;
    }

    new mio[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, mio, sizeof mio);

    if (strcmp(mio, destino, true) == 0)
    {
        Error(playerid, "No puedes transferirte dinero a ti mismo.");
        return 0;
    }

    g_DestinoCantidad[playerid] = cantidad;
    format(g_DestinoNombre[playerid], MAX_PLAYER_NAME + 1, "%s", destino);
    format(g_DestinoConcepto[playerid], 48, "Transferencia");
    g_TransferenciaPendiente[playerid] = true;

    new consulta[256];
    mysql_format(g_BD, consulta, sizeof consulta,
        "SELECT id, nombre, banco FROM cuentas WHERE nombre = '%e' LIMIT 1", destino);

    mysql_pquery(g_BD, consulta, "AlBuscarDestinatario", "d", playerid);
    return 1;
}

forward AlBuscarDestinatario(playerid);
public AlBuscarDestinatario(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        g_TransferenciaPendiente[playerid] = false;
        format(g_Buffer, sizeof g_Buffer, "No existe ninguna cuenta llamada %s.", g_DestinoNombre[playerid]);
        Error(playerid, g_Buffer);
        Info(playerid, "El nombre tiene que ser el completo, con el guion bajo. Ejemplo: Juan_Perez");
        return 1;
    }

    cache_get_value_name_int(0, "id", g_DestinoID[playerid]);
    cache_get_value_name(0, "nombre", g_DestinoNombre[playerid], MAX_PLAYER_NAME + 1);

    ShowPlayerDialog(playerid, DIALOGO_BANCO_TRANSFERIR_CONCEPTO, DIALOG_STYLE_INPUT,
        "{E8B33A}Concepto de la transferencia",
        "{FFFFFF}Escribe el concepto que quieres que figure en el recibo.\n\n\
        {B4B5B7}Por ejemplo: alquiler, compra de mercancia, prestamo...\n\
        Puedes dejarlo vacio.",
        "Continuar", "Cancelar");
    return 1;
}

stock Banco_ConfirmarTransferencia(playerid)
{
    new cantidad = g_DestinoCantidad[playerid];
    new comision = (cantidad * BANCO_COMISION) / 100;

    new textoCantidad[24], textoComision[24], textoTotal[24], destinoRP[MAX_PLAYER_NAME + 1];
    FormatearDinero(cantidad, textoCantidad, sizeof textoCantidad);
    FormatearDinero(comision, textoComision, sizeof textoComision);
    FormatearDinero(cantidad + comision, textoTotal, sizeof textoTotal);

    format(destinoRP, sizeof destinoRP, "%s", g_DestinoNombre[playerid]);
    SubrayadoAEspacio(destinoRP);

    new cuerpo[700];
    format(cuerpo, sizeof cuerpo,
        "{FFFFFF}Vas a enviar dinero desde tu cuenta bancaria.\n\n\
        {B4B5B7}Destinatario   {FFFFFF}%s\n\
        {B4B5B7}Cantidad       {3D9970}%s\n\
        {B4B5B7}Comision (%d%%) {E05A5A}%s\n\
        {B4B5B7}Total a cargar {FFFFFF}%s\n\
        {B4B5B7}Concepto       {FFFFFF}%s\n\n\
        {B4B5B7}El dinero llega al instante aunque no este conectado.\n\
        {E05A5A}Una vez enviada, la transferencia no se puede anular.",
        destinoRP, textoCantidad, BANCO_COMISION, textoComision, textoTotal,
        (g_DestinoConcepto[playerid][0] == EOS) ? ("Transferencia") : (g_DestinoConcepto[playerid]));

    ShowPlayerDialog(playerid, DIALOGO_BANCO_TRANSFERIR_CONFIRMAR, DIALOG_STYLE_MSGBOX,
        "{E8B33A}Confirmar transferencia", cuerpo, "Enviar", "Cancelar");
    return 1;
}

stock Banco_EjecutarTransferencia(playerid)
{
    // Ultima comprobacion: sigue dentro del banco y tiene el dinero.
    if (!Banco_EstaDentro(playerid))
    {
        g_TransferenciaPendiente[playerid] = false;
        Error(playerid, "Has salido del banco. La operacion se ha cancelado.");
        return 0;
    }

    new cantidad = g_DestinoCantidad[playerid];
    new comision = (cantidad * BANCO_COMISION) / 100;
    new destinoID = g_DestinoID[playerid];

    if (destinoID == 0 || cantidad <= 0)
    {
        g_TransferenciaPendiente[playerid] = false;
        Error(playerid, "La operacion ya no es valida.");
        return 0;
    }

    if (Jugador[playerid][jBanco] < cantidad + comision)
    {
        g_TransferenciaPendiente[playerid] = false;
        Error(playerid, "Tu saldo ha cambiado y ya no llega. Operacion cancelada.");
        return 0;
    }

    // Cargo en la cuenta del emisor.
    Jugador[playerid][jBanco] -= (cantidad + comision);

    new mio[MAX_PLAYER_NAME + 1], mioRP[MAX_PLAYER_NAME + 1], destinoRP[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, mio, sizeof mio);
    NombreRP(playerid, mioRP, sizeof mioRP);
    format(destinoRP, sizeof destinoRP, "%s", g_DestinoNombre[playerid]);
    SubrayadoAEspacio(destinoRP);

    Banco_Movimiento(Jugador[playerid][jID], 3, -(cantidad + comision), Jugador[playerid][jBanco],
        g_DestinoNombre[playerid], g_DestinoConcepto[playerid]);

    // Abono al destinatario: si esta conectado se le actualiza en
    // memoria, y en cualquier caso se le suma en la base de datos.
    new destinatarioConectado = INVALID_PLAYER_ID;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !Jugador[i][jLogueado]) continue;
        if (Jugador[i][jID] != destinoID) continue;
        destinatarioConectado = i;
        break;
    }

    new consulta[256];
    format(consulta, sizeof consulta, "UPDATE cuentas SET banco = banco + %d WHERE id = %d", cantidad, destinoID);
    mysql_pquery(g_BD, consulta);

    if (destinatarioConectado != INVALID_PLAYER_ID)
    {
        Jugador[destinatarioConectado][jBanco] += cantidad;

        new textoCantidad[24];
        FormatearDinero(cantidad, textoCantidad, sizeof textoCantidad);

        SendClientMessage(destinatarioConectado, COLOR_MARCA, " ");
        format(g_Buffer, sizeof g_Buffer, PREFIJO "Has recibido una transferencia de {3D9970}%s{B4B5B7}.", textoCantidad);
        SendClientMessage(destinatarioConectado, COLOR_GRIS, g_Buffer);
        format(g_Buffer, sizeof g_Buffer, PREFIJO "Remitente: {FFFFFF}%s{B4B5B7}   Concepto: {FFFFFF}%s", mioRP,
            (g_DestinoConcepto[playerid][0] == EOS) ? ("Transferencia") : (g_DestinoConcepto[playerid]));
        SendClientMessage(destinatarioConectado, COLOR_GRIS, g_Buffer);
        SendClientMessage(destinatarioConectado, COLOR_MARCA, " ");

        PlayerPlaySound(destinatarioConectado, 1052, 0.0, 0.0, 0.0);

        Banco_Movimiento(destinoID, 4, cantidad, Jugador[destinatarioConectado][jBanco], mio, g_DestinoConcepto[playerid]);
    }
    else
    {
        Banco_Movimiento(destinoID, 4, cantidad, 0, mio, g_DestinoConcepto[playerid]);
    }

    BD_GuardarCuenta(playerid);

    new textoCantidad[24], textoSaldo[24];
    FormatearDinero(cantidad, textoCantidad, sizeof textoCantidad);
    FormatearDinero(Jugador[playerid][jBanco], textoSaldo, sizeof textoSaldo);

    SendClientMessage(playerid, COLOR_MARCA, " ");
    format(g_Buffer, sizeof g_Buffer, PREFIJO "Transferencia enviada: {3D9970}%s{B4B5B7} a {FFFFFF}%s{B4B5B7}.", textoCantidad, destinoRP);
    SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
    format(g_Buffer, sizeof g_Buffer, PREFIJO "Nuevo saldo: {FFFFFF}%s{B4B5B7}.", textoSaldo);
    SendClientMessage(playerid, COLOR_GRIS, g_Buffer);
    SendClientMessage(playerid, COLOR_MARCA, " ");

    PlayerPlaySound(playerid, 1052, 0.0, 0.0, 0.0);

    new mensaje[192];
    format(mensaje, sizeof mensaje, "* %s firma unos papeles en el mostrador del banco.", mioRP);
    MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);

    format(g_Buffer, sizeof g_Buffer, "%s transfirio %d a %s (concepto: %s)", mio, cantidad,
        g_DestinoNombre[playerid], g_DestinoConcepto[playerid]);
    EscribirLog("banco", g_Buffer);

    // Se limpian los datos de la operacion.
    g_DestinoID[playerid]       = 0;
    g_DestinoCantidad[playerid] = 0;
    g_TransferenciaPendiente[playerid] = false;
    return 1;
}

// ---------------------------------------------------------------------
//  HISTORIAL
// ---------------------------------------------------------------------
stock Banco_VerMovimientos(playerid)
{
    new consulta[300];
    format(consulta, sizeof consulta,
        "SELECT tipo, cantidad, saldo, contraparte, concepto, \
                DATE_FORMAT(fecha, '%%d/%%m %%H:%%i') AS cuando \
         FROM banco_movimientos WHERE cuenta_id = %d ORDER BY fecha DESC LIMIT 20", Jugador[playerid][jID]);

    mysql_pquery(g_BD, consulta, "AlCargarMovimientos", "d", playerid);
    return 1;
}

forward AlCargarMovimientos(playerid);
public AlCargarMovimientos(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_BANCO_HISTORIAL, DIALOG_STYLE_MSGBOX,
            "{E8B33A}Movimientos",
            "{B4B5B7}Tu cuenta no tiene movimientos todavia.",
            "Volver", "");
        return 1;
    }

    static const nombresTipo[][] =
    {
        "", "Ingreso", "Retirada", "Enviado", "Recibido", "Nomina", "Ajuste"
    };

    new lista[2400], linea[220], tipo, cantidad, contraparte[24], concepto[64], cuando[16];

    format(lista, sizeof lista, "{E8B33A}Fecha\tConcepto\tImporte\n");

    for (new i = 0; i < filas; i++)
    {
        cache_get_value_name_int(i, "tipo", tipo);
        cache_get_value_name_int(i, "cantidad", cantidad);
        cache_get_value_name(i, "contraparte", contraparte, sizeof contraparte);
        cache_get_value_name(i, "concepto", concepto, sizeof concepto);
        cache_get_value_name(i, "cuando", cuando, sizeof cuando);

        if (tipo < 1 || tipo > 6) tipo = 6;
        SubrayadoAEspacio(contraparte);

        new detalle[96];
        if (contraparte[0] != EOS) format(detalle, sizeof detalle, "%s: %s", nombresTipo[tipo], contraparte);
        else                       format(detalle, sizeof detalle, "%s", nombresTipo[tipo]);

        new importe[24];
        FormatearDinero((cantidad < 0) ? (-cantidad) : (cantidad), importe, sizeof importe);

        format(linea, sizeof linea, "{B4B5B7}%s\t{FFFFFF}%s\t%s%s%s\n",
            cuando, detalle,
            (cantidad < 0) ? ("{E05A5A}-") : ("{3D9970}+"),
            importe, "");

        if (strlen(lista) + strlen(linea) >= sizeof(lista) - 8) break;
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_BANCO_HISTORIAL, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8B33A}Ultimos movimientos", lista, "Volver", "");
    return 1;
}

// ---------------------------------------------------------------------
//  DIÁLOGOS
// ---------------------------------------------------------------------
stock Banco_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    switch (dialogid)
    {
        case DIALOGO_BANCO:
        {
            if (!response) return 1;
            if (Banco_ExigirPresencia(playerid)) return 1;

            switch (listitem)
            {
                case 0:
                {
                    ShowPlayerDialog(playerid, DIALOGO_BANCO_INGRESAR, DIALOG_STYLE_INPUT,
                        "{E8B33A}Ingresar dinero",
                        "{FFFFFF}Cuanto quieres ingresar en tu cuenta?\n\n{B4B5B7}Escribe la cantidad en numeros.",
                        "Ingresar", "Atras");
                }
                case 1:
                {
                    ShowPlayerDialog(playerid, DIALOGO_BANCO_RETIRAR, DIALOG_STYLE_INPUT,
                        "{E8B33A}Retirar dinero",
                        "{FFFFFF}Cuanto quieres retirar en efectivo?\n\n{B4B5B7}Escribe la cantidad en numeros.",
                        "Retirar", "Atras");
                }
                case 2:
                {
                    ShowPlayerDialog(playerid, DIALOGO_BANCO_TRANSFERIR_DESTINO, DIALOG_STYLE_INPUT,
                        "{E8B33A}Transferencia",
                        "{FFFFFF}Escribe el nombre completo de la cuenta de destino.\n\n\
                        {B4B5B7}Tal cual aparece en el juego, con guion bajo.\n\
                        Ejemplo: {FFFFFF}Juan_Perez\n\n\
                        {B4B5B7}No hace falta que este conectado.",
                        "Continuar", "Atras");
                }
                case 3: Banco_VerMovimientos(playerid);
            }
            return 1;
        }

        case DIALOGO_BANCO_INGRESAR:
        {
            if (!response) { Banco_MostrarMenu(playerid); return 1; }
            if (Banco_ExigirPresencia(playerid)) return 1;

            new cantidad = strval(inputtext);
            if (!EsNumeroEntero(inputtext) || cantidad < 1)
            {
                Error(playerid, "Escribe una cantidad valida.");
                return 1;
            }
            if (!TieneDinero(playerid, cantidad))
            {
                Error(playerid, "No llevas tanto dinero encima.");
                return 1;
            }

            DarDinero(playerid, -cantidad);
            Jugador[playerid][jBanco] += cantidad;
            BD_GuardarCuenta(playerid);

            Banco_Movimiento(Jugador[playerid][jID], 1, cantidad, Jugador[playerid][jBanco], "", "Ingreso en ventanilla");

            new textoCantidad[24], textoSaldo[24];
            FormatearDinero(cantidad, textoCantidad, sizeof textoCantidad);
            FormatearDinero(Jugador[playerid][jBanco], textoSaldo, sizeof textoSaldo);

            format(g_Buffer, sizeof g_Buffer, "Has ingresado %s. Saldo: %s.", textoCantidad, textoSaldo);
            Exito(playerid, g_Buffer);

            new nombre[MAX_PLAYER_NAME + 1], mensaje[160];
            NombreRP(playerid, nombre, sizeof nombre);
            format(mensaje, sizeof mensaje, "* %s entrega un fajo de billetes en la ventanilla.", nombre);
            MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
            return 1;
        }

        case DIALOGO_BANCO_RETIRAR:
        {
            if (!response) { Banco_MostrarMenu(playerid); return 1; }
            if (Banco_ExigirPresencia(playerid)) return 1;

            new cantidad = strval(inputtext);
            if (!EsNumeroEntero(inputtext) || cantidad < 1)
            {
                Error(playerid, "Escribe una cantidad valida.");
                return 1;
            }
            if (Jugador[playerid][jBanco] < cantidad)
            {
                Error(playerid, "No tienes tanto saldo en la cuenta.");
                return 1;
            }

            Jugador[playerid][jBanco] -= cantidad;
            // false: es tu propio dinero cambiando de bolsillo, no un
            // ingreso nuevo -- no debe sumar a "Ganancias ultima hora".
            DarDinero(playerid, cantidad, false);
            BD_GuardarCuenta(playerid);

            Banco_Movimiento(Jugador[playerid][jID], 2, -cantidad, Jugador[playerid][jBanco], "", "Retirada en ventanilla");

            new textoCantidad[24], textoSaldo[24];
            FormatearDinero(cantidad, textoCantidad, sizeof textoCantidad);
            FormatearDinero(Jugador[playerid][jBanco], textoSaldo, sizeof textoSaldo);

            format(g_Buffer, sizeof g_Buffer, "Has retirado %s. Saldo: %s.", textoCantidad, textoSaldo);
            Exito(playerid, g_Buffer);

            new nombre[MAX_PLAYER_NAME + 1], mensaje[160];
            NombreRP(playerid, nombre, sizeof nombre);
            format(mensaje, sizeof mensaje, "* El cajero le entrega un sobre a %s.", nombre);
            MensajeCercano(playerid, RADIO_ACCION, COLOR_ACCION, mensaje);
            return 1;
        }

        case DIALOGO_BANCO_TRANSFERIR_DESTINO:
        {
            if (!response) { Banco_MostrarMenu(playerid); return 1; }
            if (Banco_ExigirPresencia(playerid)) return 1;

            if (strlen(inputtext) < 3 || strlen(inputtext) > MAX_PLAYER_NAME)
            {
                Error(playerid, "Ese nombre no es valido.");
                return 1;
            }

            format(g_DestinoNombre[playerid], MAX_PLAYER_NAME + 1, "%s", inputtext);

            ShowPlayerDialog(playerid, DIALOGO_BANCO_TRANSFERIR_CANTIDAD, DIALOG_STYLE_INPUT,
                "{E8B33A}Transferencia",
                "{FFFFFF}Cuanto quieres enviar?\n\n{B4B5B7}Se anadira la comision del banco.",
                "Continuar", "Atras");
            return 1;
        }

        case DIALOGO_BANCO_TRANSFERIR_CANTIDAD:
        {
            if (!response) { Banco_MostrarMenu(playerid); return 1; }
            if (Banco_ExigirPresencia(playerid)) return 1;

            new cantidad = strval(inputtext);
            if (!EsNumeroEntero(inputtext) || cantidad < 1)
            {
                Error(playerid, "Escribe una cantidad valida.");
                return 1;
            }

            new destino[MAX_PLAYER_NAME + 1];
            format(destino, sizeof destino, "%s", g_DestinoNombre[playerid]);
            Banco_PrepararTransferencia(playerid, destino, cantidad);
            return 1;
        }

        case DIALOGO_BANCO_TRANSFERIR_CONCEPTO:
        {
            if (!response)
            {
                g_DestinoID[playerid] = 0;
                g_TransferenciaPendiente[playerid] = false;
                Banco_MostrarMenu(playerid);
                return 1;
            }

            new concepto[48];
            format(concepto, sizeof concepto, "%s", inputtext);
            LimpiarTexto(concepto);
            format(g_DestinoConcepto[playerid], 48, "%s", (concepto[0] == EOS) ? ("Transferencia") : (concepto));

            Banco_ConfirmarTransferencia(playerid);
            return 1;
        }

        case DIALOGO_BANCO_TRANSFERIR_CONFIRMAR:
        {
            if (!response)
            {
                g_DestinoID[playerid] = 0;
                g_TransferenciaPendiente[playerid] = false;
                Info(playerid, "Transferencia cancelada.");
                return 1;
            }

            Banco_EjecutarTransferencia(playerid);
            return 1;
        }

        case DIALOGO_BANCO_HISTORIAL:
        {
            Banco_MostrarMenu(playerid);
            return 1;
        }
    }
    return 0;
}
