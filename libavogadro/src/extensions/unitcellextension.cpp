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
#include <avogadro/molecule.h>

#include <QMessageBox>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  UnitCellExtension::UnitCellExtension(QObject *parent) : Extension(parent),
                                                          m_widget(NULL),
                                                          m_molecule(NULL)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Unit Cell Parameters..."));
    m_actions.append(action);
    m_dialog = new UnitCellParamDialog(static_cast<QWidget*>(parent));

    connect(m_dialog, SIGNAL(unitCellDisplayChanged(int, int, int)),
            this, SLOT(unitCellDisplayChanged(int, int, int)));
    connect(m_dialog, SIGNAL(unitCellParametersChanged(double, double, double, double, double, double)),
            this, SLOT(unitCellParametersChanged(double, double, double, double, double, double)));
    connect(m_dialog, SIGNAL(deleteUnitCell()),
            this, SLOT(deleteUnitCell()));
    connect(m_dialog, SIGNAL(fillUnitCell()),
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
    m_molecule = molecule;
  }
  
  QUndoCommand* UnitCellExtension::performAction(QAction *, GLWidget *widget)
  {
    // FIXME: this is bad mmmkay
    m_widget = widget;
    
    if (m_molecule == NULL)
      return NULL; // nothing we can do
    
    OBUnitCell *uc = m_molecule->OBUnitCell();
    
    if (uc == NULL) {
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
        m_molecule->setOBUnitCell(uc);
	
        widget->setUnitCells(1, 1, 1);
      } else { // do nothing -- user picked "Cancel"
        return NULL;
      }
      
    } // end if (existing unit cell or create a new one)
    
    m_dialog->aCells(widget->aCells());
    m_dialog->bCells(widget->bCells());
    m_dialog->cCells(widget->cCells());
    
    m_dialog->aLength(uc->GetA());
    m_dialog->bLength(uc->GetB());
    m_dialog->cLength(uc->GetC());
    
    m_dialog->alpha(uc->GetAlpha());
    m_dialog->beta(uc->GetBeta());
    m_dialog->gamma(uc->GetGamma());
    
    m_dialog->show();
    
    return NULL;
  }
  
  void UnitCellExtension::unitCellDisplayChanged(int a, int b, int c)
  {
    if (m_widget) {
      m_widget->setUnitCells(a, b, c);
      if (m_molecule)
        m_molecule->update();
    }
  }
  
  void UnitCellExtension::unitCellParametersChanged(double a, double b, double c,
                                                    double alpha, double beta, double gamma)
  {
    if (m_molecule) {
      OBUnitCell *uc = m_molecule->OBUnitCell();
      if (uc == NULL) // huh? strange, we lost our unit cell, just return
        return;
      
      uc->SetData(a, b, c, alpha, beta, gamma);
      m_molecule->setOBUnitCell(uc);
      m_molecule->update();
      
      if (m_widget)
        m_widget->update();
      
    } // end if molecule
  } // end parameters changed

  void UnitCellExtension::deleteUnitCell()
  {
    m_molecule->setOBUnitCell(NULL); // will also delete the underlying OBMol data
    m_molecule->update();
    
    m_widget->clearUnitCell();
  }
  
  void UnitCellExtension::fillUnitCell()
  {
    /* Change coords back to inverse space, apply the space group transforms
     *  then change coords back to real space
     */
    if (!m_molecule) {
      return;
    }
    
    OBUnitCell *uc = m_molecule->OBUnitCell();
    if (uc == NULL)
      return;

    const SpaceGroup *sg = uc->GetSpaceGroup(); // the actual space group and transformations for this unit cell
    
    // We operate on a copy of the Avogadro molecule
    // For each atom, we loop through:
    // * convert the coords back to inverse space
    // * apply the transformations
    // * create new (duplicate) atoms
    OBMol mol = m_molecule->OBMol();
    vector3 uniqueV, newV;
    list<vector3> transformedVectors; // list of symmetry-defined copies of the atom
    list<vector3>::iterator transformIterator;
    OBAtom *newAtom;
    QList<const OBAtom*> atoms; // keep the current list of unique atoms -- don't double-create
    FOR_ATOMS_OF_MOL(atom, mol)
      atoms.push_back(&(*atom));
    
    foreach(const OBAtom *atom, atoms) {
      uniqueV = atom->GetVector();
      // Assert: won't crash because we already ensure uc != NULL
      uniqueV *= uc->GetFractionalMatrix();
      
      transformedVectors = sg->Transform(uniqueV);
      for (transformIterator = transformedVectors.begin();
           transformIterator != transformedVectors.end(); ++transformIterator) {
        // coordinates are in reciprocal space -- check if it's in the unit cell
        // TODO: transform everything into the unit cell and check for duplicates
        if (transformIterator->x() < 0.0 || transformIterator->x() > 1.0)
          continue;
        else if (transformIterator->y() < 0.0 || transformIterator->y() > 1.0)
          continue;
        else if (transformIterator->z() < 0.0 || transformIterator->z() > 1.0)
          continue;
	
        newAtom = mol.NewAtom();
        // it would help to have a decent "duplicate atom" method here
        newAtom->SetAtomicNum(atom->GetAtomicNum());
        newAtom->SetVector(uc->GetOrthoMatrix() * (*transformIterator));
      } // end loop of transformed atoms
    } // end loop of atoms
    
    // m_molecule->ConnectTheDots();
    // m_molecule->PerceiveBondOrders(); // optional
    
    m_molecule->setOBMol(&mol);
  }

} // end namespace Avogadro

#include "unitcellextension.moc"
Q_EXPORT_PLUGIN2(unitcellextension, Avogadro::UnitCellExtensionFactory)
