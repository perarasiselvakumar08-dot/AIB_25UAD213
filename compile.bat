@echo off
REM Compile script for Transaction Processing System - Windows
REM Usage: Double-click this file OR run from command prompt
REM Requires: GCC compiler installed (MinGW or similar)

echo ╔════════════════════════════════════════════════════════╗
echo ║   Transaction Processing System - Compile Script      ║
echo ║                    Windows (PowerShell)               ║
echo ╚════════════════════════════════════════════════════════╝
echo.

REM Check if GCC is available
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] GCC compiler not found!
    echo.
    echo Please install MinGW-64 from: https://www.mingw-w64.org/
    echo Or ensure GCC is in your system PATH
    echo.
    pause
    exit /b 1
)

echo [INFO] Compiler found. Starting compilation...
echo.

REM Compile the program
echo [COMPILE] gcc -std=c99 -Wall -pedantic -o transaction_system transaction_system.c
gcc -std=c99 -Wall -pedantic -o transaction_system.exe transaction_system.c

REM Check if compilation succeeded
if %errorlevel% equ 0 (
    echo.
    echo ╔════════════════════════════════════════════════════════╗
    echo ║           [SUCCESS] Compilation Successful!           ║
    echo ║                                                        ║
    echo ║  Executable: transaction_system.exe                   ║
    echo ║  Run with: transaction_system.exe                     ║
    echo ║           (or double-click the .exe file)             ║
    echo ╚════════════════════════════════════════════════════════╝
    echo.
    
    REM Offer to run the program
    set /p run_now="Run the program now? (Y/N): "
    if /i "%run_now%"=="Y" (
        echo.
        transaction_system.exe
    )
) else (
    echo.
    echo ╔════════════════════════════════════════════════════════╗
    echo ║      [ERROR] Compilation Failed!                      ║
    echo ║      Check error messages above                        ║
    echo ╚════════════════════════════════════════════════════════╝
    echo.
    pause
    exit /b 1
)
