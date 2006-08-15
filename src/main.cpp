
// QT Includes
#include <QApplication>

// OB Includes
#include <openbabel/mol.h>

// Avogadro Includes
#include "AvGLWidget.h"
#include "AvMainWindow.h"

using namespace OpenBabel;
using namespace Avogadro;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow window;
  window.show();
	return app.exec();
}
