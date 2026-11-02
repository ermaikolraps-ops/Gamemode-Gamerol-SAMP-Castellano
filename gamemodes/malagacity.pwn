/* =====================================================================
 *
 *   ???????  ?????? ????   ???????????????????  ??????? ???
 *  ???????? ????????????? ?????????????????????????????????
 *  ???  ?????????????????????????????  ???????????   ??????
 *  ???   ????????????????????????????  ???????????   ??????
 *  ????????????  ?????? ??? ??????????????  ????????????????????
 *   ??????? ???  ??????     ??????????????  ??? ??????? ????????
 *
 *   G A M E R O L   2 . 0   -   T U   S E G U N D A   V I D A
 *   Framework de rol para open.mp / SA-MP  ·  Los Santos, 1992
 *
 *   ------------------------------------------------------------------
 *   ESTRUCTURA DEL PROYECTO
 *   ------------------------------------------------------------------
 *   Este archivo solo enlaza los modulos. Todo el contenido vive en
 *   gamemodes/modulos/ y cada archivo tiene una responsabilidad unica.
 *   El orden de inclusion importa: los modulos de abajo pueden usar
 *   lo definido en los de arriba.
 *
 *   01_configuracion  Todos los ajustes que vas a querer tocar.
 *   02_datos          Enums, variables globales y prototipos.
 *   03_utiles         Funciones auxiliares de uso general.
 *   04_comandos       Procesador de comandos (macro CMD:).
 *   05_basedatos      Conexion MySQL, carga y guardado de cuentas.
 *   06_login          Pantalla cinematografica, registro e inicio de sesion.
 *   08_chat           Chat de rol y comandos de comunicacion.
 *   09_cuenta         Panel /cuenta y estadisticas del personaje.
 *   10_inventario     Inventario con peso, objetos, armas y consumibles.
 *   11_trabajos       6 ranuras de trabajo (maximo 2 por jugador).
 *   12_facciones      6 ranuras de faccion (maximo 1 por jugador).
 *   13_vehiculos      Propiedad, control de acceso, persistencia y catalogo.
 *   14_tuning         Tuning persistente: guarda lo elegido en los talleres nativos de GTA:SA.
 *   15_banco          Cuenta bancaria y transferencias presenciales.
 *   16_administracion 6 niveles de staff, sanciones y registro.
 *   17_payday         Nomina por hora jugada.
 *   22_dropeo         Coches libres por el mapa y robo de coches ajenos.
 *   24_casas          Propiedades: compra, venta, bahul.
 *   23_panel          Panel de administracion.
 *   25_vendedorarmas  Trabajo de Vendedor de Armas (sistema de NGRP).
 *   26_nfs            Trabajo NFS: trafico de coches libres.
 *   27_tienda         Gasolinera 24h: bidon de gasolina y picoteo.
 *   28_marihuana      Cultivo casero al aire libre y venta.
 *   29_vendedormarihuana  Trabajo de Vendedor de Marihuana: reparto a clientes NPC.
 *   30_gps            Comando /gps: ruta hacia los puntos de interes (sin blips en el mapa).
 *   31_web            Puente RCON para el panel web (renovar papeles, cosechar a distancia).
 *   32_negocios       10 ranuras de negocio: compra, caja fuerte, ingresos de la tienda.
 *   33_servicios      10 ranuras de servicio personal comprables solo desde la web (Ayudante de Vendedor).
 *   36_medicos        Trabajo Medicos Los Santos + sistema de caido/herido (reemplaza el respawn instantaneo).
 *   38_drift          Dinero por derrapar.
 *   39_atracos        Atracos a locales creados por un admin con /crearatraco.
 *   42_comida         Burger Shot, Cluckin' Bell y Well Stacked Pizza como negocios.
 *   51_pesca          Gamerol Aventuras: DLC Pesca -- cana, barbacoa, records y rangos.
 *   52_perritos       Puesto de Perritos Calientes (publico, sin negocio).
 *   53_arreglarcoche  Pay 'n' Spray casero -- repara el vehiculo por 1.000$.
 *   54_motel          Motel Angel Pine -- dos habitaciones de alquiler (24h reales).
 *   58_ascensos       Server IA, progresion de confianza por horas y entrada automatica a Policia.
 *   99_callbacks      Todos los callbacks de open.mp en un solo sitio.
 *
 *   ------------------------------------------------------------------
 *   COMPILAR:  doble clic en Compilar.bat
 *   ARRANCAR:  doble clic en Iniciar Servidor.bat
 *   ------------------------------------------------------------------
 * ===================================================================== */

