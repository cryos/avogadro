!ifndef _Avogadro_LANGUAGES_GERMAN_NSH_
!define _Avogadro_LANGUAGES_GERMAN_NSH_

!ifdef Avogadro_LANG
  !undef Avogadro_LANG
!endif
!define Avogadro_LANG ${LANG_GERMAN}

LicenseLangString AvogadroLicenseData ${Avogadro_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${Avogadro_LANG}" "Dieser Assistent wird Sie durch die Installation von Avogadro begleiten.\r\n\
					     \r\n\
					     $_CLICK"

LangString SecAssociateCmlTitle "${Avogadro_LANG}" "Dateizuordnung für Avogadro-Dateien"
LangString SecDesktopTitle "${Avogadro_LANG}" "Desktopsymbol"

LangString SecAssociateCmlDescription "${Avogadro_LANG}" "Dateien mit der Endung .cml werden automatisch mit Avogadro geöffnet."
LangString SecDesktopDescription "${Avogadro_LANG}" "Erstellt Verknüpfung zu Avogadro auf dem Desktop."

LangString StillInstalled "${Avogadro_LANG}" "Avogadro ${Version} ist bereits installiert!"

LangString FinishPageMessage "${Avogadro_LANG}" "Glückwunsch! Avogadro wurde erfolgreich installiert."
LangString FinishPageRun "${Avogadro_LANG}" "Avogadro starten"

LangString UnNotAdminLabel "${Avogadro_LANG}" "Sie benötigen Administratorrechte um Avogadro zu deinstallieren!"
LangString UnReallyRemoveLabel "${Avogadro_LANG}" "Sind Sie sicher, dass sie Avogadro deinstallieren möchten?"
LangString UnRemoveSuccessLabel "${Avogadro_LANG}" "Avogadro wurde erfolgreich von ihrem Computer entfernt."


!undef Avogadro_LANG

!endif ; _Avogadro_LANGUAGES_GERMAN_NSH_
