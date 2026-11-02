-- Edicion publica de Gamerol.

-- Estructura completa del modo. Sin cuentas, sin nombres y sin dinero de nadie.

-- Las casas, los negocios y los vehiculos estan, con el dueno vacio.

SET NAMES latin1;

SET FOREIGN_KEY_CHECKS=0;

/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*M!100616 SET @OLD_NOTE_VERBOSITY=@@NOTE_VERBOSITY, NOTE_VERBOSITY=0 */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `admin_logs` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `admin` varchar(24) NOT NULL,
  `nivel` tinyint(4) NOT NULL DEFAULT 0,
  `accion` varchar(32) NOT NULL,
  `objetivo` varchar(24) NOT NULL DEFAULT '',
  `detalles` varchar(160) NOT NULL DEFAULT '',
  `fecha` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  KEY `idx_fecha` (`fecha`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `antecedentes` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `cuenta_id` int(10) unsigned NOT NULL,
  `nombre` varchar(32) NOT NULL DEFAULT '',
  `codigo` smallint(6) NOT NULL DEFAULT 0,
  `delito` varchar(64) NOT NULL DEFAULT '',
  `puntos` smallint(6) NOT NULL DEFAULT 0,
  `agente` varchar(32) NOT NULL DEFAULT '',
  `fecha` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  KEY `idx_cuenta` (`cuenta_id`),
  KEY `idx_fecha` (`fecha`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `armas_stats` (
  `cuenta_id` int(10) unsigned NOT NULL,
  `skill` int(10) unsigned NOT NULL DEFAULT 0,
  `ventas_total` int(10) unsigned NOT NULL DEFAULT 0,
  `materiales_runs` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Mat runs completados',
  `fecha_alta` datetime NOT NULL DEFAULT current_timestamp(),
  `ultima_venta` datetime DEFAULT NULL,
  PRIMARY KEY (`cuenta_id`),
  CONSTRAINT `fk_armas_stats_cuenta` FOREIGN KEY (`cuenta_id`) REFERENCES `cuentas` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `atracos` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `nombre` varchar(40) NOT NULL,
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `interior` int(11) NOT NULL DEFAULT 0,
  `mundo` int(11) NOT NULL DEFAULT 0,
  `premio_min` int(10) unsigned NOT NULL,
  `premio_max` int(10) unsigned NOT NULL,
  `creado_por` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'cuentas.id del admin que lo creo',
  `creado_en` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `banco_movimientos` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `cuenta_id` int(10) unsigned NOT NULL,
  `tipo` tinyint(4) NOT NULL,
  `cantidad` int(11) NOT NULL,
  `saldo` int(11) NOT NULL COMMENT 'Saldo resultante',
  `contraparte` varchar(24) NOT NULL DEFAULT '',
  `concepto` varchar(64) NOT NULL DEFAULT '',
  `fecha` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  KEY `idx_cuenta_fecha` (`cuenta_id`,`fecha`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cancelas` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `propietario` int(10) unsigned NOT NULL COMMENT 'cuentas.id',
  `propietario_nombre` varchar(24) NOT NULL DEFAULT '',
  `modelo` int(11) NOT NULL DEFAULT 980,
  `x` float NOT NULL DEFAULT 0,
  `y` float NOT NULL DEFAULT 0,
  `z` float NOT NULL DEFAULT 0,
  `rot` float NOT NULL DEFAULT 0,
  `profundidad` float NOT NULL DEFAULT 4 COMMENT 'metros bajo tierra en el estado cerrado',
  `interior` int(11) NOT NULL DEFAULT 0,
  `mundo` int(11) NOT NULL DEFAULT 0,
  `estado` enum('pendiente_instalar','instalada','pendiente_mover') NOT NULL DEFAULT 'pendiente_instalar',
  `tiene_camara` tinyint(1) NOT NULL DEFAULT 0,
  `creado_en` timestamp NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  KEY `idx_propietario` (`propietario`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `casas` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `nombre` varchar(48) NOT NULL DEFAULT '',
  `interior_id` int(10) unsigned NOT NULL,
  `mundo_virtual` int(10) unsigned NOT NULL,
  `ext_x` float NOT NULL,
  `ext_y` float NOT NULL,
  `ext_z` float NOT NULL,
  `ext_a` float NOT NULL DEFAULT 0,
  `int_x` float NOT NULL,
  `int_y` float NOT NULL,
  `int_z` float NOT NULL,
  `int_a` float NOT NULL DEFAULT 0,
  `precio` int(10) unsigned NOT NULL,
  `propietario` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'cuentas.id, 0 = sin dueno',
  `propietario_nombre` varchar(24) NOT NULL DEFAULT '',
  `cerrada` tinyint(4) NOT NULL DEFAULT 1,
  `fecha_compra` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `idx_propietario` (`propietario`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `casas_almacen` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `casa_id` int(10) unsigned NOT NULL,
  `slot` tinyint(3) unsigned NOT NULL,
  `item` smallint(5) unsigned NOT NULL,
  `cantidad` int(10) unsigned NOT NULL DEFAULT 1,
  `datos` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_casa_slot` (`casa_id`,`slot`),
  CONSTRAINT `fk_casas_almacen_casa` FOREIGN KEY (`casa_id`) REFERENCES `casas` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `coordenadas` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `clave` varchar(24) NOT NULL,
  `nombre` varchar(48) NOT NULL,
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `a` float NOT NULL DEFAULT 0,
  `interior` int(10) unsigned NOT NULL DEFAULT 0,
  `mundo` int(10) unsigned NOT NULL DEFAULT 0,
  `creado_por` varchar(24) NOT NULL DEFAULT '',
  `fecha` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_clave` (`clave`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cronos` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `nombre` varchar(48) NOT NULL,
  `salida_x` float NOT NULL,
  `salida_y` float NOT NULL,
  `salida_z` float NOT NULL,
  `salida_angulo` float NOT NULL DEFAULT 0,
  `meta_x` float DEFAULT NULL,
  `meta_y` float DEFAULT NULL,
  `meta_z` float DEFAULT NULL,
  `activo` tinyint(1) NOT NULL DEFAULT 0,
  `creado_por` varchar(24) NOT NULL DEFAULT '',
  `fecha` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_nombre` (`nombre`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cronos_ranking` (
  `crono_id` int(10) unsigned NOT NULL,
  `cuenta_id` int(10) unsigned NOT NULL,
  `nombre` varchar(24) NOT NULL,
  `tiempo_ms` int(10) unsigned NOT NULL,
  `vehiculo_modelo` smallint(5) unsigned NOT NULL DEFAULT 0,
  `fecha` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`crono_id`,`cuenta_id`),
  KEY `idx_crono_tiempo` (`crono_id`,`tiempo_ms`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cuentas` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `nombre` varchar(24) NOT NULL,
  `password` varchar(64) NOT NULL COMMENT 'Hash bcrypt (coste 12)',
  `email` varchar(64) NOT NULL DEFAULT '',
  `sexo` tinyint(3) unsigned NOT NULL DEFAULT 1 COMMENT '1 hombre / 2 mujer',
  `edad` tinyint(3) unsigned NOT NULL DEFAULT 18,
  `skin` smallint(5) unsigned NOT NULL DEFAULT 26,
  `skin_inicial` smallint(5) unsigned NOT NULL DEFAULT 0,
  `origen` varchar(32) NOT NULL DEFAULT 'Malaga',
  `nivel` smallint(5) unsigned NOT NULL DEFAULT 1,
  `respeto` int(10) unsigned NOT NULL DEFAULT 0,
  `minutos_totales` int(10) unsigned NOT NULL DEFAULT 0,
  `minutos_payday` smallint(5) unsigned NOT NULL DEFAULT 0,
  `paydays_cobrados` int(10) unsigned NOT NULL DEFAULT 0,
  `dinero` int(11) NOT NULL DEFAULT 0,
  `banco` int(11) NOT NULL DEFAULT 0,
  `cuenta_bancaria` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Numero de cuenta visible',
  `job1` tinyint(4) NOT NULL DEFAULT 0,
  `job1_rango` tinyint(4) NOT NULL DEFAULT 0,
  `job2` tinyint(4) NOT NULL DEFAULT 0,
  `job2_rango` tinyint(4) NOT NULL DEFAULT 0,
  `faccion` tinyint(4) NOT NULL DEFAULT 0,
  `faccion_rango` tinyint(4) NOT NULL DEFAULT 0,
  `faccion_lider` tinyint(4) NOT NULL DEFAULT 0,
  `salud` float NOT NULL DEFAULT 100,
  `armadura` float NOT NULL DEFAULT 0,
  `pos_x` float NOT NULL DEFAULT 1481,
  `pos_y` float NOT NULL DEFAULT -1771,
  `pos_z` float NOT NULL DEFAULT 18.8,
  `pos_a` float NOT NULL DEFAULT 0,
  `interior` int(11) NOT NULL DEFAULT 0,
  `mundo` int(11) NOT NULL DEFAULT 0,
  `herido` tinyint(4) NOT NULL DEFAULT 0,
  `peso_extra` float NOT NULL DEFAULT 0 COMMENT 'Capacidad extra por mochilas',
  `hambre` float NOT NULL DEFAULT 100,
  `sed` float NOT NULL DEFAULT 100,
  `admin` tinyint(4) NOT NULL DEFAULT 0 COMMENT '0-6, 6 = Fundador',
  `avisos` tinyint(4) NOT NULL DEFAULT 0,
  `mute_segundos` int(11) NOT NULL DEFAULT 0,
  `carcel_segundos` int(11) NOT NULL DEFAULT 0,
  `baneado` tinyint(4) NOT NULL DEFAULT 0,
  `ban_motivo` varchar(128) NOT NULL DEFAULT '',
  `ban_admin` varchar(24) NOT NULL DEFAULT '',
  `ban_expira` datetime DEFAULT NULL,
  `tutorial` tinyint(4) NOT NULL DEFAULT 0,
  `telefono` int(10) unsigned NOT NULL DEFAULT 0,
  `ip` varchar(46) NOT NULL DEFAULT '',
  `fecha_registro` datetime NOT NULL DEFAULT current_timestamp(),
  `ultima_conexion` datetime DEFAULT NULL,
  `maria_ventas_npc` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Ventas completadas a clientes NPC (Vendedor de Marihuana)',
  `nfs_coches_vendidos` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Coches vendidos a otros jugadores como NFS',
  `maria_plantas_cosechadas` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Plantas cosechadas en total (in-game o desde la web)',
  `codigo_dani_usado` tinyint(1) NOT NULL DEFAULT 0,
  `buscado` int(11) NOT NULL DEFAULT 0,
  `condena_segundos` int(11) NOT NULL DEFAULT 0,
  `carcel_rol_segundos` int(11) NOT NULL DEFAULT 0,
  `carnet` tinyint(1) NOT NULL DEFAULT 0,
  `carnet_puntos` tinyint(4) NOT NULL DEFAULT 12,
  `federico_entregas` int(11) NOT NULL DEFAULT 0,
  `federico_secretos` int(10) unsigned NOT NULL DEFAULT 0,
  `hack_maleteros_hasta` datetime DEFAULT NULL,
  `hack_plantas_hasta` datetime DEFAULT NULL,
  `hack_federico_hasta` datetime DEFAULT NULL,
  `bienvenida_reclamada` tinyint(1) NOT NULL DEFAULT 0,
  `pesca_capturas` int(11) NOT NULL DEFAULT 0,
  `pesca_comidos` int(11) NOT NULL DEFAULT 0,
  `federico_historicos` int(10) unsigned NOT NULL DEFAULT 0,
  `ganancias_ventana` varchar(768) NOT NULL DEFAULT '0',
  `staff_rango` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `staff_ofertas` int(10) unsigned NOT NULL DEFAULT 0,
  `policia_ingreso` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_nombre` (`nombre`),
  KEY `idx_faccion` (`faccion`),
  KEY `idx_admin` (`admin`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `federico_chatarra` (
  `id` int(11) NOT NULL,
  `started_at` datetime NOT NULL,
  `next_rotation_at` datetime NOT NULL,
  `slot1_modelo` int(11) NOT NULL DEFAULT 0,
  `slot1_nombre` varchar(32) NOT NULL DEFAULT '',
  `slot1_entregado` tinyint(1) NOT NULL DEFAULT 0,
  `slot1_vehiculo_id` int(11) NOT NULL DEFAULT 0,
  `slot2_modelo` int(11) NOT NULL DEFAULT 0,
  `slot2_nombre` varchar(32) NOT NULL DEFAULT '',
  `slot2_entregado` tinyint(1) NOT NULL DEFAULT 0,
  `slot2_vehiculo_id` int(11) NOT NULL DEFAULT 0,
  `slot3_modelo` int(11) NOT NULL DEFAULT 0,
  `slot3_nombre` varchar(32) NOT NULL DEFAULT '',
  `slot3_entregado` tinyint(1) NOT NULL DEFAULT 0,
  `slot3_vehiculo_id` int(11) NOT NULL DEFAULT 0,
  `slot4_modelo` int(11) NOT NULL DEFAULT 0,
  `slot4_nombre` varchar(32) NOT NULL DEFAULT '',
  `slot4_entregado` tinyint(1) NOT NULL DEFAULT 0,
  `slot4_vehiculo_id` int(11) NOT NULL DEFAULT 0,
  `slot5_modelo` int(11) NOT NULL DEFAULT 0,
  `slot5_nombre` varchar(32) NOT NULL DEFAULT '',
  `slot5_entregado` tinyint(1) NOT NULL DEFAULT 0,
  `slot5_vehiculo_id` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `federico_estado` (
  `id` tinyint(3) unsigned NOT NULL,
  `started_at` datetime NOT NULL,
  `next_rotation_at` datetime NOT NULL,
  `slot1_modelo` smallint(5) unsigned NOT NULL DEFAULT 0,
  `slot1_nombre` varchar(32) NOT NULL DEFAULT '',
  `slot1_rareza` varchar(16) NOT NULL DEFAULT '',
  `slot1_recompensa` int(10) unsigned NOT NULL DEFAULT 0,
  `slot1_entregado` tinyint(1) NOT NULL DEFAULT 0,
  `slot1_vehiculo_id` int(10) unsigned NOT NULL DEFAULT 0,
  `slot2_modelo` smallint(5) unsigned NOT NULL DEFAULT 0,
  `slot2_nombre` varchar(32) NOT NULL DEFAULT '',
  `slot2_rareza` varchar(16) NOT NULL DEFAULT '',
  `slot2_recompensa` int(10) unsigned NOT NULL DEFAULT 0,
  `slot2_entregado` tinyint(1) NOT NULL DEFAULT 0,
  `slot2_vehiculo_id` int(10) unsigned NOT NULL DEFAULT 0,
  `slot3_modelo` smallint(5) unsigned NOT NULL DEFAULT 0,
  `slot3_nombre` varchar(32) NOT NULL DEFAULT '',
  `slot3_rareza` varchar(16) NOT NULL DEFAULT '',
  `slot3_recompensa` int(10) unsigned NOT NULL DEFAULT 0,
  `slot3_entregado` tinyint(1) NOT NULL DEFAULT 0,
  `slot3_vehiculo_id` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `federico_historico` (
  `id` int(11) NOT NULL,
  `personaje_id` int(11) NOT NULL DEFAULT 0,
  `vehiculo_id` int(11) NOT NULL DEFAULT 0,
  `estado` tinyint(4) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `federico_jugadores` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `slot` tinyint(3) unsigned NOT NULL,
  `vehiculo_id` int(11) NOT NULL,
  `propietario_id` int(11) NOT NULL,
  `modelo` smallint(5) unsigned NOT NULL,
  `rareza` varchar(16) NOT NULL,
  `recompensa` int(10) unsigned NOT NULL,
  `zona` varchar(16) NOT NULL,
  `entregado` tinyint(1) NOT NULL DEFAULT 0,
  `retorno_until` datetime DEFAULT NULL,
  `restaurado` tinyint(1) NOT NULL DEFAULT 0,
  `creado_en` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  KEY `idx_vehiculo` (`vehiculo_id`),
  KEY `idx_restaurado` (`restaurado`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `federico_moto` (
  `id` int(10) unsigned NOT NULL,
  `modelo` smallint(5) unsigned NOT NULL DEFAULT 0,
  `started_at` datetime NOT NULL,
  `next_rotation_at` datetime NOT NULL,
  `entregado` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `vehiculo_id` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `federico_secreto` (
  `id` tinyint(3) unsigned NOT NULL,
  `modelo` smallint(5) unsigned NOT NULL DEFAULT 0,
  `color1` int(10) unsigned NOT NULL DEFAULT 0,
  `color_nombre` varchar(24) NOT NULL DEFAULT '',
  `letra` char(1) NOT NULL DEFAULT '',
  `started_at` datetime NOT NULL,
  `next_rotation_at` datetime NOT NULL,
  `entregado` tinyint(1) NOT NULL DEFAULT 0,
  `vehiculo_id` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `guardias` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `angulo` float NOT NULL,
  `mundo` int(11) NOT NULL DEFAULT 0,
  `interior` int(11) NOT NULL DEFAULT 0,
  `skin` int(11) NOT NULL,
  `creado_por` int(11) NOT NULL,
  `creado_en` timestamp NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `inventario` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `cuenta_id` int(10) unsigned NOT NULL,
  `slot` tinyint(3) unsigned NOT NULL,
  `item` smallint(5) unsigned NOT NULL,
  `cantidad` int(11) NOT NULL DEFAULT 1,
  `datos` int(11) NOT NULL DEFAULT 0 COMMENT 'Municion, carga, calidad...',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_cuenta_slot` (`cuenta_id`,`slot`),
  CONSTRAINT `fk_inv_cuenta` FOREIGN KEY (`cuenta_id`) REFERENCES `cuentas` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `maletero` (
  `vehiculo_id` int(10) unsigned NOT NULL,
  `dinero` int(10) unsigned NOT NULL DEFAULT 0,
  `materiales` int(10) unsigned NOT NULL DEFAULT 0,
  `marihuana` int(10) unsigned NOT NULL DEFAULT 0,
  `semillas` int(10) unsigned NOT NULL DEFAULT 0,
  `portatiles` int(10) unsigned NOT NULL DEFAULT 0,
  `objeto` int(10) unsigned NOT NULL DEFAULT 0,
  `barbacoa` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`vehiculo_id`),
  CONSTRAINT `fk_maletero_vehiculo` FOREIGN KEY (`vehiculo_id`) REFERENCES `vehiculos` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `maletero_pescado` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `vehiculo_id` int(10) unsigned NOT NULL,
  `slot` tinyint(3) unsigned NOT NULL,
  `item` smallint(5) unsigned NOT NULL,
  `datos` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_maletero_pescado_slot` (`vehiculo_id`,`slot`),
  CONSTRAINT `fk_maletero_pescado_vehiculo` FOREIGN KEY (`vehiculo_id`) REFERENCES `vehiculos` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `mercado_anuncios` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `vendedor_id` int(10) unsigned NOT NULL,
  `categoria` enum('vehiculo') NOT NULL DEFAULT 'vehiculo',
  `vehiculo_id` int(10) unsigned DEFAULT NULL,
  `titulo` varchar(20) NOT NULL,
  `descripcion` varchar(200) NOT NULL DEFAULT '',
  `precio` int(10) unsigned NOT NULL,
  `foto_ruta` varchar(255) DEFAULT NULL,
  `estado` enum('pendiente','activo','vendido','caducado','cancelado') NOT NULL DEFAULT 'pendiente',
  `comprador_id` int(10) unsigned DEFAULT NULL,
  `creado_en` timestamp NOT NULL DEFAULT current_timestamp(),
  `expira_en` timestamp NULL DEFAULT NULL,
  `actualizado_en` timestamp NOT NULL DEFAULT current_timestamp() ON UPDATE current_timestamp(),
  PRIMARY KEY (`id`),
  KEY `idx_estado` (`estado`),
  KEY `idx_vendedor` (`vendedor_id`),
  KEY `idx_vehiculo` (`vehiculo_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `motel_habitaciones` (
  `habitacion` tinyint(3) unsigned NOT NULL,
  `cuenta_id` int(11) NOT NULL DEFAULT 0 COMMENT 'cuentas.id, 0 = libre',
  `cuenta_nombre` varchar(24) NOT NULL DEFAULT '',
  `expira` int(11) NOT NULL DEFAULT 0 COMMENT 'gettime() absoluto, 0 = libre',
  PRIMARY KEY (`habitacion`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `negocios` (
  `slot` tinyint(3) unsigned NOT NULL,
  `propietario` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'cuentas.id, 0 = sin dueno',
  `propietario_nombre` varchar(24) NOT NULL DEFAULT '',
  `caja_fuerte` int(10) unsigned NOT NULL DEFAULT 0,
  `precio_override` int(10) unsigned DEFAULT NULL COMMENT 'precio puesto por /cambiarprecionegocio; NULL = usar el de catalogo (32_negocios.inc)',
  `fecha_compra` datetime DEFAULT NULL,
  PRIMARY KEY (`slot`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `pesca_barbacoas` (
  `cuenta_id` int(11) NOT NULL,
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `a` float NOT NULL DEFAULT 0,
  `interior` int(11) NOT NULL DEFAULT 0,
  `mundo` int(11) NOT NULL DEFAULT 0,
  `cocinando_item` int(11) NOT NULL DEFAULT 0,
  `cocina_hasta` int(11) NOT NULL DEFAULT 0,
  `quema_hasta` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`cuenta_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `pesca_records` (
  `especie_id` tinyint(3) unsigned NOT NULL,
  `peso_gramos` int(11) NOT NULL DEFAULT 0,
  `cuenta_id` int(11) NOT NULL DEFAULT 0,
  `nombre` varchar(32) NOT NULL DEFAULT '',
  PRIMARY KEY (`especie_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `plantas` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `propietario` int(10) unsigned NOT NULL COMMENT 'cuentas.id',
  `propietario_nombre` varchar(24) NOT NULL DEFAULT '',
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `hora_lista` int(10) unsigned NOT NULL COMMENT 'gettime() en que estara lista para cosechar',
  `raza` varchar(16) NOT NULL DEFAULT 'WhiteWidow' COMMENT 'WhiteWidow/Amnesia/CriticalMass/Blueberry/Ak47',
  `agricultor` tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT '1 si la planto/replanto el Agricultor Personal',
  PRIMARY KEY (`id`),
  KEY `idx_propietario` (`propietario`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `sanciones` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `cuenta_id` int(10) unsigned NOT NULL DEFAULT 0,
  `nombre` varchar(24) NOT NULL,
  `tipo` tinyint(4) NOT NULL,
  `admin` varchar(24) NOT NULL,
  `motivo` varchar(128) NOT NULL DEFAULT '',
  `duracion` int(11) NOT NULL DEFAULT 0 COMMENT 'Segundos, 0 = permanente/instantaneo',
  `fecha` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  KEY `idx_nombre` (`nombre`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `servicios_jugador` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `cuenta_id` int(10) unsigned NOT NULL,
  `servicio_id` tinyint(3) unsigned NOT NULL,
  `activo` tinyint(3) unsigned NOT NULL DEFAULT 1,
  `rango` tinyint(3) unsigned NOT NULL DEFAULT 1,
  `comprado_en` datetime NOT NULL,
  `ultimo_tick` datetime NOT NULL,
  `ultimo_ascenso` datetime NOT NULL DEFAULT current_timestamp(),
  `aviso_stock_enviado` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `cuenta_servicio` (`cuenta_id`,`servicio_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `servidor_config` (
  `clave` varchar(48) NOT NULL,
  `valor` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`clave`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `toys` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `cuenta_id` int(10) unsigned NOT NULL,
  `ranura` tinyint(3) unsigned NOT NULL COMMENT '0-7, hasta 8 toys por jugador',
  `modelo` smallint(5) unsigned NOT NULL COMMENT 'ID de modelo real de GTA SA, ya no un indice de catalogo',
  `hueso` tinyint(3) unsigned NOT NULL DEFAULT 1,
  `pos_x` float NOT NULL DEFAULT 1,
  `pos_y` float NOT NULL DEFAULT 1,
  `pos_z` float NOT NULL DEFAULT 1,
  `rot_x` float NOT NULL DEFAULT 0,
  `rot_y` float NOT NULL DEFAULT 0,
  `rot_z` float NOT NULL DEFAULT 0,
  `escala_x` float NOT NULL DEFAULT 1,
  `escala_y` float NOT NULL DEFAULT 1,
  `escala_z` float NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_cuenta_ranura` (`cuenta_id`,`ranura`),
  CONSTRAINT `fk_toys_cuenta` FOREIGN KEY (`cuenta_id`) REFERENCES `cuentas` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `transfender_marcadores` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `nombre` varchar(48) NOT NULL,
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `creado_por` varchar(24) NOT NULL DEFAULT '',
  `fecha` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `vehiculo_tuning` (
  `veh_id` int(10) unsigned NOT NULL,
  `comp_0` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_1` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_2` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_3` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_4` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_5` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_6` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_7` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_8` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_9` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_10` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_11` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_12` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_13` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_14` smallint(5) unsigned NOT NULL DEFAULT 0,
  `comp_15` smallint(5) unsigned NOT NULL DEFAULT 0,
  `pintura` tinyint(4) NOT NULL DEFAULT -1 COMMENT '-1 = sin pintura personalizada',
  PRIMARY KEY (`veh_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `vehiculos` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `modelo` smallint(5) unsigned NOT NULL,
  `propietario` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'cuentas.id, 0 = sin dueno',
  `propietario_nombre` varchar(24) NOT NULL DEFAULT '',
  `tipo` tinyint(4) NOT NULL DEFAULT 0,
  `tipo_id` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'Indice de trabajo o faccion',
  `rango_minimo` tinyint(4) NOT NULL DEFAULT 0,
  `matricula` varchar(12) NOT NULL DEFAULT '',
  `color1` smallint(6) NOT NULL DEFAULT 1,
  `color2` smallint(6) NOT NULL DEFAULT 1,
  `pos_x` float NOT NULL DEFAULT 0,
  `pos_y` float NOT NULL DEFAULT 0,
  `pos_z` float NOT NULL DEFAULT 0,
  `pos_a` float NOT NULL DEFAULT 0,
  `interior` int(11) NOT NULL DEFAULT 0,
  `mundo` int(11) NOT NULL DEFAULT 0,
  `combustible` float NOT NULL DEFAULT 100,
  `salud` float NOT NULL DEFAULT 1000,
  `cerrado` tinyint(4) NOT NULL DEFAULT 1,
  `precio` int(11) NOT NULL DEFAULT 0,
  `precio_manual` tinyint(1) NOT NULL DEFAULT 0,
  `fecha_compra` datetime NOT NULL DEFAULT current_timestamp(),
  `dropeo_expira` datetime DEFAULT NULL COMMENT 'Solo tipo=4: fecha en la que caduca la propiedad',
  `es_nfs` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'Marcado con /nfscoche, solo se vende via NFS',
  `en_venta` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `kilometros` float NOT NULL DEFAULT 0 COMMENT 'Odometro persistente',
  `tiene_rastreador` tinyint(1) NOT NULL DEFAULT 0,
  `rastreador_x` float NOT NULL DEFAULT 0,
  `rastreador_y` float NOT NULL DEFAULT 0,
  `rastreador_actualizado_en` datetime DEFAULT NULL,
  `tiene_alarma` tinyint(1) NOT NULL DEFAULT 0,
  `federico_until` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `idx_propietario` (`propietario`),
  KEY `idx_tipo` (`tipo`,`tipo_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*M!100616 SET NOTE_VERBOSITY=@OLD_NOTE_VERBOSITY */;


SET FOREIGN_KEY_CHECKS=1;

INSERT INTO `cancelas` (`id`, `propietario`, `propietario_nombre`, `modelo`, `x`, `y`, `z`, `rot`, `profundidad`, `interior`, `mundo`, `estado`, `tiene_camara`, `creado_en`) VALUES
('10', 0, '', '980', '14819.9', '-7786.12', '4.24388', '342.765', '4', '0', '0', 'instalada', '0', '2026-08-20 00:44:22'),
('11', 0, '', '980', '15126.1', '7898.32', '8.6839', '275', '4', '0', '0', 'instalada', '0', '2026-08-20 01:03:50'),
('12', 0, '', '980', '2071.54', '-1831.5', '13.5469', '91.0632', '4', '0', '0', 'instalada', '1', '2026-08-31 22:28:18'),
('13', 0, '', '980', '1234.14', '-1302.51', '13.4072', '276.584', '4', '0', '0', 'instalada', '1', '2026-09-08 01:48:42'),
('14', 0, '', '980', '1228.79', '-1303.1', '13.4545', '107.696', '4', '0', '0', 'instalada', '1', '2026-09-08 01:48:48'),
('15', 0, '', '980', '1239', '-1307.1', '13.3633', '218.593', '4', '0', '0', 'instalada', '1', '2026-09-08 01:48:52');

INSERT INTO `casas` (`id`, `nombre`, `interior_id`, `mundo_virtual`, `ext_x`, `ext_y`, `ext_z`, `ext_a`, `int_x`, `int_y`, `int_z`, `int_a`, `precio`, `propietario`, `propietario_nombre`, `cerrada`, `fecha_compra`) VALUES
('1', 'Grove St 32', '2', '6001', '2459.46', '-1691.16', '13.5454', '184.003', '225.571', '1240.06', '1082.14', '0', '31000', 0, '', '1', NULL),
('2', 'Grove St 36', '1', '6002', '2513.95', '-1691.4', '14.046', '239.15', '224.288', '1289.19', '1082.14', '0', '32000', 0, '', '1', NULL),
('3', 'Grove St 38', '5', '6003', '2522.3', '-1679.12', '15.497', '269.23', '239.282', '1114.2', '1080.99', '0', '32700', 0, '', '1', NULL),
('4', 'Grove St 40', '15', '6004', '2524.35', '-1658.63', '15.4935', '273.303', '295.139', '1473.37', '1080.26', '0', '39000', 0, '', '1', NULL),
('5', 'Grove Street 42', '2', '6005', '2513.4', '-1650.56', '14.3557', '319.34', '446.626', '1397.74', '1084.3', '0', '33000', 0, '', '1', NULL),
('6', 'Grove Street 44', '5', '6006', '2498.37', '-1642.3', '14.1131', '358.798', '227.756', '1114.38', '1080.99', '0', '31000', 0, '', '1', NULL),
('7', 'Grove Street 46', '3', '6007', '2486.5', '-1644.82', '14.0772', '2.53428', '234.609', '1187.82', '1080.26', '0', '27000', 0, '', '1', NULL),
('8', 'Grove Street 48', '1', '6008', '2469.64', '-1646.57', '13.7801', '91.571', '224.288', '1289.19', '1082.14', '0', '29000', 0, '', '1', NULL),
('9', 'Grove St 2', '5', '6009', '2244.06', '-1638.29', '15.9074', '338.82', '239.282', '1114.2', '1080.99', '0', '80000', 0, '', '1', NULL),
('10', 'Grove St 03', '1', '6010', '2257.03', '-1644.64', '15.519', '356.994', '224.288', '1289.19', '1082.14', '0', '80000', 0, '', '1', NULL),
('11', 'Grove St 05', '15', '6011', '2282.18', '-1641.71', '15.8898', '359.501', '295.139', '1473.37', '1080.26', '0', '69000', 0, '', '1', NULL),
('12', 'Grove St 7', '3', '6012', '2307.02', '-1678.44', '14.0012', '180.273', '234.609', '1187.82', '1080.26', '0', '81000', 0, '', '1', NULL),
('13', 'Grove St 10', '2', '6013', '2368.16', '-1675.01', '14.1682', '183.719', '446.626', '1397.74', '1084.3', '0', '68000', 0, '', '1', NULL),
('14', 'Grove St 12', '2', '6014', '2362.74', '-1643.99', '13.5322', '0.730614', '446.626', '1397.74', '1084.3', '0', '70000', 0, '', '1', NULL),
('15', 'Grove St 14', '5', '6015', '2393.07', '-1646.47', '13.9051', '4.17748', '239.282', '1114.2', '1080.99', '0', '40000', 0, '', '1', NULL),
('16', 'Grove St 20', '2', '6016', '2384.48', '-1675.69', '15.2457', '181.189', '225.571', '1240.06', '1082.14', '0', '50000', 0, '', '1', NULL),
('18', 'Grove St 40', '5', '6018', '2413.8', '-1646.96', '14.0119', '6.034', '227.756', '1114.38', '1080.99', '0', '11000', 0, '', '1', NULL),
('19', 'Grove St 27', '2', '6019', '2451.89', '-1642.56', '13.7357', '1.02067', '225.571', '1240.06', '1082.14', '0', '20000', 0, '', '1', NULL),
('20', 'Grove St Sin Numero', '5', '6020', '2524.35', '-1702.63', '14.046', '50.6776', '239.282', '1114.2', '1080.99', '0', '12000', 0, '', '1', NULL),
('21', 'Ganton Av 12', '2', '6021', '2385.53', '-1711.95', '14.2422', '182.737', '446.626', '1397.74', '1084.3', '0', '55000', 0, '', '1', NULL),
('23', 'Ganton Av 18', '5', '6023', '2326.8', '-1716.88', '13.9074', '2.37702', '227.756', '1114.38', '1080.99', '0', '43000', 0, '', '1', NULL),
('24', 'RichHouse 1', '6', '6024', '228.304', '-1405.18', '51.6094', '152.907', '234.283', '1065.23', '1084.21', '0', '500000', 0, '', '1', NULL),
('25', 'RichHouse 2', '5', '6025', '189.825', '-1308.26', '70.2533', '92.1899', '227.756', '1114.38', '1080.99', '0', '2500000', 0, '', '1', NULL),
('26', 'RichHouse 3', '1', '6026', '255.139', '-1366.93', '53.1094', '117.787', '224.288', '1289.19', '1082.14', '0', '750000', 0, '', '1', NULL),
('33', 'Plazoletilla 12', '1', '6033', '1449.26', '-1848.67', '13.9407', '176.059', '224.288', '1289.19', '1082.14', '0', '80000', 0, '', '1', NULL),
('34', 'C/ Elgin N13', '5', '6034', '2379.79', '-1785.59', '13.5469', '270.879', '239.282', '1114.2', '1080.99', '0', '44000', 0, '', '1', NULL),
('35', 'C/ Elgin 16', '1', '6035', '2360.49', '-1795.5', '13.5469', '185.965', '224.288', '1289.19', '1082.14', '0', '41000', 0, '', '1', NULL),
('36', 'C/ Elgin 33', '15', '6036', '2344.59', '-1785.53', '13.5469', '270.879', '295.139', '1473.37', '1080.26', '0', '38000', 0, '', '1', NULL),
('37', 'C/ Elgin 44', '2', '6037', '2322.42', '-1796.1', '13.5469', '91.3372', '446.626', '1397.74', '1084.3', '0', '41200', 0, '', '1', NULL),
('38', 'Ganton Ave 20', '2', '6038', '2308.83', '-1715.05', '14.6496', '2.97644', '225.571', '1240.06', '1082.14', '0', '55000', 0, '', '1', NULL),
('39', 'Shaw Street 10', '5', '6039', '2157.03', '-1708.69', '15.0859', '180.301', '227.756', '1114.38', '1080.99', '0', '31000', 0, '', '1', NULL),
('40', 'Dawson Street 10', '5', '6040', '2014.95', '-1732.57', '14.2344', '268.952', '239.282', '1114.2', '1080.99', '0', '76000', 0, '', '1', NULL),
('41', 'Dawson Street 12', '2', '6041', '2015.26', '-1717.02', '13.5547', '271.459', '225.571', '1240.06', '1082.14', '0', '55000', 0, '', '1', NULL),
('42', 'Dawson Street 14', '1', '6042', '2016.91', '-1703.22', '14.1726', '269.869', '224.288', '1289.19', '1082.14', '0', '60000', 0, '', '1', NULL),
('43', 'Dawson Street 7', '15', '6043', '1982.28', '-1719.06', '16.7548', '90.3505', '295.139', '1473.37', '1080.26', '0', '80000', 0, '', '1', NULL),
('44', 'Dawson Street 11', '5', '6044', '1983.27', '-1682.91', '16.5572', '90.6637', '227.756', '1114.38', '1080.99', '0', '80000', 0, '', '1', NULL),
('45', 'Palmview street 10', '9', '6045', '2069.22', '-1703.36', '14.0342', '89.9904', '2319.13', '-1023.96', '1050.21', '0', '40000', 0, '', '1', NULL),
('46', 'Sunview street 13', '3', '6046', '2066.85', '-1717.19', '13.8058', '89.3638', '234.609', '1187.82', '1080.26', '0', '30000', 0, '', '1', NULL),
('47', 'AngelPine 1', '2', '6047', '-2134.53', '-2504.31', '31.8163', '51.5198', '446.626', '1397.74', '1084.3', '0', '175000', 0, '', '1', NULL),
('48', 'Angel Pine 2', '1', '6048', '-2160.93', '-2534.88', '31.8163', '140.484', '224.288', '1289.19', '1082.14', '0', '150000', 0, '', '1', NULL),
('49', 'Angel Pine 3', '2', '6049', '-2180.92', '-2519.76', '31.8163', '143.931', '225.571', '1240.06', '1082.14', '0', '140000', 0, '', '1', NULL),
('50', 'Angel Pine 5', '15', '6050', '-2173.97', '-2481.72', '31.8163', '320.339', '295.139', '1473.37', '1080.26', '0', '145000', 0, '', '1', NULL),
('51', 'Angel Pine 6#', '4', '6051', '-2224.27', '-2482.61', '31.8163', '140.82', '261.116', '1287.22', '1080.26', '0', '142000', 0, '', '1', NULL),
('52', 'Angel Pine 7', '1', '6052', '-2238.76', '-2424.2', '32.7073', '54.0497', '224.288', '1289.19', '1082.14', '0', '155000', 0, '', '1', NULL),
('53', 'Angel Pine 8', '15', '6053', '-2214.07', '-2451.9', '31.8163', '229.808', '295.139', '1473.37', '1080.26', '0', '132000', 0, '', '1', NULL),
('54', 'AngelPine 11', '2', '6054', '-2199.18', '-2511.83', '31.8163', '142.853', '446.626', '1397.74', '1084.3', '0', '124000', 0, '', '1', NULL),
('55', 'Angel Pine 13', '2', '6055', '-2219.49', '-2400.63', '32.5823', '45.8977', '225.571', '1240.06', '1082.14', '0', '155000', 0, '', '1', NULL),
('56', 'Angel Pine 14', '2', '6056', '-2132.49', '-2511.23', '31.8163', '314.789', '446.626', '1397.74', '1084.3', '0', '130000', 0, '', '1', NULL),
('58', 'Camping Pine 1', '10', '6058', '-2101.12', '-2532.35', '31.0668', '221.128', '419.894', '2537.12', '10', '0', '15000', 0, '', '1', NULL),
('59', 'Camping Pine 2', '10', '6059', '-2087.43', '-2510.33', '31.0668', '230.528', '419.894', '2537.12', '10', '0', '14790', 0, '', '1', NULL),
('60', 'Angel Pine 21', '8', '6060', '-2076.11', '-2312.32', '31.1313', '230.746', '2365.11', '-1133.08', '1050.88', '0', '175000', 0, '', '1', NULL),
('61', 'Parque 23', '1', '6061', '2090.9', '-1234.84', '25.6887', '177.987', '224.288', '1289.19', '1082.14', '0', '195000', 0, '', '1', NULL),
('62', 'Parque 26', '15', '6062', '2110.88', '-1244.05', '25.8516', '184.88', '295.139', '1473.37', '1080.26', '0', '210000', 0, '', '1', NULL),
('63', 'Casita Playa LS', '5', '6063', '315.882', '-1770.21', '4.64785', '359.835', '239.282', '1114.2', '1080.99', '0', '300000', 0, '', '1', NULL),
('64', 'Casita Playa 2 LS', '2', '6064', '295.326', '-1765.5', '4.54643', '2.02758', '446.626', '1397.74', '1084.3', '0', '225000', 0, '', '1', NULL),
('65', 'Calle chica SF 1', '2', '6065', '-2058.96', '890.102', '61.8535', '180.084', '225.571', '1240.06', '1082.14', '0', '110000', 0, '', '1', NULL),
('66', 'Calle Paz 1 LV', '5', '6066', '1846.27', '741.174', '11.4609', '87.0264', '227.756', '1114.38', '1080.99', '0', '315000', 0, '', '1', NULL),
('67', 'C/ Remedios 12 LV', '3', '6067', '1684.92', '2123.3', '11.4609', '90.0472', '234.609', '1187.82', '1080.26', '0', '190000', 0, '', '1', NULL),
('68', 'Cabina de las estrellas LS', '10', '6068', '1111.57', '-976.02', '42.7656', '180.523', '2261.1', '-1137.88', '1050.63', '0', '522000', 0, '', '1', NULL),
('72', 'Casa central 1', '5', '6072', '1379.24', '-1753.01', '14.1406', '93.094', '227.756', '1114.38', '1080.99', '0', '335000', 0, '', '1', NULL),
('73', 'Shaw Street N12', '5', '6073', '2139.1', '-1697.8', '15.0859', '357.705', '2233.69', '-1112.81', '1050.88', '0', '80000', 0, '', '1', NULL),
('74', 'Grove Street 67 ', '5', '6074', '2178.65', '-1599.98', '14.3454', '252.738', '2233.69', '-1112.81', '1050.88', '0', '150000', 0, '', '1', NULL),
('75', 'Calle Chica 10', '5', '6075', '2327.54', '-1681.89', '14.9297', '92.623', '2233.69', '-1112.81', '1050.88', '0', '125000', 0, '', '1', NULL),
('76', 'GroveStreet Blue', '2', '6076', '2408.99', '-1674.17', '13.6051', '183.467', '225.571', '1240.06', '1082.14', '0', '160000', 0, '', '1', NULL);

INSERT INTO `coordenadas` (`id`, `clave`, `nombre`, `x`, `y`, `z`, `a`, `interior`, `mundo`, `creado_por`, `fecha`) VALUES
('109', 'pos1', '/tratamiento medico', '2570.16', '1289.68', '866.276', '225.413', '0', '507', '', '2026-09-14 02:34:10');

INSERT INTO `cronos` (`id`, `nombre`, `salida_x`, `salida_y`, `salida_z`, `salida_angulo`, `meta_x`, `meta_y`, `meta_z`, `activo`, `creado_por`, `fecha`) VALUES
('2', 'LS-SF', '125.331', '-1550.13', '8.37257', '359.259', '-1760.62', '-632.739', '17.5524', '1', '', '2026-09-14 21:12:00'),
('3', 'Grove-Peaje', '2464.62', '-1658.71', '12.8575', '286.454', '69.5222', '-1529.16', '4.47749', '1', '', '2026-09-15 04:07:52'),
('4', 'Peaje-AngelPine', '45.2919', '-1529.36', '4.9116', '263.263', '-2102.28', '-2445.41', '30.0335', '1', '', '2026-09-16 00:18:55'),
('5', 'AngelPine - SF', '-2097.68', '-2448.84', '30.0304', '288.949', '-1637.72', '414.738', '6.60159', '1', '', '2026-09-16 04:08:37');

INSERT INTO `federico_chatarra` (`id`, `started_at`, `next_rotation_at`, `slot1_modelo`, `slot1_nombre`, `slot1_entregado`, `slot1_vehiculo_id`, `slot2_modelo`, `slot2_nombre`, `slot2_entregado`, `slot2_vehiculo_id`, `slot3_modelo`, `slot3_nombre`, `slot3_entregado`, `slot3_vehiculo_id`, `slot4_modelo`, `slot4_nombre`, `slot4_entregado`, `slot4_vehiculo_id`, `slot5_modelo`, `slot5_nombre`, `slot5_entregado`, `slot5_vehiculo_id`) VALUES
('1', '2026-11-01 21:17:29', '2026-11-02 21:17:29', '522', 'NRG-500', '0', '0', '448', 'Pizzaboy', '0', '0', '585', 'Emperor', '0', '0', '456', 'Yankee', '0', '0', '505', 'Rancher clasico', '0', '0');

INSERT INTO `federico_estado` (`id`, `started_at`, `next_rotation_at`, `slot1_modelo`, `slot1_nombre`, `slot1_rareza`, `slot1_recompensa`, `slot1_entregado`, `slot1_vehiculo_id`, `slot2_modelo`, `slot2_nombre`, `slot2_rareza`, `slot2_recompensa`, `slot2_entregado`, `slot2_vehiculo_id`, `slot3_modelo`, `slot3_nombre`, `slot3_rareza`, `slot3_recompensa`, `slot3_entregado`, `slot3_vehiculo_id`) VALUES
('1', '2026-11-01 21:17:29', '2026-11-02 21:17:29', '401', 'Bravura', 'RARE', '90000', '0', '0', '604', 'Glendale destartalado', 'RARE', '90000', '0', '0', '575', 'Broadway', 'DEL MONTON', '50000', '0', '0');

INSERT INTO `federico_historico` (`id`, `personaje_id`, `vehiculo_id`, `estado`) VALUES
('1', '3', '1118', '1');

INSERT INTO `federico_moto` (`id`, `modelo`, `started_at`, `next_rotation_at`, `entregado`, `vehiculo_id`) VALUES
('1', '522', '2026-11-01 21:17:29', '2026-11-02 21:17:29', '0', '0');

INSERT INTO `federico_secreto` (`id`, `modelo`, `color1`, `color_nombre`, `letra`, `started_at`, `next_rotation_at`, `entregado`, `vehiculo_id`) VALUES
('1', '415', '91', 'Azul marino', 'C', '2026-11-01 21:17:29', '2026-11-02 21:17:29', '0', '0');

INSERT INTO `guardias` (`id`, `x`, `y`, `z`, `angulo`, `mundo`, `interior`, `skin`, `creado_por`, `creado_en`) VALUES
('7', '2033.07', '-1720.7', '13.5469', '202.537', '0', '0', '71', 0, '2026-09-05 03:14:15'),
('16', '1547.67', '-1482.66', '13.5571', '180', '0', '0', '71', 0, '2026-09-05 04:39:33'),
('17', '1553.79', '-1482.11', '13.5567', '180', '0', '0', '71', 0, '2026-09-05 04:39:35');

INSERT INTO `motel_habitaciones` (`habitacion`, `cuenta_id`, `cuenta_nombre`, `expira`) VALUES
('0', 0, '', '0'),
('1', 0, '', '0');

INSERT INTO `negocios` (`slot`, `propietario`, `propietario_nombre`, `caja_fuerte`, `precio_override`, `fecha_compra`) VALUES
('0', 0, '', 0, '1000000', NULL),
('1', 0, '', 0, '1500000', NULL),
('2', 0, '', 0, '1500000', NULL),
('3', 0, '', 0, '800000', NULL),
('4', 0, '', 0, '800000', NULL),
('5', 0, '', 0, '800000', NULL),
('6', 0, '', 0, '450000', NULL),
('7', 0, '', 0, '500000', NULL),
('8', 0, '', 0, 'NULL', NULL),
('9', 0, '', 0, 'NULL', NULL);

INSERT INTO `servidor_config` (`clave`, `valor`) VALUES
('anuncios_indice_actual', '7.0000'),
('banco_int', '3.0000'),
('banco_x', '359.3430'),
('banco_y', '179.2440'),
('banco_z', '1008.3800'),
('clima_estado', '3.0000'),
('clima_ultima_hora', '8.0000'),
('comision_banco', '1'),
('conce_entrega_a', '270.0000'),
('conce_entrega_x', '2116.0000'),
('conce_entrega_y', '-1148.5000'),
('conce_entrega_z', '24.3500'),
('conce_x', '2129.8501'),
('conce_y', '-1139.1100'),
('conce_z', '25.5300'),
('payday_base', '800'),
('payday_faccion', '1200'),
('payday_trabajo', '1000');

INSERT INTO `transfender_marcadores` (`id`, `nombre`, `x`, `y`, `z`, `creado_por`, `fecha`) VALUES
('1', 'Transfender Los Santos', '1041.24', '-1029.72', '32.1016', '', '2026-09-12 05:55:16');

INSERT INTO `vehiculo_tuning` (`veh_id`, `comp_0`, `comp_1`, `comp_2`, `comp_3`, `comp_4`, `comp_5`, `comp_6`, `comp_7`, `comp_8`, `comp_9`, `comp_10`, `comp_11`, `comp_12`, `comp_13`, `comp_14`, `comp_15`, `pintura`) VALUES
('237', '0', '0', '0', '0', '0', '1010', '1018', '1076', '0', '0', '0', '0', '0', '0', '0', '0', '-1'),
('548', '0', '0', '1006', '1007', '0', '1010', '1018', '1074', '0', '0', '0', '0', '0', '0', '0', '0', '-1'),
('637', '1000', '1004', '1006', '1007', '1024', '1010', '0', '1096', '1086', '0', '0', '0', '1145', '0', '0', '0', '-1'),
('653', '0', '0', '0', '0', '0', '1010', '0', '1074', '1086', '0', '0', '0', '0', '0', '0', '0', '-1');

INSERT INTO `vehiculos` (`id`, `modelo`, `propietario`, `propietario_nombre`, `tipo`, `tipo_id`, `rango_minimo`, `matricula`, `color1`, `color2`, `pos_x`, `pos_y`, `pos_z`, `pos_a`, `interior`, `mundo`, `combustible`, `salud`, `cerrado`, `precio`, `precio_manual`, `fecha_compra`, `dropeo_expira`, `es_nfs`, `en_venta`, `kilometros`, `tiene_rastreador`, `rastreador_x`, `rastreador_y`, `rastreador_actualizado_en`, `tiene_alarma`, `federico_until`) VALUES
('87', '463', 0, '', '4', '0', '0', 'LS0001', '7', '90', '1174.76', '1364.83', '10.1203', '280.035', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('89', '522', 0, '', '4', '0', '0', 'LS0002', '49', '22', '1174.47', '1368.36', '10.1203', '283.055', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('90', '556', 0, '', '4', '0', '0', 'LS0003', '91', '10', '2692.03', '-1674.02', '9.4656', '178.828', '0', '0', '100', '1000', '0', '4500', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('91', '494', 0, '', '4', '0', '0', 'LS0004', '28', '110', '2676.7', '-1673.76', '9.4038', '178.828', '0', '0', '100', '1000', '0', '3900', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('92', '424', 0, '', '4', '0', '0', 'LS0005', '91', '125', '1104.93', '1614.81', '12.5546', '85.6435', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('94', '478', 0, '', '4', '0', '0', 'LS0006', '76', '6', '-1446.24', '-1494.73', '101.051', '6', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('95', '531', 0, '', '4', '0', '0', 'LS0007', '52', '115', '-1439.64', '-1576.82', '101.058', '264.118', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('97', '567', 0, '', '4', '0', '0', 'LS0008', '12', '49', '2685.98', '-2016.21', '12.5501', '0.337', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('98', '536', 0, '', '4', '0', '0', 'LS0009', '85', '24', '1772.1', '-2125.1', '13.0469', '0.3441', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('101', '562', 0, '', '4', '0', '0', 'LS0010', '82', '67', '-1952.6', '265.7', '39.9', '292.8', '0', '0', '100', '1000', '0', '3200', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('102', '560', 0, '', '4', '0', '0', 'LS0011', '92', '6', '-1957.7', '277', '34.3', '133.4', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('103', '567', 0, '', '4', '0', '0', 'LS0012', '6', '14', '-1952.8', '258.8', '39.9', '259.1', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('105', '402', 0, '', '4', '0', '0', 'LS0013', '56', '1', '886.378', '-25.6671', '63.244', '157.621', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('109', '468', 0, '', '4', '0', '0', 'LS0014', '111', '66', '-2486.05', '59.184', '24.8284', '180', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('110', '492', 0, '', '4', '0', '0', 'LS0015', '124', '43', '2216.9', '-1160.4', '24.7265', '270.801', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('111', '609', 0, '', '4', '0', '0', 'LS0016', '96', '98', '2251.03', '-1788.66', '12.7625', '358.959', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '1.15423', 0, '0', '0', NULL, '0', NULL),
('112', '609', 0, '', '4', '0', '0', 'LS0017', '78', '97', '-2118.17', '-4.0948', '35.0203', '270.142', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('113', '609', 0, '', '4', '0', '0', 'LS0018', '1', '90', '2596.75', '1444.25', '10.3203', '178.271', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('114', '545', 0, '', '4', '0', '0', 'LS0019', '71', '84', '2408.16', '-1719.46', '13.6665', '0.5881', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('115', '557', 0, '', '4', '0', '0', 'LS0020', '81', '28', '-1778.18', '1207.07', '25.1194', '91.9357', '0', '0', '100', '1000', '0', '4500', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('116', '568', 0, '', '4', '0', '0', 'LS0021', '24', '37', '-379.5', '-1443.12', '25.7266', '88.9244', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('117', '442', 0, '', '4', '0', '0', 'LS0022', '115', '83', '-2572.04', '1148.56', '55.7333', '337.843', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('119', '402', 0, '', '4', '0', '0', 'LS0023', '113', '89', '-2407.86', '4614.12', '71.1332', '50.6727', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '13.1082', 0, '0', '0', NULL, '0', NULL),
('120', '405', 0, '', '4', '0', '0', 'LS0024', '108', '22', '167.224', '-1343.1', '69.4788', '189.176', '0', '0', '100', '1000', '0', '3500', '1', NULL, NULL, '0', 0, '6.1883', 0, '0', '0', NULL, '0', NULL),
('122', '483', 0, '', '4', '0', '0', 'LS0025', '19', '89', '-2516.6', '1228.92', '36.4283', '211.5', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('123', '445', 0, '', '4', '0', '0', 'LS0026', '13', '48', '1122.29', '-1699.76', '13.43', '270', '0', '0', '100', '1000', '0', '3200', '0', NULL, NULL, '0', 0, '9.25673', 0, '0', '0', NULL, '0', NULL),
('124', '470', 0, '', '4', '0', '0', 'LS0027', '78', '119', '-1006.41', '-628.27', '32', '270', '0', '0', '100', '1000', '0', '4200', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('125', '468', 0, '', '4', '0', '0', 'LS0028', '110', '66', '-2085.23', '-2437.52', '30.31', '142', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '1.59359', 0, '0', '0', NULL, '0', NULL),
('126', '409', 0, '', '4', '0', '0', 'LS0029', '0', '56', '-1922.19', '288.34', '40.84', '180', '0', '0', '100', '1000', '0', '6000', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('128', '534', 0, '', '4', '0', '0', 'LS0030', '61', '116', '1803.38', '-1931.05', '13.66', '0', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('130', '489', 0, '', '4', '0', '0', 'LS0031', '66', '82', '-112.4', '-41.82', '3.26', '160', '0', '0', '100', '1000', '0', '2600', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('131', '439', 0, '', '4', '0', '0', 'LS0032', '83', '47', '-2456.1', '741.65', '34.92', '180', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('132', '514', 0, '', '4', '0', '0', 'LS0033', '112', '40', '-1951.81', '2393.83', '50.08', '292', '0', '0', '100', '1000', '0', '5100', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('134', '535', 0, '', '4', '0', '0', 'LS0034', '121', '67', '1923.93', '-2118.89', '13.35', '0', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('136', '580', 0, '', '4', '0', '0', 'LS0035', '3', '40', '-2430.22', '320.84', '34.97', '245', '0', '0', '100', '1000', '0', '6600', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('137', '475', 0, '', '4', '0', '0', 'LS0036', '65', '79', '-2265.33', '200.65', '34.97', '270', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('139', '429', 0, '', '4', '0', '0', 'LS0037', '80', '119', '2133.04', '1009.75', '10.49', '270', '0', '0', '100', '1000', '0', '4100', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('140', '506', 0, '', '4', '0', '0', 'LS0038', '102', '30', '2229.3', '1402.99', '10.82', '180', '0', '0', '100', '1000', '0', '6000', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('141', '508', 0, '', '4', '0', '0', 'LS0039', '96', '11', '-1550.4', '2687.54', '56.22', '90', '0', '0', '100', '1000', '0', '3200', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('142', '579', 0, '', '4', '0', '0', 'LS0040', '22', '76', '-2068.47', '-82.8145', '35.0928', '0.56279', '0', '0', '100', '1000', '0', '50000', '1', NULL, NULL, '0', 0, '0.011058', 0, '0', '0', NULL, '0', NULL),
('143', '424', 0, '', '4', '0', '0', 'LS0041', '64', '90', '682.17', '-1867.46', '4.82', '180', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('144', '536', 0, '', '4', '0', '0', 'LS0042', '9', '27', '1747.87', '-2098.03', '13.28', '180', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('145', '463', 0, '', '4', '0', '0', 'LS0043', '107', '78', '1144.46', '-1101.26', '25.35', '300', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('146', '500', 0, '', '4', '0', '0', 'LS0044', '71', '119', '-2406.25', '-2180.84', '33.39', '180', '0', '0', '100', '1000', '0', '2600', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('148', '587', 0, '', '4', '0', '0', 'LS0045', '61', '60', '2207.43', '1286.13', '10.57', '180', '0', '0', '100', '1000', '0', '3100', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('157', '531', 0, '', '4', '0', '0', 'LS0046', '43', '105', '-1110.79', '-947.792', '129', '0', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('161', '506', 0, '', '4', '0', '0', 'LS0047', '1', '23', '1220.98', '-1886.78', '13.39', '94.0515', '0', '0', '100', '1000', '0', '10000', '1', NULL, NULL, '0', 0, '7.66419', 0, '0', '0', NULL, '0', NULL),
('163', '504', 0, '', '4', '0', '0', 'LS0048', '55', '29', '-2151', '-409.1', '34.1', '307.2', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('169', '508', 0, '', '4', '0', '0', 'LS0049', '74', '119', '921.37', '-1227.43', '17.3516', '267.199', '0', '0', '100', '1000', '0', '3200', '0', NULL, NULL, '0', 0, '0.049292', 0, '0', '0', NULL, '0', NULL),
('170', '508', 0, '', '4', '0', '0', 'LS0050', '119', '113', '736.24', '-1334.2', '13.5411', '267.811', '0', '0', '100', '1000', '0', '3200', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('171', '508', 0, '', '4', '0', '0', 'LS0051', '82', '74', '855.419', '-1528.01', '13.6912', '267.384', '0', '0', '100', '1000', '0', '3200', '0', NULL, NULL, '0', 0, '0.337044', 0, '0', '0', NULL, '0', NULL),
('188', '461', 0, '', '4', '0', '0', 'LS0052', '66', '34', '435.275', '2527.52', '16.371', '90', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('189', '573', 0, '', '4', '0', '0', 'LS0053', '97', '5', '1091.89', '1612.63', '13', '206.758', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('190', '412', 0, '', '4', '0', '0', 'LS0054', '112', '44', '1772.66', '-2096.59', '13.99', '182.758', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('219', '400', 0, '', '4', '0', '0', 'LS0055', '30', '41', '1482.04', '-1029', '23.8253', '92.8865', '0', '0', '100', '1000', '0', '3000', '1', NULL, NULL, '0', 0, '1.09856', 0, '0', '0', NULL, '0', NULL),
('220', '401', 0, '', '4', '0', '0', 'LS0056', '116', '78', '830.067', '-886.579', '68.5532', '235.558', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '1.19765', 0, '0', '0', NULL, '0', NULL),
('221', '405', 0, '', '4', '0', '0', 'LS0057', '44', '111', '1120.6', '-1162.21', '23.458', '358.347', '0', '0', '100', '1000', '0', '4000', '1', NULL, NULL, '0', 0, '0.595749', 0, '0', '0', NULL, '0', NULL),
('225', '409', 0, '', '4', '0', '0', 'LS0058', '4', '61', '1225.12', '-1135.17', '23.5877', '90.5216', '0', '0', '100', '1000', '0', '5900', '0', NULL, NULL, '0', 0, '0.620557', 0, '0', '0', NULL, '0', NULL),
('226', '410', 0, '', '4', '0', '0', 'LS0059', '42', '28', '1629.6', '-1098.56', '23.5936', '90.1245', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('228', '412', 0, '', '4', '0', '0', 'LS0060', '33', '69', '1441.01', '-1028.52', '23.5753', '90.8107', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '2.13436', 0, '0', '0', NULL, '0', NULL),
('229', '413', 0, '', '4', '0', '0', 'LS0061', '120', '15', '2085.87', '-1170.17', '25.0945', '260.7', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.527254', 0, '0', '0', NULL, '0', NULL),
('230', '414', 0, '', '4', '0', '0', 'LS0062', '96', '56', '1629.41', '-1107.41', '24', '89.1327', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '2.93193', 0, '0', '0', NULL, '0', NULL),
('234', '418', 0, '', '4', '0', '0', 'LS0063', '19', '71', '1617.47', '-1132.6', '23.999', '89.5995', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('235', '419', 0, '', '4', '0', '0', 'LS0064', '48', '25', '860.654', '-850.794', '77.0594', '205.325', '0', '0', '100', '1000', '0', '2000', '1', NULL, NULL, '0', 0, '1.03642', 0, '0', '0', NULL, '0', NULL),
('237', '421', 0, '', '4', '0', '0', 'LS0065', '68', '88', '2503.21', '-1681.33', '13.3258', '310.909', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '74.5481', 0, '2503.21', '-1681.33', NULL, '1', NULL),
('238', '422', 0, '', '4', '0', '0', 'LS0066', '117', '70', '1026.08', '-801.558', '101.833', '202.826', '0', '0', '100', '1000', '0', '2500', '1', NULL, NULL, '0', 0, '1.19858', 0, '0', '0', NULL, '0', NULL),
('240', '424', 0, '', '4', '0', '0', 'LS0067', '28', '55', '2473.68', '-1694.09', '13.2967', '179.201', '0', '0', '100', '1000', '0', '3200', '1', NULL, NULL, '0', 0, '20.6127', 0, '0', '0', NULL, '0', NULL),
('242', '426', 0, '', '4', '0', '0', 'LS0068', '63', '26', '1442.5', '-1333.11', '13.2963', '267.855', '0', '0', '100', '1000', '0', '3500', '1', NULL, NULL, '0', 0, '0.477791', 0, '0', '0', NULL, '0', NULL),
('245', '429', 0, '', '4', '0', '0', 'LS0069', '2', '1', '563.473', '-1276.09', '16.9219', '101.666', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '10.5659', 0, '0', '0', NULL, '0', NULL),
('251', '434', 0, '', '4', '0', '0', 'LS0070', '12', '62', '2009.99', '-978.619', '35.5353', '211.919', '0', '0', '100', '1000', '0', '6000', '1', NULL, NULL, '0', 0, '0.517693', 0, '0', '0', NULL, '0', NULL),
('253', '436', 0, '', '4', '0', '0', 'LS0071', '24', '59', '545.409', '-1258.24', '16.4676', '123.984', '0', '0', '100', '1000', '0', '800', '1', NULL, NULL, '0', 0, '9.14459', 0, '0', '0', NULL, '0', NULL),
('256', '439', 0, '', '4', '0', '0', 'LS0072', '98', '62', '2472.37', '-1707.65', '13.4177', '176.136', '0', '0', '100', '1000', '0', '3750', '1', NULL, NULL, '0', 0, '6.44791', 0, '0', '0', NULL, '0', NULL),
('257', '440', 0, '', '4', '0', '0', 'LS0073', '44', '44', '-2142.94', '-2447.4', '30.746', '143.403', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '17.1025', 0, '0', '0', NULL, '0', NULL),
('259', '442', 0, '', '4', '0', '0', 'LS0074', '91', '116', '930.664', '-1110.38', '24.0245', '359.475', '0', '0', '100', '1000', '0', '5000', '1', NULL, NULL, '0', 0, '0.81821', 0, '0', '0', NULL, '0', NULL),
('260', '443', 0, '', '4', '0', '0', 'LS0075', '55', '56', '1712.58', '-1092.55', '24.7135', '269.418', '0', '0', '100', '1000', '0', '14000', '1', NULL, NULL, '0', 0, '2.18169', 0, '0', '0', NULL, '0', NULL),
('261', '444', 0, '', '4', '0', '0', 'LS0076', '114', '27', '2521.2', '-959.072', '82.7045', '3.1615', '0', '0', '100', '1000', '0', '20000', '1', NULL, NULL, '0', 0, '1.04542', 0, '0', '0', NULL, '0', NULL),
('264', '448', 0, '', '4', '0', '0', 'LS0077', '43', '10', '1358.99', '-1639.79', '13.1273', '182.628', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '1.75085', 0, '0', '0', NULL, '0', NULL),
('285', '489', 0, '', '4', '0', '0', 'LS0078', '123', '52', '932.288', '-852.853', '93.6476', '25.2006', '0', '0', '100', '1000', '0', '2500', '1', NULL, NULL, '0', 0, '1.12718', 0, '0', '0', NULL, '0', NULL),
('287', '492', 0, '', '4', '0', '0', 'LS0079', '115', '20', '1467.91', '-1502.12', '13.3287', '271.58', '0', '0', '100', '1000', '0', '1900', '1', NULL, NULL, '0', 0, '0.597734', 0, '0', '0', NULL, '0', NULL),
('308', '412', 0, '', '4', '0', '0', 'LS0080', '12', '45', '2471.66', '-1715.81', '13.3935', '177.18', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '0.023467', 0, '0', '0', NULL, '0', NULL),
('309', '500', 0, '', '4', '0', '0', 'LS0081', '65', '62', '1654.17', '-1048.07', '24.0067', '358.837', '0', '0', '100', '1000', '0', '3000', '1', NULL, NULL, '0', 0, '2.32423', 0, '0', '0', NULL, '0', NULL),
('314', '505', 0, '', '4', '0', '0', 'LS0082', '18', '75', '1636.26', '-1046.64', '24.0406', '0.5689', '0', '0', '100', '1000', '0', '4000', '1', NULL, NULL, '0', 0, '3.6716', 0, '0', '0', NULL, '0', NULL),
('316', '507', 0, '', '4', '0', '0', 'LS0083', '0', '29', '1004.24', '-1225.9', '16.6746', '270.106', '0', '0', '100', '1000', '0', '4000', '1', NULL, NULL, '0', 0, '0.944623', 0, '0', '0', NULL, '0', NULL),
('329', '514', 0, '', '4', '0', '0', 'LS0084', '110', '19', '1695.62', '-1092.79', '24.6672', '269.727', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('330', '515', 0, '', '4', '0', '0', 'LS0085', '15', '21', '1729.02', '-1092.78', '25.1071', '88.2875', '0', '0', '100', '1000', '0', '12500', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('331', '516', 0, '', '4', '0', '0', 'LS0086', '60', '111', '1987.41', '-1275.88', '23.6541', '359.033', '0', '0', '100', '1000', '0', '2000', '1', NULL, NULL, '0', 0, '2.45335', 0, '0', '0', NULL, '0', NULL),
('332', '517', 0, '', '4', '0', '0', 'LS0087', '124', '33', '872.035', '-870.577', '77.461', '23.3314', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '1.19892', 0, '0', '0', NULL, '0', NULL),
('336', '521', 0, '', '4', '0', '0', 'LS0088', '38', '34', '1280.07', '-1076.41', '28.1436', '261.509', '0', '0', '100', '1000', '0', '4000', '1', NULL, NULL, '0', 0, '0.513396', 0, '0', '0', NULL, '0', NULL),
('337', '522', 0, '', '4', '0', '0', 'LS0089', '56', '53', '2497.79', '-1683.23', '12.9329', '286.452', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '147.833', 0, '2497.79', '-1683.23', NULL, '1', NULL),
('341', '526', 0, '', '4', '0', '0', 'LS0090', '79', '47', '2472.85', '-1700.88', '13.2873', '355.754', '0', '0', '100', '1000', '0', '2500', '1', NULL, NULL, '0', 0, '5.46756', 0, '2472.85', '-1700.88', NULL, '1', NULL),
('344', '529', 0, '', '4', '0', '0', 'LS0091', '65', '109', '2002.48', '-1275.26', '23.4534', '359.795', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '0.461125', 0, '0', '0', NULL, '0', NULL),
('348', '533', 0, '', '4', '0', '0', 'LS0092', '9', '68', '1666.3', '-1135.42', '23.6159', '180', '0', '0', '100', '1000', '0', '2500', '1', NULL, NULL, '0', 0, '2.65289', 0, '0', '0', NULL, '0', NULL),
('349', '534', 0, '', '4', '0', '0', 'LS0093', '59', '92', '1661.8', '-1135.46', '23.6305', '178.58', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('351', '536', 0, '', '4', '0', '0', 'LS0094', '76', '101', '2013.26', '-1119.27', '25.9405', '0.574883', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '0.482198', 0, '0', '0', NULL, '0', NULL),
('357', '542', 0, '', '4', '0', '0', 'LS0095', '73', '115', '2432.15', '-1635.66', '13.1732', '179.74', '0', '0', '100', '1000', '0', '3000', '1', NULL, NULL, '0', 0, '1.00896', 0, '0', '0', NULL, '0', NULL),
('358', '543', 0, '', '4', '0', '0', 'LS0096', '105', '56', '2436.7', '-1635.71', '13.249', '181.166', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '5.81161', 0, '0', '0', NULL, '0', NULL),
('360', '545', 0, '', '4', '0', '0', 'LS0097', '89', '27', '2437.28', '-1676.39', '13.5098', '359.291', '0', '0', '100', '1000', '0', '3000', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('361', '546', 0, '', '4', '0', '0', 'LS0098', '121', '21', '2433.44', '-1676.57', '13.4291', '0.126693', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('364', '549', 0, '', '4', '0', '0', 'LS0099', '61', '78', '1925.35', '-1788.78', '13.084', '90.9887', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '9.70325', 0, '0', '0', NULL, '0', NULL),
('365', '550', 0, '', '4', '0', '0', 'LS0100', '81', '47', '2298.01', '-1630.65', '14.4978', '86.4488', '0', '0', '100', '1000', '0', '3000', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('366', '551', 0, '', '4', '0', '0', 'LS0101', '118', '72', '2298.02', '-1634.62', '14.4928', '90.1618', '0', '0', '100', '1000', '0', '3000', '1', NULL, NULL, '0', 0, '8.86948', 0, '0', '0', NULL, '0', NULL),
('367', '552', 0, '', '4', '0', '0', 'LS0102', '8', '74', '2298.5', '-1638.81', '14.4037', '89.4837', '0', '0', '100', '1000', '0', '6000', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('369', '554', 0, '', '4', '0', '0', 'LS0103', '96', '6', '2279.51', '-1683.2', '14.2583', '179.207', '0', '0', '100', '1000', '0', '4000', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('370', '555', 0, '', '4', '0', '0', 'LS0104', '119', '73', '2283.95', '-1682.9', '13.7506', '180.064', '0', '0', '100', '1000', '0', '5000', '1', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('375', '480', 0, '', '4', '0', '0', 'LS0105', '48', '48', '549.618', '-1290.54', '17.021', '5.2843', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.052782', 0, '0', '0', NULL, '0', NULL),
('376', '494', 0, '', '4', '0', '0', 'LS0106', '113', '78', '537.045', '-1276.45', '17.1376', '220.027', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.141658', 0, '0', '0', NULL, '0', NULL),
('377', '502', 0, '', '4', '0', '0', 'LS0107', '71', '117', '548.077', '-1267.4', '17.1354', '216.302', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '1.7001', 0, '0', '0', NULL, '0', NULL),
('378', '503', 0, '', '4', '0', '0', 'LS0108', '25', '83', '540.568', '-1273.29', '17.1369', '219.088', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.146241', 0, '0', '0', NULL, '0', NULL),
('379', '504', 0, '', '4', '0', '0', 'LS0109', '116', '5', '544.573', '-1270.15', '17.0409', '215.881', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.138237', 0, '0', '0', NULL, '0', NULL),
('380', '506', 0, '', '4', '0', '0', 'LS0110', '74', '32', '545.145', '-1290.61', '16.9496', '4.88757', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.051039', 0, '0', '0', NULL, '0', NULL),
('382', '539', 0, '', '4', '0', '0', 'LS0111', '124', '57', '533.638', '-1291.62', '16.6026', '2.05519', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.042308', 0, '0', '0', NULL, '0', NULL),
('385', '559', 0, '', '4', '0', '0', 'LS0112', '81', '96', '544.124', '-1282.2', '16.9016', '275.921', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.04633', 0, '0', '0', NULL, '0', NULL),
('386', '560', 0, '', '4', '0', '0', 'LS0113', '80', '105', '530.244', '-1278.32', '16.9472', '128.592', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.094701', 0, '0', '0', NULL, '0', NULL),
('387', '562', 0, '', '4', '0', '0', 'LS0114', '43', '102', '532.283', '-1281.27', '16.9013', '127.665', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.072351', 0, '0', '0', NULL, '0', NULL),
('388', '565', 0, '', '4', '0', '0', 'LS0115', '8', '26', '525.258', '-1282.14', '16.8662', '128.42', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.088173', 0, '0', '0', NULL, '0', NULL),
('389', '568', 0, '', '4', '0', '0', 'LS0116', '125', '22', '526.977', '-1285.42', '17.1068', '307.692', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.051635', 0, '0', '0', NULL, '0', NULL),
('390', '587', 0, '', '4', '0', '0', 'LS0117', '52', '4', '519.984', '-1286.49', '16.9678', '128.603', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.112224', 0, '0', '0', NULL, '0', NULL),
('391', '589', 0, '', '4', '0', '0', 'LS0118', '70', '36', '529.751', '-1292.41', '16.9003', '89.8775', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.081837', 0, '0', '0', NULL, '0', NULL),
('392', '602', 0, '', '4', '0', '0', 'LS0119', '29', '107', '514.285', '-1290.9', '17.0484', '128.118', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '1', 0, '0.125944', 0, '0', '0', NULL, '0', NULL),
('400', '551', 0, '', '4', '0', '0', 'LS0120', '26', '54', '2507.82', '-1676.21', '13.2589', '325.588', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '23.5067', 0, '2507.82', '-1676.21', NULL, '1', NULL),
('404', '491', 0, '', '4', '0', '0', 'LS0121', '100', '8', '570.012', '-1244.27', '17.1247', '115.873', '0', '0', '100', '1000', '0', '900', '1', NULL, NULL, '0', 0, '1.58023', 0, '0', '0', NULL, '0', NULL),
('408', '491', 0, '', '4', '0', '0', 'LS0122', '113', '92', '2288.33', '-1682.51', '13.7289', '358.662', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '1.30667', 0, '0', '0', NULL, '0', NULL),
('409', '492', 0, '', '4', '0', '0', 'LS0123', '48', '0', '2292.48', '-1682.49', '13.726', '359.316', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '1.71126', 0, '0', '0', NULL, '0', NULL),
('410', '413', 0, '', '4', '0', '0', 'LS0124', '69', '120', '1836.6', '-1854.2', '13.4801', '179.794', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.066037', 0, '0', '0', NULL, '0', NULL),
('414', '461', 0, '', '4', '0', '0', 'LS0125', '85', '26', '1844.38', '-1871.12', '12.9749', '0.949288', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '30.188', 0, '0', '0', NULL, '0', NULL),
('415', '462', 0, '', '4', '0', '0', 'LS0126', '65', '2', '1849.52', '-1865.61', '13.1786', '91.643', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.024617', 0, '0', '0', NULL, '0', NULL),
('416', '463', 0, '', '4', '0', '0', 'LS0127', '43', '17', '1833.89', '-1852.64', '12.9301', '180.141', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.0215', 0, '0', '0', NULL, '0', NULL),
('419', '471', 0, '', '4', '0', '0', 'LS0128', '47', '83', '1831.9', '-1872.46', '12.8688', '357.296', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.0179', 0, '0', '0', NULL, '0', NULL),
('421', '491', 0, '', '4', '0', '0', 'LS0129', '48', '17', '1838.11', '-1871.05', '13.1459', '0.244694', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.03399', 0, '0', '0', NULL, '0', NULL),
('422', '492', 0, '', '4', '0', '0', 'LS0130', '91', '112', '1850.87', '-1861.33', '13.3651', '89.9478', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.03414', 0, '0', '0', NULL, '0', NULL),
('424', '498', 0, '', '4', '0', '0', 'LS0131', '76', '0', '1834.93', '-1871.61', '13.4554', '359.09', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.039522', 0, '0', '0', NULL, '0', NULL),
('428', '566', 0, '', '4', '0', '0', 'LS0132', '98', '13', '1583.59', '-1718.24', '13.3343', '359.453', '0', '0', '100', '1000', '0', '3000', '1', NULL, NULL, '0', 0, '7.7801', 0, '0', '0', NULL, '0', NULL),
('430', '588', 0, '', '4', '0', '0', 'LS0133', '42', '104', '1895.67', '-1794.06', '13.446', '272.546', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '3.60988', 0, '0', '0', NULL, '0', NULL),
('434', '478', 0, '', '4', '0', '0', 'LS0134', '11', '110', '1576.2', '-1718.4', '13.536', '0.336326', '0', '0', '100', '1000', '0', '800', '1', NULL, NULL, '0', 0, '0.018376', 0, '0', '0', NULL, '0', NULL),
('435', '479', 0, '', '4', '0', '0', 'LS0135', '12', '124', '2375.35', '-1633.52', '13.2572', '268.684', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '0.954165', 0, '0', '0', NULL, '0', NULL),
('436', '480', 0, '', '4', '0', '0', 'LS0136', '61', '19', '1357.22', '-1488.24', '13.3126', '250.445', '0', '0', '100', '1000', '0', '10000', '1', NULL, NULL, '0', 0, '3.52263', 0, '0', '0', NULL, '0', NULL),
('437', '481', 0, '', '4', '0', '0', 'LS0137', '92', '1', '1540.31', '-1651.69', '13.0745', '270.788', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.016892', 0, '0', '0', NULL, '0', NULL),
('439', '483', 0, '', '4', '0', '0', 'LS0138', '82', '105', '1536.16', '-1667.02', '13.3759', '0.405419', '0', '0', '100', '1000', '0', '1000', '1', NULL, NULL, '0', 0, '6.45389', 0, '0', '0', NULL, '0', NULL),
('441', '412', 0, '', '4', '0', '0', 'LS0139', '109', '84', '2458.89', '-1653.91', '13.2035', '89.8076', '0', '0', '100', '1000', '0', '1900', '1', NULL, NULL, '0', 0, '8.36111', 0, '0', '0', NULL, '0', NULL),
('442', '413', 0, '', '4', '0', '0', 'LS0140', '10', '18', '204.712', '-1359.1', '50.4185', '131.121', '0', '0', '100', '1000', '0', '1500', '1', NULL, NULL, '0', 0, '20.8232', 0, '204.712', '-1359.1', NULL, '1', NULL),
('449', '478', 0, '', '4', '0', '0', 'LS0141', '17', '72', '1362.3', '-1673.18', '8.5999', '359.942', '0', '0', '100', '1000', '0', '800', '1', NULL, NULL, '0', 0, '8.57134', 0, '0', '0', NULL, '0', NULL),
('451', '444', 0, '', '4', '0', '0', 'LS0142', '48', '68', '2353.38', '-1817.41', '13.9225', '270.783', '0', '0', '100', '1000', '0', '4700', '0', NULL, NULL, '0', 0, '0.721608', 0, '0', '0', NULL, '0', NULL),
('454', '554', 0, '', '4', '0', '0', 'LS0143', '106', '111', '2400.92', '-1653.92', '13.5163', '91.5114', '0', '0', '100', '1000', '0', '3000', '1', NULL, NULL, '0', 0, '8.01833', 0, '0', '0', NULL, '0', NULL),
('455', '555', 0, '', '4', '0', '0', 'LS0144', '46', '9', '2120.62', '-1779.34', '13.0744', '270.3', '0', '0', '100', '1000', '0', '5000', '1', NULL, NULL, '0', 0, '1.26942', 0, '0', '0', NULL, '0', NULL),
('458', '558', 0, '', '4', '0', '0', 'LS0145', '67', '71', '2120.9', '-1775.98', '13.0212', '270.906', '0', '0', '91.9792', '1000', '0', '9000', '1', NULL, NULL, '0', 0, '1.79396', 0, '0', '0', NULL, '0', NULL),
('459', '559', 0, '', '4', '0', '0', 'LS0146', '76', '55', '2509.96', '-1670.47', '13.0862', '355.508', '0', '0', '100', '1000', '0', '4500', '1', NULL, NULL, '0', 0, '0.50594', 0, '0', '0', NULL, '0', NULL),
('460', '560', 0, '', '4', '0', '0', 'LS0147', '56', '111', '2102.2', '-1784.27', '13.0964', '89.3185', '0', '0', '100', '1000', '0', '7000', '1', NULL, NULL, '0', 0, '0.073376', 0, '0', '0', NULL, '0', NULL),
('461', '561', 0, '', '4', '0', '0', 'LS0148', '19', '122', '2102.62', '-1779.78', '13.2058', '88.3095', '0', '0', '100', '1000', '0', '4000', '1', NULL, NULL, '0', 0, '0.021938', 0, '0', '0', NULL, '0', NULL),
('464', '549', 0, '', '4', '0', '0', 'LS0149', '121', '66', '2103.79', '-1771.56', '13.0906', '87.0169', '0', '0', '100', '1000', '0', '2000', '1', NULL, NULL, '0', 0, '0.02388', 0, '0', '0', NULL, '0', NULL),
('468', '456', 0, '', '4', '0', '0', 'LS0150', '96', '73', '2428.9', '-1242.26', '24.3901', '180.926', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '0.030359', 0, '0', '0', NULL, '0', NULL),
('469', '459', 0, '', '4', '0', '0', 'LS0151', '98', '77', '2434.68', '-1242.46', '24.3674', '0.579257', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.020324', 0, '0', '0', NULL, '0', NULL),
('470', '461', 0, '', '4', '0', '0', 'LS0152', '80', '123', '885.792', '-1799.86', '13.3553', '266.46', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '5.06402', 0, '0', '0', NULL, '0', NULL),
('472', '455', 0, '', '4', '0', '0', 'LS0153', '109', '25', '957.021', '-1356.81', '13.7819', '268.43', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '0.067681', 0, '0', '0', NULL, '0', NULL),
('477', '409', 0, '', '4', '0', '0', 'LS0154', '39', '61', '1268.13', '-1798.24', '13.2058', '181.636', '0', '0', '100', '1000', '0', '4000', '1', NULL, NULL, '0', 0, '0.030255', 0, '0', '0', NULL, '0', NULL),
('480', '405', 0, '', '4', '0', '0', 'LS0155', '113', '54', '1277.32', '-1796.46', '13.2668', '179.756', '0', '0', '100', '1000', '0', '4000', '1', NULL, NULL, '0', 0, '1.80907', 0, '0', '0', NULL, '0', NULL),
('481', '404', 0, '', '4', '0', '0', 'LS0156', '6', '122', '1261.85', '-1796.9', '13.1518', '359.826', '0', '0', '100', '1000', '0', '800', '1', NULL, NULL, '0', 0, '4.13979', 0, '0', '0', NULL, '0', NULL),
('494', '400', 0, '', '4', '0', '0', 'LS0157', '85', '63', '14814.6', '-7780.76', '4.34844', '245.562', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '3.16624', 0, '0', '0', NULL, '0', NULL),
('496', '422', 0, '', '4', '0', '0', 'LS0158', '44', '121', '14819.9', '-7778.42', '4.24398', '250.875', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '1.41154', 0, '0', '0', NULL, '0', NULL),
('499', '521', 0, '', '4', '0', '0', 'LS0159', '37', '60', '14588.4', '8310.47', '15.7935', '151.631', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '11.2273', 0, '0', '0', NULL, '0', NULL),
('501', '478', 0, '', '4', '0', '0', 'LS0160', '108', '31', '14816', '-7807.87', '4.34709', '157.233', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.02209', 0, '0', '0', NULL, '0', NULL),
('505', '466', 0, '', '4', '0', '0', 'LS0161', '80', '87', '13915.3', '11983.8', '6.25985', '243.745', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '5.26895', 0, '0', '0', NULL, '0', NULL),
('507', '596', 0, '', '2', '1', '0', 'LS0162', '1', '1', '1545.66', '-1667.85', '5.61205', '89.8497', '0', '0', '100', '1000', '0', '0', '0', NULL, NULL, '0', 0, '0.028295', 0, '0', '0', NULL, '0', NULL),
('508', '596', 0, '', '2', '1', '0', 'LS0163', '1', '1', '1545.9', '-1672.18', '5.61295', '91.4468', '0', '0', '100', '1000', '0', '0', '0', NULL, NULL, '0', 0, '0.030503', 0, '0', '0', NULL, '0', NULL),
('509', '596', 0, '', '2', '1', '2', 'LS0164', '1', '1', '1546.02', '-1676.31', '5.61193', '89.9795', '0', '0', '100', '1000', '0', '0', '0', NULL, NULL, '0', 0, '0.031669', 0, '0', '0', NULL, '0', NULL),
('510', '596', 0, '', '2', '1', '2', 'LS0165', '1', '1', '1545.94', '-1680.58', '5.61105', '90.8737', '0', '0', '100', '1000', '0', '0', '0', NULL, NULL, '0', 0, '0.016805', 0, '0', '0', NULL, '0', NULL),
('511', '596', 0, '', '2', '1', '2', 'LS0166', '1', '1', '1546', '-1684.46', '5.61082', '89.9905', '0', '0', '100', '1000', '0', '0', '0', NULL, NULL, '0', 0, '0.032987', 0, '0', '0', NULL, '0', NULL),
('512', '523', 0, '', '2', '1', '2', 'LS0167', '1', '1', '1547.78', '-1664.23', '5.45544', '89.3174', '0', '0', '100', '1000', '0', '0', '0', NULL, NULL, '0', 0, '0.044695', 0, '0', '0', NULL, '0', NULL),
('513', '523', 0, '', '2', '1', '2', 'LS0168', '1', '1', '1547.66', '-1661.67', '5.46067', '90.3599', '0', '0', '100', '1000', '0', '0', '0', NULL, NULL, '0', 0, '0.005478', 0, '0', '0', NULL, '0', NULL),
('514', '523', 0, '', '2', '1', '2', 'LS0169', '1', '1', '1547.61', '-1660.16', '5.46', '92.0219', '0', '0', '100', '1000', '0', '0', '0', NULL, NULL, '0', 0, '0.010572', 0, '0', '0', NULL, '0', NULL),
('515', '523', 0, '', '2', '1', '2', 'LS0170', '1', '1', '1547.65', '-1657.83', '5.46047', '89.2112', '0', '0', '100', '1000', '0', '0', '0', NULL, NULL, '0', 0, '0.010488', 0, '0', '0', NULL, '0', NULL),
('516', '427', 0, '', '2', '1', '8', 'LS0171', '1', '1', '1530.56', '-1643.83', '6.02509', '180.933', '0', '0', '100', '1000', '0', '0', '0', NULL, NULL, '0', 0, '0.039832', 0, '0', '0', NULL, '0', NULL),
('517', '558', 0, '', '4', '0', '0', 'LS0172', '30', '21', '1507.48', '-1848.41', '13.1759', '90.9426', '0', '0', '100', '1000', '0', '5000', '1', NULL, NULL, '0', 0, '0.227353', 0, '0', '0', NULL, '0', NULL),
('520', '586', 0, '', '4', '0', '0', 'LS0173', '34', '82', '2488.47', '-1683.02', '12.8553', '87.8802', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '31.5191', 0, '0', '0', NULL, '0', NULL),
('521', '586', 0, '', '4', '0', '0', 'LS0174', '80', '21', '2475.86', '-1708.13', '13.0477', '173.557', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '3.1667', 0, '0', '0', NULL, '0', NULL),
('522', '422', 0, '', '4', '0', '0', 'LS0175', '124', '14', '1397.18', '-1646.59', '13.3607', '91.2025', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.604406', 0, '0', '0', NULL, '0', NULL),
('523', '423', 0, '', '4', '0', '0', 'LS0176', '76', '24', '1213.88', '-1422.99', '13.3901', '178.266', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.138577', 0, '0', '0', NULL, '0', NULL),
('529', '437', 0, '', '4', '0', '0', 'LS0177', '15', '41', '1001.76', '-1365.61', '13.4176', '359.742', '0', '0', '100', '1000', '0', '178000', '1', NULL, NULL, '0', 0, '8.05551', 0, '0', '0', NULL, '0', NULL),
('531', '481', 0, '', '4', '0', '0', 'LS0178', '6', '70', '1005', '-1309.36', '12.8902', '114.4', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.028733', 0, '0', '0', NULL, '0', NULL),
('532', '483', 0, '', '4', '0', '0', 'LS0179', '102', '88', '1066.21', '-1351.45', '13.3749', '359.578', '0', '0', '100', '1000', '0', '1000', '1', NULL, NULL, '0', 0, '1.95206', 0, '0', '0', NULL, '0', NULL),
('540', '422', 0, '', '4', '0', '0', 'LS0180', '48', '68', '1167.89', '-1275.41', '13.4368', '90.9733', '0', '0', '100', '1000', '0', '3000', '1', NULL, NULL, '0', 0, '9.73744', 0, '0', '0', NULL, '0', NULL),
('544', '479', 0, '', '4', '0', '0', 'LS0181', '57', '94', '1062.26', '-1757.87', '13.2339', '89.7855', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '4.76336', 0, '0', '0', NULL, '0', NULL),
('548', '477', 0, '', '4', '0', '0', 'LS0182', '3', '49', '-2235.35', '-2429.71', '31.0437', '139.727', '0', '0', '100', '1000', '0', '3300', '0', NULL, NULL, '0', 0, '46.4816', 0, '-2235.35', '-2429.71', NULL, '1', NULL),
('552', '422', 0, '', '4', '0', '0', 'LS0183', '34', '34', '2440.8', '-1653.86', '13.3314', '91.9518', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.056962', 0, '0', '0', NULL, '0', NULL),
('553', '423', 0, '', '4', '0', '0', 'LS0184', '24', '35', '2173.6', '-1726.23', '13.5621', '0.709236', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '2.2072', 0, '0', '0', NULL, '0', NULL),
('554', '491', 0, '', '4', '0', '0', 'LS0185', '7', '101', '869.69', '-1589.33', '13.2153', '316.47', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.018078', 0, '0', '0', NULL, '0', NULL),
('555', '492', 0, '', '4', '0', '0', 'LS0186', '49', '62', '748.87', '-1695.19', '4.40681', '181.214', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.171196', 0, '0', '0', NULL, '0', NULL),
('556', '494', 0, '', '4', '0', '0', 'LS0187', '33', '8', '718.404', '-1807.33', '12.3609', '74.5329', '0', '0', '100', '1000', '0', '11000', '1', NULL, NULL, '0', 0, '7.25673', 0, '0', '0', NULL, '0', NULL),
('558', '481', 0, '', '4', '0', '0', 'LS0188', '28', '28', '682.709', '-1772.34', '13.1481', '34.3877', '0', '0', '100', '1000', '0', '50', '1', NULL, NULL, '0', 0, '0.050898', 0, '0', '0', NULL, '0', NULL),
('562', '502', 0, '', '4', '0', '0', 'LS0189', '101', '119', '-2228.73', '-2421.88', '31.1253', '229.914', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '60.6554', 0, '-2228.73', '-2421.88', NULL, '1', NULL),
('563', '412', 0, '', '4', '0', '0', 'LS0190', '89', '95', '1235.68', '-1033.46', '31.6685', '90.6714', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.010328', 0, '0', '0', NULL, '0', NULL),
('564', '413', 0, '', '4', '0', '0', 'LS0191', '110', '23', '1188.47', '-1044.7', '31.9001', '91.1289', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.036964', 0, '0', '0', NULL, '0', NULL),
('565', '414', 0, '', '4', '0', '0', 'LS0192', '87', '115', '1138.26', '-1025.13', '32.0025', '359.487', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '0.046138', 0, '0', '0', NULL, '0', NULL),
('568', '418', 0, '', '4', '0', '0', 'LS0193', '99', '39', '1035.93', '-1027.4', '32.1944', '179.529', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.092683', 0, '0', '0', NULL, '0', NULL),
('569', '419', 0, '', '4', '0', '0', 'LS0194', '24', '55', '1041.12', '-1054.7', '31.5007', '358.697', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.029685', 0, '0', '0', NULL, '0', NULL),
('570', '421', 0, '', '4', '0', '0', 'LS0195', '91', '62', '1029.06', '-1054.02', '31.5449', '359.402', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.087406', 0, '0', '0', NULL, '0', NULL),
('571', '422', 0, '', '4', '0', '0', 'LS0196', '51', '9', '1016.43', '-1056.12', '30.9191', '358.347', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.040608', 0, '0', '0', NULL, '0', NULL),
('572', '424', 0, '', '4', '0', '0', 'LS0197', '33', '53', '1012.99', '-1028.4', '31.8829', '178.376', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.021158', 0, '0', '0', NULL, '0', NULL),
('576', '431', 0, '', '4', '0', '0', 'LS0198', '81', '91', '1019.23', '-1098.99', '23.9349', '272.115', '0', '0', '100', '1000', '0', '25000', '1', NULL, NULL, '0', 0, '4.01982', 0, '0', '0', NULL, '0', NULL),
('579', '483', 0, '', '4', '0', '0', 'LS0199', '110', '124', '324.432', '2524.5', '16.7063', '109.614', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '0.022094', 0, '0', '0', NULL, '0', NULL),
('580', '449', 0, '', '4', '0', '0', 'LS0200', '66', '104', '342.967', '2529.79', '16.7441', '270.359', '0', '0', '100', '1000', '0', '5000', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('582', '410', 0, '', '4', '0', '0', 'LS0201', '110', '120', '344.094', '2531.3', '16.4262', '178.804', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '5.10031', 0, '0', '0', NULL, '0', NULL),
('583', '410', 0, '', '4', '0', '0', 'LS0202', '42', '91', '348.989', '2531.18', '16.397', '179.757', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.033989', 0, '0', '0', NULL, '0', NULL),
('584', '410', 0, '', '4', '0', '0', 'LS0203', '39', '29', '354.058', '2531.11', '16.3709', '179.253', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.041639', 0, '0', '0', NULL, '0', NULL),
('585', '410', 0, '', '4', '0', '0', 'LS0204', '63', '56', '358.938', '2531.14', '16.3462', '180.236', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.053839', 0, '0', '0', NULL, '0', NULL),
('586', '410', 0, '', '4', '0', '0', 'LS0205', '107', '64', '344.154', '2536.92', '16.4219', '178.006', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.043629', 0, '0', '0', NULL, '0', NULL),
('587', '410', 0, '', '4', '0', '0', 'LS0206', '92', '63', '348.989', '2536.87', '16.3956', '179.776', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.039703', 0, '0', '0', NULL, '0', NULL),
('588', '410', 0, '', '4', '0', '0', 'LS0207', '41', '45', '354.13', '2536.91', '16.37', '179.798', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.046032', 0, '0', '0', NULL, '0', NULL),
('589', '410', 0, '', '4', '0', '0', 'LS0208', '57', '28', '358.98', '2536.87', '16.3477', '180.026', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.05066', 0, '0', '0', NULL, '0', NULL),
('591', '602', 0, '', '4', '0', '0', 'LS0209', '54', '20', '721.466', '1948.11', '5.34609', '178.377', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.012323', 0, '0', '0', NULL, '0', NULL),
('592', '600', 0, '', '4', '0', '0', 'LS0210', '16', '124', '173.736', '1188.47', '14.4729', '149.31', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.026885', 0, '0', '0', NULL, '0', NULL),
('593', '585', 0, '', '4', '0', '0', 'LS0211', '57', '40', '750.624', '334.087', '19.9037', '188.484', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.025305', 0, '0', '0', NULL, '0', NULL),
('594', '581', 0, '', '4', '0', '0', 'LS0212', '106', '22', '1355.74', '244.983', '19.1633', '241.343', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.019154', 0, '0', '0', NULL, '0', NULL),
('595', '578', 0, '', '4', '0', '0', 'LS0213', '26', '92', '985.533', '-913.685', '42.7602', '3.74194', '0', '0', '100', '1000', '0', '3100', '0', NULL, NULL, '0', 0, '0.037758', 0, '0', '0', NULL, '0', NULL),
('596', '576', 0, '', '4', '0', '0', 'LS0214', '69', '67', '1012.11', '-1411.16', '12.7245', '270.896', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.949616', 0, '0', '0', NULL, '0', NULL),
('597', '575', 0, '', '4', '0', '0', 'LS0215', '66', '58', '1132.36', '-1675.46', '13.2889', '96.3101', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '1.26719', 0, '0', '0', NULL, '0', NULL),
('598', '565', 0, '', '4', '0', '0', 'LS0216', '56', '86', '-2026.9', '34.5548', '33.6516', '88.0581', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '8.86028', 0, '0', '0', NULL, '0', NULL),
('599', '540', 0, '', '4', '0', '0', 'LS0217', '84', '19', '653.646', '-1543.88', '14.7124', '90.1423', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.513605', 0, '0', '0', NULL, '0', NULL),
('600', '499', 0, '', '4', '0', '0', 'LS0218', '27', '106', '594.786', '-1504.13', '15.2021', '93.4835', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '3.18987', 0, '0', '0', NULL, '0', NULL),
('602', '474', 0, '', '4', '0', '0', 'LS0219', '43', '45', '2055.92', '-1904.03', '13.3096', '359.401', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '2.19601', 0, '0', '0', NULL, '0', NULL),
('603', '467', 0, '', '4', '0', '0', 'LS0220', '1', '106', '2419.02', '-2103.05', '13.1583', '181.458', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.527588', 0, '0', '0', NULL, '0', NULL),
('604', '458', 0, '', '4', '0', '0', 'LS0221', '65', '89', '2731.02', '-1999.49', '13.3307', '269.214', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.51397', 0, '0', '0', NULL, '0', NULL),
('605', '438', 0, '', '4', '0', '0', 'LS0222', '122', '57', '1544.06', '36.4495', '24.1439', '103.456', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.271235', 0, '0', '0', NULL, '0', NULL),
('606', '403', 0, '', '4', '0', '0', 'LS0223', '94', '56', '2192.2', '35.9763', '27.0087', '270.84', '0', '0', '100', '1000', '0', '4700', '0', NULL, NULL, '0', 0, '0.721081', 0, '0', '0', NULL, '0', NULL),
('607', '539', 0, '', '4', '0', '0', 'LS0224', '103', '76', '2165.08', '-106.744', '2.16783', '117.65', '0', '0', '100', '1000', '0', '3600', '0', NULL, NULL, '0', 0, '0.193492', 0, '0', '0', NULL, '0', NULL),
('608', '503', 0, '', '4', '0', '0', 'LS0225', '61', '38', '2279.44', '4.21335', '27.363', '359.442', '0', '0', '100', '1000', '0', '11000', '1', NULL, NULL, '0', 0, '0.232623', 0, '0', '0', NULL, '0', NULL),
('611', '600', 0, '', '4', '0', '0', 'LS0226', '110', '52', '2407.11', '-1538.62', '23.7141', '1.88661', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.068351', 0, '0', '0', NULL, '0', NULL),
('612', '600', 0, '', '4', '0', '0', 'LS0227', '3', '13', '2031.85', '-1284.99', '20.6762', '90.1892', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.535832', 0, '0', '0', NULL, '0', NULL),
('613', '588', 0, '', '4', '0', '0', 'LS0228', '0', '66', '2025.36', '-1276.43', '20.8721', '178.652', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '0.027228', 0, '0', '0', NULL, '0', NULL),
('614', '586', 0, '', '4', '0', '0', 'LS0229', '98', '16', '2024.99', '-1285.66', '20.4673', '181.467', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.020142', 0, '0', '0', NULL, '0', NULL),
('615', '585', 0, '', '4', '0', '0', 'LS0230', '112', '33', '2015.37', '-1335.88', '23.487', '89.437', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.047064', 0, '0', '0', NULL, '0', NULL),
('616', '580', 0, '', '4', '0', '0', 'LS0231', '55', '52', '1832.22', '-1402.91', '13.2229', '270.115', '0', '0', '100', '1000', '0', '6500', '0', NULL, NULL, '0', 0, '0.21583', 0, '0', '0', NULL, '0', NULL),
('617', '578', 0, '', '4', '0', '0', 'LS0232', '45', '63', '1858.63', '-1388.55', '14.1885', '178.821', '0', '0', '100', '1000', '0', '3100', '0', NULL, NULL, '0', 0, '0.036151', 0, '0', '0', NULL, '0', NULL),
('618', '410', 0, '', '4', '0', '0', 'LS0233', '44', '109', '1211.28', '-1355.49', '13.1319', '0.393993', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '0.025793', 0, '0', '0', NULL, '0', NULL),
('619', '410', 0, '', '4', '0', '0', 'LS0234', '112', '77', '1211.24', '-1348.82', '13.1377', '359.289', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '5.29519', 0, '0', '0', NULL, '0', NULL),
('620', '410', 0, '', '4', '0', '0', 'LS0235', '78', '76', '1211.21', '-1362.25', '13.0933', '359.186', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '0.0398', 0, '0', '0', NULL, '0', NULL),
('621', '410', 0, '', '4', '0', '0', 'LS0236', '64', '31', '1211.33', '-1377.64', '13.0202', '1.18857', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '0.67551', 0, '0', '0', NULL, '0', NULL),
('622', '410', 0, '', '4', '0', '0', 'LS0237', '88', '93', '1211.18', '-1371.2', '13.0531', '1.16985', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '0.085617', 0, '0', '0', NULL, '0', NULL),
('623', '410', 0, '', '4', '0', '0', 'LS0238', '34', '34', '1211.09', '-1301.6', '13.1214', '0.290333', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '8.4026', 0, '0', '0', NULL, '0', NULL),
('624', '410', 0, '', '4', '0', '0', 'LS0239', '10', '21', '1184.05', '-1660.86', '13.4239', '358.443', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '6.68218', 0, '0', '0', NULL, '0', NULL),
('625', '410', 0, '', '4', '0', '0', 'LS0240', '62', '21', '1211.34', '-1314.86', '13.13', '0.768024', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '8.74452', 0, '0', '0', NULL, '0', NULL),
('626', '410', 0, '', '4', '0', '0', 'LS0241', '86', '70', '1211.48', '-1321.48', '13.1288', '0.158717', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '3.66226', 0, '0', '0', NULL, '0', NULL),
('627', '410', 0, '', '4', '0', '0', 'LS0242', '7', '121', '1211.4', '-1327.65', '13.1279', '0.382727', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '1.31616', 0, '0', '0', NULL, '0', NULL),
('628', '410', 0, '', '4', '0', '0', 'LS0243', '88', '15', '1211.32', '-1333.96', '13.1334', '0.188937', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '2.73051', 0, '0', '0', NULL, '0', NULL),
('629', '410', 0, '', '4', '0', '0', 'LS0244', '17', '23', '2260.68', '-1786.31', '13.1999', '5.86171', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '4.63568', 0, '0', '0', NULL, '0', NULL),
('630', '410', 0, '', '4', '0', '0', 'LS0245', '75', '22', '1210.98', '-1295.93', '13.1139', '359.469', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '7.61218', 0, '0', '0', NULL, '0', NULL),
('631', '576', 0, '', '4', '0', '0', 'LS0246', '12', '78', '1344.43', '-1753.05', '12.9706', '179.232', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '1.01286', 0, '0', '0', NULL, '0', NULL),
('632', '576', 0, '', '4', '0', '0', 'LS0247', '114', '41', '1669.07', '-1696.08', '15.2191', '0.512442', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '1.70488', 0, '0', '0', NULL, '0', NULL),
('633', '575', 0, '', '4', '0', '0', 'LS0248', '15', '107', '1663.88', '-1696.29', '15.2115', '177.487', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.021993', 0, '0', '0', NULL, '0', NULL),
('634', '566', 0, '', '4', '0', '0', 'LS0249', '98', '84', '595.576', '-1496.98', '14.994', '282.179', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '2.67603', 0, '0', '0', NULL, '0', NULL),
('635', '566', 0, '', '4', '0', '0', 'LS0250', '106', '63', '1401.46', '-1447.39', '13.3379', '269.714', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '7.9361', 0, '0', '0', NULL, '0', NULL),
('636', '552', 0, '', '4', '0', '0', 'LS0251', '121', '2', '1652.27', '-1506', '13.1624', '179.077', '0', '0', '100', '1000', '0', '2600', '0', NULL, NULL, '0', 0, '0.268459', 0, '0', '0', NULL, '0', NULL),
('637', '589', 0, '', '4', '0', '0', 'LS0252', '4', '93', '1255.67', '-2012.32', '59.1393', '0.103729', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '5.80734', 0, '0', '0', NULL, '0', NULL),
('638', '547', 0, '', '4', '0', '0', 'LS0253', '62', '97', '1649.72', '-1696.84', '20.4777', '287.295', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.865639', 0, '0', '0', NULL, '0', NULL),
('639', '546', 0, '', '4', '0', '0', 'LS0254', '88', '52', '1669.85', '-1712.14', '20.2015', '94.8462', '0', '0', '100', '1000', '0', '2600', '0', NULL, NULL, '0', 0, '0.035248', 0, '0', '0', NULL, '0', NULL),
('640', '543', 0, '', '4', '0', '0', 'LS0255', '29', '37', '1189.77', '-1365.4', '13.2794', '179.003', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '6.93587', 0, '0', '0', NULL, '0', NULL),
('641', '540', 0, '', '4', '0', '0', 'LS0256', '12', '16', '1731.31', '-1750.89', '13.3728', '0.928306', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.036804', 0, '0', '0', NULL, '0', NULL),
('642', '535', 0, '', '4', '0', '0', 'LS0257', '104', '21', '1790.18', '-1931.76', '13.1513', '180.295', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.278543', 0, '0', '0', NULL, '0', NULL),
('643', '526', 0, '', '4', '0', '0', 'LS0258', '50', '72', '1980.82', '-1994.86', '13.3201', '181.507', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '13.4618', 0, '0', '0', NULL, '0', NULL),
('644', '518', 0, '', '4', '0', '0', 'LS0259', '5', '117', '1987.23', '-1986.43', '13.2176', '181.135', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.042651', 0, '0', '0', NULL, '0', NULL),
('645', '516', 0, '', '4', '0', '0', 'LS0260', '32', '101', '1947.55', '-1978.46', '13.3809', '270.084', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.028888', 0, '0', '0', NULL, '0', NULL),
('646', '507', 0, '', '4', '0', '0', 'LS0261', '80', '8', '1948.12', '-2133.33', '13.3689', '89.0555', '0', '0', '100', '1000', '0', '3300', '0', NULL, NULL, '0', 0, '0.984196', 0, '0', '0', NULL, '0', NULL),
('647', '499', 0, '', '4', '0', '0', 'LS0262', '57', '52', '1928.66', '-2141.71', '13.551', '0.200778', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '1.89975', 0, '0', '0', NULL, '0', NULL),
('648', '496', 0, '', '4', '0', '0', 'LS0263', '17', '107', '1941.21', '-2142.27', '13.2743', '0.661126', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '3.10565', 0, '0', '0', NULL, '0', NULL),
('649', '498', 0, '', '4', '0', '0', 'LS0264', '9', '124', '2046.04', '-2153.26', '13.6636', '355.281', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.137686', 0, '0', '0', NULL, '0', NULL),
('650', '498', 0, '', '4', '0', '0', 'LS0265', '90', '50', '2124.95', '-2175.88', '13.6151', '313.283', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.071129', 0, '0', '0', NULL, '0', NULL),
('651', '498', 0, '', '4', '0', '0', 'LS0266', '11', '110', '2193.87', '-2273.71', '13.6208', '226.693', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '1.03626', 0, '0', '0', NULL, '0', NULL),
('653', '587', 0, '', '4', '0', '0', 'LS0267', '85', '90', '1183.94', '-1668.11', '13.3115', '180.15', '0', '0', '100', '1000', '0', '11000', '1', NULL, NULL, '0', 0, '9.1108', 0, '0', '0', NULL, '0', NULL),
('654', '579', 0, '', '4', '0', '0', 'LS0268', '56', '39', '2569.35', '-2175.94', '12.9633', '269.947', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '0.343104', 0, '0', '0', NULL, '0', NULL),
('655', '579', 0, '', '4', '0', '0', 'LS0269', '66', '4', '2515.92', '-2052.88', '6.60255', '269.578', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '0.378903', 0, '0', '0', NULL, '0', NULL),
('656', '565', 0, '', '4', '0', '0', 'LS0270', '101', '8', '2463.53', '-1413.47', '23.3735', '270.412', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.70453', 0, '0', '0', NULL, '0', NULL),
('657', '562', 0, '', '4', '0', '0', 'LS0271', '31', '24', '1437.27', '-1845.53', '13.2062', '268.623', '0', '0', '100', '1000', '0', '11000', '1', NULL, NULL, '0', 0, '4.1008', 0, '0', '0', NULL, '0', NULL),
('658', '550', 0, '', '4', '0', '0', 'LS0272', '46', '38', '2161.16', '-1148.2', '24.2102', '271.723', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.033007', 0, '0', '0', NULL, '0', NULL),
('659', '529', 0, '', '4', '0', '0', 'LS0273', '119', '23', '2148.77', '-1153.24', '23.5507', '90.0414', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.021657', 0, '0', '0', NULL, '0', NULL),
('661', '527', 0, '', '4', '0', '0', 'LS0274', '49', '107', '1422.16', '-922.005', '35.7598', '352.199', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.774824', 0, '0', '0', NULL, '0', NULL),
('662', '527', 0, '', '4', '0', '0', 'LS0275', '59', '110', '1254.48', '-1232.07', '16.3654', '88.0857', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '4.90139', 0, '0', '0', NULL, '0', NULL),
('663', '517', 0, '', '4', '0', '0', 'LS0276', '80', '4', '2431.96', '-1653.39', '13.3167', '95.8026', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '10.9122', 0, '0', '0', NULL, '0', NULL),
('664', '482', 0, '', '4', '0', '0', 'LS0277', '60', '1', '2229.09', '-1363.47', '24.1071', '269.816', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.504737', 0, '0', '0', NULL, '0', NULL),
('665', '475', 0, '', '4', '0', '0', 'LS0278', '29', '16', '2162.1', '-1693.66', '14.8926', '175.517', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '2.86898', 0, '0', '0', NULL, '0', NULL),
('666', '471', 0, '', '4', '0', '0', 'LS0279', '43', '16', '2217', '-1313.64', '23.4657', '92.5851', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.043399', 0, '0', '0', NULL, '0', NULL),
('667', '581', 0, '', '4', '0', '0', 'LS0280', '25', '6', '2168.91', '-1395.47', '23.5887', '180.681', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.050802', 0, '0', '0', NULL, '0', NULL),
('668', '401', 0, '', '4', '0', '0', 'LS0281', '97', '21', '2092.65', '-1371.86', '23.7637', '178.905', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.151082', 0, '0', '0', NULL, '0', NULL),
('669', '404', 0, '', '4', '0', '0', 'LS0282', '116', '74', '2096.88', '-1372.38', '23.7205', '179.46', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.010698', 0, '0', '0', NULL, '0', NULL),
('670', '421', 0, '', '4', '0', '0', 'LS0283', '118', '21', '2105.21', '-1233.72', '24.1176', '359.813', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.277852', 0, '0', '0', NULL, '0', NULL),
('671', '426', 0, '', '4', '0', '0', 'LS0284', '84', '51', '2054.9', '-1103.74', '24.2183', '273.68', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.148976', 0, '0', '0', NULL, '0', NULL),
('672', '559', 0, '', '4', '0', '0', 'LS0285', '46', '82', '1455.23', '-921.638', '37.2657', '2.65229', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '0.729449', 0, '0', '0', NULL, '0', NULL),
('673', '526', 0, '', '4', '0', '0', 'LS0286', '61', '109', '1275.46', '-1100.52', '25.6011', '89.4843', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.762476', 0, '0', '0', NULL, '0', NULL),
('674', '505', 0, '', '4', '0', '0', 'LS0287', '37', '100', '1249.56', '-1183.05', '23.7207', '179.308', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '1.28965', 0, '0', '0', NULL, '0', NULL),
('675', '474', 0, '', '4', '0', '0', 'LS0288', '66', '7', '1202.99', '-1203.23', '19.5275', '270.744', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.11573', 0, '0', '0', NULL, '0', NULL),
('676', '470', 0, '', '4', '0', '0', 'LS0289', '121', '92', '708.247', '-1221.79', '17.4636', '307.909', '0', '0', '100', '1000', '0', '4100', '0', NULL, NULL, '0', 0, '0.872632', 0, '0', '0', NULL, '0', NULL),
('677', '467', 0, '', '4', '0', '0', 'LS0290', '80', '37', '720.099', '-1192.26', '19.1085', '332.612', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.022943', 0, '0', '0', NULL, '0', NULL),
('678', '466', 0, '', '4', '0', '0', 'LS0291', '42', '106', '767.058', '-1034.71', '23.7916', '0.540719', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.191554', 0, '0', '0', NULL, '0', NULL),
('679', '462', 0, '', '4', '0', '0', 'LS0292', '9', '115', '746.109', '-1051.01', '23.1818', '120.572', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.010393', 0, '0', '0', NULL, '0', NULL),
('680', '459', 0, '', '4', '0', '0', 'LS0293', '84', '57', '694.306', '-1180.54', '15.4666', '59.9231', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.16557', 0, '0', '0', NULL, '0', NULL),
('681', '458', 0, '', '4', '0', '0', 'LS0294', '41', '40', '612.943', '-1305.56', '14.3988', '357.338', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.172058', 0, '0', '0', NULL, '0', NULL),
('682', '456', 0, '', '4', '0', '0', 'LS0295', '16', '113', '735.868', '-1350.11', '13.6763', '87.5892', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '0.307803', 0, '0', '0', NULL, '0', NULL),
('683', '448', 0, '', '4', '0', '0', 'LS0296', '93', '86', '922.533', '-1350.96', '12.975', '359.024', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.207208', 0, '0', '0', NULL, '0', NULL),
('684', '445', 0, '', '4', '0', '0', 'LS0297', '125', '121', '904.484', '-1454.62', '13.0111', '93.1783', '0', '0', '100', '1000', '0', '3200', '0', NULL, NULL, '0', 0, '0.099151', 0, '0', '0', NULL, '0', NULL),
('685', '440', 0, '', '4', '0', '0', 'LS0298', '33', '102', '963.387', '-1523.62', '13.665', '268.773', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.09168', 0, '0', '0', NULL, '0', NULL),
('686', '436', 0, '', '4', '0', '0', 'LS0299', '54', '19', '1034.91', '-1536.5', '13.2185', '170.229', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '2.52342', 0, '0', '0', NULL, '0', NULL),
('687', '434', 0, '', '4', '0', '0', 'LS0300', '41', '116', '1052.51', '-1667.1', '13.5925', '268.116', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.134996', 0, '0', '0', NULL, '0', NULL),
('688', '573', 0, '', '4', '0', '0', 'LS0301', '54', '17', '952.846', '-1878.59', '8.12697', '78.4638', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '1.61444', 0, '0', '0', NULL, '0', NULL),
('689', '603', 0, '', '4', '0', '0', 'LS0302', '27', '84', '858.509', '-1796.29', '13.6713', '84.6342', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '5.59809', 0, '0', '0', NULL, '0', NULL),
('690', '602', 0, '', '4', '0', '0', 'LS0303', '52', '30', '764.434', '-1660.54', '4.23652', '90.8319', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.251071', 0, '0', '0', NULL, '0', NULL),
('691', '568', 0, '', '4', '0', '0', 'LS0304', '69', '4', '761.599', '-1498.12', '13.4472', '341.091', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.151598', 0, '0', '0', NULL, '0', NULL),
('692', '515', 0, '', '4', '0', '0', 'LS0305', '23', '47', '835.44', '-1452.73', '14.6247', '269.397', '0', '0', '100', '1000', '0', '4900', '0', NULL, NULL, '0', 0, '0.107434', 0, '0', '0', NULL, '0', NULL),
('693', '504', 0, '', '4', '0', '0', 'LS0306', '116', '27', '852.703', '-1453.19', '13.4088', '0.728923', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.031841', 0, '0', '0', NULL, '0', NULL),
('694', '495', 0, '', '4', '0', '0', 'LS0307', '117', '29', '998.11', '-1092.65', '24.1796', '359.402', '0', '0', '100', '1000', '0', '3400', '0', NULL, NULL, '0', 0, '0.507982', 0, '0', '0', NULL, '0', NULL),
('696', '443', 0, '', '4', '0', '0', 'LS0308', '56', '111', '1050.56', '-924.396', '43.3261', '277.236', '0', '0', '100', '1000', '0', '19000', '1', NULL, NULL, '0', 0, '0.28088', 0, '0', '0', NULL, '0', NULL),
('697', '437', 0, '', '4', '0', '0', 'LS0309', '54', '17', '1048.64', '-918.465', '42.799', '276.713', '0', '0', '100', '1000', '0', '115000', '1', NULL, NULL, '0', 0, '0.087329', 0, '0', '0', NULL, '0', NULL),
('698', '431', 0, '', '4', '0', '0', 'LS0310', '116', '81', '1047.62', '-912.884', '42.7627', '277.42', '0', '0', '100', '1000', '0', '155000', '1', NULL, NULL, '0', 0, '0.030994', 0, '0', '0', NULL, '0', NULL),
('699', '438', 0, '', '4', '0', '0', 'LS0311', '19', '124', '925.103', '-997.187', '38.1889', '270.096', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '3.54184', 0, '0', '0', NULL, '0', NULL),
('700', '439', 0, '', '4', '0', '0', 'LS0312', '4', '50', '2165.51', '-1693.82', '14.9818', '355.537', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '18.6986', 0, '0', '0', NULL, '1', NULL),
('701', '455', 0, '', '4', '0', '0', 'LS0313', '14', '12', '212.193', '-1423.29', '13.698', '133.072', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '0.943139', 0, '0', '0', NULL, '0', NULL),
('703', '588', 0, '', '4', '0', '0', 'LS0314', '62', '35', '215.824', '-1427.75', '13.1715', '134.381', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '0.018967', 0, '0', '0', NULL, '0', NULL),
('704', '586', 0, '', '4', '0', '0', 'LS0315', '109', '57', '218.943', '-1431.95', '12.7907', '134.137', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.014541', 0, '0', '0', NULL, '0', NULL),
('705', '585', 0, '', '4', '0', '0', 'LS0316', '76', '70', '204.892', '-1444.17', '12.684', '138.601', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.02723', 0, '0', '0', NULL, '0', NULL),
('706', '578', 0, '', '4', '0', '0', 'LS0317', '62', '47', '195.66', '-1436.23', '13.7016', '317.671', '0', '0', '100', '1000', '0', '3100', '0', NULL, NULL, '0', 0, '0.057208', 0, '0', '0', NULL, '0', NULL),
('707', '575', 0, '', '4', '0', '0', 'LS0318', '42', '125', '199.513', '-1441.22', '12.6798', '138.746', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.037794', 0, '0', '0', NULL, '0', NULL),
('708', '605', 0, '', '4', '0', '0', 'LS0319', '102', '76', '158.255', '-1750.69', '4.73941', '356.35', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.348273', 0, '0', '0', NULL, '0', NULL),
('709', '604', 0, '', '4', '0', '0', 'LS0320', '108', '71', '1109.42', '-1876.24', '13.2887', '89.5569', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '1.02379', 0, '0', '0', NULL, '0', NULL),
('710', '541', 0, '', '4', '0', '0', 'LS0321', '31', '96', '2200.46', '-2636.28', '13.1718', '357.195', '0', '0', '100', '1000', '0', '8200', '0', NULL, NULL, '0', 0, '2.07367', 0, '0', '0', NULL, '0', NULL),
('711', '502', 0, '', '4', '0', '0', 'LS0322', '61', '15', '2323.72', '-2007.79', '13.4385', '359.54', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '0.877288', 0, '0', '0', NULL, '0', NULL),
('712', '451', 0, '', '4', '0', '0', 'LS0323', '50', '22', '2247.01', '-1917.67', '13.2538', '180.098', '0', '0', '100', '1000', '0', '9500', '0', NULL, NULL, '0', 0, '0.164522', 0, '0', '0', NULL, '0', NULL),
('713', '415', 0, '', '4', '0', '0', 'LS0324', '91', '43', '1910.35', '-1117.63', '25.4798', '357.577', '0', '0', '100', '1000', '0', '8000', '0', NULL, NULL, '0', 0, '3.87888', 0, '0', '0', NULL, '0', NULL),
('714', '411', 0, '', '4', '0', '0', 'LS0325', '84', '9', '686.334', '-1072.51', '49.2755', '237.343', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '2.58111', 0, '0', '0', NULL, '0', NULL),
('715', '557', 0, '', '4', '0', '0', 'LS0326', '60', '73', '647.425', '-1049.38', '54.0798', '146.442', '0', '0', '100', '1000', '0', '4500', '0', NULL, NULL, '0', 0, '0.062555', 0, '0', '0', NULL, '0', NULL),
('716', '556', 0, '', '4', '0', '0', 'LS0327', '27', '74', '154.768', '-751.976', '36.2476', '64.1312', '0', '0', '100', '1000', '0', '4500', '0', NULL, NULL, '0', 0, '0.648003', 0, '0', '0', NULL, '0', NULL),
('717', '503', 0, '', '4', '0', '0', 'LS0328', '108', '100', '346.565', '-1197.43', '76.4103', '216.371', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '0.755089', 0, '0', '0', NULL, '0', NULL),
('718', '477', 0, '', '4', '0', '0', 'LS0329', '1', '28', '289.291', '-1158.67', '80.6621', '42.4495', '0', '0', '100', '1000', '0', '3300', '0', NULL, NULL, '0', 0, '0.050252', 0, '0', '0', NULL, '0', NULL),
('719', '403', 0, '', '4', '0', '0', 'LS0330', '39', '39', '360.564', '-804.278', '13.2313', '342.105', '0', '0', '100', '1000', '0', '4700', '0', NULL, NULL, '0', 0, '0.492947', 0, '0', '0', NULL, '0', NULL),
('720', '410', 0, '', '4', '0', '0', 'LS0331', '117', '68', '2474.1', '-2633.1', '13.3083', '90.5872', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '4.9763', 0, '0', '0', NULL, '0', NULL),
('721', '410', 0, '', '4', '0', '0', 'LS0332', '17', '2', '2474.09', '-2630.22', '13.3051', '91.1036', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '0.03483', 0, '0', '0', NULL, '0', NULL),
('722', '410', 0, '', '4', '0', '0', 'LS0333', '46', '3', '2474.11', '-2627.24', '13.3048', '90.5508', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '1.66013', 0, '0', '0', NULL, '0', NULL),
('723', '410', 0, '', '4', '0', '0', 'LS0334', '51', '9', '1046.71', '-1044.84', '31.5191', '268.995', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '9.4703', 0, '0', '0', NULL, '0', NULL),
('724', '410', 0, '', '4', '0', '0', 'LS0335', '35', '100', '2489.69', '-2642.22', '13.1991', '0.313699', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '11.1985', 0, '0', '0', NULL, '0', NULL),
('725', '410', 0, '', '4', '0', '0', 'LS0336', '21', '28', '2489.55', '-2636.87', '13.203', '359.869', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '3.4386', 0, '0', '0', NULL, '0', NULL),
('726', '410', 0, '', '4', '0', '0', 'LS0337', '42', '119', '2489.65', '-2631.61', '13.2101', '358.689', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '13.9447', 0, '0', '0', NULL, '0', NULL),
('727', '410', 0, '', '4', '0', '0', 'LS0338', '101', '120', '2479.68', '-2639.92', '13.1961', '0.760729', '0', '0', '100', '1000', '0', '500', '1', NULL, NULL, '0', 0, '60.8954', 0, '0', '0', NULL, '0', NULL),
('728', '567', 0, '', '4', '0', '0', 'LS0339', '50', '60', '-2348.52', '-125.282', '35.1825', '359.281', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '5.46728', 0, '0', '0', NULL, '0', NULL),
('729', '561', 0, '', '4', '0', '0', 'LS0340', '1', '29', '-2027.43', '126.023', '28.8638', '178.912', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.70986', 0, '0', '0', NULL, '0', NULL),
('730', '551', 0, '', '4', '0', '0', 'LS0341', '42', '53', '-2072.79', '-83.1539', '34.9648', '0.303258', '0', '0', '100', '1000', '0', '50000', '1', NULL, NULL, '0', 0, '0.7591', 0, '0', '0', NULL, '0', NULL),
('731', '589', 0, '', '4', '0', '0', 'LS0342', '120', '50', '-1929.27', '586.108', '34.7682', '179.315', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.392294', 0, '0', '0', NULL, '0', NULL),
('732', '580', 0, '', '4', '0', '0', 'LS0343', '65', '66', '-1730.79', '761.907', '24.6875', '89.6348', '0', '0', '100', '1000', '0', '6500', '0', NULL, NULL, '0', 0, '0.342761', 0, '0', '0', NULL, '0', NULL),
('733', '568', 0, '', '4', '0', '0', 'LS0344', '112', '19', '-1506.92', '732.538', '7.05474', '270.788', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.210523', 0, '0', '0', NULL, '0', NULL),
('734', '565', 0, '', '4', '0', '0', 'LS0345', '28', '9', '-1751.11', '949.19', '24.3654', '89.689', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.380603', 0, '0', '0', NULL, '0', NULL),
('735', '561', 0, '', '4', '0', '0', 'LS0346', '72', '40', '-1887.55', '971.632', '34.9047', '350.088', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.174372', 0, '0', '0', NULL, '0', NULL),
('736', '556', 0, '', '4', '0', '0', 'LS0347', '2', '25', '-2029.82', '707.237', '51.4146', '158.046', '0', '0', '100', '1000', '0', '4500', '0', NULL, NULL, '0', 0, '0.351005', 0, '0', '0', NULL, '0', NULL),
('737', '555', 0, '', '4', '0', '0', 'LS0348', '96', '109', '-2134.42', '838.039', '69.1685', '358.713', '0', '0', '100', '1000', '0', '3700', '0', NULL, NULL, '0', 0, '0.247572', 0, '0', '0', NULL, '0', NULL),
('738', '554', 0, '', '4', '0', '0', 'LS0349', '26', '125', '-2173.18', '1104.64', '80.0832', '359.148', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.275431', 0, '0', '0', NULL, '0', NULL),
('739', '552', 0, '', '4', '0', '0', 'LS0350', '21', '43', '-2354.53', '1232.51', '31.8206', '264.557', '0', '0', '100', '1000', '0', '2600', '0', NULL, NULL, '0', 0, '0.303537', 0, '0', '0', NULL, '0', NULL),
('740', '550', 0, '', '4', '0', '0', 'LS0351', '109', '29', '-2439.83', '1297.7', '19.6493', '180.444', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.260173', 0, '0', '0', NULL, '0', NULL),
('741', '549', 0, '', '4', '0', '0', 'LS0352', '119', '21', '-2636.56', '1375.87', '6.82928', '0.46111', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.241481', 0, '0', '0', NULL, '0', NULL),
('742', '547', 0, '', '4', '0', '0', 'LS0353', '56', '94', '-2638.83', '1335.69', '6.9218', '179.575', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.02906', 0, '0', '0', NULL, '0', NULL),
('743', '546', 0, '', '4', '0', '0', 'LS0354', '91', '96', '-2899.96', '1175.9', '12.6316', '89.7698', '0', '0', '100', '1000', '0', '2600', '0', NULL, NULL, '0', 0, '0.425454', 0, '0', '0', NULL, '0', NULL),
('744', '545', 0, '', '4', '0', '0', 'LS0355', '3', '36', '-2894.09', '1032.4', '35.6803', '108.389', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '0.138634', 0, '0', '0', NULL, '0', NULL),
('745', '543', 0, '', '4', '0', '0', 'LS0356', '35', '62', '-2796.93', '776.426', '49.9698', '183.358', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.450881', 0, '0', '0', NULL, '0', NULL),
('746', '542', 0, '', '4', '0', '0', 'LS0357', '31', '44', '-2673.29', '800.983', '49.7215', '358.555', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.128434', 0, '0', '0', NULL, '0', NULL),
('747', '535', 0, '', '4', '0', '0', 'LS0358', '78', '95', '-2539.62', '938.924', '64.1395', '90.9957', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.288383', 0, '0', '0', NULL, '0', NULL),
('749', '534', 0, '', '4', '0', '0', 'LS0359', '104', '20', '-2236.25', '853.897', '63.9741', '268.681', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.409224', 0, '0', '0', NULL, '0', NULL),
('750', '533', 0, '', '4', '0', '0', 'LS0360', '53', '71', '-2425.59', '956.665', '45.006', '88.0427', '0', '0', '100', '1000', '0', '4700', '0', NULL, NULL, '0', 0, '0.240337', 0, '0', '0', NULL, '0', NULL),
('751', '529', 0, '', '4', '0', '0', 'LS0361', '102', '34', '-2544.36', '587.237', '14.0853', '271.17', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.571975', 0, '0', '0', NULL, '0', NULL),
('752', '526', 0, '', '4', '0', '0', 'LS0362', '1', '28', '-2589.15', '622.366', '14.2258', '91.0605', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.043597', 0, '0', '0', NULL, '0', NULL),
('753', '518', 0, '', '4', '0', '0', 'LS0363', '80', '63', '-2657.93', '379.714', '3.90192', '179.955', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.438083', 0, '0', '0', NULL, '0', NULL),
('754', '517', 0, '', '4', '0', '0', 'LS0364', '43', '105', '-2665.22', '268.858', '4.19083', '180.383', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.09588', 0, '0', '0', NULL, '0', NULL),
('755', '516', 0, '', '4', '0', '0', 'LS0365', '95', '93', '-2796.51', '209.981', '7.02119', '269.691', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.218091', 0, '0', '0', NULL, '0', NULL),
('756', '515', 0, '', '4', '0', '0', 'LS0366', '71', '35', '-2774.12', '-283.286', '8.06456', '1.19978', '0', '0', '100', '1000', '0', '4900', '0', NULL, NULL, '0', 0, '0.577382', 0, '0', '0', NULL, '0', NULL),
('757', '514', 0, '', '4', '0', '0', 'LS0367', '120', '103', '-2895.33', '-1191.19', '9.63755', '171.62', '0', '0', '100', '1000', '0', '5100', '0', NULL, NULL, '0', 0, '0.951144', 0, '0', '0', NULL, '0', NULL),
('758', '507', 0, '', '4', '0', '0', 'LS0368', '21', '42', '-2127.39', '-415.167', '35.1594', '160.249', '0', '0', '100', '1000', '0', '3300', '0', NULL, NULL, '0', 0, '1.45817', 0, '0', '0', NULL, '0', NULL),
('759', '505', 0, '', '4', '0', '0', 'LS0369', '94', '67', '-1977.43', '-542.762', '36.5905', '271.07', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.216946', 0, '0', '0', NULL, '0', NULL),
('760', '500', 0, '', '4', '0', '0', 'LS0370', '77', '113', '-1848.7', '-172.692', '9.48324', '87.9897', '0', '0', '100', '1000', '0', '2600', '0', NULL, NULL, '0', 0, '0.447776', 0, '0', '0', NULL, '0', NULL),
('761', '499', 0, '', '4', '0', '0', 'LS0371', '21', '107', '-1818.52', '-163.156', '9.39514', '182.864', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '0.048479', 0, '0', '0', NULL, '0', NULL),
('762', '489', 0, '', '4', '0', '0', 'LS0372', '53', '3', '-1716.96', '-83.0284', '3.69745', '314.829', '0', '0', '100', '1000', '0', '2600', '0', NULL, NULL, '0', 0, '0.174363', 0, '0', '0', NULL, '0', NULL),
('763', '482', 0, '', '4', '0', '0', 'LS0373', '70', '2', '-1687.34', '386.336', '7.30356', '314.548', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.548601', 0, '0', '0', NULL, '0', NULL),
('764', '479', 0, '', '4', '0', '0', 'LS0374', '29', '34', '-1703.61', '408.167', '6.97263', '42.5836', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.010068', 0, '0', '0', NULL, '0', NULL),
('765', '474', 0, '', '4', '0', '0', 'LS0375', '93', '114', '-1588.03', '674.237', '6.95027', '2.15163', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.407806', 0, '0', '0', NULL, '0', NULL),
('766', '471', 0, '', '4', '0', '0', 'LS0376', '77', '9', '-1659.05', '766.313', '17.599', '0.518223', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.226951', 0, '0', '0', NULL, '0', NULL),
('767', '470', 0, '', '4', '0', '0', 'LS0377', '73', '19', '-1915.4', '793.212', '39.6392', '90.5828', '0', '0', '100', '1000', '0', '4100', '0', NULL, NULL, '0', 0, '0.36947', 0, '0', '0', NULL, '0', NULL),
('768', '467', 0, '', '4', '0', '0', 'LS0378', '115', '26', '-1985.16', '947.63', '45.1853', '179.948', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.210035', 0, '0', '0', NULL, '0', NULL),
('769', '466', 0, '', '4', '0', '0', 'LS0379', '43', '76', '-1987.18', '1168.45', '45.109', '89.9086', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.273872', 0, '0', '0', NULL, '0', NULL),
('770', '462', 0, '', '4', '0', '0', 'LS0380', '63', '96', '-2060.37', '1229.36', '31.1706', '88.6344', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.10075', 0, '0', '0', NULL, '0', NULL),
('771', '459', 0, '', '4', '0', '0', 'LS0381', '58', '18', '-2009.24', '1361.87', '7.2295', '359.266', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.157084', 0, '0', '0', NULL, '0', NULL),
('772', '458', 0, '', '4', '0', '0', 'LS0382', '73', '42', '-1698.37', '1035.54', '45.0896', '271.078', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.529401', 0, '0', '0', NULL, '0', NULL),
('773', '456', 0, '', '4', '0', '0', 'LS0383', '27', '23', '-1722.59', '288.862', '7.36085', '231.037', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '1.10371', 0, '0', '0', NULL, '0', NULL),
('774', '455', 0, '', '4', '0', '0', 'LS0384', '41', '32', '-1719.33', '295.645', '7.624', '52.3936', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '0.027743', 0, '0', '0', NULL, '0', NULL),
('775', '448', 0, '', '4', '0', '0', 'LS0385', '41', '49', '-1817.87', '613.765', '34.7704', '272.214', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.4877', 0, '0', '0', NULL, '0', NULL),
('776', '445', 0, '', '4', '0', '0', 'LS0386', '123', '1', '-1950.5', '586.329', '34.9703', '0.807549', '0', '0', '100', '1000', '0', '3200', '0', NULL, NULL, '0', 0, '0.143521', 0, '0', '0', NULL, '0', NULL),
('777', '444', 0, '', '4', '0', '0', 'LS0387', '107', '37', '-2043.51', '607.826', '42.9533', '73.994', '0', '0', '100', '1000', '0', '4700', '0', NULL, NULL, '0', 0, '0.078697', 0, '0', '0', NULL, '0', NULL),
('778', '443', 0, '', '4', '0', '0', 'LS0388', '86', '99', '-2007.76', '299.302', '35.2188', '179.963', '0', '0', '100', '1000', '0', '23000', '1', NULL, NULL, '0', 0, '0.562094', 0, '0', '0', NULL, '0', NULL),
('779', '540', 0, '', '4', '0', '0', 'LS0389', '12', '83', '-2186.57', '514.111', '34.9516', '88.6645', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.703516', 0, '0', '0', NULL, '0', NULL),
('780', '496', 0, '', '4', '0', '0', 'LS0390', '31', '37', '-2157.06', '655.164', '52.0854', '359.071', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.208531', 0, '0', '0', NULL, '0', NULL),
('781', '442', 0, '', '4', '0', '0', 'LS0391', '19', '21', '-2273.82', '540.357', '34.8987', '90.7266', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.214406', 0, '0', '0', NULL, '0', NULL),
('782', '440', 0, '', '4', '0', '0', 'LS0392', '2', '64', '-2425.52', '518.389', '30.0515', '41.0598', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.235894', 0, '0', '0', NULL, '0', NULL),
('783', '437', 0, '', '4', '0', '0', 'LS0393', '95', '78', '-2409.91', '500.914', '30.1267', '110.059', '0', '0', '100', '1000', '0', '7000', '0', NULL, NULL, '0', 0, '0.086989', 0, '0', '0', NULL, '0', NULL),
('784', '436', 0, '', '4', '0', '0', 'LS0394', '62', '118', '-2503.68', '378.227', '34.886', '56.7061', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.196734', 0, '0', '0', NULL, '0', NULL),
('785', '434', 0, '', '4', '0', '0', 'LS0395', '43', '103', '-2526.43', '325.664', '35.0852', '72.2581', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.068638', 0, '0', '0', NULL, '0', NULL),
('786', '431', 0, '', '4', '0', '0', 'LS0396', '92', '82', '-2508.72', '372.322', '27.8774', '328.474', '0', '0', '100', '1000', '0', '5300', '0', NULL, NULL, '0', 0, '0.178654', 0, '0', '0', NULL, '0', NULL),
('787', '426', 0, '', '4', '0', '0', 'LS0397', '104', '20', '-2648.65', '205.131', '4.07864', '178.515', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.283592', 0, '0', '0', NULL, '0', NULL),
('788', '423', 0, '', '4', '0', '0', 'LS0398', '70', '124', '-2675', '267.728', '4.35724', '179.636', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.09727', 0, '0', '0', NULL, '0', NULL),
('790', '421', 0, '', '4', '0', '0', 'LS0399', '69', '31', '-2693.83', '67.7858', '4.21842', '269.623', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.300861', 0, '0', '0', NULL, '0', NULL),
('791', '419', 0, '', '4', '0', '0', 'LS0400', '13', '50', '-2796.64', '-92.5888', '6.98508', '269.975', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.431578', 0, '0', '0', NULL, '0', NULL),
('792', '418', 0, '', '4', '0', '0', 'LS0401', '41', '84', '-2754.54', '-166.45', '7.03133', '357.826', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.198798', 0, '0', '0', NULL, '0', NULL),
('793', '404', 0, '', '4', '0', '0', 'LS0402', '98', '60', '-2817.13', '-284.675', '6.83721', '174.435', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.110276', 0, '0', '0', NULL, '0', NULL),
('794', '403', 0, '', '4', '0', '0', 'LS0403', '67', '119', '-2549.07', '-0.426814', '17.0275', '179.34', '0', '0', '100', '1000', '0', '4700', '0', NULL, NULL, '0', 0, '0.671718', 0, '0', '0', NULL, '0', NULL),
('795', '401', 0, '', '4', '0', '0', 'LS0404', '125', '62', '-2650.65', '58.2948', '3.94483', '1.60259', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.13437', 0, '0', '0', NULL, '0', NULL),
('796', '400', 0, '', '4', '0', '0', 'LS0405', '115', '123', '-2495.59', '155.308', '8.02255', '294.733', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '0.360444', 0, '0', '0', NULL, '0', NULL),
('797', '557', 0, '', '4', '0', '0', 'LS0406', '56', '75', '-2575.15', '104.428', '4.71097', '179.128', '0', '0', '100', '1000', '0', '4500', '0', NULL, NULL, '0', 0, '0.086911', 0, '0', '0', NULL, '0', NULL),
('798', '588', 0, '', '4', '0', '0', 'LS0407', '94', '36', '-2875.36', '465.404', '4.76439', '358.516', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '0.61084', 0, '0', '0', NULL, '0', NULL),
('799', '578', 0, '', '4', '0', '0', 'LS0408', '116', '28', '-2925.46', '491.044', '5.53411', '89.0164', '0', '0', '100', '1000', '0', '3100', '0', NULL, NULL, '0', 0, '0.035492', 0, '0', '0', NULL, '0', NULL),
('800', '554', 0, '', '4', '0', '0', 'LS0409', '58', '68', '-2906.93', '489.009', '4.91406', '264.44', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.020022', 0, '0', '0', NULL, '0', NULL),
('801', '549', 0, '', '4', '0', '0', 'LS0410', '70', '106', '-2905.03', '422.429', '4.61121', '179.132', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.065126', 0, '0', '0', NULL, '0', NULL),
('802', '547', 0, '', '4', '0', '0', 'LS0411', '72', '47', '-2860.94', '687.434', '23.0765', '113.689', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.300694', 0, '0', '0', NULL, '0', NULL),
('803', '546', 0, '', '4', '0', '0', 'LS0412', '77', '81', '-2880.46', '740.419', '29.3887', '96.6775', '0', '0', '100', '1000', '0', '2600', '0', NULL, NULL, '0', 0, '0.06921', 0, '0', '0', NULL, '0', NULL),
('804', '543', 0, '', '4', '0', '0', 'LS0413', '108', '104', '-2856.47', '829.397', '40.1489', '62.1831', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.088292', 0, '0', '0', NULL, '0', NULL),
('805', '533', 0, '', '4', '0', '0', 'LS0414', '41', '87', '-2835.52', '888.234', '43.7638', '88.9607', '0', '0', '100', '1000', '0', '4700', '0', NULL, NULL, '0', 0, '0.07669', 0, '0', '0', NULL, '0', NULL),
('806', '499', 0, '', '4', '0', '0', 'LS0415', '31', '24', '-2848.07', '953.617', '44.0466', '114.639', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '0.086966', 0, '0', '0', NULL, '0', NULL),
('807', '609', 0, '', '4', '0', '0', 'LS0416', '35', '118', '-2847.64', '984.265', '43.0896', '37.9795', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.044784', 0, '0', '0', NULL, '0', NULL),
('808', '600', 0, '', '4', '0', '0', 'LS0417', '61', '124', '-2710.34', '1310.33', '6.78412', '0.155465', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.386149', 0, '0', '0', NULL, '0', NULL),
('809', '567', 0, '', '4', '0', '0', 'LS0418', '43', '9', '-2695.54', '1297.16', '6.97991', '269.142', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.008949', 0, '0', '0', NULL, '0', NULL),
('810', '552', 0, '', '4', '0', '0', 'LS0419', '62', '99', '-2629.77', '1335.96', '6.89214', '1.90649', '0', '0', '100', '1000', '0', '2600', '0', NULL, NULL, '0', 0, '0.092626', 0, '0', '0', NULL, '0', NULL),
('811', '542', 0, '', '4', '0', '0', 'LS0420', '104', '17', '-2619.05', '1376.4', '6.86926', '2.177', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.049454', 0, '0', '0', NULL, '0', NULL),
('812', '529', 0, '', '4', '0', '0', 'LS0421', '8', '43', '-2623.28', '1376.25', '6.75785', '183.466', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.006049', 0, '0', '0', NULL, '0', NULL),
('814', '518', 0, '', '4', '0', '0', 'LS0422', '31', '19', '-1852.05', '1095.59', '45.035', '272.612', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.962301', 0, '0', '0', NULL, '0', NULL),
('815', '496', 0, '', '4', '0', '0', 'LS0423', '100', '42', '-1633.54', '1110.31', '23.3336', '268.469', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.221318', 0, '0', '0', NULL, '0', NULL),
('816', '482', 0, '', '4', '0', '0', 'LS0424', '83', '6', '-1545.16', '1111.47', '7.30776', '268.79', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.056698', 0, '0', '0', NULL, '0', NULL),
('817', '479', 0, '', '4', '0', '0', 'LS0425', '46', '114', '-1630.03', '893.44', '9.84505', '181.985', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.254691', 0, '0', '0', NULL, '0', NULL),
('818', '471', 0, '', '4', '0', '0', 'LS0426', '7', '28', '-1629.08', '871.317', '7.78159', '177.899', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('819', '467', 0, '', '4', '0', '0', 'LS0427', '114', '77', '-2285.65', '726.781', '49.174', '89.4426', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.879856', 0, '0', '0', NULL, '0', NULL),
('820', '466', 0, '', '4', '0', '0', 'LS0428', '98', '52', '-2515.69', '1004.31', '78.053', '90.0608', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.676863', 0, '0', '0', NULL, '0', NULL),
('821', '462', 0, '', '4', '0', '0', 'LS0429', '59', '31', '-2558.79', '1009.31', '77.8044', '89.1039', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.030598', 0, '0', '0', NULL, '0', NULL),
('824', '448', 0, '', '4', '0', '0', 'LS0430', '95', '1', '-2572.12', '1009.6', '78.1406', '97.0709', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('825', '440', 0, '', '4', '0', '0', 'LS0431', '60', '38', '-2724.24', '987.296', '54.592', '182.088', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.162159', 0, '0', '0', NULL, '0', NULL),
('826', '414', 0, '', '4', '0', '0', 'LS0432', '62', '65', '2607.18', '-1189.19', '63.4776', '269.216', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '1.94537', 0, '0', '0', NULL, '0', NULL),
('1024', '403', 0, '', '4', '0', '0', 'LS0433', '62', '24', '-2108.33', '-2419.81', '31.2323', '137.362', '0', '0', '100', '1000', '0', '4700', '0', NULL, NULL, '0', 0, '0.428573', 0, '0', '0', NULL, '0', NULL),
('1025', '404', 0, '', '4', '0', '0', 'LS0434', '65', '47', '-2225.16', '-2401.93', '31.7213', '231.638', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.384338', 0, '0', '0', NULL, '0', NULL),
('1026', '418', 0, '', '4', '0', '0', 'LS0435', '101', '69', '-2181.15', '-2478.36', '30.7101', '140.433', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.285253', 0, '0', '0', NULL, '0', NULL),
('1027', '423', 0, '', '4', '0', '0', 'LS0436', '8', '78', '-2198.14', '-2265.18', '30.6527', '53.0339', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.564869', 0, '0', '0', NULL, '0', NULL),
('1028', '431', 0, '', '4', '0', '0', 'LS0437', '106', '76', '-2383.61', '-2170.28', '33.3895', '85.907', '0', '0', '100', '1000', '0', '125000', '1', NULL, NULL, '0', 0, '0.235813', 0, '0', '0', NULL, '0', NULL),
('1029', '437', 0, '', '4', '0', '0', 'LS0438', '25', '102', '-2397.2', '-2169.51', '33.4233', '86.7122', '0', '0', '100', '1000', '0', '125000', '1', NULL, NULL, '0', 0, '0.052884', 0, '0', '0', NULL, '0', NULL),
('1030', '442', 0, '', '4', '0', '0', 'LS0439', '115', '19', '-2123.41', '-2486.99', '30.4569', '233.06', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.537979', 0, '0', '0', NULL, '0', NULL),
('1031', '443', 0, '', '4', '0', '0', 'LS0440', '6', '14', '-2241.64', '-2568.03', '32.559', '64.9294', '0', '0', '100', '1000', '0', '5300', '0', NULL, NULL, '0', 0, '0.378291', 0, '0', '0', NULL, '0', NULL),
('1032', '455', 0, '', '4', '0', '0', 'LS0441', '60', '63', '-2060.24', '-2441.6', '31.0609', '141.665', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '0.394112', 0, '0', '0', NULL, '0', NULL),
('1033', '456', 0, '', '4', '0', '0', 'LS0442', '7', '81', '-2149.07', '-2543.85', '30.7901', '138.795', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '0.452279', 0, '0', '0', NULL, '0', NULL),
('1034', '459', 0, '', '4', '0', '0', 'LS0443', '69', '122', '-2145.19', '-2445.84', '30.6747', '143.574', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '2.97098', 0, '0', '0', NULL, '0', NULL),
('1035', '514', 0, '', '4', '0', '0', 'LS0444', '97', '7', '-2098.74', '-2230.69', '31.3026', '323.378', '0', '0', '100', '1000', '0', '5100', '0', NULL, NULL, '0', 0, '0.294512', 0, '0', '0', NULL, '0', NULL),
('1036', '515', 0, '', '4', '0', '0', 'LS0445', '12', '9', '-2093.2', '-2235.47', '31.9069', '141.01', '0', '0', '100', '1000', '0', '4900', '0', NULL, NULL, '0', 0, '0.090419', 0, '0', '0', NULL, '0', NULL),
('1041', '440', 0, '', '4', '0', '0', 'LS0446', '112', '52', '-2149.39', '-2442.29', '30.7382', '143.355', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.578279', 0, '-2149.39', '-2442.29', NULL, '1', NULL),
('1043', '483', 0, '', '4', '0', '0', 'LS0447', '0', '0', '-1115', '-1620.32', '76.3645', '178.938', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '0.037519', 0, '0', '0', NULL, '0', NULL),
('1044', '483', 0, '', '4', '0', '0', 'LS0448', '92', '15', '-1110.92', '-1620.29', '76.3612', '181.329', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '0.031563', 0, '0', '0', NULL, '0', NULL),
('1045', '440', 0, '', '4', '0', '0', 'LS0449', '17', '79', '-1105.37', '-1642.77', '76.4801', '273.492', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.032522', 0, '0', '0', NULL, '0', NULL),
('1046', '593', 0, '', '4', '0', '0', 'LS0450', '23', '23', '-1081.51', '-1625.23', '76.8299', '270.166', '0', '0', '100', '1000', '0', '1500000', '1', NULL, NULL, '0', 0, '0.059151', 0, '0', '0', NULL, '0', NULL),
('1047', '551', 0, '', '4', '0', '0', 'LS0451', '55', '1', '-1106.37', '-1672.59', '76.1711', '181.023', '0', '0', '100', '1000', '0', '2800', '1', NULL, NULL, '0', 0, '0.036418', 0, '0', '0', NULL, '0', NULL),
('1048', '492', 0, '', '4', '0', '0', 'LS0452', '9', '6', '324.382', '-1808.42', '4.28095', '179.529', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.034414', 0, '0', '0', NULL, '0', NULL),
('1049', '404', 0, '', '4', '0', '0', 'LS0453', '76', '52', '347.021', '-1809.1', '4.26433', '179.609', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.03422', 0, '0', '0', NULL, '0', NULL),
('1050', '413', 0, '', '4', '0', '0', 'LS0454', '58', '114', '331.226', '-1789.63', '4.96098', '180.072', '0', '0', '0', '959.124', '0', '2300', '0', NULL, NULL, '0', 0, '2.79293', 0, '0', '0', NULL, '0', NULL),
('1051', '605', 0, '', '4', '0', '0', 'LS0455', '92', '54', '-1763', '-128.561', '3.36709', '178.556', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.107165', 0, '0', '0', NULL, '0', NULL),
('1052', '604', 0, '', '4', '0', '0', 'LS0456', '5', '93', '-2085.41', '-84.0748', '34.9063', '0.413778', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.360932', 0, '0', '0', NULL, '0', NULL),
('1053', '589', 0, '', '4', '0', '0', 'LS0457', '92', '24', '-2206.09', '-64.8161', '34.9021', '88.7999', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.110676', 0, '0', '0', NULL, '0', NULL),
('1054', '587', 0, '', '4', '0', '0', 'LS0458', '56', '107', '-2323.32', '-42.3105', '35.0407', '90.476', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '0.112146', 0, '0', '0', NULL, '0', NULL),
('1055', '562', 0, '', '4', '0', '0', 'LS0459', '47', '44', '-2529.89', '-142.939', '19.4284', '179.207', '0', '0', '100', '1000', '0', '3100', '0', NULL, NULL, '0', 0, '0.216859', 0, '0', '0', NULL, '0', NULL),
('1056', '559', 0, '', '4', '0', '0', 'LS0460', '81', '61', '-2654.19', '-179.454', '3.73175', '2.38362', '0', '0', '100', '1000', '0', '2900', '0', NULL, NULL, '0', 0, '0.24772', 0, '0', '0', NULL, '0', NULL),
('1057', '539', 0, '', '4', '0', '0', 'LS0461', '123', '17', '-2895.41', '-205.225', '3.02927', '316.658', '0', '0', '100', '1000', '0', '3600', '0', NULL, NULL, '0', 0, '0.230618', 0, '0', '0', NULL, '0', NULL),
('1058', '522', 0, '', '4', '0', '0', 'LS0462', '25', '35', '-2373.67', '600.766', '28.3001', '1.56672', '0', '0', '100', '1000', '0', '15000', '1', NULL, NULL, '0', 0, '2.07373', 0, '0', '0', NULL, '0', NULL),
('1059', '503', 0, '', '4', '0', '0', 'LS0463', '119', '33', '-2407.18', '918.869', '45.4046', '4.57103', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '0.276206', 0, '0', '0', NULL, '0', NULL),
('1060', '502', 0, '', '4', '0', '0', 'LS0464', '51', '101', '-2339.64', '1021.48', '50.5866', '272.784', '0', '0', '100', '1000', '0', '3800', '0', NULL, NULL, '0', 0, '0.133037', 0, '0', '0', NULL, '0', NULL),
('1061', '495', 0, '', '4', '0', '0', 'LS0465', '25', '103', '-2313.41', '1072.19', '65.9374', '174.952', '0', '0', '100', '1000', '0', '15000', '1', NULL, NULL, '0', 0, '0.153344', 0, '0', '0', NULL, '0', NULL),
('1062', '480', 0, '', '4', '0', '0', 'LS0466', '23', '5', '-2380.58', '1305.46', '18.2402', '91.9151', '0', '0', '100', '1000', '0', '5100', '0', NULL, NULL, '0', 0, '0.134955', 0, '0', '0', NULL, '0', NULL),
('1063', '477', 0, '', '4', '0', '0', 'LS0467', '90', '71', '-2064.33', '1380.91', '6.85418', '359.952', '0', '0', '100', '1000', '0', '12000', '1', NULL, NULL, '0', 0, '0.341262', 0, '0', '0', NULL, '0', NULL),
('1064', '451', 0, '', '4', '0', '0', 'LS0468', '72', '72', '-1644.79', '1389.44', '6.87869', '313.81', '0', '0', '100', '1000', '0', '9500', '0', NULL, NULL, '0', 0, '0.445519', 0, '0', '0', NULL, '0', NULL),
('1065', '429', 0, '', '4', '0', '0', 'LS0469', '47', '105', '-1660.9', '1212.72', '6.92958', '88.9704', '0', '0', '100', '1000', '0', '13200', '1', NULL, NULL, '0', 0, '0.121854', 0, '0', '0', NULL, '0', NULL),
('1066', '415', 0, '', '4', '0', '0', 'LS0470', '16', '115', '-1689.24', '987.361', '17.3587', '269.525', '0', '0', '100', '1000', '0', '13200', '1', NULL, NULL, '0', 0, '0.485871', 0, '0', '0', NULL, '0', NULL),
('1067', '411', 0, '', '4', '0', '0', 'LS0471', '108', '122', '-1688.02', '1020.15', '17.313', '270.422', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.025367', 0, '0', '0', NULL, '0', NULL),
('1068', '603', 0, '', '4', '0', '0', 'LS0472', '14', '96', '-1699.52', '1058.49', '17.4238', '358.757', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.028035', 0, '0', '0', NULL, '0', NULL),
('1069', '581', 0, '', '4', '0', '0', 'LS0473', '70', '44', '-1735.72', '1024.61', '17.1823', '88.9948', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.052727', 0, '0', '0', NULL, '0', NULL),
('1070', '573', 0, '', '4', '0', '0', 'LS0474', '17', '75', '-2056.06', '926.857', '57.9343', '75.4669', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.323541', 0, '0', '0', NULL, '0', NULL),
('1071', '589', 0, '', '4', '0', '0', 'LS0475', '85', '65', '-2173.98', '-2533.74', '30.2752', '140.911', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.095876', 0, '0', '0', NULL, '0', NULL),
('1072', '566', 0, '', '4', '0', '0', 'LS0476', '82', '56', '-2052.14', '-2467.24', '30.4078', '141.239', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.195759', 0, '0', '0', NULL, '0', NULL),
('1073', '527', 0, '', '4', '0', '0', 'LS0477', '65', '114', '-2192.63', '-2269.43', '30.3403', '51.0298', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '1.06047', 0, '0', '0', NULL, '0', NULL),
('1074', '521', 0, '', '4', '0', '0', 'LS0478', '18', '74', '-2183.99', '-2284.82', '30.4688', '231.262', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.051229', 0, '0', '0', NULL, '0', NULL),
('1075', '505', 0, '', '4', '0', '0', 'LS0479', '31', '1', '-2387.53', '-2189.23', '33.4327', '357.9', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.247974', 0, '0', '0', NULL, '0', NULL),
('1076', '504', 0, '', '4', '0', '0', 'LS0480', '8', '61', '-2094.77', '-2265.75', '30.3353', '50.3818', '0', '0', '100', '1000', '0', '7500', '1', NULL, NULL, '0', 0, '0.413573', 0, '0', '0', NULL, '0', NULL),
('1077', '475', 0, '', '4', '0', '0', 'LS0481', '25', '120', '-2088.89', '-2270.58', '30.3461', '51.3262', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.025022', 0, '0', '0', NULL, '0', NULL),
('1078', '468', 0, '', '4', '0', '0', 'LS0482', '17', '25', '-2197.82', '-2237.09', '30.5086', '56.1312', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.105214', 0, '0', '0', NULL, '0', NULL),
('1079', '439', 0, '', '4', '0', '0', 'LS0483', '73', '55', '-2237.51', '-2552.09', '31.8177', '57.1318', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.363476', 0, '0', '0', NULL, '0', NULL),
('1080', '438', 0, '', '4', '0', '0', 'LS0484', '57', '28', '-1615.61', '-2695.06', '48.5393', '326.742', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.893644', 0, '0', '0', NULL, '0', NULL),
('1081', '585', 0, '', '4', '0', '0', 'LS0485', '1', '44', '-1611.57', '-2697.76', '48.1223', '145.78', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.028971', 0, '0', '0', NULL, '0', NULL),
('1082', '576', 0, '', '4', '0', '0', 'LS0486', '2', '97', '-1607.56', '-2700.9', '48.1452', '147.977', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.014241', 0, '0', '0', NULL, '0', NULL),
('1083', '575', 0, '', '4', '0', '0', 'LS0487', '12', '15', '-1628.65', '-2695.69', '48.1417', '50.8651', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.026181', 0, '0', '0', NULL, '0', NULL),
('1084', '505', 0, '', '4', '0', '0', 'LS0488', '49', '123', '-1631.65', '-2698.86', '48.6832', '231.085', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.027527', 0, '0', '0', NULL, '0', NULL),
('1085', '504', 0, '', '4', '0', '0', 'LS0489', '20', '83', '-1589.88', '-2712.2', '48.3315', '148.761', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.063696', 0, '0', '0', NULL, '0', NULL),
('1086', '479', 0, '', '4', '0', '0', 'LS0490', '2', '53', '-224.784', '-2872.34', '39.0851', '256.756', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '1.48365', 0, '0', '0', NULL, '0', NULL),
('1087', '459', 0, '', '4', '0', '0', 'LS0491', '86', '14', '34.9624', '-2632.87', '40.4547', '271.344', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.414449', 0, '0', '0', NULL, '0', NULL),
('1088', '419', 0, '', '4', '0', '0', 'LS0492', '68', '33', '35.5461', '-2636.62', '40.2134', '91.5567', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.02085', 0, '0', '0', NULL, '0', NULL),
('1089', '609', 0, '', '4', '0', '0', 'LS0493', '38', '72', '38.0308', '-2660.37', '40.6055', '178.209', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.053598', 0, '0', '0', NULL, '0', NULL),
('1090', '431', 0, '', '4', '0', '0', 'LS0494', '83', '18', '7.819', '-2609.11', '40.1019', '15.3669', '0', '0', '100', '1000', '0', '5300', '0', NULL, NULL, '0', 0, '0.048704', 0, '0', '0', NULL, '0', NULL),
('1091', '437', 0, '', '4', '0', '0', 'LS0495', '23', '106', '0.517882', '-2585.66', '39.5612', '20.6413', '0', '0', '100', '1000', '0', '7000', '0', NULL, NULL, '0', 0, '0.035584', 0, '0', '0', NULL, '0', NULL),
('1092', '442', 0, '', '4', '0', '0', 'LS0496', '23', '108', '-12.04', '-2512.71', '36.4864', '301.71', '0', '0', '100', '1000', '0', '2500', '0', NULL, NULL, '0', 0, '0.117086', 0, '0', '0', NULL, '0', NULL),
('1093', '448', 0, '', '4', '0', '0', 'LS0497', '70', '31', '-39.2717', '-2499.25', '36.1858', '33.2269', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.014832', 0, '0', '0', NULL, '0', NULL),
('1094', '456', 0, '', '4', '0', '0', 'LS0498', '75', '51', '-15.7875', '-2506.9', '36.8282', '303.036', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '0.032503', 0, '0', '0', NULL, '0', NULL),
('1095', '414', 0, '', '4', '0', '0', 'LS0499', '62', '109', '-33.6604', '-2495.06', '36.7422', '34.328', '0', '0', '100', '1000', '0', '2800', '0', NULL, NULL, '0', 0, '0.021483', 0, '0', '0', NULL, '0', NULL),
('1096', '423', 0, '', '4', '0', '0', 'LS0500', '106', '1', '-34.8648', '-2511.86', '36.3437', '214.8', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '0.034841', 0, '0', '0', NULL, '0', NULL),
('1097', '466', 0, '', '4', '0', '0', 'LS0501', '54', '105', '-19.7161', '-2522.93', '36.392', '30.667', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.034194', 0, '0', '0', NULL, '0', NULL),
('1098', '585', 0, '', '4', '0', '0', 'LS0502', '121', '233', '1190.15', '-1345.86', '13.0669', '359.666', '0', '0', '100', '1000', '0', '3000', '1', NULL, NULL, '0', 0, '10.6834', 0, '0', '0', NULL, '0', NULL),
('1101', '466', 0, '', '4', '0', '0', 'LS0503', '42', '115', '1363.28', '-1280.84', '13.2033', '358.364', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '4.32553', 0, '0', '0', NULL, '0', NULL),
('1102', '604', 0, '', '4', '0', '0', 'LS0504', '59', '88', '1512.71', '-1430.74', '13.5469', '266.242', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '0.486882', 0, '0', '0', NULL, '0', NULL),
('1103', '605', 0, '', '4', '0', '0', 'LS0505', '55', '8', '1849.92', '735.13', '11.225', '90.9228', '0', '0', '100', '1000', '0', '2000', '0', NULL, NULL, '0', 0, '2.80305', 0, '0', '0', NULL, '0', NULL),
('1104', '580', 0, '', '4', '0', '0', 'LS0506', '26', '118', '2086.66', '767.72', '11.0353', '356.342', '0', '0', '100', '1000', '0', '6500', '0', NULL, NULL, '0', 0, '0.264716', 0, '0', '0', NULL, '0', NULL),
('1105', '568', 0, '', '4', '0', '0', 'LS0507', '124', '52', '2478.22', '606.09', '10.4381', '270.703', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.49759', 0, '0', '0', NULL, '0', NULL),
('1107', '557', 0, '', '4', '0', '0', 'LS0508', '3', '33', '2459.78', '626.353', '11.2254', '61.389', '0', '0', '100', '1000', '0', '4500', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('1108', '556', 0, '', '4', '0', '0', 'LS0509', '31', '15', '2481.23', '628.121', '11.95', '283.573', '0', '0', '100', '1000', '0', '4500', '0', NULL, NULL, '0', 0, '0', 0, '0', '0', NULL, '0', NULL),
('1109', '550', 0, '', '4', '0', '0', 'LS0510', '50', '34', '2521.86', '1289.36', '10.5654', '179.679', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.988615', 0, '0', '0', NULL, '0', NULL),
('1110', '518', 0, '', '4', '0', '0', 'LS0511', '121', '84', '2282.54', '1388.69', '10.4911', '178.73', '0', '0', '100', '1000', '0', '2200', '0', NULL, NULL, '0', 0, '0.297445', 0, '0', '0', NULL, '0', NULL),
('1111', '444', 0, '', '4', '0', '0', 'LS0512', '122', '20', '1895.94', '1281.17', '11.1916', '91.1508', '0', '0', '100', '1000', '0', '4700', '0', NULL, NULL, '0', 0, '0.715998', 0, '0', '0', NULL, '0', NULL),
('1112', '438', 0, '', '4', '0', '0', 'LS0513', '6', '6', '2037.76', '1543.51', '10.7445', '1.16602', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.389367', 0, '0', '0', NULL, '0', NULL),
('1113', '438', 0, '', '4', '0', '0', 'LS0514', '6', '6', '1940.75', '2089.14', '10.8229', '179.354', '0', '0', '100', '1000', '0', '2300', '0', NULL, NULL, '0', 0, '0.794922', 0, '0', '0', NULL, '0', NULL),
('1114', '404', 0, '', '4', '0', '0', 'LS0515', '8', '6', '1701.95', '2202.37', '10.5531', '4.70976', '0', '0', '100', '1000', '0', '2100', '0', NULL, NULL, '0', 0, '0.2811', 0, '0', '0', NULL, '0', NULL),
('1115', '440', 0, '', '4', '0', '0', 'LS0516', '38', '27', '1602.07', '1831.7', '10.9423', '1.36838', '0', '0', '100', '1000', '0', '2400', '0', NULL, NULL, '0', 0, '0.295609', 0, '0', '0', NULL, '0', NULL),
('1116', '483', 0, '', '4', '0', '0', 'LS0517', '100', '82', '1721.86', '1491.9', '10.7309', '164.236', '0', '0', '100', '1000', '0', '3000', '0', NULL, NULL, '0', 0, '5.8845', 0, '0', '0', NULL, '0', NULL),
('1118', '429', 0, '', '4', '0', '0', 'LS0518', '27', '29', '887.207', '-1271.95', '14.366', '269.984', '0', '0', '100', '1000', '0', '10000000', '1', NULL, NULL, '0', 0, '5.20444', 0, '0', '0', NULL, '0', NULL),
('1119', '522', 0, '', '4', '0', '0', 'LS0519', '52', '96', '-1753.66', '-622.643', '16.9133', '193.01', '0', '0', '100', '1000', '0', '2700', '0', NULL, NULL, '0', 0, '35.1601', 0, '0', '0', NULL, '0', NULL),
('1121', '522', 0, '', '4', '0', '0', 'LS0520', '101', '116', '87.0307', '-1550.88', '5.64324', '171.173', '0', '0', '100', '1000', '0', '25000000', '1', NULL, NULL, '0', 0, '14.576', 0, '0', '0', NULL, '0', NULL),
('1122', '522', 0, '', '4', '0', '0', 'LS0521', '76', '44', '85.1043', '-1550.58', '5.57232', '179.455', '0', '0', '100', '1000', '0', '25000000', '1', NULL, NULL, '0', 0, '0.018137', 0, '0', '0', NULL, '0', NULL);
