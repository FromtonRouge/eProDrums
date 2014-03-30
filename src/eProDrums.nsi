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

  File "..\install\Qt5CLucene.dll"
  File "..\install\Qt5Core.dll"
  File "..\install\Qt5Gui.dll"
  File "..\install\Qt5Help.dll"
  File "..\install\Qt5Multimedia.dll"
  File "..\install\Qt5MultimediaWidgets.dll"
  File "..\install\Qt5Network.dll"
  File "..\install\Qt5OpenGL.dll"
  File "..\install\Qt5Positioning.dll"
  File "..\install\Qt5PrintSupport.dll"
  File "..\install\Qt5Qml.dll"
  File "..\install\Qt5Quick.dll"
  File "..\install\Qt5Sensors.dll"
  File "..\install\Qt5Sql.dll"
  File "..\install\Qt5Svg.dll"
  File "..\install\Qt5WebKit.dll"
  File "..\install\Qt5WebKitWidgets.dll"
  File "..\install\Qt5Widgets.dll"
  File "..\install\assistant.exe"
  File "..\install\icudt51.dll"
  File "..\install\icuin51.dll"
  File "..\install\icuuc51.dll"
  File "..\install\msvcp100.dll"
  File "..\install\msvcr100.dll"
  File "..\install\portmidi.dll"
  File "..\install\qwt.dll"

  ; Platforms plugin
  SetOutPath $INSTDIR\platforms
  File "..\install\platforms\qwindows.dll"

  ; Sql drivers for assistant
  SetOutPath $INSTDIR\sqldrivers
  File "..\install\sqldrivers\qsqlite.dll"

  ; Documentation
  SetOutPath $INSTDIR\doc
  File "doc\collection.qhc"
  File "doc\doc.qch"

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

  Delete "$INSTDIR\${APPLICATION_NAME}.exe"

  Delete "$INSTDIR\Qt5CLucene.dll"
  Delete "$INSTDIR\Qt5Core.dll"
  Delete "$INSTDIR\Qt5Gui.dll"
  Delete "$INSTDIR\Qt5Help.dll"
  Delete "$INSTDIR\Qt5Multimedia.dll"
  Delete "$INSTDIR\Qt5MultimediaWidgets.dll"
  Delete "$INSTDIR\Qt5Network.dll"
  Delete "$INSTDIR\Qt5OpenGL.dll"
  Delete "$INSTDIR\Qt5Positioning.dll"
  Delete "$INSTDIR\Qt5PrintSupport.dll"
  Delete "$INSTDIR\Qt5Qml.dll"
  Delete "$INSTDIR\Qt5Quick.dll"
  Delete "$INSTDIR\Qt5Sensors.dll"
  Delete "$INSTDIR\Qt5Sql.dll"
  Delete "$INSTDIR\Qt5Svg.dll"
  Delete "$INSTDIR\Qt5WebKit.dll"
  Delete "$INSTDIR\Qt5WebKitWidgets.dll"
  Delete "$INSTDIR\Qt5Widgets.dll"
  Delete "$INSTDIR\assistant.exe"
  Delete "$INSTDIR\icudt51.dll"
  Delete "$INSTDIR\icuin51.dll"
  Delete "$INSTDIR\icuuc51.dll"
  Delete "$INSTDIR\msvcp100.dll"
  Delete "$INSTDIR\msvcr100.dll"
  Delete "$INSTDIR\portmidi.dll"
  Delete "$INSTDIR\qwt.dll"

  Delete "$INSTDIR\uninstall.exe"

  Delete "$INSTDIR\platforms\qwindows.dll"
  RMDir "$INSTDIR\platforms"

  Delete "$INSTDIR\sqldrivers\qsqlite.dll"
  RMDir "$INSTDIR\sqldrivers"

  Delete "$INSTDIR\doc\collection.qhc"
  Delete "$INSTDIR\doc\doc.qch"
  RMDir "$INSTDIR\doc"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${APPLICATION_NAME}\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\${APPLICATION_NAME}"
  RMDir "$INSTDIR"

SectionEnd
