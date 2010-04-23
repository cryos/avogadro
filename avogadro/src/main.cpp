/**********************************************************************
  main.cpp - main program, initialization and launching

  Copyright (C) 2006-2009 by Geoffrey R. Hutchison
  Copyright (C) 2006-2008 by Donald Ephraim Curtis
  Copyright (C) 2008-2009 by Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include <avogadro/global.h>
#include <openbabel/babelconfig.h>

#ifdef ENABLE_GLSL
  #include <GL/glew.h>
#endif

// Qt Includes
#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QSystemLocale>
#include <QGLFormat>
#include <QDebug>
#include <QLibraryInfo>
#include <QProcess>

#include <iostream>

// get the SVN revision string
#include "config.h" // krazy:exclude=includes

// Avogadro Includes
#include "mainwindow.h"
#include "application.h"

#ifdef Q_WS_X11
  #include <X11/Xlib.h>
#endif

#ifdef WIN32
  #include <stdlib.h>
#endif

#ifdef AVO_APP_BUNDLE
  #include <cstdlib>
#endif

using namespace Avogadro;

void printVersion(const QString &appName);
void printHelp(const QString &appName);

int main(int argc, char *argv[])
{
#ifdef Q_WS_X11
  if(Library::threadedGL()) {
    std::cout << "Enabling Threads" << std::endl;
    XInitThreads();
  }
#endif

  // set up groups for QSettings
  QCoreApplication::setOrganizationName("SourceForge");
  QCoreApplication::setOrganizationDomain("sourceforge.net");
  QCoreApplication::setApplicationName("Avogadro");

  Application app(argc, argv);

  // Output the untranslated application and library version - bug reports
  QString versionInfo = "Avogadro version:\t" + QString(VERSION) + "\tGit:\t"
                        + QString(SCM_REVISION) + "\nLibAvogadro version:\t"
                        + Library::version() + "\tGit:\t" + Library::scmRevision();
  qDebug() << versionInfo;

#ifdef WIN32
#ifndef AVO_APP_BUNDLE
  // Need to add an environment variable to the current process in order
  // to load the forcefield parameters in OpenBabel.
  QString babelDataDir = "BABEL_DATADIR=" + QCoreApplication::applicationDirPath();
  qDebug() << babelDataDir;
  _putenv(babelDataDir.toStdString().c_str());
#endif
#endif

#ifdef AVO_APP_BUNDLE
  // Set up the babel data and plugin directories for Mac - relocatable
  // This also works for the Windows package, but BABEL_LIBDIR is ignored

  // Make sure to enclose the environment variable in quotes, or spaces will cause problems
  QString escapedAppPath = QCoreApplication::applicationDirPath().replace(' ', "\ ");
  QByteArray babelDataDir((QCoreApplication::applicationDirPath()
                          + "/../share/openbabel/"
                           + BABEL_VERSION).toAscii());
  QByteArray babelLibDir((QCoreApplication::applicationDirPath()
                         + "/../lib/openbabel").toAscii());
  int res1 = setenv("BABEL_DATADIR", babelDataDir.data(), 1);
  int res2 = setenv("BABEL_LIBDIR", babelLibDir.data(), 1);

  qDebug() << "BABEL_LIBDIR" << babelLibDir.data();

  if (res1 != 0 || res2 != 0)
    qDebug() << "Error: setenv failed." << res1 << res2;

  // Override the Qt plugin search path too
  QStringList pluginSearchPaths;
  pluginSearchPaths << QCoreApplication::applicationDirPath() + "/../plugins";
  QCoreApplication::setLibraryPaths(pluginSearchPaths);
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

  translationPaths << QCoreApplication::applicationDirPath() + "/../share/avogadro/i18n/";
#ifdef Q_WS_MAC
  translationPaths << QString(INSTALL_PREFIX) + "/share/avogadro/i18n/";
#endif

  // Get the locale for translations
  QString translationCode = QLocale::system().name();

  // The QLocale::system() call on Mac doesn't reflect the default language -- only the default locale formatting
  // so we'll fine-tune the respone with QSystemLocale
  // This only applies to Qt/Mac 4.6.x and later, which added the appropriate Carbon magic to QSystemLocale.
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
#ifdef Q_WS_MAC
  QSystemLocale sysLocale;
  QLocale::Language sysLanguage = static_cast<QLocale::Language>(sysLocale.query(QSystemLocale::LanguageId, QVariant()).toInt());
  QLocale::Country sysCountry = static_cast<QLocale::Country>(sysLocale.query(QSystemLocale::CountryId, QVariant()).toInt());
  QLocale macSystemPrefsLanguage(sysLanguage, sysCountry);
  translationCode = macSystemPrefsLanguage.name();
#endif
#endif

  qDebug() << "Locale: " << translationCode;

  // Load Qt translations first
  bool tryLoadingQtTranslations = false;
  QString qtFilename = "qt_" + translationCode + ".qm";
  QTranslator qtTranslator(0);
  if (qtTranslator.load(qtFilename, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    app.installTranslator(&qtTranslator);
  else
    tryLoadingQtTranslations = true;

  // Load the libavogadro translations
  QPointer <QTranslator> libTranslator = Library::createTranslator();
  if (libTranslator)
    app.installTranslator(libTranslator);

  // Load the Avogadro translations
  QTranslator avoTranslator(0);
  QString avoFilename = "avogadro_" + translationCode + ".qm";

  foreach (const QString &translationPath, translationPaths) {
    // We can't find the normal Qt translations (maybe we're in a "bundle"?)
    if (tryLoadingQtTranslations) {
      if (qtTranslator.load(qtFilename, translationPath)) {
        app.installTranslator(&qtTranslator);
        tryLoadingQtTranslations = false; // already loaded
      }
    }

    if (avoTranslator.load(avoFilename, translationPath)) {
      app.installTranslator(&avoTranslator);
      qDebug() << "Translation successfully loaded.";
    }
  }

  // Check if we just need a version or help message
  QStringList arguments = app.arguments();
  if(arguments.contains("-v") || arguments.contains("--version")) {
    printVersion(arguments[0]);
    return 0;
  }
  else if(arguments.contains("-h") || arguments.contains("-help") 
  	|| arguments.contains("--help")) {
    printHelp(arguments[0]);
    return 0;
  }

  if (!QGLFormat::hasOpenGL()) {
  //  QMessageBox::information(0, QCoreApplication::translate("main.cpp", "Avogadro"),
  //      QCoreApplication::translate("main.cpp", "This system does not support OpenGL."));
      QMessageBox::information(0, "Avogadro", "This system does not support OpenGL.");
    return -1;
  }
  qDebug() << /*QCoreApplication::translate("main.cpp", */"System has OpenGL support."/*)*/;

  // Extra debug messages to check out where some init segfaults are happening
  qDebug() << /*QCoreApplication::translate("main.cpp", */"About to test OpenGL capabilities."/*)*/;
  // use multi-sample (anti-aliased) OpenGL if available
  QGLFormat defFormat = QGLFormat::defaultFormat();
  defFormat.setSampleBuffers(true);
  QGLFormat::setDefaultFormat(defFormat);

  // Test what capabilities we have
  //qDebug() << /*QCoreApplication::translate("main.cpp", */"OpenGL capabilities found: "/*)*/;
  std::cout << "OpenGL capabilities found: " << std::endl;
  if (defFormat.doubleBuffer())
    std::cout << "\t" << "Double Buffering." << std::endl;
  if (defFormat.directRendering())
    std::cout << "\t" << "Direct Rendering." << std::endl;
  if (defFormat.sampleBuffers())
    std::cout << "\t" << "Antialiasing." << std::endl;

  // Now load any files supplied on the command-line or via launching a file
  MainWindow *window = new MainWindow();
  if (arguments.size() > 1) {
    QPoint p(100, 100), offset(40,40);
    QList<QString>::const_iterator i = arguments.constBegin();
    for (++i; i != arguments.constEnd(); ++i) {
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
  std::cout << "Avogadro: " << VERSION << std::endl;
  std::cout << "Qt: \t\t" << qVersion() << std::endl;
  #else
  std::wcout << QCoreApplication::translate("main.cpp", "Avogadro: \t%1 (Hash %2)\n"
      "LibAvogadro: \t%3 (Hash %4)\n"
      "Qt: \t\t%5\n").arg(VERSION, SCM_REVISION, Library::version(), Library::scmRevision(), qVersion()).toStdWString();
  std::wcout << "OpenBabel: \t" << BABEL_VERSION << std::endl;
  #endif
}

void printHelp(const QString &appName)
{
  #ifdef WIN32
  std::cout << "Usage: avogadro [options] [files]" << std::endl << std::endl;
  std::cout << "Avogadro - Advanced Molecular Editor (version " << VERSION << ')' << std::endl << std::endl;
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
