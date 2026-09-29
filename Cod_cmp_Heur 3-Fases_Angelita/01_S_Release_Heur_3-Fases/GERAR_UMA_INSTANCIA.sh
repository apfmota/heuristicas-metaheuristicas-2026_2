#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
if [ "$#" -ne 4 ]; then
  echo "Uso: ./GERAR_UMA_INSTANCIA.sh <struc|nonstruc> <maquinas> <tarefas> <numero>"
  echo "Exemplo: ./GERAR_UMA_INSTANCIA.sh struc 2 20 1"
  exit 1
fi
TIPO="$1"; M="$2"; N="$3"; I="$4"
SRC="$HERE/02_Geradores/01_Uma_Instancia/gerar_uma_instancia_sem_release.c"
BIN="$HERE/02_Geradores/01_Uma_Instancia/gerar_uma_instancia_sem_release"
if [ "$TIPO" = "struc" ]; then OUT="$HERE/03_Instancias_Geradas/01_Estruturadas"; else OUT="$HERE/03_Instancias_Geradas/02_Nao_Estruturadas"; fi
gcc -O2 -std=gnu89 "$SRC" -o "$BIN" -lm
"$BIN" "$TIPO" "$M" "$N" "$I" "$OUT"
