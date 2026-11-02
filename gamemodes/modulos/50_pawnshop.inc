/* =====================================================================
 *  50_pawnshop.inc
 *  ---------------------------------------------------------------------
 *  Casa de Empeños, encargo de Vlad del 06/09/2026. Cierra el circuito
 *  del sistema de maleteros (45_maletero.inc): coche libre -> /hacker ->
 *  /forzarmaletero -> objeto en el inventario -> aqui -> /vender -> $.
 *
 *  Exterior: coordenada ya guardada en el panel (`coordenadas`, id 56,
 *  clave "pozzzzzision", nombre "casa venta"). Interior: "Zero's RC
 *  Shop" (interior 10) -- ver EMPENOS_INT_* en 01_configuracion.inc para
 *  la nota sobre de donde sale cada numero. Si el punto de aparicion no
 *  cae exacto, /setempeños (16_administracion.inc) lo corrige en vivo
 *  sin tocar codigo, mismo patron que /setbanco.
 *
 *  NO es un inventario ni un sistema paralelo: compra exactamente lo que
 *  ya vive en el inventario real del jugador (Inv_TieneItem/Inv_QuitarItem,
 *  10_inventario.inc) -- los 25 objetos Pawnshop (g_PawnshopCatalogo,
 *  45_maletero.inc), Materiales y Marihuana, ambos ya items normales del
 *  catalogo. Reutiliza /entrar y /salir genericos (15_banco.inc) igual
 *  que Tienda/Policia/Casas/Comida -- solo se añade una linea a cada uno
 *  de esos dos comandos, ver el diff en ese archivo.
 *
 *  PRECIO DE LA MARIHUANA -- no es un numero fijo (encargo expreso: nada
 *  de "numeros magicos" si cambia la economia). Se calcula en
 *  Empenos_PrecioMarihuana() a partir del catalogo REAL de la tienda
 *  (27_tienda.inc, Tienda_BuscarPorNombre) y el rendimiento de cultivo
 *  (MARIA_RENDIMIENTO, 28_marihuana.inc):
 *
 *      coste_por_gramo = precio_semillas / cantidad_semillas_por_compra / rendimiento
 *      precio_empeños  = coste_por_gramo * EMPENOS_MARIHUANA_MULTIPLICADOR
 *
 *  Con los valores de hoy (5.000$ / 10 semillas / 100 g cada una):
 *  coste = 5.000/10/100 = 5$/g -> empeños paga 10$/g (verificado con
 *  Vlad a mano antes de implementar, coincide).
 * ===================================================================== */

static STREAMER_TAG_PICKUP:g_EmpenosPickup;
static STREAMER_TAG_3D_TEXT_LABEL:g_EmpenosEtiquetaExt;
static STREAMER_TAG_3D_TEXT_LABEL:g_EmpenosEtiquetaInt;
static bool:g_EmpenosEtiquetaIntCreada = false;

// ---------------------------------------------------------------------
//  ARRANQUE
// ---------------------------------------------------------------------
stock Empenos_Iniciar()
{
    g_EmpenosPickup = CreateDynamicPickup(EMPENOS_PICKUP_MODELO, 23,
        EMPENOS_EXT_X, EMPENOS_EXT_Y, EMPENOS_EXT_Z, 0, 0, -1, 100.0);

    g_EmpenosEtiquetaExt = CreateDynamic3DTextLabel(
        "{53C168}Casa de empeños\n{FFFFFF}/Entrar",
        0x53C168FF, EMPENOS_EXT_X, EMPENOS_EXT_Y, EMPENOS_EXT_Z + 0.7, 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, 0, 0, -1, 30.0);

    Empenos_CrearEtiquetaInterior();
    return 1;
}

// Aparte porque /setempeños la recrea en el sitio nuevo (los 3D text
// labels no se pueden "mover", solo destruir y volver a crear).
stock Empenos_CrearEtiquetaInterior()
{
    if (g_EmpenosEtiquetaIntCreada)
    {
        DestroyDynamic3DTextLabel(g_EmpenosEtiquetaInt);
    }
    g_EmpenosEtiquetaIntCreada = true;

    // Delante del punto de aparicion (1 metro en +Y de donde se coloca al
    // entrar), mismo estilo que el resto de textos flotantes del server
    // (Transfender, bienvenida, guia) -- pedido expreso de Vlad.
    g_EmpenosEtiquetaInt = CreateDynamic3DTextLabel(
        "{53C168}Casa de empeños\n{FFFFFF}Usa /vender para hacer negocios.",
        0x53C168FF, g_EmpenosIntPos[0], g_EmpenosIntPos[1] + 1.0, g_EmpenosIntPos[2], 12.0,
        INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, g_EmpenosIntInterior, MUNDO_EMPENOS, -1, 30.0);
    return 1;
}

