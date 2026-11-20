@echo off
if not exist bin\inventory_app.exe (
    echo [INFO] Application not built yet. Building first...
    call build.bat
)

if exist bin\inventory_app.exe (
    cls
    bin\inventory_app.exe
)
