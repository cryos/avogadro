/**********************************************************************
  BondItems - Project Tree Items for bonds.

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

#include "bonditems.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/bond.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  BondItems::BondItems() : m_label(0), m_widget(0)
  {
  }
 
  BondItems::~BondItems()
  {
  }
    
  void BondItems::setupModelData(ProjectTreeModel *model, GLWidget *widget, ProjectTreeItem *parent)
  {
    // save the widget
    m_widget = widget;
    m_model = model;

    // add the labels
    int position = parent->childCount();
    m_model->insertRows(parent, position, 1);
    m_label = parent->child(position);
    m_label->setData(0, alias());
    
    Molecule *molecule = m_widget->molecule();
    disconnect(molecule, 0, this, 0);
    // connect some signals to keep track of changes
    connect(molecule, SIGNAL(primitiveAdded(Primitive*)), this, SLOT(primitiveAdded(Primitive*)));
    connect(molecule, SIGNAL(primitiveUpdated(Primitive*)), this, SLOT(primitiveUpdated(Primitive*)));
    connect(molecule, SIGNAL(primitiveRemoved(Primitive*)), this, SLOT(primitiveRemoved(Primitive*)));

    initialize();
  }

  void BondItems::initialize()
  {
    Molecule *molecule = m_widget->molecule();

    // remove any existing rows
    if (m_label->childCount())
      m_model->removeRows(m_label, 0, m_label->childCount());

    // add the bonds...  
    m_model->insertRows(m_label, 0, molecule->numBonds());
    for (int i = 0; i < m_label->childCount(); ++i) {
      ProjectTreeItem *item = m_label->child(i);
      item->setData(0, tr("bond %1").arg(i + 1));
      // set the primitive
      PrimitiveList primitives;
      primitives.append(molecule->bond(i));
      item->setPrimitives(primitives);
    }

  }
  
  void BondItems::primitiveAdded(Primitive *primitive)
  {
    if (primitive->type() != Primitive::BondType) 
      return;
      
    // add the new primitive to the end
    m_model->insertRows(m_label, m_label->childCount(), 1); 
    // get the item
    ProjectTreeItem *item = m_label->child(m_label->childCount() - 1);
    item->setData(0, tr("bond %1").arg(primitive->index() + 1));
    // set the primitive
    PrimitiveList primitives;
    primitives.append(primitive);
    item->setPrimitives(primitives);
  }
 
  void BondItems::primitiveUpdated(Primitive *primitive)
  {
    if (primitive->type() == Primitive::MoleculeType) {
      initialize();
      return;
    }

    if (primitive->type() != Primitive::BondType)
      return;
    
    ProjectTreeItem *item = m_label->child(primitive->index()); 
    item->setData(0, tr("bond %1").arg(primitive->index() + 1));
  }
 
  void BondItems::primitiveRemoved(Primitive *primitive)
  {
    if (primitive->type() != Primitive::BondType)
      return;

    // remove the row
    m_model->removeRows(m_label, primitive->index(), 1);

    // loop over all atoms to fix their index if needed
    if ((primitive->index() + 1) < (unsigned long int) m_label->childCount()) {
      for (int i = 0; i < m_label->childCount(); ++i) {
        ProjectTreeItem *item = m_label->child(i);
        item->setData(0, tr("bond %1").arg(i + 1));
      }
    }
  }
 
  void BondItems::writeSettings(QSettings &settings) const
  {
    ProjectPlugin::writeSettings(settings);
  }
  
  void BondItems::readSettings(QSettings &settings)
  {
    ProjectPlugin::readSettings(settings);
  }

} // end namespace Avogadro

#include "bonditems.moc"
