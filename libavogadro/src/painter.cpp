/**********************************************************************
  Painter - drawing spheres, cylinders and text in a GLWidget

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

#include <config.h>

#include <avogadro/painter.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include "sphere.h"
#include "cylinder.h"
#include "textrenderer.h"

#include <cassert>

using namespace Eigen;
using namespace std;
using namespace Avogadro;

namespace Avogadro
{

  const int      PAINTER_GLOBAL_QUALITY_SETTINGS       = 3;
  const int      DEFAULT_GLOBAL_QUALITY_SETTING        = PAINTER_GLOBAL_QUALITY_SETTINGS - 1;
  const int      PAINTER_DETAIL_LEVELS                 = 10;
  const int      PAINTER_SPHERES_LEVELS_ARRAY[3][10]
    = { {0, 0, 1, 1, 2, 2, 3, 3, 4, 4} ,
      {0, 1, 2, 3, 4, 4, 5, 5, 6, 6} ,
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 9} };
  const double   PAINTER_SPHERES_LIMIT_MIN_LEVEL       = 0.005;
  const double   PAINTER_SPHERES_LIMIT_MAX_LEVEL       = 0.15;

  const int      PAINTER_CYLINDERS_LEVELS_ARRAY[3][10]
    = { {0, 3, 5, 5, 8, 8, 12, 12, 16, 16} ,
      {0, 4, 6, 9, 12, 12, 16, 16, 20, 20},
      {0, 4, 6, 10, 14, 18, 22, 26, 32, 40} };
  const double   PAINTER_CYLINDERS_LIMIT_MIN_LEVEL     = 0.001;
  const double   PAINTER_CYLINDERS_LIMIT_MAX_LEVEL     = 0.03;
  const int      PAINTER_MAX_DETAIL_LEVEL = PAINTER_DETAIL_LEVELS - 1;
  const double   PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL
    = sqrt(PAINTER_SPHERES_LIMIT_MIN_LEVEL);
  const double   PAINTER_SPHERES_SQRT_LIMIT_MAX_LEVEL
    = sqrt(PAINTER_SPHERES_LIMIT_MAX_LEVEL);
  const double   PAINTER_SPHERES_DETAIL_COEFF
    = static_cast<double>(PAINTER_MAX_DETAIL_LEVEL - 1)
    / (PAINTER_SPHERES_SQRT_LIMIT_MAX_LEVEL - PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL);
  const double   PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL
    = sqrt(PAINTER_CYLINDERS_LIMIT_MIN_LEVEL);
  const double   PAINTER_CYLINDERS_SQRT_LIMIT_MAX_LEVEL
    = sqrt(PAINTER_CYLINDERS_LIMIT_MAX_LEVEL);
  const double   PAINTER_CYLINDERS_DETAIL_COEFF
    = static_cast<double>(PAINTER_MAX_DETAIL_LEVEL - 1)
    / (PAINTER_CYLINDERS_SQRT_LIMIT_MAX_LEVEL - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL);

  class PainterPrivate
  {
    public:
      PainterPrivate() : widget(0), quality(0), spheres(0), cylinders(0),
      textRenderer(new TextRenderer), initialized(false), shared(false)  {};
      ~PainterPrivate()
      {
        deleteObjects();
        delete textRenderer;
      }

      GLWidget *widget;

      int quality;

      /** array of pointers to Spheres. You might ask, why not have
        * a plain array of Spheres. The idea is that more than one global detail level
        * may use a given sphere detail level. It is therefore interesting to be able
        * to share that sphere, instead of having redundant spheres in memory.
        */
      Sphere **spheres;
      /** array of pointers to Cylinders. You might ask, why not have
        * a plain array of Cylinders. The idea is that more than one global detail level
        * may use a given cylinder detail level. It is therefore interesting to be able
        * to share that cylinder, instead of having redundant cylinder in memory.
        */
      Cylinder **cylinders;
      
      TextRenderer *textRenderer;

      bool initialized;

      void deleteObjects();
      void createObjects();

      /**
       * Painters can be shared, we must keep track of this.
       */
      int shared;

  };

  void PainterPrivate::deleteObjects()
  {
    int level, lastLevel, n;
    // delete the spheres. One has to be wary that more than one sphere
    // pointer may have the same value. One wants to avoid deleting twice the same sphere.
    if(spheres) {
      lastLevel = -1;
      for(n = 0; n < PAINTER_DETAIL_LEVELS; n++)
      {
        level = PAINTER_SPHERES_LEVELS_ARRAY[quality][n];
        if( level != lastLevel ) {
          lastLevel = level;
          if( spheres[n] ) {
            delete spheres[n];
            spheres[n] = 0;
          }
        }
      }
      delete[] spheres;
      spheres = 0;
    }
    
    // delete the cylinders. One has to be wary that more than one cylinder
    // pointer may have the same value. One wants to avoid deleting twice the same cylinder.
    if(cylinders) {
      lastLevel = -1;
      for(n = 0; n < PAINTER_DETAIL_LEVELS; n++)
      {
        level = PAINTER_CYLINDERS_LEVELS_ARRAY[quality][n];
        if( level != lastLevel ) {
          lastLevel = level;
          if( cylinders[n] ) {
            delete cylinders[n];
            cylinders[n] = 0;
          }
        }
      }
      delete[] cylinders;
      cylinders = 0;
    }
  }

  void PainterPrivate::createObjects()
  {
    int level, lastLevel, n;
    // create the spheres. More than one sphere detail level may have the same value.
    // in that case we want to reuse the corresponding sphere by just copying the pointer,
    // instead of creating redundant spheres.
    if(spheres == 0)
    {
      spheres = new Sphere*[PAINTER_DETAIL_LEVELS];
      int level, lastLevel;
      lastLevel = PAINTER_SPHERES_LEVELS_ARRAY[quality][0];
      spheres[0] = new Sphere( lastLevel );
      for(int n = 1; n < PAINTER_DETAIL_LEVELS; n++ )
      {
        level = PAINTER_SPHERES_LEVELS_ARRAY[quality][n];
        if( level == lastLevel ) {
          spheres[n] = spheres[n-1];
        }
        else {
          lastLevel = level;
          spheres[n] = new Sphere( level );
        }
      }
    }
    
    // create the cylinders. More than one cylinder detail level may have the same value.
    // in that case we want to reuse the corresponding cylinder by just copying the pointer,
    // instead of creating redundant cylinders.
    if(cylinders == 0)
    {
      cylinders = new Cylinder*[PAINTER_DETAIL_LEVELS];
      int level, lastLevel;
      lastLevel = PAINTER_SPHERES_LEVELS_ARRAY[quality][0];
      cylinders[0] = new Cylinder( lastLevel );
      for(int n = 1; n < PAINTER_DETAIL_LEVELS; n++ )
      {
        level = PAINTER_CYLINDERS_LEVELS_ARRAY[quality][n];
        if( level == lastLevel ) {
          cylinders[n] = cylinders[n-1];
        }
        else {
          lastLevel = level;
          cylinders[n] = new Cylinder( level );
        }
      }
    }
  }

  Painter::Painter(int quality) : d(new PainterPrivate)
  {
    if(quality < 0 || quality >= PAINTER_MAX_DETAIL_LEVEL) {
      quality = DEFAULT_GLOBAL_QUALITY_SETTING;
    }
    d->quality = quality;
  }

  Painter::~Painter()
  {
    delete d;
  }

