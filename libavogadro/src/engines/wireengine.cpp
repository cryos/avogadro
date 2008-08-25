/**********************************************************************
  WireEngine - Engine for wireframe display

  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2006-2007 Benoit Jacob

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "wireengine.h"

#include <config.h>
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <Eigen/Regression>

#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  WireEngine::WireEngine(QObject *parent) : Engine(parent), m_settingsWidget(NULL),
                                            m_showMulti(0), m_showDots(2)
  {
    setDescription(tr("Wireframe rendering"));
  }

  Engine* WireEngine::clone() const
  {
    WireEngine* engine = new WireEngine(parent());
    
    engine->setAlias(alias());
    engine->setShowDots(m_showDots);
    engine->setShowMultipleBonds(m_showMulti);
    engine->setEnabled(isEnabled());

    return engine;
  }

  bool WireEngine::renderOpaque(PainterDevice *pd)
  {
    QList<Primitive *> list;

    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    // Skip this entire step if the user turns it off
    if (m_showDots) {
      list = primitives().subList(Primitive::AtomType);
      foreach( Primitive *p, list ) {
        renderOpaque(pd, static_cast<const Atom *>(p));
      }
    }
    
    list = primitives().subList(Primitive::BondType);
    foreach( Primitive *p, list ) {
      renderOpaque(pd, static_cast<const Bond *>(p));
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_BLEND);

    return true;
  }

  bool WireEngine::renderOpaque(PainterDevice *pd, const Atom *a)
  {
    const Vector3d & v = a->pos();
    const Camera *camera = pd->camera();

    // perform a rough form of frustum culling
    Eigen::Vector3d transformedPos = pd->camera()->modelview() * v;
    double dot = transformedPos.z() / transformedPos.norm();
    if(dot > -0.8) return true;

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    glPushName(Primitive::AtomType);
    glPushName(a->GetIdx());

    // Compute a rough "dynamic" size for the atom dots
    // We could probably have a better gradient here, but it looks decent
    double size = 3.0; // default size;
    if (camera->distance(v) < 5.0)
      size = 5.0;
    else if (camera->distance(v) > 5.0 && camera->distance(v) < 10.0)
      size = 4.0;
    else if (camera->distance(v) > 40.0 && camera->distance(v) < 85.0)
      size = 2.0;
    else if (camera->distance(v) > 85.0)
      size = 1.5;

    // All dots are scaled by the VDW radius -- subtle, but effective
    if (pd->isSelected(a)) {
      map->setToSelectionColor();
      map->apply();
      glPointSize(etab.GetVdwRad(a->GetAtomicNum()) * (size + 1.0));
    }
    else {
      map->set(a);
      map->apply();
      glPointSize(etab.GetVdwRad(a->GetAtomicNum()) * size);
    }

    glBegin(GL_POINTS);
    glVertex3d(v.x(), v.y(), v.z());
    glEnd();

    glPopName(); // atom index
    glPopName(); // Primitive::AtomType

    return true;
  }

  inline double WireEngine::radius (const Atom *atom) const
  {
    return etab.GetVdwRad(atom->GetAtomicNum());
  }

  bool WireEngine::renderOpaque(PainterDevice *pd, const Bond *b)
  {
    const Atom *atom1 = static_cast<const Atom *>( b->GetBeginAtom() );
    const Vector3d & v1 = atom1->pos();
    const Camera *camera = pd->camera();

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    // perform a rough form of frustum culling
    Eigen::Vector3d transformedEnd1 = pd->camera()->modelview() * v1;
    double dot = transformedEnd1.z() / transformedEnd1.norm();
    if(dot > -0.8) return true; // i.e., don't bother rendering

    const Atom *atom2 = static_cast<const Atom *>( b->GetEndAtom() );
    const Vector3d & v2 = atom2->pos();
    Vector3d d = v2 - v1;
    d.normalize(); // compute the "transition point" between the two atoms
    Vector3d v3(( v1 + v2 + d*( radius( atom1 )-radius( atom2 ) ) ) / 2 );

    // Compute the width to draw the wireframe bonds
    double width = 1.0;
    double averageDistance = (camera->distance(v1) + camera->distance(v2)) / 2.0;
    if (averageDistance < 20.0 && averageDistance > 10.0)
      width = 1.5;
    else if (averageDistance < 10.0 && averageDistance > 5.0)
      width = 2.0;
    else if (averageDistance < 5.0)
      width = 2.5;

    int order = 1;
    if (m_showMulti) {
      order = b->GetBO();
      // For aromatic (dashed bonds) eventually
//       if (b->IsAromatic())
//         order = 5;
    }

    // optional line stipple to use for aromatic bonds
    //    int stipple = 0xF0F0;
    int stipple = 0xFFFF;

    map->set(atom1);
    pd->painter()->setColor(map);
    if (order > 1) pd->painter()->drawMultiLine(v1, v3, width, order, stipple);
    else pd->painter()->drawLine(v1, v3, width);

    map->set(atom2);
    pd->painter()->setColor(map);
    if (order > 1) pd->painter()->drawMultiLine(v2, v3, width, order, stipple);
    else pd->painter()->drawLine(v2, v3, width);

    return true;
  }

  void WireEngine::setShowMultipleBonds(int setting)
  {
    m_showMulti = setting;
    emit changed();
  }

  void WireEngine::setShowDots(int setting)
  {
    m_showDots = setting;
    emit changed();
  }

  QWidget* WireEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new WireSettingsWidget();
      connect(m_settingsWidget->showMultipleCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setShowMultipleBonds(int)));
      connect(m_settingsWidget->showDotsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setShowDots(int)));

      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
      
      m_settingsWidget->showDotsCheckBox->setCheckState((Qt::CheckState)m_showDots);
      m_settingsWidget->showMultipleCheckBox->setCheckState((Qt::CheckState)m_showMulti);
    }
    return m_settingsWidget;
  }

  void WireEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }
  
  void WireEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("showDots", m_showDots);
    settings.setValue("showMulti", m_showMulti);
  }

  void WireEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setShowDots(settings.value("showDots", 2).toInt());
    setShowMultipleBonds(settings.value("showMulti", 0).toInt());
    if (m_settingsWidget) {
      m_settingsWidget->showDotsCheckBox->setCheckState((Qt::CheckState)m_showDots);
      m_settingsWidget->showMultipleCheckBox->setCheckState((Qt::CheckState)m_showMulti);
    }
  }
}

#include "wireengine.moc"

Q_EXPORT_PLUGIN2(wireengine, Avogadro::WireEngineFactory)
