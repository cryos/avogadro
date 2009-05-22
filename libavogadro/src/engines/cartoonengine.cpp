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
#include <avogadro/mesh.h>
#include <avogadro/painterdevice.h>

#include <avogadro/protein.h>

#include <QMessageBox>
#include <QString>
#include <QDebug>

#include <Eigen/Geometry>

using Eigen::Vector3d;

namespace Avogadro {

  /**
   * Code taken from Zodiac: www.zeden.org
   Nicola Zonta
   nicola.zonta@zeden.org

   Ian J. Grimstead
   I.J.Grimstead@cs.cardiff.ac.uk

   Andrea Brancale
   brancalea@cf.ac.uk
   *
   */
  struct BackboneToMesh
  {
    Molecule *m_molecule;
    Mesh *m_mesh;
    Protein *m_protein;
    std::vector<std::vector<Eigen::Vector3d> > m_backbonePoints;
    std::vector<Eigen::Vector3d> m_backboneDirections;

    QColor m_helixColor;
    QColor m_sheetColor;
    QColor m_loopColor;

    // mesh
    std::vector<Eigen::Vector3d> m_vertices;
    std::vector<Eigen::Vector3d> m_normals;
    std::vector<QColor> m_colors;

    BackboneToMesh(Molecule *molecule, Mesh *mesh = 0)
    {
      m_molecule = molecule;
      if (mesh)
        m_mesh = mesh;
      else
        m_mesh = molecule->addMesh();

      m_protein = new Protein(m_molecule);

      m_backbonePoints.resize(m_molecule->numResidues());
      m_backboneDirections.resize(m_molecule->numResidues());
      
      m_helixColor = QColor(255, 0, 0); // red
      m_sheetColor = QColor(255, 255, 0); // yellow
      m_loopColor = QColor(0, 255, 0); // purple

      generateMesh();
    }

    void setHelixColor(const QColor &color)
    {
      m_helixColor = color;
    }

    void setSheetColor(const QColor &color)
    {
      m_sheetColor = color;
    }

    void setLoopColor(const QColor &color)
    {
      m_loopColor = color;
    }



    void setBackbonePoints(Residue *residue, const std::vector<Eigen::Vector3d> &points)
    {
      m_backbonePoints[residue->index()] = points;
    }

    const std::vector<Eigen::Vector3d>& backbonePoints(Residue *residue) const
    {
      return m_backbonePoints.at(residue->index());
    }

    void setBackboneDirection(Residue *residue, const Eigen::Vector3d &direction)
    {
      m_backboneDirections[residue->index()] = direction;
    }

    const Eigen::Vector3d& backboneDirection(Residue *residue) const
    {
      return m_backboneDirections.at(residue->index());
    }

    Residue* previousResidue(Residue *residue, const QVector<Residue*> &chain) const
    {
      int index = chain.indexOf(residue);
      if (index > 0)
        return chain.at(index - 1);
      return 0;
    }

    Residue* nextResidue(Residue *residue, const QVector<Residue*> &chain) const
    {
      int index = chain.indexOf(residue);
      if (index + 1 < chain.size())
        return chain.at(index + 1);
      return 0;
    }

    void generateMesh()
    {
      findBackboneData();
      foreach(const QVector<Residue*> &chain, m_protein->chains()) {
        foreach(Residue* residue, chain) {
          drawBackboneStick(residue, chain);
        }
      }
      //surf ->render();
    }

