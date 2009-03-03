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
#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  VibrationExtension::VibrationExtension(QObject *parent) : Extension(parent),
                                                            m_molecule(NULL),
                                                            m_scale(1.0),
                                                            m_framesPerStep(8),
                                                            m_displayVectors(true),
                                                            m_animating(false)
  {
    QAction *action = new QAction( this );
    action->setSeparator(true);
    m_actions.append( action );

    action = new QAction(this);
    action->setText(tr("Vibrations..."));
    m_actions.append(action);

    m_widget =  static_cast<GLWidget*>(parent);
    m_animation = new Animation(parent);
    m_animation->setLoopCount(0); // continual loopback

    m_dialog = new VibrationDialog(m_widget);
    connect(m_dialog, SIGNAL(selectedMode(int)),
            this, SLOT(updateMode(int)));
    connect(m_dialog, SIGNAL(scaleUpdated(double)),
            this, SLOT(setScale(double)));
    connect(m_dialog, SIGNAL(setEnabledForceVector(bool)),
            this, SLOT(setDisplayForceVectors(bool)));
    connect(m_dialog, SIGNAL(toggleAnimation()),
            this, SLOT(toggleAnimation()));
  }

  VibrationExtension::~VibrationExtension()
  {
    if (m_dialog) {
      m_dialog->deleteLater();
    }
    if (m_animation) {
      m_animation->deleteLater();
    }

    clearAnimationFrames();
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
    if (m_animation)
      m_animation->setMolecule(molecule);

    // update m_vibrations
    if (!m_molecule)
      m_vibrations = NULL;
    clearAnimationFrames();
  }

  void VibrationExtension::updateMode(int mode)
  {
    qDebug() << " update Mode ";
    // animate the mode, add force arrows, etc.
    if (!m_molecule)
      return;

    // stop animating
    if (m_animating)
      m_animation->stop();

    if (mode == -1) {
      return;
    }

    OBMol obmol = m_molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));

    if (m_vibrations->GetLx().size() != 0) {

      vector<vector3> displacementVectors = m_vibrations->GetLx()[mode];
      // Sanity check
      if (displacementVectors.size() != m_molecule->numAtoms()) {
        QMessageBox::warning(m_widget, tr("Vibrational Analysis"), tr("The computed vibrations do not match this molecule."));
        return;
      }

      vector3 obDisplacement;
      Eigen::Vector3d displacement, atomPos;
      
      // delete any old frames
      clearAnimationFrames();
      for (unsigned int frame = 0; frame < m_framesPerStep * 4 - 1; ++frame)
        m_animationFrames.push_back( new vector<Vector3d>(m_molecule->numAtoms()) );

      if (m_displayVectors)
        enableForceDisplay();

      foreach (Atom *atom, m_molecule->atoms()) {
        obDisplacement = displacementVectors[atom->index()];
        displacement = Eigen::Vector3d(obDisplacement.x(), obDisplacement.y(), obDisplacement.z());

        if (m_displayVectors)
          atom->setForceVector(displacement);

        // We'll create frames for 4 "steps"
        // 1) current coordinates -> + displacement
        // 2)  + displacement -> original coords
        // 3) original coords ->  - displacement
        // 4)  - displacement -> original coords
        for (unsigned int frame = 0; frame < m_framesPerStep; ++frame) {
          atomPos = *(atom->pos());
          m_animationFrames[frame]->at(atom->index()) = atomPos + displacement * (m_scale * frame/m_framesPerStep);
          m_animationFrames[frame + 1*m_framesPerStep]->at(atom->index()) = atomPos + displacement * ( m_scale * (m_framesPerStep - frame) / m_framesPerStep);
          m_animationFrames[frame + 2*m_framesPerStep]->at(atom->index()) = atomPos - displacement * (m_scale * frame / m_framesPerStep);
          if (frame < m_framesPerStep - 1) // skip last frame here
            m_animationFrames[frame + 3*m_framesPerStep]->at(atom->index()) = atomPos - displacement * (m_scale * (m_framesPerStep - frame) / m_framesPerStep);
        }
      }
      m_animation->setFrames(m_animationFrames);
      if (m_animating)
        m_animation->start();
      m_molecule->update();
    } else {
      if (m_widget)
        QMessageBox::warning(m_widget, tr("Vibrational Analysis"), tr("No vibrational displacements exist."));
    }
  }

  QUndoCommand* VibrationExtension::performAction( QAction *, GLWidget *widget )
  {
    if (m_molecule == NULL)
      return NULL;

    m_widget = widget; // save for warnings in updateMode()
    
    OBMol obmol = m_molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));

    if (m_vibrations)
      m_dialog->show();
    else {
      QMessageBox::warning(widget, tr("Vibrational Analysis"), tr("No vibrations have been computed for this molecule."));
      // show a warning
    }

    return NULL;
  }

  void VibrationExtension::enableForceDisplay()
  {
    if (!m_widget)
      return;

    foreach (Engine *engine, m_widget->engines()) {
      if (engine->identifier() == "Force") {
        engine->setEnabled(true);
      }
    }
    m_widget->update();
  }

  void VibrationExtension::setScale(double scale)
  {
    m_scale = scale;
    // need to update arrows and animation
  }

  void VibrationExtension::setDisplayForceVectors(bool enabled)
  {
    m_displayVectors = enabled;
  }

  void VibrationExtension::toggleAnimation()
  {
    if (m_animationFrames.size() == 0) {
      m_dialog->animateButtonClicked(false);
      return;
    }

    m_animating = !m_animating;
    if (m_animating) {
      m_animation->start();
    }
    else {
      m_animation->stop();
    }
  }

  void VibrationExtension::clearAnimationFrames()
  {
    for (unsigned int frame = 0; frame < m_animationFrames.size(); ++frame)
      delete m_animationFrames[frame];
    m_animationFrames.clear();
  }

} // end namespace Avogadro

#include "vibrationextension.moc"
Q_EXPORT_PLUGIN2(vibrationextension, Avogadro::VibrationExtensionFactory)
