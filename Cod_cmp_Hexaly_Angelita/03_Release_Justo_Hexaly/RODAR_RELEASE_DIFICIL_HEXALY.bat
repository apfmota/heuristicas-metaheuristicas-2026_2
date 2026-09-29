@echo off
setlocal EnableExtensions EnableDelayedExpansion

REM ================================================================
REM HEXALY - RELEASE DIFICIL - 500 s POR INSTANCIA
REM Coloque este .BAT na pasta "release_dificil" e execute por duplo clique
REM ================================================================

pushd "%~dp0"
set "ROOT=%CD%"
set "INST_ROOT=%ROOT%\03_Instancias_Geradas_release_dificil"
set "MODEL=%ROOT%\Codigo_com_release_lote_release_dificil.hxm"
set "RESULT_ROOT=%ROOT%\Resultados_Hexaly_release_dificil"
set "OUT_E=%RESULT_ROOT%\Estruturadas"
set "OUT_NE=%RESULT_ROOT%\Nao_Estruturadas"

if not exist "%INST_ROOT%" (
    echo ERRO: pasta de instancias nao encontrada:
    echo %INST_ROOT%
    pause
    exit /b 1
)

if not exist "%MODEL%" (
    echo ERRO: modelo nao encontrado:
    echo %MODEL%
    pause
    exit /b 1
)

where hexaly >nul 2>&1
if errorlevel 1 (
    echo ERRO: o comando "hexaly" nao foi encontrado no PATH.
    echo Abra o mesmo Prompt/ambiente usado para executar o Hexaly nos lotes anteriores.
    pause
    exit /b 1
)

if not exist "%RESULT_ROOT%" mkdir "%RESULT_ROOT%"
if not exist "%OUT_E%" mkdir "%OUT_E%"
if not exist "%OUT_NE%" mkdir "%OUT_NE%"

set /a FOUND_E=0, FOUND_NE=0, OK=0, ERR=0, SKIP=0

echo.
echo ================================================================
echo RELEASE DIFICIL - HEXALY
echo Instancias: %INST_ROOT%
echo Modelo:     %MODEL%
echo Resultados: %RESULT_ROOT%
echo Limite:     500 segundos por instancia
echo Seed:       0
echo ================================================================
echo.

REM ---------------------------------------------------------------
REM 1) ESTRUTURADAS
REM ---------------------------------------------------------------
for /R "%INST_ROOT%" %%F in (dados_trab_*) do (
    echo(%%~nxF | findstr /I /C:"nonstruc" >nul
    if errorlevel 1 (
        echo(%%~nxF | findstr /I /C:"struc" >nul
        if not errorlevel 1 (
            set /a FOUND_E+=1
            call :RUN_ONE "%%~fF" "E"
        )
    )
)

REM ---------------------------------------------------------------
REM 2) NAO ESTRUTURADAS
REM ---------------------------------------------------------------
for /R "%INST_ROOT%" %%F in (dados_trab_*) do (
    echo(%%~nxF | findstr /I /C:"nonstruc" >nul
    if not errorlevel 1 (
        set /a FOUND_NE+=1
        call :RUN_ONE "%%~fF" "NE"
    )
)

echo.
echo ================================================================
echo EXECUCAO FINALIZADA
echo Estruturadas encontradas:     !FOUND_E!
echo Nao estruturadas encontradas: !FOUND_NE!
echo Executadas com sucesso:       !OK!
echo Com erro:                     !ERR!
echo Ja existentes - ignoradas:    !SKIP!
echo ================================================================
echo.
echo Resultados em:
echo %RESULT_ROOT%
echo.
pause
popd
exit /b 0

:RUN_ONE
set "SRC=%~1"
set "TIPO=%~2"
set "BASE=%~n1"
set "CLEAN=!BASE:dados_trab_=!"

if /I "%TIPO%"=="NE" (
    set "OUTFILE=%OUT_NE%\terminal_Hexaly_!CLEAN!.txt"
    set "LABEL=NAO ESTRUTURADA"
) else (
    set "OUTFILE=%OUT_E%\terminal_Hexaly_!CLEAN!.txt"
    set "LABEL=ESTRUTURADA"
)

REM Protecao: nao sobrescreve resultado ja existente.
if exist "!OUTFILE!" (
    echo [PULA] !LABEL!: %~nx1 - resultado ja existe.
    set /a SKIP+=1
    exit /b 0
)

echo [RODANDO] !LABEL!: %~nx1
pushd "%~dp1"
hexaly "%MODEL%" instanceFile="%~nx1" hxTimeLimit=500 hxSeed=0 hxVerbosity=1 > "!OUTFILE!" 2>&1
set "RC=!ERRORLEVEL!"
popd

if "!RC!"=="0" (
    echo [OK] %~nx1
    set /a OK+=1
) else (
    echo [ERRO !RC!] %~nx1
    set /a ERR+=1
)
exit /b 0
