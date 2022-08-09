@echo off

if not exist build mkdir build
set OUTPUT=%1

rem PROJECT INFO / SOURCE FILES
rem ============================================================
set PROJECT_DIR=%cd%
set PROJECT_NAME=main
set SOURCES=^
%PROJECT_DIR%\src\%PROJECT_NAME%.c ^
%PROJECT_DIR%\thirdparty\cimgui\cimgui.cpp ^
%PROJECT_DIR%\thirdparty\cimgui\imgui\*.cpp ^
%PROJECT_DIR%\thirdparty\cimgui\imgui\backends\imgui_impl_dx11.cpp ^
%PROJECT_DIR%\thirdparty\cimgui\imgui\backends\imgui_impl_win32.cpp

rem COMPILER(MSVC) OPTIONS
rem ============================================================
set MSVC_WARNINGS=^ -wd4700 -wd4820 -wd4668 -wd4577 -wd5045 -wd4505 ^
-wd4365 -wd4305 -wd4201 -wd4100 -wd4191 -wd5246  -wd4061 -wd4514 -wd5219
set MSVC_FLAGS= %MSVC_WARNINGS% -nologo -Wall -Zi -Od -std:c11 ^
-DIMGUI_IMPL_API="extern \"C\"" ^
-DCIMGUI_USE_WIN32 ^
-DCIMGUI_USE_DX11 ^
-DCIMGUI_FREETYPE=1 ^
-I%PROJECT_DIR%\thirdparty\cimgui\ ^
-I%PROJECT_DIR%\thirdparty\cimgui\imgui\ ^
-I%PROJECT_DIR%\thirdparty\cimgui\generator\output\



rem START BUILD
rem ============================================================
set path=%PROJECT_DIR%\build;%path%
if "%OUTPUT%" equ "exe" (goto :COMPILE_EXE)
goto :PRINT_USAGE
goto :eof


:PRINT_USAGE
echo Usage: build [option]
echo Options:
echo         exe  :Builds main app platform specific executable.
echo         dll  :Builds hotloadable library.
echo         test :Builds app test suite.
goto :eof

:COMPILE_EXE
taskkill /f /im %PROJECT_NAME%.exe
pushd build
cl %MSVC_FLAGS% %SOURCES% /link -subsystem:windows /MAPINFO:EXPORTS
popd
goto :eof
