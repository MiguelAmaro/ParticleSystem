@echo off

set OPTION=%1

set DEBUG_FILE=debug.rdbg
set EXE=main.exe
rem  -g -q 

if "%OPTION%" equ "-rdbg" (goto :REMEDY)
if "%OPTION%" equ "-rdoc" (goto :RENDERDOC)
goto :REMEDY rem !!!DEFAULT PATH!!!

:REMEDY
call remedybg.exe .\debug\%DEBUG_FILE%
goto eof

:RENDERDOC
pushd build
F:\\Dev_Tools\\RenderDoc\\qrenderdoc.exe %EXE%
popd
goto eof

:eof
pause



