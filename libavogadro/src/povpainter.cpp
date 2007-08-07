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
      POVPainterPrivate() : pd ( 0 ), initialized ( false ), sharing ( 0 ),
        color(0), output(0)  { color.set(0., 0., 0., 0.); }
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
  };


  POVPainter::POVPainter () : d ( new POVPainterPrivate )
  {

  }

  POVPainter::~POVPainter()
  {

  }

  void POVPainter::setName ( const Primitive * )
  {
    // This is not currently used for the POVPainter
  }

  void POVPainter::setName ( Primitive::Type,  int )
  {
    // This is not currently used for the POVPainter
  }

  void POVPainter::setColor ( const Color *color )
  {
    d->color = *color;
  }

  void POVPainter::setColor ( float red, float green, float blue, float alpha )
  {
    d->color = Color(red, green, blue, alpha);
  }

  void POVPainter::drawSphere ( const Eigen::Vector3d & center, double radius )
  {
    // Write out a POVRay sphere for rendering
    *(d->output) << "sphere {\n"
      << "\t<" << center.x() << ", " << center.y() << ", " << center.z() << ">, " << radius
      << "\n\tpigment { rgbf <" << d->color.red() << ", " << d->color.green() << ", "
      << d->color.blue() << ", " << 1.0 - d->color.alpha() << "> }\n}\n";
  }

  void POVPainter::drawCylinder ( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
                      double radius )
  {
    // Write out a POVRay cylinder for rendering
    *(d->output) << "cylinder {\n"
      << "\t<" << end1.x() << ", " << end1.y() << ", " << end1.z() << ">, "
      << "\t<" << end2.x() << ", " << end2.y() << ", " << end2.z() << ">, " << radius
      << "\n\tpigment { rgbf <" << d->color.red() << ", " << d->color.green() << ", "
      << d->color.blue() << ", " << 1.0 - d->color.alpha() << "> }\n}\n";
  }

  void POVPainter::drawMultiCylinder ( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
                           double radius, int order, double shift )
  {

  }

  void POVPainter::drawShadedSector(Eigen::Vector3d origin, Eigen::Vector3d direction1,
                        Eigen::Vector3d direction2, double radius)
  {
  }

  void POVPainter::drawArc(Eigen::Vector3d origin, Eigen::Vector3d direction1, Eigen::Vector3d direction2,
               double radius, double lineWidth)
  {
  }

  void POVPainter::drawShadedQuadrilateral(Eigen::Vector3d point1, Eigen::Vector3d point2,
                               Eigen::Vector3d point3, Eigen::Vector3d point4)
  {
  }

  void POVPainter::drawQuadrilateral(Eigen::Vector3d point1, Eigen::Vector3d point2,
                         Eigen::Vector3d point3, Eigen::Vector3d point4,
                         double lineWidth)
  {
  }

  int POVPainter::drawText ( int x, int y, const QString &string ) const
  {
    return 0;
  }

  int POVPainter::drawText ( const QPoint& pos, const QString &string ) const
  {
    return 0;
  }

  int POVPainter::drawText ( const Eigen::Vector3d & pos, const QString &string ) const
  {
    return 0;
  }

  void POVPainter::begin(QTextStream *output)
  {
    d->output = output;
  }

  void POVPainter::end()
  {
    d->output = 0;
  }

  POVPainterDevice::POVPainterDevice(const QString& filename, const GLWidget* glwidget)
  {
    m_painter = new POVPainter;
    m_file = new QFile(filename);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    m_output = new QTextStream(m_file);
    m_painter->begin(m_output);
    m_glwidget = glwidget;

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
    // Initialise our POVRay scene
    Eigen::Vector3d cameraPos = m_glwidget->camera()->modelview().translationVector();
    Eigen::Vector3d lookatPos = m_glwidget->camera()->backtransformedZAxis();
    qDebug() << "Entered initializePOV() function.";
    *(m_output) << "global_settings {\n"
      << "\tambient_light rgb <1,1,1>\n"
      << "\tmax_trace_level 20\n}\n\n"
      << "background { color rgb <1,1,1> }\n\n"
      << "camera {\n"
      << "\tperspective\n"
      << "\tlocation <" << cameraPos.x() << ", " << cameraPos.y() << ", " << cameraPos.z() << ">\n"
      << "\tright 1.33*x\n"
      << "\tlook_at <" << lookatPos.x() << ", " << lookatPos.y() << ", " << lookatPos.z() << ">\n}\n\n"
      << "light_source {\n"
      << "\t<10, -10, 10>\n"
      << "\tcolor rgb<1,1,1>\n"
      << "}\n\n";
    //m_painter->drawSphere(Eigen::Vector3d(0, 0, 0), 2);
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

