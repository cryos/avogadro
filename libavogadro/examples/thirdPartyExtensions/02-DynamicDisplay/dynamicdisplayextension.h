/**********************************************************************
  DynamicDisplayExtension

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

#ifndef DYNAMICDISPLAYEXTENSION_H
#define DYNAMICDISPLAYEXTENSION_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/extension.h>

// This is a work around for a bug on older versions Avogadro, bug 3104853
using Avogadro::Plugin;

namespace DynamicDisplay {
  class DynamicDisplayDialog;

  class DynamicDisplayExtension : public Avogadro::Extension
  {
    Q_OBJECT
    // Setup the plugin:
    AVOGADRO_EXTENSION(// Static identifier:
                       "Dynamic Display Extension",
                       // Short description:
                       tr("Example Extension with Dynamic Display"),
                       // Long description:
                       tr("Provides a dialog box that displays information about the current molecule."))

  public:
    DynamicDisplayExtension(QObject *parent=0);
    ~DynamicDisplayExtension() {};

    virtual QList<QAction *> actions() const;
    virtual QUndoCommand* performAction(QAction *action,
                                        Avogadro::GLWidget *widget);
    virtual QString menuPath(QAction *action) const;
    // This is new -- we will be using this to update the dialog when
    // a new molecule is loaded.
    virtual void setMolecule(Avogadro::Molecule *mol);

  public slots:
    // This slot is called when the molecule changes (see ctor)
    void moleculeUpdated();

  signals:
    // Tell any listeners that they should reset themselves because no
    // valid molecule is loaded.
    void resetExtension();
    // Tell any listeners to update themselves because the molecule
    // has changed.
    void updateExtension(Avogadro::Molecule *mol);

  private:
    QList<QAction *> m_actions;
    DynamicDisplayDialog *m_dialog;
    // Cached pointer to the current molecule
    Avogadro::Molecule *m_molecule;
  };

  class DynamicDisplayExtensionFactory
    : public QObject,
      public Avogadro::PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(DynamicDisplayExtension)
  };

}

#endif

