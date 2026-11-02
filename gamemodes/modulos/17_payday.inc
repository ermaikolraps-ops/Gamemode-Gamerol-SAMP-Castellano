/* =====================================================================
 *  17_payday.inc
 *  ---------------------------------------------------------------------
 *  Nomina unica y fija: {PAYDAY_SUBVENCION}$ cada {PAYDAY_MINUTOS} minutos
 *  de juego efectivo, sin suplementos por trabajo ni por nada mas -- da
 *  igual el trabajo o faccion que tengas, cobras siempre lo mismo.
 *
 *  "Efectiva" quiere decir que los minutos en los que el jugador esta
 *  ausente (sin tocar nada durante mas de dos minutos) no cuentan. Asi
 *  no se puede dejar el personaje quieto toda la noche para cobrar.
 *
 *  El primer payday de una cuenta nueva llega a los PAYDAY_MINUTOS_PRIMERO
 *  minutos; los siguientes, cada PAYDAY_MINUTOS. El dinero se ingresa en
 *  la cuenta bancaria, no en el bolsillo.
 * ===================================================================== */

// ---------------------------------------------------------------------
//  ENTREGA DE LA NOMINA
// ---------------------------------------------------------------------
public Payday_Entregar(playerid)
{
    if (!Jugador[playerid][jLogueado] || !Jugador[playerid][jSpawneado]) return 0;

    new vehiculos = Veh_ContarDeJugador(playerid);
    new impuesto = vehiculos * NEGOCIO_IMPUESTO_VEHICULO;

    new neto = PAYDAY_SUBVENCION - impuesto;
    if (neto < 0) neto = 0;   // sin tope de vehiculos por jugador, esto SI puede pasar con muchos coches

    Jugador[playerid][jBanco]        += neto;
    Negocio_IngresoNomina();                          // %10 de PAYDAY_SUBVENCION, aparte, no le resta nada al jugador
    Negocio_IngresoImpuestoVehiculos(impuesto);        // esto SI sale del bolsillo del jugador
    Jugador[playerid][jMinutosPayday] = 0;
    Jugador[playerid][jPaydays]++;
    Jugador[playerid][jRespeto]++;

    // Subida de nivel: cada nivel cuesta cuatro nominas mas que el anterior.
    new bool:subioNivel = false;
    if (Jugador[playerid][jRespeto] >= Jugador[playerid][jNivel] * 4)
    {
        Jugador[playerid][jRespeto] = 0;
        Jugador[playerid][jNivel]++;
        SetPlayerScore(playerid, Jugador[playerid][jNivel]);
        subioNivel = true;
    }

    Banco_Movimiento(Jugador[playerid][jID], 5, neto, Jugador[playerid][jBanco], "", "Nomina");
    BD_GuardarCuenta(playerid);

    // --- Aviso en el chat, sin ventana -----------------------------------
    new tBruto[24], tNeto[24], tImpuesto[24];
    FormatearDinero(PAYDAY_SUBVENCION, tBruto, sizeof tBruto);
    FormatearDinero(neto, tNeto, sizeof tNeto);
    FormatearDinero(impuesto, tImpuesto, sizeof tImpuesto);

    format(g_Buffer, sizeof g_Buffer,
        "{E05A5A}(Nomina) {FFFFFF}%s{B4B5B7} - {E05A5A}(-%s x%d vehiculo%s){B4B5B7} - {3D9970}Recibido: %s",
        tBruto, tImpuesto, vehiculos, (vehiculos == 1) ? ("") : ("s"), tNeto);
    SendClientMessage(playerid, COLOR_BLANCO, g_Buffer);

    PlayerPlaySound(playerid, 1052, 0.0, 0.0, 0.0);

    if (subioNivel)
    {
        format(g_Buffer, sizeof g_Buffer, "~y~Nivel ~w~%d", Jugador[playerid][jNivel]);
        GameTextForPlayer(playerid, g_Buffer, 4000, 1);

        format(g_Buffer, sizeof g_Buffer, "Has subido al nivel %d.", Jugador[playerid][jNivel]);
        Exito(playerid, g_Buffer);
    }

    new nombre[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, nombre, sizeof nombre);
    format(g_Buffer, sizeof g_Buffer, "%s cobro nomina de %d (impuesto %d por %d vehiculos)", nombre, neto, impuesto, vehiculos);
    EscribirLog("economia", g_Buffer);
    return 1;
}

// ---------------------------------------------------------------------
//  TICK DE UN MINUTO
//  Se llama desde el temporizador global una vez por minuto.
// ---------------------------------------------------------------------
stock Payday_Minuto(playerid)
{
    if (!Jugador[playerid][jLogueado] || !Jugador[playerid][jSpawneado]) return 0;

    // Los minutos ausente no cuentan para la ayuda.
    if (Jugador[playerid][jAFK]) return 0;

    Jugador[playerid][jMinutosTotales]++;
    Jugador[playerid][jMinutosPayday]++;

    // La primera ayuda de una cuenta nueva llega casi al momento; las
    // siguientes, al ritmo normal.
    new umbral = (Jugador[playerid][jPaydays] == 0) ? PAYDAY_MINUTOS_PRIMERO : PAYDAY_MINUTOS;

    if (Jugador[playerid][jMinutosPayday] >= umbral)
    {
        Payday_Entregar(playerid);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDOS
// ---------------------------------------------------------------------
CMD:nomina(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new tBase[24], tImpuesto[24];
    new vehiculos = Veh_ContarDeJugador(playerid);
    new impuesto = vehiculos * NEGOCIO_IMPUESTO_VEHICULO;
    new neto = PAYDAY_SUBVENCION - impuesto;
    if (neto < 0) neto = 0;

    FormatearDinero(neto, tBase, sizeof tBase);
    FormatearDinero(impuesto, tImpuesto, sizeof tImpuesto);

    new umbral = (Jugador[playerid][jPaydays] == 0) ? PAYDAY_MINUTOS_PRIMERO : PAYDAY_MINUTOS;
    format(g_Buffer, sizeof g_Buffer, "Cobras {FFFFFF}%s{8BC4E8} cada nomina (ya descontado {FFFFFF}%s{8BC4E8} de impuestos por %d vehiculo%s). Te faltan {FFFFFF}%d minutos{8BC4E8}.",
        tBase, tImpuesto, vehiculos, (vehiculos == 1) ? ("") : ("s"), umbral - Jugador[playerid][jMinutosPayday]);
    Info(playerid, g_Buffer);

    if (Jugador[playerid][jAFK]) Aviso(playerid, "Ahora mismo estas ausente: estos minutos no cuentan.");
    return 1;
}
CMD:payday(playerid, params[]) return cmd_nomina(playerid, params);

// Comando de administracion para probar la ayuda sin esperar.
CMD:forzarnomina(playerid, params[])
{
    if (SinPermiso(playerid, ADMIN_JEFE)) return 1;

    new objetivoTexto[24];
    if (sscanf(params, "s[24]", objetivoTexto))
    {
        Uso(playerid, "/forzarnomina [id o nombre]");
        return 1;
    }

    new objetivo = BuscarJugador(objetivoTexto);
    if (JugadorInvalido(playerid, objetivo)) return 1;

    Payday_Entregar(objetivo);
    Exito(playerid, "Nomina entregada manualmente.");

    new cuenta[MAX_PLAYER_NAME + 1];
    GetPlayerName(objetivo, cuenta, sizeof cuenta);
    Admin_Registrar(playerid, "NOMINA", cuenta, "Entrega manual");
    return 1;
}
