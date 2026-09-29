#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/02_Geradores/02_Todas_Instancias/gerar_todas_instancias_sem_release.c"
BIN="$HERE/02_Geradores/02_Todas_Instancias/gerar_todas_instancias_sem_release"
OUT="$HERE/03_Instancias_Geradas"
gcc -O2 -std=gnu89 "$SRC" -o "$BIN" -lm
rm -f "$OUT/01_Estruturadas"/dados_trab_* "$OUT/02_Nao_Estruturadas"/dados_trab_* "$OUT/manifesto_instancias.csv" 2>/dev/null || true
"$BIN" "$OUT"
echo "Total gerado: $(find "$OUT/01_Estruturadas" "$OUT/02_Nao_Estruturadas" -maxdepth 1 -type f -name 'dados_trab_*' | wc -l)"
