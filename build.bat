@echo off

if not exist .\build\win32 mkdir .\build\win32
cmake -A Win32 -S . -B .\build\win32
cmake --build .\build\win32 --config %1%
