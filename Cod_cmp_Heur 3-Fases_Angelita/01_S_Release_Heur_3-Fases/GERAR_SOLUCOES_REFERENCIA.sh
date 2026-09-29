#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="$HERE/01_Codigo/02_Revisado_Artigo/tabu_revisado_sem_release"
INST="$HERE/03_Instancias_Geradas"
OUT="$HERE/04_Resultados_Referencia"
TIME_EXTRA="${1:-}"
if [ ! -x "$BIN" ]; then "$HERE/COMPILAR_CODIGOS.sh"; fi
if [ ! -d "$INST/01_Estruturadas" ] || [ ! -d "$INST/02_Nao_Estruturadas" ]; then echo "Gere primeiro as instancias sem release."; exit 1; fi
rm -rf "$OUT/Estruturadas" "$OUT/Nao_Estruturadas"
mkdir -p "$OUT/Estruturadas" "$OUT/Nao_Estruturadas"
rodar(){ local idir="$1"; local odir="$2"; mkdir -p "$odir"; cd "$odir"; if [ -n "$TIME_EXTRA" ]; then "$BIN" "$idir" "$TIME_EXTRA"; else "$BIN" "$idir"; fi; }
echo "[1/2] Solucoes de referencia: estruturadas"
rodar "$INST/01_Estruturadas" "$OUT/Estruturadas"
echo "[2/2] Solucoes de referencia: nao estruturadas"
rodar "$INST/02_Nao_Estruturadas" "$OUT/Nao_Estruturadas"
echo "Solucoes de referencia salvas em: $OUT"
