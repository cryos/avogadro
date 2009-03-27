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

#include "supercellextension.h"
#include "supercelldialog.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/glwidget.h>
#include <avogadro/neighborlist.h>

#include <openbabel/mol.h>
#include <openbabel/generic.h>

#include <QMessageBox>
#include <QCoreApplication>
#include <QDebug>

namespace Avogadro {

  using OpenBabel::OBUnitCell;
  using OpenBabel::vector3;
  using OpenBabel::OBMol;
  using OpenBabel::OBAtom;
  using OpenBabel::OBMolAtomIter;
  using OpenBabel::SpaceGroup;
  using std::list;
  using std::vector;

  SuperCellExtension::SuperCellExtension(QObject *parent) : Extension(parent),
    m_dialog(0), m_widget(0), m_molecule(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Super Cell Builder..."));
    m_actions.append(action);
  }

  SuperCellExtension::~SuperCellExtension()
  {
  }

  QList<QAction *> SuperCellExtension::actions() const
  {
    return m_actions;
  }

  QString SuperCellExtension::menuPath(QAction *) const
  {
    return tr("&Build");
  }

  void SuperCellExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* SuperCellExtension::performAction(QAction *, GLWidget *widget)
  {
    if (m_molecule == 0)
      return 0;

    m_widget = widget;

    OBUnitCell *uc = m_molecule->OBUnitCell();

    if (uc == 0) {
      QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                           tr("Warning"),
                           tr("This document is currently an isolated molecule. "
                              "You need to create a unit cell."));
      return 0;
    }
    if (!m_dialog) {
      m_dialog = new SuperCellDialog(qobject_cast<QWidget*>(parent()));
      connect(m_dialog, SIGNAL(fillCell()), this, SLOT(fillCell()));
    }

    m_dialog->aCells(widget->aCells());
    m_dialog->bCells(widget->bCells());
    m_dialog->cCells(widget->cCells());

    m_dialog->show();

