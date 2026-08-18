@echo off
setlocal EnableDelayedExpansion
:: ─────────────────────────────────────────────────────────────────────────────
:: Rigset — self-installer for Windows
::
:: Requirements: winget (Windows Package Manager) — ships with Windows 11
::               and Windows 10 (1809+) via Microsoft Store / App Installer.
::
:: Run as a normal user (winget will request elevation when needed).
:: ─────────────────────────────────────────────────────────────────────────────

title Rigset — Self Installer

echo.
echo  ╔══════════════════════════════════════════╗
echo  ║       Rigset — Self Installer            ║
echo  ╚══════════════════════════════════════════╝
echo.

:: ── Check Windows version ────────────────────────────────────────────────────
for /f "tokens=4-5 delims=. " %%i in ('ver') do set WIN_VER=%%i.%%j
echo [INFO]  Windows version: %WIN_VER%

:: ── Check winget ─────────────────────────────────────────────────────────────
where winget >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARN]  winget not found. Opening Microsoft Store to install App Installer...
    start ms-windows-store://pdp/?ProductId=9NBLGGH4NNS1
    echo [INFO]  Please install "App Installer" from the Store, then re-run this script.
    pause
    exit /b 1
)
echo [ OK ]  winget found.

:: ── Check for pre-built binary alongside the script ──────────────────────────
set "SCRIPT_DIR=%~dp0"
set "PREBUILT=%SCRIPT_DIR%build\Release\Rigset.exe"
if not exist "%PREBUILT%" set "PREBUILT=%SCRIPT_DIR%build\Rigset.exe"

if exist "%PREBUILT%" (
    echo [ OK ]  Pre-built binary found: %PREBUILT%
    goto :do_install
)

:: ── Install build toolchain via winget ───────────────────────────────────────
echo [INFO]  Installing build tools (CMake, Visual Studio Build Tools, Qt6)...

:: CMake
winget install --silent --accept-package-agreements --accept-source-agreements Kitware.CMake
if %errorlevel% neq 0 (
    echo [WARN]  CMake install may have failed or was already installed.
)

:: Visual Studio 2022 Build Tools (C++ workload)
winget install --silent --accept-package-agreements --accept-source-agreements ^
    Microsoft.VisualStudio.2022.BuildTools ^
    --override "--quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
if %errorlevel% neq 0 (
    echo [WARN]  VS Build Tools install may have failed or was already installed.
)

:: Qt 6 via winget (Qt Online Installer)
:: Note: Qt doesn't have a winget package for the full SDK.
:: We use the Qt Maintenance Tool approach via a direct download.
echo [INFO]  Checking for Qt6 installation...
set "QT_DIR="
for %%d in (
    "C:\Qt\6.7\msvc2022_64"
    "C:\Qt\6.6\msvc2022_64"
    "C:\Qt\6.5\msvc2022_64"
    "%USERPROFILE%\Qt\6.7\msvc2022_64"
    "%USERPROFILE%\Qt\6.6\msvc2022_64"
) do (
    if exist "%%~d\lib\cmake\Qt6\Qt6Config.cmake" (
        set "QT_DIR=%%~d"
        goto :qt_found
    )
)

echo [WARN]  Qt6 not found in common locations.
echo [WARN]  Please install Qt6 from https://www.qt.io/download-open-source
echo [WARN]  Then re-run this script.
echo.
echo [INFO]  Opening Qt download page...
start https://www.qt.io/download-open-source
pause
exit /b 1

:qt_found
echo [ OK ]  Qt6 found at: %QT_DIR%

:: ── Configure CMake PATH ──────────────────────────────────────────────────────
:: Add cmake to PATH for this session (winget may have just installed it)
set "CMAKE_SEARCH=C:\Program Files\CMake\bin"
if exist "%CMAKE_SEARCH%\cmake.exe" (
    set "PATH=%CMAKE_SEARCH%;%PATH%"
)

:: ── Build ─────────────────────────────────────────────────────────────────────
set "SRC_DIR=%SCRIPT_DIR%"
set "BUILD_DIR=%SRC_DIR%build"

if not exist "%SRC_DIR%CMakeLists.txt" (
    echo [FAIL]  CMakeLists.txt not found in %SRC_DIR%
    echo         Run this script from the Rigset source directory.
    pause
    exit /b 1
)

