/*
    41_toys.inc  Sistema de "toys" (objetos adjuntos al personaje)

    Reescrito el 01/09/2026 tras revisar el codigo REAL de Next
    Generation Role Play a fondo (la primera version, del 31/08, estaba
    mal: confundio los numeros de `ToyList.txt` con offsets de
    colocacion en el cuerpo). La realidad, comprobada linea a linea en
    el .pwn original:

    - `ToyList.txt` se carga con `LoadModelSelectionMenu()`
      (ICs/pawno/include/mSelection.inc), una libreria de vista previa
      en 3D para dialogos de tienda. Sus 4 numeros por linea son
      **rotacion y zoom DE LA CAMARA del dialogo de compra**, no del
      objeto en el hueso. No pintan nada aqui porque este modulo no
      replica ese dialogo 3D (se hace con lista de texto simple).
    - NGRP **no tiene una tabla de offsets calculados por objeto**. Todo
      toy se pone con un valor por defecto deliberadamente "mal puesto"
      (posicion 1.0, 1.0, 1.0 / rotacion 0 / escala 1.0 -- para que se
      note que hay que ajustarlo) y el propio jugador lo coloca a mano
      con el editor 3D nativo de SA-MP (`EditAttachedObject`, arrastrar
      con el raton en el juego), exactamente igual que hace
      GTA:SA/SA-MP con el editor de objetos de mapas. El resultado final
      se guarda en `OnPlayerEditAttachedObject` (99_callbacks.inc).

    Lo unico que SI se reutiliza de NGRP: el catalogo de IDs de modelo
    (que objetos existen como toy), sin sus rotaciones de camara. El
    resto -- comandos, edicion, persistencia, economia, permisos -- esta
    escrito para este gamemode: Jugador[][], DarDinero/TieneDinero,
    EsAdmin, g_BD con mysql_format/mysql_pquery, CMD:.

    RANURAS DE SetPlayerAttachedObject: ranuras 2-9 (8 toys por
    jugador). La ranura 1 la usa el porro de 28_marihuana.inc
    (PORRO_SLOT) -- no tocar.
*/

#define MAX_TOYS_JUGADOR       8
#define TOY_SLOT_BASE          2       // ranuras 2..9 de SetPlayerAttachedObject
#define TOY_HUESO_DEFECTO      1       // 1 = Spine (columna), igual que NGRP
#define TOY_PRECIO           25000     // precio fijo por toy (corregido 04/09/2026, era 750 por error)
#define TOY_ADMIN_MINIMO       ADMIN_MODERADOR // nivel minimo para /dartoy

// Nombres de los 18 huesos reales de SA-MP, mismo orden/indice que usaba
// NGRP (HoldingBones[] en variables.pwn) -- el indice ES el bone id.
new const g_NombreHueso[19][] = {
    "Ninguno", "Columna", "Cabeza", "Brazo sup. izq.", "Brazo sup. der.",
    "Mano izq.", "Mano der.", "Muslo izq.", "Muslo der.", "Pie izq.",
    "Pie der.", "Gemelo der.", "Gemelo izq.", "Antebrazo izq.",
    "Antebrazo der.", "Clavicula izq.", "Clavicula der.", "Cuello", "Mandibula"
};

// ---------------------------------------------------------------------
//  CATALOGO: solo los IDs de modelo (de NGRP, ToyList.txt), sin las
//  rotaciones de camara del dialogo original -- no aplican aqui.
// ---------------------------------------------------------------------
new const g_CatalogoToys[] = {
    18647, 365, 368, 373, 371, 1486, 1575, 1212, 348, 349, 352, 353, 343,
    326, 325, 355, 1484, 3028, 1279, 3027, 2114, 19348, 19349, 19350,
    19469, 19472, 19352, 19487, 351, 335, 356, 1550, 19347, 19418, 19514,
    19515, 19161, 19162, 18636, 19099, 19100, 18642, 18637, 358, 366, 334,
    19421, 19422, 19423, 19424, 2036, 2052, 2614, 11245, 19351, 19520,
    19521, 19516, 19517, 19518, 19519, 19488, 19308, 19309, 19310, 19311,
    2590, 347, 19330, 19331, 336, 341, 2035, 1000, 1001, 1002, 2044, 367,
    // Laseres (antes /shoplaser de NGRP)
    18643, // rojo
    19083, // verde
    19082, // naranja
    19084, // amarillo
    19081, // rosa
    19080  // azul
};
#define CANTIDAD_TOYS (sizeof g_CatalogoToys)