#define EMPENOS_ICON_SLOT 92   // libre entre BANCO_ICON_SLOT (89) y MVEND_ICON_SLOT (91)

// Icono de minimapa quitado el 11/09/2026 -- peticion expresa de Vlad
// de limpiar todos los iconos del mapa (la gente usa /gps). Se deja el
// stock vacio para no tocar el call site en OnPlayerConnect.
stock Empenos_OnConectar(playerid)
{
    #pragma unused playerid
    return 1;
}

// ---------------------------------------------------------------------
//  ENTRAR Y SALIR -- delegado desde CMD:entrar/CMD:salir (15_banco.inc),
//  mismo patron que Tienda_IntentarEntrar y compañia.
// ---------------------------------------------------------------------
stock Empenos_IntentarEntrar(playerid)
{
    if (GetPlayerInterior(playerid) != 0) return 0;
    if (!EnPunto(playerid, EMPENOS_EXT_RADIO, EMPENOS_EXT_X, EMPENOS_EXT_Y, EMPENOS_EXT_Z)) return 0;

    SetPlayerPos(playerid, g_EmpenosIntPos[0], g_EmpenosIntPos[1], g_EmpenosIntPos[2]);
    SetPlayerFacingAngle(playerid, g_EmpenosIntAngulo);
    SetPlayerInterior(playerid, g_EmpenosIntInterior);
    SetPlayerVirtualWorld(playerid, MUNDO_EMPENOS);
    SetCameraBehindPlayer(playerid);

    Info(playerid, "Casa de empeños. Usa /vender para hacer negocios.");
    return 1;
}

// A diferencia del banco, /salir funciona desde CUALQUIER punto del
// interior -- pedido expreso de Vlad, 06/09/2026 (el local es pequeno,
// exigir estar justo en el punto de aparicion era innecesario aqui).
stock Empenos_IntentarSalir(playerid)
{
    if (GetPlayerVirtualWorld(playerid) != MUNDO_EMPENOS) return 0;

    SetPlayerPos(playerid, EMPENOS_EXT_X, EMPENOS_EXT_Y, EMPENOS_EXT_Z);
    SetPlayerFacingAngle(playerid, EMPENOS_EXT_ANGULO);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, MUNDO_NORMAL);
    SetCameraBehindPlayer(playerid);
    return 1;
}

// ---------------------------------------------------------------------
//  PRECIOS -- unico punto de calculo, tanto para pintar el menu como
//  para cobrar de verdad (nunca dos formulas distintas).
// ---------------------------------------------------------------------
stock Empenos_PrecioMarihuana()
{
    new precioSemillas, cantidadSemillas;
    if (!Tienda_BuscarPorNombre("Semilla de marihuana", precioSemillas, cantidadSemillas) || cantidadSemillas <= 0 || MARIA_RENDIMIENTO <= 0)
    {
        return 0; // catalogo o rendimiento invalido -- mejor no vender a 0 costes raros que a un numero inventado
    }

    new Float:costePorGramo = (float(precioSemillas) / float(cantidadSemillas)) / float(MARIA_RENDIMIENTO);
    return floatround(costePorGramo * EMPENOS_MARIHUANA_MULTIPLICADOR);
}

// Precio por unidad de CUALQUIER item que la Casa de Empeños acepte, 0
// si no lo acepta. Punto unico -- el menu y el cobro final llaman aqui,
// nunca guardan un precio "de antes" para no vender a un precio viejo.
stock Empenos_PrecioUnidad(item)
{
    if (item == Item_Buscar("Materiales")) return EMPENOS_MATERIALES_PRECIO;
    if (item == Item_Buscar("Marihuana"))  return Empenos_PrecioMarihuana();

    new idx = Pawnshop_IndicePorItem(item);
    if (idx == -1) return 0;
    return g_PawnshopCatalogo[idx][ppValor];
}

// ---------------------------------------------------------------------
//  /VENDER -- solo dentro de la Casa de Empeños.
// ---------------------------------------------------------------------
#define EMPENOS_MAX_FILAS 30   // 25 objetos Pawnshop + Materiales + Marihuana, con margen

static g_EmpenosListaItem[MAX_PLAYERS][EMPENOS_MAX_FILAS];
static g_EmpenosListaCantidad[MAX_PLAYERS][EMPENOS_MAX_FILAS];
static g_EmpenosListaTotal[MAX_PLAYERS];

