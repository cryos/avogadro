/***************************************************************************
 *   copyright            : (C) 2007 by Benoit Jacob
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

using namespace Eigen;

namespace Avogadro
{
  class CameraPrivate
  {
    public:
      CameraPrivate() {};

      MatrixP3d matrix;
      const GLWidget *parent;
      double angleOfViewY;
  };
  
  Camera::Camera(const GLWidget *parent, double angleOfViewY) : d(new CameraPrivate)
  {
    d->matrix.loadIdentity();
    d->parent = parent;
    d->angleOfViewY = angleOfViewY;
  }
  
  Camera::~Camera()
  {
    delete d;
  }
  
  void Camera::setParent(const GLWidget *parent)
  {
    d->parent = parent;
  }
    
  const GLWidget *Camera::parent() const
  {
    return d->parent;
  }
    
  void Camera::setAngleOfViewY(double angleOfViewY)
  {
    d->angleOfViewY = angleOfViewY;
  }
  
  double Camera::angleOfViewY() const
  {
    return d->angleOfViewY;
  }
  
  void Camera::translate(const Eigen::Vector3d &vector)
  {
    d->matrix.translate(vector);
  }
  
  void Camera::pretranslate(const Eigen::Vector3d &vector)
  {
    d->matrix.pretranslate(vector);
  }
  
  void Camera::rotate(const double &angle, const Eigen::Vector3d &axis)
  {
    d->matrix.rotate3(angle, axis);
  }
  
  void Camera::prerotate(const double &angle, const Eigen::Vector3d &axis)
  {
    d->matrix.prerotate3(angle, axis);
  }
  
  const Eigen::Vector3d Camera::translationVector() const
  {
    return d->matrix.translationVector();
  }
  
  void Camera::setMatrix(const Eigen::MatrixP3d &matrix)
  {
    d->matrix = matrix;
  }
  
  const Eigen::MatrixP3d & Camera::matrix() const
  {
    return d->matrix;
  }
  
  Eigen::MatrixP3d & Camera::matrix()
  {
    return d->matrix;
  }
  
  const double *Camera::matrixArray() const
  {
    return d->matrix.array();
  }
  
  double *Camera::matrixArray()
  {
    return d->matrix.array();
  }
  
  void Camera::initializeViewPoint()
  {
    d->matrix.loadIdentity();
    if( d->parent == 0 ) return;
    if( d->parent->molecule() == 0 || d->parent->molecule()->NumAtoms() == 0 )
    {
      d->matrix.translate( Vector3d( 0, 0, -10 ) );
      return;
    }
    
    Matrix3d rotation;
    rotation.setRow(2, d->parent->molGeomInfo().normalVector());
    rotation.setRow(0, rotation.row(2).ortho());
    rotation.setRow(1, rotation.row(2).cross(rotation.row(0)));
    setMatrix(rotation);
  
    const Vector3d Zaxis(0,0,1);
    pretranslate( - 2 * d->parent->molGeomInfo().radius() * Zaxis );
    
    translate( - d->parent->molGeomInfo().center() );
  }
  
  void Camera::applyPerspective() const
  {
    double aspectRatio, nearEnd, farEnd;
    if( d->parent == 0 ) return;
    if( d->parent->molecule() == 0 || d->parent->molecule()->NumAtoms() == 0 )
    {
      nearEnd = 1.0;
      farEnd = 20.0;
    }
    else
    {
      double molRadius = d->parent->molGeomInfo().radius();
      Eigen::Vector3d molCenter = d->parent->molGeomInfo().center();
      double distanceToMol = (translationVector() - molCenter).norm();
      if( distanceToMol < 2.0 * molRadius)
      {
        nearEnd = molRadius / 12.0;
        farEnd = molRadius * 4.0;
      }
      else
      {
        nearEnd = distanceToMol - molRadius * 1.5;
        farEnd = distanceToMol + molRadius * 1.5;
      }
    }
    aspectRatio = static_cast<double>(d->parent->width()) / d->parent->height();
    gluPerspective( d->angleOfViewY, aspectRatio, nearEnd, farEnd );
  }

  void Camera::applyModelview() const
  {
    glMultMatrixd( matrixArray() );
  }

} // end namespace Avogadro