// ---------------------------------------------------------------------
//  DATOS POR JUGADOR
// ---------------------------------------------------------------------
enum E_TOY_JUGADOR
{
    tjID,               // id en la tabla `toys`, 0 = todavia no guardado
    tjModelo,           // 0 = ranura vacia
    tjHueso,
    Float:tjPosX, Float:tjPosY, Float:tjPosZ,
    Float:tjRotX, Float:tjRotY, Float:tjRotZ,
    Float:tjEscalaX, Float:tjEscalaY, Float:tjEscalaZ
}

new PlayerToy[MAX_PLAYERS][MAX_TOYS_JUGADOR][E_TOY_JUGADOR];
new bool:g_ToysCargados[MAX_PLAYERS];
new g_ToyEditando[MAX_PLAYERS] = {-1, ...}; // que ranura se esta editando ahora mismo, -1 = ninguna

// ---------------------------------------------------------------------
//  APLICAR / QUITAR (SetPlayerAttachedObject real)
// ---------------------------------------------------------------------
stock Toy_Aplicar(playerid, ranura)
{
    if (PlayerToy[playerid][ranura][tjModelo] < 1) return 0;

    SetPlayerAttachedObject(playerid, TOY_SLOT_BASE + ranura,
        PlayerToy[playerid][ranura][tjModelo], PlayerToy[playerid][ranura][tjHueso],
        PlayerToy[playerid][ranura][tjPosX], PlayerToy[playerid][ranura][tjPosY], PlayerToy[playerid][ranura][tjPosZ],
        PlayerToy[playerid][ranura][tjRotX], PlayerToy[playerid][ranura][tjRotY], PlayerToy[playerid][ranura][tjRotZ],
        PlayerToy[playerid][ranura][tjEscalaX], PlayerToy[playerid][ranura][tjEscalaY], PlayerToy[playerid][ranura][tjEscalaZ]);
    return 1;
}

stock Toy_Quitar(playerid, ranura)
{
    if (IsPlayerAttachedObjectSlotUsed(playerid, TOY_SLOT_BASE + ranura))
        RemovePlayerAttachedObject(playerid, TOY_SLOT_BASE + ranura);
    return 1;
}

// Llamado desde OnPlayerSpawn (99_callbacks.inc) -- SA-MP borra los
// objetos adjuntos al cambiar de piel/reaparecer, hay que reponerlos.
stock Toys_OnSpawn(playerid)
{
    if (!g_ToysCargados[playerid]) return 0;
    for (new i; i < MAX_TOYS_JUGADOR; i++)
    {
        if (PlayerToy[playerid][i][tjModelo] > 0) Toy_Aplicar(playerid, i);
    }
    return 1;
}

stock Toys_Limpiar(playerid)
{
    for (new i; i < MAX_TOYS_JUGADOR; i++)
    {
        PlayerToy[playerid][i][tjID] = 0;
        PlayerToy[playerid][i][tjModelo] = 0;
        PlayerToy[playerid][i][tjHueso] = 0;
        PlayerToy[playerid][i][tjPosX] = 0.0;
        PlayerToy[playerid][i][tjPosY] = 0.0;
        PlayerToy[playerid][i][tjPosZ] = 0.0;
        PlayerToy[playerid][i][tjRotX] = 0.0;
        PlayerToy[playerid][i][tjRotY] = 0.0;
        PlayerToy[playerid][i][tjRotZ] = 0.0;
        PlayerToy[playerid][i][tjEscalaX] = 1.0;
        PlayerToy[playerid][i][tjEscalaY] = 1.0;
        PlayerToy[playerid][i][tjEscalaZ] = 1.0;
    }
    g_ToysCargados[playerid] = false;
    g_ToyEditando[playerid] = -1;
    return 1;
}

stock Toy_PrimeraRanuraLibre(playerid)
{
    for (new i; i < MAX_TOYS_JUGADOR; i++)
        if (PlayerToy[playerid][i][tjModelo] == 0) return i;
    return -1;
}

stock Toy_IndiceCatalogoValido(modelo)
{
    for (new i; i < CANTIDAD_TOYS; i++)
        if (g_CatalogoToys[i] == modelo) return true;
    return false;
}

