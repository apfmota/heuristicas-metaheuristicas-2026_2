#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"

echo "============================================"
echo "1/4 - Gerando instancias SEM RELEASE"
echo "============================================"
"$HERE/01_Sem_Release/GERAR_TODAS_INSTANCIAS.sh"

echo "============================================"
echo "2/4 - Obtendo solucoes de referencia SEM RELEASE"
echo "============================================"
"$HERE/01_Sem_Release/GERAR_SOLUCOES_REFERENCIA.sh"

echo "============================================"
echo "3/4 - Gerando instancias COM RELEASE FACIL (gerador C)"
echo "============================================"
"$HERE/02_Com_Release_Facil/GERAR_TODAS_INSTANCIAS.sh"

echo "============================================"
echo "4/4 - Gerando instancias COM RELEASE DIFICIL (gerador C)"
echo "============================================"
"$HERE/03_Com_Release_Dificil/GERAR_TODAS_INSTANCIAS.sh"

gcc -std=gnu89 -O2 -w "$HERE/VALIDAR_TUDO.c" -o "$HERE/VALIDAR_TUDO"
"$HERE/VALIDAR_TUDO" "$HERE"
rm -f "$HERE/VALIDAR_TUDO"

echo "============================================"
echo "PROCESSO CONCLUIDO"
echo "Foram preparadas as tres familias de instancias."
echo "============================================"
