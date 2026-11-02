#!/bin/bash
# Compila la edicion publica. El repositorio esta en UTF-8 para poder leerlo.
# El juego necesita latin1, asi que la conversion se hace en una carpeta temporal.
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")" && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cp "$RAIZ/gamemodes/malagacity.pwn" "$TMP/"
mkdir -p "$TMP/modulos"
for f in "$RAIZ"/gamemodes/modulos/*.inc; do
  iconv -f UTF-8 -t LATIN1 "$f" > "$TMP/modulos/$(basename "$f")"
done
iconv -f UTF-8 -t LATIN1 "$RAIZ/gamemodes/malagacity.pwn" > "$TMP/malagacity.pwn"
PAWNCC="${PAWNCC:-pawncc}"
cd "$TMP"
"$PAWNCC" malagacity.pwn -i"$RAIZ/include" -d3 -o"$RAIZ/gamemodes/malagacity.amx"
echo "Listo: gamemodes/malagacity.amx"
