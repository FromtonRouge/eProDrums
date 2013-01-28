; eProDrums.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install eProDrums.nsi into a directory that the user selects,

;--------------------------------

!define ORGANIZATION_NAME "FromtonRouge"
!define APPLICATION_NAME "eProDrums"

; The name of the installer
Name "${APPLICATION_NAME}"

!define /date TIMESTAMP "%Y-%m-%d"

; The file to write
OutFile "${TIMESTAMP}-unstable-${APPLICATION_NAME}.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\${ORGANIZATION_NAME}\${APPLICATION_NAME}"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\${ORGANIZATION_NAME}\${APPLICATION_NAME}" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "${APPLICATION_NAME} (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "Release\${APPLICATION_NAME}.exe"
  File "..\install\assistant.exe"
  File "..\install\QtGui4.dll"
  File "..\install\QtCore4.dll"
  File "..\install\QtSvg4.dll"
  File "..\install\qwt.dll"
  File "..\install\msvcp100.dll"
  File "..\install\msvcr100.dll"
  File "..\install\portmidi.dll"

  ; Documentation
  SetOutPath $INSTDIR\doc
  File "doc\collection.qhc"

  ; Create user settings directories
  SetOutPath "$LOCALAPPDATA\${ORGANIZATION_NAME}\${APPLICATION_NAME}\Kits"

  ; Write the installation path into the registry
  WriteRegStr HKLM "Software\${ORGANIZATION_NAME}\${APPLICATION_NAME}" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ORGANIZATION_NAME}\${APPLICATION_NAME}" "DisplayName" "${APPLICATION_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ORGANIZATION_NAME}\${APPLICATION_NAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ORGANIZATION_NAME}\${APPLICATION_NAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ORGANIZATION_NAME}\${APPLICATION_NAME}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\${APPLICATION_NAME}"
  CreateShortCut "$SMPROGRAMS\${APPLICATION_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\${APPLICATION_NAME}\${APPLICATION_NAME}.lnk" "$INSTDIR\${APPLICATION_NAME}.exe" "" "$INSTDIR\${APPLICATION_NAME}.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ORGANIZATION_NAME}\${APPLICATION_NAME}"
  DeleteRegKey HKLM "Software\${ORGANIZATION_NAME}\${APPLICATION_NAME}"

  ; Remove files and uninstaller
  Delete $INSTDIR\${APPLICATION_NAME}.exe
  Delete $INSTDIR\assistant.exe
  Delete $INSTDIR\QtGui4.dll
  Delete $INSTDIR\QtCore4.dll
  Delete $INSTDIR\QtSvg4.dll
  Delete $INSTDIR\qwt.dll
  Delete $INSTDIR\msvcp100.dll
  Delete $INSTDIR\msvcr100.dll
  Delete $INSTDIR\portmidi.dll
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${APPLICATION_NAME}\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\${APPLICATION_NAME}"
  RMDir "$INSTDIR"

SectionEnd
