/**********************************************************************
  ToolGroup - GLWidget manager for Tools.

  Copyright (C) 2007,2008 Donald Ephraim Curtis

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

#ifndef TOOLGROUP_H
#define TOOLGROUP_H

#include <QObject>
#include <avogadro/plugin.h>
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
       * Append the @p tools to the toolgroup.
       */
      void append(QList<Tool *> tools);
      
      /**
       * Append the @p tool to the toolgroup.
       */
      void append(Tool *tool);

      /**
       * @return The active tool.
       */
      Tool* activeTool() const;

      /**
       * @param i index of the tool to return
       * @return The tool at index i.
       */
      Tool* tool(int i) const;

      /**
       * @return Constant list of the tools.
       */
      const QList<Tool *>& tools() const;

      /**
       * @return constant QActionGroup of all the tool select actions.
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

      /**
       * Write the settings of the GLWidget in order to save them to disk.
       */
      void writeSettings(QSettings &settings) const;

      /**
       * Read the settings of the GLWidget and restore them.
       */
      void readSettings(QSettings &settings);
      /**
       * Reset the toolgroup to it's original state.
       */
      void removeAllTools();

    private Q_SLOTS:
      void activateTool();

    Q_SIGNALS:
      /**
       * @param tool the activated tool
       */
      void toolActivated(Tool *tool);
      /**
       * This signal is emitted when one or more tools are destoyed. 
       * (Happens when plugins are reloaded)
       */
      void toolsDestroyed();

    private:
      ToolGroupPrivate * const d;
  };
} // end namespace Avogadro

#endif