//   void Painter::setGLWidget( GLWidget * widget )
//   {
//     d->widget = widget;
//     d->textRenderer->setGLWidget(d->widget);
//   }
// 
  void Painter::setQuality( int quality )
  {
    assert( quality >= 0 && quality < PAINTER_GLOBAL_QUALITY_SETTINGS );
    d->deleteObjects();
    d->quality = quality;
    d->createObjects();
  }

  int Painter::quality() const
  {
    return d->quality;
  }

//   void Painter::initialize( GLWidget * widget, int quality )
//   {
//     if(quality == -1) { 
//       quality = DEFAULT_GLOBAL_QUALITY_SETTING; 
//     }
//     else {
//       assert( quality >= 0 && quality < PAINTER_GLOBAL_QUALITY_SETTINGS );
//     }
//     d->initialized = true;
//     setGLWidget(widget);
//     setQuality(quality);
//   }

  void Painter::drawSphere( const Eigen::Vector3d & center, double radius, int detailLevel ) const
  {
    if(d->textRenderer->isActive())
    {
      d->textRenderer->end();
    }
    assert( d->widget );
    assert( detailLevel >= 0 && detailLevel <= PAINTER_MAX_DETAIL_LEVEL );
    d->spheres[detailLevel]->draw(center, radius);
  }

  void Painter::drawSphere( const Eigen::Vector3d & center, double radius ) const
  {
    if(d->textRenderer->isActive())
    {
      d->textRenderer->end();
    }
    assert( d->widget );
    double apparentRadius = radius / d->widget->camera()->distance(center);
    int detailLevel = 1 + static_cast<int>( floor(
          PAINTER_SPHERES_DETAIL_COEFF * (sqrt(apparentRadius) - PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL)
          ) );
    if( detailLevel < 0 ) {
      detailLevel = 0;
    }
    if( detailLevel > PAINTER_MAX_DETAIL_LEVEL ) {
      detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }
    d->spheres[detailLevel]->draw(center, radius);
  }

  void Painter::drawCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
      double radius, int detailLevel ) const
  {
    if(d->textRenderer->isActive())
    {
      d->textRenderer->end();
    }
    assert( d->widget );
    assert( detailLevel >= 0 && detailLevel <= PAINTER_MAX_DETAIL_LEVEL );
    d->cylinders[detailLevel]->draw(end1, end2, radius);
  }

  void Painter::drawCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
      double radius) const
  {
    if(d->textRenderer->isActive())
    {
      d->textRenderer->end();
    }
    assert( d->widget );
    double apparentRadius = radius / d->widget->camera()->distance(end1);
    int detailLevel = 1 + static_cast<int>( floor(
          PAINTER_CYLINDERS_DETAIL_COEFF
          * (sqrt(apparentRadius) - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL)
          ) );
    if( detailLevel < 0 ) {
      detailLevel = 0;
    }
    if( detailLevel > PAINTER_MAX_DETAIL_LEVEL ) {
      detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }
    d->cylinders[detailLevel]->draw(end1, end2, radius);
  }

  void Painter::drawMultiCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
      double radius, int order, double shift, int detailLevel ) const
  {
    if(d->textRenderer->isActive())
    {
      d->textRenderer->end();
    }
    assert( d->widget );
    assert( detailLevel >= 0 && detailLevel <= PAINTER_MAX_DETAIL_LEVEL );
    d->cylinders[detailLevel]->drawMulti(end1, end2, radius, order,
        shift, d->widget->normalVector() );
  }

  void Painter::drawMultiCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
      double radius, int order, double shift ) const
  {
    if(d->textRenderer->isActive())
    {
      d->textRenderer->end();
    }
    assert( d->widget );
    double apparentRadius = radius / d->widget->camera()->distance(end1);
    int detailLevel = 1 + static_cast<int>( floor(
          PAINTER_CYLINDERS_DETAIL_COEFF
          * (sqrt(apparentRadius) - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL)
          ) );
    if( detailLevel < 0 ) {
      detailLevel = 0;
    }
    if( detailLevel > PAINTER_MAX_DETAIL_LEVEL ) {
      detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }
    d->cylinders[detailLevel]->drawMulti(end1, end2, radius, order,
        shift, d->widget->normalVector());
  }

  int Painter::drawText( int x, int y, const QString &string ) const
  {
    if(!d->textRenderer->isActive())
    {
      d->textRenderer->begin(d->widget);
    }
    return d->textRenderer->draw(x, y, string);
  }

  int Painter::drawText( const QPoint& pos, const QString &string ) const
  {
    if(!d->textRenderer->isActive())
    {
      d->textRenderer->begin(d->widget);
    }
    return d->textRenderer->draw(pos.x(), pos.y(), string);
  }

  int Painter::drawText( const Eigen::Vector3d &pos, const QString &string ) const
  {
    if(!d->textRenderer->isActive())
    {
      d->textRenderer->begin(d->widget);
    }
    return d->textRenderer->draw(pos, string);
  }

  void Painter::begin(GLWidget *widget)
  {
    if(!d->initialized)
    {
      setQuality(d->quality);
      d->initialized = true;
    }
    d->widget = widget;
  }

  void Painter::end()
  {
    if(d->textRenderer->isActive())
    {
      d->textRenderer->end();
    }
    d->widget = 0;
  }

  int Painter::defaultQuality()
  {
    return DEFAULT_GLOBAL_QUALITY_SETTING;
  }

  int Painter::maxQuality()
  {
    return PAINTER_GLOBAL_QUALITY_SETTINGS-1;
  }

} // end namespace Avogadro
