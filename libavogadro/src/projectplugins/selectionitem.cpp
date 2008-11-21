/**********************************************************************
  selectionitem.h - Base class for ProjectItem plugins.

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

#include "selectionitem.h"

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  void SelectionItems::setupModelData(GLWidget *widget, ProjectItem *parent)
  {
    QList<QVariant> data;
    
    // add the label
    data << alias();
    ProjectItem *label = new ProjectItem(parent, data);
    parent->appendChild(label);

    // add the named selections
    for (int i = 0; i < widget->namedSelections().size(); ++i) 
    {
      data.clear();
      data << widget->namedSelections().at(i);
      ProjectItem *item = new ProjectItem(label, data);
      PrimitiveList primitives = widget->namedSelectionPrimitives(i);
      item->setPrimitives(primitives);
      label->appendChild(item);
    }
  }

  void SelectionItems::refresh()
  {
  
  }
 
  
} // end namespace Avogadro

#include "selectionitem.moc"

Q_EXPORT_PLUGIN2(selectionitem, Avogadro::SelectionItemsFactory)