    void findBackboneData()
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
            std::vector<Eigen::Vector3d> lis = backbonePoints(residue);
            addGuidePointsToBackbone(residue, residues, lis);
            lis = smoothList(lis);
            setBackbonePoints(residue, lis);
          }
        }
      }
    }

    Atom* atomFromResidue(Residue *residue, const QString &atomID)
    {
      foreach (unsigned long atom, residue->atoms()) {
        if (residue->atomId(atom).trimmed() == atomID)
          return m_molecule->atomById(atom);
      }
      return 0;
    }

    void findBackbonePoints(Residue *residue, const QVector<Residue*> &chain)
    {
      bool hasPrevious = false, hasNext = false;
      Eigen::Vector3d previousCpos = Eigen::Vector3d::Zero();
      Eigen::Vector3d nextNpos = Eigen::Vector3d::Zero();
      std::vector<Eigen::Vector3d> out;
      // get the index for the residue in the chain
      int index = chain.indexOf(residue);
      // find the previous residue in the chain
      if (index > 0) {
        Residue *previousRes = chain.at(index - 1);
        Atom *previousC = atomFromResidue(previousRes, "C");
        if (previousC) {
          hasPrevious = true;
          previousCpos = *(previousC->pos());
        }
      }
      if (index + 1 < chain.size()) {
        Residue *nextRes = chain.at(index + 1);
        Atom *nextN = atomFromResidue(nextRes, "N");
        if (nextN) {
          hasNext = true;
          nextNpos = *(nextN->pos());
        }
      }

      Atom *n = atomFromResidue(residue, "N");
      Atom *ca = atomFromResidue(residue, "CA");
      Atom *c = atomFromResidue(residue, "C");
      if (n && c && ca) {
        //Eigen::Vector3d vca = ca->pos();
        Eigen::Vector3d vc = *(c->pos());
        Eigen::Vector3d vn = *(n->pos());
        if (hasPrevious)
          out.push_back(0.5 * (previousCpos + vn));
        else
          out.push_back(vn);

        if (hasNext)
          out.push_back(0.5 * (nextNpos + vc));
        else
          out.push_back (vc);
      }

      setBackbonePoints(residue, out);
    }

    void findBackboneDirection(Residue *residue)
    {
      Eigen::Vector3d out(0., 0., 1.);
      Atom *o = atomFromResidue(residue, "O");
      Atom *c = atomFromResidue(residue, "C");
      if (o && c) {
        out = *(o->pos()) - *(c->pos());
      }

      setBackboneDirection(residue, out);
    }

    Eigen::Vector3d startReference(Residue *residue)
    {
      std::vector<Eigen::Vector3d> lis = backbonePoints(residue);
      if (lis.size())
        return lis[1];
      return Eigen::Vector3d::Zero();
    }

    Eigen::Vector3d endReference(Residue *residue)
    {
      std::vector<Eigen::Vector3d> lis = backbonePoints(residue);
      return lis[lis.size()-2];
    }

    void addGuidePointsToBackbone(Residue *residue, const QVector<Residue*> &chain,
        std::vector<Eigen::Vector3d> &lis)
    {
      Residue *previousRes = previousResidue(residue, chain);
      if (previousRes) {
        lis.insert(lis.begin(), endReference(previousRes));
      } else if (lis.size () > 1) {
        Eigen::Vector3d v = lis [1];
        Eigen::Vector3d c = lis [0];
        c *= 2.0;
        lis.insert(lis.begin(), c - v);
      } else {
        lis.insert(lis.begin(), Eigen::Vector3d::Zero());
      }

      Residue *nextRes = nextResidue(residue, chain);
      if (nextRes) {
        lis.push_back(startReference(nextRes));
      } else if (lis.size() > 1) {
        Eigen::Vector3d v = lis[lis.size()-2];
        Eigen::Vector3d c = lis[lis.size()-1];
        c *= 2.0;
        lis.push_back (c - v);
      } else {
        lis.push_back(Eigen::Vector3d(0., 0., -1.));
      }
    }

    // P     A  B ... C      F     interpolates between A B C etc, P and F are discarded after calculation
    std::vector<Eigen::Vector3d> smoothList(const std::vector<Eigen::Vector3d> &lis)
    {
      if (lis.size () > 2) {
        std::vector<Eigen::Vector3d> ilist, out;
        Eigen::Vector3d lasti;
        for (unsigned int i = 1; i < lis.size () -1; i++) {
          Eigen::Vector3d i1, i2;
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

    Eigen::Vector3d circumcenter(const Eigen::Vector3d &v1,
        const Eigen::Vector3d &v2, const Eigen::Vector3d v3)
    {
      Eigen::Vector3d A = v1;
      Eigen::Vector3d B = v2;
      Eigen::Vector3d C = v3;
      double a2 = (B - C).squaredNorm();
      double b2 = (C - A).squaredNorm();
      double c2 = (A - B).squaredNorm();

      double aFactor = a2*(c2 + b2 - a2);
      double cFactor = c2*(a2 + b2 - c2);
      double bFactor = b2*(a2 + c2 - b2);

      double den = 2 * (a2*b2 + a2*c2 + b2*c2)-(a2*a2 + b2*b2 + c2*c2);
      if (den < 1.0e-4)
        den = 1.0e-4;
      Eigen::Vector3d out = (A*aFactor + B*bFactor + C*cFactor) / den;
      return out;
    }

    void interpolate(const Eigen::Vector3d &v1, const Eigen::Vector3d &v2, const Eigen::Vector3d &v3,
        Eigen::Vector3d &i1, Eigen::Vector3d &i2)
    {
      Eigen::Vector3d d1 = v1 - v2;
      Eigen::Vector3d d2 = v3 - v2;
      d1.normalize();
      d2.normalize();
      if (d1.dot(d2) < -0.999) {
        i1 = 0.5 * (v1 + v2);
        i2 = 0.5 * (v2 + v3);
      } else {
        Eigen::Vector3d c = circumcenter(v1, v2, v3);
        Eigen::Vector3d m1 = 0.5 * (v1 + v2);
        Eigen::Vector3d m2 = 0.5 * (v2 + v3);
        Eigen::Vector3d r1 = m1 - c;
        Eigen::Vector3d r2 = m2 - c;
        r1.normalize();
        r2.normalize();
        double r = (c - v1).norm();
        r1 *= r;
        r2 *= r;
        i1 = c + r1;
        i2 = c + r2;
      }
    }

    const QColor& color(Residue *residue) const
    {
      if (m_protein->isHelix(residue))
        return m_helixColor;
      if (m_protein->isSheet(residue))
        return m_sheetColor;
      return m_loopColor;
    }

    QColor mixColors(const QColor &c1, const QColor &c2) 
    {
      QColor color;
      color.setRgbF((float) (c1.redF()   + c2.redF()  ) * 0.5f,
                    (float) (c1.greenF() + c2.greenF()) * 0.5f,
                    (float) (c1.blueF()  + c2.blueF() ) * 0.5f,
                    (float) (c1.alphaF() + c2.alphaF()) * 0.5f);
      return color;
    }

    void drawBackboneStick(Residue *residue, const QVector<Residue*> &chain)
    {
      //int n_points = *ddwin ->data ->quality_scale * 9;
      unsigned int n_points = 5 * 9; // FIXME
      std::vector<Eigen::Vector3d> random_points;
      std::vector<Eigen::Vector3d> helix_points;
      std::vector<Eigen::Vector3d> sheet_points;
      double ha = 1.0; // FIXME start settings!!!
      double hb = 0.3;
      double hc = 1.0;
      double sa = 1.0;
      double sb = 0.3;
      double sc = 1.0;
      double ra = 0.2;
      double rb = 0.2;
      double rc = 0.0; // FIXME end settings!!!

      for (unsigned int i = 0; i < n_points; i++) {
        double da = 2 * M_PI / (n_points - 1);
        double angl = i * da;
        random_points.push_back( Eigen::Vector3d(sin(angl) * rb-(rc*rb*sin(angl)*sin(angl)*sin(angl)), cos (angl)*ra , 0.));
        helix_points.push_back( Eigen::Vector3d(sin(angl) * hb-(hc*hb*sin(angl)*sin(angl)*sin(angl)), cos (angl)*ha , 0.));
        sheet_points.push_back( Eigen::Vector3d(sin(angl) * sb-(sc*sb*sin(angl)*sin(angl)*sin(angl)), cos (angl)*sa , 0.));
      }
      std::vector<Eigen::Vector3d> *last_shape, *shape, *next_shape;
      QColor last_col, col, next_col;
      col = color(residue);
      last_col = col;
      next_col = col;
      last_shape = &random_points;
      next_shape = &random_points;
      shape = &random_points;

      // this residue
      Eigen::Vector3d dir = backboneDirection(residue);
      if (m_protein->isHelix(residue))
        shape = &helix_points;
      else if (m_protein->isSheet(residue))
        shape = &sheet_points;

      // previous residue
      Residue *previousRes = previousResidue(residue, chain);
      Eigen::Vector3d lastdir;
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
      Eigen::Vector3d nextdir;
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
      std::vector<Eigen::Vector3d> points = backbonePoints(residue);
      addGuidePointsToBackbone(residue, chain, points);

      QColor c2 = mixColors(last_col, col);
      QColor c1 = mixColors(next_col, col);

      if (points.size () > 3) {
        double tot = ((double) points.size () -3);
        for (unsigned int i = 3; i < points.size (); i++) {
          double dt = ((double)i-3) / tot;
          dt = sin ((dt-0.5f)*M_PI);
          dt *= 0.5f;
          dt += 0.5f;

          QColor cc1, cc2;
          cc1.setRgbF((double)(c1.redF()*dt+c2.redF()*(1-dt)),
              ((double) c1.greenF()*dt+c2.greenF()*(1-dt)),
              (double) (c1.blueF()*dt+c2.blueF()*(1-dt)),
              ((double) c1.alphaF()*dt+c2.alphaF()*(1-dt)));
          Eigen::Vector3d v1, v2;
          v2 = lastdir;
          v1 = nextdir;
          std::vector<Eigen::Vector3d> shape1, shape2;
          for (unsigned int n = 0; n < n_points; n++) {
            Eigen::Vector3d vv2 = 0.5 * ((*last_shape)[n] + (*shape)[n]);
            Eigen::Vector3d vv1 = 0.5 * ((*next_shape)[n] + (*shape)[n]);
            shape1.push_back( Eigen::Vector3d(vv1.x() * dt + vv2.x() * (1 - dt),
                  vv1.y() * dt + vv2.y() * (1 - dt),
                  vv1.z() * dt + vv2.z() * (1 - dt)) );
          }

          Eigen::Vector3d d(v1.x() * dt + v2.x() * (1 - dt),
              v1.y() * dt + v2.y() * (1 - dt),
              v1.z() * dt + v2.z() * (1 - dt)  );

          dt = ((double) i-2) / tot;
          dt = sin ((dt-0.5f) * M_PI);
          dt*= 0.5f;
          dt += 0.5f;

          cc2.setRgbF((double)(c1.redF()*dt+c2.redF()*(1-dt)),
              ((double) c1.greenF()*dt+c2.greenF()*(1-dt)),
              (double) (c1.blueF()*dt+c2.blueF()*(1-dt)),
              ((double) c1.alphaF()*dt+c2.alphaF()*(1-dt)));

          for (unsigned int n = 0; n < n_points; n++) {
            Eigen::Vector3d vv2 = 0.5 * ((*last_shape)[n] + (*shape)[n]);
            Eigen::Vector3d vv1 = 0.5 * ((*next_shape)[n] + (*shape)[n]);
            shape2.push_back( Eigen::Vector3d (vv1.x() * dt + vv2.x() * (1 - dt),
                  vv1.y() * dt + vv2.y() * (1 - dt),
                  vv1.z() * dt + vv2.z() * (1 - dt)) );
          }

          Eigen::Vector3d d2(v1.x() * dt + v2.x() * (1 - dt),
              v1.y() * dt + v2.y() * (1 - dt),
              v1.z() * dt + v2.z() * (1 - dt) );

          backboneRibbon(points[i-3], points [i-2],points [i-1],points [i], d, d2, cc1, cc2, shape1, shape2);
        }
      }
    }

    void components(const Eigen::Vector3d &vec, const Eigen::Vector3d &ref,
        Eigen::Vector3d &parallel, Eigen::Vector3d &normal)
    {
      //assert (!isnan(vec.module()));
      //assert (!isnan(ref.module()));
      Eigen::Vector3d reference = ref.normalized();
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
        SurfVertex () : color(QColor(255, 255, 255)) {};
        Eigen::Vector3d coords;
        Eigen::Vector3d normal;
        QColor         color;
    };

    void backboneRibbon(const Eigen::Vector3d &v1, const Eigen::Vector3d &v2,
        const Eigen::Vector3d &v3, const Eigen::Vector3d &v4, const Eigen::Vector3d &dir,
        const Eigen::Vector3d &dir2, const QColor &c1, const QColor &c2,
        const std::vector<Eigen::Vector3d> &shape1, const std::vector<Eigen::Vector3d> &shape2)
    {
      //Sandri's method
      Eigen::Vector3d prec_vect = v2 - v1;
      Eigen::Vector3d cyl_vect = v3 - v2;
      Eigen::Vector3d post_vect = v4 - v3;
      prec_vect.normalize();
      post_vect.normalize();
      cyl_vect.normalize();
      Eigen::Vector3d norm1 = 0.5 * (prec_vect + cyl_vect);
      Eigen::Vector3d norm2 = 0.5 * (cyl_vect + post_vect);

      Eigen::Vector3d par1, pp1, par2, pp2;
      components(dir, norm1, par1, pp1);
      components(dir2, norm2, par2, pp2);

      Eigen::Matrix3d m1, m2;

      Eigen::Vector3d newz1 = norm1;
      Eigen::Vector3d newy1 = pp1;
      newz1.normalize();
      newy1.normalize();
      Eigen::Vector3d newx1 = newy1.cross(newz1);

      m1.col(0) = newx1;
      m1.col(1) = newy1;
      m1.col(2) = newz1;

      Eigen::Vector3d newz2 = norm2;
      Eigen::Vector3d newy2 = pp2;
      newz2.normalize();
      newy2.normalize();
      Eigen::Vector3d newx2 = newy2.cross(newz2);

      m2.col(0) = newx2;
      m2.col(1) = newy2;
      m2.col(2) = newz2;

      unsigned int slices = shape1.size();
      std::vector<SurfVertex*> vertices;
      SurfVertex *lastv1, *lastv2;

      for (unsigned int n = 0; n < slices; n++){
        Eigen::Vector3d p1 = shape1[n];
        Eigen::Vector3d p2 = shape2[n];

        int last_n = n - 1;
        if (last_n < 0)
          last_n = slices - 2;
        unsigned int next_n = n + 1;
        if (next_n >= slices)
          next_n = 1;

        Eigen::Vector3d tan1 = shape1[next_n] - shape1[last_n];
        Eigen::Vector3d tan2 = shape2[next_n] - shape2[last_n];
        Eigen::Vector3d n1 = Eigen::Vector3d(-tan1.y(), tan1.x(), 0.0);
        Eigen::Vector3d n2 = Eigen::Vector3d(-tan2.y(), tan2.x(), 0.0);

        p1 = m1*p1 + v2;
        p2 = m2*p2 + v3;

        n1 = m1*n1;
        n2 = m1*n2;

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

      std::vector<Eigen::Vector3f> tmp(m_vertices.size());
      for (unsigned int i = 0; i < tmp.size(); ++i)
        tmp[i] = m_vertices[i].cast<float>();
      m_mesh->setVertices(tmp);

      for (unsigned int i = 0; i < tmp.size(); ++i)
        tmp[i] = m_normals[i].cast<float>();
      m_mesh->setNormals(tmp);

      m_mesh->setColors(m_colors);
    }

  };
  
  const float chainColors[6][3] = {
    { 1.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0 },
    { 1.0, 0.0, 1.0 },
    { 1.0, 1.0, 0.0 },
    { 0.0, 1.0, 1.0 }
  };

  CartoonEngine::CartoonEngine(QObject *parent) : Engine(parent),
    m_type(0), m_radius(1.0), m_useNitrogens(2), m_mesh(0)
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

    glDisable( GL_CULL_FACE );
    pd->painter()->setColor(0.0, 0.0, 0.0, 1.0);
    if (m_mesh)
      pd->painter()->drawColorMesh(*m_mesh);
    glEnable( GL_CULL_FACE );

    return true;
  }

  bool CartoonEngine::renderQuick(PainterDevice *pd)
  {
    glDisable( GL_CULL_FACE );
    if (m_mesh)
      pd->painter()->drawMesh(*m_mesh);
    glEnable( GL_CULL_FACE );

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

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map
 
    if (!m_mesh && molecule->numAtoms() != 0) {
      Molecule *mol = (Molecule*) molecule;
      m_mesh = mol->addMesh();
      BackboneToMesh((Molecule*)molecule, m_mesh);
    }

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

Q_EXPORT_PLUGIN2(cartoonengine, Avogadro::CartoonEngineFactory)
