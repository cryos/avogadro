/**********************************************************************
  MoleculeDelegate - Project Tree Items for molecule (atoms, bonds, ...).

  Copyright (C) 2008 by Tim Vandermeersch

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

#include "moleculedelegate.h"
#include "atomdelegate.h"
#include "bonddelegate.h"
#include "residuedelegate.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  MoleculeDelegate::MoleculeDelegate(ProjectTreeModel *model) : ProjectTreeModelDelegate(model)
  {
  }
 
  MoleculeDelegate::~MoleculeDelegate()
  {
  }
    
  void MoleculeDelegate::initStructure(GLWidget *widget, ProjectTreeItem *parent)
  {
    // add the "Molecule" label
    ProjectTreeItem *moleculeLabel = insertExpandableItem(parent);
    moleculeLabel->setData(0, alias());
    
    // delegate the atoms
    ProjectTreeModelDelegate *atomDelegate = new AtomDelegate(model());
    atomDelegate->initStructure(widget, moleculeLabel);
    exportDelegate(atomDelegate);
 
    // delegate the bonds
    ProjectTreeModelDelegate *bondDelegate = new BondDelegate(model());
    bondDelegate->initStructure(widget, moleculeLabel);
    exportDelegate(bondDelegate);

    // delegate the residues
    ProjectTreeModelDelegate *residueDelegate = new ResidueDelegate(model());
    residueDelegate->initStructure(widget, moleculeLabel);
    exportDelegate(residueDelegate);
  }

  void MoleculeDelegate::writeSettings(QSettings &settings) const
  {
    ProjectTreeModelDelegate::writeSettings(settings);
  }
  
  void MoleculeDelegate::readSettings(QSettings &settings)
  {
    ProjectTreeModelDelegate::readSettings(settings);
  }

} // end namespace Avogadro

#include "moleculedelegate.moc"

