@echo off

if not exist build mkdir build
set OUTPUT=%1

rem PROJECT INFO / SOURCE FILES
rem ============================================================
set PROJECT_DIR=%cd%
set PROJECT_NAME=main
set SOURCES=%PROJECT_DIR%\src\%PROJECT_NAME%.c

rem COMPILER(MSVC) OPTIONS
rem ============================================================
set MSVC_WARNINGS=^ -wd4700 -wd4820 -wd4668 -wd4577 -wd5045 -wd4505 ^
-wd4365 -wd4305 -wd4201 -wd4100 -wd4191 -wd5246  -wd4061 -wd4514 -wd5219 -wd4189 -wd4101 -wd4013 -wd4324
set MSVC_FLAGS= %MSVC_WARNINGS% -nologo -Wall -Zi -Od -std:c11 ^
-I%PROJECT_DIR%\thirdparty\cimgui\ -I%PROJECT_DIR%\thirdparty\cimgui\imgui\ -I%PROJECT_DIR%\thirdparty\cimgui\generator\output\ ^
-I%PROJECT_DIR%\thirdparty\cimgui\imgui\backends\ ^
-I%PROJECT_DIR%\thirdparty\stb\

rem START BUILD
rem ============================================================
set path=%PROJECT_DIR%\build;%path%
if "%OUTPUT%" equ "exe" (goto :COMPILE_EXE)
echo "%OUTPUT%"
if "%OUTPUT%" equ "imgui" (goto :COMPILE_IMGUI_LIB)
goto :PRINT_USAGE
goto :eof

:COMPILE_EXE
pushd build
taskkill /f /im %PROJECT_NAME%.exe
cl %MSVC_FLAGS% %SOURCES% /link -subsystem:windows /MAPINFO:EXPORTS cimgui.lib
popd
goto :eof

:COMPILE_IMGUI_LIB
set IMGUI_FLAGS=^
-DIMGUI_IMPL_API="extern \"C\"" -DCIMGUI_USE_WIN32 -DCIMGUI_USE_DX11 -DCIMGUI_FREETYPE=1
set IMGUI_SRC=^
%PROJECT_DIR%\thirdparty\cimgui\cimgui.cpp ^
%PROJECT_DIR%\thirdparty\cimgui\imgui\*.cpp ^
%PROJECT_DIR%\thirdparty\cimgui\imgui\backends\imgui_impl_dx11.cpp ^
%PROJECT_DIR%\thirdparty\cimgui\imgui\backends\imgui_impl_win32.cpp
pushd build
rem /link -subsystem:windows /MAPINFO:EXPORTS
cl /c %MSVC_FLAGS% %IMGUI_FLAGS% %IMGUI_SRC%
lib *.obj
popd
goto :eof

:PRINT_USAGE
echo Usage: build [option]
echo Options:
echo         exe  :Builds main app platform specific executable.
echo         dll  :Builds hotloadable library.
echo         test :Builds app test suite.
goto :eof
