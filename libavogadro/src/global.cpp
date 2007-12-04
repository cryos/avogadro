/**********************************************************************
  main.cpp - Global library functions

  Copyright (C) 2007 by Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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
#include <config.h>

#include <QLocale>

namespace Avogadro
{
  QTranslator* Library::createTranslator()
  {
    QString translationCode = QLocale::system().name();
    QString prefixPath = QString( INSTALL_PREFIX ) + "/share/libavogadro/i18n/";
    QString fileName = "avogadro_" + translationCode + ".qm";
    
    QTranslator *translator = new QTranslator(0);
    
    if (translator->load(fileName, prefixPath ))
    {
      return translator;
    }
    delete translator;
    return 0;
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

}
