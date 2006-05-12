
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

using namespace OpenBabel;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	OBMol *mol = new OBMol();
	OBAtom *atom; 
	atom = mol->CreateAtom();
	atom->SetAtomicNum(5);
	atom->SetVector(5.0,7.0,2.0);

	printf("x:%f y:%f z:%f\n", atom->x(), atom->y(), atom->z());

	AMainWindow window;
	window.show();
	app.setStyle(QStyleFactory::create("Windows"));
	return app.exec();
}
