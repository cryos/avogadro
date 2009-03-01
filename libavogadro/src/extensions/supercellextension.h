/**********************************************************************
  SuperCellExtension - Extension for creating super cells

  Copyright (C) 2009 Marcus D. Hanwell
  Copyright (C) 2009 Geoffrey R. Hutchison

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

#ifndef SUPERCELLEXTENSION_H
#define SUPERCELLEXTENSION_H

#include <avogadro/extension.h>

namespace Avogadro {

  class SuperCellDialog;

  class SuperCellExtension : public Extension
  {
  Q_OBJECT
  AVOGADRO_EXTENSION("SuperCell", tr("Super Cell"),
                     tr("Build and display crystallographic super cells"))
  public:
    //! Constructor
    SuperCellExtension(QObject *parent=0);
    //! Destructor
    ~SuperCellExtension();

    //! Perform Action
    QList<QAction *> actions() const;
    QUndoCommand* performAction(QAction *action, GLWidget *widget);
    QString menuPath(QAction *action) const;
    void setMolecule(Molecule *molecule);

  public Q_SLOTS:
    void fillCell();

  private:
    void cellParametersChanged(double a, double b, double c);
    //! Simpler bonding routine for inorganics
    void connectTheDots();
    //! Actually duplicate the unit cell to build a super cell
    void duplicateUnitCell();

    QList<QAction *> m_actions;
    SuperCellDialog *m_dialog;
    GLWidget *m_widget;
    Molecule *m_molecule;
  };

  class SuperCellExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(SuperCellExtension)
  };

} // end namespace Avogadro

#endif
