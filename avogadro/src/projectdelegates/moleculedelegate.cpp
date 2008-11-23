/**********************************************************************
  MoleculeDelegate - Project Tree Items for molecule (atoms, bonds, ...).

  Copyright (C) 2008 by Tim Vandermeersch

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

#include "moleculedelegate.h"
#include "atomdelegate.h"
#include "bonddelegate.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  MoleculeDelegate::MoleculeDelegate(ProjectTreeModel *model) : ProjectTreeModelDelegate(model), 
      m_moleculeLabel(0), m_atomItems(0), m_bondItems(0)/*, m_residueLabel(0)*/
  {
  }
 
  MoleculeDelegate::~MoleculeDelegate()
  {
    // pointer now stolen by exportDelegate(...) !!

    //if (m_atomItems)
    //  delete m_atomItems;
    //if (m_bondItems)
    //  delete m_bondItems;
  }
    
  void MoleculeDelegate::initStructure(GLWidget *widget, ProjectTreeItem *parent)
  {
    // add the "Molecule" label
    m_moleculeLabel = insertExpandableItem(parent);
    m_moleculeLabel->setData(0, alias());
    
    // delegate the atoms
    m_atomItems = new AtomDelegate(model());
    m_atomItems->initStructure(widget, m_moleculeLabel);
    exportDelegate(m_atomItems);
 
    // delegate the bonds
    m_bondItems = new BondDelegate(model());
    m_bondItems->initStructure(widget, m_moleculeLabel);
    exportDelegate(m_bondItems);
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

