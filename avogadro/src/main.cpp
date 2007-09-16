/**********************************************************************
  main.cpp - main program, initialization and launching

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Donald Ephraim Curtis

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
#include "mainwindow.h"
#include "application.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

// get the SVN revision string
#include<svn.h>

using namespace Avogadro;

// Import static plugins
// Q_IMPORT_PLUGIN(BSEngine)
// Q_IMPORT_PLUGIN(WireframeEngine)

int main(int argc, char *argv[])
{
  qDebug() << "This is Avogadro, SVN revision" << SVN_REVISION_STRING;

  // set up groups for QSettings
  QCoreApplication::setOrganizationName("SourceForge");
  QCoreApplication::setOrganizationDomain("sourceforge.net");
  QCoreApplication::setApplicationName("Avogadro");

#ifdef ENABLE_THREADED_GL
#ifdef Q_WS_X11
  XInitThreads();
#endif
#endif

  Application app(argc, argv);
  if (!QGLFormat::hasOpenGL()) {
    QMessageBox::information(0, "Avogadro",
        "This system does not support OpenGL.");
    return -1;
  }

  // use multi-sample (anti-aliased) OpenGL if available
  QGLFormat defFormat = QGLFormat::defaultFormat();
  defFormat.setSampleBuffers(true);
  QGLFormat::setDefaultFormat(defFormat);

  QStringList files = app.arguments();

  qDebug() << files;
  if (files.size() > 1) {
    QPoint p(100, 100), offset(40,40);
//     foreach(QString file, files)
    QList<QString>::const_iterator i = files.constBegin();
    for (++i; i != files.constEnd(); ++i)
    {
      MainWindow *other = new MainWindow;
      p += offset;
      other->move(p);
      other->loadFile(*i);
      other->show();
    }
  } else {
    MainWindow *window = new MainWindow;
    window->show();
  }
  return app.exec();
}
