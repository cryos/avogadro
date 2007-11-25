/**********************************************************************
  ToolGroup - GLWidget manager for Tools.

  Copyright (C) 2007 Donald Ephraim Curtis

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

#ifndef __TOOLGROUP_H
#define __TOOLGROUP_H

#include <QObject>
#include <avogadro/tool.h>

class QActionGroup;

namespace Avogadro {
  class Molecule;
  /**
   * @class ToolGroup
   * @brief Manager for GLWidget Tools
   * @author Donald Ephraim Curtis
   *
   * This class is a collection of Tools which allow manipulation of the 
   * GLWidget area.
   */

  class ToolGroupPrivate;
  class A_EXPORT ToolGroup : public QObject
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       */
      ToolGroup(QObject *parent = 0);

      /**
       * Deconstructor
       */
      ~ToolGroup();

      /**
       * Search and load plugins from the following paths:
       *    /usr/lib/avogadro/tools
       *    /usr/local/lib/avogadro/tools
       *
       * You can set the AVOGADRO_PATH to designate a path
       * at runtime.
       */
      void load();

      /**
       * @return the active tool
       */
      Tool* activeTool() const;

      /**
       * @param i index of the tool to return
       * @return tool at index i
       */
      Tool* tool(int i) const;

      /**
       * @return constant list of the tools
       */
      const QList<Tool *>& tools() const;

      /**
       * @return constant QActionGroup of all the tool select actions
       */
      const QActionGroup * activateActions() const;

    public Q_SLOTS:
      /**
       * @param i index of the tool to set active
       */
      void setActiveTool(int i);

      /**
      * @param name the name of the tool to set active (if it's found)
      */
      void setActiveTool(const QString& name);
    
      /** 
       * @param tool pointer to the tool to set active
       */
      void setActiveTool(Tool *tool);
      
      /**
       * @param molecule pointer to the molecule tools in this group should use
       */
      void setMolecule(Molecule *molecule);

    private Q_SLOTS:
      void activateTool();

    Q_SIGNALS:
      /**
       * @param the activated tool
       */
      void toolActivated(Tool *tool);

    private:
      ToolGroupPrivate * const d;
  };
} // end namespace Avogadro

#endif
