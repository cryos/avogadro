/**********************************************************************
  SelectionDelegate - Project Tree Item for user selections.

  Copyright (C) 2009 by Tim Vandermeersch

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

#include "selectiondelegate.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  SelectionDelegate::SelectionDelegate(ProjectTreeModel *model) : ProjectTreeModelDelegate(model), m_label(0), m_widget(0)
  {
  }
 
  SelectionDelegate::~SelectionDelegate()
  {
  }
    
  void SelectionDelegate::initStructure(GLWidget *widget, ProjectTreeItem *parent)
  {
    // save the widget
    m_widget = widget;

    // add the labels
    m_label = insertExpandableItem(parent);
    m_label->setData(0, alias());
  }

  void SelectionDelegate::fetchMore(ProjectTreeItem *)
  {
    // the user has expanded our label, we now initialize the bond items
    // and keep track of the using the signals...
    
    disconnect(m_widget, 0, this, 0);
    // connect some signals to keep track of changes
    connect(m_widget, SIGNAL(namedSelectionsChanged()), this, SLOT(initialize()));
    
    initialize();
  }

  void SelectionDelegate::initialize()
  {
    // remove any existing rows
    if (m_label->childCount())
      model()->removeRows(m_label, 0, m_label->childCount());

    QList<QString> names = m_widget->namedSelections();
    // add the selectionss...
    model()->insertRows(m_label, 0, names.size());
    for (int i = 0; i < m_label->childCount(); ++i) {
      ProjectTreeItem *item = m_label->child(i);
      item->setData(0,   names.at(i));
      item->setData(1, QString("%1").arg(i));
      // set the primitive
      PrimitiveList primitives = m_widget->namedSelectionPrimitives(i);
      item->setPrimitives(primitives);
    }

  }
  
  void SelectionDelegate::writeSettings(QSettings &settings) const
  {
    ProjectTreeModelDelegate::writeSettings(settings);
  }
  
  void SelectionDelegate::readSettings(QSettings &settings)
  {
    ProjectTreeModelDelegate::readSettings(settings);
  }

} // end namespace Avogadro

#include "selectiondelegate.moc"
