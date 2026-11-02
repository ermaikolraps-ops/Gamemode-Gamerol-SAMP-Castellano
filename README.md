# Gamemode Gamerol SA:MP (Castellano)

Modo de juego de rol, escrito en castellano, para SA:MP y open.mp.
Se puede montar en un PC o en un VPS. No necesita la web de Gamerol.

## El servidor sigue en marcha

El modo sigue activo en [www.gamerol.online](https://www.gamerol.online).

Si quieres probarlo ahí, conéctate con el cliente de SA:MP o con open.mp:

`51.91.157.152:7777`

Ese es el servidor nuestro, el que está en la web. Lo de este repositorio es otra cosa: el modo para instalarlo tú, en tu máquina, sin nuestra web y sin nuestros jugadores.

## Qué trae

- Cuentas, trabajos, facciones, coches, casas, negocios, policía, médicos, hambre y el mapa.
- Las casas, los negocios y los vehículos. Van sin dueño: las casas y los negocios están libres, y los coches no están a nombre de nadie.
- El panel de administración de dentro del juego. El comando es `/panel`.

## Qué no trae

- La web, el Panel del Jugador y el panel del VPS.
- `/duda`, el teletipo y los anuncios automáticos. Esas partes viven en la web y aquí no están.
- Ninguna cuenta, nombre, contraseña, IP, dinero ni objeto de los jugadores del servidor oficial.
- Contraseñas, llaves ni el acceso al servidor nuestro.

## Qué hace falta

- El servidor [open.mp](https://open.mp/). El ejecutable no va en este repositorio.
- MariaDB o MySQL.
- Tres plugins, con el nombre que ya trae `config.json.ejemplo`: `mysql`, `streamer` y `samp_bcrypt`.
- El include de MySQL de esta copia es el R41-4. El plugin tiene que ser esa rama.
- `sscanf` lo trae el propio open.mp.

Los `.so` no van incluidos: son de otros autores. Las cabeceras para compilar sí, en `include/`.

## Cómo ponerlo en marcha

1. Descarga open.mp y copia dentro de su carpeta `gamemodes/`, `plugins/`, `config.json` y `mysql.ini`.
2. Copia `config.json.ejemplo` a `config.json`. Cambia la contraseña de RCON. Por defecto pone `cambia_esto` y el RCON está encendido para que puedas hacerte fundador. Cámbiala antes de abrir el puerto a internet.
3. Crea una base de datos vacía. Copia `mysql.ini.ejemplo` a `mysql.ini` y pon el servidor, el usuario, la contraseña y el nombre de la base.
4. Importa el mundo:

```bash
mysql -u gamerol -p gamerol < esquema.sql
```

5. Coloca `mysql.so`, `streamer.so` y `samp_bcrypt.so` en `plugins/`.
6. Compila el modo (más abajo) y arranca open.mp.
7. Entra al juego, regístrate y hazte fundador:
   - `/rcon login cambia_esto` (o la contraseña que hayas puesto)
   - `/fundador`
   - `/admin` y `/panel` enseñan los comandos

A partir de ahí las casas y los negocios se compran dentro del juego. Nadie llega ya con una casa a su nombre.

## Compilar

El código del repositorio está en UTF-8, para poder leerlo. El cliente del juego usa latin1, y `compilar.sh` hace esa conversión en una carpeta temporal.

En Linux, con `iconv` y el `pawncc` de open.mp:

```bash
PAWNCC=/ruta/al/pawncc bash compilar.sh
```

El resultado es `gamemodes/malagacity.amx`. `config.json.ejemplo` ya apunta a ese modo.

## Licencia

El modo se publica con la licencia MIT. Ver `LICENCIA`.

Los includes de terceros que van en `include/` siguen con la licencia de su autor. Streamer es de Incognito, licencia Apache 2.0.
