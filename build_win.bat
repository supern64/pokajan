@echo off
setlocal
 
rem === Configuration ===
set RAYLIB_PATH=D:\Programming\Other\raylib\raylib
set COMPILER_PATH=D:\Programming\Other\raylib\w64devkit\bin
set PATH=%COMPILER_PATH%;%PATH%
set CC=gcc
set CFLAGS=-s -static -O3 -std=c99 -Wall -Wshadow -Wunused-parameter -I%RAYLIB_PATH%\src -Iexternal -DPLATFORM_DESKTOP -L.
set LDFLAGS=-lraylib -lopengl32 -lgdi32 -lwinmm -lshcore
set POKAJAN_SOURCE_FILES=src\main.c src\component\component_card.c src\component\component_hud.c src\sound\sound.c src\pokajan_core\cards.c src\pokajan_core\pokajan.c src\scene\scene_title.c src\scene\scene_game.c src\scene\overlay_card_instructions.c src\scene\scene_manager.c src\utils\text.c src\utils\input.c
set PLAYER_UNIT_SIM_SOURCE_FILES=src\player_unit_sim.c src\component\component_card.c

set ACTION=%~1

echo ^>^>^> Compiler
%CC% --version

if /I "%ACTION%"=="unit" goto :unit

echo ^>^>^> Building Main Screen

echo ^> Clean latest build
if exist "pokajan.exe" del /F "pokajan.exe"
 
echo ^> Compile program
echo %CC% -o pokajan.exe %POKAJAN_SOURCE_FILES% %CFLAGS% %LDFLAGS%
%CC% -o pokajan.exe %POKAJAN_SOURCE_FILES% %CFLAGS% %LDFLAGS%
if errorlevel 1 (
    echo ERR^> Build failed, see errors above.
    exit /b 1
)

echo ^> Execute program
"pokajan.exe"

goto :eof

:unit
echo ^>^>^> Building Player Unit Simulator

echo ^> Clean latest build
if exist "player_unit_sim.exe" del /F "player_unit_sim.exe"
 
echo ^> Compile program
echo %CC% -o pokajan.exe %POKAJAN_SOURCE_FILES% %CFLAGS% %LDFLAGS%
%CC% -o player_unit_sim.exe %PLAYER_UNIT_SIM_SOURCE_FILES% %CFLAGS% %LDFLAGS%
if errorlevel 1 (
    echo ERR^> Build failed, see errors above.
    exit /b 1
)

echo ^> Execute program
"player_unit_sim.exe"

endlocal