#include <QApplication>

#include <QMessageBox>
#include <QInputDialog>

#include "breakpaddialog.h"
#include "client/windows/handler/exception_handler.h"

class breakpaddialog : public QWidget
{
public:
	bool ask(void* context, EXCEPTION_POINTERS* exinfo,
		MDRawAssertionInfo* assertion)
	{

		switch (QMessageBox::question(
			this,
			tr("Avogadro"),
			tr("Avogadro has encountered an error and must close.\n Would you like to send an error report to tell us about this problem?"),

			QMessageBox::Yes |
			QMessageBox::No |
			QMessageBox::Cancel,

			QMessageBox::Cancel))
		{
		case QMessageBox::Yes:
			return true;
			break;
		case QMessageBox::No:
			return false;
			break;
		case QMessageBox::Cancel:
			return false;
			break;
		default:
			return false;
			break;
		}
	}
};