#define MAX_PLAYERS             1000

#include <open.mp>
#include <a_mysql>
#include <streamer>
#include <sscanf2>
#include <samp_bcrypt>
// Reloj de hora real: rsetiawan7/samp-realtime-clock (vendorizada en
// qawno/include/, sin tocar). Va antes de los modulos porque engancha
// OnGameModeInit/OnPlayerConnect de 99_callbacks.inc mediante el patron
// ALS estandar de Pawn (ver 60_climahora.inc para el porque del cambio).
#include <realtime-clock>

#pragma dynamic                 16384
#pragma tabsize                 0

#include "modulos/00_sinweb.inc"
#include "modulos/01_configuracion.inc"
#include "modulos/02_datos.inc"
#include "modulos/03_utiles.inc"
#include "modulos/04_comandos.inc"
#include "modulos/05_basedatos.inc"
#include "modulos/06_login.inc"
#include "modulos/08_chat.inc"
#include "modulos/09_cuenta.inc"
#include "modulos/10_inventario.inc"
#include "modulos/11_trabajos.inc"
#include "modulos/12_facciones.inc"
#include "modulos/13_vehiculos.inc"
#include "modulos/14_tuning.inc"
// 35_cancelas se incluye aqui (no donde indicaria su numero) porque
// 16_administracion.inc usa g_Cancelas directamente, mismo motivo que
// 13_vehiculos.inc va antes que 16_administracion.inc.
#include "modulos/35_cancelas.inc"
#include "modulos/15_banco.inc"
#include "modulos/16_administracion.inc"
#include "modulos/17_payday.inc"
#include "modulos/22_dropeo.inc"
#include "modulos/24_casas.inc"
#include "modulos/23_panel.inc"
#include "modulos/25_vendedorarmas.inc"
#include "modulos/26_nfs.inc"
#include "modulos/27_tienda.inc"
#include "modulos/28_marihuana.inc"
#include "modulos/29_vendedormarihuana.inc"
#include "modulos/30_gps.inc"
#include "modulos/32_negocios.inc"
#include "modulos/33_servicios.inc"
#include "modulos/34_mercado.inc"
#include "modulos/36_medicos.inc"
#include "modulos/37_policia.inc"
#include "modulos/38_drift.inc"
#include "modulos/39_atracos.inc"
#include "modulos/40_tactil.inc"
#include "modulos/41_toys.inc"
#include "modulos/42_comida.inc"
#include "modulos/43_federico.inc"
#include "modulos/44_hambre_sed.inc"
#include "modulos/45_maletero.inc"
#include "modulos/47_bienvenida.inc"
#include "modulos/48_guia.inc"
#include "modulos/49_guiavehiculos.inc"
#include "modulos/50_pawnshop.inc"
#include "modulos/51_pesca.inc"
#include "modulos/52_perritos.inc"
#include "modulos/53_arreglarcoche.inc"
#include "modulos/54_motel.inc"
#include "modulos/55_ganancias.inc"
#include "modulos/56_consejos.inc"
#include "modulos/57_roboexpress.inc"
#include "modulos/58_ascensos.inc"
#include "modulos/59_federicojugadores.inc"
#include "modulos/60_climahora.inc"
#include "modulos/64_hospitalinterior.inc"
#include "modulos/65_cronos.inc"
#include "modulos/66_airdrop.inc"
#include "modulos/99_callbacks.inc"

main()
{
    print(" ");
    print("  ==================================================");
    print("   GAMEROL 2.0 - Framework cargado");
    print("  ==================================================");
    print(" ");
}
