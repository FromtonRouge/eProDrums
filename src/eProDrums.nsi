; eProDrums.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install eProDrums.nsi into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "eProDrums"

!define /date TIMESTAMP "%Y-%m-%d"

; The file to write
OutFile "${TIMESTAMP}-unstable-eProDrums.exe"

; The default installation directory
InstallDir $PROGRAMFILES\eProDrums

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\eProDrums" "Install_Dir"

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
Section "eProDrums (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "Release\eProDrums.exe"
  File "..\install\assistant.exe"
  File "..\install\QtGui4.dll"
  File "..\install\QtCore4.dll"
  File "..\install\QtSvg4.dll"
  File "..\install\qwt.dll"
  File "..\install\msvcp100.dll"
  File "..\install\msvcr100.dll"
  File "..\install\portmidi.dll"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\eProDrums "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eProDrums" "DisplayName" "eProDrums"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eProDrums" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eProDrums" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eProDrums" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\eProDrums"
  CreateShortCut "$SMPROGRAMS\eProDrums\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\eProDrums\eProDrums.lnk" "$INSTDIR\eProDrums.exe" "" "$INSTDIR\eProDrums.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\eProDrums"
  DeleteRegKey HKLM SOFTWARE\eProDrums

  ; Remove files and uninstaller
  Delete $INSTDIR\eProDrums.exe
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
  Delete "$SMPROGRAMS\eProDrums\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\eProDrums"
  RMDir "$INSTDIR"

SectionEnd
