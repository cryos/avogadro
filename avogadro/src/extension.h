/**********************************************************************
  Extension - Extension Class Interface

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

#ifndef __EXTENSION_H
#define __EXTENSION_H

#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>

#include <QGLWidget>
#include <QList>
#include <QObject>
#include <QString>
#include <QTextEdit>
#include <QVector>

class QAction;
class QUndoCommand;
namespace Avogadro {

  /**
   * @class Extension
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
  class Extension
  {
    public:
    Extension() {};
    virtual ~Extension() {};

    /** @return the name of the extension
     */
    virtual QString name() const
    { return QObject::tr("Unknown"); }

    /** @return a brief description of what the extension does (e.g., tooltip)
     */
    virtual QString description() const
    { return QObject::tr("Unknown Extension"); }

    /** @return a menu path for the extension's actions
     *
     * A "menu path" specifies the menu and any submenus where
     * actions will be installed. Submenus are separated by ">" marks.
     * For example:
     * "Tools"  = all actions will be installed under the "Tools" menu.
     * "Tools>Molecular Mechanics" = all actions will be installed
     *   into a submenu "Molecular Mechanics" of the "Tools" menu.
     *
     * If the menu or submenu name does not exist, it will be created.
     * If you wish to use an existing menu, make sure the path matches exactly.
     * For example: "&Tools" not "Tools"
     */
    virtual QString menuPath() const
    { return QObject::tr("&Tools"); }

    /**
     * @return a list of actions which this widget can perform
     */
    virtual QList<QAction *> actions() const = 0;
    /**
     * @param action the action that triggered the calls
     * @param molecule the molecule to perform the action on
     * @param widget the currently active GLWidget
     * @param messages a QTextEdit to push information too (allowing 
     * feedback to the user)
     * @return an undo command for this action
     */
    virtual QUndoCommand* performAction(QAction *action, Molecule *molecule, 
                                        GLWidget *widget, QTextEdit *messages = NULL) = 0;

  };

  class ExtensionFactory
  {
    public:
      /**
       * Extension factory deconstructor.
       */
      virtual ~ExtensionFactory() {}

      /**
       * @return pointer to a new instance of an Engine subclass object
       */
      virtual Extension *createInstance(QObject *parent=0) = 0;
  };

} // end namespace Avogadro

// Q_DECLARE_INTERFACE(Avogadro::Extension, "net.sourceforge.avogadro.extension/1.0")
Q_DECLARE_INTERFACE(Avogadro::ExtensionFactory, "net.sourceforge.avogadro.extensionfactory/1.0");


#endif
