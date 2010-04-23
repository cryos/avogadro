/**********************************************************************
  global.cpp - Global library functions

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
// The Krazy checker doesn't like this, but it's correct
// krazy:excludeall=includes
#include "config.h"

#include <QLocale>
#include <QDebug>
#include <QCoreApplication>
#include <QStringList>
#include <QProcess>

namespace Avogadro
{
  QPointer<QTranslator> QPeriodicTable::createTranslator()
  {
    QString translationCode = QLocale::system().name();
    QStringList translationPaths;
    
    foreach (const QString &variable, QProcess::systemEnvironment()) {
      QStringList split1 = variable.split('=');
      if (split1[0] == "QPERIODICTABLE_TRANSLATIONS") {
        foreach (const QString &path, split1[1].split(':'))
          translationPaths << path;
      }
    }
    
    translationPaths << QCoreApplication::applicationDirPath() + "/../share/avogadro/i18n/";
#ifdef Q_WS_MAC
    translationPaths << QString(INSTALL_PREFIX) + "/share/avogadro/i18n/";
#endif
    
    QString fileName = "libqperiodictable_" + translationCode + ".qm";
  
    // Load the translations
    QPointer<QTranslator> translator = new QTranslator(0);
    foreach (const QString &translationPath, translationPaths) {
      if (translator->load(fileName, translationPath)) {
        return translator;
      }
    }

    qDebug() << "QPeriodicTable translations not found.";
    delete translator;
    return 0;
  }
}

