/**********************************************************************
  Tool - Avogadro Tool Interface

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

#ifndef __TOOL_H
#define __TOOL_H

#include <avogadro/global.h>

#include <QWheelEvent>

class QAction;
class QUndoCommand;
class QWidget;

namespace Avogadro {

  class GLWidget;
  class Molecule;

  /**
   * @class Tool tool.h <avogadro/tool.h>
   * @brief Interface for tool plugins
   * @author Donald Ephraim Curtis
   *
   * This is a template class for tools which manipulate the GLWidget
   * area.  The functions they implement are in response to actions
   * performed by the user on the GLWidget.
   */
  class ToolPrivate;
  class A_EXPORT Tool : public QObject
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       */
      Tool(QObject *parent = 0);

      /**
       * Destructor
       */
      virtual ~Tool();

      /**
       * @return the name of the tool.
       */
      virtual QString name() const;

      /**
       * @return a description of the tool.
       */
      virtual QString description() const;

      /**
       * @return the QAction of the tool
       */
      virtual QAction* activateAction() const;

      /**
       * @return the settings widget for the tool.
       */
      virtual QWidget* settingsWidget();

      /**
       * Response to mouse press
       * @param widget the %GLWidget where the even occurred
       * @param event the mouse event information
       */
      virtual QUndoCommand* mousePress(GLWidget *widget, const QMouseEvent *event) = 0;

      /**
       * Response to mouse release
       * @param widget the %GLWidget where the even occurred
       * @param event the mouse event information
       */
      virtual QUndoCommand* mouseRelease(GLWidget *widget, const QMouseEvent *event) = 0;

      /**
       * Response to mouse movement
       * @param widget the %GLWidget where the even occurred
       * @param event the mouse event information
       */
      virtual QUndoCommand* mouseMove(GLWidget *widget, const QMouseEvent *event) = 0;

      /**
       * Response to mouse wheel movement
       * @param widget the %GLWidget where the even occurred
       * @param event the mouse wheel event information
       */
      virtual QUndoCommand* wheel(GLWidget *widget, const QWheelEvent *event) = 0;

      /**
       * Called by the GLWidget allowing overlay painting by the
       * tool.  Tools get painted last in the overall scheme.
       * @param widget the %GLWidget to paint to
       */
      virtual bool paint(GLWidget *widget);

      /**
       * Called by the parent (toolGroup) to tell the tool the underlying
       * model (molecule) has changed
       */
      virtual void setMolecule(Molecule *molecule);

      /**
       * Determines the ordering of the tools.  More useful
       * tools are placed first.  It is up to the tool designer
       * to be humble about their usefulness value.
       * @return usefulness value
       */
      virtual int usefulness() const;

      bool operator<(const Tool &other) const;

    Q_SIGNALS:
      /**
       * Can be used to add messages to the message pane.
       * @param m the message to add to the message pane.
       */
      void message(const QString &m);

    protected:
      ToolPrivate *const d;
  };

  /**
   * @class ToolFactory tool.h <avogadro/tool.h>
   * @brief Generates new instances of the Tool class for which it is defined.
   *
   * Generates new instances of the Tool class for which it is defined.
   */
  class A_EXPORT ToolFactory
  {
    public:
      /**
       * Destructor.
       */
      virtual ~ToolFactory() {}

      /**
       * @return pointer to a new instance of an Engine subclass object.
       */
      virtual Tool *createInstance(QObject *parent=0) = 0;
  };

} // end namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Tool*);
Q_DECLARE_INTERFACE(Avogadro::ToolFactory, "net.sourceforge.avogadro.toolfactory/1.0");

#endif
