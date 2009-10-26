/**********************************************************************
  CartoonEngine - Engine for protein structures.

   Code taken from Zodiac: www.zeden.org
   
   Nicola Zonta
   nicola.zonta@zeden.org

   Ian J. Grimstead
   I.J.Grimstead@cs.cardiff.ac.uk

   Andrea Brancale
   brancalea@cf.ac.uk
 
  Some portions Copyright (C) 2009 Tim Vandermeersch

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

#include "cartoonmeshgenerator.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/residue.h>
#include <avogadro/mesh.h>
#include <avogadro/color.h>
#include <avogadro/protein.h>

#include <QMessageBox>
#include <QString>
#include <QDebug>

#include <Eigen/Geometry>

namespace Avogadro {

  CartoonMeshGenerator::CartoonMeshGenerator(QObject *parent) : QThread(parent),
      m_molecule(0), m_mesh(0), m_protein(0)
  {
    m_quality = 2;
    setHelixABC(1.0, 0.3, 1.0);
    setSheetABC(1.0, 0.3, 1.0);
    setLoopABC(0.2, 0.2, 0.0);
    m_helixColor = Color3f(255, 0, 0); // red
    m_sheetColor = Color3f(255, 255, 0); // yellow
    m_loopColor = Color3f(0, 255, 0); // purple
  }

  CartoonMeshGenerator::CartoonMeshGenerator(const Molecule *molecule, Mesh *mesh, 
      QObject *parent) : QThread(parent), m_molecule((Molecule*)molecule), m_mesh(mesh),
      m_protein(0)
  {
    m_backbonePoints.resize(m_molecule->numResidues());
    m_backboneDirections.resize(m_molecule->numResidues());
    
    m_quality = 2;
    setHelixABC(1.0, 0.3, 1.0);
    setSheetABC(1.0, 0.3, 1.0);
    setLoopABC(0.2, 0.2, 0.0);
    m_helixColor = Color3f(255, 0, 0); // red
    m_sheetColor = Color3f(255, 255, 0); // yellow
    m_loopColor = Color3f(0, 255, 0); // purple
  }
    
  CartoonMeshGenerator::~CartoonMeshGenerator()
  {
    if (m_protein) {
      delete m_protein;
      m_protein = 0;
    }
  }
    
  bool CartoonMeshGenerator::initialize(const Molecule *molecule, Mesh *mesh)
  {
    m_molecule = (Molecule*)molecule;
    m_mesh = mesh;

    m_backbonePoints.resize(m_molecule->numResidues());
    m_backboneDirections.resize(m_molecule->numResidues());
    return true;
  }
    
  void CartoonMeshGenerator::run()
  {
    if (!m_molecule || !m_mesh) {
      qDebug() << "CartoonMeshGenerator: No mesh or molecule set...";
      return;
    }

    // Mark the mesh as being worked on and clear it
    m_mesh->setStable(false);
    m_mesh->clear();

    m_protein = new Protein(m_molecule);

    findBackboneData();
    foreach(const QVector<Residue*> &chain, m_protein->chains()) {
      foreach(Residue* residue, chain) {
        drawBackboneStick(residue, chain);
      }
    }
    
    m_mesh->setVertices(m_vertices);
    m_mesh->setNormals(m_normals);
    m_mesh->setColors(m_colors);
    m_mesh->setStable(true);
  }
    
  void CartoonMeshGenerator::clear()
  {
    if (m_protein) {
      delete m_protein;
      m_protein = 0;
    }

    m_molecule = 0;
    m_mesh = 0;

    m_backbonePoints.clear();
    m_backboneDirections.clear(); 
  }

  void CartoonMeshGenerator::setBackbonePoints(Residue *residue, const std::vector<Eigen::Vector3f> &points)
  {
    m_backbonePoints[residue->index()] = points;
  }

  const std::vector<Eigen::Vector3f>& CartoonMeshGenerator::backbonePoints(Residue *residue) const
  {
    return m_backbonePoints.at(residue->index());
  }

  void CartoonMeshGenerator::setBackboneDirection(Residue *residue, const Eigen::Vector3f &direction)
  {
    m_backboneDirections[residue->index()] = direction;
  }

  const Eigen::Vector3f& CartoonMeshGenerator::backboneDirection(Residue *residue) const
  {
    return m_backboneDirections.at(residue->index());
  }

  Residue* CartoonMeshGenerator::previousResidue(Residue *residue, const QVector<Residue*> &chain) const
  {
    int index = chain.indexOf(residue);
    if (index > 0)
      return chain.at(index - 1);
    return 0;
  }

  Residue* CartoonMeshGenerator::nextResidue(Residue *residue, const QVector<Residue*> &chain) const
  {
    int index = chain.indexOf(residue);
    if (index + 1 < chain.size())
      return chain.at(index + 1);
    return 0;
  }

  void CartoonMeshGenerator::findBackboneData()
  {
    foreach(const QVector<Residue*> &chain, m_protein->chains()) {
      foreach(Residue* residue, chain) {
        findBackbonePoints(residue, chain);
        findBackboneDirection(residue);
      }
    }

    int smoothCycles = 3;
    for (int i = 0; i < smoothCycles; ++i) {
      foreach(const QVector<Residue*> &residues, m_protein->chains()) {
        foreach(Residue* residue, residues) {
          std::vector<Eigen::Vector3f> lis = backbonePoints(residue);
          addGuidePointsToBackbone(residue, residues, lis);
          lis = smoothList(lis);
          setBackbonePoints(residue, lis);
        }
      }
    }
  }

  Atom* CartoonMeshGenerator::atomFromResidue(Residue *residue, const QString &atomID)
  {
    foreach (unsigned long atom, residue->atoms()) {
      if (residue->atomId(atom).trimmed() == atomID)
        return m_molecule->atomById(atom);
    }
    return 0;
  }

  void CartoonMeshGenerator::findBackbonePoints(Residue *residue, const QVector<Residue*> &chain)
  {
    bool hasPrevious = false, hasNext = false;
    Eigen::Vector3f previousCpos = Eigen::Vector3f::Zero();
    Eigen::Vector3f nextNpos = Eigen::Vector3f::Zero();
    std::vector<Eigen::Vector3f> out;
    // get the index for the residue in the chain
    int index = chain.indexOf(residue);
    // find the previous residue in the chain
    if (index > 0) {
      Residue *previousRes = chain.at(index - 1);
      Atom *previousC = atomFromResidue(previousRes, "C");
      if (previousC) {
        hasPrevious = true;
        previousCpos = previousC->pos()->cast<float>();
      }
    }
    if (index + 1 < chain.size()) {
      Residue *nextRes = chain.at(index + 1);
      Atom *nextN = atomFromResidue(nextRes, "N");
      if (nextN) {
        hasNext = true;
        nextNpos = nextN->pos()->cast<float>();
      }
    }

    Atom *n = atomFromResidue(residue, "N");
    Atom *ca = atomFromResidue(residue, "CA");
    Atom *c = atomFromResidue(residue, "C");
    if (n && c && ca) {
      //Eigen::Vector3f vca = ca->pos();
      Eigen::Vector3f vc = c->pos()->cast<float>();
      Eigen::Vector3f vn = n->pos()->cast<float>();
      if (hasPrevious)
        out.push_back(0.5 * (previousCpos + vn));
      else
        out.push_back(vn);

      if (hasNext)
        out.push_back(0.5 * (nextNpos + vc));
      else
        out.push_back(vc);
    }

    setBackbonePoints(residue, out);
  }

  void CartoonMeshGenerator::findBackboneDirection(Residue *residue)
  {
    Eigen::Vector3f out(0., 0., 1.);
    Atom *o = atomFromResidue(residue, "O");
    Atom *c = atomFromResidue(residue, "C");
    if (o && c) {
      out = (*(o->pos()) - *(c->pos())).cast<float>();
    }

    setBackboneDirection(residue, out);
  }

  Eigen::Vector3f CartoonMeshGenerator::startReference(Residue *residue)
  {
    std::vector<Eigen::Vector3f> lis = backbonePoints(residue);
    if (lis.size())
      return lis[1];
    return Eigen::Vector3f::Zero();
  }

  Eigen::Vector3f CartoonMeshGenerator::endReference(Residue *residue)
  {
    std::vector<Eigen::Vector3f> lis = backbonePoints(residue);
    return lis[lis.size()-2];
  }

  void CartoonMeshGenerator::addGuidePointsToBackbone(Residue *residue, 
      const QVector<Residue*> &chain, std::vector<Eigen::Vector3f> &lis)
  {
    Residue *previousRes = previousResidue(residue, chain);
    if (previousRes) {
      lis.insert(lis.begin(), endReference(previousRes));
    } else if (lis.size () > 1) {
      Eigen::Vector3f v = lis[1];
      Eigen::Vector3f c = lis[0];
      c *= 2.0;
      lis.insert(lis.begin(), c - v);
    } else {
      lis.insert(lis.begin(), Eigen::Vector3f::Zero());
    }

    Residue *nextRes = nextResidue(residue, chain);
    if (nextRes) {
      lis.push_back(startReference(nextRes));
    } else if (lis.size() > 1) {
      Eigen::Vector3f v = lis[lis.size()-2];
      Eigen::Vector3f c = lis[lis.size()-1];
      c *= 2.0;
      lis.push_back (c - v);
    } else {
      lis.push_back(Eigen::Vector3f(0., 0., -1.));
    }
  }

  // P     A  B ... C      F     interpolates between A B C etc, P and F are discarded after calculation
  std::vector<Eigen::Vector3f> CartoonMeshGenerator::smoothList(const std::vector<Eigen::Vector3f> &lis)
  {
    if (lis.size () > 2) {
      std::vector<Eigen::Vector3f> ilist, out;
      Eigen::Vector3f lasti;
      for (unsigned int i = 1; i < lis.size () -1; i++) {
        Eigen::Vector3f i1, i2;
        interpolate(lis[i-1], lis[i], lis[i+1], i1, i2);
        if (i > 1)
          ilist.push_back(0.5 * (i1 + lasti));
        lasti = i2;
      }
      out.push_back(lis[1]);
      //discarding first and last interpolated points
      for (unsigned int i = 0; i < ilist.size (); i++) {
        out.push_back(ilist[i]);
        out.push_back(lis[i+2]);
      }
      return out;
    } else {
      return lis;
    }
  }

  Eigen::Vector3f CartoonMeshGenerator::circumcenter(const Eigen::Vector3f &v1,
      const Eigen::Vector3f &v2, const Eigen::Vector3f v3)
  {
    Eigen::Vector3f A = v1;
    Eigen::Vector3f B = v2;
    Eigen::Vector3f C = v3;
    double a2 = (B - C).squaredNorm();
    double b2 = (C - A).squaredNorm();
    double c2 = (A - B).squaredNorm();
    
    double aFactor = a2*(c2 + b2 - a2);
    double cFactor = c2*(a2 + b2 - c2);
    double bFactor = b2*(a2 + c2 - b2);

    double den = 2 * (a2*b2 + a2*c2 + b2*c2)-(a2*a2 + b2*b2 + c2*c2);
    if (den < 1.0e-4)
      den = 1.0e-4;
    Eigen::Vector3f out = (A*aFactor + B*bFactor + C*cFactor) / den;
    return out;
  }

  void CartoonMeshGenerator::interpolate(const Eigen::Vector3f &v1, const Eigen::Vector3f &v2, 
      const Eigen::Vector3f &v3, Eigen::Vector3f &i1, Eigen::Vector3f &i2)
  {
    Eigen::Vector3f d1 = v1 - v2;
    Eigen::Vector3f d2 = v3 - v2;
    d1.normalize();
    d2.normalize();
    if (d1.dot(d2) < -0.999) {
      i1 = 0.5 * (v1 + v2);
      i2 = 0.5 * (v2 + v3);
    } else {
      Eigen::Vector3f c = circumcenter(v1, v2, v3);
      Eigen::Vector3f m1 = 0.5 * (v1 + v2);
      Eigen::Vector3f m2 = 0.5 * (v2 + v3);
      Eigen::Vector3f r1 = m1 - c;
      Eigen::Vector3f r2 = m2 - c;
      r1.normalize();
      r2.normalize();
      double r = (c - v1).norm();
      r1 *= r;
      r2 *= r;
      i1 = c + r1;
      i2 = c + r2;
    }
  }

  const Color3f& CartoonMeshGenerator::color(Residue *residue) const
  {
    if (m_protein->isHelix(residue))
      return m_helixColor;
    if (m_protein->isSheet(residue))
      return m_sheetColor;
    return m_loopColor;
  }

  Color3f CartoonMeshGenerator::mixColors(const Color3f &c1, const Color3f &c2)
  {
    Color3f color((c1.red()   + c2.red())   * 0.5f,
                  (c1.green() + c2.green()) * 0.5f,
                  (c1.blue()  + c2.blue())  * 0.5f);
    return color;
  }

  void CartoonMeshGenerator::drawBackboneStick(Residue *residue, const QVector<Residue*> &chain)
  {
    std::vector<Eigen::Vector3f> random_points;
    std::vector<Eigen::Vector3f> helix_points;
    std::vector<Eigen::Vector3f> sheet_points;
    
    unsigned int n_points = m_quality * 9;
    for (unsigned int i = 0; i < n_points; i++) {
      double da = 2 * M_PI / (n_points - 1);
      double ang = i * da;
      double sine = sin(ang);
      double cosine = cos(ang);
      double sine3 = sine * sine * sine;
      random_points.push_back( Eigen::Vector3f(sine * m_bLoop - 
            (m_cLoop * m_bLoop * sine3), cosine * m_aLoop, 0.));
      helix_points.push_back( Eigen::Vector3f(sine * m_bHelix -
            (m_cHelix * m_bHelix * sine3), cosine * m_aHelix, 0.));
      sheet_points.push_back( Eigen::Vector3f(sine * m_bSheet -
            (m_cSheet * m_bSheet * sine3), cosine * m_aSheet, 0.));
    }
    std::vector<Eigen::Vector3f> *last_shape, *shape, *next_shape;
    Color3f last_col, col, next_col;
    col = color(residue);
    last_col = col;
    next_col = col;
    last_shape = &random_points;
    next_shape = &random_points;
    shape = &random_points;

    // this residue
    Eigen::Vector3f dir = backboneDirection(residue);
    if (m_protein->isHelix(residue))
      shape = &helix_points;
    else if (m_protein->isSheet(residue))
      shape = &sheet_points;

    // previous residue
    Residue *previousRes = previousResidue(residue, chain);
    Eigen::Vector3f lastdir;
    if (previousRes) {
      last_col = color(previousRes);
      lastdir = backboneDirection(previousRes);
      if (m_protein->isHelix(previousRes))
        last_shape = &helix_points;
      else if (m_protein->isSheet(previousRes))
        last_shape = &sheet_points;
    } else
      lastdir = dir;

    // next residue
    Residue *nextRes = nextResidue(residue, chain);
    Eigen::Vector3f nextdir;
    if (nextRes) {
      next_col = color(nextRes);
      nextdir = backboneDirection(nextRes);
      if (m_protein->isHelix(nextRes))
        next_shape = &helix_points;
      else if (m_protein->isSheet(nextRes))
        next_shape = &sheet_points;
    } else
      nextdir = dir;

    if (lastdir.dot(dir) < 0.0)
      lastdir *= -1.0;
    if (nextdir.dot(dir) < 0.0)
      nextdir *= -1.0;

    lastdir = 0.5 * (dir + lastdir);
    nextdir = 0.5 * (nextdir + dir);
    lastdir.normalize();
    nextdir.normalize();
    std::vector<Eigen::Vector3f> points = backbonePoints(residue);
    addGuidePointsToBackbone(residue, chain, points);

    Color3f c2 = mixColors(last_col, col);
    Color3f c1 = mixColors(next_col, col);

    if (points.size () > 3) {
      double tot = ((double) points.size () -3);
      for (unsigned int i = 3; i < points.size (); i++) {
        double dt = ((double)i-3) / tot;
        dt = sin ((dt-0.5f)*M_PI);
        dt *= 0.5f;
        dt += 0.5f;

        Color3f cc1, cc2;
        cc1.set((c1.red()*dt+c2.red()*(1-dt)),
                (c1.green()*dt+c2.green()*(1-dt)),
                (c1.blue()*dt+c2.blue()*(1-dt)));
        Eigen::Vector3f v1, v2;
        v2 = lastdir;
        v1 = nextdir;
        std::vector<Eigen::Vector3f> shape1, shape2;
        for (unsigned int n = 0; n < n_points; n++) {
          Eigen::Vector3f vv2 = 0.5 * ((*last_shape)[n] + (*shape)[n]);
          Eigen::Vector3f vv1 = 0.5 * ((*next_shape)[n] + (*shape)[n]);
          shape1.push_back( Eigen::Vector3f(vv1.x() * dt + vv2.x() * (1 - dt),
                vv1.y() * dt + vv2.y() * (1 - dt),
                vv1.z() * dt + vv2.z() * (1 - dt)) );
        }

        Eigen::Vector3f d(v1.x() * dt + v2.x() * (1 - dt),
            v1.y() * dt + v2.y() * (1 - dt),
            v1.z() * dt + v2.z() * (1 - dt)  );

        dt = ((double) i-2) / tot;
        dt = sin ((dt-0.5f) * M_PI);
        dt*= 0.5f;
        dt += 0.5f;

        cc2.set((c1.red()*dt+c2.red()*(1-dt)),
                (c1.green()*dt+c2.green()*(1-dt)),
                (c1.blue()*dt+c2.blue()*(1-dt)));

        for (unsigned int n = 0; n < n_points; n++) {
          Eigen::Vector3f vv2 = 0.5 * ((*last_shape)[n] + (*shape)[n]);
          Eigen::Vector3f vv1 = 0.5 * ((*next_shape)[n] + (*shape)[n]);
          shape2.push_back( Eigen::Vector3f (vv1.x() * dt + vv2.x() * (1 - dt),
                vv1.y() * dt + vv2.y() * (1 - dt),
                vv1.z() * dt + vv2.z() * (1 - dt)) );
        }

        Eigen::Vector3f d2(v1.x() * dt + v2.x() * (1 - dt),
            v1.y() * dt + v2.y() * (1 - dt),
            v1.z() * dt + v2.z() * (1 - dt) );

        backboneRibbon(points[i-3], points [i-2],points [i-1],points [i], d, d2, cc1, cc2, shape1, shape2);
      }
    }
  }

  void CartoonMeshGenerator::components(const Eigen::Vector3f &vec, const Eigen::Vector3f &ref,
      Eigen::Vector3f &parallel, Eigen::Vector3f &normal)
  {
    //assert (!isnan(vec.module()));
    //assert (!isnan(ref.module()));
    Eigen::Vector3f reference = ref.normalized();
    double mod = vec.dot(ref);
    reference *= mod;
    parallel = reference;
    normal = vec - parallel;
    //assert (!isnan(parallel.module ()));
    //assert (!isnan(normal.module ()));
  }

  class SurfVertex
  {
    public:
      SurfVertex () : color(Color3f(255, 255, 255)) {};
      Eigen::Vector3f coords;
      Eigen::Vector3f normal;
      Color3f         color;
  };
  
  void CartoonMeshGenerator::backboneRibbon(const Eigen::Vector3f &v1, const Eigen::Vector3f &v2,
      const Eigen::Vector3f &v3, const Eigen::Vector3f &v4, const Eigen::Vector3f &dir,
      const Eigen::Vector3f &dir2, const Color3f &c1, const Color3f &c2,
      const std::vector<Eigen::Vector3f> &shape1, const std::vector<Eigen::Vector3f> &shape2)
  {
    //Sandri's method
    Eigen::Vector3f prec_vect = v2 - v1;
    Eigen::Vector3f cyl_vect = v3 - v2;
    Eigen::Vector3f post_vect = v4 - v3;
    prec_vect.normalize();
    post_vect.normalize();
    cyl_vect.normalize();
    Eigen::Vector3f norm1 = 0.5 * (prec_vect + cyl_vect);
    Eigen::Vector3f norm2 = 0.5 * (cyl_vect + post_vect);

    Eigen::Vector3f par1, pp1, par2, pp2;
    components(dir, norm1, par1, pp1);
    components(dir2, norm2, par2, pp2);

    Eigen::Matrix3f m1, m2;

    Eigen::Vector3f newz1 = norm1;
    Eigen::Vector3f newy1 = pp1;
    newz1.normalize();
    newy1.normalize();
    Eigen::Vector3f newx1 = newy1.cross(newz1);

    m1.col(0) = newx1;
    m1.col(1) = newy1;
    m1.col(2) = newz1;
 
    Eigen::Vector3f newz2 = norm2;
    Eigen::Vector3f newy2 = pp2;
    newz2.normalize();
    newy2.normalize();
    Eigen::Vector3f newx2 = newy2.cross(newz2);

    m2.col(0) = newx2;
    m2.col(1) = newy2;
    m2.col(2) = newz2;

    unsigned int slices = shape1.size();
    std::vector<SurfVertex*> vertices;
    SurfVertex *lastv1, *lastv2;

    for (unsigned int n = 0; n < slices; n++){
      Eigen::Vector3f p1 = shape1[n];
      Eigen::Vector3f p2 = shape2[n];

      int last_n = n - 1;
      if (last_n < 0)
        last_n = slices - 2;
      unsigned int next_n = n + 1;
      if (next_n >= slices)
        next_n = 1;

      Eigen::Vector3f tan1 = shape1[next_n] - shape1[last_n];
      Eigen::Vector3f tan2 = shape2[next_n] - shape2[last_n];
      Eigen::Vector3f n1 = Eigen::Vector3f(-tan1.y(), tan1.x(), 0.0);
      Eigen::Vector3f n2 = Eigen::Vector3f(-tan2.y(), tan2.x(), 0.0);

      p1 = m1*p1 + v2;
      p2 = m2*p2 + v3;
      n1 = m1*n1;
      n2 = m2*n2;

      SurfVertex *newv1 = new SurfVertex;
      newv1->normal = n1;
      newv1->coords = p1;
      newv1->color  = c1;

      SurfVertex *newv2 = new SurfVertex;
      newv2->normal = n2;
      newv2->coords = p2;
      newv2->color  = c2;

      if (n > 0) {
        // lastv1
        m_vertices.push_back(lastv1->coords);
        m_normals.push_back(lastv1->normal);
        m_colors.push_back(lastv1->color);
        // lastv2
        m_vertices.push_back(lastv2->coords);
        m_normals.push_back(lastv2->normal);
        m_colors.push_back(lastv2->color);
        // newv2
        m_vertices.push_back(newv2->coords);
        m_normals.push_back(newv2->normal);
        m_colors.push_back(newv2->color);

        // newv2
        m_vertices.push_back(newv2->coords);
        m_normals.push_back(newv2->normal);
        m_colors.push_back(newv2->color);
        // newv1
        m_vertices.push_back(newv1->coords);
        m_normals.push_back(newv1->normal);
        m_colors.push_back(newv1->color);
        // lastv1
        m_vertices.push_back(lastv1->coords);
        m_normals.push_back(lastv1->normal);
        m_colors.push_back(lastv1->color);
      }
      // store last
      lastv1 = newv1;
      lastv2 = newv2;

      vertices.push_back(lastv1);
      vertices.push_back(lastv2);
    }

  }


}

