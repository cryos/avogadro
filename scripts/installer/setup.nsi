# owner		Avogadro Team
# license	GPL version 2
# author	Uwe Stöhr
# file version	1.0	date	28-05-2008

# To compile this script NSIS 2.30 or newer are required
# http://nsis.sourceforge.net/


# Do a Cyclic Redundancy Check to make sure the installer
# was not corrupted by the download.
CRCCheck force

# Make the installer as small as possible.
SetCompressor lzma

# set execution level for Windows Vista
RequestExecutionLevel user

# general definitions
# you only need to change this section for new releases
VIProductVersion "1.1.0.0" # file version for the installer in the scheme "x.x.x.x"
!define VERSION "1.1.0"
Name "Avogadro"
!define REGKEY "SOFTWARE\Avogadro"
!define COMPANY "Avogadro Team"
!define URL "http://avogadro.openmolecules.net/wiki/Main_Page"
!define PRODUCT_NAME "Avogadro"
!define PRODUCT_EXE "$INSTDIR\Avogadro.exe"
!define PRODUCT_EXE2 "Avogadro.exe"
!define PRODUCT_REGNAME "Avogadro.Document"
!define PRODUCT_EXT ".cml"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)"
!define PRODUCT_LICENSE_FILE "dist\gpl.txt"


# registry preparations
!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0


# Variables
Var StartmenuFolder
Var CreateFileAssociations
Var CreateDesktopIcon
Var Answer
Var UserName


# Included files
!include "MUI.nsh"
!include "LogicLib.nsh"


# This macro is used to check if we are administrator or user
!macro IsUserAdmin Result UName

  # clear previous set errors
  ClearErrors
  # see if we can get user name
  UserInfo::GetName
  # if an error occured, goto Win9x
  IfErrors Win9x
  # pop the name from the stack into $0
  Pop $0
  # copy the name to ${UName}
  StrCpy ${UName} $0
  # get the account type
  UserInfo::GetAccountType
  # pop the account type from the stack
  Pop $1
  # check the account type and set ${Result}
  ${if} $1 == "Admin"
    StrCpy ${Result} "yes"
  ${else}
    StrCpy ${Result} "no"
  ${endif}
  Goto done

  Win9x:
  # Win9x doesn't have administrator, we can write wherever we want (not sure if we support Win9x though)
  StrCpy ${Result} "yes"
  done:

!macroend

; ################################################################
; appends \ to the path if missing
; example: !insertmacro GetCleanDir "c:\blabla"
; Pop $0 => "c:\blabla\"
!macro GetCleanDir INPUTDIR
  !define Index_GetCleanDir 'GetCleanDir_Line${__LINE__}'
  Push $R0
  Push $R1
  StrCpy $R0 "${INPUTDIR}"
  StrCmp $R0 "" ${Index_GetCleanDir}-finish
  StrCpy $R1 "$R0" "" -1
  StrCmp "$R1" "\" ${Index_GetCleanDir}-finish
  StrCpy $R0 "$R0\"
${Index_GetCleanDir}-finish:
  Pop $R1
  Exch $R0
  !undef Index_GetCleanDir
!macroend
 


; ################################################################
; similar to "RMDIR /r DIRECTORY", but does not remove DIRECTORY itself
; example: !insertmacro RemoveFilesAndSubDirs "$INSTDIR"
!macro RemoveFilesAndSubDirs DIRECTORY
  !define Index_RemoveFilesAndSubDirs 'RemoveFilesAndSubDirs_${__LINE__}'
 
  Push $R0
  Push $R1
  Push $R2
 
  !insertmacro GetCleanDir "${DIRECTORY}"
  Pop $R2
  FindFirst $R0 $R1 "$R2*.*"
${Index_RemoveFilesAndSubDirs}-loop:
  StrCmp $R1 "" ${Index_RemoveFilesAndSubDirs}-done
  StrCmp $R1 "." ${Index_RemoveFilesAndSubDirs}-next
  StrCmp $R1 ".." ${Index_RemoveFilesAndSubDirs}-next
  IfFileExists "$R2$R1\*.*" ${Index_RemoveFilesAndSubDirs}-directory
  ; file
  Delete "$R2$R1"
  goto ${Index_RemoveFilesAndSubDirs}-next
