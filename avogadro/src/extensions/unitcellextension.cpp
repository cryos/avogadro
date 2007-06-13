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
#include <avogadro/primitive.h>
#include <avogadro/color.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {
  UnitCellExtension::UnitCellExtension(QObject *parent) : QObject(parent), m_Widget(NULL)
  {
    QAction *action = new QAction(this);
    action->setText("Unit Cell Parameters...");
    m_actions.append(action);
    m_Dialog = new UnitCellParamDialog(static_cast<QWidget*>(parent));
      
    connect(m_Dialog, SIGNAL(unitCellsChanged(int, int, int)), this, SLOT(unitCellsChanged(int, int, int)));
  }

  UnitCellExtension::~UnitCellExtension() 
  {
  }

  QList<QAction *> UnitCellExtension::actions() const
  {
    return m_actions;
  }

  QUndoCommand* UnitCellExtension::performAction(QAction *, 
                                                 Molecule *molecule,
                                                 GLWidget *widget,
                                                 QTextEdit *)
  {
    m_Widget = widget;
    m_Dialog->aCells(widget->aCells());
    m_Dialog->bCells(widget->bCells());
    m_Dialog->cCells(widget->cCells());
      
    m_Dialog->show();
      
    return NULL;
  }
    
  void UnitCellExtension::unitCellsChanged(int a, int b, int c)
  {
    if (m_Widget) {
      m_Widget->setUnitCells(a, b, c);
      m_Widget->update();
    }
  }

} // end namespace Avogadro

#include "unitcellextension.moc"
Q_EXPORT_PLUGIN2(unitcellextension, Avogadro::UnitCellExtensionFactory)
