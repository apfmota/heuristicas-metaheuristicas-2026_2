#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="$HERE/01_Codigo/02_Revisado_Artigo/tabu_revisado_com_release"
INST="$HERE/03_Instancias_Geradas"
OUT="$HERE/04_Resultados/Revisada"
TIME_EXTRA="${1:-}"
if [ ! -x "$BIN" ]; then "$HERE/COMPILAR_CODIGOS.sh"; fi
rm -rf "$OUT"; mkdir -p "$OUT/Estruturadas" "$OUT/Nao_Estruturadas"
rodar(){ local idir="$1"; local odir="$2"; cd "$odir"; if [ -n "$TIME_EXTRA" ]; then "$BIN" "$idir" "$TIME_EXTRA"; else "$BIN" "$idir"; fi; }
rodar "$INST/01_Estruturadas" "$OUT/Estruturadas"
rodar "$INST/02_Nao_Estruturadas" "$OUT/Nao_Estruturadas"