${Index_RemoveFilesAndSubDirs}-directory:
  ; directory
  RMDir /r "$R2$R1"
${Index_RemoveFilesAndSubDirs}-next:
  FindNext $R0 $R1
  Goto ${Index_RemoveFilesAndSubDirs}-loop
${Index_RemoveFilesAndSubDirs}-done:
  FindClose $R0
 
  Pop $R2
  Pop $R1
  Pop $R0
  !undef Index_RemoveFilesAndSubDirs
!macroend



# Installer pages
# Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

# let warning appear when installation is canceled
!define MUI_ABORTWARNING

# Icons for the installer program
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install-full.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall-full.ico"

# Welcome page
!define MUI_WELCOMEPAGE_TEXT "$(WelcomePageText)"
!insertmacro MUI_PAGE_WELCOME

# Show the license.
!insertmacro MUI_PAGE_LICENSE "${PRODUCT_LICENSE_FILE}"

# Specify the installation directory.
!insertmacro MUI_PAGE_DIRECTORY

# choose the components to install.
!insertmacro MUI_PAGE_COMPONENTS

# Specify where to install program shortcuts.
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "$(^Name)"
!insertmacro MUI_PAGE_STARTMENU ${PRODUCT_NAME} $StartmenuFolder

# Watch the components being installed.
!insertmacro MUI_PAGE_INSTFILES

# Finish page
!define MUI_FINISHPAGE_RUN "${PRODUCT_EXE}"
!define MUI_FINISHPAGE_TEXT "$(FinishPageMessage)"
!define MUI_FINISHPAGE_RUN_TEXT "$(FinishPageRun)"
!insertmacro MUI_PAGE_FINISH

# The uninstaller
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES


# Installer languages
!insertmacro MUI_LANGUAGE "English" # first language is the default language
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Russian"

!include "installer_languages\english.nsh"
!include "installer_languages\german.nsh"
!include "installer_languages\russian.nsh"


# Installer attributes
OutFile "avogadro-win32-${VERSION}.exe"
InstallDir "$PROGRAMFILES\Avogadro"
BrandingText "$(^Name) Installer" # appear at the bottom of the installer windows
XPStyle on # use XP style for installer windows
LicenseData "$(AvogadroLicenseData)"

# creates file informations for the AvogadroSetup.exe
VIAddVersionKey ProductName "Avogadro"
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey CompanyName "${COMPANY}"
VIAddVersionKey CompanyWebsite "${URL}"
VIAddVersionKey FileDescription "Avogadro installation program"
VIAddVersionKey LegalCopyright "under the GPL version 2"
VIAddVersionKey FileVersion ""


# Installer sections
Section "!Avogadro" SecCore
 SectionIn RO
SectionEnd

Section "$(SecAssociateCmlTitle)" SecAssociateCml
 StrCpy $CreateFileAssociations "true"
SectionEnd

Section "$(SecDesktopTitle)" SecDesktop
 StrCpy $CreateDesktopIcon "true"
SectionEnd

