/**********************************************************************
  BSDYEngine - Dynamic detail engine for "balls and sticks" display

  Copyright (C) 2007 Donald Ephraim Curtis

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

#include "bsdyengine.h"

#include <avogadro/camera.h>
#include <avogadro/painter.h>
#include <avogadro/painterdevice.h>
#include <avogadro/color.h>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/obeigenconv.h>

#include <QtCore/QDebug>

#include <QtOpenGL/QGLWidget> // for OpenGL bits

#include <openbabel/mol.h>
#include <openbabel/math/vector3.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <limits>

using namespace std;
using namespace Eigen;

namespace Avogadro
{

// our sort function
/*  Camera *camera = 0;
  bool sortCameraFarthest( const Primitive* lhs, const Primitive* rhs )
  {
    if ( !lhs ) {
      if ( rhs ) {
        return true;
      } else {
        return false;
      }
    }

    if ( lhs->type() == Primitive::BondType && rhs->type() == Primitive::BondType ) {
      if ( camera ) {
        const Bond *l = static_cast<const Bond *>( lhs );
        const Bond *r = static_cast<const Bond *>( rhs );

        const Atom* latom1 = static_cast<const Atom *>( l->GetBeginAtom() );
        const Atom* latom2 = static_cast<const Atom *>( l->GetEndAtom() );
        Vector3d lv1( latom1->pos() );
        Vector3d lv2( latom2->pos() );
        Vector3d ld1 = lv2 - lv1;
        ld1.normalize();

        const Atom* ratom1 = static_cast<const Atom *>( r->GetBeginAtom() );
        const Atom* ratom2 = static_cast<const Atom *>( r->GetEndAtom() );
        Vector3d rv1( ratom1->pos() );
        Vector3d rv2( ratom2->pos() );
        Vector3d rd1 = rv2 - rv1;
        return camera->distance( ld1 ) >= camera->distance( rd1 );
      }
    } else if ( lhs->type() == Primitive::AtomType && rhs->type() == Primitive::AtomType ) {
      if ( camera ) {
        const Atom *l = static_cast<const Atom*>( lhs );
        const Atom *r = static_cast<const Atom*>( rhs );
        return camera->distance( l->pos() ) >= camera->distance( r->pos() );
      }
    }
    return false;
  } */


  // Protect globally declared functions in an anonymous namespace
  namespace
  {
    double radiusCovalent(const Atom *atom)
    {
      return OpenBabel::etab.GetCovalentRad(atom->atomicNumber());
    }

    double radiusVdW(const Atom *atom)
    {
      return OpenBabel::etab.GetVdwRad(atom->atomicNumber());
    }
  } // End of anonymous namespace


  BSDYEngine::BSDYEngine(QObject *parent) : Engine(parent),
      m_settingsWidget(0), m_atomRadiusPercentage(0.3), m_atomRadiusScale(50.0),
      m_bondRadius(0.1), m_bondRadiusScale(40.0),
      m_atomRadiusType(1), m_showMulti(2), m_alpha(1.), pRadius(radiusVdW)
  {  }

  Engine *BSDYEngine::clone() const
  {
    BSDYEngine *engine = new BSDYEngine(parent());
    engine->setAlias(alias());
    engine->m_atomRadiusPercentage = m_atomRadiusPercentage;
    engine->m_bondRadius = m_bondRadius;
    engine->m_showMulti = m_showMulti;
    engine->m_atomRadiusType = m_atomRadiusType;
    engine->m_alpha = m_alpha;
    engine->setEnabled(isEnabled());

    return engine;
  }

  BSDYEngine::~BSDYEngine()
  {
    if ( m_settingsWidget ) {
      m_settingsWidget->deleteLater();
    }
  }

  bool BSDYEngine::renderOpaque( PainterDevice *pd )
  {
    // Render the opaque balls & sticks if m_alpha is 1
    if (m_alpha < 0.999) {
      return true;
    }
    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    // Render the bonds. If there is no unit cell, render as intracell
    OpenBabel::OBUnitCell *cell = m_molecule->OBUnitCell();
    if (cell == NULL) {
      foreach(const Bond *b, bonds()) {
        this->renderIntracellBond(b, pd, map);
      }
    }
    // Otherwise, test to see if each bond crosses the unit cell bounds.
    // If it does not, render intracell.
    // If it does, find the planes it crosses and render it intercell.
    else {
      // First, cache some cell data
      std::vector<OpenBabel::vector3> obvecs = cell->GetCellVectors();
      const Eigen::Vector3d v1 (obvecs.at(0).AsArray());
      const Eigen::Vector3d v2 (obvecs.at(1).AsArray());
      const Eigen::Vector3d v3 (obvecs.at(2).AsArray());
      const Eigen::Vector3d origin (cell->GetOffset().AsArray());
      const Eigen::Vector3d diagonal (origin + v1 + v2 + v3);
      const Eigen::Vector3d v1xv2 (v1.cross(v2).normalized());
      const Eigen::Vector3d v2xv3 (v2.cross(v3).normalized());
      const Eigen::Vector3d v3xv1 (v3.cross(v1).normalized());

      // Calculate the planes:
      Hyperplane<double, 3> planes[6];
      planes[0] = Hyperplane<double, 3> (v1xv2, origin);
      planes[1] = Hyperplane<double, 3> (v2xv3, origin);
      planes[2] = Hyperplane<double, 3> (v3xv1, origin);
      planes[3] = Hyperplane<double, 3> (v1xv2, diagonal);
      planes[4] = Hyperplane<double, 3> (v2xv3, diagonal);
      planes[5] = Hyperplane<double, 3> (v3xv1, diagonal);

      // data structures used in loop:
      Vector3d imageVector;
      Vector3d shortestBondVector;
      int validIntersections[3];
      double intParams[6];

      // Check each bond for intersections with the unit cell planes.
      foreach(const Bond *b, bonds()) {
        int numInts = this->findCellIntersections(b, imageVector,
                                                  shortestBondVector, cell,
                                                  planes, intParams,
                                                  validIntersections);

        if (numInts > 0) {
          this->renderIntercellBond(b, pd, map, cell, imageVector,
                                    shortestBondVector, planes, intParams,
                                    validIntersections, numInts);
        }
        else {
          this->renderIntracellBond(b, pd, map);
        }
      }
    }

    glDisable( GL_NORMALIZE );
    glEnable( GL_RESCALE_NORMAL );

    // Render the atoms
    foreach(const Atom *a, atoms()) {
      map->setFromPrimitive(a);
      if (a->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(a->customColorName());
      pd->painter()->drawSphere(a->pos(), radius(a));
    }

    // normalize normal vectors of bonds
    glDisable( GL_RESCALE_NORMAL );
    glEnable( GL_NORMALIZE );

    return true;
  }

  bool BSDYEngine::renderTransparent(PainterDevice *pd)
  {
    // Render selections when not renderquick
    Color *map = colorMap();
    if (!map) map = pd->colorMap();
    Color selectionMap;
    selectionMap.setToSelectionColor();

    glDisable( GL_NORMALIZE );
    glEnable( GL_RESCALE_NORMAL );
    foreach(const Atom *a, atoms()) {
      // First render the atom if it is transparent.
      if (m_alpha < 0.999 && m_alpha > 0.001) {
        map->setFromPrimitive(a);
        map->setAlpha(m_alpha);
        pd->painter()->setColor(map);
        pd->painter()->drawSphere(a->pos(), radius(a));
      }
      // If the atom is selected render the selection
      if (pd->isSelected(a)) {
        pd->painter()->setColor(&selectionMap);
        pd->painter()->drawSphere(a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a));
      }
    }

    glDisable( GL_RESCALE_NORMAL );
    glEnable( GL_NORMALIZE );
    // Render the bonds. If there is no unit cell, render as intracell
    OpenBabel::OBUnitCell *cell = m_molecule->OBUnitCell();
    if (cell == NULL) {
      foreach(const Bond *b, bonds()) {
        // If the bond is not selected and balls and sticks are opaque do not render it
        if (!pd->isSelected(b) && m_alpha > 0.999)
          continue;
        this->renderIntracellBond(b, pd, map);
      }
    }
    // Otherwise, test to see if each bond crosses the unit cell bounds.
    // If it does not, render intracell.
    // If it does, find the planes it crosses and render it intercell.
    else {
      // First, cache some cell data
      std::vector<OpenBabel::vector3> obvecs = cell->GetCellVectors();
      const Eigen::Vector3d v1 (obvecs.at(0).AsArray());
      const Eigen::Vector3d v2 (obvecs.at(1).AsArray());
      const Eigen::Vector3d v3 (obvecs.at(2).AsArray());
      const Eigen::Vector3d origin (cell->GetOffset().AsArray());
      const Eigen::Vector3d diagonal (origin + v1 + v2 + v3);
      const Eigen::Vector3d v1xv2 (v1.cross(v2).normalized());
      const Eigen::Vector3d v2xv3 (v2.cross(v3).normalized());
      const Eigen::Vector3d v3xv1 (v3.cross(v1).normalized());

      // Calculate the planes:
      Hyperplane<double, 3> planes[6];
      planes[0] = Hyperplane<double, 3> (v1xv2, origin);
      planes[1] = Hyperplane<double, 3> (v2xv3, origin);
      planes[2] = Hyperplane<double, 3> (v3xv1, origin);
      planes[3] = Hyperplane<double, 3> (v1xv2, diagonal);
      planes[4] = Hyperplane<double, 3> (v2xv3, diagonal);
      planes[5] = Hyperplane<double, 3> (v3xv1, diagonal);

      // data structures used in loop:
      Vector3d imageVector;
      Vector3d shortestBondVector;
      int validIntersections[3];
      double intParams[6];

      // Check each bond for intersections with the unit cell planes.
      foreach(const Bond *b, bonds()) {
        // If the bond is not selected and balls and sticks are opaque do not render it
        if (!pd->isSelected(b) && m_alpha > 0.999)
          continue;
        int numInts = this->findCellIntersections(b, imageVector,
                                                  shortestBondVector, cell,
                                                  planes, intParams,
                                                  validIntersections);

        if (numInts > 0) {
          this->renderIntercellBond(b, pd, map, cell, imageVector,
                                    shortestBondVector, planes, intParams,
                                    validIntersections, numInts);
          if (pd->isSelected(b)) {
            this->renderIntercellBond(b, pd, &selectionMap, cell, imageVector,
                                      shortestBondVector, planes, intParams,
                                      validIntersections, numInts,
                                      SEL_BOND_EXTRA_RADIUS);
          }
        }
        else {
          this->renderIntracellBond(b, pd, map);
          if (pd->isSelected(b)) {
            this->renderIntracellBond(b, pd, &selectionMap,
                                      SEL_BOND_EXTRA_RADIUS);
          }
        }
      }
    }

    return true;
  }

  bool BSDYEngine::renderQuick(PainterDevice *pd)
  {
    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map
    Color cSel;
    cSel.setToSelectionColor();

    // Render the bonds
    foreach(Bond *b, bonds()) {
      Atom* atom1 = pd->molecule()->atomById(b->beginAtomId());
      Atom* atom2 = pd->molecule()->atomById(b->endAtomId());
      Vector3d v1(*atom1->pos());
      Vector3d v2(*atom2->pos());
      Vector3d d = v2 - v1;
      d.normalize();
      Vector3d v3((v1 + v2 + d*(radius(atom1)-radius(atom2))) / 2);

      double shift = 0.15;
      int order = 1;
      if (m_showMulti) order = b->order();

      if (pd->isSelected(b)) {
        pd->painter()->setColor(&cSel);
        pd->painter()->drawMultiCylinder(v1, v2, SEL_BOND_EXTRA_RADIUS +
                                         m_bondRadius, order, shift);
      }
      else {
        map->setFromPrimitive(atom1);
        pd->painter()->setColor(map);
        pd->painter()->drawMultiCylinder(v1, v3, m_bondRadius, order, shift);

        map->setFromPrimitive( atom2 );
        pd->painter()->setColor(map);
        pd->painter()->drawMultiCylinder(v3, v2, m_bondRadius, order, shift);
      }
    }

    glDisable(GL_NORMALIZE);
    glEnable(GL_RESCALE_NORMAL);

    // Render the atoms
    foreach(Atom *a, atoms()) {
      if (pd->isSelected(a)) {
        pd->painter()->setColor(&cSel);
        pd->painter()->drawSphere(a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a));
      }
      else {
        map->setFromPrimitive(a);
        pd->painter()->setColor(map);
        pd->painter()->drawSphere(a->pos(), radius(a));
      }
    }

    // normalize normal vectors of bonds
    glDisable(GL_RESCALE_NORMAL);
    glEnable(GL_NORMALIZE);
    return true;
  }

  bool BSDYEngine::renderPick(PainterDevice *pd)
  {
    // Render the bonds. If there is no unit cell, render as intracell
    OpenBabel::OBUnitCell *cell = m_molecule->OBUnitCell();
    const double bondPickPadding = 0.05;
    if (cell == NULL) {
      foreach(const Bond *b, bonds()) {
        pd->painter()->setName(b);
        this->renderIntracellBond(b, pd, NULL, bondPickPadding);
      }
    }
    // Otherwise, test to see if each bond crosses the unit cell bounds.
    // If it does not, render intracell.
    // If it does, find the planes it crosses and render it intercell.
    else {
      // First, cache some cell data
      std::vector<OpenBabel::vector3> obvecs = cell->GetCellVectors();
      const Eigen::Vector3d v1 (obvecs.at(0).AsArray());
      const Eigen::Vector3d v2 (obvecs.at(1).AsArray());
      const Eigen::Vector3d v3 (obvecs.at(2).AsArray());
      const Eigen::Vector3d origin (cell->GetOffset().AsArray());
      const Eigen::Vector3d diagonal (origin + v1 + v2 + v3);
      const Eigen::Vector3d v1xv2 (v1.cross(v2).normalized());
      const Eigen::Vector3d v2xv3 (v2.cross(v3).normalized());
      const Eigen::Vector3d v3xv1 (v3.cross(v1).normalized());

      // Calculate the planes:
      Hyperplane<double, 3> planes[6];
      planes[0] = Hyperplane<double, 3> (v1xv2, origin);
      planes[1] = Hyperplane<double, 3> (v2xv3, origin);
      planes[2] = Hyperplane<double, 3> (v3xv1, origin);
      planes[3] = Hyperplane<double, 3> (v1xv2, diagonal);
      planes[4] = Hyperplane<double, 3> (v2xv3, diagonal);
      planes[5] = Hyperplane<double, 3> (v3xv1, diagonal);

      // data structures used in loop:
      Vector3d imageVector;
      Vector3d shortestBondVector;
      int validIntersections[3];
      double intParams[6];

      // Check each bond for intersections with the unit cell planes.
      foreach(const Bond *b, bonds()) {
        pd->painter()->setName(b);
        int numInts = this->findCellIntersections(b, imageVector,
                                                  shortestBondVector, cell,
                                                  planes, intParams,
                                                  validIntersections);

        if (numInts > 0) {
          this->renderIntercellBond(b, pd, NULL, cell, imageVector,
                                    shortestBondVector, planes, intParams,
                                    validIntersections, numInts,
                                    bondPickPadding);
        }
        else {
          this->renderIntracellBond(b, pd, NULL, bondPickPadding);
        }
      }
    }

    // Render the atoms
    foreach(Atom *a, atoms())  {
      pd->painter()->setName(a);
      // add a slight "slop" factor to make it easier to pick
      // (e.g., during drawing)
      // heavy atoms get a bit more, hydrogens get a bit less
      if (a->atomicNumber() > 1)
        pd->painter()->drawSphere(a->pos(), radius(a) + 0.03);
      else
        pd->painter()->drawSphere(a->pos(), radius(a) - 0.06);
    }
    return true;
  }

  bool BSDYEngine::renderIntracellBond(const Bond *bond, PainterDevice *pd,
                                       Color *map, double extraRadius)
  {
    Atom* atom1 = pd->molecule()->atomById(bond->beginAtomId());
    Atom* atom2 = pd->molecule()->atomById(bond->endAtomId());
    if (!atom1 || !atom2) {
      qDebug() << "Invalid bond atom IDs" << bond->beginAtomId() << atom1
               << bond->endAtomId() << atom2 << "Bond" << bond->id();
      return true;
    }

    Vector3d v1(*atom1->pos());
    Vector3d v2(*atom2->pos());
    Vector3d d = v2 - v1;
    d.normalize();
    Vector3d v3((v1 + v2 + d*(radius(atom1) - radius(atom2))) * 0.5);

    double shift = 0.15;
    int order = 1;
    if (m_showMulti) order = bond->order();

    if (map != NULL) {
      map->setFromPrimitive(atom1);
      map->setAlpha(m_alpha);
      if (atom1->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(atom1->customColorName());
    }
    pd->painter()->drawMultiCylinder( v1, v3, m_bondRadius + extraRadius,
                                      order, shift );

    if (map != NULL) {
      map->setFromPrimitive(atom2);
      map->setAlpha(m_alpha);
      if (atom2->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(atom2->customColorName());
    }
    pd->painter()->drawMultiCylinder( v3, v2, m_bondRadius + extraRadius,
                                      order, shift );
    return true;
  }

  bool BSDYEngine::renderIntercellBond(const Bond *bond, PainterDevice *pd,
                                       Color *map,
                                       OpenBabel::OBUnitCell *cell,
                                       const Eigen::Vector3d &imageVector,
                                       const Eigen::Vector3d &shortestBondVector,
                                       Eigen::Hyperplane<double, 3> planes[6],
                                       double intParams[6],
                                       int validIntersections[3],
                                       int numValidIntersections,
                                       double extraRadius)
  {
    // Render as intracell if no intersections detected
    if (numValidIntersections == 0) {
      return this->renderIntracellBond(bond, pd, map, extraRadius);
    }

    // Get atom positions.
    Atom* atom1 = pd->molecule()->atomById(bond->beginAtomId());
    Atom* atom2 = pd->molecule()->atomById(bond->endAtomId());
    if (!atom1 || !atom2) {
      qDebug() << "Invalid bond atom IDs" << bond->beginAtomId() << atom1
               << bond->endAtomId() << atom2 << "Bond" << bond->id();
      return true;
    }

    const Vector3d &begPos (*atom1->pos());
    const Vector3d &endPos (*atom2->pos());

    // Determine clipping planes
    const Vector3d &normal1 (planes[validIntersections[0]].normal());
    const Vector3d origin1 (planes[validIntersections[0]].offset()
                            * -normal1);

    const int endClipPlaneIndex = numValidIntersections - 1;
    const Vector3d &normal2
        (planes[validIntersections[endClipPlaneIndex]].normal());
    const Vector3d origin2
        (planes[validIntersections[endClipPlaneIndex]].offset()
         * -normal2 - imageVector);

    // Render clipped bonds (atoms->boundary)
    if (map != NULL) {
      map->setFromPrimitive(atom1);
      map->setAlpha(m_alpha);
      if (atom1->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(atom1->customColorName());
    }
    pd->painter()->drawClippedCylinder(begPos, endPos + imageVector,
                                       m_bondRadius + extraRadius,
                                       normal1, origin1);

    if (map != NULL) {
      map->setFromPrimitive(atom2);
      map->setAlpha(m_alpha);
      if (atom2->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(atom2->customColorName());
    }
    pd->painter()->drawClippedCylinder(endPos, begPos - imageVector,
                                       m_bondRadius + extraRadius,
                                       -normal2, origin2);

    // We're done if there is only one valid intersection:
    if (numValidIntersections == 1)
      return true;

    // If there is more more than one valid intersection, we'll need to
    // stitch together a few more clipped cylinders to fill in gaps.
    // Eg. in 2D, the left figure has multiple intersections with the cell,
    // and the missing bits are quite obvious.
    //                                                                      //
    //  +---------+---------+     +--\------+-\-------+                     //
    //  |   \   O |  \    O |     |   \   O |  \    O |                     //
    //  |    O   \|   O    \|     |    O   \|   O    \|                     //
    //  |         |         |     \         \         \                     //
    //  |         |         |     |\        |\        |                     //
    //  +---------+---------+ --> +-\-------+-\-------+                     //
    //  |   \   O |  \    O |     |  \    O |  \    O |                     //
    //  |    O   \|   O    \|     |   O    \|   O    \|                     //
    //  |         |         |     \         \         \                     //
    //  |         |         |     |\        |\        |                     //
    //  +---------+---------+     +-\-------+-\-------+                     //
    //                                                                      //
    // First stitch together intersections at index 0 and 1
    int beginIndex = validIntersections[0];
    int endIndex = validIntersections[1];
    const Vector3d &normal3 (planes[endIndex].normal());
    const Vector3d origin3 (planes[endIndex].offset() * -normal3);

    // Find the center of the segment image that lies within the unit cell
    const Vector3d normalizedBondVector (shortestBondVector.normalized());
    const Vector3d origSegment1Center
        (begPos + 0.5 * (intParams[beginIndex] + intParams[endIndex])
         * normalizedBondVector);
    const Vector3d segment1Center =
        OB2Eigen(cell->WrapCartesianCoordinate(Eigen2OB(origSegment1Center)));
    const Vector3d segment1ImageVector (segment1Center - origSegment1Center);

    // Render clipped bonds (boundary->boundary)
    if (map != NULL) {
      map->setFromPrimitive(atom1);
      map->setAlpha(m_alpha);
      if (atom1->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(atom1->customColorName());
    }
    pd->painter()->drawClippedCylinder(segment1Center,
                                       begPos + segment1ImageVector,
                                       m_bondRadius + extraRadius,
                                       -normal1,
                                       origin1 + segment1ImageVector);

    if (map != NULL) {
      map->setFromPrimitive(atom2);
      map->setAlpha(m_alpha);
      if (atom2->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(atom2->customColorName());
    }
    pd->painter()->drawClippedCylinder(segment1Center,
                                       begPos + segment1ImageVector + shortestBondVector,
                                       m_bondRadius + extraRadius,
                                       -normal3,
                                       origin3 + segment1ImageVector);

    // We're done if there are only 2 intersections.
    if (numValidIntersections == 2)
      return true;

    // Otherwise stitch together intersections at index 1 and 2
    beginIndex = validIntersections[1];
    endIndex = validIntersections[2];
    const Vector3d &normal4 (planes[endIndex].normal());
    const Vector3d origin4 (planes[endIndex].offset() * -normal4);

    // Find the center of the segment image that lies within the unit cell
    const Vector3d origSegment2Center
        (begPos + 0.5 * (intParams[beginIndex] + intParams[endIndex])
         * normalizedBondVector);
    const Vector3d segment2Center =
        OB2Eigen(cell->WrapCartesianCoordinate(Eigen2OB(origSegment2Center)));
    const Vector3d segment2ImageVector (segment2Center - origSegment2Center);

    // Render clipped bonds (boundary->boundary)
    if (map != NULL) {
      map->setFromPrimitive(atom1);
      map->setAlpha(m_alpha);
      if (atom1->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(atom1->customColorName());
    }
    pd->painter()->drawClippedCylinder(segment2Center,
                                       begPos + segment2ImageVector,
                                       m_bondRadius + extraRadius,
                                       -normal3,
                                       origin3 + segment2ImageVector);

    if (map != NULL) {
      map->setFromPrimitive(atom2);
      map->setAlpha(m_alpha);
      if (atom2->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(atom2->customColorName());
    }
    pd->painter()->drawClippedCylinder(segment2Center,
                                       begPos + segment2ImageVector + shortestBondVector,
                                       m_bondRadius + extraRadius,
                                       -normal4,
                                       origin4 + segment2ImageVector);
    return true;
  }

  inline double BSDYEngine::radius(const Atom *atom) const
  {
    if (atom->customRadius())
      return atom->customRadius()* m_atomRadiusPercentage;
    else {
      if (atom->atomicNumber())
        return pRadius(atom) * m_atomRadiusPercentage;
    }
    return m_atomRadiusPercentage;
  }

  int BSDYEngine::findCellIntersections(const Bond *bond,
                                        Eigen::Vector3d &imageVector,
                                        Eigen::Vector3d &shortestBondVector,
                                        OpenBabel::OBUnitCell *cell,
                                        Eigen::Hyperplane<double, 3> planes[6],
                                        double intParams[6],
                                        int validIntersections[3])
  {
    const Vector3d &begPos = *bond->beginPos();
    const Vector3d &endPos = *bond->endPos();

    // Find the shortest bond vector
    const Vector3d origBondVector (endPos - begPos);

    const Matrix3d cellMatrixT = OB2Eigen(cell->GetCellMatrix()).transpose();
    Vector3d shift(0,0,0);
    double minLengthSq = numeric_limits<double>::max();

    // Loop through all neighboring unit cells
    Vector3d aTrans;
    Vector3d bTrans;
    Vector3d curImage;
    for (int aInd = -1; aInd <= 1; ++aInd) {
      aTrans = cellMatrixT.col(0) * aInd;
      for (int bInd = -1; bInd <= 1; ++bInd) {
        bTrans = cellMatrixT.col(1) * bInd;
        for (int cInd = -1; cInd <= 1; ++cInd) {
          curImage = aTrans + bTrans + (cellMatrixT.col(2) * cInd);
          double lengthSq = (origBondVector + curImage).squaredNorm();
          if (lengthSq < minLengthSq) {
            minLengthSq = lengthSq;
            shift = curImage;
          }
        }
      }
    }

    shortestBondVector = origBondVector + shift;

    // If we're in the original cell, render intracell.
    if (shortestBondVector.isApprox(origBondVector, 1e-4)) {
      return 0;
    }

    const double shortestBondLength = shortestBondVector.norm();
    const Vector3d normalizedShortestBondVector =
        shortestBondVector / shortestBondLength;

    // Find the image vector, which points from the image end atom to
    // the original end atom
    imageVector = (begPos + shortestBondVector) - endPos;
    ParametrizedLine<double, 3> bondLine (begPos,
                                          normalizedShortestBondVector);

    // Find all intersections with param between 0 and shortestBondLength
    int numValidIntersections = 0;
    for (int i = 0; i < 6; ++i) {
      Q_ASSERT_X(numValidIntersections <= 3, Q_FUNC_INFO,
                 "Too many unit cell intersections for one bond!");
      intParams[i] = bondLine.intersection(planes[i]);
      if (intParams[i] >= 0.0 && intParams[i] <= shortestBondLength)
        validIntersections[numValidIntersections++] = i;
    }

    // Sort valid intersections by intParam
    for (int i = 0; i < numValidIntersections; ++i) {
      int index1 = validIntersections[i];
      double param1 = intParams[index1];
      for (int j = i + 1; j < numValidIntersections; ++j) {
        int index2 = validIntersections[j];
        double param2 = intParams[index2];
        if (param2 < param1) {
          qSwap(validIntersections[i], validIntersections[j]);
          qSwap(index1, index2);
          qSwap(param1, param2);
        }
      }
    }

    return numValidIntersections;
  }

  void BSDYEngine::setAtomRadiusPercentage(int value)
  {
    m_atomRadiusPercentage = value / m_atomRadiusScale;
    emit changed();
  }

  void BSDYEngine::setAtomRadiusType(int type)
  {
    m_atomRadiusType = type;
    if (type == 0)
      pRadius = radiusCovalent;
    else
      pRadius = radiusVdW;
    emit changed();
  }

  void BSDYEngine::setBondRadius(int value)
  {
    m_bondRadius = value / m_bondRadiusScale;
    emit changed();
  }

  void BSDYEngine::setShowMulti(int value)
  {
    m_showMulti = value;
    emit changed();
  }

  void BSDYEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    emit changed();
  }

  double BSDYEngine::radius( const PainterDevice *pd, const Primitive *p ) const
  {
    // Atom radius
    if ( p->type() == Primitive::AtomType ) {
      if ( pd ) {
        if ( pd->isSelected( p ) )
          return radius( static_cast<const Atom *>( p ) ) + SEL_ATOM_EXTRA_RADIUS;
      }
      return radius( static_cast<const Atom *>( p ) );
    }
    // Bond radius
    else if ( p->type() == Primitive::BondType ) {
      if ( pd ) {
        if ( pd->isSelected( p ) )
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

  Engine::Layers BSDYEngine::layers() const
  {
    return Engine::Opaque | Engine::Transparent;
  }

  QWidget *BSDYEngine::settingsWidget()
  {
    if (!m_settingsWidget) {
      m_settingsWidget = new BSDYSettingsWidget();
      connect(m_settingsWidget->atomRadiusSlider, SIGNAL(valueChanged(int)),
              this, SLOT(setAtomRadiusPercentage(int)));
      connect(m_settingsWidget->combo_radius, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setAtomRadiusType(int)));
      connect(m_settingsWidget->bondRadiusSlider, SIGNAL(valueChanged(int)),
              this, SLOT(setBondRadius(int)));
      connect(m_settingsWidget->showMulti, SIGNAL(stateChanged(int)),
              this, SLOT(setShowMulti(int)));
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)),
              this, SLOT(setOpacity(int)));
      connect(m_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));
      m_settingsWidget->atomRadiusSlider
          ->setValue(int(m_atomRadiusScale * m_atomRadiusPercentage));
      m_settingsWidget->bondRadiusSlider
          ->setValue(int(m_bondRadiusScale * m_bondRadius));
      m_settingsWidget->showMulti->setCheckState((Qt::CheckState)m_showMulti);
      m_settingsWidget->opacitySlider->setValue(int(20 * m_alpha));
      m_settingsWidget->combo_radius->setCurrentIndex(m_atomRadiusType);
    }
    return m_settingsWidget;
  }

  void BSDYEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  void BSDYEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("atomRadius",
                      m_atomRadiusScale * m_atomRadiusPercentage);
    settings.setValue("radiusType", m_atomRadiusType);
    settings.setValue("bondRadius",
                      m_bondRadiusScale * m_bondRadius);
    settings.setValue("showMulti", m_showMulti);
    settings.setValue("opacity", 20 * m_alpha);
  }

  void BSDYEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setAtomRadiusPercentage(settings.value("atomRadius", 25).toDouble());
    setBondRadius(settings.value("bondRadius", 4).toDouble());
    setShowMulti(settings.value("showMulti", 2).toInt());
    setOpacity(settings.value("opacity", 100).toInt());
    setAtomRadiusType(settings.value("radiusType", 1).toInt());

    if (m_settingsWidget) {
      m_settingsWidget->atomRadiusSlider
          ->setValue(int(m_atomRadiusScale * m_atomRadiusPercentage));
      m_settingsWidget->combo_radius->setCurrentIndex(m_atomRadiusType);
      m_settingsWidget->bondRadiusSlider
          ->setValue(int(m_bondRadiusScale * m_bondRadius));
      m_settingsWidget->showMulti->setCheckState((Qt::CheckState)m_showMulti);
      m_settingsWidget->opacitySlider->setValue(int(20 * m_alpha));
      m_settingsWidget->combo_radius->setCurrentIndex(m_atomRadiusType);
    }
  }

}

 Q_EXPORT_PLUGIN2( bsdyengine, Avogadro::BSDYEngineFactory )
