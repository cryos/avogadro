/**********************************************************************
  BSDYEngine - Engine for "balls and sticks" display

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "config.h"
#include "bsdyengine.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtPlugin>
#include <QMessageBox>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;
using namespace Avogadro;

BSDYEngine::~BSDYEngine()
{
  int size = m_cylinders.size();
  for(int i=0; i<size; i++)
  {
    delete m_cylinders.takeLast();
  }
}

bool BSDYEngine::render(GLWidget *gl)
{
  Color map = colorMap();

  QList<Primitive *> list;

  if (!m_setup) {
    for(int i=0; i < 4; i++)
    {
      m_cylinders.append(new Cylinder(i * 3));
    }
    m_setup = true;
  }


  m_update = false;
  glPushAttrib(GL_TRANSFORM_BIT);
  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );
  list = queue().primitiveList(Primitive::AtomType);
  glPushName(Primitive::AtomType);
  foreach( Primitive *p, list ) {
    // FIXME: should be qobject_cast but bug with Qt/Mac
    Atom * a = dynamic_cast<Atom *>(p);
    glPushName(a->GetIdx());

    map.set(a);
    map.applyAsMaterials();

    gl->painter()->drawSphere( a->pos(), radius(a) );

    if (a->isSelected())
    {
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

  list = queue().primitiveList(Primitive::BondType);
  Eigen::Vector3d normalVector;
  if(gl) {
    normalVector = gl->normalVector();
  }
  Atom *atom1;
  Atom *atom2;
  foreach( Primitive *p, list ) {
    // FIXME: should be qobject_cast but bug with Qt/Mac
    Bond *b = dynamic_cast<Bond *>(p);

    atom1 = (Atom *) b->GetBeginAtom();
    atom2 = (Atom *) b->GetEndAtom();
    Vector3d v1 (atom1->pos());
    Vector3d v2 (atom2->pos());
    Vector3d v3 (( v1 + v2 ) / 2);

    double bondRadius = 0.1;
    double shift = 0.15;
    int order = b->GetBO();

    // for now, just allow selecting atoms
    //  glPushName(bondType);
    //  glPushName(b->GetIdx());
    double zDistance = gl->camera().distance(v3);
//     float zDistance = 200;
    int detail = 0;
    if(zDistance >= 100.0 && zDistance < 200.0)
    {
      detail = 1;
    }
    else if(zDistance >= 20.0 && zDistance < 100.0)
    {
      detail = 2;
    }
    else if(zDistance >= 0.0  && zDistance < 20.0)
    {
      detail = 3;
    }
    map.set(atom1);
    map.applyAsMaterials();
    m_cylinders.at(detail)->draw( v1, v3, bondRadius, order, shift, normalVector);

    map.set(atom2);
    map.applyAsMaterials();
    m_cylinders.at(detail)->draw( v3, v2, bondRadius, order, shift, normalVector);
    //  glPopName();
    //  glPopName();
  }

  glPopAttrib();

  return true;
}

inline double BSDYEngine::radius(const Atom *atom)
{
  return etab.GetVdwRad(atom->GetAtomicNum()) * 0.3;
}

double BSDYEngine::radius(const Primitive *primitive)
{
  if (primitive->type() == Primitive::AtomType) {
    double r = radius(static_cast<const Atom *>(primitive));
    if(primitive->isSelected())
    {
      return r + SEL_ATOM_EXTRA_RADIUS;
    }
    return r;
  } else {
    return 0.;
  }
}

bool BSDYEngine::render(const Atom *a)
{
  return true;
}

bool BSDYEngine::render(const Bond *b)
{
  return true;
}

bool BSDYEngine::render(const Molecule *m)
{
  // Disabled
  return false;
}

void BSDYEngine::addPrimitive(Primitive *primitive)
{
  Engine::addPrimitive(primitive);
  m_update = true;
}

void BSDYEngine::updatePrimitive(Primitive *primitive)
{
  Engine::updatePrimitive(primitive);
  m_update = true;
}

void BSDYEngine::removePrimitive(Primitive *primitive)
{
  Engine::removePrimitive(primitive);
  m_update = true;
}

void BSDYEngine::options()
{
  QMessageBox::StandardButton ret;
  ret = QMessageBox::information(qobject_cast<QWidget*>(parent()),
                                 tr("Avogadro"),
                                 tr("This will be for render options"),
                                 QMessageBox::Yes | QMessageBox::Cancel);
}

#include "bsdyengine.moc"

Q_EXPORT_PLUGIN2(bsdyengine, Avogadro::BSDYEngineFactory)
