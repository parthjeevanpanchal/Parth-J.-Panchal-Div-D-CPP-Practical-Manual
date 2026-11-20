@echo off
echo ============================================================
echo   Starting Inventory Manager Web Application (OOP & STL)
echo ============================================================
echo.
echo Connecting to CSV Data Layer (data/inventory.csv)...
echo Starting server on http://localhost:3000
echo.

start http://localhost:3000
node server.js
