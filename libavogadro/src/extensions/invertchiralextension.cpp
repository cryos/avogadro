/**********************************************************************
  InvertChiral - Invert selected stereocenters

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "invertchiralextension.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>
#include <openbabel/builder.h>

#include <QAction>
#include <QInputDialog>
#include <QString>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  InvertChiralExtension::InvertChiralExtension(QObject *parent) : Extension(parent), m_molecule(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Invert Chirality"));
    m_actions.append(action);

    action = new QAction( this );
    action->setSeparator(true);
    m_actions.append( action );
  }

  InvertChiralExtension::~InvertChiralExtension()
  {
  }

  QList<QAction *> InvertChiralExtension::actions() const
  {
    return m_actions;
  }

  QString InvertChiralExtension::menuPath(QAction *) const
  {
    return tr("&Build");
  }

  void InvertChiralExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* InvertChiralExtension::performAction(QAction *, GLWidget *widget)
  {
    if (widget == NULL)
      return 0;

    m_molecule = widget->molecule();
    QList<Primitive *> selectedAtoms = widget->selectedPrimitives().subList(Primitive::AtomType);

    if (selectedAtoms.isEmpty()) {
      // make an enantiomer by reflecting everything in the x-axis
      foreach(Atom *atom, m_molecule->atoms()) {
        const Eigen::Vector3d *pos = atom->pos();
        Eigen::Vector3d newPos = *pos;
        newPos.x() = pos->x() * -1.0;
        atom->setPos(newPos);
      }
    } else {
      // harder, since we have to flip two neighbors
      OBMol obmol = m_molecule->OBMol();
      foreach(Primitive *primitive, selectedAtoms) {
        Atom *atom = static_cast<Atom *>(primitive);
        if (!atom)
          continue; // shouldn't happen, since we specifically requested Primitive::AtomType

        int index = atom->index() + 1; // OBMol indexes atoms by 1, not 0
        QList<unsigned long> neighborList = atom->neighbors();
        if (neighborList.size() > 1) { // it doesn't matter which two -- we can simply swap any two atoms to invert
          int a = m_molecule->atomById(neighborList[0])->index();
          int b = m_molecule->atomById(neighborList[1])->index();
          OBBuilder::Swap(obmol, index, a + 1, index, b + 1); // this unfortunately doesn't work for ring atoms
        }
      }
      m_molecule->setOBMol(&obmol);
    }

    m_molecule->update();
    widget->update();

    return 0;
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(hydrogensextension, Avogadro::InvertChiralExtensionFactory)

