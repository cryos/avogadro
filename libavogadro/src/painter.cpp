/**********************************************************************
  Painter - drawing spheres and cylinders in a GLWidget

  Copyright (C) 2007 Benoit Jacob

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

#include <avogadro/painter.h>
#include <avogadro/glwidget.h>
#include <avogadro/sphere.h>
#include <avogadro/cylinder.h>
#include <avogadro/camera.h>

#include <cassert>

using namespace Eigen;
using namespace std;
using namespace Avogadro;

namespace Avogadro
{

  const int      PAINTER_MAX_DETAIL_LEVEL = PAINTER_DETAIL_LEVELS - 1;
  const double   PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL
                 = sqrt(PAINTER_SPHERES_LIMIT_MIN_LEVEL);
  const double   PAINTER_SPHERES_SQRT_LIMIT_MAX_LEVEL
                 = sqrt(PAINTER_SPHERES_LIMIT_MAX_LEVEL);
  const double   PAINTER_SPHERES_DETAIL_COEFF
                 = static_cast<double>(PAINTER_MAX_DETAIL_LEVEL - 1)
                 / (PAINTER_SPHERES_SQRT_LIMIT_MAX_LEVEL - PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL);

  class PainterPrivate
  {
    public:
      PainterPrivate() : widget(0), globalQualitySetting(0), spheres(0),
                         initialized(false) {};
      ~PainterPrivate()
      {
        deleteSpheres();
      }

      GLWidget *widget;

      int globalQualitySetting;

      Sphere **spheres;

      bool initialized;

      void deleteSpheres();
      void createSpheres();
  };

  void PainterPrivate::deleteSpheres()
  {
    if(spheres == 0) {
      return;
    }
    int sphereLevel, lastSphereLevel = -1;
    for(int n = 0; n < PAINTER_DETAIL_LEVELS; n++ )
    {
      sphereLevel = PAINTER_SPHERES_LEVELS_ARRAY[globalQualitySetting][n];
      if( sphereLevel != lastSphereLevel ) {
        lastSphereLevel = sphereLevel;
        if( spheres[n] ) {
          delete spheres[n];
          spheres[n] = 0;
        }
      }
    }
    delete[] spheres;
    spheres = 0;
  }

  void PainterPrivate::createSpheres()
  {
    assert(spheres == 0);
    spheres = new Sphere*[PAINTER_DETAIL_LEVELS];
    int sphereLevel, lastSphereLevel;
    lastSphereLevel = PAINTER_SPHERES_LEVELS_ARRAY[globalQualitySetting][0];
    spheres[0] = new Sphere( lastSphereLevel );
    for(int n = 1; n < PAINTER_DETAIL_LEVELS; n++ )
    {
      sphereLevel = PAINTER_SPHERES_LEVELS_ARRAY[globalQualitySetting][n];
      if( sphereLevel == lastSphereLevel ) {
        spheres[n] = spheres[n-1];
      }
      else {
        lastSphereLevel = sphereLevel;
        spheres[n] = new Sphere( sphereLevel );
      }
    }
  }

  Painter::Painter() : d(new PainterPrivate)
  {
  }

  Painter::~Painter()
  {
    delete d;
  }

  void Painter::setGLWidget( GLWidget * widget )
  {
    d->widget = widget;
  }
  
  void Painter::setGlobalQualitySetting( int globalQualitySetting )
  {
    assert( d->initialized );
    assert( globalQualitySetting >= 0 && globalQualitySetting < PAINTER_GLOBAL_QUALITY_SETTINGS );
    d->globalQualitySetting = globalQualitySetting;
    d->deleteSpheres();
    d->createSpheres();
  }

  int Painter::globalQualitySetting() const
  {
    return d->globalQualitySetting;
  }

  void Painter::initialize( GLWidget * widget, int globalQualitySetting )
  {
    assert( globalQualitySetting >= 0 && globalQualitySetting < PAINTER_GLOBAL_QUALITY_SETTINGS );
    d->initialized = true;
    setGLWidget(widget);
    setGlobalQualitySetting(globalQualitySetting);
  }
  
  void Painter::drawSphere( const Eigen::Vector3d & center, double radius, int detailLevel )
  {
    assert( d->initialized );
    assert( detailLevel >= 0 && detailLevel <= PAINTER_MAX_DETAIL_LEVEL );
    d->spheres[detailLevel]->draw(center, radius);
  }

  void Painter::drawSphere( const Eigen::Vector3d & center, double radius )
  {
    assert( d->initialized );
    double apparentRadius = radius / d->widget->camera().distance(center);
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

} // end namespace Avogadro
