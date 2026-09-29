#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
SEM="$(cd "$HERE/../01_Sem_Release" && pwd)"
SRC="$HERE/02_Geradores/02_Todas_Instancias/gerar_todas_instancias_release_facil.c"
BIN="$HERE/02_Geradores/02_Todas_Instancias/gerar_todas_instancias_release_facil"

gcc -std=gnu89 -O2 -w "$SRC" -o "$BIN" -lm
"$BIN" \
  "$SEM/03_Instancias_Geradas" \
  "$SEM/04_Resultados_Referencia" \
  "$HERE/03_Instancias_Geradas"
