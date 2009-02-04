/**********************************************************************
  main.cpp - Global library functions

  Copyright (C) 2007 by Donald Ephraim Curtis
  Copyright (C) 2009 Marcus D. Hanwell

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

#include "global.h"
#include "config.h"

#include <QLocale>
#include <QDebug>
#include <QCoreApplication>

namespace Avogadro
{
  QTranslator* Library::createTranslator()
  {
    QString translationCode = QLocale::system().name();
    #ifdef WIN32
      QString prefixPath = QCoreApplication::applicationDirPath() + "/i18n/libavogadro/";
    #else
      QString prefixPath = QString( INSTALL_PREFIX ) + "/share/libavogadro/i18n/";
    #endif
    QString fileName = "avogadro_" + translationCode + ".qm";

    QTranslator *translator = new QTranslator(0);

    if (translator->load(fileName, prefixPath )) {
      return translator;
    }
    else {
      qDebug() << prefixPath + fileName << "not found.";
      delete translator;
      return 0;
    }
  }

  QString Library::version()
  {
    return VERSION;
  }

  QString Library::svnRevision()
  {
    return SVN_REVISION;
  }

  QString Library::prefix()
  {
    return INSTALL_PREFIX;
  }

  bool Library::threadedGL()
  {
    return THREADED_GL;
  }

}

