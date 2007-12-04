/**********************************************************************
  Camera - Class for representing the view.

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
#include <avogadro/camera.h>
#include <avogadro/glwidget.h>

using namespace Eigen;

namespace Avogadro
{
  class CameraPrivate
  {
    public:
      CameraPrivate() {};

      MatrixP3d modelview, projection;
      const GLWidget *parent;
      double angleOfViewY;
  };

  Camera::Camera(const GLWidget *parent, double angleOfViewY) : d(new CameraPrivate)
  {
    d->modelview.loadIdentity();
    d->projection.loadIdentity();
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
    d->modelview.getLinearComponent(&m);

    m.getColumn(0, &c0);
    c0.normalize();
    m.setColumn(0, c0);
    m.getColumn(1, &c1);
    c1.normalize();
    c1 -= dot(c0, c1) * c0;
    c1.normalize();
    m.setColumn(1, c1);
    m.getColumn(2, &c2);
    c2.normalize();
    c2 -= dot(c0, c2) * c0;
    c2 -= dot(c1, c2) * c1;
    c2.normalize();
    m.setColumn(2, c2);

    d->modelview.setLinearComponent(m);
    d->modelview.matrix().setRow(3, Vector4d(0.0, 0.0, 0.0, 1.0));
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
    d->modelview.translate(vector);
  }

  void Camera::pretranslate(const Eigen::Vector3d &vector)
  {
    d->modelview.pretranslate(vector);
  }

  void Camera::rotate(const double &angle, const Eigen::Vector3d &axis)
  {
    d->modelview.rotate3(angle, axis);
    normalize();
  }

  void Camera::prerotate(const double &angle, const Eigen::Vector3d &axis)
  {
    d->modelview.prerotate3(angle, axis);
    normalize();
  }

  const double Camera::distance(const Eigen::Vector3d & point) const
  {
    return ( d->modelview * point ).norm();
  }

  void Camera::setModelview(const Eigen::MatrixP3d &matrix)
  {
    d->modelview = matrix;
  }

  const Eigen::MatrixP3d & Camera::modelview() const
  {
    return d->modelview;
  }

  Eigen::MatrixP3d & Camera::modelview()
  {
    return d->modelview;
  }

  void Camera::initializeViewPoint()
  {
    d->modelview.loadIdentity();
    if( d->parent == 0 ) return;
    if( d->parent->molecule() == 0 ) return;

    // if the molecule is empty, we want to look at its center
    // (which is probably at the origin, but who knows) from some distance
    // (here 10.0).
    if( d->parent->molecule()->NumAtoms() == 0 )
    {
      d->modelview.translate( d->parent->center() - Vector3d( 0, 0, 10 ) );
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
    setModelview(rotation);

    // now we want to move backwards, in order
    // to view the molecule from a distance, not from inside it.
    // This translation must be applied after the above rotation, so we
    // want a left-multiplication here. Whence pretranslate().
    const Vector3d Zaxis(0,0,1);
    pretranslate( - 3.0 * ( d->parent->radius() + CAMERA_NEAR_DISTANCE ) * Zaxis );

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

    double molRadius = d->parent->radius() + CAMERA_MOL_RADIUS_MARGIN;
    double distanceToMolCenter = distance( d->parent->center() );
    double zNear = std::max( CAMERA_NEAR_DISTANCE, distanceToMolCenter - molRadius );
    double zFar = distanceToMolCenter + molRadius;
    double aspectRatio = static_cast<double>(d->parent->width()) / d->parent->height();
    gluPerspective( d->angleOfViewY, aspectRatio, zNear, zFar );
    glGetDoublev(GL_PROJECTION_MATRIX, d->projection.array());
  }

  void Camera::applyModelview() const
  {
    glMultMatrixd( d->modelview.array() );
  }

  Eigen::Vector3d Camera::unProject(const Eigen::Vector3d & v) const
  {
    GLint viewport[4] = {0, 0, parent()->width(), parent()->height() };
    Eigen::Vector3d pos;
    gluUnProject(v.x(), parent()->height() - v.y(), v.z(),
                 d->modelview.array(), d->projection.array(), viewport, &pos.x(), &pos.y(), &pos.z());
    return pos;
  }

  Eigen::Vector3d Camera::unProject(const QPoint& p, const Eigen::Vector3d& ref) const
  {
    return unProject( Eigen::Vector3d( p.x(), p.y(), project(ref).z() ));
  }

  Eigen::Vector3d Camera::unProject(const QPoint& p) const
  {
    return unProject(p, parent()->center());
  }

  Eigen::Vector3d Camera::project(const Eigen::Vector3d & v) const
  {
    GLint viewport[4] = {0, 0, parent()->width(), parent()->height() };
    Eigen::Vector3d pos;
    gluProject(v.x(), v.y(), v.z(),
               d->modelview.array(), d->projection.array(), viewport, &pos.x(), &pos.y(), &pos.z());

    pos.y() = parent()->height() - pos.y();
    return pos;
  }

  Eigen::Vector3d Camera::backTransformedXAxis() const
  {
    return Eigen::Vector3d( d->modelview(0, 0),
                            d->modelview(0, 1),
                            d->modelview(0, 2) );
  }

  Eigen::Vector3d Camera::backTransformedYAxis() const
  {
    return Eigen::Vector3d( d->modelview(1, 0),
                            d->modelview(1, 1),
                            d->modelview(1, 2) );
  }

  Eigen::Vector3d Camera::backTransformedZAxis() const
  {
    return Eigen::Vector3d( d->modelview(2, 0),
                            d->modelview(2, 1),
                            d->modelview(2, 2) );
  }

  Eigen::Vector3d Camera::transformedXAxis() const
  {
    // The x unit vector in space coordinates
    return Eigen::Vector3d( d->modelview(0, 0),
                            d->modelview(1, 0),
                            d->modelview(2, 0) );
  }

  Eigen::Vector3d Camera::transformedYAxis() const
  {
    // The y unit vector in space coordinates
    return Eigen::Vector3d( d->modelview(0, 1),
                            d->modelview(1, 1),
                            d->modelview(2, 1) );
  }

  Eigen::Vector3d Camera::transformedZAxis() const
  {
    // The z unit vector in space coordinates
    return Eigen::Vector3d( d->modelview(0, 2),
                            d->modelview(1, 2),
                            d->modelview(2, 2) );
  }

} // end namespace Avogadro