// ---------------------------------------------------------------------
//  EDICION EN VIVO (EditAttachedObject nativo de SA-MP -- arrastrar con
//  el raton en el juego, exactamente como hacia NGRP). El resultado
//  llega a OnPlayerEditAttachedObject, en 99_callbacks.inc.
// ---------------------------------------------------------------------
stock Toys_EmpezarEdicion(playerid, ranura)
{
    g_ToyEditando[playerid] = ranura;
    EditAttachedObject(playerid, TOY_SLOT_BASE + ranura);
    SendClientMessage(playerid, COLOR_GRIS, "Arrastra el objeto con el raton hasta dejarlo donde quieras. ESC para cancelar.");
    return 1;
}

// Llamado desde OnPlayerEditAttachedObject (99_callbacks.inc).
stock Toys_TerminarEdicion(playerid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, Float:sx, Float:sy, Float:sz)
{
    new ranura = g_ToyEditando[playerid];
    g_ToyEditando[playerid] = -1;
    if (ranura < 0 || ranura >= MAX_TOYS_JUGADOR) return 0;
    if (response != _:EDIT_RESPONSE_FINAL)
    {
        Toy_Aplicar(playerid, ranura); // response == CANCEL: se queda como estaba antes
        return 0;
    }

    PlayerToy[playerid][ranura][tjPosX] = x;
    PlayerToy[playerid][ranura][tjPosY] = y;
    PlayerToy[playerid][ranura][tjPosZ] = z;
    PlayerToy[playerid][ranura][tjRotX] = rx;
    PlayerToy[playerid][ranura][tjRotY] = ry;
    PlayerToy[playerid][ranura][tjRotZ] = rz;
    PlayerToy[playerid][ranura][tjEscalaX] = sx;
    PlayerToy[playerid][ranura][tjEscalaY] = sy;
    PlayerToy[playerid][ranura][tjEscalaZ] = sz;

    Toys_GuardarRanura(playerid, ranura);
    Info(playerid, "Posicion del toy guardada.");
    return 1;
}

// ---------------------------------------------------------------------
//  PERSISTENCIA
// ---------------------------------------------------------------------
public Toys_Cargar(playerid)
{
    Toys_Limpiar(playerid);

    new consulta[220];
    mysql_format(g_BD, consulta, sizeof consulta,
        "SELECT id, ranura, modelo, hueso, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, escala_x, escala_y, escala_z FROM toys WHERE cuenta_id = %d",
        Jugador[playerid][jID]);
    mysql_pquery(g_BD, consulta, "AlCargarToys", "d", playerid);
    return 1;
}

forward AlCargarToys(playerid);
public AlCargarToys(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new filas, ranura;
    cache_get_row_count(filas);

    for (new i; i < filas; i++)
    {
        cache_get_value_name_int(i, "ranura", ranura);
        if (ranura < 0 || ranura >= MAX_TOYS_JUGADOR) continue;

        cache_get_value_name_int(i, "id", PlayerToy[playerid][ranura][tjID]);
        cache_get_value_name_int(i, "modelo", PlayerToy[playerid][ranura][tjModelo]);
        cache_get_value_name_int(i, "hueso", PlayerToy[playerid][ranura][tjHueso]);
        cache_get_value_name_float(i, "pos_x", PlayerToy[playerid][ranura][tjPosX]);
        cache_get_value_name_float(i, "pos_y", PlayerToy[playerid][ranura][tjPosY]);
        cache_get_value_name_float(i, "pos_z", PlayerToy[playerid][ranura][tjPosZ]);
        cache_get_value_name_float(i, "rot_x", PlayerToy[playerid][ranura][tjRotX]);
        cache_get_value_name_float(i, "rot_y", PlayerToy[playerid][ranura][tjRotY]);
        cache_get_value_name_float(i, "rot_z", PlayerToy[playerid][ranura][tjRotZ]);
        cache_get_value_name_float(i, "escala_x", PlayerToy[playerid][ranura][tjEscalaX]);
        cache_get_value_name_float(i, "escala_y", PlayerToy[playerid][ranura][tjEscalaY]);
        cache_get_value_name_float(i, "escala_z", PlayerToy[playerid][ranura][tjEscalaZ]);
    }
    g_ToysCargados[playerid] = true;

    if (Jugador[playerid][jSpawneado]) Toys_OnSpawn(playerid);
    return 1;
}

