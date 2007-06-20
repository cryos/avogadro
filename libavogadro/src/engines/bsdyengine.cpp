/**********************************************************************
  BSDYEngine - Dynamic detail engine for "balls and sticks" display

  Copyright (C) 2007 Donald Ephraim Curtis

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

#include <config.h>
#include "bsdyengine.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QMessageBox>
#include <QSlider>
#include <QtPlugin>
#include <QVBoxLayout>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;
using namespace Avogadro;

// our sort function
Camera *camera = 0;
bool sortCameraFarthest(const Primitive* lhs, const Primitive* rhs)
{
  if(!lhs)
  {
    if(rhs) {
      return true;
    }
    else
    {
      return false;
    }
  }

  if(lhs->type() == Primitive::BondType && rhs->type() == Primitive::BondType)
  {
    if(camera)
    {
      const Bond *l = static_cast<const Bond *>(lhs);
      const Bond *r = static_cast<const Bond *>(rhs);

      const Atom* latom1 = static_cast<const Atom *>(l->GetBeginAtom());
      const Atom* latom2 = static_cast<const Atom *>(l->GetEndAtom());
      Vector3d lv1 (latom1->pos());
      Vector3d lv2 (latom2->pos());
      Vector3d ld1 = lv2 - lv1;
      ld1.normalize();

      const Atom* ratom1 = static_cast<const Atom *>(r->GetBeginAtom());
      const Atom* ratom2 = static_cast<const Atom *>(r->GetEndAtom());
      Vector3d rv1 (ratom1->pos());
      Vector3d rv2 (ratom2->pos());
      Vector3d rd1 = rv2 - rv1;
      return camera->distance(ld1) >= camera->distance(rd1);
    }
  }
  else if(lhs->type() == Primitive::AtomType && rhs->type() == Primitive::AtomType)
  {
    if(camera)
    {
      const Atom *l = static_cast<const Atom*>(lhs);
      const Atom *r = static_cast<const Atom*>(rhs);
      return camera->distance(l->pos()) >= camera->distance(r->pos());
    }
  }
  return false;
}

BSDYEngine::BSDYEngine(QObject *parent) : Engine(parent),
  m_settingsWidget(0), m_atomRadiusPercentage(0.3), m_bondRadius(0.1)
{
  setName(tr("Dynamic Ball and Stick"));

  setDescription(tr("Renders primitives using Balls (atoms) and Sticks (bonds).  Includes demonstration of dynamic rendering based on distance from camera"));

}

BSDYEngine::~BSDYEngine()
{
  if(m_settingsWidget) {
    m_settingsWidget->deleteLater();
  }

}

bool BSDYEngine::renderOpaque(GLWidget *gl)
{
  QList<Primitive *> list;

  gl->painter()->begin(gl);

  glPushAttrib(GL_TRANSFORM_BIT);

  Color map = colorMap();

  // Get a list of bonds and render them
  list = primitives().subList(Primitive::BondType);

  // push bond type
  glPushName(Primitive::BondType);
  foreach( Primitive *p, list )
  {
    const Bond *b = static_cast<const Bond *>(p);
    // push bond index
    glPushName(b->GetIdx()+1);

    const Atom* atom1 = static_cast<const Atom *>(b->GetBeginAtom());
    const Atom* atom2 = static_cast<const Atom *>(b->GetEndAtom());
    Vector3d v1 (atom1->pos());
    Vector3d v2 (atom2->pos());
    Vector3d d = v2 - v1;
    d.normalize();
    Vector3d v3 ( (v1 + v2 + d*(radius(atom1)-radius(atom2))) / 2 );

    double shift = 0.15;
    int order = b->GetBO();

    map.set(atom1);
    map.applyAsMaterials();
    gl->painter()->drawMultiCylinder( v1, v3, m_bondRadius, order, shift );

    map.set(atom2);
    map.applyAsMaterials();
    gl->painter()->drawMultiCylinder( v3, v2, m_bondRadius, order, shift );

    glPopName();
  }
  glPopName();

  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );

  // Build up a list of the atoms and render them
  list = primitives().subList(Primitive::AtomType);
  glPushName(Primitive::AtomType);
  foreach( Primitive *p, list )
  {
    const Atom *a = static_cast<const Atom *>(p);

    Color map = colorMap();

  // Push the atom type and name
    glPushName(a->GetIdx());

    map.set(a);
    map.applyAsMaterials();

    gl->painter()->drawSphere( a->pos(), radius(a) );

    glPopName();
  }
  glPopName();

  // normalize normal vectors of bonds
  glDisable( GL_RESCALE_NORMAL);
  glEnable( GL_NORMALIZE );

  glPopAttrib();

  gl->painter()->end();
  return true;
}

bool BSDYEngine::renderTransparent(GLWidget *gl)
{
  QList<Primitive *> list;

  gl->painter()->begin(gl);

  camera = gl->camera();

  glPushAttrib(GL_TRANSFORM_BIT);

  Color map = colorMap();

  // Get a list of bonds and render them
  list = primitives().subList(Primitive::BondType);

  // sort our atom list
  qSort(list.begin(), list.end(), sortCameraFarthest);

  // enable depth mast for bonds
  glDepthMask(GL_TRUE);

  // push bond type
  glPushName(Primitive::BondType);
  foreach( Primitive *p, list )
  {
    const Bond *b = static_cast<const Bond *>(p);

    // Render the selection highlight
    if (gl->isSelected(b))
    {
      // push bond index
      glPushName(b->GetIdx()+1);

      const Atom* atom1 = static_cast<const Atom *>(b->GetBeginAtom());
      const Atom* atom2 = static_cast<const Atom *>(b->GetEndAtom());
      Vector3d v1 (atom1->pos());
      Vector3d v2 (atom2->pos());

      double shift = 0.15;
      int order = b->GetBO();

      map.set( 0.3, 0.6, 1.0, 0.7 );
      map.applyAsMaterials();
      glEnable( GL_BLEND );
      gl->painter()->drawMultiCylinder( v1, v2, SEL_BOND_EXTRA_RADIUS + m_bondRadius, order, shift );
      glDisable( GL_BLEND );
    }

    glPopName();
  }
  glPopName();

  glDepthMask(GL_FALSE);
  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );

  // Build up a list of the atoms and render them
  list = primitives().subList(Primitive::AtomType);

  // sort our atom list
  qSort(list.begin(), list.end(), sortCameraFarthest);

  glPushName(Primitive::AtomType);
  foreach( Primitive *p, list )
  {
    const Atom *a = static_cast<const Atom *>(p);

    // Render the selection highlight
    if (gl->isSelected(a))
    {
    // Push the atom type and name
      glPushName(a->GetIdx());
      map.set( 0.3, 0.6, 1.0, 0.7 );
      map.applyAsMaterials();
      glEnable( GL_BLEND );
      gl->painter()->drawSphere( a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a) );
      glDisable( GL_BLEND );
    }

    glPopName();
  }
  glPopName();

  // normalize normal vectors of bonds
  glDisable( GL_RESCALE_NORMAL);
  glEnable( GL_NORMALIZE );

  glPopAttrib();

  gl->painter()->end();
  return true;
}

inline double BSDYEngine::radius(const Atom *atom) const
{
  return etab.GetVdwRad(atom->GetAtomicNum()) * m_atomRadiusPercentage;
}

void BSDYEngine::setAtomRadiusPercentage(int percent)
{
  m_atomRadiusPercentage = 0.1 * percent;
  emit changed();
}

void BSDYEngine::setBondRadius(int value)
{
  m_bondRadius = value * 0.1;
  emit changed();
}

double BSDYEngine::radius(const GLWidget *gl, const Primitive *p) const
{
  // Atom radius
  if (p->type() == Primitive::AtomType)
  {
    if (gl)
    {
      if (gl->isSelected(p))
        return radius(static_cast<const Atom *>(p)) + SEL_ATOM_EXTRA_RADIUS;
    }
    return radius(static_cast<const Atom *>(p));
  }
  // Bond radius
  else if (p->type() == Primitive::BondType)
  {
    if (gl)
    {
      if (gl->isSelected(p))
        return m_bondRadius + SEL_BOND_EXTRA_RADIUS;
    }
    return m_bondRadius;
  }
  // Something else
  else
    return 0.;
}

double BSDYEngine::transparencyDepth() const
{
  return m_atomRadiusPercentage;
}

QWidget *BSDYEngine::settingsWidget()
{
  if(!m_settingsWidget)
  {
    m_settingsWidget = new BSDYSettingsWidget();
    connect(m_settingsWidget->atomRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(setAtomRadiusPercentage(int)));
    connect(m_settingsWidget->bondRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(setBondRadius(int)));
    connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
  }
  return m_settingsWidget;
}

void BSDYEngine::settingsWidgetDestroyed()
{
  qDebug() << "Destroyed Settings Widget";
  m_settingsWidget = 0;
}

Engine::EngineFlags BSDYEngine::flags() const
{
  return Engine::Transparent | Engine::Atoms | Engine::Bonds;
}

#include "bsdyengine.moc"

Q_EXPORT_PLUGIN2(bsdyengine, Avogadro::BSDYEngineFactory)
