@echo off
setlocal
 
rem === Configuration ===
set RAYLIB_PATH=D:\Programming\Other\raylib\raylib
set COMPILER_PATH=D:\Programming\Other\raylib\w64devkit\bin
set PATH=%COMPILER_PATH%;%PATH%
set CC=gcc
set CFLAGS=-s -static -O3 -std=c99 -Wall -Wshadow -Wunused-parameter -I%RAYLIB_PATH%\src -Iexternal -DPLATFORM_DESKTOP -L.
set LDFLAGS=-lraylib -lopengl32 -lgdi32 -lwinmm -lshcore
set SOURCE_FILES=src\main.c src\component\component_card.c src\component\component_hud.c src\sound\sound.c src\pokajan_core\cards.c src\pokajan_core\pokajan.c src\scene\scene_title.c src\scene\scene_game.c src\scene\overlay_card_instructions.c src\scene\scene_manager.c src\utils\text.c src\utils\input.c
 
if /I "%ACTION%"=="run" goto :run
 
echo ^> Clean latest build
echo -------------------------
if exist "pokajan.exe" del /F "pokajan.exe"
 
echo.
echo ^> Compile program
echo -----------------------
%CC% --version
%CC% -o pokajan.exe %SOURCE_FILES% %CFLAGS% %LDFLAGS%
if errorlevel 1 (
    echo.
    echo ^> Build failed, see errors above.
    exit /b 1
)
 
if /I "%ACTION%"=="build" goto :eof
 
:run
echo.
echo ^> Execute program
echo -----------------------
if exist "pokajan.exe" (
    "pokajan.exe"
) else (
    echo pokajan.exe not found - build it first.
)
 
endlocal