#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
SEM="$(cd "$HERE/../01_Sem_Release" && pwd)"
SRC="$HERE/02_Geradores/02_Todas_Instancias/gerar_todas_instancias_release_dificil.c"
BIN="$HERE/02_Geradores/02_Todas_Instancias/gerar_todas_instancias_release_dificil"
RATIO="${1:-0.20}"
EVERY="${2:-4}"

gcc -std=gnu89 -O2 -w "$SRC" -o "$BIN" -lm
"$BIN" \
  "$SEM/03_Instancias_Geradas" \
  "$SEM/04_Resultados_Referencia" \
  "$HERE/03_Instancias_Geradas" \
  "$RATIO" "$EVERY"
