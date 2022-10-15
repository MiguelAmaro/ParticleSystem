@echo off

@ECHO OFF
SETLOCAL EnableExtensions DisableDelayedExpansion
for /F %%a in ('echo prompt $E ^| cmd') do ( set "ESC=%%a")
SETLOCAL EnableDelayedExpansion

SET WILDCRARDS=*.h *.cpp *.inl *.c

pushd src
echo !ESC![31m
echo TODOS FOUND:
findstr -s -n -i -l "TODO" %WILDCRARDS%
echo -------
echo !ESC![0m
echo !ESC![32m
echo NOTES FOUND:
findstr -s -n -i -l "NOTE" %WILDCRARDS%
echo -------
echo !ESC![0m

popd

                                                                        