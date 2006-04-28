#include "AWindow.h"
#include <QApplication>

int main(int argc, char ** argv)
{
	QApplication app(argc, argv);
	printf("Starting Avogadro.\n");

	return app.exec();
}
