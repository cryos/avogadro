/**********************************************************************
  Camera - Class for representing the view.

  Copyright (C) 2007 Benoit Jacob
  Copyright (C) 2011 David C. Lonie

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

#include "camera.h"

#include "glwidget.h"

#include <avogadro/molecule.h>

#include <openbabel/generic.h>
#include <openbabel/math/vector3.h>

#include <Eigen/Geometry>
#include <Eigen/LU>

#include <GL/glu.h>

using namespace Eigen;

namespace Avogadro
{
  class CameraPrivate
  {
    public:
      EIGEN_MAKE_ALIGNED_OPERATOR_NEW

      CameraPrivate() {};

      Eigen::Transform3d modelview, projection;
      const GLWidget *parent;
      double angleOfViewY;
  };

  Camera::Camera(const GLWidget *parent, double angleOfViewY) : d(new CameraPrivate)
  {
    d->modelview.setIdentity();
    d->projection.setIdentity();
    d->parent = parent;
    d->angleOfViewY = angleOfViewY;
  }

  Camera::~Camera()
  {
    delete d;
  }

  Camera::Camera(const Camera *camera) : d(new CameraPrivate)
  {
    d->modelview = camera->d->modelview;
    d->projection = camera->d->projection;
    d->parent = camera->d->parent;
    d->angleOfViewY = camera->d->angleOfViewY;
  }

  void Camera::setParent(const GLWidget *parent)
  {
    d->parent = parent;
  }

  void Camera::normalize()
  {
    Eigen::Block<Matrix4d, 3, 1> c0(d->modelview.matrix(), 0, 0),
                                 c1(d->modelview.matrix(), 0, 1),
                                 c2(d->modelview.matrix(), 0, 2);
    c0.normalize();
    c1.normalize();
    c1 -= c0.dot(c1) * c0;
    c1.normalize();
    c2.normalize();
    c2 -= c0.dot(c2) * c0;
    c2 -= c1.dot(c2) * c1;
    c2.normalize();
    d->modelview.matrix().row(3) << 0, 0, 0, 1;
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
    d->modelview.rotate(Eigen::AngleAxisd(angle, axis));
    normalize();
  }

  void Camera::prerotate(const double &angle, const Eigen::Vector3d &axis)
  {
    d->modelview.prerotate(Eigen::AngleAxisd(angle, axis));
    normalize();
  }

  double Camera::distance(const Eigen::Vector3d & point) const
  {
    return ( d->modelview * point ).norm();
  }

  void Camera::setModelview(const Eigen::Transform3d &matrix)
  {
    d->modelview = matrix;
  }

  const Eigen::Transform3d & Camera::modelview() const
  {
    return d->modelview;
  }

  Eigen::Transform3d & Camera::modelview()
  {
    return d->modelview;
  }

  void Camera::initializeViewPoint()
  {
    d->modelview.setIdentity();
    if( d->parent == 0 ) return;
    if( d->parent->molecule() == 0 ) return;

    // If we have a unit cell, initialize centered on the center of the unit
    // cell, zoomed out enough so that the cell fits nicely in the window
    // with the a-axis aligned with x, b in the x,y plane, and z pointing
    // upwards.
    OpenBabel::OBUnitCell *cell = d->parent->molecule()->OBUnitCell();
    if (cell != NULL) {
      std::vector<OpenBabel::vector3> obvecs = cell->GetCellVectors();

      // Account for translational images (aCells, bCells, cCells)
      const Eigen::Vector3d v1 ((obvecs[0] * d->parent->aCells()).AsArray());
      const Eigen::Vector3d v2 ((obvecs[1] * d->parent->bCells()).AsArray());
      const Eigen::Vector3d v3 ((obvecs[2] * d->parent->cCells()).AsArray());

      Eigen::Matrix3d targetBasis;
      targetBasis.row(0) = v1.normalized();
      targetBasis.row(1) = v1.cross(v2).normalized();
      targetBasis.row(2) = targetBasis.row(0).cross(targetBasis.row(1)).normalized();

      // Translate to the center of the a-c face
      translate((-(v1 + v3) * 0.5));

      // Rotate into our target basis
      Eigen::AngleAxisd rot (targetBasis);
      prerotate(rot.angle(), rot.axis());

      // Move back so that we get a nice, consistent view of the cell:
      //
      //     ^\        v = angleOfView(X/Y) / 2.0
      //     | \       d = desired (width/height) of viewport
      // d/2 |  \        . . .
      //     | v(\     z = distance to move backwards
      //     +---->      = d / (2.0 * tan(v))
      //       z         = d * 0.5 / tan(v)
      //
      // Think of the "desired" width/height defining an 2D orthogonal bounding
      // box around the unit cell's projection onto the x,y plane at z=0.
      //
      // We will determine which is most appropriate based on the ratios
      // cellHeight/viewHeight   and   cellWidth/viewWidth.
      //
      // If the height ratio larger than the width ratio, use the height to
      // determine the distance to move back, and vice-versa.
      //
      // The cellWidth is:
      //
      // max(proj_v1(v1 + v3).norm(), proj_v1(v1 - v3).norm()) * fudge factor
      //
      // Use the sign of the dot product to determine which is larger.
      // (acute angles will use the addition, obtuse will use subtraction)
      const double cellWidth = (v1.dot(v3) > 0) ?
            targetBasis.row(0).dot(v1 + v3) : // targetBasis.row(0) = v1.normalized().
            targetBasis.row(0).dot(v1 - v3) ; // this saves a norm and a division

      // The same logic holds for the cell height, but we only need
      // to project v3 onto v1.cross(v2), a.k.a. targetBasis.row(1) (which is
      // handily already normalized):
      const double cellHeight = fabs(targetBasis.row(1).dot(v3));

      // Grab the viewport dimensions
      const double viewWidth  = static_cast<double>(d->parent->width());
      const double viewHeight = static_cast<double>(d->parent->height());

      // Determine which direction to use for scaling, width or height
      const double fudgeFactor = 1.25; // Fractional multiplier for cellDims
      if ( cellWidth / viewWidth > cellHeight / viewHeight ) {
        // Scale by width
        const double zTrans = -fudgeFactor * cellWidth * 0.5 /
            tan(d->angleOfViewY * DEG_TO_RAD * 0.5);

        pretranslate(Eigen::Vector3d(0.0, 0.0, zTrans));
      }
      else {
        // Scale by height
        const double angleOfViewX = d->angleOfViewY * viewHeight / viewWidth;
        const double zTrans = -fudgeFactor * cellHeight * 0.5 /
            tan(angleOfViewX * DEG_TO_RAD * 0.5);

        pretranslate(Eigen::Vector3d(0.0, 0.0, zTrans));
      }

      return;
    }

    // if the molecule is empty, we want to look at its center
    // (which is probably at the origin, but who knows) from some distance
    // (here 20.0) -- this gives us some room to work PR#1964674
    if( d->parent->molecule()->numAtoms() < 2 )
    {
      d->modelview.translate( d->parent->center() - Vector3d( 0.0, 0.0, 20.0 ) );
      return;
    }

    // if we're here, we have a non-empty isolated molecule.
    // we want a top-down view on it, i.e. the molecule should fit as well as
    // possible in the (X,Y)-plane. Equivalently, we want the Z axis to be parallel
    // to the normal vector of the molecule's fitting plane.
    // Thus we construct a suitable base-change rotation.
    Matrix3d rotation;
    rotation.row(2) = d->parent->normalVector();
    rotation.row(0) = rotation.row(2).unitOrthogonal();
    rotation.row(1) = rotation.row(2).cross(rotation.row(0));

    // set the camera's matrix to be (the 4x4 version of) this rotation.
    d->modelview.linear() = rotation;

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
    glGetDoublev(GL_PROJECTION_MATRIX, d->projection.data());
  }

  void Camera::applyModelview() const
  {
    glMultMatrixd( d->modelview.data() );
  }

  Eigen::Vector3d Camera::unProject(const Eigen::Vector3d & v) const
  {
    GLint viewport[4] = {0, 0, parent()->width(), parent()->height() };
    Eigen::Vector3d pos;
    gluUnProject(v.x(), parent()->height() - v.y(), v.z(),
                 d->modelview.data(), d->projection.data(), viewport, &pos.x(), &pos.y(), &pos.z());
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
               d->modelview.data(), d->projection.data(), viewport, &pos.x(), &pos.y(), &pos.z());

    pos.y() = parent()->height() - pos.y();
    return pos;
  }

  Eigen::Vector3d Camera::backTransformedXAxis() const
  {
    return d->modelview.linear().row(0).transpose();
  }

  Eigen::Vector3d Camera::backTransformedYAxis() const
  {
    return d->modelview.linear().row(1).transpose();
  }

  Eigen::Vector3d Camera::backTransformedZAxis() const
  {
    return d->modelview.linear().row(2).transpose();
  }

  Eigen::Vector3d Camera::transformedXAxis() const
  {
    return d->modelview.linear().col(0);
  }

  Eigen::Vector3d Camera::transformedYAxis() const
  {
    return d->modelview.linear().col(1);
  }

  Eigen::Vector3d Camera::transformedZAxis() const
  {
    return d->modelview.linear().col(2);
  }

  bool Camera::nearClippingPlane(Vector3d *normal, Vector3d *point)
  {
    // Determine near plane from three coplanar points:
    // (http://www.songho.ca/opengl/gl_projectionmatrix.html is a
    // helpful resource here.)

    // We will convert following points (which are in the near plane)
    // from NDC coordinates to object coordinates:
    //
    // (-1, -1, -1), (1,-1,-1), and (-1,1,-1).
    //
    // First get the current transformation matrix (T = PM, P is
    // projection matrix, M is modelview matrix), which converts
    // Object coordinates (O) to NDC coordinates (N) via:
    //
    // N = T O
    //
    // These are stored in the private class, no need to query OpenGL
    // for them.
    const Matrix4d &proj = d->projection.matrix();
    const Matrix4d &modv = d->modelview.matrix();

    // Now invert the matrix so that we can find our three coplanar
    // points in Object coordinates via:
    //
    // O = Inv(T) N
    //
    // Calculate T ( = PM ) here, too:
    const Matrix4d invT ((proj * modv).inverse());

    // Now to get three points and a normal vector:
    // (V4toV3DivW converts {x,y,z,w} to {x,y,z}/w)
    *point = V4toV3DivW(invT * Vector4d(-1,-1,-1,1) );
    const Vector3d p1 ( V4toV3DivW(invT * Vector4d(1,-1,-1,1) ));
    const Vector3d p2 ( V4toV3DivW(invT * Vector4d(-1,1,-1,1) ));

    // This cross product ensures that the normal points into the
    // viewing volume:
    *normal = (p2-(*point)).cross(p1-(*point)).normalized();

    return true;
  }

} // end namespace Avogadro
