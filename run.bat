@echo off

pushd build
if exist main.exe (start main.exe) else (echo "ERROR: main.exe" does not exist!!!)
popd
