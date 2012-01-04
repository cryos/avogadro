/**********************************************************************
  Tool - Avogadro Tool Interface

  Copyright (C) 2007 Donald Ephraim Curtis

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

#ifndef TOOL_H
#define TOOL_H

#include <avogadro/global.h>
#include "plugin.h"

#include <QSettings>
#include <QtPlugin>
#include <QWheelEvent>

#define AVOGADRO_TOOL(i, t, d, s)                            \
  public: \
    static QString staticIdentifier() { return i; }          \
    QString identifier() const { return i; }                 \
    static QString staticName() { return t; }                \
    QString name() const { return t; }                       \
    static QString staticDescription() { return d; }         \
    QString description() const { return d; }                \
    QString settingsTitle() const { return s; }

#define AVOGADRO_TOOL_FACTORY(c)                             \
  public: \
    Avogadro::Plugin *createInstance(QObject *parent = 0) { return new c(parent); } \
    Avogadro::Plugin::Type type() const { return Avogadro::Plugin::ToolType; } \
    QString identifier() const { return c::staticIdentifier(); } \
    QString name() const { return c::staticName(); }         \
    QString description() const { return c::staticDescription(); }

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
  class A_EXPORT Tool : public Plugin
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
       * Plugin Type.
       */
      Plugin::Type type() const;

      /**
       * Plugin Type Name (Tools).
       */
      QString typeName() const;

      /**
       * @return The QAction of the tool.
       */
      virtual QAction* activateAction() const;

      /**
       * @return The settings widget for the tool.
       */
      virtual QWidget* settingsWidget();

      /**
       * @return The translated name of the settings widget
       */
      virtual QString settingsTitle() const = 0;

      /**
       * Response to mouse press
       * @param widget the %GLWidget where the even occurred
       * @param event the mouse event information
       */
      virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event) = 0;

      /**
       * Response to mouse release
       * @param widget the %GLWidget where the even occurred
       * @param event the mouse event information
       */
      virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event) = 0;

      /**
       * Response to mouse movement
       * @param widget the %GLWidget where the event occurred
       * @param event the mouse event information
       */
      virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event) = 0;

      /**
       * Response to a user double-click
       * @param widget the %GLWidget where the event occurred
       * @param event the mouse event information
       * @since version 1.1
       */
      virtual QUndoCommand* mouseDoubleClickEvent(GLWidget *widget, QMouseEvent *event) = 0;

      /**
       * Response to mouse wheel movement
       * @param widget the %GLWidget where the event occurred
       * @param event the mouse wheel event information
       */
      virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);

      /**
       * Response to key press events.
       * @param widget the %GLWidget where the event occurred
       * @param event the key event information
       */
      virtual QUndoCommand* keyPressEvent(GLWidget *widget, QKeyEvent *event);

      /**
       * Response to key release events.
       * @param widget the %GLWidget where the event occurred
       * @param event the key event information
       */
      virtual QUndoCommand* keyReleaseEvent(GLWidget *widget, QKeyEvent *event);

      /**
       * Called by the GLWidget allowing overlay painting by the
       * tool.  Tools get painted last in the overall scheme.
       * @param widget the %GLWidget to paint to
       */
      virtual bool paint(GLWidget *widget);

      /**
       * Determines the ordering of the tools.  More useful
       * tools are placed first.  It is up to the tool designer
       * to be humble about their usefulness value.
       * @return usefulness value
       */
      virtual int usefulness() const;

      bool operator<(const Tool &other) const;

      /**
       * Write the tool settings so that they can be saved between sessions.
       */
      virtual void writeSettings(QSettings &settings) const;

      /**
       * Read in the settings that have been saved for the tool instance.
       */
      virtual void readSettings(QSettings &settings);

    Q_SIGNALS:
      /**
       * Can be used to add messages to the message pane.
       * @param m the message to add to the message pane.
       */
      void message(const QString &m);

    public Q_SLOTS:
      /**
       * Called by the parent (normally toolGroup) to tell the tool the underlying
       * model (molecule) has changed
       */
      virtual void setMolecule(Molecule *molecule);

    protected:
      QAction *m_activateAction;
      ToolPrivate *const d;
  };

} // end namespace Avogadro

#endif
