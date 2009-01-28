/**********************************************************************
  POVPainter - drawing spheres, cylinders and text in a POVRay scene

  Copyright (C) 2007-2008 Marcus D. Hanwell

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

#include "povpainter.h"

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/mesh.h>

#include <QFile>
#include <QDebug>
#include <Eigen/Geometry>

namespace Avogadro
{
  class POVPainterPrivate
  {
    public:
      POVPainterPrivate() : pd (0), initialized (false), sharing(0),
        color(0), output(0), planeNormalVector(0., 0., 0.)  { color.set(0., 0., 0., 0.); }
      ~POVPainterPrivate()
      {
      }

      POVPainterDevice *pd;

      bool initialized;

      inline bool isValid();

      /**
       * Painters can be shared, we must keep track of this.
       */
      int sharing;

      Color color;
      QTextStream *output;
      Vector3d planeNormalVector;
  };


  POVPainter::POVPainter() : d (new POVPainterPrivate)
  {

  }

  POVPainter::~POVPainter()
  {
    delete d;
  }

  void POVPainter::setColor (const Color *color)
  {
    d->color = *color;
  }

  void POVPainter::setColor (const QColor *color)
  {
    d->color = Color(color->redF(), color->greenF(), color->blueF(),
                     color->alphaF());
  }

  void POVPainter::setColor (float red, float green, float blue, float alpha)
  {
    d->color = Color(red, green, blue, alpha);
  }

  void POVPainter::setPlaneNormal (Vector3d planeNormalVector)
  {
    // Set the plane normal, multiCylinders are drawn relative to this
    d->planeNormalVector = planeNormalVector;
  }

  void POVPainter::drawSphere (const Vector3d *center, float radius)
  {
    // Write out a POVRay sphere for rendering
    *(d->output) << "sphere {\n"
      << "\t<" << center->x() << ", " << center->y() << ", " << center->z()
      << ">, " << radius
      << "\n\tpigment { rgbt <" << d->color.red() << ", " << d->color.green()
      << ", " << d->color.blue() << "," << 1.0 - d->color.alpha() << "> }\n}\n";
  }

  void POVPainter::drawCylinder (const Vector3d &end1, const Vector3d &end2,
                      double radius)
  {
    // Write out a POVRay cylinder for rendering
    *(d->output) << "cylinder {\n"
      << "\t<" << end1.x() << ", " << end1.y() << ", " << end1.z() << ">, "
      << "\t<" << end2.x() << ", " << end2.y() << ", " << end2.z() << ">, " << radius
      << "\n\tpigment { rgbt <" << d->color.red() << ", " << d->color.green() << ", "
      << d->color.blue() << ", " << 1.0 - d->color.alpha() << "> }\n}\n";
  }

  void POVPainter::drawMultiCylinder (const Vector3d &end1, const Vector3d &end2,
                           double radius, int order, double)
  {
    // Just render single bonds with the standard drawCylinder function
    if (order == 1)
    {
      drawCylinder(end1, end2, radius);
      return;
    }

    // Find the bond axis
    Vector3d axis = end2 - end1;
    double axisNorm = axis.norm();
    if( axisNorm == 0.0 ) return;
    Vector3d axisNormalized = axis / axisNorm;

    // Use the plane normal vector for the molecule to draw multicylinders along
    Vector3d ortho1 = axisNormalized.cross(d->planeNormalVector);
    double ortho1Norm = ortho1.norm();
    if( ortho1Norm > 0.001 ) ortho1 /= ortho1Norm;
    else ortho1 = axisNormalized.unitOrthogonal();
    // This number seems to work well for drawing the multiCylinder inside
    ortho1 *= radius*1.5;
    Vector3d ortho2 = axisNormalized.cross(ortho1);
    // Use an angle offset of zero for double bonds, 90 for triple and 22.5 for higher order
    double angleOffset = 0.0;
    if( order >= 3 )
    {
      if( order == 3 ) angleOffset = 90.0;
      else angleOffset = 22.5;
    }
    // Actually draw the cylinders
    for( int i = 0; i < order; i++)
    {
      double alpha = angleOffset / 180.0 * M_PI + 2.0 * M_PI * i / order;
      Vector3d displacement = cos(alpha) * ortho1 + sin(alpha) * ortho2;
      Vector3d displacedEnd1 = end1 + displacement;
      Vector3d displacedEnd2 = end2 + displacement;
      // Write out a POVRay cylinder for rendering
      *(d->output) << "cylinder {\n"
        << "\t<" << displacedEnd1.x() << ", "
                 << displacedEnd1.y() << ", "
                 << displacedEnd1.z() << ">, "
        << "\t<" << displacedEnd2.x() << ", "
                 << displacedEnd2.y() << ", "
                 << displacedEnd2.z() << ">, " << radius
        << "\n\tpigment { rgbt <" << d->color.red() << ", " << d->color.green() << ", "
        << d->color.blue() << ", " << 1.0 - d->color.alpha() << "> }\n}\n";

    }
  }

  void POVPainter::drawShadedSector(const Eigen::Vector3d &, const Eigen::Vector3d &,
                        const Eigen::Vector3d &, double, bool)
  {
  }

  void POVPainter::drawArc(const Eigen::Vector3d &, const Eigen::Vector3d &, const Eigen::Vector3d &,
               double, double, bool)
  {
  }

  void POVPainter::drawShadedQuadrilateral(const Eigen::Vector3d &, const Eigen::Vector3d &,
                               const Eigen::Vector3d &, const Eigen::Vector3d &)
  {
  }

  void POVPainter::drawQuadrilateral(const Eigen::Vector3d &, const Eigen::Vector3d &,
                         const Eigen::Vector3d &, const Eigen::Vector3d &,
                         double)
  {
  }

  void POVPainter::drawMesh(const Mesh & mesh, int mode, bool normalWind)
  {
    // Now we draw the given mesh to the OpenGL widget
    switch (mode)
    {
      case 0: // Filled triangles
        break;
      case 1: // Lines
        break;
      case 2: // Points
        break;
    }

    // Render the triangles of the mesh
    std::vector<Eigen::Vector3f> t = mesh.vertices();
    std::vector<Eigen::Vector3f> n = mesh.normals();

    // If there are no triangles then don't bother doing anything
    if (t.size() == 0) {
      return;
    }

    // Normal or reverse winding?
    QString vertsStr, ivertsStr, normsStr, inormsStr;
    QTextStream verts(&vertsStr);
    verts << "vertex_vectors{" << t.size() << ",\n";
    QTextStream iverts(&ivertsStr);
    iverts << "face_indices{" << t.size() / 3 << ",\n";
    QTextStream norms(&normsStr);
    norms << "normal_vectors{" << n.size() << ",\n";
    if (normalWind) {
      for(unsigned int i = 0; i < t.size(); ++i) {
        verts << "<" << t[i].x() << "," << t[i].y() << "," << t[i].z() << ">";
        norms << "<" << n[i].x() << "," << n[i].y() << "," << n[i].z() << ">";
        if (i != t.size()-1) {
          verts << ", ";
          norms << ", ";
        }
        if (i != 0 && i%3 == 0) {
          verts << "\n";
          norms << "\n";
        }
      }
      // Now to write out the indices
      for (unsigned int i = 0; i < t.size(); i += 3) {
        iverts << "<" << i << "," << i+1 << "," << i+2 << ">";
        if (i != t.size()-3) {
          iverts << ", ";
        }
        if (i != 0 && ((i+1)/3)%3 == 0) {
          iverts << "\n";
        }
      }
    }
    /// FIXME - this is a fudge to fix the negative windings right now - FIXME!
    else {
      for(unsigned int i = t.size(); i > 0; --i) {
        Eigen::Vector3f tmp = n[i-1] * -1;
        verts << "<" << t[i-1].x() << "," << t[i-1].y() << "," << t[i-1].z() << ">";
        norms << "<" << tmp.x() << "," << tmp.y() << "," << tmp.z() << ">";
        if (i != t.size()-1) {
          verts << ", ";
          norms << ", ";
        }
        if (i != 0 && i%3 == 0) {
          verts << "\n";
          norms << "\n";
        }
      }
      // Now to write out the indices
      for (unsigned int i = 0; i < t.size(); i += 3) {
        iverts << "<" << i << "," << i+1 << "," << i+2 << ">";
        if (i != t.size()-3) {
          iverts << ", ";
        }
        if (i != 0 && ((i+1)/3)%3 == 0) {
          iverts << "\n";
        }
      }
    }
    // Now to close off all the arrays
    verts << "\n}";
    norms << "\n}";
    iverts << "\n}";
    // Now to write out the full mesh - could be pretty big...
    *(d->output) << "mesh2 {\n"
                 << vertsStr << "\n"
                 << normsStr << "\n"
                 << ivertsStr << "\n"
                 << "\tpigment { rgbt <" << d->color.red() << ", "
                 << d->color.green() << ", "
                 << d->color.blue() << ", " << 1.0 - d->color.alpha() << "> }"
                 << "}\n\n";

  }

  void POVPainter::drawColorMesh(const Mesh & mesh, int mode, bool normalWind)
  {
    // Now we draw the given mesh to the OpenGL widget
    switch (mode)
    {
      case 0: // Filled triangles
        break;
      case 1: // Lines
        break;
      case 2: // Points
        break;
    }

    // Render the triangles of the mesh
    std::vector<Eigen::Vector3f> v = mesh.vertices();
    std::vector<Eigen::Vector3f> n = mesh.normals();
    std::vector<QColor> c = mesh.colors();

    // If there are no triangles then don't bother doing anything
    if (v.size() == 0 || v.size() != c.size()) {
      return;
    }

    // Normal or reverse winding?
    QString vertsStr, ivertsStr, normsStr, texturesStr;
    QTextStream verts(&vertsStr);
    verts << "vertex_vectors{" << v.size() << ",\n";
    QTextStream iverts(&ivertsStr);
    iverts << "face_indices{" << v.size() / 3 << ",\n";
    QTextStream norms(&normsStr);
    norms << "normal_vectors{" << n.size() << ",\n";
    QTextStream textures(&texturesStr);
    textures << "texture_list{" << c.size() << ",\n";
    if (normalWind) {
      for(unsigned int i = 0; i < v.size(); ++i) {
        verts << "<" << v[i].x() << "," << v[i].y() << "," << v[i].z() << ">";
        norms << "<" << n[i].x() << "," << n[i].y() << "," << n[i].z() << ">";
        textures << "texture{pigment{rgbt<" << c[i].redF() << ","
                 << c[i].greenF() << "," << c[i].blueF() << "," << c[i].alphaF()
                 << ">}},\n";
        if (i != v.size()-1) {
          verts << ", ";
          norms << ", ";
        }
        if (i != 0 && i%3 == 0) {
          verts << "\n";
          norms << "\n";
        }
      }
      // Now to write out the indices
      for (unsigned int i = 0; i < v.size(); i += 3) {
        iverts << "<" << i << "," << i+1 << "," << i+2 << ">";
        iverts << "," << i << "," << i+1 << "," << i+2;
        if (i != v.size()-3) {
          iverts << ", ";
        }
        if (i != 0 && ((i+1)/3)%3 == 0) {
          iverts << "\n";
        }
      }
    }
    /// FIXME - this is a fudge to fix the negative windings right now - FIXME!
    else {
      for(unsigned int i = v.size(); i > 0; --i) {
        Eigen::Vector3f tmp = n[i-1] * -1;
        verts << "<" << v[i-1].x() << "," << v[i-1].y() << "," << v[i-1].z() << ">";
        norms << "<" << tmp.x() << "," << tmp.y() << "," << tmp.z() << ">";
        textures << "texture{pigment{rgbt{" << c[i].redF() << ","
                 << c[i].greenF() << "," << c[i].blueF() << "," << c[i].alphaF()
                 << "}}}\n";
        if (i != v.size()-1) {
          verts << ", ";
          norms << ", ";
        }
        if (i != 0 && i%3 == 0) {
          verts << "\n";
          norms << "\n";
        }
      }
      // Now to write out the indices
      for (unsigned int i = 0; i < v.size(); i += 3) {
        iverts << "<" << i << "," << i+1 << "," << i+2 << ">";
        if (i != v.size()-3) {
          iverts << ", ";
        }
        if (i != 0 && ((i+1)/3)%3 == 0) {
          iverts << "\n";
        }
      }
    }
    // Now to close off all the arrays
    verts << "\n}";
    norms << "\n}";
    iverts << "\n}";
    textures << "\n}";
    // Now to write out the full mesh - could be pretty big...
    *(d->output) << "mesh2 {\n"
                 << vertsStr << "\n"
                 << normsStr << "\n"
                 << texturesStr << "\n"
                 << ivertsStr << "\n"
                 << "}\n\n";
  }

  int POVPainter::drawText (int, int, const QString &) const
  {
    return 0;
  }

  int POVPainter::drawText (const QPoint&, const QString &) const
  {
    return 0;
  }

  int POVPainter::drawText (const Vector3d &, const QString &) const
  {
    return 0;
  }

  void POVPainter::begin(QTextStream *output, Vector3d planeNormalVector)
  {
    d->output = output;
    d->planeNormalVector = planeNormalVector;
  }

  void POVPainter::end()
  {
    d->output = 0;
  }

  POVPainterDevice::POVPainterDevice(const QString& filename, double aspectRatio,
                                     const GLWidget* glwidget)
  {
    m_painter = 0;
    m_output = 0;
    m_file = 0;
    m_aspectRatio = aspectRatio;
    m_glwidget = glwidget;
    m_painter = new POVPainter;
    m_file = new QFile(filename);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    m_output = new QTextStream(m_file);
    m_output->setRealNumberPrecision(15);
    m_painter->begin(m_output, m_glwidget->normalVector());

    m_engines = m_glwidget->engines();

    initializePOV();
    render();
    m_painter->end();
    m_file->close();
  }

  POVPainterDevice::~POVPainterDevice()
  {
    delete m_output;
    m_output = 0;
    delete m_file;
  }

  void POVPainterDevice::initializePOV()
  {
    // Initialise our POV-Ray scene
    // The POV-Ray camera basically has the same matrix elements - we just need to translate
    // FIXME Still working on getting the translation to POV-Ray right...
    Vector3d cameraT = -( m_glwidget->camera()->modelview().linear().adjoint()
                          * m_glwidget->camera()->modelview().translation()
                        );
    Vector3d cameraX = m_glwidget->camera()->backTransformedXAxis();
    Vector3d cameraY = m_glwidget->camera()->backTransformedYAxis();
    Vector3d cameraZ = -m_glwidget->camera()->backTransformedZAxis();

    double huge;
    if(m_glwidget->farthestAtom()) {
      huge = 10 * m_glwidget->farthestAtom()->pos()->norm();
    }
    else {
      huge = 10;
    }

    Vector3d light0pos = huge * ( m_glwidget->camera()->modelview().linear().adjoint()
                                  * Vector3d(LIGHT0_POSITION[0], LIGHT0_POSITION[1], LIGHT0_POSITION[2]) );
    Vector3d light1pos = huge * ( m_glwidget->camera()->modelview().linear().adjoint()
                                  * Vector3d(LIGHT1_POSITION[0], LIGHT1_POSITION[1], LIGHT1_POSITION[2]) );

    // Output the POV-Ray initialisation code
    *(m_output) << "global_settings {\n"
      << "\tambient_light rgb <"
      << LIGHT_AMBIENT[0] << ", " << LIGHT_AMBIENT[1] << ", " << LIGHT_AMBIENT[2] << ">\n"
      << "\tmax_trace_level 15\n}\n\n"
      << "background { color rgb <"
      << m_glwidget->background().redF() << ","
      << m_glwidget->background().greenF() << ","
      << m_glwidget->background().blueF()
      << "> }\n\n"

      << "camera {\n"
      << "\tperspective\n"
      << "\tlocation <" << cameraT.x() << ", " << cameraT.y() << ", " << cameraT.z() << ">\n"
      << "\tangle " << m_glwidget->camera()->angleOfViewY() << "\n"
      << "\tup <" << cameraY.x() << ", " << cameraY.y() << ", " << cameraY.z() << ">\n"
      << "\tright <" << cameraX.x() << ", " << cameraX.y() << ", " << cameraX.z()
      << "> * " << m_aspectRatio << "\n"
      << "\tdirection <" << cameraZ.x() << ", " << cameraZ.y() << ", " << cameraZ.z() << "> }\n\n"

      << "light_source {\n"
      << "\t<" << light0pos[0]
      << ", " << light0pos[1]
      << ", " << light0pos[2] << ">\n"
      << "\tcolor rgb <" << LIGHT0_DIFFUSE[0] << ", "
                         << LIGHT0_DIFFUSE[1] << ", "
                         << LIGHT0_DIFFUSE[2] << ">\n"
      << "\tfade_distance " << 2 * huge << "\n"
      << "\tfade_power 0\n"
      << "\tparallel\n"
      << "\tpoint_at <" << -light0pos[0]
                << ", " << -light0pos[1]
                << ", " << -light0pos[2] << ">\n"
      << "}\n\n"

      << "light_source {\n"
      << "\t<" << light1pos[0]
      << ", " << light1pos[1]
      << ", " << light1pos[2] << ">\n"
      << "\tcolor rgb <" << LIGHT1_DIFFUSE[0] << ", "
                         << LIGHT1_DIFFUSE[1] << ", "
                         << LIGHT1_DIFFUSE[2] << ">\n"
      << "\tfade_distance " << 2 * huge << "\n"
      << "\tfade_power 0\n"
      << "\tparallel\n"
      << "\tpoint_at <" << -light1pos[0]
                << ", " << -light1pos[1]
                << ", " << -light1pos[2] << ">\n"
      << "}\n\n"

      << "#default {\n\tfinish {ambient .8 diffuse 1 specular 1 roughness .005 metallic 0.5}\n}\n\n";
  }

  void POVPainterDevice::render()
  {
    // Now render the scene using the active engines
    foreach( Engine *engine, m_engines ) {
      if (engine->isEnabled()) {
        // Use unions for opaque objects - they are faster
        *m_output << "union {\n";
        engine->renderOpaque(this);
        *m_output << "}\n";
      }
      if (engine->isEnabled() && engine->layers() & Engine::Transparent) {
        // Use merge for transparent objects, slower but more correct
        *m_output << "merge {\n";
        engine->renderTransparent(this);
        *m_output << "}\n";
      }
    }
  }


} // End namespace Avogadro

