/**********************************************************************
  Application - main application events

  Copyright (C) 2007 by Geoffrey R. Hutchison

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

#include "application.h"
#include "mainwindow.h"

#ifdef ENABLE_PYTHON
#include <avogadro/pythonwhenidle.h>
#endif

// The Krazy checker doesn't like this, but it's correct
// krazy:excludeall=cpp
#ifdef Q_OS_UNIX
#  include "locale.h"
#endif // Q_OS_UNIX

using namespace std;

namespace Avogadro {

  Application::Application(int &argc, char **argv): QApplication(argc, argv)
  {
#ifdef Q_OS_UNIX
    // work around a bug in OpenBabel: the chemical data files parsing
    // is dependent on the LC_NUMERIC locale.
    // Note that similar code currently exists in Qt (as of 4.3.1) in
    // src/corelib/kernel/qcoreapplication.cpp, so the code here is
    // not currently needed, but the following link indicates that the
    // fix might be removed from Qt in a future version:
    // http://trolltech.com/developer/task-tracker/index_html?method=entry&id=132859
    // So we prefer to have this fix here preventively.
    setlocale(LC_NUMERIC, "C");
#endif // Q_OS_UNIX

#ifdef ENABLE_PYTHON
    PythonWhenIdle::initialize();
#endif
  }

  // Handle open events (e.g., Mac OS X open files)
  bool Application::event(QEvent *event)
  {
    switch (event->type()) {
      case QEvent::FileOpen:
        return loadFile(static_cast<QFileOpenEvent *>(event)->file());
      default:
        return QApplication::event(event);
    }
  }

  bool Application::loadFile(const QString &fileName)
  {
    if (fileName.isEmpty()) {
      return false;
    }

    // check to see if we already have an open window
    // (we'll let MainWindow handle the real work)
    MainWindow *window = NULL;
    foreach (QWidget *widget, topLevelWidgets()) {
      window = qobject_cast<MainWindow *>(widget);
      if (window)
        break;
    }

    // if not, need to make this spawn a new instance
    if (!window)
      window = new MainWindow;

    window->openFile(fileName);
    window->show();
    return true;
  }

} // end namespace Avogadro

#include "application.moc"
