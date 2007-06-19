/**********************************************************************
  SphereEngine - Engine for "spheres" display

  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2007      Benoit Jacob
  Copyright (C) 2007      Marcus D. Hanwell

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

#include "sphereengine.h"
#include <config.h>

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtPlugin>
#include <QMessageBox>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

SphereEngine::SphereEngine(QObject *parent) : Engine(parent), m_glwidget(0), m_settingsWidget(0),
  m_alpha(1.)
{
  setName(tr("VdW Sphere"));
  setDescription(tr("Renders atoms as Van der Waals spheres"));
}

SphereEngine::~SphereEngine()
{
  // Delete the settings widget if it exists
  if(m_settingsWidget) {
    m_settingsWidget->deleteLater();
  }
}

bool SphereEngine::renderOpaque(GLWidget *gl)
{
  m_glwidget = gl;
  m_glwidget->painter()->begin(m_glwidget);

  // Render the opaque spheres if m_alpha is 1
  if (m_alpha > 0.999)
  {
    QList<Primitive *> list;
    list = primitives().subList(Primitive::AtomType);
    // Render the atoms as VdW spheres
    glDisable( GL_NORMALIZE );
    glEnable( GL_RESCALE_NORMAL );
    foreach( Primitive *p, list )
      render(static_cast<const Atom *>(p));
    glDisable( GL_RESCALE_NORMAL);
    glEnable( GL_NORMALIZE );
  }

  gl->painter()->end();
  return true;
}

bool SphereEngine::renderTransparent(GLWidget *gl)
{
  m_glwidget = gl;
  m_glwidget->painter()->begin(m_glwidget);

  // If m_alpha is between 0 and 1 then render our transparent spheres
  if (m_alpha > 0.001 && m_alpha < 0.999)
  {
    QList<Primitive *> list;
    list = primitives().subList(Primitive::AtomType);

    // First pass using a colour mask - nothing is actually drawn
    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    foreach( Primitive *p, list )
      render(static_cast<const Atom *>(p));
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glEnable(GL_BLEND);
    glEnable(GL_LIGHTING);

    // Render the atoms as VdW spheres
    glDisable( GL_NORMALIZE );
    glEnable( GL_RESCALE_NORMAL );
    foreach( Primitive *p, list )
      render(static_cast<const Atom *>(p));

    glDisable( GL_RESCALE_NORMAL);
    glEnable( GL_NORMALIZE );
  }

  gl->painter()->end();
  return true;
}

bool SphereEngine::render(const Atom *a)
{
  // Render the atoms as Van der Waals spheres
  Color map = colorMap();
  glPushName(Primitive::AtomType);
  glPushName(a->GetIdx());
  map.set(a);
  map.setAlpha(m_alpha);
  map.applyAsMaterials();

  m_glwidget->painter()->drawSphere( a->pos(), radius(a) );

  // Draw a selection sphere if necessary
  if (m_glwidget->isSelected(a))
  {
    map.set( 0.3, 0.6, 1.0, 0.7 );
    map.applyAsMaterials();
    if (m_alpha > 0.999)
      glEnable( GL_BLEND );
    m_glwidget->painter()->drawSphere( a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a) );
    if (m_alpha > 0.999)
      glDisable( GL_BLEND );
  }

  glPopName();
  glPopName();

  return true;
}

inline double SphereEngine::radius(const Atom *a) const
{
  return etab.GetVdwRad(a->GetAtomicNum());
}

double SphereEngine::radius(const Primitive *p) const
{
  // Atom radius
  if (p->type() == Primitive::AtomType)
  {
    if (m_glwidget)
    {
      if (m_glwidget->isSelected(p))
        return radius(static_cast<const Atom *>(p)) + SEL_ATOM_EXTRA_RADIUS;
    }
    return radius(static_cast<const Atom *>(p));
  }
  // Something else
  else
    return 0.;
}

double SphereEngine::transparencyDepth() const
{
  return 1.0;
}

Engine::EngineFlags SphereEngine::flags() const
{
  return Engine::Transparent | Engine::Atoms;
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
  }
  return m_settingsWidget;
}

void SphereEngine::settingsWidgetDestroyed()
{
  qDebug() << "Destroyed Settings Widget";
  m_settingsWidget = 0;
}


#include "sphereengine.moc"

Q_EXPORT_PLUGIN2(sphereengine, SphereEngineFactory)
