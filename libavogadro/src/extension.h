/**********************************************************************
  Extension - Extension Class Interface

  Copyright (C) 2007-2008 Donald Ephraim Curtis

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

#ifndef EXTENSION_H
#define EXTENSION_H

#include <avogadro/glwidget.h>
#include "plugin.h"

#include <QVector>
#include <QSettings>
#include <QtPlugin>

#define AVOGADRO_EXTENSION(i, t)                    \
  public: \
    QString identifier() const { return i; } \
    QString name() const { return t; } \

#define AVOGADRO_EXTENSION_FACTORY(c,i,n,d)     \
  public: \
    Plugin *createInstance(QObject *parent = 0) { return new c(parent); } \
    Plugin::Type type() const { return Plugin::ExtensionType; } \
    QString identifier() const { return i; } \
    QString name() const { return n; } \
    QString description() const { return d; }; 

class QDockWidget;
class QUndoCommand;
class QTextEdit;

namespace Avogadro {

  /**
   * @class Extension extension.h <avogadro/extension.h>
   * @brief Interface for adding extensions
   * @author Donald Ephraim Curtis
   *
   * This is a template class used for adding extensions
   * to Avogadro.  Implementing the pure virutal functions
   * provides a mechanism for more functionality.
   * Extensions work by allowing each extension to have an unlimited
   * number of possible actions (each represented by a QAction).  To
   * perform an action the extension should implement performAction
   * and perform the correct action based on the action it receives.
   * The actual action should will be performed by the parent object
   * (usually MainWindow) as a result of a ::redo call on the returned
   * QUndoCommand from the performAction function.  Thus, to implement
   * functionality you should subclass QUndoCommand accordingly
   * based on the required functionality of the extension and return
   * the command based on the action being peformed.
   */
  class A_EXPORT Extension : public Plugin
  {
    Q_OBJECT

    public:
    Extension(QObject *parent) : Plugin(parent) {};
    virtual ~Extension() {};

    /** 
     * Plugin Type 
     */
    Plugin::Type type() const;

    /** 
     * Plugin Type Name (Extensions)
     */
    QString typeName() const;
 
    /**
     * @return a list of actions which this widget can perform
     */
    virtual QList<QAction *> actions() const = 0;

    /**
     * @return the menu path for the specified action
     */
    virtual QString menuPath(QAction *action) const;

    /**
     * @return a list of dock widgets associated with this extensions
     */
    virtual QDockWidget * dockWidget();

    /**
     * @param action the action that triggered the calls
     * @param widget the currently active GLWidget
     * feedback to the user)
     * @return an undo command for this action
     */
    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget) = 0;

    /**
     * save settings for this extension
     * @param settings settings variable to write settings to
     */
    virtual void writeSettings(QSettings &settings) const;

    /**
     * read settings for this extension
     * @param settings settings variable to read settings from
     */
    virtual void readSettings(QSettings &settings);

  public Q_SLOTS:
    virtual void setMolecule(Molecule *m);

  Q_SIGNALS:
    /**
     * Can be used to add messages to the message pane.
     * @param m the message to add to the message pane.
     */
    void message(const QString &m);
    /**
     * Can be used to notify the MainWindow to refresh the QActions for this extension.
     */
    void actionsChanged(Extension*);

  };

} // end namespace Avogadro

#endif