stock Toys_GuardarRanura(playerid, ranura)
{
    new consulta[400];
    if (PlayerToy[playerid][ranura][tjID] > 0)
    {
        mysql_format(g_BD, consulta, sizeof consulta,
            "UPDATE toys SET modelo=%d, hueso=%d, pos_x=%f, pos_y=%f, pos_z=%f, rot_x=%f, rot_y=%f, rot_z=%f, escala_x=%f, escala_y=%f, escala_z=%f WHERE id=%d",
            PlayerToy[playerid][ranura][tjModelo], PlayerToy[playerid][ranura][tjHueso],
            PlayerToy[playerid][ranura][tjPosX], PlayerToy[playerid][ranura][tjPosY], PlayerToy[playerid][ranura][tjPosZ],
            PlayerToy[playerid][ranura][tjRotX], PlayerToy[playerid][ranura][tjRotY], PlayerToy[playerid][ranura][tjRotZ],
            PlayerToy[playerid][ranura][tjEscalaX], PlayerToy[playerid][ranura][tjEscalaY], PlayerToy[playerid][ranura][tjEscalaZ],
            PlayerToy[playerid][ranura][tjID]);
        mysql_pquery(g_BD, consulta);
    }
    else
    {
        mysql_format(g_BD, consulta, sizeof consulta,
            "INSERT INTO toys (cuenta_id, ranura, modelo, hueso, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, escala_x, escala_y, escala_z) VALUES (%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f)",
            Jugador[playerid][jID], ranura,
            PlayerToy[playerid][ranura][tjModelo], PlayerToy[playerid][ranura][tjHueso],
            PlayerToy[playerid][ranura][tjPosX], PlayerToy[playerid][ranura][tjPosY], PlayerToy[playerid][ranura][tjPosZ],
            PlayerToy[playerid][ranura][tjRotX], PlayerToy[playerid][ranura][tjRotY], PlayerToy[playerid][ranura][tjRotZ],
            PlayerToy[playerid][ranura][tjEscalaX], PlayerToy[playerid][ranura][tjEscalaY], PlayerToy[playerid][ranura][tjEscalaZ]);
        mysql_pquery(g_BD, consulta, "AlGuardarToyNuevo", "ii", playerid, ranura);
    }
    return 1;
}

forward AlGuardarToyNuevo(playerid, ranura);
public AlGuardarToyNuevo(playerid, ranura)
{
    if (!IsPlayerConnected(playerid)) return 0;
    if (ranura < 0 || ranura >= MAX_TOYS_JUGADOR) return 0;
    PlayerToy[playerid][ranura][tjID] = cache_insert_id();
    return 1;
}

public Toys_Guardar(playerid)
{
    if (!g_ToysCargados[playerid]) return 0;

    for (new i; i < MAX_TOYS_JUGADOR; i++)
    {
        if (PlayerToy[playerid][i][tjModelo] == 0)
        {
            if (PlayerToy[playerid][i][tjID] > 0)
            {
                new consulta[64];
                mysql_format(g_BD, consulta, sizeof consulta, "DELETE FROM toys WHERE id = %d", PlayerToy[playerid][i][tjID]);
                mysql_pquery(g_BD, consulta);
                PlayerToy[playerid][i][tjID] = 0;
            }
            continue;
        }
        Toys_GuardarRanura(playerid, i);
    }
    return 1;
}

// ---------------------------------------------------------------------
//  COMANDOS
// ---------------------------------------------------------------------

CMD:vertoys(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new pagina = 1;
    if (params[0] != 0) pagina = strval(params);
    if (pagina < 1) pagina = 1;

    new total_paginas = (CANTIDAD_TOYS + 9) / 10;
    if (pagina > total_paginas) pagina = total_paginas;

    new desde = (pagina - 1) * 10;
    new hasta = desde + 10;
    if (hasta > CANTIDAD_TOYS) hasta = CANTIDAD_TOYS;

    new linea[144];
    format(linea, sizeof linea, PREFIJO HEX_GRIS "-- Catalogo de toys, pagina %d/%d (/vertoys [pagina]) --", pagina, total_paginas);
    SendClientMessage(playerid, COLOR_GRIS, linea);

    for (new i = desde; i < hasta; i++)
    {
        format(linea, sizeof linea, "  %d) modelo %d", i + 1, g_CatalogoToys[i]);
        SendClientMessage(playerid, COLOR_GRIS, linea);
    }
    Info(playerid, "/toy [numero] para comprarlo. Sale mal puesto a proposito: usa /editartoy para colocarlo tu con el raton.");
    return 1;
}

