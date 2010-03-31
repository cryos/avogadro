#include <QtGui/QApplication>
#include <QtGui/QMainWindow>

#include <avogadro/periodictableview.h>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  QMainWindow *window = new QMainWindow();
  Avogadro::PeriodicTableView* periodicTable = new Avogadro::PeriodicTableView;
  window->setCentralWidget(periodicTable);
  window->show();    
  return app.exec();
}
