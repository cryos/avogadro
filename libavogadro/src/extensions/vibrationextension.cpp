/**********************************************************************
  VibrationExtension - Visualize vibrational modes from QM calculations

  Copyright (C) 2009 by Geoffrey R. Hutchison

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

#include "vibrationextension.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/generic.h>
#include <openbabel/mol.h>

#include <QAction>
#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  VibrationExtension::VibrationExtension(QObject *parent) : Extension(parent),
                                                            m_molecule(NULL)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Vibrations..."));
    m_actions.append(action);

    m_dialog = new VibrationDialog(static_cast<QWidget*>(parent));
    connect(m_dialog, SIGNAL(selectedMode(int)),
            this, SLOT(updateMode(int)));

    action = new QAction( this );
    action->setSeparator(true);
    m_actions.append( action );
  }

  VibrationExtension::~VibrationExtension()
  {
    if (m_dialog) {
      m_dialog->deleteLater();
    }
  }

  QList<QAction *> VibrationExtension::actions() const
  {
    return m_actions;
  }

  QString VibrationExtension::menuPath(QAction *) const
  {
    return tr("&Extensions");
  }

  void VibrationExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    if (m_dialog)
      m_dialog->setMolecule(molecule);

    // update m_vibrations
    if (!m_molecule)
      m_vibrations = NULL;
  }

  void VibrationExtension::updateMode(int mode)
  {
    // animate the mode, add force arrows, etc.
    if (!m_molecule)
      return;

    if (mode == -1) {
      // stop animating
      return;
    }

    // Else, at least add some force arrows to atoms
    qDebug() << " mode: " << mode << " Lx size: ";

    OBMol obmol = m_molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));

    vector<vector3> displacementVectors = m_vibrations->GetLx()[mode];
    vector3 displacement;

    foreach (Atom *atom, m_molecule->atoms()) {
      displacement = displacementVectors[atom->index()];
      atom->setForceVector(Eigen::Vector3d(displacement.x(), displacement.y(), displacement.z()));
    }
    
  }

  QUndoCommand* VibrationExtension::performAction( QAction *, GLWidget *widget )
  {
    if (m_molecule == NULL)
      return NULL;
   
    if (m_vibrations)
      m_dialog->show();
    else {
      // show a warning
    }

    return NULL;
  }

} // end namespace Avogadro

#include "vibrationextension.moc"
Q_EXPORT_PLUGIN2(vibrationextension, Avogadro::VibrationExtensionFactory)
