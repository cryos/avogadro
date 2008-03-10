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
#include <QGLFormat>
#include <QDebug>
#include <QTimer>
#include <QSplashScreen>
#include <QtGui/QPixmap>

#include <iostream>

#include <avogadro/global.h>

// Avogadro Includes
#include "mainwindow.h"
#include "application.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
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
#ifdef Q_WS_X11
  if(Library::threadedGL())
  {
    std::cout << "Enabling Threads\n";
    XInitThreads();
  }
#endif

  // set up groups for QSettings
  QCoreApplication::setOrganizationName("SourceForge");
  QCoreApplication::setOrganizationDomain("sourceforge.net");
  QCoreApplication::setApplicationName("Avogadro");

  Application app(argc, argv);

  if (!QGLFormat::hasOpenGL()) {
    QMessageBox::information(0, "Avogadro",
        "This system does not support OpenGL.");
    return -1;
  }
  std::cout << "System has OpenGL support." << std::endl;

  QString translationCode = QLocale::system().name();
  QString prefixPath = QString( INSTALL_PREFIX ) + "/share/avogadro/i18n/";

  QTranslator qtTranslator(0);
  QTranslator avoTranslator(0);
  QString avoFilename = "avogadro_" + translationCode + ".qm";
  QString qtFilename = "qt_" + translationCode;

  qDebug() << "Locale: " << translationCode;
  QTranslator *libTranslator;
  if((libTranslator = Library::createTranslator()))
  {
    qDebug() << "Loading LibAvogadro Translations";
    app.installTranslator(libTranslator);
  }

  if (qtTranslator.load(qtFilename))
  {
    qDebug() << "Loading QT Translations";
    app.installTranslator(&qtTranslator);
  }

  if (avoTranslator.load(avoFilename, prefixPath ))
  {
    qDebug() << "Loading Avogadro Translations";
    app.installTranslator(&avoTranslator);
  }

  // Extra debug messages to check out where some init segfaults are happening
  std::cout << "Loaded translations (if needed) about to test OpenGL capabilities."
            << std::endl;
  // use multi-sample (anti-aliased) OpenGL if available
  QGLFormat defFormat = QGLFormat::defaultFormat();
  defFormat.setSampleBuffers(true);
  QGLFormat::setDefaultFormat(defFormat);

  // Test what capabilities we have
  std::cout << "OpenGL capabilities found: " << std::endl;
  if (defFormat.doubleBuffer())
    std::cout << "\tDouble Buffering." << std::endl;
  if (defFormat.directRendering())
    std::cout << "\tDirect Rendering." << std::endl;
  if (defFormat.sampleBuffers())
    std::cout << "\tAntialiasing." << std::endl << std::endl;

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

  MainWindow *window = new MainWindow();
  if (arguments.size() > 1) {
    QPoint p(100, 100), offset(40,40);
    QList<QString>::const_iterator i = arguments.constBegin();
    for (++i; i != arguments.constEnd(); ++i)
    {
      window->openFile(*i);
      // this costs us a few more function calls
      // but makes our loading look nicer
      window->show();
      app.processEvents();
    }
  }
  window->show();
  return app.exec();
}

void printVersion(const QString &)
{
  std::wcout << QObject::tr("Avogadro: \t%1 (rev %2)\n"
      "LibAvogadro: \t%3 (rev %4)\n"
      "Qt: \t\t%5\n").arg(VERSION, SVN_REVISION, Library::version(), Library::svnRevision(), qVersion()).toStdWString();
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
