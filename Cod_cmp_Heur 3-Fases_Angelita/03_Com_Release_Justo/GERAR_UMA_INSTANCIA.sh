#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
SEM="$(cd "$HERE/../01_Sem_Release" && pwd)"
SRC="$HERE/02_Geradores/01_Uma_Instancia/gerar_uma_instancia_release_dificil.c"
BIN="$HERE/02_Geradores/01_Uma_Instancia/gerar_uma_instancia_release_dificil"

if [ "$#" -lt 4 ] || [ "$#" -gt 6 ]; then
  echo "Uso: ./GERAR_UMA_INSTANCIA.sh <struc|nonstruc> <maquinas> <tarefas> <numero> [delay_ratio] [every]"
  echo "Exemplo: ./GERAR_UMA_INSTANCIA.sh struc 2 20 1"
  exit 1
fi

TIPO="$1"; M="$2"; N="$3"; I=$(printf '%02d' "$4"); RATIO="${5:-0.20}"; EVERY="${6:-4}"
if [ "$TIPO" = "struc" ]; then SUBBASE="01_Estruturadas"; SUBSOL="Estruturadas"; else SUBBASE="02_Nao_Estruturadas"; SUBSOL="Nao_Estruturadas"; fi
BASE="$SEM/03_Instancias_Geradas/$SUBBASE/dados_trab_${M}_${N}_${TIPO}_${I}"
SOL=$(find "$SEM/04_Resultados_Referencia/$SUBSOL" -type f -name "solucaoTabu.dados_trab_${M}_${N}_${TIPO}_${I}" | head -1 || true)
OUT="$HERE/03_Instancias_Geradas/$SUBBASE/dados_trab_${M}_${N}_${TIPO}_release_dificil_${I}"

if [ ! -f "$BASE" ]; then echo "Instancia-base nao encontrada: $BASE"; exit 1; fi
if [ -z "$SOL" ]; then echo "Solucao de referencia nao encontrada. Rode primeiro ../01_Sem_Release/GERAR_SOLUCOES_REFERENCIA.sh"; exit 1; fi

mkdir -p "$(dirname "$OUT")"
gcc -std=gnu89 -O2 -w "$SRC" -o "$BIN" -lm
"$BIN" "$BASE" "$SOL" "$OUT" "$RATIO" "$EVERY"
