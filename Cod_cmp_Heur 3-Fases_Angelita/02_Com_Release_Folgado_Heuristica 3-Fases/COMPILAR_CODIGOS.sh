#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
gcc -O2 -std=gnu89 "$HERE/01_Codigo/02_Revisado_Artigo/tabu_revisado_com_release.c" -o "$HERE/01_Codigo/02_Revisado_Artigo/tabu_revisado_com_release" -lm
echo "Codigo revisado com release facil compilado."
