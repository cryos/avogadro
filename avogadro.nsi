;--------------------------------

; The name of the installer
Name "Avogadro"
!define VERSION "0.2.0"

!define QT_VERSION "4.3.1"

; The file to write
OutFile "avogadro-${VERSION}.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Avogadro

; Build directory
!define BUILDDIR "C:\avogadro\build"

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
  File ${BUILDDIR}\libavogadro\src\engines\release\axesengine.dll
  File ${BUILDDIR}\libavogadro\src\engines\release\bsdyengine.dll
  File ${BUILDDIR}\libavogadro\src\engines\release\debugengine.dll
  File ${BUILDDIR}\libavogadro\src\engines\release\labelengine.dll
  File ${BUILDDIR}\libavogadro\src\engines\release\sphereengine.dll
  File ${BUILDDIR}\libavogadro\src\engines\release\stickengine.dll
  File ${BUILDDIR}\libavogadro\src\engines\release\wireengine.dll
  
  SetOutPath $INSTDIR\tools
  File ${BUILDDIR}\libavogadro\src\tools\release\autorotatetool.dll
  File ${BUILDDIR}\libavogadro\src\tools\release\bondcentrictool.dll
  File ${BUILDDIR}\libavogadro\src\tools\release\clickmeasuretool.dll
  File ${BUILDDIR}\libavogadro\src\tools\release\drawtool.dll
  File ${BUILDDIR}\libavogadro\src\tools\release\manipulatetool.dll
  File ${BUILDDIR}\libavogadro\src\tools\release\navigatetool.dll
  File ${BUILDDIR}\libavogadro\src\tools\release\selectrotatetool.dll

  SetOutPath $INSTDIR\extensions
  File ${BUILDDIR}\avogadro\src\extensions\release\gamessextension.dll
  File ${BUILDDIR}\avogadro\src\extensions\release\ghemicalextension.dll
  File ${BUILDDIR}\avogadro\src\extensions\release\h2methylextension.dll
  File ${BUILDDIR}\avogadro\src\extensions\release\hydrogensextension.dll
  File ${BUILDDIR}\avogadro\src\extensions\release\smartsextension.dll
  File ${BUILDDIR}\avogadro\src\extensions\release\unitcellextension.dll

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File C:\Qt\${QT_VERSION}\lib\QtGui4.dll
  File C:\Qt\${QT_VERSION}\lib\QtCore4.dll
  File C:\Qt\${QT_VERSION}\lib\QtOpenGL4.dll
  File C:\openbabel\data\ghemical.prm
  File C:\openbabel\windows-vc2005\OpenBabelDLL\OpenBabel.dll
  File C:\openbabel\windows-vc2005\iconv.dll
  File C:\openbabel\windows-vc2005\libxml2.dll
  File C:\openbabel\windows-vc2005\libinchi.dll
  File C:\openbabel\windows-vc2005\zlib1.dll
  File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcm80.dll"
  File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcp80.dll"
  File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcr80.dll"
  File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\Microsoft.VC80.CRT.manifest"
  File ${BUILDDIR}\libavogadro\src\release\avogadro.dll
  File ${BUILDDIR}\avogadro\src\release\avogadro.exe

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Avogadro "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Avogadro" "DisplayName" "Avogadro -- Molecular Editing Software"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Avogadro" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Avogadro" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Avogadro" "NoRepair" 1
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

  Delete $INSTDIR\engines\axesengine.dll
  Delete $INSTDIR\engines\bsdyengine.dll
  Delete $INSTDIR\engines\debugengine.dll
  Delete $INSTDIR\engines\labelengine.dll
  Delete $INSTDIR\engines\sphereengine.dll
  Delete $INSTDIR\engines\stickengine.dll
  Delete $INSTDIR\engines\wireengine.dll
  
  ; Remove files and uninstaller
  Delete $INSTDIR\tools\autorotatetool.dll
  Delete $INSTDIR\tools\bondcentrictool.dll
  Delete $INSTDIR\tools\clickmeasuretool.dll
  Delete $INSTDIR\tools\drawtool.dll
  Delete $INSTDIR\tools\manipulatetool.dll
  Delete $INSTDIR\tools\navigatetool.dll
  Delete $INSTDIR\tools\selectrotatetool.dll
	
  Delete $INSTDIR\extensions\gamessextension.dll
  Delete $INSTDIR\extensions\ghemicalextension.dll
  Delete $INSTDIR\extensions\h2methylextension.dll
  Delete $INSTDIR\extensions\hydrogensextension.dll
  Delete $INSTDIR\extensions\smartsextension.dll
  Delete $INSTDIR\extensions\unitcellextension.dll

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
  Delete $INSTDIR\ghemical.prm
  Delete $INSTDIR\uninstall.exe


  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Avogadro\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Avogadro"
  RMDir "$INSTDIR\tools"
  RMDir "$INSTDIR\extensions"
  RMDir "$INSTDIR\engines"
  RMDir "$INSTDIR"

SectionEnd