// /toy [numero] -- compra y equipa en la primera ranura libre. Sale con
// la posicion por defecto de NGRP (1.0, 1.0, 1.0 / sin rotar / tamano
// normal) -- visiblemente descolocado a proposito, para que se note que
// hace falta /editartoy.
CMD:toy(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;
    if (!g_ToysCargados[playerid]) return Error(playerid, "Tus toys todavia se estan cargando, espera un segundo.");

    new indice;
    if (sscanf(params, "i", indice))
    {
        Uso(playerid, "/toy [numero del catalogo, ver /vertoys]");
        return 1;
    }

    if (indice < 1 || indice > CANTIDAD_TOYS) return Error(playerid, "Ese numero no existe. Mira /vertoys.");

    new ranura = Toy_PrimeraRanuraLibre(playerid);
    if (ranura == -1) return Error(playerid, "No te quedan ranuras libres de toys (maximo 8). Usa /quitartoy antes.");

    if (!TieneDinero(playerid, TOY_PRECIO))
    {
        new mensaje[64];
        format(mensaje, sizeof mensaje, "No tienes los $%d que cuesta.", TOY_PRECIO);
        return Error(playerid, mensaje);
    }

    DarDinero(playerid, -TOY_PRECIO);

    PlayerToy[playerid][ranura][tjModelo] = g_CatalogoToys[indice - 1];
    PlayerToy[playerid][ranura][tjHueso] = TOY_HUESO_DEFECTO;
    PlayerToy[playerid][ranura][tjPosX] = 1.0;
    PlayerToy[playerid][ranura][tjPosY] = 1.0;
    PlayerToy[playerid][ranura][tjPosZ] = 1.0;
    PlayerToy[playerid][ranura][tjRotX] = 0.0;
    PlayerToy[playerid][ranura][tjRotY] = 0.0;
    PlayerToy[playerid][ranura][tjRotZ] = 0.0;
    PlayerToy[playerid][ranura][tjEscalaX] = 1.0;
    PlayerToy[playerid][ranura][tjEscalaY] = 1.0;
    PlayerToy[playerid][ranura][tjEscalaZ] = 1.0;

    Toy_Aplicar(playerid, ranura);
    Toys_GuardarRanura(playerid, ranura);

    new aviso[112];
    format(aviso, sizeof aviso, "Te has comprado el toy %d (ranura %d). Sale mal puesto: usa /editartoy %d para colocarlo.", indice, ranura + 1, ranura + 1);
    Info(playerid, aviso);
    return 1;
}

// /editartoy [ranura] -- abre el editor 3D nativo de SA-MP (arrastrar
// con el raton), igual que hacia NGRP.
CMD:editartoy(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new ranura;
    if (sscanf(params, "i", ranura))
    {
        Uso(playerid, "/editartoy [ranura, ver /mistoys]");
        return 1;
    }
    ranura--;

    if (ranura < 0 || ranura >= MAX_TOYS_JUGADOR) return Error(playerid, "Esa ranura no existe (1-8).");
    if (PlayerToy[playerid][ranura][tjModelo] == 0) return Error(playerid, "Esa ranura esta vacia.");

    Toys_EmpezarEdicion(playerid, ranura);
    return 1;
}

// /huesotoy [ranura] [numero de hueso 1-18]
CMD:huesotoy(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new ranura, hueso;
    if (sscanf(params, "ii", ranura, hueso))
    {
        Uso(playerid, "/huesotoy [ranura] [hueso 1-18, ver /mistoys]");
        return 1;
    }
    ranura--;

    if (ranura < 0 || ranura >= MAX_TOYS_JUGADOR) return Error(playerid, "Esa ranura no existe (1-8).");
    if (PlayerToy[playerid][ranura][tjModelo] == 0) return Error(playerid, "Esa ranura esta vacia.");
    if (hueso < 1 || hueso > 18) return Error(playerid, "Ese hueso no existe (1-18).");

    Toy_Quitar(playerid, ranura);
    PlayerToy[playerid][ranura][tjHueso] = hueso;
    Toy_Aplicar(playerid, ranura);
    Toys_GuardarRanura(playerid, ranura);

    new aviso[80];
    format(aviso, sizeof aviso, "Toy movido al hueso: %s.", g_NombreHueso[hueso]);
    Info(playerid, aviso);
    return 1;
}

