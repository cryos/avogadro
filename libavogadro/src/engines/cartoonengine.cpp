/**********************************************************************
  CartoonEngine - Engine for protein structures.

  Copyright (C) 2009 Tim Vandermeersch
  Some portions Copyright (C) 2007-2008 by Marcus D. Hanwell

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

#include "cartoonengine.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/residue.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/painterdevice.h>

#include <avogadro/protein.h>

#include <QMessageBox>
#include <QString>
#include <QDebug>

#include <Eigen/Geometry>

using Eigen::Vector3d;

namespace Avogadro {

  const float chainColors[6][3] = {
    { 1.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0 },
    { 1.0, 0.0, 1.0 },
    { 1.0, 1.0, 0.0 },
    { 0.0, 1.0, 1.0 }
  };

  CartoonEngine::CartoonEngine(QObject *parent) : Engine(parent),
    m_type(0), m_radius(1.0), m_useNitrogens(2)
  {
    // Initialise variables
    m_update = true;
  }

  Engine *CartoonEngine::clone() const
  {
    CartoonEngine *engine = new CartoonEngine(parent());
    engine->setAlias(alias());
    engine->m_type = m_type;
    engine->m_radius = m_radius;
    engine->setEnabled(isEnabled());

    return engine;
  }

  CartoonEngine::~CartoonEngine()
  {
  }

  bool CartoonEngine::renderOpaque(PainterDevice *pd)
  {
    // Check if the chains need updating before drawing them
    if (m_update) updateChains(pd);

    // draw debug points...
    /*
    pd->painter()->setColor(0.0, 1.0, 0.0);
    foreach (const Eigen::Vector3d &p, m_helixPoints)
      pd->painter()->drawSphere(&p, 0.1);
    */

    pd->painter()->setColor(chainColors[0][0], chainColors[0][1], chainColors[0][2]);
    int numTriangles = m_normals.size();
    glDisable( GL_CULL_FACE );
    for (int i = 0; i < numTriangles; ++i) {
      pd->painter()->drawTriangle(m_triangles.at(i*3), m_triangles.at(i*3+1),
          m_triangles.at(i*3+2), m_normals.at(i));
    }
    glEnable( GL_CULL_FACE );


    pd->painter()->setColor(chainColors[0][0], chainColors[0][1], chainColors[0][2]);
    for (int i = 0; i < m_helixes3.size(); ++i)
      pd->painter()->drawCylinder(m_helixes3[i][0], m_helixes3[i][1], 2.3);
    pd->painter()->setColor(chainColors[1][0], chainColors[1][1], chainColors[1][2]);

    /*
    if (m_type == 0) {
      for (int i = 0; i < m_chains.size(); i++) {
        if (m_chains[i].size() <= 1)
          continue;
        pd->painter()->setColor(chainColors[i % 6][0], chainColors[i % 6][1], chainColors[i % 6][2]);
        pd->painter()->drawSpline(m_chains[i], m_radius);
      }
    }
    else {
      // Render cylinders between the points and spheres at each point
      for (int i = 0; i < m_chains.size(); i++) {
        if (m_chains[i].size() <= 1)
          continue;
        pd->painter()->setColor(chainColors[i % 6][0], chainColors[i % 6][1], chainColors[i % 6][2]);
        pd->painter()->drawSphere(&m_chains[i][0], m_radius);
        for (int j = 1; j < m_chains[i].size(); j++) {
          pd->painter()->drawSphere(&m_chains[i][j], m_radius);
          pd->painter()->drawCylinder(m_chains[i][j-1], m_chains[i][j], m_radius);
        }
      }
    }
    */

    return true;
  }

  bool CartoonEngine::renderQuick(PainterDevice *pd)
  {
    pd->painter()->setColor(chainColors[0][0], chainColors[0][1], chainColors[0][2]);
    int numTriangles = m_normals.size();
    glDisable( GL_CULL_FACE );
    for (int i = 0; i < numTriangles; ++i) {
      pd->painter()->drawTriangle(m_triangles.at(i*3), m_triangles.at(i*3+1),
          m_triangles.at(i*3+2), m_normals.at(i));
    }
    glEnable( GL_CULL_FACE );


    pd->painter()->setColor(chainColors[0][0], chainColors[0][1], chainColors[0][2]);
    for (int i = 0; i < m_helixes3.size(); ++i)
      pd->painter()->drawCylinder(m_helixes3[i][0], m_helixes3[i][1], 2.3);
    pd->painter()->setColor(chainColors[1][0], chainColors[1][1], chainColors[1][2]);

    // Just render cylinders between the backbone...
    double tRadius = m_radius / 2.0;
    for (int i = 0; i < m_chains.size(); i++) {
      if (m_chains[i].size() <= 1)
        continue;
      pd->painter()->setColor(chainColors[i % 6][0], chainColors[i % 6][1], chainColors[i % 6][2]);
      pd->painter()->drawSphere(&m_chains[i][0], tRadius);
      for (int j = 1; j < m_chains[i].size(); j++) {
        pd->painter()->drawSphere(&m_chains[i][j], tRadius);
        pd->painter()->drawCylinder(m_chains[i][j-1], m_chains[i][j], tRadius);
      }
    }

    return true;
  }

  double CartoonEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return m_radius;
  }

  void CartoonEngine::setPrimitives(const PrimitiveList &primitives)
  {
    Engine::setPrimitives(primitives);
    m_update = true;
  }

  void CartoonEngine::addPrimitive(Primitive *primitive)
  {
    Engine::addPrimitive(primitive);
    m_update = true;
  }

  void CartoonEngine::updatePrimitive(Primitive *)
  {
    m_update = true;
  }

  void CartoonEngine::removePrimitive(Primitive *primitive)
  {
    Engine::removePrimitive(primitive);
    m_update = true;
  }

  void CartoonEngine::updateChains(PainterDevice *pd)
  {
    if (!isEnabled()) return;
    // Get a list of residues for the molecule
    const Molecule *molecule = pd->molecule();
    Protein protein((Molecule*)molecule);


    m_triangles.clear();
    m_normals.clear();
    // 4-turn helixes
    for (int i = 0; i < protein.num4turnHelixes(); ++i) {
      QList<Eigen::Vector3d> helixPoints;
      QList<Eigen::Vector3d> helixAxis;
      QList<Eigen::Vector3d> helixNormals;

      // all N, CA, C, O atoms in that order
      QList<unsigned long> helix = protein.helix4BackboneAtoms(i);
      int numResidues = helix.size() / 4;

      // compute centers...
      // compute the helix centers
      QList<Eigen::Vector3d> helixCenters;
      for (int i = 0; i < helix.size() - 15; i+= 16) {
        Eigen::Vector3d p1 = Eigen::Vector3d::Zero();
        for (int j = 0; j < 16; ++j)
          p1 += *(molecule->atomById(helix.at(i+j))->pos());
        p1 /= 16.0;
        helixCenters.append(p1);
      }

      if (numResidues % 4 != 0) {
        Eigen::Vector3d p2 = Eigen::Vector3d::Zero();
        for (int i = helix.size() - 16; i < helix.size(); ++i)
          p2 += *(molecule->atomById(helix.at(i))->pos());
        p2 /= 16.0;
        helixCenters.append(p2);
      }

      for (int i = 0; i < numResidues - 1; ++i) {
        // the axis points
        Eigen::Vector3d P1 = helixCenters.at(0);
        Eigen::Vector3d P2 = helixCenters.at(helixCenters.size()-1);
        // the central axis
        Eigen::Vector3d axis = P2 - P1;
        axis.normalize();
        // first nitrogen position
        Eigen::Vector3d posN1 = *(molecule->atomById(helix.at(i*4))->pos());
        // find point on line, closest to N
        Eigen::Vector3d p1 = P1 + (posN1 - P1).dot(axis) * axis;
        // second nitrogen position
        Eigen::Vector3d posN2 = *(molecule->atomById(helix.at(i*4+4))->pos());
        // find point on line, closest to N
        Eigen::Vector3d p2 = P1 + (posN2 - P1).dot(axis) * axis;

        double deltaHeight = (p1 - p2).norm();
        // shortest lines from axis to nitrogens
        Eigen::Vector3d r1 = posN1 - p1;
        Eigen::Vector3d r2 = posN2 - p2;
        double deltaRadius = r2.norm() - r1.norm();
        double angle = acos( r1.dot(r2) / (r1.norm() * r2.norm()) );

        helixPoints.append(posN1);
        helixNormals.append(r1);
        helixAxis.append(axis);

        double incHeight = deltaHeight / 4.0;
        double incRadius = deltaRadius / 4.0;
        double incAngle = angle / 4.0;

        Eigen::Transform3d m;
        m = Eigen::AngleAxisd(incAngle, axis);

        for (int j = 1; j < 4; ++j) {
          Eigen::Vector3d lastPoint = helixPoints.last();
          Eigen::Vector3d onLine = P1 + (lastPoint - P1).dot(axis) * axis;
          // correct for deltaRadius
          lastPoint -= (onLine - lastPoint).normalized() * incRadius;
          Eigen::Vector3d newPoint = m * (lastPoint - onLine) + onLine;
          newPoint += axis * incHeight;
          helixPoints.append(newPoint);
          helixNormals.append(newPoint - onLine);
          helixAxis.append(axis);
        }

      }

      for (int i = 0; i < helixNormals.size() - 1; ++i) {
        Eigen::Vector3d axis(helixAxis.at(i));
        m_triangles.append(helixPoints.at(i) + axis);
        m_triangles.append(helixPoints.at(i+1) - axis);
        m_triangles.append(helixPoints.at(i) - axis);

        m_triangles.append(helixPoints.at(i) + axis);
        m_triangles.append(helixPoints.at(i+1) + axis);
        m_triangles.append(helixPoints.at(i+1) - axis);

        m_normals.append(helixNormals.at(i));
        m_normals.append(helixNormals.at(i));
      }

    }
    // 3-turn helixes
    for (int i = 0; i < protein.num3turnHelixes(); ++i) {
      QList<unsigned long> helix = protein.helix3BackboneAtoms(i);

      Eigen::Vector3d p1 = Eigen::Vector3d::Zero();
      for (int i = 0; i < 12; ++i)
        p1 += *(molecule->atomById(helix.at(i))->pos());
      p1 /= 12.0;

      Eigen::Vector3d p2 = Eigen::Vector3d::Zero();
      for (int i = helix.size() - 12; i < helix.size(); ++i)
        p2 += *(molecule->atomById(helix.at(i))->pos());
      p2 /= 12.0;

      Eigen::Vector3d ab = p1 - p2;
      ab.normalize();
      ab *= 3.0;

      p1 += ab;
      p2 -= ab;

      QVector<Vector3d> helixPoints;
      helixPoints.append(p1);
      helixPoints.append(p2);
      m_helixes3.append(helixPoints);
    }


    m_chains.clear();
    QList<Primitive *> list;
    list = primitives().subList(Primitive::ResidueType);
    unsigned int currentChain = 0;
    QVector<Vector3d> pts;

    foreach(Primitive *p, list) {
      Residue *r = static_cast<Residue *>(p);
      if(r->name() =="HOH") {
        continue;
      }

      if(r->chainNumber() != currentChain) {
        // this residue is on a new chain
        if(pts.size() > 0)
          m_chains.push_back(pts);
        currentChain = r->chainNumber();
        pts.clear();
      }

      foreach (unsigned long atom, r->atoms()) {
        // should be CA
        QString atomId = r->atomId(atom);
        atomId = atomId.trimmed();
        if (atomId == "CA") {
          pts.push_back(*molecule->atomById(atom)->pos());
        }
        else if (atomId == "N" && m_useNitrogens == 2) {
          pts.push_back(*molecule->atomById(atom)->pos());
        }
      } // end atoms in residue

    } // end primitive list (i.e., all residues)
    m_chains.push_back(pts); // Add the last chain (possibly the only chain)
    m_update = false;
  }

  Engine::PrimitiveTypes CartoonEngine::primitiveTypes() const
  {
    return Engine::Atoms;
  }

  Engine::ColorTypes CartoonEngine::colorTypes() const
  {
    return Engine::IndexedColors;
  }

  void CartoonEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
  }

  void CartoonEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
  }

}

#include "cartoonengine.moc"

Q_EXPORT_PLUGIN2(cartoonengine, Avogadro::CartoonEngineFactory)

