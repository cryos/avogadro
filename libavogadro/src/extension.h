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

#include "plugin.h"

#include <QVector>
#include <QSettings>
#include <QtPlugin>

#define AVOGADRO_EXTENSION(i, t, d)          \
  public: \
    static QString staticIdentifier() { return i; }          \
    QString identifier() const { return i; }                 \
    static QString staticName() { return t; }                \
    QString name() const { return t; }                       \
    static QString staticDescription() { return d; }         \
    QString description() const { return d; }

#define AVOGADRO_EXTENSION_FACTORY(c)     \
  public: \
    Avogadro::Plugin *createInstance(QObject *parent = 0) { return new c(parent); } \
    Avogadro::Plugin::Type type() const { return Avogadro::Plugin::ExtensionType; } \
    QString identifier() const { return c::staticIdentifier(); } \
    QString name() const { return c::staticName(); }         \
    QString description() const { return c::staticDescription(); }

class QDockWidget;
class QUndoCommand;
class QTextEdit;
class QAction;

namespace Avogadro {

  class GLWidget;
  class Molecule;

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
    /**
     * Constructor.
     */
    Extension(QObject *parent = 0);

    /**
     * Destructor.
     */
    virtual ~Extension();

    /**
     * \enum Possible Molecule change hints. The enumeration lists the
     * desired treatmest of the Molecule emitted. The hints should be honoured
     * by the class receiving the moleculeChanged signal.
     *
     * KeepOld - the old Molecule is not deleted, it is simply replaced.

     * DeleteOld - old default behaviour, old Molecule is deleted and
     * the new Molecule replaces the old one in the current window.
     *
     * NewWindow - open the new Molecule in a new Window if the old one was
     * modified.
     */
    enum MoleculeChangedHint
    {
      KeepOld   = 0x00, /// Keeps the old Molecule
      DeleteOld = 0x01, /// Deletes the old Molecule
      NewWindow = 0x02 /// Open the new Molecule in a new window if old is modified
    };

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
     * @return whether the specified action requires the network
     */
    virtual bool usesNetwork(QAction *action) const;

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

    /**
     * Determines the ordering of the extensions.  More useful
     * extensions will be placed first in menus. It is up to the
     * extension designer to be humble about their usefulness value.
     * @return usefulness value
     */
    virtual int usefulness() const;

  public Q_SLOTS:
    /**
     * Slot to set the Molecule for the Extension - should be called whenever
     * the active Molecule changes.
     */
    virtual void setMolecule(Molecule *molecule);

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

    /**
     * Can be used to notify the MainWindow to change the molecule to a new one.
     * The MoleculeChangedHint allows the extension to specify how the new and
     * old Molecule objects should be treated.
     */
    void moleculeChanged(Molecule *, int);
    
    /**
     * Can be used to notify the MainWindow of a delayed action.
     * For example, if the Extension shows a window that alters a bond length
     * or inserts a new framgent.
     * The action should be added to the undo stack.
     */
    void performCommand(QUndoCommand*);

  };

} // end namespace Avogadro

#endif
