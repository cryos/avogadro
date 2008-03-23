/**********************************************************************
  Extension - Extension Class Interface

  Copyright (C) 2007-2008 Donald Ephraim Curtis

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

#include "extension.h"

namespace Avogadro {

  QString Extension::name() const
  { 
    return QObject::tr("Unknown"); 
  }

  QString Extension::description() const
  { 
    return QObject::tr("Unknown Extension"); 
  }

  QString Extension::menuPath(QAction *action) const 
  { 
    Q_UNUSED(action); return QString(); 
  }

  QDockWidget * Extension::dockWidget() 
  { 
    return 0; 
  }

<<<<<<< HEAD:libavogadro/src/extension.cpp
  void Extension::setMolecule(Molecule *molecule)
  {
    Q_UNUSED(molecule);
  }
=======
>>>>>>> Move extensions to libavogadro:libavogadro/src/extension.cpp

}

#include "extension.moc"
