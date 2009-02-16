/**********************************************************************
  ResidueDelegate - Project Tree Item for residues.

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

#include "residuedelegate.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/residue.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

#include <openbabel/mol.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  ResidueDelegate::ResidueDelegate(ProjectTreeModel *model) : ProjectTreeModelDelegate(model), m_label(0), m_widget(0)
  {
  }
 
  ResidueDelegate::~ResidueDelegate()
  {
  }
    
  void ResidueDelegate::initStructure(GLWidget *widget, ProjectTreeItem *parent)
  {
    // save the widget
    m_widget = widget;

    // add the labels
    m_label = insertExpandableItem(parent);
    m_label->setData(0, alias());
  }

  void ResidueDelegate::fetchMore(ProjectTreeItem *)
  {
    // the user has expanded our label, we now initialize the bond items
    // and keep track of the using the signals...
    
    Molecule *molecule = m_widget->molecule();
    disconnect(molecule, 0, this, 0);
    // connect some signals to keep track of changes
    connect(molecule, SIGNAL(primitiveAdded(Primitive*)), this, SLOT(primitiveAdded(Primitive*)));
    connect(molecule, SIGNAL(primitiveUpdated(Primitive*)), this, SLOT(primitiveUpdated(Primitive*)));
    connect(molecule, SIGNAL(primitiveRemoved(Primitive*)), this, SLOT(primitiveRemoved(Primitive*)));

    initialize();
  }

  void ResidueDelegate::initialize()
  {
    Molecule *molecule = m_widget->molecule();

    // remove any existing rows
    if (m_label->childCount())
      model()->removeRows(m_label, 0, m_label->childCount());
      
    QList<Residue *> residues(molecule->residues());

    // add the residues...  
    model()->insertRows(m_label, 0, molecule->numResidues());
    for (int i = 0; i < m_label->childCount(); ++i) {
      ProjectTreeItem *item = m_label->child(i);
      item->setData(0, residues.at(i)->name());
      item->setData(1, QString("%1").arg(i));
      
      // set the primitive
      PrimitiveList primitives;
      primitives.append(residues.at(i));
      foreach (unsigned long id, residues.at(i)->atoms()) {
        Atom *atom = molecule->atomById(id);
        if (atom)
          primitives.append(atom);
      }
      foreach (unsigned long id, residues.at(i)->bonds()) {
        Bond *bond = molecule->bondById(id);
        if (bond)
          primitives.append(bond);
      }
      item->setPrimitives(primitives);
    }

  }
  
  void ResidueDelegate::primitiveAdded(Primitive *primitive)
  {
    if (primitive->type() != Primitive::ResidueType)
      return;
    
    Molecule *molecule = m_widget->molecule();
    Residue *residue = static_cast<Residue*>(primitive);
    
    // add the new primitive to the end
    model()->insertRows(m_label, m_label->childCount(), 1);
    ProjectTreeItem *item = m_label->child(m_label->childCount() - 1);
    item->setData(0, residue->name());
    item->setData(1, QString("%1").arg(primitive->index()));

    // set the primitive
    PrimitiveList primitives;
      primitives.append(residue);
    foreach (unsigned long id, residue->atoms()) {
      Atom *atom = molecule->atomById(id);
      if (atom)
        primitives.append(atom);
    }
    foreach (unsigned long id, residue->bonds()) {
      Bond *bond = molecule->bondById(id);
      if (bond)
        primitives.append(bond);
    }
    item->setPrimitives(primitives);
  }
 
  void ResidueDelegate::primitiveUpdated(Primitive *primitive)
  {
    if (primitive->type() == Primitive::MoleculeType) {
      initialize();
      return;
    }

    if (primitive->type() != Primitive::ResidueType) 
      return;

    Molecule *molecule = m_widget->molecule();
    Residue *residue = static_cast<Residue*>(primitive);
    
    ProjectTreeItem *item = m_label->child(primitive->index());
    item->setData(0, residue->name());
    item->setData(1, QString("%1").arg(primitive->index()));

    // set the primitive
    PrimitiveList primitives;
    primitives.append(residue);
    foreach (unsigned long id, residue->atoms()) {
      Atom *atom = molecule->atomById(id);
      if (atom)
        primitives.append(atom);
    }
    foreach (unsigned long id, residue->bonds()) {
      Bond *bond = molecule->bondById(id);
      if (bond)
        primitives.append(bond);
    }
    item->setPrimitives(primitives);
 
    model()->emitDataChanged(m_label, primitive->index());
  }
 
  void ResidueDelegate::primitiveRemoved(Primitive *primitive)
  {
    if (primitive->type() != Primitive::ResidueType)
      return;

    // remove the row
    model()->removeRows(m_label, primitive->index(), 1);

    // loop over residues below to fix their index if needed
    if ((primitive->index() + 1) < (unsigned long) m_label->childCount()) {
      for (int i = primitive->index(); i < m_label->childCount(); ++i) {
        ProjectTreeItem *item = m_label->child(i);
        item->setData(1, QString("%1").arg(i));
      }
    }
  }
 
  void ResidueDelegate::writeSettings(QSettings &settings) const
  {
    ProjectTreeModelDelegate::writeSettings(settings);
  }
  
  void ResidueDelegate::readSettings(QSettings &settings)
  {
    ProjectTreeModelDelegate::readSettings(settings);
  }

} // end namespace Avogadro

#include "residuedelegate.moc"