echo [INFO]  Creating build directory: %BUILD_DIR%
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo [INFO]  Configuring with CMake...
cmake -S "%SRC_DIR%" -B "%BUILD_DIR%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%QT_DIR%"
if %errorlevel% neq 0 (
    echo [FAIL]  CMake configuration failed.
    pause
    exit /b 1
)

echo [INFO]  Building...
cmake --build "%BUILD_DIR%" --config Release --parallel
if %errorlevel% neq 0 (
    echo [FAIL]  Build failed.
    pause
    exit /b 1
)

set "PREBUILT=%BUILD_DIR%\Release\Rigset.exe"
if not exist "%PREBUILT%" set "PREBUILT=%BUILD_DIR%\Rigset.exe"

if not exist "%PREBUILT%" (
    echo [FAIL]  Binary not found after build. Check build output above.
    pause
    exit /b 1
)
echo [ OK ]  Build complete: %PREBUILT%

:do_install
:: ── Deploy Qt DLLs ────────────────────────────────────────────────────────────
echo [INFO]  Deploying Qt runtime DLLs...
set "DEPLOY_DIR=%SCRIPT_DIR%dist"
if not exist "%DEPLOY_DIR%" mkdir "%DEPLOY_DIR%"

copy /y "%PREBUILT%" "%DEPLOY_DIR%\Rigset.exe" >nul

:: Find windeployqt
set "WINDEPLOY="
for %%p in (
    "%QT_DIR%\..\..\..\bin\windeployqt.exe"
    "%QT_DIR%\bin\windeployqt.exe"
    "C:\Qt\6.7\msvc2022_64\bin\windeployqt.exe"
) do (
    if exist "%%~p" (
        set "WINDEPLOY=%%~p"
        goto :deploy
    )
)
echo [WARN]  windeployqt not found — skipping Qt DLL deployment.
echo [WARN]  The app may fail to launch without Qt DLLs in the same folder.
goto :install_shortcut

:deploy
echo [INFO]  Running windeployqt...
"%WINDEPLOY%" --release --no-translations --no-system-d3d-compiler ^
    --no-opengl-sw "%DEPLOY_DIR%\Rigset.exe"
echo [ OK ]  Qt DLLs deployed.

:install_shortcut
:: ── Create Start Menu shortcut ────────────────────────────────────────────────
echo [INFO]  Creating Start Menu shortcut...
set "SHORTCUT_DIR=%APPDATA%\Microsoft\Windows\Start Menu\Programs"
set "SHORTCUT=%SHORTCUT_DIR%\Rigset.lnk"

:: Use PowerShell to create the shortcut
powershell -NoProfile -Command ^
    "$ws = New-Object -ComObject WScript.Shell; " ^
    "$sc = $ws.CreateShortcut('%SHORTCUT%'); " ^
    "$sc.TargetPath = '%DEPLOY_DIR%\Rigset.exe'; " ^
    "$sc.WorkingDirectory = '%DEPLOY_DIR%'; " ^
    "$sc.Description = 'Install your favourite apps in one click'; " ^
    "$sc.Save()"

if %errorlevel% eq 0 (
    echo [ OK ]  Start Menu shortcut created.
) else (
    echo [WARN]  Could not create Start Menu shortcut.
)

:: ── Desktop shortcut (optional) ──────────────────────────────────────────────
set "DESKTOP_SC=%USERPROFILE%\Desktop\Rigset.lnk"
powershell -NoProfile -Command ^
    "$ws = New-Object -ComObject WScript.Shell; " ^
    "$sc = $ws.CreateShortcut('%DESKTOP_SC%'); " ^
    "$sc.TargetPath = '%DEPLOY_DIR%\Rigset.exe'; " ^
    "$sc.WorkingDirectory = '%DEPLOY_DIR%'; " ^
    "$sc.Description = 'Install your favourite apps in one click'; " ^
    "$sc.Save()" >nul 2>&1

:: ── Done ──────────────────────────────────────────────────────────────────────
echo.
echo  ════════════════════════════════════════════
echo    Rigset installed successfully!
echo    Location: %DEPLOY_DIR%\Rigset.exe
echo    Shortcut: Start Menu ^> Rigset
echo  ════════════════════════════════════════════
echo.

set /p LAUNCH="Launch Rigset now? [Y/n]: "
if /i "!LAUNCH!" neq "n" (
    start "" "%DEPLOY_DIR%\Rigset.exe"
)

endlocal
