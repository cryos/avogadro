;--------------------------------

; The name of the installer
Name "Avogadro"
!define VERSION "0.0.2"

; The file to write
OutFile "avogadro-${VERSION}.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Avogadro

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Avogadro" "Install_Dir"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Avogadro Core (required)"

  SectionIn RO
  
  SetOutPath $INSTDIR\engines
  File libavogadro\src\engines\release\bsengine.dll
  
  SetOutPath $INSTDIR\plugins
  File avogadro\src\plugins\release\draw.dll
  File avogadro\src\plugins\release\selectrotate.dll

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File C:\Qt\4.2.2\lib\QtGui4.dll
  File C:\Qt\4.2.2\lib\QtCore4.dll
  File C:\Qt\4.2.2\lib\QtOpenGL4.dll
  File C:\openbabel\windows-vc2005\OpenBabelDLL\OpenBabel.dll
  File C:\openbabel\windows-vc2005\iconv.dll
  File C:\openbabel\windows-vc2005\libxml2.dll
  File C:\openbabel\windows-vc2005\libinchi.dll
  File C:\openbabel\windows-vc2005\zlib1.dll
  File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcm80.dll"
  File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcp80.dll"
  File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcr80.dll"
  File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\Microsoft.VC80.CRT.manifest"
  File libavogadro\src\release\avogadro.dll
  File avogadro\src\release\avogadro.exe

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Avogadro "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "DisplayName" "Avogadro"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Avogadro"
  CreateShortCut "$SMPROGRAMS\Avogadro\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Avogadro\Avogadro.lnk" "$INSTDIR\avogadro.exe" "" "$INSTDIR\avogadro.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Avogadro"
  DeleteRegKey HKLM SOFTWARE\Avogadro

  ; Remove files and uninstaller
  Delete $INSTDIR\plugins\draw.dll
  Delete $INSTDIR\plugins\selectrotate.dll
  Delete $INSTDIR\engines\bsengine.dll
  Delete $INSTDIR\QtGui4.dll
  Delete $INSTDIR\QtCore4.dll
  Delete $INSTDIR\QtOpenGL4.dll
  Delete $INSTDIR\OpenBabel.dll
  Delete $INSTDIR\iconv.dll
  Delete $INSTDIR\libxml2.dll
  Delete $INSTDIR\libinchi.dll
  Delete $INSTDIR\zlib1.dll
  Delete $INSTDIR\msvcp80.dll
  Delete $INSTDIR\msvcm80.dll
  Delete $INSTDIR\msvcp80.dll
  Delete $INSTDIR\msvcr80.dll
  Delete $INSTDIR\Microsoft.VC80.CRT.manifest
  Delete $INSTDIR\avogadro.dll
  Delete $INSTDIR\avogadro.exe
  Delete $INSTDIR\uninstall.exe


  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Avogadro\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Avogadro"
  RMDir "$INSTDIR\plugins"
  RMDir "$INSTDIR\engines"
  RMDir "$INSTDIR"

SectionEnd
