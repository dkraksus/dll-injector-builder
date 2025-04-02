@echo off
echo [*] Building injector.exe using MinGW...
g++ -static -o injector.exe dll_inject_launcher.cpp
if exist injector.exe (
    echo [+] Build complete! injector.exe generated.
) else (
    echo [!] Build failed. Check if g++ (MinGW) is installed and in PATH.
)
pause