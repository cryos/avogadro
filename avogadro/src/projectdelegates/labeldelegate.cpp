/**********************************************************************
  LabelDelegate - Project Tree Items for labels.

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

#include "labeldelegate.h"

#include <avogadro/glwidget.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  LabelDelegate::LabelDelegate(ProjectTreeModel *model) : ProjectTreeModelDelegate(model), m_label(0)
  {
  }
 
  LabelDelegate::~LabelDelegate()
  {
  }
    
  void LabelDelegate::initStructure(GLWidget *, ProjectTreeItem *parent)
  {
    // add the label
    m_label = insertExpandableItem(parent);
    m_label->setData(0, alias());
  }

  void LabelDelegate::writeSettings(QSettings &settings) const
  {
    ProjectTreeModelDelegate::writeSettings(settings);
  }
  
  void LabelDelegate::readSettings(QSettings &settings)
  {
    ProjectTreeModelDelegate::readSettings(settings);
  }

} // end namespace Avogadro

#include "labeldelegate.moc"

