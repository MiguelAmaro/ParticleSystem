@echo off

if exist build\main.exe (start build\main.exe) else (echo "ERROR: main.exe" does not exist!!!)
