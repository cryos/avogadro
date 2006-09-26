/**********************************************************************
  main.cpp - main program, initialization and launching

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

// QT Includes
#include <QApplication>
#include <QMessageBox>

// Avogadro Includes
#include "MainWindow.h"

using namespace Avogadro;

// Import static plugins
// Q_IMPORT_PLUGIN(BSEngine)
// Q_IMPORT_PLUGIN(WireframeEngine)

int main(int argc, char *argv[])
{
  // set up groups for QSettings
  QCoreApplication::setOrganizationName("SourceForge");
  QCoreApplication::setOrganizationDomain("avogadro.sourceforge.net");
  QCoreApplication::setApplicationName("Avogadro");

  QApplication app(argc, argv);
  if (!QGLFormat::hasOpenGL()) {
    QMessageBox::information(0, "Avogadro",
        "This system does not support OpenGL.");
    return -1;
  }
  //  app.setQuitOnLastWindowClosed(false); // remain open until quit()

  QStringList files = app.arguments();

  if (files.size() > 1) {
    QPoint p(100, 100), offset(40,40);
    QList<QString>::const_iterator i = files.constBegin();
    for (++i; i != files.constEnd(); ++i)
    {
      MainWindow *other = new MainWindow;
      p += offset;
      other->move(p);
      other->show();
      other->loadFile(*i);
    }
  } else {
    MainWindow *window = new MainWindow;
    window->show();
  }
  return app.exec();
}