// /quitartoy [ranura 1-8]
CMD:quitartoy(playerid, params[])
{
    if (NoPuedeJugar(playerid)) return 1;

    new ranura;
    if (sscanf(params, "i", ranura))
    {
        Uso(playerid, "/quitartoy [ranura, ver /mistoys]");
        return 1;
    }
    ranura--;

    if (ranura < 0 || ranura >= MAX_TOYS_JUGADOR) return Error(playerid, "Esa ranura no existe (1-8).");
    if (PlayerToy[playerid][ranura][tjModelo] == 0) return Error(playerid, "Esa ranura ya esta vacia.");

    Toy_Quitar(playerid, ranura);
    PlayerToy[playerid][ranura][tjModelo] = 0;
    Toys_Guardar(playerid); // borra la fila (modelo == 0) en el siguiente guardado

    Info(playerid, "Te has quitado el toy.");
    return 1;
}

// /mistoys -- lista lo que llevas puesto.
CMD:mistoys(playerid, params[])
{
    #pragma unused params
    if (NoPuedeJugar(playerid)) return 1;

    SendClientMessage(playerid, COLOR_GRIS, PREFIJO HEX_GRIS "-- Tus toys --");
    new vacio = 1, linea[96];
    for (new i; i < MAX_TOYS_JUGADOR; i++)
    {
        if (PlayerToy[playerid][i][tjModelo] == 0) continue;
        vacio = 0;
        format(linea, sizeof linea, "  Ranura %d: modelo %d, hueso: %s", i + 1,
            PlayerToy[playerid][i][tjModelo], g_NombreHueso[PlayerToy[playerid][i][tjHueso]]);
        SendClientMessage(playerid, COLOR_GRIS, linea);
    }
    if (vacio) Info(playerid, "No llevas ningun toy puesto. Usa /toy [numero].");
    return 1;
}

// /dartoy [jugador] [numero] -- staff, gratis, mismo defecto sin colocar.
CMD:dartoy(playerid, params[])
{
    if (SinPermiso(playerid, TOY_ADMIN_MINIMO)) return 1;

    new objetivo, indice;
    if (sscanf(params, "ui", objetivo, indice))
    {
        Uso(playerid, "/dartoy [jugador] [numero del catalogo]");
        return 1;
    }
    if (objetivo == INVALID_PLAYER_ID) return Error(playerid, "Ese jugador no esta conectado.");
    if (indice < 1 || indice > CANTIDAD_TOYS) return Error(playerid, "Ese numero no existe. Mira /vertoys.");
    if (!g_ToysCargados[objetivo]) return Error(playerid, "Los toys de ese jugador todavia se estan cargando.");

    new ranura = Toy_PrimeraRanuraLibre(objetivo);
    if (ranura == -1) return Error(playerid, "Ese jugador no tiene ranuras libres (maximo 8).");

    PlayerToy[objetivo][ranura][tjModelo] = g_CatalogoToys[indice - 1];
    PlayerToy[objetivo][ranura][tjHueso] = TOY_HUESO_DEFECTO;
    PlayerToy[objetivo][ranura][tjPosX] = 1.0;
    PlayerToy[objetivo][ranura][tjPosY] = 1.0;
    PlayerToy[objetivo][ranura][tjPosZ] = 1.0;
    PlayerToy[objetivo][ranura][tjRotX] = 0.0;
    PlayerToy[objetivo][ranura][tjRotY] = 0.0;
    PlayerToy[objetivo][ranura][tjRotZ] = 0.0;
    PlayerToy[objetivo][ranura][tjEscalaX] = 1.0;
    PlayerToy[objetivo][ranura][tjEscalaY] = 1.0;
    PlayerToy[objetivo][ranura][tjEscalaZ] = 1.0;

    Toy_Aplicar(objetivo, ranura);
    Toys_GuardarRanura(objetivo, ranura);

    new aviso[112];
    format(aviso, sizeof aviso, "Un administrador te ha dado el toy %d (ranura %d). Usa /editartoy %d para colocarlo.", indice, ranura + 1, ranura + 1);
    Info(objetivo, aviso);
    format(aviso, sizeof aviso, "Le has dado el toy %d a %s (ranura %d).", indice, Jugador[objetivo][jNombre], ranura + 1);
    Info(playerid, aviso);
    return 1;
}
