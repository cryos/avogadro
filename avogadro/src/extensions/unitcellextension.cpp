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
    Action *action = new Action(this);
    action->setText("Unit Cell Parameters...");
    m_actions.append(action);
    m_Dialog = new UnitCellParamDialog(static_cast<QWidget*>(parent));

    connect(m_Dialog, SIGNAL(unitCellDisplayChanged(int, int, int)),
            this, SLOT(unitCellDisplayChanged(int, int, int)));
    connect(m_Dialog, SIGNAL(unitCellParametersChanged(double, double, double, double, double, double)),
            this, SLOT(unitCellParametersChanged(double, double, double, double, double, double)));
  }

  UnitCellExtension::~UnitCellExtension()
  {
  }

  QList<Action *> UnitCellExtension::actions() const
  {
    return m_actions;
  }

  QUndoCommand* UnitCellExtension::performAction(Action *,
                                                 Molecule *molecule,
                                                 GLWidget *widget,
                                                 QTextEdit *)
  {
    m_Molecule = molecule;
    m_Widget = widget;

    OBUnitCell *uc = NULL;
    if (molecule && molecule->HasData(OBGenericDataType::UnitCell)) {
      uc = dynamic_cast<OBUnitCell*>(molecule->GetData(OBGenericDataType::UnitCell));
    } else {
      // show warning and ask if the user wants to create a unit cell
      // (otherwise this extension isn't very useful)

      QMessageBox::StandardButton ret;
      ret = QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                                 tr("Avogadro"),
          tr("This document is currently an isolated molecule.\n\n"
            "Do you want to create a crystal unit cell?"),
                                 QMessageBox::Yes
                                 | QMessageBox::No);
      if (ret == QMessageBox::Yes) {
        // Set some initial data (e.g., a box about the size of the molecule)
        // and one unit cell in each direction
        uc = new OBUnitCell;
        double estimatedSize = widget->radius() + 2.0;
        uc->SetData(estimatedSize, estimatedSize, estimatedSize,
                    90.0, 90.0, 90.0);
        molecule->SetData(uc);

        widget->setUnitCells(1, 1, 1);
      } else { // do nothing -- user picked "Cancel"
        return NULL;
      }

    } // end if (existing unit cell or create a new one)

    m_Dialog->aCells(widget->aCells());
    m_Dialog->bCells(widget->bCells());
    m_Dialog->cCells(widget->cCells());

    m_Dialog->aLength(uc->GetA());
    m_Dialog->bLength(uc->GetB());
    m_Dialog->cLength(uc->GetC());

    m_Dialog->alpha(uc->GetAlpha());
    m_Dialog->beta(uc->GetBeta());
    m_Dialog->gamma(uc->GetGamma());

    m_Dialog->show();

    return NULL;
  }

  void UnitCellExtension::unitCellDisplayChanged(int a, int b, int c)
  {
    if (m_Widget) {
      m_Widget->setUnitCells(a, b, c);
    }
  }

  void UnitCellExtension::unitCellParametersChanged(double a, double b, double c,
                                     double alpha, double beta, double gamma)
  {
    if (m_Molecule) {
      OBUnitCell *uc = NULL;
      if (m_Molecule && m_Molecule->HasData(OBGenericDataType::UnitCell)) {
        uc = dynamic_cast<OBUnitCell*>(m_Molecule->GetData(OBGenericDataType::UnitCell));
        uc->SetData(a, b, c, alpha, beta, gamma);

        if (m_Widget)
          m_Widget->update();

      } // end if unit cell
    } // end if molecule
  } // end parameters changed

} // end namespace Avogadro

#include "unitcellextension.moc"
Q_EXPORT_PLUGIN2(unitcellextension, Avogadro::UnitCellExtensionFactory)