static g_EmpenosItemSel[MAX_PLAYERS];
static g_EmpenosCantidadMax[MAX_PLAYERS];
static g_EmpenosCantidadSel[MAX_PLAYERS];

CMD:vender(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    if (GetPlayerVirtualWorld(playerid) != MUNDO_EMPENOS)
    {
        Error(playerid, "Tienes que estar dentro de la Casa de Empeños.");
        return 1;
    }

    Empenos_MostrarMenu(playerid);
    return 1;
}

// Recorre SOLO lo que la Casa de Empeños acepta (25 objetos Pawnshop +
// Materiales + Marihuana) y se queda con lo que el jugador tiene de
// verdad ahora mismo -- nunca los 27 completos si solo lleva dos.
stock Empenos_ConstruirLista(playerid)
{
    new total = 0;

    for (new i = 0; i < sizeof g_PawnshopCatalogo; i++)
    {
        new item = g_PawnshopCatalogo[i][ppItem];
        new cantidad = Inv_TieneItem(playerid, item);
        if (cantidad <= 0) continue;

        g_EmpenosListaItem[playerid][total] = item;
        g_EmpenosListaCantidad[playerid][total] = cantidad;
        total++;
    }

    new itemMateriales = Item_Buscar("Materiales");
    new cantidadMateriales = Inv_TieneItem(playerid, itemMateriales);
    if (cantidadMateriales > 0)
    {
        g_EmpenosListaItem[playerid][total] = itemMateriales;
        g_EmpenosListaCantidad[playerid][total] = cantidadMateriales;
        total++;
    }

    new itemMarihuana = Item_Buscar("Marihuana");
    new cantidadMarihuana = Inv_TieneItem(playerid, itemMarihuana);
    if (cantidadMarihuana > 0)
    {
        g_EmpenosListaItem[playerid][total] = itemMarihuana;
        g_EmpenosListaCantidad[playerid][total] = cantidadMarihuana;
        total++;
    }

    g_EmpenosListaTotal[playerid] = total;
    return total;
}

stock Empenos_MostrarMenu(playerid)
{
    new total = Empenos_ConstruirLista(playerid);
    if (total == 0)
    {
        Info(playerid, "No llevas nada que nos interese.");
        return 1;
    }

    new lista[1200], linea[100], nombre[32], unidad[24], subtotal[24];
    format(lista, sizeof lista, "{E8B33A}Articulo\tPrecio\n");

    for (new i = 0; i < total; i++)
    {
        new item = g_EmpenosListaItem[playerid][i];
        new cantidad = g_EmpenosListaCantidad[playerid][i];
        new precio = Empenos_PrecioUnidad(item);

        Item_Nombre(item, nombre, sizeof nombre);
        FormatearDinero(precio, unidad, sizeof unidad);

        if (cantidad > 1)
        {
            FormatearDinero(precio * cantidad, subtotal, sizeof subtotal);
            format(linea, sizeof linea, "{FFFFFF}%s x%d\t{B4B5B7}%s (%s c/u)\n", nombre, cantidad, subtotal, unidad);
        }
        else
        {
            format(linea, sizeof linea, "{FFFFFF}%s\t{B4B5B7}%s\n", nombre, unidad);
        }
        strcat(lista, linea, sizeof lista);
    }

    ShowPlayerDialog(playerid, DIALOGO_EMPENOS, DIALOG_STYLE_TABLIST_HEADERS,
        "{E8973A}Casa de Empeños", lista, "Elegir", "Cerrar");
    return 1;
}

stock Empenos_MostrarConfirmacion(playerid, cantidad)
{
    g_EmpenosCantidadSel[playerid] = cantidad;

    new nombre[32], total[24];
    Item_Nombre(g_EmpenosItemSel[playerid], nombre, sizeof nombre);
    FormatearDinero(Empenos_PrecioUnidad(g_EmpenosItemSel[playerid]) * cantidad, total, sizeof total);

    format(g_Buffer, sizeof g_Buffer,
        "{FFFFFF}Vender:\n\n{E8B33A}%d %s\n\n{FFFFFF}Recibiras:\n{3D9970}%s",
        cantidad, nombre, total);

    ShowPlayerDialog(playerid, DIALOGO_EMPENOS_CONFIRMAR, DIALOG_STYLE_MSGBOX,
        "{E8973A}Casa de Empeños", g_Buffer, "Confirmar", "Cancelar");
    return 1;
}