    return NULL;
  }

  void SuperCellExtension::cellParametersChanged(double a, double b, double c)
  {
    if (m_molecule) {
      OBUnitCell *uc = m_molecule->OBUnitCell();
      if (!uc) // Should not happen
        return;

      uc->SetData(a * uc->GetA(), b * uc->GetB(), c * uc->GetC(),
                  uc->GetAlpha(), uc->GetBeta(), uc->GetGamma());
      m_molecule->setOBUnitCell(uc);
      m_molecule->update();

      if (m_widget)
        m_widget->update();
    } // end if molecule
  } // end parameters changed

  vector3 transformedFractionalCoordinate(vector3 originalCoordinate)
  {
    // ensure the fractional coordinate is entirely within the unit cell
    vector3 returnValue(originalCoordinate);

    // So if we have -2.08, we take -2.08 - (-2) = -0.08 .... exactly what we want
    returnValue.SetX(originalCoordinate.x() - static_cast<int>(originalCoordinate.x()));
    returnValue.SetY(originalCoordinate.y() - static_cast<int>(originalCoordinate.y()));
    returnValue.SetZ(originalCoordinate.z() - static_cast<int>(originalCoordinate.z()));

    return returnValue;
  }

  void SuperCellExtension::fillCell()
  {
    /* Change coords back to inverse space, apply the space group transforms
     *  then change coords back to real space
     */
    if (!m_molecule)
      return;

    OBUnitCell *uc = m_molecule->OBUnitCell();
    if (!uc) {
      qDebug() << "No unit cell found - fillCell() returning...";
      return;
    }

    m_molecule->blockSignals(true);
    const SpaceGroup *sg = uc->GetSpaceGroup(); // the actual space group and transformations for this unit cell
    if (sg) {
      qDebug() << "Space group:" << sg->GetId();// << sg->GetHMName();
      // We operate on a copy of the Avogadro molecule
      // For each atom, we loop through:
      // * convert the coords back to inverse space
      // * apply the transformations
      // * create new (duplicate) atoms
      OBMol mol = m_molecule->OBMol();
      vector3 uniqueV, newV;
      list<vector3> transformedVectors; // list of symmetry-defined copies of the atom
      list<vector3>::iterator transformIterator, duplicateIterator;
      vector3 updatedCoordinate;
      bool foundDuplicate;

      OBAtom *addAtom;
      QList<OBAtom*> atoms; // keep the current list of unique atoms -- don't double-create
      list<vector3> coordinates; // all coordinates to prevent duplicates
      FOR_ATOMS_OF_MOL(atom, mol)
        atoms.push_back(&(*atom));

      foreach(OBAtom *atom, atoms) {
        uniqueV = atom->GetVector();
        // Assert: won't crash because we already ensure uc != NULL
        uniqueV *= uc->GetFractionalMatrix();
        uniqueV = transformedFractionalCoordinate(uniqueV);
        coordinates.push_back(uniqueV);

        transformedVectors = sg->Transform(uniqueV);
        for (transformIterator = transformedVectors.begin();
             transformIterator != transformedVectors.end(); ++transformIterator) {
          // coordinates are in reciprocal space -- check if it's in the unit cell
          // if not, transform it in place
          updatedCoordinate = transformedFractionalCoordinate(*transformIterator);
          foundDuplicate = false;

          // Check if the transformed coordinate is a duplicate of an atom
          for (duplicateIterator = coordinates.begin();
               duplicateIterator != coordinates.end(); ++duplicateIterator) {
            if (duplicateIterator->distSq(updatedCoordinate) < 1.0e-4) {
              foundDuplicate = true;
              break;
            }
          }
          if (foundDuplicate)
            continue;

          addAtom = mol.NewAtom();
          addAtom->Duplicate(atom);
          addAtom->SetVector(uc->GetOrthoMatrix() * updatedCoordinate);
        } // end loop of transformed atoms

        // Put the original atom into the proper space in the unit cell too
        atom->SetVector(uc->GetOrthoMatrix() * uniqueV);
      } // end loop of atoms

      m_molecule->setOBMol(&mol);
      qDebug() << "Spacegroups done...";
      uc->SetSpaceGroup(1);
    }

    m_molecule->update();
    QCoreApplication::processEvents();

    // Remove any bonds that may have snook in
    foreach(Bond *b, m_molecule->bonds())
      m_molecule->removeBond(b);

    // Now duplicate the entire cell so that inter-cell bonding can be done
    duplicateUnitCell();
    qDebug() << "Unit cell duplicated...";
    m_molecule->update();

    // Simpler version of connect the dots
    connectTheDots();
    qDebug() << "Dots connected...";
    m_molecule->blockSignals(false);
    m_molecule->update();
  }

  void SuperCellExtension::connectTheDots()
  {
    // Add single bonds between all atoms closer than their combined atomic
    // covalent radii.
    vector<double> rad;
    NeighborList nbrs(m_molecule, 2.2);

    rad.reserve(m_molecule->numAtoms());

    foreach (Atom *atom, m_molecule->atoms())
      rad.push_back(OpenBabel::etab.GetCovalentRad(atom->atomicNumber()));

    foreach (Atom *atom1, m_molecule->atoms()) {
      foreach (Atom *atom2, nbrs.nbrs(atom1)) {
        if (m_molecule->bond(atom1, atom2))
          continue;
        if (atom1->isHydrogen() && atom2->isHydrogen())
          continue;
        // bonded if closer than elemental Rcov + tolerance
        double cutoff = (rad[atom1->index()] + rad[atom2->index()] + 0.45)
               * (rad[atom1->index()] + rad[atom2->index()] + 0.45);

        double d2  = ((*atom1->pos()) - (*atom2->pos())).squaredNorm();

        if (d2 > cutoff || d2 < 0.40)
          continue;

        Bond *bond = m_molecule->addBond();
        bond->setAtoms(atom1->id(), atom2->id(), 1);
      }
      QCoreApplication::processEvents();
    }
  }

  void SuperCellExtension::duplicateUnitCell()
  {
    // Duplicates the entire unit cell the number of times specified
    std::vector<vector3> cellVectors = m_molecule->OBUnitCell()->GetCellVectors();
    QList<Atom*> orig = m_molecule->atoms();
    for (int a = 0; a < m_dialog->aCells(); ++a) {
      for (int b = 0; b < m_dialog->bCells(); ++b)  {
        for (int c = 0; c < m_dialog->cCells(); ++c)  {
          Eigen::Vector3d disp(
                       cellVectors[0].x() * a
                       + cellVectors[1].x() * b
                       + cellVectors[2].x() * c,
                       cellVectors[0].y() * a
                       + cellVectors[1].y() * b
                       + cellVectors[2].y() * c,
                       cellVectors[0].z() * a
                       + cellVectors[1].z() * b
                       + cellVectors[2].z() * c );
          foreach(Atom *atom, orig) {
            Atom *newAtom = m_molecule->addAtom();
            *newAtom = *atom;
            newAtom->setPos((*atom->pos())+disp);
          }
        }
        QCoreApplication::processEvents();
      }
    } // end of for loops
    // Update the length of the unit cell
    cellParametersChanged(m_dialog->aCells(), m_dialog->bCells(),
                          m_dialog->cCells());
  }

} // end namespace Avogadro

#include "supercellextension.moc"
Q_EXPORT_PLUGIN2(supercellextension, Avogadro::SuperCellExtensionFactory)
