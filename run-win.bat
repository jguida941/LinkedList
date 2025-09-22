@echo off
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set BUILD_DIR=%SCRIPT_DIR%build

rem Resize console for readability (override with env)
set TARGET_COLS=%TARGET_COLS%
if "%TARGET_COLS%"=="" set TARGET_COLS=120
set TARGET_ROWS=%TARGET_ROWS%
if "%TARGET_ROWS%"=="" set TARGET_ROWS=35
mode con: cols=%TARGET_COLS% lines=%TARGET_ROWS% >nul 2>&1

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

rem Try Visual Studio 2022 generator first
cmake -S "%SCRIPT_DIR%" -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
  echo Visual Studio 2022 not detected, trying MinGW Makefiles...
  cmake -S "%SCRIPT_DIR%" -B "%BUILD_DIR%" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release || goto :cmake_failed
  cmake --build "%BUILD_DIR%" --config Release || goto :build_failed
  set BIN="%BUILD_DIR%\Linked_List.exe"
) else (
  cmake --build "%BUILD_DIR%" --config Release || goto :build_failed
  set BIN="%BUILD_DIR%\Release\Linked_List.exe"
)

set CSV=%1
if "%CSV%"=="" set CSV=eBid_Monthly_Sales.csv
set BID=%2
if "%BID%"=="" set BID=98109

rem Resolve CSV relative to script dir; allow CLion debug fallback
set "_ABS=%CSV%"
for /f "delims=: tokens=1" %%A in ("%_ABS%") do set _DRIVE=%%A
if "%_DRIVE%"=="%_ABS%" (
  rem No drive letter -> treat as relative
  if not exist "%SCRIPT_DIR%%CSV%" (
    if exist "%SCRIPT_DIR%cmake-build-debug\%CSV%" (
      set CSV=cmake-build-debug\%CSV%
    )
  )
  set CSV=%SCRIPT_DIR%%CSV%
) else (
  rem Has drive letter; leave as-is
)

if not exist "%CSV%" (
  echo Error: CSV file not found at: %CSV%
  echo Pass a valid path as the first argument, e.g.
  echo    %~nx0 C:\path\to\eBid_Monthly_Sales.csv 98109
  echo.
  pause
  exit /b 1
)

echo Running: %BIN% "%CSV%" %BID%
%BIN% "%CSV%" %BID%
if %ERRORLEVEL% NEQ 0 echo Program returned error code %ERRORLEVEL%

echo.
pause
exit /b 0

:cmake_failed
echo CMake configuration failed.
pause
exit /b 1

:build_failed
echo Build failed.
pause
exit /b 1
