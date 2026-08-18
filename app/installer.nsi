; ─────────────────────────────────────────────────────────────────────────────
; installer.nsi — NSIS Installer Script for Rigset
;
; Creates a proper Windows .exe installer with a wizard UI:
;   Welcome → License → Choose Dir → Install → Finish
;
; Prerequisites:
;   - NSIS (https://nsis.sourceforge.io) installed
;   - Rigset built (build\Rigset.exe exists)
;
; Compile:
;   makensis installer.nsi
;
; Output:
;   Rigset-Setup.exe (standalone installer)
; ─────────────────────────────────────────────────────────────────────────────

!include "MUI2.nsh"

; ── General ──────────────────────────────────────────────────────────────────
Name "Rigset"
OutFile "Rigset-Setup.exe"
InstallDir "$LOCALAPPDATA\Rigset"
InstallDirRegKey HKCU "Software\Rigset" "InstallDir"
RequestExecutionLevel user
BrandingText "Rigset"

!define APP_NAME      "Rigset"
!define APP_VERSION   "1.0"
!define APP_PUBLISHER "Rigset Contributors"
!define APP_WEB_SITE  "https://flickfpz.github.io/appleinstaller-site"
!define HELP_LINK     "https://github.com/flickfpz/appleinstaller/issues"

; ── Version Info ─────────────────────────────────────────────────────────────
VIProductVersion "${APP_VERSION}.0.0"
VIAddVersionKey "ProductName"     "${APP_NAME}"
VIAddVersionKey "ProductVersion"  "${APP_VERSION}"
VIAddVersionKey "CompanyName"     "${APP_PUBLISHER}"
VIAddVersionKey "FileDescription" "${APP_NAME} Installer"
VIAddVersionKey "LegalCopyright"  "MIT License"

; ── MUI Settings ─────────────────────────────────────────────────────────────
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; ── Pages ────────────────────────────────────────────────────────────────────
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; ── Installer Sections ──────────────────────────────────────────────────────
Section "Rigset (required)" SecMain
  SectionIn RO

  SetOutPath "$INSTDIR"

  ; ── Check for pre-built binary ─────────────────────────────────────────────
  ; Try multiple locations where the built binary might be

  IfFileExists "build\Release\Rigset.exe" 0 +3
    CopyFiles "build\Release\Rigset.exe" "$INSTDIR\Rigset.exe"
    Goto binary_done

  IfFileExists "build\Rigset.exe" 0 +3
    CopyFiles "build\Rigset.exe" "$INSTDIR\Rigset.exe"
    Goto binary_done

  IfFileExists "..\build\Release\Rigset.exe" 0 +3
    CopyFiles "..\build\Release\Rigset.exe" "$INSTDIR\Rigset.exe"
    Goto binary_done

  IfFileExists "..\build\Rigset.exe" 0 +3
    CopyFiles "..\build\Rigset.exe" "$INSTDIR\Rigset.exe"
    Goto binary_done

  IfFileExists "build\Release\Rigset.exe" 0 +3
    CopyFiles "build\Release\Rigset.exe" "$INSTDIR\Rigset.exe"
    Goto binary_done

  IfFileExists "build\Rigset.exe" 0 +3
    CopyFiles "build\Rigset.exe" "$INSTDIR\Rigset.exe"
    Goto binary_done

  IfFileExists "..\build\Release\Rigset.exe" 0 +3
    CopyFiles "..\build\Release\Rigset.exe" "$INSTDIR\Rigset.exe"
    Goto binary_done

  IfFileExists "..\build\Rigset.exe" 0 +3
    CopyFiles "..\build\Rigset.exe" "$INSTDIR\Rigset.exe"
    Goto binary_done

  ; Binary not found — show error
  MessageBox MB_ICONSTOP "Built binary not found.$\n$\nPlease build the project first (run install.bat or build with CMake),$\nthen run this installer from the project directory."
  Abort

  binary_done:

  ; ── Deploy Qt DLLs via windeployqt ─────────────────────────────────────────
  DetailPrint "Deploying Qt runtime DLLs..."
  nsExec::ExecToLog '"$INSTDIR\Rigset.exe" --version'
  ; Try windeployqt from common Qt locations
  var /GLOBAL WINDEPLOY
  StrCpy $WINDEPLOY ""

  IfFileExists "C:\Qt\6.7\msvc2022_64\bin\windeployqt.exe" 0 +3
    StrCpy $WINDEPLOY "C:\Qt\6.7\msvc2022_64\bin\windeployqt.exe"
    Goto deploy_qt

  IfFileExists "C:\Qt\6.6\msvc2022_64\bin\windeployqt.exe" 0 +3
    StrCpy $WINDEPLOY "C:\Qt\6.6\msvc2022_64\bin\windeployqt.exe"
    Goto deploy_qt

  IfFileExists "C:\Qt\6.5\msvc2022_64\bin\windeployqt.exe" 0 +3
    StrCpy $WINDEPLOY "C:\Qt\6.5\msvc2022_64\bin\windeployqt.exe"
    Goto deploy_qt

  Goto skip_deploy

  deploy_qt:
    DetailPrint "Running windeployqt..."
    nsExec::ExecToLog '"$WINDEPLOY" --release --no-translations --no-system-d3d-compiler --no-opengl-sw "$INSTDIR\Rigset.exe"'

  skip_deploy:

  ; ── Copy install scripts ──────────────────────────────────────────────────
  SetOutPath "$INSTDIR"
  IfFileExists "..\install.bat" 0 +2
    CopyFiles "..\install.bat" "$INSTDIR\install.bat"
  IfFileExists "..\install.sh" 0 +2
    CopyFiles "..\install.sh" "$INSTDIR\install.sh"

  ; ── Start Menu shortcut ───────────────────────────────────────────────────
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\Rigset.exe"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"   "$INSTDIR\Uninstall.exe"

  ; ── Desktop shortcut (optional) ───────────────────────────────────────────
  MessageBox MB_YESNO "Create a desktop shortcut?" IDNO skip_desktop
    CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\Rigset.exe"
  skip_desktop:

  ; ── Store install path ────────────────────────────────────────────────────
  WriteRegStr HKCU "Software\Rigset" "InstallDir" "$INSTDIR"

  ; ── Uninstaller ────────────────────────────────────────────────────────────
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; ── Add/Remove Programs entry ─────────────────────────────────────────────
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rigset" \
    "DisplayName"     "${APP_NAME}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rigset" \
    "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rigset" \
    "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rigset" \
    "DisplayVersion"  "${APP_VERSION}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rigset" \
    "Publisher"       "${APP_PUBLISHER}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rigset" \
    "URLInfoAbout"    "${APP_WEB_SITE}"
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rigset" \
    "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rigset" \
    "NoRepair" 1

SectionEnd

; ── Uninstaller Section ──────────────────────────────────────────────────────
Section "Uninstall"

  ; Remove files
  Delete "$INSTDIR\Rigset.exe"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\install.bat"
  Delete "$INSTDIR\install.sh"
  RMDir  "$INSTDIR"

  ; Remove shortcuts
  Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\${APP_NAME}"
  Delete "$DESKTOP\${APP_NAME}.lnk"

  ; Remove registry keys
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rigset"
  DeleteRegKey HKCU "Software\Rigset"

SectionEnd

; ── Callbacks ────────────────────────────────────────────────────────────────
Function .onInit
  ; Check if already installed
  ReadRegStr $0 HKCU "Software\Rigset" "InstallDir"
  StrCmp $0 "" done
    MessageBox MB_YESNO "${APP_NAME} is already installed.$\n$\nDo you want to reinstall?" IDYES done
    Abort
  done:
FunctionEnd
