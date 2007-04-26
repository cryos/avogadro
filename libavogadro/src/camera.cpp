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

  void Camera::normalize()
  {
    Matrix3d m;
    Vector3d c0, c1, c2;
    d->matrix.getLinearComponent(&m);

    m.getColumn(0, &c0);
    c0.normalize();
    m.setColumn(0, c0);
    m.getColumn(1, &c1);
    c1.normalize();
    c1 -= dot(c0, c1) * c1;
    c1.normalize();
    m.setColumn(1, c1);
    m.getColumn(2, &c2);
    c2.normalize();
    c2 -= dot(c0, c2) * c2;
    c2 -= dot(c1, c2) * c2;
    c2.normalize();
    m.setColumn(2, c2);

    d->matrix.setLinearComponent(m);
    d->matrix.matrix().setRow(3, Vector4d(0.0, 0.0, 0.0, 1.0));
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
    normalize();
  }
  
  void Camera::prerotate(const double &angle, const Eigen::Vector3d &axis)
  {
    d->matrix.prerotate3(angle, axis);
    normalize();
  }
  
  const double Camera::distance(const Eigen::Vector3d & point) const
  {
    return ( d->matrix * point ).norm();
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
    if( d->parent->molecule() == 0 ) return;

    // if the molecule is empty, we want to look at its center
    // (which is probably at the origin, but who knows) from some distance
    // (here 10.0).
    if( d->parent->molecule()->NumAtoms() == 0 )
    {
      d->matrix.translate( d->parent->center() - Vector3d( 0, 0, 10 ) );
      return;
    }
    
    // if we're here, the molecule is not empty, i.e. has atoms.
    // we want a top-down view on it, i.e. the molecule should fit as well as
    // possible in the (X,Y)-plane. Equivalently, we want the Z axis to be parallel
    // to the normal vector of the molecule's fitting plane.
    // Thus we construct a suitable base-change rotation.
    Matrix3d rotation;
    rotation.setRow(2, d->parent->normalVector());
    rotation.setRow(0, rotation.row(2).ortho());
    rotation.setRow(1, rotation.row(2).cross(rotation.row(0)));

    // set the camera's matrix to be (the 4x4 version of) this rotation.
    setMatrix(rotation);
  
    // now we want to move backwards, in order
    // to view the molecule from a distance, not from inside it.
    // This translation must be applied after the above rotation, so we
    // want a left-multiplication here. Whence pretranslate().
    const Vector3d Zaxis(0,0,1);
    pretranslate( - 3.0 * d->parent->radius() * Zaxis );
    
    // the above rotation is meant to be a rotation around the molecule's
    // center. So before this rotation is applied, the molecule's center
    // must be brought to the origin of the coordinate systemby a translation.
    // As this translation must be applied first, we want a right-multiplication here.
    // Whence translate().
    translate( - d->parent->center() );
  }
  
  void Camera::applyPerspective() const
  {
    if( d->parent == 0 ) return;
    if( d->parent->molecule() == 0 ) return;

    // radius() returns the radius without electrons. We add 0.3 nanometer,
    // which is about the biggest possible VDW radius of an atom.
    double molRadius = d->parent->radius() + CAMERA_MOL_RADIUS_MARGIN;
    double distanceToMolCenter = distance( d->parent->center() );
    double nearEnd = std::max( CAMERA_NEAR_DISTANCE, distanceToMolCenter - molRadius );
    double farEnd = distanceToMolCenter + molRadius;

    double aspectRatio = static_cast<double>(d->parent->width()) / d->parent->height();
    gluPerspective( d->angleOfViewY, aspectRatio, nearEnd, farEnd );
  }

  void Camera::applyModelview() const
  {
    glMultMatrixd( matrixArray() );
  }

} // end namespace Avogadro