stock Empenos_Dialogo(playerid, dialogid, response, listitem, const inputtext[])
{
    switch (dialogid)
    {
        case DIALOGO_EMPENOS:
        {
            if (!response) return 1;
            if (listitem < 0 || listitem >= g_EmpenosListaTotal[playerid]) return 1;

            g_EmpenosItemSel[playerid] = g_EmpenosListaItem[playerid][listitem];
            new cantidad = g_EmpenosListaCantidad[playerid][listitem];

            if (cantidad <= 1)
            {
                Empenos_MostrarConfirmacion(playerid, 1);
            }
            else
            {
                ShowPlayerDialog(playerid, DIALOGO_EMPENOS_ACCION, DIALOG_STYLE_LIST,
                    "{E8973A}Casa de Empeños", "Vender 1\nElegir cantidad\nVender todo", "Elegir", "Atras");
            }
            return 1;
        }

        case DIALOGO_EMPENOS_ACCION:
        {
            if (!response) { Empenos_MostrarMenu(playerid); return 1; }

            // Revalidado aqui tambien (no el numero que salio en la lista
            // hace un rato) -- puede haber cambiado si vendio/uso/tiro algo
            // entre medias.
            new maximo = Inv_TieneItem(playerid, g_EmpenosItemSel[playerid]);
            if (maximo <= 0)
            {
                Error(playerid, "Ya no tienes eso.");
                return 1;
            }

            switch (listitem)
            {
                case 0: Empenos_MostrarConfirmacion(playerid, 1);
                case 1:
                {
                    g_EmpenosCantidadMax[playerid] = maximo;
                    format(g_Buffer, sizeof g_Buffer, "Cuanto quieres vender? (1 a %d)", maximo);
                    ShowPlayerDialog(playerid, DIALOGO_EMPENOS_CANTIDAD, DIALOG_STYLE_INPUT,
                        "{E8973A}Casa de Empeños", g_Buffer, "Confirmar", "Cancelar");
                }
                case 2: Empenos_MostrarConfirmacion(playerid, maximo);
            }
            return 1;
        }

        case DIALOGO_EMPENOS_CANTIDAD:
        {
            if (!response) return 1;

            if (!EsNumeroEntero(inputtext) || strval(inputtext) < 1 || strval(inputtext) > g_EmpenosCantidadMax[playerid])
            {
                Error(playerid, "Cantidad invalida.");
                return 1;
            }

            Empenos_MostrarConfirmacion(playerid, strval(inputtext));
            return 1;
        }

        case DIALOGO_EMPENOS_CONFIRMAR:
        {
            if (!response) return 1;

            new item = g_EmpenosItemSel[playerid];
            new cantidad = g_EmpenosCantidadSel[playerid];

            // Revalidacion final contra el inventario REAL -- nunca fiarse
            // de lo que decia el dialogo anterior. Cantidad 0/negativa no
            // puede llegar aqui (ya se filtra en los pasos de arriba), pero
            // se comprueba de todas formas por si acaso.
            new disponibleReal = Inv_TieneItem(playerid, item);
            if (cantidad < 1 || cantidad > disponibleReal)
            {
                Error(playerid, "Ya no tienes esa cantidad. Vuelve a comprobarlo con /vender.");
                return 1;
            }

            // Precio recalculado en el instante de cobrar, no el que se
            // pinto al abrir el menu (la marihuana depende del precio de
            // la semilla, que puede cambiar).
            new precioUnidad = Empenos_PrecioUnidad(item);
            if (precioUnidad <= 0)
            {
                Error(playerid, "Ahora mismo no compramos eso.");
                return 1;
            }

            if (!Inv_QuitarItem(playerid, item, cantidad))
            {
                Error(playerid, "No se ha podido completar la venta.");
                return 1;
            }

            new total = precioUnidad * cantidad;
            DarDinero(playerid, total);

            new nombre[32], valorTexto[24];
            Item_Nombre(item, nombre, sizeof nombre);
            FormatearDinero(total, valorTexto, sizeof valorTexto);
            format(g_Buffer, sizeof g_Buffer, "Has vendido %d %s por %s.", cantidad, nombre, valorTexto);
            Exito(playerid, g_Buffer);

            format(g_Buffer, sizeof g_Buffer, "EMPENOS_VENTA cuenta=%d item=%d cantidad=%d total=%d",
                Jugador[playerid][jID], item, cantidad, total);
            EscribirLog("economia", g_Buffer);
            return 1;
        }
    }
    return 0;
}
