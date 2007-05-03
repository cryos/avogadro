/**********************************************************************
  Application - main application events

  Copyright (C) 2007 by Geoffrey R. Hutchison

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

#include "application.h"
#include "mainwindow.h"

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  Application::Application(int &argc, char **argv): QApplication(argc, argv)
  {  }

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
    // need to make this spawn a new instance
    MainWindow *other = new MainWindow;
    other->show();
    other->setFile(fileName);
  }

} // end namespace Avogadro

#include "application.moc"
