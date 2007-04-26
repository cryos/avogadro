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

#include <avogadro/primitives.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;
using namespace Avogadro;

BSDYEngine::~BSDYEngine()
{
  int size = m_spheres.size();
  for(int i=0; i<size; i++)
  {
    delete m_spheres.takeLast();
  }
  size = m_cylinders.size();
  for(int i=0; i<size; i++)
  {
    delete m_cylinders.takeLast();
  }
}

bool BSDYEngine::render(GLWidget *gl)
{
  // make a DL for very far objects.  Cube on its side.
  if(!m_dl) {
    m_dl = glGenLists(1);
    double x = sqrt(.5);
    glNewList(m_dl, GL_COMPILE);
    glPushMatrix();
    glRotated(45, 1, 0, 0);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3d(0,1,0); glVertex3d(0,1,0);
    glNormal3d(-x,0,x); glVertex3d(-x,0,x);
    glNormal3d(x,0,x); glVertex3d(x,0,x);
    glNormal3d(x,0,-x); glVertex3d(x,0,-x);
    glNormal3d(-x,0,-x); glVertex3d(-x,0,-x);
    glNormal3d(-x,0,x); glVertex3d(-x,0,x);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3d(0,-1,0); glVertex3d(0,-1,0);
    glNormal3d(-x,0,x); glVertex3d(-x,0,x);
    glNormal3d(-x,0,-x); glVertex3d(-x,0,-x);
    glNormal3d(x,0,-x); glVertex3d(x,0,-x);
    glNormal3d(x,0,x); glVertex3d(x,0,x);
    glNormal3d(-x,0,x); glVertex3d(-x,0,x);
    glEnd();
    glPopMatrix();
    glEndList();

  }

  QList<Primitive *> list;

  if (!m_setup) {
    for(int i=0; i < 10; i++)
    {
      m_spheres.append(new Sphere(i+1));
    }
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

    Color(a).applyAsMaterials();

    double zDistance = gl->camera().distance(a->pos());
//     float zDistance = 200;
    int detail = 0;
    if(zDistance >= 0.0 && zDistance < 200.0)
    {
      if(zDistance >= 100.0 && zDistance < 200.0)
      {
        detail = 1 - (( static_cast<int>(zDistance) - 100) / 50);
      }
      else if(zDistance >= 20.0 && zDistance < 100.0)
      {
        detail = 5 - (( static_cast<int>(zDistance) - 20) / 20);
      }
      else
      {
        detail = 9 -  static_cast<int>(zDistance / 5);
      }
      m_spheres.at(detail)->draw(a->pos(), etab.GetVdwRad(a->GetAtomicNum()) * 0.3);
    }
    else
    {
      glPushMatrix();
      const Vector3d & loc = a->pos();
      glTranslated( loc[0], loc[1], loc[2] );
      double radius = etab.GetVdwRad(a->GetAtomicNum()) * 0.3;
      glScaled( radius, radius, radius );
      glCallList(m_dl);
      glPopMatrix();
    }

    if (a->isSelected())
    {
      Color( 0.3, 0.6, 1.0, 0.7 ).applyAsMaterials();
      glEnable( GL_BLEND );
      if(zDistance < 200.0)
      {
        m_spheres.at(detail)->draw(a->pos(), 0.10 + etab.GetVdwRad(a->GetAtomicNum()) * 0.3);
      }
      else
      {
        glPushMatrix();
        const Vector3d & loc = a->pos();
        glTranslated( loc[0], loc[1], loc[2] );
        double radius = 0.10 + etab.GetVdwRad(a->GetAtomicNum()) * 0.3;
        glScaled( radius, radius, radius );
        glCallList(m_dl);
        glPopMatrix();
      }
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

    double radius = 0.1;
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
    Color(atom1).applyAsMaterials();
    m_cylinders.at(detail)->draw( v1, v3, radius, order, shift, normalVector);

    Color(atom2).applyAsMaterials();
    m_cylinders.at(detail)->draw( v3, v2, radius, order, shift, normalVector);
    //  glPopName();
    //  glPopName();
  }

  glPopAttrib();

  return true;
}

bool BSDYEngine::render(const Atom *a)
{
  return true;
}

bool BSDYEngine::render(const Bond *b)
{
//   GLWidget *gl = qobject_cast<GLWidget *>(parent());
//   if(gl) {
//     normalVector = gl->normalVector();
//   }
// 
//   const OBAtom *atom1 = static_cast<const OBAtom *>( b->GetBeginAtom() );
//   const OBAtom *atom2 = static_cast<const OBAtom *>( b->GetEndAtom() );
// 
//   Vector3d v1 (atom1->GetVector().AsArray());
//   Vector3d v2 (atom2->GetVector().AsArray());
//   Vector3d v3 (( v1 + v2 ) / 2);
//   std::vector<double> rgb;
// 
//   double radius = 0.1;
//   double shift = 0.15;
//   int order = b->GetBO();
// 
//   // for now, just allow selecting atoms
//   //  glPushName(bondType);
//   //  glPushName(b->GetIdx());
//   Color(atom1).applyAsMaterials();
//   m_cylinder.draw( v1, v3, radius, order, shift, normalVector);
// 
//   Color(atom2).applyAsMaterials();
//   m_cylinder.draw( v3, v2, radius, order, shift, normalVector);
//   //  glPopName();
//   //  glPopName();
// 
  return true;
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

bool BSDYEngine::render(const Molecule *m)
{
  // Disabled
  return false;
}

#include "bsdyengine.moc"

Q_EXPORT_PLUGIN2(bsdyengine, Avogadro::BSDYEngineFactory)
