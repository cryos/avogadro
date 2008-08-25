/**********************************************************************
  SuperCell - Build up surfaces from crystallographic unit cels

  Copyright (C) 2007-2008 by Geoffrey R. Hutchison

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

#include "supercellbuilder.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>

#include <QMessageBox>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  SuperCellBuilder::SuperCellBuilder(QObject *parent) : Extension(parent)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Build Super Cell..."));
    m_actions.append(action);
    m_Dialog = new SuperCellDialog(static_cast<QWidget*>(parent));
  }

  SuperCellBuilder::~SuperCellBuilder()
  {
  }

  QList<QAction *> SuperCellBuilder::actions() const
  {
    return m_actions;
  }

  QString SuperCellBuilder::menuPath(QAction *) const
  {
    return tr("&Build");
  }

  void SuperCellBuilder::setMolecule(Molecule *molecule)
  {
    m_Molecule = molecule;
  }

  QUndoCommand* SuperCellBuilder::performAction(QAction *, GLWidget *widget)
  {
    OBUnitCell *uc = NULL;
    if (m_Molecule && m_Molecule->HasData(OBGenericDataType::UnitCell)) {
      uc = dynamic_cast<OBUnitCell*>(m_Molecule->GetData(OBGenericDataType::UnitCell));
    } else { // no unit cell -- do nothing
      //TODO: Add a warning here
      return NULL;
    } // end if (existing unit cell or create a new one)

// update the miller indices?

    m_Dialog->exec();
    
    // What follows is a complete rotation for (hkl)[uvw]
    int h = m_Dialog->millerH();
    int k = m_Dialog->millerK();
    int l = m_Dialog->millerL();
    int u = 1;
    int v = 0;
    int w = 0;
    double m = sqrt(h*h + k*k + l*l);
    double n = sqrt(u*u + v*v + w*w);
    
    Eigen::Matrix3d millerRotation;
    // Down columns, then across
    millerRotation(0,0) = u/n;
    millerRotation(0,1) = v/n;
    millerRotation(0,2) = w/n;
    millerRotation(1,0) = (k*w - l*v)/(m*n);
    millerRotation(1,1) = (l*u - h*w)/(m*n);
    millerRotation(1,2) = (h*v - k*u)/(m*n);
    millerRotation(2,0) = h/m;
    millerRotation(2,1) = k/m;
    millerRotation(2,2) = l/m;
    
    Eigen::Transform3d modelview;
    const Eigen::Vector3d Zaxis(0.0,0.0,1.0);
    
    modelview.matrix().setIdentity();
    modelview.linear() = millerRotation;
    widget->camera()->setModelview(modelview);
    widget->camera()->pretranslate( -3.0 * ( widget->radius() ) * Zaxis );
    widget->camera()->translate( - widget->center());
    widget->camera()->normalize();

    return NULL;
  }

} // end namespace Avogadro

#include "supercellbuilder.moc"
Q_EXPORT_PLUGIN2(supercellbuilder, Avogadro::SuperCellBuilderFactory)
