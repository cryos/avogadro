/**********************************************************************
  VibrationExtension - Visualize vibrational modes from QM calculations

  Copyright (C) 2009 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2010 by Konstantin Tokarev

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

#include <avogadro/pluginmanager.h>
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/engine.h>

#include <openbabel/generic.h>
#include <openbabel/mol.h>

#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtCore/QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  class VibrationDock : public QDockWidget
  {
  public:
    VibrationDock( const QString & title, QWidget * parent = 0,
      Qt::WindowFlags flags = 0 ) : QDockWidget(title, parent, flags) {}
      
  protected: 
    void closeEvent ( QCloseEvent * event )
    {
      VibrationWidget *w = qobject_cast<VibrationWidget *>(widget());
      if (w)
        w->reject();
      event->accept();
    }
  };

  VibrationExtension::VibrationExtension(QObject *parent) : DockExtension(parent),
                                                            m_mode(-1),
                                                            m_dialog(0),
                                                            m_dock(0),
                                                            m_molecule(NULL),
                                                            m_widget(0),
                                                            m_animation(0),
                                                            m_scale(1.0),
                                                            m_framesPerStep(8),
                                                            m_normalize(true),
                                                            m_displayVectors(true),
                                                            m_animationSpeed(false),
                                                            m_animating(false),
                                                            m_paused(false),
                                                            m_geometry("")
  {
  }

  VibrationExtension::~VibrationExtension()
  {
    clearAnimationFrames();
  }

  QDockWidget * VibrationExtension::dockWidget()
  {
    if (!m_dock) {
      m_dock = new VibrationDock( tr("Vibrations"), qobject_cast<QWidget *>(parent()) );
      m_dock->setObjectName("vibrationDock");
      //qDebug() << "geom" << m_geometry.size();
      m_dock->restoreGeometry(m_geometry);
      //m_dock->setAllowedAreas(Qt::RightDockWidgetArea);
    
      if (!m_dialog) {
        m_dialog = new VibrationWidget();

        m_dialog->getUi()->normalizeDispCheckBox->setChecked(m_normalize);
        m_dialog->getUi()->displayForcesCheckBox->setChecked(m_displayVectors);
        m_dialog->getUi()->animationSpeedCheckBox->setChecked(m_animationSpeed);
        //m_dialog->setScale(m_scale);        
        
        connect(m_dialog, SIGNAL(selectedMode(int)),
                this, SLOT(updateMode(int)));
        connect(m_dialog, SIGNAL(scaleUpdated(double)),
                this, SLOT(setScale(double)));
        connect(m_dialog, SIGNAL(forceVectorUpdated(bool)),
                this, SLOT(setDisplayForceVectors(bool)));
        connect(m_dialog, SIGNAL(normalizeUpdated(bool)),
                this, SLOT(setNormalize(bool)));
        connect(m_dialog, SIGNAL(animationSpeedUpdated(bool)),
                this, SLOT(setAnimationSpeed(bool)));
        connect(m_dialog, SIGNAL(toggleAnimation()),
                this, SLOT(toggleAnimation()));
        connect(m_dialog, SIGNAL(pauseAnimation()),
                this, SLOT(pauseAnimation()));
        connect(m_dialog, SIGNAL(showSpectra()),
                this, SLOT(showSpectra()));
        m_dialog->setMolecule(m_molecule);
        m_animation = new Animation(this);
        m_animation->setLoopCount(0); // continual loopback
      }
    }
    m_dock->setWidget(m_dialog);
    m_dock->setVisible(false);
    return m_dock;
  }

  void VibrationExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    GLWidget *widget = GLWidget::current();        
    if (widget) {
      m_widget = widget; // engines, extensions, warnings in updateMode()
      foreach (Engine *engine, widget->engines()) {
        if (engine->identifier() == "Force") {
          m_dialog->setDisplayForceVectors(engine->isEnabled());
          connect(engine, SIGNAL(enableToggled(bool)), m_dialog, SLOT(setDisplayForceVectors(bool)));
        }
      }
    }
        
    if (m_dock) {
      if (molecule !=0) {
        if (molecule->OBMol().GetData(OBGenericDataType::VibrationData)) {
          //m_dock->show();
          m_dialog->setEnabled(true);
          if (!m_dock->toggleViewAction()->isChecked())
            m_dock->toggleViewAction()->activate(QAction::Trigger);
        }
        else {
          //m_dock->close();
          m_dialog->setEnabled(false);
          if (m_dock->toggleViewAction()->isChecked())
            m_dock->toggleViewAction()->activate(QAction::Trigger);
        }
      } else {
        m_dock->close();
          if (m_dock->toggleViewAction()->isChecked())
            m_dock->toggleViewAction()->activate(QAction::Trigger);
        m_dialog->setEnabled(false);
      }
      m_dialog->setMolecule(molecule);
    }
    if (m_animation)
      m_animation->setMolecule(molecule);

    // update m_vibrations
    if (!m_molecule) {
      m_vibrations = NULL;
      m_mode = -1;
    }
    clearAnimationFrames();
  }

  void VibrationExtension::updateMode(int mode)
  {
    if (!m_molecule)
      return; //nothing to do

    // stop animating
    if (m_animating) {
      m_animation->stop();
    }

    if (mode == -1) {
      return; // signal to end updates
    }

    OBMol obmol = m_molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));
    if (m_vibrations == NULL)
      return; // e.g., when destroying the molecule;

    if (m_vibrations->GetLx().size() != 0 &&
        mode < static_cast<int>(m_vibrations->GetLx().size())) {
      m_mode = mode;
      updateForcesAndFrames();
    }
    else {
      if (m_widget) {
        QMessageBox::warning(m_widget, tr("Vibrational Analysis"),
                             tr("No vibrational displacements exist."));
      }
    }
  }

  // animate the mode, add force arrows, etc.
  void VibrationExtension::updateForcesAndFrames()
  {
    if (m_mode == -1)
      return;
    // we do this check above
    /*
    if (m_vibrations == NULL 
        || m_vibrations->GetLx().size() == 0
        || m_mode > m_vibrations->GetLx().size())
      return;
    */

    vector<vector3> displacementVectors = m_vibrations->GetLx()[m_mode];
    // Sanity check
    // Currently disabled -- GAMESS does weird things.
    //    if (displacementVectors.size() != m_molecule->numAtoms()) {
    //      QMessageBox::warning(m_widget, tr("Vibrational Analysis"), tr("The computed vibrations do not match this molecule."));
    //      return;
    //    }

    vector3 obDisplacement;
    Eigen::Vector3d displacement, atomPos;
    double norm = 1;

    // delete any old frames
    clearAnimationFrames();
    // We do 4 "phases" of vibrations
    for (unsigned int frame = 0; frame < m_framesPerStep * 4; ++frame)
      m_animationFrames.push_back( new vector<Vector3d>(m_molecule->numAtoms()) );

    if (m_displayVectors)
      setDisplayForceVectors(true);

     if (m_normalize) {
       norm = 0;
       foreach (Atom *atom, m_molecule->atoms()) {
          obDisplacement = displacementVectors[atom->index()];
          displacement = Eigen::Vector3d(obDisplacement.x(), obDisplacement.y(), obDisplacement.z());      
          norm += displacement.norm();
        }
      }

    foreach (Atom *atom, m_molecule->atoms()) {
      obDisplacement = displacementVectors[atom->index()];
      displacement = Eigen::Vector3d(obDisplacement.x(), obDisplacement.y(), obDisplacement.z());      

      if (m_normalize) {        
        displacement /= norm;
      }

      if (m_displayVectors)
        atom->setForceVector(displacement*5);

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
        m_animationFrames[frame + 3*m_framesPerStep]->at(atom->index()) = atomPos - displacement * (m_scale * (m_framesPerStep - frame) / m_framesPerStep);
      } // foreach frame set

    } // foreach atom

    // and we remove the first frame (duplicate)
    m_animationFrames.erase(m_animationFrames.begin());
    m_animation->setFrames(m_animationFrames);
    if (m_animationSpeed) {
      // vibrations per femtosecond
      // wavenumber * 3.0e10 cm/s * 1e-15 s/fs = 3e-5 fs-1
      if (m_mode < static_cast<int>(m_vibrations->GetFrequencies().size())) {
        double vibPerFs = m_vibrations->GetFrequencies()[m_mode] * 3.0e-5;
        // 10fs = 4000 cm-1 gets 1 second apparent vibration
        // fs-1 above * 10fs / 1 s => per second * frames = fps
        double fps = vibPerFs * 10.0;
        m_animation->setFps(fps * m_animationFrames.size());
        qDebug() << vibPerFs << " fps " << fps * m_animationFrames.size();
      }
    }
    if (m_animating && !m_paused) {
      m_animation->start();
    }
    m_molecule->update();
  }

  void VibrationExtension::setScale(double scale)
  {
    m_scale = scale;
    updateMode(m_mode);
  }

  void VibrationExtension::setNormalize(bool normalize)
  {
    m_normalize = normalize;
    updateMode(m_mode);
  }

  void VibrationExtension::setDisplayForceVectors(bool enabled)
  {
    if (m_displayVectors == enabled)
      return; // nothing to do
    

    m_displayVectors = enabled;
    foreach (Engine *engine, m_widget->engines()) {
      if (engine->identifier() == "Force") {
        engine->setEnabled(enabled);
      }
    }

    m_widget->update();
  }

  void VibrationExtension::setAnimationSpeed(bool enabled)
  {
    if (!m_widget || !m_animation)
      return;

    m_animationSpeed = enabled;
    updateMode(m_mode);
    m_widget->update();
  }

  void VibrationExtension::toggleAnimation()
  {
    QSettings settings;
    
    if (m_animationFrames.size() == 0) {
      m_dialog->animateButtonClicked(false);
      return;
    }

    m_animating = !m_animating;
    int q = m_widget->quality();
    if (m_animating) {
      if (m_widget->quickRender() && (q > 0))
        m_widget->setQuality(q-1);
      m_animation->start();
    }
    else {
      m_animation->stop();
      if (m_widget->quickRender())
        m_widget->setQuality(settings.value("quality", 2).toInt());
    }
  }

  void VibrationExtension::pauseAnimation()
  {
    QSettings settings;

    m_paused = !m_paused;
    int q = m_widget->quality();
    if (m_paused) {
      if (m_widget->quickRender())
        m_widget->setQuality(settings.value("quality", 2).toInt());
      m_animation->pause();
    }
    else {
      if (m_widget->quickRender() && (q > 0))
        m_widget->setQuality(q-1);
      m_animation->start();
    }
  }

  void VibrationExtension::clearAnimationFrames()
  {
    m_animationFrames.clear();
  }

  void VibrationExtension::showSpectra()
  {
    if(!m_molecule)
      return;
      
    PluginManager *plugins = PluginManager::instance();
    Extension * spectra = plugins->extension("Spectra", m_widget);
    if (!spectra)
      return;
    spectra->setMolecule(m_molecule);
    spectra->performAction(0, m_widget);
  } 
  
  void VibrationExtension::writeSettings(QSettings &settings) const
  {    
  	if (m_dock)
  	  settings.setValue("vibration/geometry", m_dock->saveGeometry());
    settings.setValue("vibration/normalize", m_normalize);
    settings.setValue("vibration/forces", m_displayVectors);
    settings.setValue("vibration/speed", m_animationSpeed);
  }
  
  void VibrationExtension::readSettings(QSettings &settings)
  {
  	m_geometry = settings.value("vibration/geometry").toByteArray();
    m_normalize = settings.value("vibration/normalize", true).toBool();
    m_displayVectors = settings.value("vibration/forces", true).toBool();
    m_animationSpeed = settings.value("vibration/speed", false).toBool();
    //m_scale = settings.value("vibration/scale").toDouble();
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(vibrationextension, Avogadro::VibrationExtensionFactory)
