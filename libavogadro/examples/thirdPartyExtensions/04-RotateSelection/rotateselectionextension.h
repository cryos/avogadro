/**********************************************************************
  RotateSelectionExtension

  Copyright (C) 2010 David C. Lonie

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

#ifndef ROTATESELECTIONEXTENSION_H
#define ROTATESELECTIONEXTENSION_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/extension.h>

// This is a work around for a bug on older versions Avogadro, bug
// 3104853. Patch submitted.
using Avogadro::Plugin;

namespace RotateSelection {
  // Forward declaration of header
  class RotateSelectionDialog;

  class RotateSelectionExtension : public Avogadro::Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Rotate Selection Extension",
                       tr("Rotate selected atoms about an arbitrary axis"),
                       tr("Select atoms and rotate about an arbitrary axis and angle."))

  public:
    RotateSelectionExtension(QObject *parent=0);
    ~RotateSelectionExtension() {};

    // This tells Avogadro what actions to create
    virtual QList<QAction *> actions() const;
    // This returns a string that tells Avogadro where to put the menu entries
    virtual QString menuPath(QAction *action) const;
    // When an action is requested (e.g. a user selects a menu entry),
    // here is where it is handled:
    virtual QUndoCommand* performAction(QAction *action,
                                        Avogadro::GLWidget *widget);
    // This is called whenever a new molecule is loaded. This is not
    // needed for this extension.
    virtual void setMolecule(Avogadro::Molecule *) {};

  public slots:
    void updateAxisFromBond();
    void apply();

  private:
    QList<QAction *> m_actions;
    RotateSelectionDialog *m_dialog;
    Avogadro::GLWidget *m_gl;

  };

  // Plugin factory setup
  class RotateSelectionExtensionFactory
    : public QObject,
      public Avogadro::PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(RotateSelectionExtension)
  };

}

#endif

