/**********************************************************************
  POVPainter - drawing spheres, cylinders and text in a POVRay scene

  Copyright (C) 2007 Marcus D. Hanwell

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

#include <QFile>
#include <QString>
#include <QTextStream>

#include <QDebug>

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

  }

  void POVPainter::setName (const Primitive *)
  {
    // This is not currently used for the POVPainter
  }

  void POVPainter::setName (Primitive::Type,  int)
  {
    // This is not currently used for the POVPainter
  }

  void POVPainter::setColor (const Color *color)
  {
    d->color = *color;
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

  void POVPainter::drawSphere (const Vector3d & center, double radius)
  {
    // Write out a POVRay sphere for rendering
    *(d->output) << "sphere {\n"
      << "\t<" << center.x() << ", " << center.y() << ", " << center.z() << ">, " << radius
      << "\n\tpigment { rgbf <" << d->color.red() << ", " << d->color.green() << ", "
      << d->color.blue() << ", " << 1.0 - d->color.alpha() << "> }\n}\n";
  }

  void POVPainter::drawCylinder (const Vector3d &end1, const Vector3d &end2,
                      double radius)
  {
    // Write out a POVRay cylinder for rendering
    *(d->output) << "cylinder {\n"
      << "\t<" << end1.x() << ", " << end1.y() << ", " << end1.z() << ">, "
      << "\t<" << end2.x() << ", " << end2.y() << ", " << end2.z() << ">, " << radius
      << "\n\tpigment { rgbf <" << d->color.red() << ", " << d->color.green() << ", "
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
    else ortho1 = axisNormalized.ortho();
    // This number seems to work well for drawing the multiCylinder inside
    ortho1 *= radius*1.5;
    Vector3d ortho2 = cross( axisNormalized, ortho1 );
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
        << "\n\tpigment { rgbf <" << d->color.red() << ", " << d->color.green() << ", "
        << d->color.blue() << ", " << 1.0 - d->color.alpha() << "> }\n}\n";

    }
  }

  void POVPainter::drawShadedSector(Eigen::Vector3d, Eigen::Vector3d,
                        Eigen::Vector3d, double)
  {
  }

  void POVPainter::drawArc(Eigen::Vector3d, Eigen::Vector3d, Eigen::Vector3d,
               double, double)
  {
  }

  void POVPainter::drawShadedQuadrilateral(Eigen::Vector3d, Eigen::Vector3d,
                               Eigen::Vector3d, Eigen::Vector3d)
  {
  }

  void POVPainter::drawQuadrilateral(Eigen::Vector3d, Eigen::Vector3d,
                         Eigen::Vector3d, Eigen::Vector3d,
                         double)
  {
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

  POVPainterDevice::POVPainterDevice(const QString& filename, const GLWidget* glwidget)
  {
    m_glwidget = glwidget;
    m_painter = new POVPainter;
    m_file = new QFile(filename);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    m_output = new QTextStream(m_file);
    m_painter->begin(m_output, m_glwidget->normalVector());

    m_engines = m_glwidget->engines();

    initializePOV();
    render();
  }

  POVPainterDevice::~POVPainterDevice()
  {
    m_painter->end();
    delete m_output;
    m_output = 0;
    m_file->close();
    delete m_file;
  }

  void POVPainterDevice::initializePOV()
  {
    // Initialise our POV-Ray scene
    // The POV-Ray camera basically has the same matrix elements - we just need to translate
    // FIXME Still working on getting the translation to POV-Ray right...
    Vector3d cameraT = m_glwidget->camera()->modelview().translationVector();
    Vector3d cameraX = m_glwidget->camera()->backtransformedXAxis();
    Vector3d cameraY = m_glwidget->camera()->backtransformedYAxis();
    Vector3d cameraZ = m_glwidget->camera()->backtransformedZAxis();
    Vector3d light = cameraT + Vector3d(0.8, 0.7, 1.0);

    // Output the POV-Ray initialisation code
    *(m_output) << "global_settings {\n"
      << "\tambient_light rgb <1,1,1>\n"
      << "\tmax_trace_level 20\n}\n\n"
      << "background { color rgb <1,1,1> }\n\n"
      << "camera {\n"
      << "\tperspective\n"
      << "\tlocation <" << cameraT.x() << ", " << cameraT.y() << ", " << cameraT.z() << ">\n"
//      << "\tright 1.33 * <" << cameraX.x() << ", " << cameraX.y() << ", " << cameraX.z() << ">\n"
//      << "\tup <" << cameraY.x() << ", " << cameraY.y() << ", " << cameraY.z() << ">\n"
//      << "\tdirection <" << cameraZ.x() << ", " << cameraZ.y() << ", " << cameraZ.z() << ">\n"
      << "\tlook_at <0, 0, 0>\n"
      << "\tsky y\n}\n\n"
      << "light_source {\n"
      << "\t<" << light.x() << ", " << light.y() << ", " << light.z() << ">\n"
      << "\tcolor rgb<1,1,1>\n"
      << "}\n\n"
      << "#default {\n\tfinish {ambient .4 diffuse .6 specular 0.9 roughness .005 metallic}\n}";
  }

  void POVPainterDevice::render()
  {
    // Now render the scene using the active engines
    foreach( Engine *engine, m_engines ) {
      if ( engine->isEnabled() )
        engine->renderOpaque(this);
      if ( engine->isEnabled() && engine->flags() & Engine::Transparent )
        engine->renderTransparent(this);
    }
  }


} // End namespace Avogadro

