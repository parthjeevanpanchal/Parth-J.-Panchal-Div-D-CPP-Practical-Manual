@echo off
echo ============================================================
echo   Building Inventory Manager (MinGW C++14)
echo ============================================================

if not exist bin mkdir bin

g++ -std=c++14 -Wall -Wextra -Iinclude ^
    src/Item.cpp ^
    src/PerishableItem.cpp ^
    src/ElectronicsItem.cpp ^
    src/Transaction.cpp ^
    src/FileStorage.cpp ^
    src/InventoryManager.cpp ^
    src/ConsoleUI.cpp ^
    src/main.cpp ^
    -o bin/inventory_app.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Build succeeded! Executable created at bin/inventory_app.exe
) else (
    echo.
    echo [ERROR] Build failed! Check compiler errors above.
)
