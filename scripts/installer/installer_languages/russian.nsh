!ifndef _Avogadro_LANGUAGES_ENGLISH_NSH_
!define _Avogadro_LANGUAGES_ENGLISH_NSH_

!ifdef Avogadro_LANG
  !undef Avogadro_LANG
!endif
!define Avogadro_LANG ${LANG_RUSSIAN}

LicenseLangString AvogadroLicenseData ${Avogadro_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${Avogadro_LANG}" "This wizard will guide you through the installation of Avogadro.\r\n\
					     \r\n\
					     $_CLICK"

LangString SecAssociateCmlTitle "${Avogadro_LANG}" "Associate Avogadro-files"
LangString SecDesktopTitle "${Avogadro_LANG}" "Ярлык на рабочем столе"

LangString SecAssociateCmlDescription "${Avogadro_LANG}" "Files with the extension .cml will automatically be opened with Avogadro."
LangString SecDesktopDescription "${Avogadro_LANG}" "Размещает ярлык Avogadro на рабочем столе."

LangString StillInstalled "${Avogadro_LANG}" "Avogadro ${Version} уже установлен!"

LangString FinishPageMessage "${Avogadro_LANG}" "Поздравляем! Avogadro был успешно установлен."
LangString FinishPageRun "${Avogadro_LANG}" "Запустить Avogadro"

LangString UnNotAdminLabel "${Avogadro_LANG}" "You must have administrator privileges to uninstall Avogadro!"
LangString UnReallyRemoveLabel "${Avogadro_LANG}" "Вы уверены, что хотите полностью удалить Avogadro?"
LangString UnRemoveSuccessLabel "${Avogadro_LANG}" "Avogadro был успешно удален с вашего компьютера."


!undef Avogadro_LANG

!endif ; _Avogadro_LANGUAGES_ENGLISH_NSH_
