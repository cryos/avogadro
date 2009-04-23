/**********************************************************************
  SphereEngine - Engine for "spheres" display

  Copyright (C) 2007-2008 Marcus D. Hanwell
  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2007      Benoit Jacob

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

#include "sphereengine.h"

#include <avogadro/primitive.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/painterdevice.h>

#include <QMessageBox>
#include <QDebug>

#include <openbabel/mol.h>

using namespace Eigen;

namespace Avogadro {

  SphereEngine::SphereEngine(QObject *parent) : Engine(parent), m_settingsWidget(0),
  m_alpha(1.0)
  {
  }

  SphereEngine::~SphereEngine()
  {
    // Delete the settings widget if it exists
    if(m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  Engine* SphereEngine::clone() const
  {
    SphereEngine* engine = new SphereEngine(parent());

    engine->setAlias(alias());
    engine->m_alpha = m_alpha;
    engine->setEnabled(isEnabled());
    return engine;
  }

  bool SphereEngine::renderOpaque(PainterDevice *pd)
  {
    // Render the opaque spheres if m_alpha is 1
    if (m_alpha >= 0.999)
    {
      // Render the atoms as VdW spheres
      glDisable(GL_NORMALIZE);
      glEnable(GL_RESCALE_NORMAL);
      foreach(Atom *a, atoms())
        render(pd, a);
      glDisable(GL_RESCALE_NORMAL);
      glEnable(GL_NORMALIZE);
    }
    return true;
  }

  bool SphereEngine::renderTransparent(PainterDevice *pd)
  {
    // If m_alpha is between 0 and 1 then render our transparent spheres
    if (m_alpha > 0.001 && m_alpha < 0.999)
    {
      // First pass using a colour mask - nothing is actually drawn
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glDisable(GL_LIGHTING);
      glDisable(GL_BLEND);
      // This is a little hackish but I am not sure there is a better way,
      // OpenGL requires this to cull the internal surfaces but it breaks POV-Ray
      // renders. So I set the color to black and totally transparent, render
      // with a slightly smaller radius than the actual VdW spheres. Works but
      // not pretty...
      pd->painter()->setColor(0.0, 0.0, 0.0, 1.0);
      foreach(Atom *a, atoms()) {
        pd->painter()->drawSphere(a->pos(), radius(a)*0.9999);
      }

      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glEnable(GL_BLEND);
      glEnable(GL_LIGHTING);

      // Render the atoms as VdW spheres
      glDisable(GL_NORMALIZE);
      glEnable(GL_RESCALE_NORMAL);

      foreach(Atom *a, atoms())
        render(pd, a);

      glDisable(GL_RESCALE_NORMAL);
      glEnable(GL_NORMALIZE);
    }

    // Render the selection sphere if required
    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map
    foreach(Atom *a, atoms()) {
      if (pd->isSelected(a)) {
        map->setToSelectionColor();
        pd->painter()->setColor(map);
        pd->painter()->setName(a);
        pd->painter()->drawSphere(a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a));
      }
    }

    return true;
  }

  bool SphereEngine::renderQuick(PainterDevice *pd)
  {
    // Render the atoms as VdW spheres
    glDisable(GL_NORMALIZE);
    glEnable(GL_RESCALE_NORMAL);
    Color *map = colorMap();
    if (!map) map = pd->colorMap();

    foreach(Atom *a, atoms()) {
      map->set(a);
      pd->painter()->setColor(map);
      pd->painter()->setName(a);
      pd->painter()->drawSphere(a->pos(), radius(a));
    }

    glDisable(GL_RESCALE_NORMAL);
    glEnable(GL_NORMALIZE);
    return true;
  }

  bool SphereEngine::render(PainterDevice *pd, const Atom *a)
  {
    // Render the atoms as Van der Waals spheres
    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    map->set(a);
    map->setAlpha(m_alpha);
    pd->painter()->setColor(map);
    pd->painter()->setName(a);
    pd->painter()->drawSphere(a->pos(), radius(a));

    return true;
  }

  inline double SphereEngine::radius(const Atom *a) const
  {
    return OpenBabel::etab.GetVdwRad(a->atomicNumber());
  }

  double SphereEngine::radius(const PainterDevice *pd, const Primitive *p) const
  {
    // Atom radius
    if (p->type() == Primitive::AtomType)
    {
      if(primitives().contains(p))
      {
        if (pd && pd->isSelected(p))
        {
          return radius(static_cast<const Atom *>(p)) + SEL_ATOM_EXTRA_RADIUS;
        }
        return radius(static_cast<const Atom *>(p));
      }
    }
    // Something else
    return 0.;
  }

  double SphereEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::Layers SphereEngine::layers() const
  {
    return Engine::Opaque | Engine::Transparent;
  }

  Engine::PrimitiveTypes SphereEngine::primitiveTypes() const
  {
    return Engine::Atoms;
  }

  void SphereEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    emit changed();
  }

  QWidget* SphereEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new SphereSettingsWidget();
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
      m_settingsWidget->opacitySlider->setValue(20*m_alpha);
    }
    return m_settingsWidget;
  }

  void SphereEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  void SphereEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("opacity", 20*m_alpha);
  }

  void SphereEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setOpacity(settings.value("opacity", 20).toInt());
    if (m_settingsWidget) {
      m_settingsWidget->opacitySlider->setValue(20*m_alpha);
    }
  }

}

#include "sphereengine.moc"

Q_EXPORT_PLUGIN2(sphereengine, Avogadro::SphereEngineFactory)
