/**********************************************************************
  UnitCell - Change parameters for crystallographic unit cells

  Copyright (C) 2007 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "unitcellextension.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>

#include <QMessageBox>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  UnitCellExtension::UnitCellExtension(QObject *parent) : Extension(parent), m_Widget(NULL)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Unit Cell Parameters..."));
    m_actions.append(action);
    m_Dialog = new UnitCellParamDialog(static_cast<QWidget*>(parent));

    connect(m_Dialog, SIGNAL(unitCellDisplayChanged(int, int, int)),
        this, SLOT(unitCellDisplayChanged(int, int, int)));
    connect(m_Dialog, SIGNAL(unitCellParametersChanged(double, double, double, double, double, double)),
        this, SLOT(unitCellParametersChanged(double, double, double, double, double, double)));
    connect(m_Dialog, SIGNAL(deleteUnitCell()),
        this, SLOT(deleteUnitCell()));
    connect(m_Dialog, SIGNAL(fillUnitCell()),
        this, SLOT(fillUnitCell()));
  }

  UnitCellExtension::~UnitCellExtension()
  {
  }

  QList<QAction *> UnitCellExtension::actions() const
  {
    return m_actions;
  }

  QString UnitCellExtension::menuPath(QAction *) const
  {
    return tr("&Build");
  }

  void UnitCellExtension::setMolecule(Molecule *molecule)
  {
    m_Molecule = molecule;
  }

  QUndoCommand* UnitCellExtension::performAction(QAction *, GLWidget *widget)
  {
    // FIXME: this is bad mmmkay
    m_Widget = widget;

    OBUnitCell *uc = NULL;
    if (m_Molecule && m_Molecule->HasData(OBGenericDataType::UnitCell)) {
      uc = dynamic_cast<OBUnitCell*>(m_Molecule->GetData(OBGenericDataType::UnitCell));
    } else {
      // show warning and ask if the user wants to create a unit cell
      // (otherwise this extension isn't very useful)

      QMessageBox::StandardButton ret;
      ret = QMessageBox::warning(qobject_cast<QWidget*>(parent()),
          tr("Avogadro"),
          tr("This document is currently an isolated molecule.\n\n"
            "Do you want to create a crystal unit cell?"),
          QMessageBox::Yes
          | QMessageBox::No);
      if (ret == QMessageBox::Yes) {
        // Set some initial data (e.g., a box about the size of the molecule)
        // and one unit cell in each direction
        uc = new OBUnitCell;
        double estimatedSize = widget->radius() + 2.0;
        uc->SetData(estimatedSize, estimatedSize, estimatedSize,
            90.0, 90.0, 90.0);
        m_Molecule->SetData(uc);

        widget->setUnitCells(1, 1, 1);
      } else { // do nothing -- user picked "Cancel"
        return NULL;
      }

    } // end if (existing unit cell or create a new one)

    m_Dialog->aCells(widget->aCells());
    m_Dialog->bCells(widget->bCells());
    m_Dialog->cCells(widget->cCells());

    m_Dialog->aLength(uc->GetA());
    m_Dialog->bLength(uc->GetB());
    m_Dialog->cLength(uc->GetC());

    m_Dialog->alpha(uc->GetAlpha());
    m_Dialog->beta(uc->GetBeta());
    m_Dialog->gamma(uc->GetGamma());

    m_Dialog->show();

    return NULL;
  }

  void UnitCellExtension::unitCellDisplayChanged(int a, int b, int c)
  {
    if (m_Widget) {
      m_Widget->setUnitCells(a, b, c);
    }
  }

  void UnitCellExtension::unitCellParametersChanged(double a, double b, double c,
      double alpha, double beta, double gamma)
  {
    if (m_Molecule) {
      OBUnitCell *uc = NULL;
      if (m_Molecule && m_Molecule->HasData(OBGenericDataType::UnitCell)) {
        uc = dynamic_cast<OBUnitCell*>(m_Molecule->GetData(OBGenericDataType::UnitCell));
        uc->SetData(a, b, c, alpha, beta, gamma);

        if (m_Widget)
          m_Widget->update();

      } // end if unit cell
    } // end if molecule
  } // end parameters changed

  void UnitCellExtension::deleteUnitCell()
  {
    m_Molecule->DeleteData(OBGenericDataType::UnitCell);
    m_Widget->clearUnitCell();
  }
  
  void UnitCellExtension::fillUnitCell()
  {
    /* Change coords back to inverse space, apply the space group transforms
    *  then change coords back to real space
    */
    if (!m_Molecule) {
      return;
    }
    
      OBUnitCell *uc = NULL;
      if (m_Molecule && m_Molecule->HasData(OBGenericDataType::UnitCell)) {
        uc = dynamic_cast<OBUnitCell*>(m_Molecule->GetData(OBGenericDataType::UnitCell));
        
        const SpaceGroup *sg = uc->GetSpaceGroup(); // the actual space group and transformations for this unit cell
        
        // For each atom, we loop through: convert the coords back to inverse space, apply the transformations and create new atoms
        vector3 uniqueV, newV;
        list<vector3> transformedVectors; // list of symmetry-defined copies of the atom
        list<vector3>::iterator transformIterator;
        OBAtom *newAtom;
        QList<const OBAtom*> atoms; // keep the current list of unique atoms -- don't double-create
        FOR_ATOMS_OF_MOL(atom, m_Molecule)
          atoms.push_back(&(*atom));

        foreach(const OBAtom *atom, atoms) {
          uniqueV = atom->GetVector();
          if (uc != NULL)
            uniqueV *= uc->GetFractionalMatrix();
            
          transformedVectors = sg->Transform(uniqueV);
          for (transformIterator = transformedVectors.begin();
               transformIterator != transformedVectors.end(); ++transformIterator) {
            // coordinates are in reciprocal space -- check if it's in the unit cell
            // TODO: transform these into the unit cell and check for duplicates
            if (transformIterator->x() < 0.0 || transformIterator->x() > 1.0)
              continue;
            else if (transformIterator->y() < 0.0 || transformIterator->y() > 1.0)
              continue;
            else if (transformIterator->z() < 0.0 || transformIterator->z() > 1.0)
              continue;
                 
            newAtom = m_Molecule->NewAtom();
            // it would help to have a decent "duplicate atom" method here
            newAtom->SetAtomicNum(atom->GetAtomicNum());
            newAtom->SetVector(uc->GetOrthoMatrix() * (*transformIterator));
          } // end loop of transformed atoms
        } // end loop of atoms
        
        // m_Molecule->ConnectTheDots();
        // m_Molecule->PerceiveBondOrders(); // optional
        
      } // end (if unit cell)
  }

} // end namespace Avogadro

#include "unitcellextension.moc"
Q_EXPORT_PLUGIN2(unitcellextension, Avogadro::UnitCellExtensionFactory)
