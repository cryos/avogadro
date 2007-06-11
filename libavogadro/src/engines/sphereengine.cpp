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
  m_alpha(1.), m_bondRadius(0.1), m_atomRadiusPercentage(0.3)
{
  setName(tr("VdW Sphere"));
  setDescription(tr("Renders atoms as Van der Waals spheres"));
}

bool SphereEngine::render(GLWidget *gl)
{
  m_glwidget = gl;
  m_glwidget->painter()->begin(m_glwidget);

  QList<Primitive *> list;

  // First we render the skeleton if the VdW spheres are not opaque
  if (m_alpha < 1.0)
  {
    list = primitives().subList(Primitive::BondType);
    foreach( Primitive *p, list )
      renderSkeleton(static_cast<const Bond *>(p));

    glDisable( GL_NORMALIZE );
    glEnable( GL_RESCALE_NORMAL );
    list = primitives().subList(Primitive::AtomType);
    foreach( Primitive *p, list )
      renderSkeleton(static_cast<const Atom *>(p));
  }
  // Render the 
  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );
  list = primitives().subList(Primitive::AtomType);
  foreach( Primitive *p, list )
  {
    render(static_cast<const Atom *>(p));
  }

  glDisable( GL_RESCALE_NORMAL);
  glEnable( GL_NORMALIZE );

  gl->painter()->end();
  return true;
}

inline double SphereEngine::radius(const Atom *a)
{
  return etab.GetVdwRad(a->GetAtomicNum());
}

double SphereEngine::radius(const Primitive *p)
{
  if (p->type() == Primitive::AtomType)
  {
    const Atom *a = static_cast<const Atom *>(p);
    double r = radius(a);
    if (m_glwidget)
    {
      if (m_glwidget->isSelected(p))
        return r + SEL_ATOM_EXTRA_RADIUS;
    }
    return r;
  }
  else
    return 0.;
}

bool SphereEngine::render(const Atom *a)
{
  Color map = colorMap();
  glPushName(Primitive::AtomType);
  glPushName(a->GetIdx());
  map.set(a);
  // Render this transparently
  map.setAlpha(m_alpha);
  map.applyAsMaterials();
  glEnable( GL_BLEND );
  m_glwidget->painter()->drawSphere( a->pos(), radius(a) );
  glDisable( GL_BLEND );
  map.setAlpha(0.0);
  map.applyAsMaterials();

  if (m_glwidget->isSelected(a))
    {
      map.set( 0.3, 0.6, 1.0, 0.7 );
      map.applyAsMaterials();
      glEnable( GL_BLEND );
      m_glwidget->painter()->drawSphere( a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a) );
      glDisable( GL_BLEND );
    }

  glPopName();
  glPopName();

  return true;
}

bool SphereEngine::renderSkeleton(const Atom *a)
{
  Color map = colorMap();
  map.set(a);
  map.applyAsMaterials();
  m_glwidget->painter()->drawSphere( a->pos(), radius(a) * m_atomRadiusPercentage);
  return true;
}

bool SphereEngine::renderSkeleton(const Bond* b)
{
  Color map = colorMap();

  const Atom* atom1 = static_cast<const Atom *>(b->GetBeginAtom());
  const Atom* atom2 = static_cast<const Atom *>(b->GetEndAtom());
  Vector3d v1 (atom1->pos());
  Vector3d v2 (atom2->pos());
  Vector3d d = v2 - v1;
  d.normalize();
  Vector3d v3 ( (v1 + v2 + d*(radius(atom1)*0.3-radius(atom2)*0.3)) / 2 );

  double shift = 0.15;
  int order = b->GetBO();

  map.set(atom1);
  map.applyAsMaterials();
  m_glwidget->painter()->drawMultiCylinder( v1, v3, m_bondRadius, order, shift );

  map.set(atom2);
  map.applyAsMaterials();
  m_glwidget->painter()->drawMultiCylinder( v3, v2, m_bondRadius, order, shift );

  return true;
}

void SphereEngine::setOpacity(int percent)
{
  m_alpha = 0.01 * percent;
  emit changed();
}

void SphereEngine::setAtomRadiusPercentage(int percent)
{
  m_atomRadiusPercentage = 0.1 * percent;
  emit changed();
}

void SphereEngine::setBondRadius(int value)
{
  m_bondRadius = value * 0.1;
  emit changed();
}

QWidget* SphereEngine::settingsWidget()
{
  if(!m_settingsWidget)
  {
    m_settingsWidget = new SphereSettingsWidget();
    connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
    connect(m_settingsWidget->atomRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(setAtomRadiusPercentage(int)));
    connect(m_settingsWidget->bondRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(setBondRadius(int)));
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
