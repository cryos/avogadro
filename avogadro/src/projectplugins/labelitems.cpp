/**********************************************************************
  LabelItems - Project Tree Items for labels.

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

#include "labelitems.h"

#include <avogadro/glwidget.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  LabelItems::LabelItems() : m_label(0)
  {
  }
 
  LabelItems::~LabelItems()
  {
  }
    
  void LabelItems::setupModelData(ProjectTreeModel *model, GLWidget *widget, ProjectTreeItem *parent)
  {
    // add the label
    int position = parent->childCount();
    model->insertRows(parent, position, 1);
    m_label = parent->child(position);
    m_label->setData(0, alias());
  }

  void LabelItems::writeSettings(QSettings &settings) const
  {
    ProjectPlugin::writeSettings(settings);
  }
  
  void LabelItems::readSettings(QSettings &settings)
  {
    ProjectPlugin::readSettings(settings);
  }

} // end namespace Avogadro

#include "labelitems.moc"

