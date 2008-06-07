!ifndef _Avogadro_LANGUAGES_ENGLISH_NSH_
!define _Avogadro_LANGUAGES_ENGLISH_NSH_

!ifdef Avogadro_LANG
  !undef Avogadro_LANG
!endif
!define Avogadro_LANG ${LANG_ENGLISH}

LicenseLangString AvogadroLicenseData ${Avogadro_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${Avogadro_LANG}" "This wizard will guide you through the installation of Avogadro.\r\n\
					     \r\n\
					     $_CLICK"

LangString SecAssociateCmlTitle "${Avogadro_LANG}" "Associate Avogadro-files"
LangString SecDesktopTitle "${Avogadro_LANG}" "Desktop icon"

LangString SecAssociateCmlDescription "${Avogadro_LANG}" "Files with the extension .cml will automatically be opened with Avogadro."
LangString SecDesktopDescription "${Avogadro_LANG}" "Puts Avogadro icon on the desktop."

LangString StillInstalled "${Avogadro_LANG}" "Avogadro ${Version} is already installed!"

LangString FinishPageMessage "${Avogadro_LANG}" "Congratulations! Avogadro has been installed successfully."
LangString FinishPageRun "${Avogadro_LANG}" "Launch Avogadro"

LangString UnNotAdminLabel "${Avogadro_LANG}" "You must have administrator privileges to uninstall Avogadro!"
LangString UnReallyRemoveLabel "${Avogadro_LANG}" "Are you sure you want to completely remove Avogadro?"
LangString UnRemoveSuccessLabel "${Avogadro_LANG}" "Avogadro was successfully removed from your computer."


!undef Avogadro_LANG

!endif ; _Avogadro_LANGUAGES_ENGLISH_NSH_
