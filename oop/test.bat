@echo off
echo ============================================================
echo   Building and Running Automated Test Suite and Demo
echo ============================================================

if not exist bin mkdir bin

g++ -std=c++14 -Wall -Wextra -Iinclude ^
    src/Item.cpp ^
    src/PerishableItem.cpp ^
    src/ElectronicsItem.cpp ^
    src/Transaction.cpp ^
    src/FileStorage.cpp ^
    src/InventoryManager.cpp ^
    tests/demo_runner.cpp ^
    -o bin/demo_runner.exe

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Compilation successful. Running test cases...
    echo.
    bin\demo_runner.exe
) else (
    echo [ERROR] Compilation failed.
)
