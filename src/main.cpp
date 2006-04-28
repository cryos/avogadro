#include <QApplication>
#include <QPushButton>

int main(int argc, char ** argv)
{
	QApplication app(argc, argv);
	printf("Starting Avogadro.\n");

	QPushButton hello("Hello Avogadro");
	hello.resize(100,30);
	hello.show();

	return app.exec();
}
