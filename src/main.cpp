
// QT Includes
#include <QApplication>
#include <QFont>
#include <QPushButton>
#include <QWidget>
#include <QtGui>

// OB Includes
#include <openbabel/mol.h>

#include "AGLWidget.h"
#include "AMainWindow.h"
#include "ui_avogadro.h"

using namespace OpenBabel;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	AMainWindow window;
  //  Ui_MainWindow form;
  //  form.setupUi(&window);
  window.show();
  //  app.setStyle(QStyleFactory::create("Windows"));
	return app.exec();
}
