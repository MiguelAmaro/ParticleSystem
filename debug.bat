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
F:\\Dev_Tools\\RenderDoc\\qrenderdoc.exe build\%EXE%
rem copy build\%EXE% %cd%
rem del %EXE%
goto eof

:eof
pause



