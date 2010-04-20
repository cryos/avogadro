/**********************************************************************
 QPeriodicTable - Periodic Table based on Avogadro

 Copyright (C) 2010 by Konstantin Tokarev

 This file is part of the Avogadro molecular editor project.
 For more information, see <http://avogadro.openmolecules.net/>

 Avogadro is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
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

#ifndef QPERIODICTABLE_H
#define QPERIODICTABLE_H

#include <QtGui/QPlainTextEdit>
#include "periodictableview.h"

class PeriodicTableWatcher : public QObject
{
  Q_OBJECT

  public:
  PeriodicTableWatcher(Avogadro::PeriodicTableView *periodicTable,
                         QPlainTextEdit *elementInfo);
    
  public slots:
    void elementChanged(int);

  private:
    Avogadro::PeriodicTableView *m_periodicTable;
    QPlainTextEdit *m_elementInfo;

};

#endif
