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
#include <QTranslator>

#include <avogadro/global.h>

// Avogadro Includes
#include "mainwindow.h"
#include "application.h"

#ifdef ENABLE_THREADED_GL
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif
#endif

// get the SVN revision string
#include <config.h>

using namespace Avogadro;

void printVersion(const QString &appName);
void printHelp(const QString &appName);

// Import static plugins
// Q_IMPORT_PLUGIN(BSEngine)
// Q_IMPORT_PLUGIN(WireframeEngine)

int main(int argc, char *argv[])
{
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

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name());
  app.installTranslator(&qtTranslator);

  QTranslator avoTranslator;
  avoTranslator.load("avo_" + QLocale::system().name());
  app.installTranslator(&avoTranslator);



  // use multi-sample (anti-aliased) OpenGL if available
  QGLFormat defFormat = QGLFormat::defaultFormat();
  defFormat.setSampleBuffers(true);
  QGLFormat::setDefaultFormat(defFormat);

  QStringList arguments = app.arguments();

  if(arguments.contains("-v") || arguments.contains("--version"))
  {
    printVersion(arguments[0]);
    return 0;
  }
  else if(arguments.contains("-h") || arguments.contains("--help"))
  {
    printHelp(arguments[0]);
    return 0;
  }

  qDebug() << arguments;
  if (arguments.size() > 1) {
    QPoint p(100, 100), offset(40,40);
//     foreach(QString file, files)
    QList<QString>::const_iterator i = arguments.constBegin();
    for (++i; i != arguments.constEnd(); ++i)
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

void printVersion(const QString &)
{
  std::wcout << QObject::tr("Avogadro: \t%1 (rev %2)\n"
      "LibAvogadro: \t%3 (rev %4)\n"
      "Qt: \t\t%5\n").arg(VERSION, SVN_REVISION, libVersion(), libSvnRevision(), qVersion()).toStdWString();
}

void printHelp(const QString &appName)
{
  std::wcout << QObject::tr("Usage: %1 [options] [files]\n\n"
      "Advanced Molecular Editor (version %2)\n\n"
      "Options:\n"
      "  -h, --help\t\tShow help options (this)\n"
      "  -v, --version\t\tShow version information\n"
                           ).arg(appName, VERSION).toStdWString();
}
