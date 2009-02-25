/**********************************************************************
  main.cpp - main program, initialization and launching

  Copyright (C) 2006-2009 by Geoffrey R. Hutchison
  Copyright (C) 2006-2008 by Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include <avogadro/global.h>

#ifdef ENABLE_GLSL
  #include <GL/glew.h>
#endif

// QT Includes
#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QGLFormat>
#include <QDebug>
#include <QLibraryInfo>
#include <QProcess>

#include <iostream>

// get the SVN revision string
#include <config.h>

#include <avogadro/pluginmanager.h>

// Avogadro Includes
#include "mainwindow.h"
#include "application.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#ifdef WIN32
#include <stdlib.h>
#endif

using namespace Avogadro;

void printVersion(const QString &appName);
void printHelp(const QString &appName);

int main(int argc, char *argv[])
{
#ifdef Q_WS_X11
  if(Library::threadedGL())
  {
    std::cout << "Enabling Threads" << std::endl;
    XInitThreads();
  }
#endif

  // set up groups for QSettings
  QCoreApplication::setOrganizationName("SourceForge");
  QCoreApplication::setOrganizationDomain("sourceforge.net");
  QCoreApplication::setApplicationName("Avogadro");

  Application app(argc, argv);

#ifdef WIN32
  // Need to add an environment variable to the current process in order
  // to load the forcefield parameters in OpenBabel.
  QString babelDataDir = "BABEL_DATADIR=" + QCoreApplication::applicationDirPath();
  qDebug() << babelDataDir;
  _putenv(babelDataDir.toStdString().c_str());
#endif

  // Before we do much else, load translations
  // This ensures help messages and debugging info will be translated
  QStringList translationPaths;

  foreach (const QString &variable, QProcess::systemEnvironment()) {
    QStringList split1 = variable.split('=');
    if (split1[0] == "AVOGADRO_TRANSLATIONS") {
      foreach (const QString &path, split1[1].split(':'))
        translationPaths << path;
    }
  }

  QString translationCode = QLocale::system().name();
  QString prefixPath = QCoreApplication::applicationDirPath() + "/../share/avogadro/i18n/";
  translationPaths << prefixPath;

  qDebug() << "Locale: " << translationCode;
  // Load Qt translations first
  QString qtFilename = "qt_" + translationCode + ".qm";
  QTranslator qtTranslator(0);
  if (qtTranslator.load(qtFilename, QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
    app.installTranslator(&qtTranslator);
  }

  // Now load LibAvogadro and then Avogadro translations
  QTranslator *libTranslator;
  if((libTranslator = Library::createTranslator())) {
    app.installTranslator(libTranslator);
  }
  QTranslator avoTranslator(0);
  QString avoFilename = "avogadro_" + translationCode + ".qm";

  foreach (QString translationPath, translationPaths) {
    qDebug() << "path = " << translationPath;

    if (avoTranslator.load(avoFilename, translationPath)) {
      app.installTranslator(&avoTranslator);
      break;
    }
    else {
      qDebug() << translationPath + avoFilename << "not found.";
    }
  }

  // Check if we just need a version or help message
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
  
  if (!QGLFormat::hasOpenGL()) {
    QMessageBox::information(0, QCoreApplication::translate("main.cpp", "Avogadro"),
        QCoreApplication::translate("main.cpp", "This system does not support OpenGL."));
    return -1;
  }
  qDebug() << QCoreApplication::translate("main.cpp", "System has OpenGL support.");

  // Extra debug messages to check out where some init segfaults are happening
  qDebug() << QCoreApplication::translate("main.cpp", "About to test OpenGL capabilities.");
  // use multi-sample (anti-aliased) OpenGL if available
  QGLFormat defFormat = QGLFormat::defaultFormat();
  defFormat.setSampleBuffers(true);
  QGLFormat::setDefaultFormat(defFormat);

  // Test what capabilities we have
  qDebug() << QCoreApplication::translate("main.cpp", "OpenGL capabilities found: ");
  if (defFormat.doubleBuffer())
    qDebug() << "\t" << QCoreApplication::translate("main.cpp", "Double Buffering.");
  if (defFormat.directRendering())
    qDebug() << "\t" << QCoreApplication::translate("main.cpp", "Direct Rendering.");
  if (defFormat.sampleBuffers())
    qDebug() << "\t" << QCoreApplication::translate("main.cpp", "Antialiasing.");

  // Now load any files supplied on the command-line or via launching a file
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
  #ifdef WIN32
  std::cout << "Avogadro: 0.8.0" << std::endl;
  std::cout << "LibAvogadro: 0.8.0" << std::endl;
  std::cout << "Qt: \t\t4.3.4" << std::endl;
  #else
  std::wcout << QCoreApplication::translate("main.cpp", "Avogadro: \t%1 (rev %2)\n"
      "LibAvogadro: \t%3 (rev %4)\n"
      "Qt: \t\t%5\n").arg(VERSION, SVN_REVISION, Library::version(), Library::svnRevision(), qVersion()).toStdWString();
  #endif
}

void printHelp(const QString &appName)
{
  #ifdef WIN32
  std::cout << "Usage: avogadro [options] [files]" << std::endl << std::endl;
  std::cout << "Advanced Molecular Editor (version 0.8.0)" << std::endl << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -h, --help\t\tShow help options (this)" << std::endl;
  std::cout << "  -v, --version\t\tShow version information" << std::endl;
  #else
  std::wcout << QCoreApplication::translate("main.cpp", "Usage: %1 [options] [files]\n\n"
      "Advanced Molecular Editor (version %2)\n\n"
      "Options:\n"
      "  -h, --help\t\tShow help options (this)\n"
      "  -v, --version\t\tShow version information\n"
      ).arg(appName, VERSION).toStdWString();
  #endif
}
