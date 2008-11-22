/**********************************************************************
  MoleculeItems - Project Tree Items for molecule (atoms, bonds, ...).

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

#include "moleculeitems.h"
#include "atomitems.h"
#include "bonditems.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  MoleculeItems::MoleculeItems() : m_moleculeLabel(0), m_atomItems(0), 
      m_bondItems(0), m_residueLabel(0), m_widget(0)
  {
  }
 
  MoleculeItems::~MoleculeItems()
  {
    if (m_atomItems)
      delete m_atomItems;
    if (m_bondItems)
      delete m_bondItems;
  }
    
  void MoleculeItems::setupModelData(ProjectTreeModel *model, GLWidget *widget, ProjectTreeItem *parent)
  {
    // save the widget
    m_widget = widget;
    m_model = model;

    // add the labels
    int position = parent->childCount();
    m_model->insertRows(parent, position, 1);
    m_moleculeLabel = parent->child(position);
    m_moleculeLabel->setData(0, alias());
    
    m_atomItems = new AtomItems();
    m_atomItems->setupModelData(model, widget, m_moleculeLabel);
 
    m_bondItems = new BondItems();
    m_bondItems->setupModelData(model, widget, m_moleculeLabel);
 
  }

  void MoleculeItems::writeSettings(QSettings &settings) const
  {
    ProjectPlugin::writeSettings(settings);
  }
  
  void MoleculeItems::readSettings(QSettings &settings)
  {
    ProjectPlugin::readSettings(settings);
  }

} // end namespace Avogadro

#include "moleculeitems.moc"

//Q_EXPORT_PLUGIN2(smartsitems, Avogadro::MoleculeItemsFactory)
