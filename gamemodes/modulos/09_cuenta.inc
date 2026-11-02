/* =====================================================================
 *  09_cuenta.inc
 *  ---------------------------------------------------------------------
 *  Panel /cuenta, estadísticas del personaje, seguridad de la cuenta y
 *  el sistema de ayuda del servidor.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  /cuenta  ·  Ficha completa del personaje
// ---------------------------------------------------------------------
CMD:cuenta(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    Cuenta_MostrarFicha(playerid);
    return 1;
}
CMD:stats(playerid, params[]) return cmd_cuenta(playerid, params);
CMD:personaje(playerid, params[]) return cmd_cuenta(playerid, params);

stock Cuenta_MostrarFicha(playerid)
{
    new nombre[MAX_PLAYER_NAME + 1];
    NombreRP(playerid, nombre, sizeof nombre);

    new mano[24], banco[24], total[24];
    FormatearDinero(Jugador[playerid][jDinero], mano, sizeof mano);
    FormatearDinero(Jugador[playerid][jBanco], banco, sizeof banco);
    FormatearDinero(Jugador[playerid][jDinero] + Jugador[playerid][jBanco], total, sizeof total);

    new tiempo[48];
    FormatearMinutos(Jugador[playerid][jMinutosTotales], tiempo, sizeof tiempo);

    // --- Ocupación ---------------------------------------------------
    new ocupacion[256], trabajo[48], rango[48];
    ocupacion[0] = EOS;

    for (new i = 0; i < MAX_TRABAJOS_JUGADOR; i++)
    {
        if (Jugador[playerid][jTrabajo][i] == 0) continue;

        Trabajo_Nombre(Jugador[playerid][jTrabajo][i], trabajo, sizeof trabajo);
        Trabajo_NombreRango(Jugador[playerid][jTrabajo][i], Jugador[playerid][jTrabajoRango][i], rango, sizeof rango);

        format(ocupacion, sizeof ocupacion, "%s{FFFFFF}%s{B4B5B7} (%s)   ", ocupacion, trabajo, rango);
    }
    if (ocupacion[0] == EOS) format(ocupacion, sizeof ocupacion, "{E05A5A}En paro");

    new faccion[128], nombreFaccion[48], rangoFaccion[48];
    if (Jugador[playerid][jFaccion] > 0)
    {
        Faccion_Nombre(Jugador[playerid][jFaccion], nombreFaccion, sizeof nombreFaccion);
        Faccion_NombreRango(Jugador[playerid][jFaccion], Jugador[playerid][jFaccionRango], rangoFaccion, sizeof rangoFaccion);
        format(faccion, sizeof faccion, "{FFFFFF}%s{B4B5B7} - %s%s",
            nombreFaccion, rangoFaccion, (Jugador[playerid][jFaccionLider]) ? (" {E8B33A}[Lider]") : (""));
    }
    else format(faccion, sizeof faccion, "{B4B5B7}Ninguna");

    // --- Administración ----------------------------------------------
    new adminTexto[64];
    if (Jugador[playerid][jAdmin] > 0)
    {
        new rangoAdmin[32];
        Admin_NombreNivel(Jugador[playerid][jAdmin], rangoAdmin, sizeof rangoAdmin);
        format(adminTexto, sizeof adminTexto, "{E8973A}%s (nivel %d)", rangoAdmin, Jugador[playerid][jAdmin]);
    }
    else format(adminTexto, sizeof adminTexto, "{B4B5B7}Jugador");

    new correo[64];
    if (Jugador[playerid][jEmail][0] == EOS) format(correo, sizeof correo, "sin definir");
    else                                     format(correo, sizeof correo, "%s", Jugador[playerid][jEmail]);

    // Vehiculos: sin tope, se listan por nombre (no hay "de X").
    new vehiculosTexto[512];
    Veh_ListaTexto(playerid, vehiculosTexto, sizeof vehiculosTexto);

    // --- Cuerpo del diálogo ------------------------------------------
    new cuerpo[1600];
    format(cuerpo, sizeof cuerpo,
        "{E8B33A}%s{B4B5B7}   ~   %s de %d anos   ~   %s\n\n\
        {E8B33A}PERSONAJE\n\
        {B4B5B7}Nivel                {FFFFFF}%d{B4B5B7}      Respeto  {FFFFFF}%d\n\
        {B4B5B7}Tiempo jugado        {FFFFFF}%s\n\
        {B4B5B7}Nominas cobradas     {FFFFFF}%d{B4B5B7}      Proxima en {FFFFFF}%d min\n\n\
        {E8B33A}ECONOMIA\n\
        {B4B5B7}Dinero encima        {3D9970}%s\n\
        {B4B5B7}Cuenta bancaria      {3D9970}%s{B4B5B7}   (n%c %d)\n\
        {B4B5B7}Patrimonio total     {FFFFFF}%s\n\
        {B4B5B7}Vehiculos: %s\n\n\
        {E8B33A}OCUPACION\n\
        {B4B5B7}Trabajos   %s\n\
        {B4B5B7}Faccion    %s\n\n\
        {E8B33A}OTROS\n\
        {B4B5B7}Peso que cargas      {FFFFFF}%.1f{B4B5B7} de %.1f kg\n\
        {B4B5B7}Rango administrativo %s\n\
        {B4B5B7}Avisos acumulados    {FFFFFF}%d{B4B5B7} de 3\n\
        {B4B5B7}Correo de contacto   {FFFFFF}%s",
        nombre,
        (Jugador[playerid][jSexo] == 2) ? ("Mujer") : ("Hombre"),
        Jugador[playerid][jEdad],
        Jugador[playerid][jOrigen],
        Jugador[playerid][jNivel],
        Jugador[playerid][jRespeto],
        tiempo,
        Jugador[playerid][jPaydays],
        PAYDAY_MINUTOS - Jugador[playerid][jMinutosPayday],
        mano,
        banco, 186, Jugador[playerid][jCuentaBancaria],
        total,
        vehiculosTexto,
        ocupacion,
        faccion,
        Inv_PesoActual(playerid), Inv_PesoMaximo(playerid),
        adminTexto,
        Jugador[playerid][jAvisos],
        correo);

    ShowPlayerDialog(playerid, DIALOGO_CUENTA, DIALOG_STYLE_MSGBOX,
        "{E8B33A}Tu ficha de personaje", cuerpo, "Opciones", "Cerrar");
    return 1;
}

// ---------------------------------------------------------------------
//  Menú de opciones de la cuenta
// ---------------------------------------------------------------------
stock Cuenta_MostrarOpciones(playerid)
{
    ShowPlayerDialog(playerid, DIALOGO_CUENTA_AJUSTES, DIALOG_STYLE_LIST,
        "{E8B33A}Opciones de la cuenta",
        "{FFFFFF}Cambiar la contrasena\n\
        {FFFFFF}Cambiar el correo de contacto\n\
        {FFFFFF}Ver mi historial de sanciones\n\
        {FFFFFF}Volver a la ficha",
        "Elegir", "Cerrar");
    return 1;
}

// ---------------------------------------------------------------------
//  Cambio de contraseña
// ---------------------------------------------------------------------
forward AlVerificarCambioPass(playerid, bool:correcta);
public AlVerificarCambioPass(playerid, bool:correcta)
{
    if (!IsPlayerConnected(playerid)) return 0;

    if (!correcta)
    {
        Error(playerid, "La contrasena actual no es correcta.");
        return 1;
    }

    ShowPlayerDialog(playerid, DIALOGO_CUENTA_CAMBIAR_PASS2, DIALOG_STYLE_PASSWORD,
        "{E8B33A}Nueva contrasena",
        "{FFFFFF}Escribe la contrasena nueva.\n\n{B4B5B7}Entre 6 y 32 caracteres.",
        "Cambiar", "Cancelar");
    return 1;
}

// ---------------------------------------------------------------------
//  Historial de sanciones
// ---------------------------------------------------------------------
stock Cuenta_VerSanciones(playerid)
{
    new consulta[256];
    format(consulta, sizeof consulta,
        "SELECT tipo, admin, motivo, duracion, DATE_FORMAT(fecha, '%%d/%%m/%%Y') AS dia \
         FROM sanciones WHERE cuenta_id = %d ORDER BY fecha DESC LIMIT 20", Jugador[playerid][jID]);

    mysql_pquery(g_BD, consulta, "AlCargarSanciones", "d", playerid);
    return 1;
}

forward AlCargarSanciones(playerid);
public AlCargarSanciones(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas;
    cache_get_row_count(filas);

    if (filas == 0)
    {
        ShowPlayerDialog(playerid, DIALOGO_CUENTA_SANCIONES, DIALOG_STYLE_MSGBOX,
            "{E8B33A}Historial de sanciones",
            "{53C168}Tu expediente esta limpio.\n\n{B4B5B7}No tienes ninguna sancion registrada. Sigue asi.",
            "Volver", "");
        return 1;
    }

    static const nombresTipo[][] = { "", "Aviso", "Expulsion", "Silencio", "Carcel", "Baneo" };

    new lista[1600], tipo, duracion, admin[24], motivo[128], dia[16], linea[240];

    format(lista, sizeof lista, "{E8B33A}Fecha        Tipo         Administrador   Motivo\n");

    for (new i = 0; i < filas; i++)
    {
        cache_get_value_name_int(i, "tipo", tipo);
        cache_get_value_name_int(i, "duracion", duracion);
        cache_get_value_name(i, "admin", admin, sizeof admin);
        cache_get_value_name(i, "motivo", motivo, sizeof motivo);
        cache_get_value_name(i, "dia", dia, sizeof dia);

        if (tipo < 1 || tipo > 5) tipo = 1;

        format(linea, sizeof linea, "{B4B5B7}%s   {FFFFFF}%s   {B4B5B7}%s   %s\n",
            dia, nombresTipo[tipo], admin, motivo);

        if (strlen(lista) + strlen(linea) >= sizeof(lista) - 8) break;
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_CUENTA_SANCIONES, DIALOG_STYLE_MSGBOX,
        "{E8B33A}Historial de sanciones", lista, "Volver", "");
    return 1;
}

// ---------------------------------------------------------------------
//  Respuestas a los diálogos de cuenta
// ---------------------------------------------------------------------
stock Cuenta_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    switch (dialogid)
    {
        case DIALOGO_CUENTA:
        {
            if (response) Cuenta_MostrarOpciones(playerid);
            return 1;
        }

        case DIALOGO_CUENTA_AJUSTES:
        {
            if (!response) return 1;

            switch (listitem)
            {
                case 0:
                {
                    ShowPlayerDialog(playerid, DIALOGO_CUENTA_CAMBIAR_PASS, DIALOG_STYLE_PASSWORD,
                        "{E8B33A}Cambiar contrasena",
                        "{FFFFFF}Escribe tu contrasena ACTUAL para confirmar que eres tu.",
                        "Continuar", "Cancelar");
                }
                case 1:
                {
                    ShowPlayerDialog(playerid, DIALOGO_CUENTA_EMAIL, DIALOG_STYLE_INPUT,
                        "{E8B33A}Correo de contacto",
                        "{FFFFFF}Escribe tu correo electronico.\n\n{B4B5B7}Sirve para recuperar la cuenta.",
                        "Guardar", "Cancelar");
                }
                case 2: Cuenta_VerSanciones(playerid);
                case 3: Cuenta_MostrarFicha(playerid);
            }
            return 1;
        }

        case DIALOGO_CUENTA_CAMBIAR_PASS:
        {
            if (!response) return 1;
            if (strlen(inputtext) < 1)
            {
                Error(playerid, "Tienes que escribir tu contrasena actual.");
                return 1;
            }
            bcrypt_verify(playerid, "AlVerificarCambioPass", inputtext, Jugador[playerid][jPassword]);
            return 1;
        }

        case DIALOGO_CUENTA_CAMBIAR_PASS2:
        {
            if (!response) return 1;

            new largo = strlen(inputtext);
            if (largo < PASSWORD_MIN || largo > PASSWORD_MAX)
            {
                format(g_Buffer, sizeof g_Buffer, "La contrasena debe tener entre %d y %d caracteres.", PASSWORD_MIN, PASSWORD_MAX);
                Error(playerid, g_Buffer);
                return 1;
            }

            Jugador[playerid][jTemporal2] = 2;
            bcrypt_hash(playerid, "AlHashearPassword", inputtext, COSTE_BCRYPT);
            Info(playerid, "Cifrando la contrasena nueva...");
            return 1;
        }

        case DIALOGO_CUENTA_EMAIL:
        {
            if (!response) return 1;

            if (strfind(inputtext, "@") == -1 || strfind(inputtext, ".") == -1 || strlen(inputtext) > 60)
            {
                Error(playerid, "Ese correo no parece valido.");
                return 1;
            }

            format(Jugador[playerid][jEmail], 64, "%s", inputtext);
            BD_GuardarCuenta(playerid);
            Exito(playerid, "Correo de contacto actualizado.");
            return 1;
        }

        case DIALOGO_CUENTA_SANCIONES:
        {
            Cuenta_MostrarOpciones(playerid);
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------
//  SISTEMA DE AYUDA
// ---------------------------------------------------------------------
CMD:ayuda(playerid, params[])
{
    if (NoLogueado(playerid)) return 1;

    ShowPlayerDialog(playerid, DIALOGO_AYUDA, DIALOG_STYLE_LIST,
        "{E8B33A}Ayuda de " SERVIDOR_NOMBRE_CORTO,
        "{E05A5A}Rol\n\
        {4FD1C5}Actividades\n\
        {4FD1C5}Sistemas\n\
        {E05A5A}Vehiculos\n\
        {53C168}Negocios\n\
        {FFFFFF}Trabajos\n\
        {FFFFFF}Personaje e inventario\n\
        {FFFFFF}Dinero y banco\n\
        {FFFFFF}Normas del servidor\n\
        {E8973A}Comandos de administracion",
        "Ver", "Cerrar");
    return 1;
}
CMD:comandos(playerid, params[]) return cmd_ayuda(playerid, params);
CMD:help(playerid, params[]) return cmd_ayuda(playerid, params);

CMD:normas(playerid, params[])
{
    Login_MostrarNormas(playerid);
    return 1;
}

stock Ayuda_MostrarSeccion(playerid, seccion)
{
    new cuerpo[2048], titulo[64];

    switch (seccion)
    {
        case 0:
        {
            format(titulo, sizeof titulo, "{E05A5A}Rol");
            format(cuerpo, sizeof cuerpo,
                "{E8B33A}HABLAR\n\
                {FFFFFF}texto{B4B5B7}      hablas en voz normal, chat de proximidad\n\
                {FFFFFF}/g{B4B5B7} texto   gritas (proximidad mas lejos)\n\
                {FFFFFF}/s{B4B5B7} texto   susurras (proximidad intimo)\n\n\
                {E8B33A}ACTUAR\n\
                {FFFFFF}/me{B4B5B7} accion       describe lo que hace tu personaje\n\
                {FFFFFF}/do{B4B5B7} situacion    describe el entorno o una situacion\n\
                {FFFFFF}/ame{B4B5B7} accion      igual que /me pero sobre tu cabeza\n\n\
                {E8B33A}FUERA DE PERSONAJE\n\
                {FFFFFF}/b{B4B5B7} texto     comentario OOC cercano\n\
                {FFFFFF}/ooc{B4B5B7} texto   OOC global (necesita nivel 2)\n\n\
                {E8B33A}UTILIDADES\n\
                {FFFFFF}/tiempo{B4B5B7}          hora del servidor y tu sesion\n\
                {FFFFFF}/duda{B4B5B7} pregunta   pregunta rapida al equipo (1 vez por minuto)");
        }
        case 1:
        {
            // Submenu propio, mismo patron que "Sistemas" -- ver
            // Actividades_MostrarLista mas abajo. Movido justo debajo de
            // "Rol" a peticion de Vlad el 07/09/2026 (antes case 8).
            Actividades_MostrarLista(playerid);
            return 1;
        }
        case 2:
        {
            // Submenu propio de 10 huecos (Dropeo, Marihuana, Casas, y 7
            // libres para lo que se vaya anadiendo) -- ver
            // Sistemas_MostrarLista mas abajo.
            Sistemas_MostrarLista(playerid);
            return 1;
        }
        case 3:
        {
            format(titulo, sizeof titulo, "{E05A5A}Vehiculos");
            format(cuerpo, sizeof cuerpo,
                "{FFFFFF}/comprarcoche{B4B5B7}     Comprar un vehiculo montado de conductor\n\
                {FFFFFF}/misveh{B4B5B7}           Lista tus vehiculos: modelo y tiempo restante de papeles\n\
                {FFFFFF}/taller{B4B5B7}           En el taller de la NFS: Renovar - Dropear\n\
                {FFFFFF}/abandonarcoche{B4B5B7}   Vende tu vehiculo por un precio simbolico, queda libre ahi mismo\n\n\
                {FFFFFF}/park{B4B5B7}             Guarda nueva posicion de respawn para tu vehiculo\n\
                {FFFFFF}/motor{B4B5B7}            Enciende y apaga el motor de tu coche\n\
                {FFFFFF}/luces{B4B5B7}            Enciende y apaga las luces de tu coche");
        }
        case 4:
        {
            format(titulo, sizeof titulo, "{53C168}Negocios");
            format(cuerpo, sizeof cuerpo,
                "{FFFFFF}/comprarnegocio{B4B5B7}   Junto al icono del dolar $, si esta libre\n\
                {FFFFFF}/soltarnegocio{B4B5B7}    Lo dejas libre ahi mismo, recibes %d%% de lo pagado\n\
                {FFFFFF}/vendernegocio{B4B5B7} jugador precio   Oferta a otro jugador\n\
                {FFFFFF}/aceptarnegocio{B4B5B7}   Acepta una oferta de venta\n\n\
                {E8B33A}CAJA FUERTE{B4B5B7} (solo el propietario)\n\
                {FFFFFF}/cajafuerte{B4B5B7}       Junto al icono: mirar saldo o sacar dinero\n\
                {FFFFFF}/vercajafuerte{B4B5B7}    Consulta el saldo desde cualquier sitio\n\n\
                {E8B33A}NEGOCIOS{B4B5B7}\n\
                {FF0000}Gasolinera Los Santos{B4B5B7}   %d%% de cada venta de esa tienda\n\
                {FF0000}Tienda Angel Pine{B4B5B7}   %d%% de cada venta de esa tienda\n\
                {FF0000}DGT{B4B5B7} (Banco)   100%% de cada /comprarcoche\n\
                {FF0000}Bonos del Estado{B4B5B7} (Banco)   40%% de tiendas, 10%% nominas, %d$ impuesto de circulacion, dinero restante de NFS\n\
                {FF0000}Burger Shot{B4B5B7}   100%% de cada /comer + ingresos pasivos de 5.000$ cada 24h {FF0000}(Comida){B4B5B7}\n\
                {FF0000}Cluckin' Bell{B4B5B7}   100%% de cada /comer + ingresos pasivos de 5.000$ cada 24h {FF0000}(Comida){B4B5B7}\n\
                {FF0000}Well Stacked Pizza{B4B5B7}   100%% de cada /comer + ingresos pasivos de 5.000$ cada 24h {FF0000}(Comida){B4B5B7}\n\
                {FF0000}Motel Angel Pine{B4B5B7}   100%% de cada /motel (1.000$) + ingresos pasivos de 3.500$ cada 24h {FF0000}(Habitaciones){B4B5B7}\n\n\
                {B4B5B7}Solo puedes tener un negocio a la vez.",
                NEGOCIO_PORCENTAJE_SOLTAR, NEGOCIO_PORCENTAJE_CAJA, NEGOCIO_PORCENTAJE_CAJA, NEGOCIO_IMPUESTO_VEHICULO);
        }
        case 5:
        {
            // La lista de verdad, la misma que mostraba el antiguo
            // comando /trabajos (ya retirado -- todo pasa por aqui).
            // Clickear una fila abre su ficha y marca el punto de
            // contratacion en el mapa, igual que antes.
            Trabajo_MostrarLista(playerid);
            return 1;
        }
        case 6:
        {
            format(titulo, sizeof titulo, "{E8B33A}Personaje e inventario");
            format(cuerpo, sizeof cuerpo,
                "{E8B33A}TU PERSONAJE\n\
                {FFFFFF}/cuenta /stats{B4B5B7}         ficha completa del personaje\n\
                {FFFFFF}/normas{B4B5B7}                normas del servidor\n\n\
                {E8B33A}INVENTARIO{B4B5B7} (%d ranuras, limite de peso; una mochila lo amplia)\n\
                {FFFFFF}/inventario /inv{B4B5B7}       abre tu inventario\n\
                {FFFFFF}/usar{B4B5B7} objeto           usa un objeto directamente\n\
                {FFFFFF}/tirar{B4B5B7} objeto cantidad deja un objeto en el suelo\n\
                {FFFFFF}/dar{B4B5B7} id objeto cant.   entrega un objeto a alguien al lado\n\
                {FFFFFF}/coger{B4B5B7}                 recoge lo que haya en el suelo\n\
                {FFFFFF}/peso{B4B5B7}                  cuanto cargas y cuanto aguantas\n\n\
                {E8B33A}INTERACCION\n\
                {FFFFFF}/pagar{B4B5B7} id cantidad     entrega dinero en mano", MAX_SLOTS_INVENTARIO);
        }
        case 7:
        {
            format(titulo, sizeof titulo, "{E8B33A}Dinero y banco");
            format(cuerpo, sizeof cuerpo,
                "{FFFFFF}/nomina{B4B5B7}                 %d$ cada %d min. de juego\n\
                {FFFFFF}/banco{B4B5B7}                   terminal bancario (dentro del banco, o en el punto de Angel Pine)\n\
                {FFFFFF}/saldo{B4B5B7}                   consulta rapida del saldo\n\
                {FFFFFF}/transferir{B4B5B7} nombre cant. envia dinero a otra cuenta\n\
                {FFFFFF}/gps{B4B5B7}                     te lleva al banco y a los demas puntos",
                PAYDAY_SUBVENCION, PAYDAY_MINUTOS);
        }
        case 8:
        {
            Login_MostrarNormas(playerid);
            return 1;
        }
        case 9:
        {
            if (Jugador[playerid][jAdmin] == 0)
            {
                Error(playerid, "Esa seccion es solo para el equipo administrativo.");
                return 1;
            }
            return Admin_MostrarComandos(playerid);
        }
    }

    ShowPlayerDialog(playerid, DIALOGO_AYUDA_SECCION, DIALOG_STYLE_MSGBOX, titulo, cuerpo, "Volver", "Cerrar");
    return 1;
}

// ---------------------------------------------------------------------
//  AYUDA > SISTEMAS
//  Submenu de 10 huecos para explicar sistemas grandes del servidor
//  (mas alla de rol/vehiculos/trabajos/dinero, que ya tienen su propio
//  apartado). De momento solo el 1 y el 2 estan rellenos; el resto son
//  huecos libres para lo que se vaya anadiendo -- para rellenar uno,
//  añade su nombre a la lista de abajo y su texto en
//  Sistemas_MostrarDetalle().
// ---------------------------------------------------------------------
stock Sistemas_MostrarLista(playerid)
{
    ShowPlayerDialog(playerid, DIALOGO_SISTEMAS, DIALOG_STYLE_LIST,
        "{4FD1C5}Sistemas del servidor",
        "{FFFFFF}Dropeo\n\
        {FFFFFF}Marihuana\n\
        {FFFFFF}Casas\n\
        {B4B5B7}(Libre)\n\
        {B4B5B7}(Libre)\n\
        {B4B5B7}(Libre)\n\
        {B4B5B7}(Libre)\n\
        {B4B5B7}(Libre)\n\
        {B4B5B7}(Libre)\n\
        {B4B5B7}(Libre)",
        "Ver", "Volver");
    return 1;
}

stock Sistemas_MostrarDetalle(playerid, indice)
{
    new cuerpo[900], titulo[64];

    switch (indice)
    {
        case 0:
        {
            format(titulo, sizeof titulo, "{4FD1C5}Sistemas > Dropeo");
            format(cuerpo, sizeof cuerpo,
                "{FFFFFF}/puente{B4B5B7}            coche CON dueno dejado sin /lock (30%% exito)\n\
                {FFFFFF}/dropearcoche{B4B5B7}       devuelve el coche puenteado al pool libre y cobras\n\
                {FFFFFF}/renovarpapeles{B4B5B7}     renueva los papeles de un coche tuyo\n\
                {FFFFFF}/taller{B4B5B7}             menu unico, la accion depende del coche\n\
                {FFFFFF}/abandonarcoche{B4B5B7}     tu coche se queda libre ahi mismo (precio simbolico)");
        }
        case 1:
        {
            format(titulo, sizeof titulo, "{4FD1C5}Sistemas > Marihuana");
            format(cuerpo, sizeof cuerpo,
                "{FFFFFF}/plantar{B4B5B7}                    siembra una semilla (24h para madurar)\n\
                {FFFFFF}/cosechar{B4B5B7}                    recoge tu cosecha lista\n\
                {FFFFFF}/arrancar{B4B5B7}                    arranca la de otro (te la quedas si esta lista)\n\
                {FFFFFF}/fumar{B4B5B7}                       lia un porro (2g) y dale caladas con click\n\
                {FFFFFF}/vendermaria{B4B5B7} id cant precio oferta a otro jugador\n\
                {FFFFFF}/aceptarmaria{B4B5B7}                acepta una oferta\n\n\
                {B4B5B7}Venta a clientes NPC: trabajo Vendedor de Marihuana ({FFFFFF}/ayuda{B4B5B7} > Trabajos)");
        }
        case 2:
        {
            format(titulo, sizeof titulo, "{4FD1C5}Sistemas > Casas");
            format(cuerpo, sizeof cuerpo,
                "{FFFFFF}/casas{B4B5B7}              catalogo, te marca la puerta en el mapa\n\
                {FFFFFF}/comprarcasa{B4B5B7}        en la puerta, si no tiene dueno\n\
                {FFFFFF}/vendercasa{B4B5B7}         en la puerta o dentro, si es tuya\n\n\
                {FFFFFF}/entrar /salir{B4B5B7}      mismos comandos que el banco y la tienda, detectan tu puerta solos\n\
                {FFFFFF}/abrir /cerrar{B4B5B7}       echar o quitar la llave, en la puerta o dentro\n\n\
                {FFFFFF}/armario{B4B5B7}            bahul e inventario en un dialogo, clic en una fila para mover el objeto");
        }
        default:
        {
            Error(playerid, "Ese hueco todavia esta libre.");
            Sistemas_MostrarLista(playerid);
            return 1;
        }
    }

    ShowPlayerDialog(playerid, DIALOGO_SISTEMAS_DETALLE, DIALOG_STYLE_MSGBOX, titulo, cuerpo, "Volver", "Cerrar");
    return 1;
}

// ---------------------------------------------------------------------
//  AYUDA > ACTIVIDADES
//  Mismo patron que "Sistemas" de arriba: submenu propio para explicar
//  las actividades "para jugar" del servidor (Gamerol Aventuras),
//  distintas de trabajos/negocios porque no dan nomina ni caja fuerte.
//  De momento Pesca y Barbacoa (51_pesca.inc, 06/09/2026) -- para añadir
//  una nueva, suma su nombre a la lista y su texto en
//  Actividades_MostrarDetalle().
// ---------------------------------------------------------------------
stock Actividades_MostrarLista(playerid)
{
    ShowPlayerDialog(playerid, DIALOGO_ACTIVIDADES, DIALOG_STYLE_LIST,
        "{4FD1C5}Actividades",
        "{FFFFFF}Pesca\n\
        {FFFFFF}Barbacoa",
        "Ver", "Volver");
    return 1;
}

stock Actividades_MostrarDetalle(playerid, indice)
{
    new cuerpo[1024], titulo[64];

    switch (indice)
    {
        case 0:
        {
            format(titulo, sizeof titulo, "{4FD1C5}Actividades > Pesca");
            format(cuerpo, sizeof cuerpo,
                "{E8B33A}COMO EMPEZAR\n\
                {FFFFFF}Compra una Cana de pescar (10.000$) en cualquier tienda.\n\n\
                {E8B33A}COMANDOS\n\
                {FFFFFF}/pescar{B4B5B7}          en un punto de pesca, con la cana encima (10s, 60%% de exito)\n\
                {FFFFFF}/venderpescado{B4B5B7}   en un puesto de venta, vende lo que has pescado\n\
                {FFFFFF}/pescas{B4B5B7}          tu rango, tus capturas y los records por especie\n\
                {FFFFFF}/rankingpesca{B4B5B7}    podio de los 3 con mas capturas y el record mundial\n\n\
                {E8B33A}DONDE PESCAR\n\
                {FFFFFF}Hay zonas en Los Santos y en Angel Pine. Usa {E8B33A}/gps{FFFFFF} y busca \"(Pescar)\" para llegar.\n\n\
                {E8B33A}DONDE VENDER\n\
                {FFFFFF}Tierra Robada Bait Shop y Angel Pine Bait Shop -- ambos en {E8B33A}/gps{FFFFFF}, busca \"(Venta Pescado)\".");
        }
        case 1:
        {
            format(titulo, sizeof titulo, "{4FD1C5}Actividades > Barbacoa");
            format(cuerpo, sizeof cuerpo,
                "{E8B33A}COMO EMPEZAR\n\
                {FFFFFF}Compra una Barbacoa (10.000$) en cualquier tienda.\n\n\
                {E8B33A}COMANDOS\n\
                {FFFFFF}/ponerbarbacoa{B4B5B7}     la despliega delante de ti (una por cuenta)\n\
                {FFFFFF}/cocinar{B4B5B7}           junto a la tuya: abre el menu de pescados para cocinar\n\
                {FFFFFF}/recogercocinado{B4B5B7}   recoge el plato antes de que se queme\n\
                {FFFFFF}/recogerbarbacoa{B4B5B7}   la guarda de vuelta en tu inventario\n\n\
                {E8B33A}COMER\n\
                {FFFFFF}/usar{B4B5B7} + nombre del plato -- vida al 100%% y algo de escudo.\n\n\
                {B4B5B7}Necesitas pescado crudo: consiguelo con {FFFFFF}/pescar{B4B5B7} (usa {E8B33A}/gps{B4B5B7} para llegar a un punto de pesca).");
        }
        default:
        {
            Error(playerid, "Ese hueco todavia esta libre.");
            Actividades_MostrarLista(playerid);
            return 1;
        }
    }

    ShowPlayerDialog(playerid, DIALOGO_ACTIVIDADES_DETALLE, DIALOG_STYLE_MSGBOX, titulo, cuerpo, "Volver", "Cerrar");
    return 1;
}

// ---------------------------------------------------------------------
//  /peso  ·  atajo rápido de carga
// ---------------------------------------------------------------------
CMD:peso(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    format(g_Buffer, sizeof g_Buffer, "Cargas {FFFFFF}%.1f kg{8BC4E8} de un maximo de {FFFFFF}%.1f kg{8BC4E8}.",
        Inv_PesoActual(playerid), Inv_PesoMaximo(playerid));
    Info(playerid, g_Buffer);
    return 1;
}
