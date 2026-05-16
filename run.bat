@echo off
REM Run script for Transaction Processing System - Windows
REM Double-click this file to run the program

echo ╔════════════════════════════════════════════════════════╗
echo ║    Transaction Processing System - Run Script         ║
echo ║                    Windows                            ║
echo ╚════════════════════════════════════════════════════════╝
echo.

REM Check if executable exists
if not exist transaction_system.exe (
    echo [ERROR] transaction_system.exe not found!
    echo.
    echo Please compile first using: compile.bat
    echo.
    pause
    exit /b 1
)

REM Run the program
echo [INFO] Starting Transaction Processing System...
echo.
transaction_system.exe

echo.
echo [INFO] Program terminated.
pause
