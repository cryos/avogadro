/**********************************************************************
  DockExtension - interface for extensions which provide docks

  Copyright (C) 2010 Konstantin Tokarev

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

#ifndef DOCKEXTENSION_H
#define DOCKEXTENSION_H

#include "extension.h"

namespace Avogadro {
  class A_EXPORT DockExtension : public Extension
  {
      
    Q_OBJECT

   public:
    /**
     * Constructor.
     */
    DockExtension(QObject *parent = 0);

    /**
     * Destructor.
     */
    virtual ~DockExtension();

    // This methods are not needed for docks
    virtual QList<QAction *> actions() const;
    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
        
    /**
     * @return dock widget associated with this extensions
     */
    virtual QDockWidget * dockWidget() = 0;
    /**
     * @return default DockWidgetArea for dock
     */
    virtual Qt::DockWidgetArea preferredDockArea() const;
    /**
     * @return Determines the ordering of the docks
     */
    virtual int dockOrder() const;
  };
  
}

#endif
