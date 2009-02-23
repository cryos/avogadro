/**********************************************************************
  WireEngine - Engine for wireframe display

  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2006-2007 Benoit Jacob

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

#include "wireengine.h"

#include <config.h>
#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/painterdevice.h>

#include <openbabel/mol.h>

#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace Eigen;

namespace Avogadro {

  WireEngine::WireEngine(QObject *parent) : Engine(parent), m_settingsWidget(NULL),
                                            m_showMulti(0), m_showDots(2)
  {
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
    glDisable(GL_LIGHTING);

    // Skip this entire step if the user turns it off
    if (m_showDots) {
      foreach(Atom *a, atoms())
        renderOpaque(pd, a);
    }

    foreach(Bond *b, bonds())
      renderOpaque(pd, b);

    glEnable(GL_LIGHTING);

    return true;
  }

  bool WireEngine::renderOpaque(PainterDevice *pd, const Atom *a)
  {
    const Vector3d & v = *a->pos();
    const Camera *camera = pd->camera();

    // perform a rough form of frustum culling
    Eigen::Vector3d transformedPos = pd->camera()->modelview() * v;
    double dot = transformedPos.z() / transformedPos.norm();
    if(dot > -0.8) return true;

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    glPushName(Primitive::AtomType);
    glPushName(a->index());

    // Compute a rough "dynamic" size for the atom dots
    // We could probably have a better gradient here, but it looks decent
    double size = 3.0; // default size;
    if (camera->distance(v) < 5.0)
      size = 7.0;
    else if (camera->distance(v) > 5.0 && camera->distance(v) < 10.0)
      size = 4.0;
    else if (camera->distance(v) > 30.0 && camera->distance(v) < 60.0)
      size = 2.0;
    else if (camera->distance(v) > 60.0 && camera->distance(v) < 85.0)
      size = 1.5;
    else if (camera->distance(v) > 85.0)
      size = 1.0;

    // All dots are scaled by the VDW radius -- subtle, but effective
    if (pd->isSelected(a)) {
      map->setToSelectionColor();
      map->apply();
      glPointSize(OpenBabel::etab.GetVdwRad(a->atomicNumber()) * (size + 1.0));
    }
    else {
      map->set(a);
      map->apply();
      glPointSize(OpenBabel::etab.GetVdwRad(a->atomicNumber()) * size);
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
    return OpenBabel::etab.GetVdwRad(atom->atomicNumber());
  }

  bool WireEngine::renderOpaque(PainterDevice *pd, const Bond *b)
  {
    const Atom* atom1 = pd->molecule()->atomById(b->beginAtomId());
    const Vector3d & v1 = *atom1->pos();
    const Camera *camera = pd->camera();

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    // perform a rough form of frustum culling
    Eigen::Vector3d transformedEnd1 = pd->camera()->modelview() * v1;
    double dot = transformedEnd1.z() / transformedEnd1.norm();
    if(dot > -0.8) return true; // i.e., don't bother rendering

    const Atom* atom2 = pd->molecule()->atomById(b->endAtomId());
    const Vector3d & v2 = *atom2->pos();
    Vector3d d = v2 - v1;
    d.normalize();

    Vector3d v3;
    if (atom1->atomicNumber() != atom2->atomicNumber()) {
      // compute the "transition point" between the two atoms
      v3 = ( v1 + v2 + d*( radius( atom1 )-radius( atom2 ) ) ) / 2.0;
    }

    // Compute the width to draw the wireframe bonds
    double width = 1.0;
    double averageDistance = (camera->distance(v1) + camera->distance(v2)) / 2.0;
    if (averageDistance < 20.0 && averageDistance > 10.0)
      width = 1.5;
    else if (averageDistance < 10.0 && averageDistance > 5.0)
      width = 2.0;
    else if (averageDistance < 5.0)
      width = 2.5;

    // Default to single bond, no stipple
    short stipple = static_cast<short>(0xFFFF);
    int order = 1;
    if (m_showMulti) {
      order = b->order();
      if (order > 1)
        width *= order * 0.75; // make multiple bonds a litte thicker too
      // For aromatic (dashed bonds)
      if (b->isAromatic()) {
        order = -1;
	stipple = static_cast<short>(0xCCCC);
      }
    }

    map->set(atom1);
    pd->painter()->setColor(map);
    // if have two of the same atoms, just draw one line
    if (atom1->atomicNumber() == atom2->atomicNumber()) {
      if (order != 1) pd->painter()->drawMultiLine(v1, v2, width, order, stipple);
      else pd->painter()->drawLine(v1, v2, width);
      return true;
    }
    // otherwise, we draw a line to the "transition point", change color, etc.
    if (order != 1) pd->painter()->drawMultiLine(v1, v3, width, order, stipple);
    else pd->painter()->drawLine(v1, v3, width);

    map->set(atom2);
    pd->painter()->setColor(map);
    if (order != 1) pd->painter()->drawMultiLine(v3, v2, width, order, stipple);
    else pd->painter()->drawLine(v3, v2, width);

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
    setShowMultipleBonds(settings.value("showMulti", 1).toInt());
    if (m_settingsWidget) {
      m_settingsWidget->showDotsCheckBox->setCheckState((Qt::CheckState)m_showDots);
      m_settingsWidget->showMultipleCheckBox->setCheckState((Qt::CheckState)m_showMulti);
    }
  }
}

#include "wireengine.moc"

Q_EXPORT_PLUGIN2(wireengine, Avogadro::WireEngineFactory)