# section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecAssociateCml} "$(SecAssociateCmlDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "$(SecDesktopDescription)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "-Installation actions" SecInstallation

  # remove old files...
  !insertmacro RemoveFilesAndSubDirs "$INSTDIR"
  # set the output path
  SetOutPath "$INSTDIR"
  # if there are older files, overwrite them
  SetOverwrite on
  # extract all files from dist/ directory (recursively)
  File /r dist\*.*
  WriteRegStr SHCTX "${REGKEY}\Components" Main 1
  
  # register Avogadro
  WriteRegStr SHCTX "${REGKEY}" Path $INSTDIR
  WriteUninstaller $INSTDIR\uninstall.exe
  
  # create shortcuts to startmenu
  SetOutPath "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\$StartmenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\$(^Name).lnk" "${PRODUCT_EXE}" "" "$INSTDIR\Avogadro.exe"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\Release Notes.lnk" "http://avogadro.openmolecules.net/wiki/Avogadro_${VERSION}" ""
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  
  # create desktop icon
  ${if} $CreateDesktopIcon == "true"
    SetOutPath "$INSTDIR"
    CreateShortCut "$DESKTOP\$(^Name).lnk" "${PRODUCT_EXE}" "" "${PRODUCT_EXE}" #$(^Name).lnk
  ${endif}
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "StartMenu" "$SMPROGRAMS\$StartmenuFolder"
  
  ${if} $Answer == "yes" # if user is admin
    # register information that appear in Windows' software listing
    WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
    WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "Publisher" "${COMPANY}"
    WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${URL}"
    WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayIcon" "${PRODUCT_EXE}"
    WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"   
    WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoModify" 0x00000001
    WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoRepair" 0x00000001
  ${endif}
  
  # register the extension .cml
  ${if} $CreateFileAssociations == "true"
    # write informations about file type
    WriteRegStr SHCTX "Software\Classes\${PRODUCT_REGNAME}" "" "${PRODUCT_NAME} Document"
    WriteRegStr SHCTX "Software\Classes\${PRODUCT_REGNAME}\DefaultIcon" "" "${PRODUCT_EXE},0"
    WriteRegStr SHCTX "Software\Classes\${PRODUCT_REGNAME}\Shell\open\command" "" '"${PRODUCT_EXE}" "%1"' 
    # write informations about file extensions
    WriteRegStr SHCTX "Software\Classes\${PRODUCT_EXT}" "" "${PRODUCT_REGNAME}"
    # refresh shell
    System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
  ${endif}

  # reset settings
  DeleteRegKey SHCTX "Software\SourceForge\Avogadro"
  DeleteRegKey HKCU "Software\SourceForge\Avogadro"

SectionEnd


# Uninstaller sections
Section "un.Avogadro" un.SecUnProgramFiles

  SectionIn RO
  # delete installation folder
  RMDir /r $INSTDIR	
  # delete start menu entry
  ReadRegStr $0 SHCTX "${PRODUCT_UNINST_KEY}" "StartMenu"
  RMDir /r "$0"
  
  # delete desktop icon
  Delete "$DESKTOP\$(^Name).lnk"
  
  # remove file extension .lyx
  ReadRegStr $R0 SHCTX "Software\Classes\${PRODUCT_EXT}" ""
  ${if} $R0 == "${PRODUCT_REGNAME}"
   DeleteRegKey SHCTX "Software\Classes\${PRODUCT_EXT}"
   DeleteRegKey SHCTX "Software\Classes\${PRODUCT_REGNAME}"
  ${endif}
  
  # delete remaining registry entries
  DeleteRegKey HKCU "${PRODUCT_UNINST_KEY}"
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKCR "Applications\${PRODUCT_EXE2}"
  DeleteRegKey HKCU "${REGKEY}"
  DeleteRegKey SHCTX "${REGKEY}"
  
  # close uninstaller automatically
  SetAutoClose true

SectionEnd


# Installer functions
Function .onInit

  # check if the same Avogadro version is already installed
  ReadRegStr $0 SHCTX "${PRODUCT_UNINST_KEY}" "Publisher"
  ${if} $0 != ""
    MessageBox MB_OK|MB_ICONSTOP "$(StillInstalled)"
    Abort
  ${endif}
  
  InitPluginsDir
  # If the user does *not* have administrator privileges, abort
  StrCpy $Answer ""
  StrCpy $UserName ""
  !insertmacro IsUserAdmin $Answer $UserName # macro from LyXUtils.nsh
  ${if} $Answer == "yes"
    # set shell variables for all user
    # set that e.g. shortcuts will be created for all users
    SetShellVarContext all 
  ${else}
    # set shell variables for current user only
    SetShellVarContext current
    # and install to a writable directory
    StrCpy $INSTDIR "$APPDATA\$(^Name)"
  ${endif}

FunctionEnd


# Uninstaller functions
Function un.onInit

  # If the user does *not* have administrator privileges, abort
  StrCpy $Answer ""
  !insertmacro IsUserAdmin $Answer $UserName
  ${if} $Answer == "yes"
   SetShellVarContext all
  ${else}
   # check if the Avogadro has been installed with admin permisions
   ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "Publisher"
   ${if} $0 != ""
    MessageBox MB_OK|MB_ICONSTOP "$(UnNotAdminLabel)"
    Abort
   ${endif}
   SetShellVarContext current
  ${endif}
  
  # ask if it should really be removed
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(UnReallyRemoveLabel)" IDYES +2
  Abort

FunctionEnd

Function un.onUninstSuccess

  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(UnRemoveSuccessLabel)"

FunctionEnd
