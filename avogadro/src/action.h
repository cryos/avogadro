/**********************************************************************
  action - Class Description

  Copyright (C) 2007  <>

  This file is part of the $PROJECT$.
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
#ifndef AVOGADROACTION_H
#define AVOGADROACTION_H

#include <QAction>

namespace Avogadro
{

  /**
   @author
  */
  class Action : public QAction
  {
      Q_OBJECT;

    public:
      Action( QObject * parent );
      Action( const QString & text, QObject * parent );
      Action( const QIcon & icon, const QString & text, QObject * parent );

      ~Action();

      /** @return a menu path for this action
       *
       * A "menu path" specifies the menu and any submenus where
       * actions will be installed. Submenus are separated by ">" marks.
       * For example:
       * "Tools"  = all actions will be installed under the "Tools" menu. (default)
       * "Tools>Molecular Mechanics" = all actions will be installed
       *   into a submenu "Molecular Mechanics" of the "Tools" menu.
       *
       * If the menu or submenu name does not exist, it will be created.
       * If you wish to use an existing menu, make sure the path matches exactly.
       * For example: "&Tools" not "Tools"
       * this allows each action to exist in it's own menu.
       */
      QString menuPath() const;
      void setMenuPath( const QString& menuPath );

    private:
      QString m_menuPath;

  };

}

#endif
