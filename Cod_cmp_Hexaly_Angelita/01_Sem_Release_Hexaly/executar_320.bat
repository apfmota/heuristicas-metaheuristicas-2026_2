@echo off
setlocal

mkdir Resultados_Hexaly 2>nul
mkdir Resultados_Hexaly\Estruturadas 2>nul
mkdir Resultados_Hexaly\Nao_Estruturadas 2>nul

echo ========================================
echo INICIANDO 320 INSTANCIAS - HEXALY
echo Limite: 500 segundos por instancia
echo ========================================

for %%F in ("03_Instancias_Geradas\01_Estruturadas\*") do (
    echo.
    echo Executando: %%~nxF
    hexaly Codigo_Hexaly_sem_release.hxm instanceFileName="%%F" hxTimeLimit=500 > "Resultados_Hexaly\Estruturadas\%%~nF.txt" 2>&1
)

for %%F in ("03_Instancias_Geradas\02_Nao_Estruturadas\*") do (
    echo.
    echo Executando: %%~nxF
    hexaly Codigo_Hexaly_sem_release.hxm instanceFileName="%%F" hxTimeLimit=500 > "Resultados_Hexaly\Nao_Estruturadas\%%~nF.txt" 2>&1
)

echo.
echo ========================================
echo TODAS AS 320 INSTANCIAS FORAM EXECUTADAS
echo ========================================

pause