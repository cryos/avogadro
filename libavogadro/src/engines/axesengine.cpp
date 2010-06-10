/**********************************************************************
  AxesEngine - Engine to display the x, y, z axes

  Copyright (C) 2008      Marcus D. Hanwell
  Copyright (C) 2009      Konstantin L. Tokarev

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "axesengine.h"

#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/painterdevice.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace Eigen;

namespace Avogadro {

  AxesEngine::AxesEngine(QObject *parent) : Engine(parent), m_settingsWidget(0),
              m_axesType(0), m_preserveNorms(false),
              m_origin(0,0,0), m_axis1(1,0,0), m_axis2(0,1,0), m_axis3(0,0,1)
  {
  }

  AxesEngine::~AxesEngine()
  {
  }

  Engine* AxesEngine::clone() const
  {
    AxesEngine* engine = new AxesEngine(parent());

    engine->setAlias(alias());
    engine->setEnabled(isEnabled());
    return engine;
  }

  bool AxesEngine::renderOpaque(PainterDevice *pd)
  {
    // Right now just draw x, y, z axes one unit long. Will add more options.
    
    Vector3d aXa = m_origin+m_axis1*0.85; 
    Vector3d aX = m_origin+m_axis1; 
    Vector3d aYa = m_origin+m_axis2*0.85;
    Vector3d aY = m_origin+m_axis2; 
    Vector3d aZa = m_origin+m_axis3*0.85; 
    Vector3d aZ = m_origin+m_axis3; 
    // 1 axis
    pd->painter()->setColor(1.0, 0.0, 0.0);
    pd->painter()->drawCylinder(m_origin, aXa, 0.05);
    pd->painter()->drawCone(aXa, aX, 0.1);
    // 2 axis
    pd->painter()->setColor(0.0, 1.0, 0.0);
    pd->painter()->drawCylinder(m_origin, aYa, 0.05);
    pd->painter()->drawCone(aYa, aY, 0.1);
    // 3 axis
    pd->painter()->setColor(0.0, 0.0, 1.0);
    pd->painter()->drawCylinder(m_origin, aZa, 0.05);
    pd->painter()->drawCone(aZa, aZ, 0.1);

    return true;
  }

  double AxesEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return 0.;
  }

  double AxesEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::Layers AxesEngine::layers() const
  {
    return Engine::Overlay;
  }

  Engine::PrimitiveTypes AxesEngine::primitiveTypes() const
  {
    return Engine::NoPrimitives;
  }

  Engine::ColorTypes AxesEngine::colorTypes() const
  {
    return Engine::NoColors;
  }

  QWidget *AxesEngine::settingsWidget()
  {
    if(!m_settingsWidget)
      {
        /*
		 * Complex system of connects is used:
		 * - Vector norms are updated when user changes X,Y,Z
		 * - Coordinates are scaled when user changes norm
		 * - To prevent signal hell, disconnects are used inside code
		 * 
		 */
		 		
		m_settingsWidget = new AxesSettingsWidget();
		connect(m_settingsWidget->axesType, SIGNAL(currentIndexChanged(int)), this, SLOT(setAxesType(int)));
        connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
        connect(m_settingsWidget->x1SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateAxes(double)));
        connect(m_settingsWidget->y1SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateAxes(double)));
        connect(m_settingsWidget->z1SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateAxes(double)));
        connect(m_settingsWidget->x2SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateAxes(double)));
        connect(m_settingsWidget->y2SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateAxes(double)));
        connect(m_settingsWidget->z2SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateAxes(double)));
        connect(m_settingsWidget->x3SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateAxes(double)));
        connect(m_settingsWidget->y3SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateAxes(double)));
        connect(m_settingsWidget->z3SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateAxes(double)));
        connect(m_settingsWidget->xOriginSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateOrigin(double)));
        connect(m_settingsWidget->yOriginSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateOrigin(double)));
        connect(m_settingsWidget->zOriginSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateOrigin(double)));
        connect(m_settingsWidget->l1SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateValues1(double)));
        connect(m_settingsWidget->l2SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateValues2(double)));
        connect(m_settingsWidget->l3SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateValues3(double)));
        connect(m_settingsWidget->preserveNormsCheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(preserveNormsChanged(int)));
      }
    return m_settingsWidget;
  }

  void AxesEngine::setAxesType(int value)
  {
    m_axesType = value;
    updateAxes();

    switch (m_axesType) {
	  case 0:
        m_settingsWidget->x1SpinBox->setEnabled(false);
        m_settingsWidget->y1SpinBox->setEnabled(false);
        m_settingsWidget->z1SpinBox->setEnabled(false);
        m_settingsWidget->x2SpinBox->setEnabled(false);
        m_settingsWidget->y2SpinBox->setEnabled(false);
        m_settingsWidget->z2SpinBox->setEnabled(false);
        m_settingsWidget->x3SpinBox->setEnabled(false);
        m_settingsWidget->y3SpinBox->setEnabled(false);
        m_settingsWidget->z3SpinBox->setEnabled(false);
	    break;
	  
      case 1:
        m_settingsWidget->x1SpinBox->setEnabled(true);
        m_settingsWidget->y1SpinBox->setEnabled(true);
        m_settingsWidget->z1SpinBox->setEnabled(true);
        m_settingsWidget->x2SpinBox->setEnabled(true);
        m_settingsWidget->y2SpinBox->setEnabled(true);
        m_settingsWidget->z2SpinBox->setEnabled(true);
        m_settingsWidget->x3SpinBox->setEnabled(false);
        m_settingsWidget->y3SpinBox->setEnabled(false);
        m_settingsWidget->z3SpinBox->setEnabled(false);
	    break;

	  default:
        m_settingsWidget->x1SpinBox->setEnabled(true);
        m_settingsWidget->y1SpinBox->setEnabled(true);
        m_settingsWidget->z1SpinBox->setEnabled(true);
        m_settingsWidget->x2SpinBox->setEnabled(true);
        m_settingsWidget->y2SpinBox->setEnabled(true);
        m_settingsWidget->z2SpinBox->setEnabled(true);
        m_settingsWidget->x3SpinBox->setEnabled(true);
        m_settingsWidget->y3SpinBox->setEnabled(true);
        m_settingsWidget->z3SpinBox->setEnabled(true);
    }

    emit changed();
  }

  void AxesEngine::updateAxes(double)
  {
    updateVectors();

    if (m_preserveNorms) {
      if ((m_axis1.norm() == m_settingsWidget->l1SpinBox->value()) &&
          (m_axis2.norm() == m_settingsWidget->l2SpinBox->value()) &&
          (m_axis3.norm() == m_settingsWidget->l3SpinBox->value())) {
        emit changed();
        return;
      } else {
        updateValues1(m_settingsWidget->l1SpinBox->value());
        updateValues2(m_settingsWidget->l2SpinBox->value());
        updateValues3(m_settingsWidget->l3SpinBox->value());
        //updateVectors();
      }
    } else {
      // Recalculate norms
        disconnect(m_settingsWidget->l1SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateValues1(double)));
        disconnect(m_settingsWidget->l2SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateValues2(double)));
        disconnect(m_settingsWidget->l3SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateValues3(double)));

        m_settingsWidget->l1SpinBox->setValue(m_axis1.norm());
        m_settingsWidget->l2SpinBox->setValue(m_axis2.norm());
        m_settingsWidget->l3SpinBox->setValue(m_axis3.norm());
        
        connect(m_settingsWidget->l1SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateValues1(double)));
        connect(m_settingsWidget->l2SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateValues2(double)));
        connect(m_settingsWidget->l3SpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateValues3(double)));
    }
      
    emit changed();
  }

  void AxesEngine::updateVectors()
  {
    double l=0;
    switch(m_axesType)
    {
      case 0: // cartesian
        l = m_settingsWidget->x1SpinBox->value();
        m_axis1 = Vector3d(l,0,0);
        m_axis2 = Vector3d(0,l,0);
        m_axis3 = Vector3d(0,0,l);
        break;

      case 1: // orthogonal	    
        m_axis1 = Vector3d(m_settingsWidget->x1SpinBox->value(),
                           m_settingsWidget->y1SpinBox->value(),
                           m_settingsWidget->z1SpinBox->value());
        m_axis2 = Vector3d(m_settingsWidget->x2SpinBox->value(),
                           m_settingsWidget->y2SpinBox->value(),
                           m_settingsWidget->z2SpinBox->value());

		if (fabs(m_axis1.dot(m_axis2)) >= 1e-6) {
		  if (fabs(m_axis1.x()) >=1e-6) {
            m_axis2[0] = (-m_axis1.y()*m_axis2.y()-m_axis1.z()*m_axis2.z()) / m_axis1.x();
	      } else if (fabs(m_axis1.y()) >=1e-6) {
            m_axis2[1] = (-m_axis1.x()*m_axis2.x()-m_axis1.z()*m_axis2.z()) / m_axis1.y();
		  }	else if (fabs(m_axis1.z()) >=1e-6) {
            l = (-m_axis1.y()*m_axis2.y()-m_axis1.x()*m_axis2.x()) / m_axis1.z();
			m_axis2[2] = l;
          }		  
		}
		updateValues2(m_settingsWidget->l2SpinBox->value());
		m_axis3 = m_axis1.cross(m_axis2);
		updateValues3(m_settingsWidget->l3SpinBox->value());
      
      
      default: //
        m_axis1 = Vector3d(m_settingsWidget->x1SpinBox->value(),
                           m_settingsWidget->y1SpinBox->value(),
                           m_settingsWidget->z1SpinBox->value());
        m_axis2 = Vector3d(m_settingsWidget->x2SpinBox->value(),
                           m_settingsWidget->y2SpinBox->value(),
                           m_settingsWidget->z2SpinBox->value());
        m_axis3 = Vector3d(m_settingsWidget->x3SpinBox->value(),
                           m_settingsWidget->y3SpinBox->value(),
                           m_settingsWidget->z3SpinBox->value());
    }
  }
  
  void AxesEngine::updateOrigin(double)
  {
      m_origin = Vector3d(m_settingsWidget->xOriginSpinBox->value(),
                          m_settingsWidget->yOriginSpinBox->value(),
                          m_settingsWidget->zOriginSpinBox->value());
      emit changed();
  }
  
  void AxesEngine::updateValues1(double newNorm)
  {
    double k;
    disconnect(m_settingsWidget->x1SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    disconnect(m_settingsWidget->y1SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    disconnect(m_settingsWidget->z1SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));

    k = newNorm/m_axis1.norm();
    m_settingsWidget->x1SpinBox->setValue(m_axis1.x()*k);
    m_settingsWidget->y1SpinBox->setValue(m_axis1.y()*k);
    m_settingsWidget->z1SpinBox->setValue(m_axis1.z()*k);
    m_axis1 = Vector3d(m_axis1.x()*k,m_axis1.y()*k,m_axis1.z()*k);
    
    connect(m_settingsWidget->x1SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    connect(m_settingsWidget->y1SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    connect(m_settingsWidget->z1SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));

    emit changed();
  }
  
  void AxesEngine::updateValues2(double newNorm)
  {
    double k;
    disconnect(m_settingsWidget->x2SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    disconnect(m_settingsWidget->y2SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    disconnect(m_settingsWidget->z2SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));

    k = newNorm/m_axis2.norm();
    m_settingsWidget->x2SpinBox->setValue(m_axis2.x()*k);
    m_settingsWidget->y2SpinBox->setValue(m_axis2.y()*k);
    m_settingsWidget->z2SpinBox->setValue(m_axis2.z()*k);
    m_axis2 = Vector3d(m_axis2.x()*k,m_axis2.y()*k,m_axis2.z()*k);
    
    connect(m_settingsWidget->x2SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    connect(m_settingsWidget->y2SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    connect(m_settingsWidget->z2SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));

    emit changed();
  }

  void AxesEngine::updateValues3(double newNorm)
  {
    double k;
    disconnect(m_settingsWidget->x3SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    disconnect(m_settingsWidget->y3SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    disconnect(m_settingsWidget->z3SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));

    k = newNorm/m_axis3.norm();
    m_settingsWidget->x3SpinBox->setValue(m_axis3.x()*k);
    m_settingsWidget->y3SpinBox->setValue(m_axis3.y()*k);
    m_settingsWidget->z3SpinBox->setValue(m_axis3.z()*k);
    m_axis3 = Vector3d(m_axis3.x()*k,m_axis3.y()*k,m_axis3.z()*k);
    
    connect(m_settingsWidget->x3SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    connect(m_settingsWidget->y3SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));
    connect(m_settingsWidget->z3SpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateAxes(double)));

    emit changed();
  }

  void AxesEngine::preserveNormsChanged(int value)
  {
    if (value == Qt::Checked) {
      m_settingsWidget->l1SpinBox->setEnabled(false);
      m_settingsWidget->l2SpinBox->setEnabled(false);
      m_settingsWidget->l3SpinBox->setEnabled(false);
      m_preserveNorms = true;
    } else {
      m_settingsWidget->l1SpinBox->setEnabled(true);
      m_settingsWidget->l2SpinBox->setEnabled(true);
      m_settingsWidget->l3SpinBox->setEnabled(true);
      m_preserveNorms = false;
    }
  }
  
  void AxesEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  void AxesEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    //settings.setValue("atomLabel", m_atomType);
    //settings.setValue("bondLabel", m_bondType);
  }

  void AxesEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    //setAtomType(settings.value("atomLabel", 1).toInt());
    //setBondType(settings.value("bondLabel", 0).toInt());
    if(m_settingsWidget) {
      //m_settingsWidget->atomType->setCurrentIndex(m_atomType);
      //m_settingsWidget->bondType->setCurrentIndex(m_bondType);
    }

  }
}

Q_EXPORT_PLUGIN2(axesengine, Avogadro::AxesEngineFactory)
