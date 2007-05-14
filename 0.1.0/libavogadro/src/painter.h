/**********************************************************************
  Painter - drawing spheres and cylinders in a GLWidget

  Copyright (C) 2007 Benoit Jacob

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

#ifndef __PAINTER_H
#define __PAINTER_H

#include <avogadro/global.h>
#include <eigen/vector.h>
#include <QString>
#include <QPoint>

namespace Avogadro {

  /** @class Painter
    *
    * A simple Painter class for Avogadro. Currently it can draw spheres, cylinders,
    * multiple cylinders (as in multiple bonds) and text.
    *
    * Each GLWidget has a Painter which you can get by calling GLWidget::painter().
    *
    * The motivation for having a centralized Painter class is that it allows for global
    * quality control. Just call setGlobalQualitySetting() to adjust the quality setting.
    *
    * Another nice thing is that it handles level-of-detail computation, so that
    * you can just call drawSphere(center,radius) and the Painter automatically
    * determines the correct
    * detail level based on the radius of the sphere, on the distance to the camera, and on
    * the global quality setting.
    *
    * The multiple cylinders are drawn in such a way that the individual cylinders avoid
    * hiding each other, at least in the default viewpoint.
    *
    * The text is drawn with a black outlining so that it remains readable regardless of the
    * background color.
    */
  
  class A_EXPORT GLWidget;
  class PainterPrivate;
  class A_EXPORT Painter
  {
    public:
      Painter();
      ~Painter();

      void setGLWidget( GLWidget * widget );
      void setGlobalQualitySetting( int globalQualitySetting );
      int globalQualitySetting() const;
      void initialize( GLWidget * widget, int globalQualitySetting );

      void drawSphere( const Eigen::Vector3d & center, double radius, int detailLevel ) const;
      void drawSphere( const Eigen::Vector3d & center, double radius ) const;

      void drawCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
          double radius, int detailLevel ) const;
      void drawCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
          double radius) const;

      void drawMultiCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
          double radius, int order, double shift, int detailLevel ) const;
      void drawMultiCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
          double radius, int order, double shift ) const;

      int drawText( int x, int y, const QString &string ) const;
      int drawText( const QPoint& pos, const QString &string ) const;
      int drawText( const Eigen::Vector3d & pos, const QString &string ) const;
      void beginText() const;
      void endText() const;

    private:
      PainterPrivate * const d;
  };

} // end namespace Avogadro

#endif // _PAINTER_H
