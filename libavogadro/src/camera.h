/**********************************************************************
  Camera - Class for representing the view.

	Copyright (c) 2007-2008 Benoit Jacob
	Copyright (c) 2007 Donald Ephraim Curtis
	Copyright (c) 2007 Carsten Niehaus
	Copyright (c) 2007-2008 Marcus D. Hanwell
	Copyright (c) 2007-2009 Geoff Hutchison
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

#ifndef CAMERA_H
#define CAMERA_H

#include <avogadro/global.h>
#include <Eigen/Geometry>
#include <QPoint>

namespace Avogadro {

  class GLWidget;

  /**
   * @class Camera camera.h <avogadro/camera.h>
   * @brief Representation of the camera looking at the molecule.
   * @author Benoit Jacob
   *
   * This class represents a camera looking at the molecule loaded
   * in a GLWidget. It stores the parameters describing the camera and
   * a pointer to the parent GLWidget. It uses this to retrieve information
   * about the molecule being looked at, automatically setting up the OpenGL
   * projection matrix to ensure that the molecule will not be clipped. It also
   * provides a method to initialize a nice default viewpoint of the molecule.
   * In order to setup the OpenGL matrices before rendering the molecule, do the
   * following:
   * @code
    // setup the OpenGL projection matrix using the camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    camera.applyPerspective();

    // setup the OpenGL modelview matrix using the camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.applyModelview();
   * @endcode
   * The reason why Camera class does not provide a single method to do all of this
   * is that in some cases you don't want to. For instance, when doing OpenGL selection,
   * you want to call gluPickMatrix() right before Camera::applyPerspective().
   */
  class CameraPrivate;
  class A_EXPORT Camera
  {
    protected:
      friend class GLWidget;
      /** Sets which GLWidget owns this camera.
        * @sa parent() */
      void setParent(const GLWidget *glwidget);

    public:
      /** The constructor.
        * @param parent the GLWidget parent of this camera instance.
        * @param angleOfViewY the vertical viewing angle in degrees.
        * @sa setParent(), setAngleOfViewY() */
      explicit Camera(const GLWidget *parent = 0, double angleOfViewY = 40.0);
      /** The destructor. */
      virtual ~Camera();
      /**
       * The copy constructor - it is useful to be able to copy the Camera.
       */
      Camera(const Camera *camera);
      /** @return a pointer to the parent GLWidget.
        * @sa setParent() */
      const GLWidget *parent() const;
      /** Sets the vertical viewing angle.
        * @param angleOfViewY the new vertical viewing angle in degrees.
        * @sa angleOfViewY() */
      void setAngleOfViewY(double angleOfViewY);
      /** @return the vertical viewing angle in degrees.
        * @sa setAngleOfViewY() */
      double angleOfViewY() const;
      /** Sets 4x4 "modelview" matrix representing the camera orientation and position.
        * @param matrix the matrix to copy from
        * @sa Eigen::Transform3d & modelview(), applyModelview() */
      void setModelview(const Eigen::Transform3d &matrix);
      /** @return a constant reference to the 4x4 "modelview" matrix representing
        *         the camera orientation and position
        * @sa setModelview(), Eigen::Transform3d & modelview() */
      const Eigen::Transform3d & modelview() const;
      /** @return a non-constant reference to the 4x4 "modelview" matrix representing
        *         the camera orientation and position
        * @sa setModelview(), const Eigen::Transform3d & modelview() const */
      Eigen::Transform3d & modelview();
      /** Calls gluPerspective() with parameters automatically chosen
        * for rendering the GLWidget's molecule with this camera. Should be called
        * only in GL_PROJECTION matrix mode. Example code is given
        * in the class's comment.
        * @sa applyModelview(), initializeViewPoint()
        */
      void applyPerspective() const;
      /** Calls glMultMatrix() with the camera's "modelview" matrix. Should be called
        * only in GL_MODELVIEW matrix mode. Example code is given
        * in the class's comment.
        * @sa applyPerspective(), initializeViewPoint()
        */
      void applyModelview() const;
      /** Sets up the camera so that it gives a nice view of the molecule loaded in the
        * parent GLWidget. Typically you would call this method right after loading a molecule.
        * @sa applyPerspective(), applyModelview()
        */
      void initializeViewPoint();
      /** Returns the distance between @a point and the camera. For instance, to determine the
        * distance between a molecule's center and the camera, do:
        * @code
          double d = camera.distance( molecule.center() );
        * @endcode
        */
      double distance(const Eigen::Vector3d & point) const;
      /** Multiply the camera's "modelview" matrix on the right by the translation of the given
        * vector. As the translation is applied on the right, the vector is understood in
        * the molecule's coordinate system. Use this method if you want to give the impression 
        * that the molecule is moving while the camera remains fixed. This is the equivalent of 
        * the OpenGL function glTranslate().
        * @param vector the translation vector
        * @sa pretranslate(), translationVector()*/
      void translate(const Eigen::Vector3d &vector);
      /** Multiply the camera's "modelview" matrix on the left by the translation of given
        * vector. Because the translation is applied on the left, the vector is understood in
        * the coordinate system obtained by applying the camera's matrix to the molecule's
        * coordinate system. Use this method if you want to give the impression that the camera
        * is moving while the molecule remains fixed.
        * @warning This is NOT the equivalent of the OpenGL function glTranslate().
        * @param vector the translation vector
        * @sa translate(), translationVector()*/
      void pretranslate(const Eigen::Vector3d &vector);
      /** Multiply the camera's "modelview" matrix on the right by the rotation of the given
        * angle and axis. As the rotation is applied on the right, the axis vector is
        * understood in the molecule's coordinate system. Use this method if you want to give
        * the impression that the molecule is rotating while the camera remains fixed. This is the
        * equivalent of the OpenGL function glRotate(), except that here the angle is expressed
        * in radians, not in degrees.
        *
        * After the rotation is multiplied, a normalization is performed to ensure that the
        * camera matrix remains sane.
        *
        * @param angle the rotation angle in radians
        * @param axis a unit vector around which to rotate. This MUST be a unit vector, i.e.
        *             axis.norm() must be close to 1.
        * @sa prerotate()*/
      void rotate(const double &angle, const Eigen::Vector3d &axis);
      /** Multiply the camera's "modelview" matrix on the left by the rotation of the given
        * angle and axis. Because the rotation is applied on the left, the axis vector is
        * understood in the the coordinate system obtained by applying the camera's matrix to
        * the molecule's coordinate system. Use this method if you want to give
        * the impression that the camera is rotating while the molecule remains fixed.
        *
        * After the rotation is multiplied, a normalization is performed to ensure that the
        * camera matrix remains sane.
        *
        * @warning This is NOT the equivalent of the OpenGL function glRotate().
        * @param angle the rotation angle in radians
        * @param axis a unit vector around which to rotate. This MUST be a unit vector, i.e.
        *             axis.norm() must be close to 1.
        * @sa prerotate()*/
      void prerotate(const double &angle, const Eigen::Vector3d &axis);

      /**
       * Performs an unprojection from window coordinates to space coordinates.
       * @param v The vector to unproject, expressed in window coordinates.
       *          Thus v.x() and v.y() are the x and y coords of the pixel to unproject.
       *          v.z() represents it's "z-distance". If you don't know what value to
       *          put in v.z(), see the other unProject(const QPoint&) method.
       * @return vector of the unprojected space coordinates
       *
       * @sa unProject(const QPoint&), project()
       */
      Eigen::Vector3d unProject(const Eigen::Vector3d& v) const;

      /**
       * Performs an unprojection from window coordinates to space coordinates,
       * into the plane passing through a given reference point and parallel to the screen.
       * Thus the returned vector is a point of that plane. The rationale is that
       * when unprojecting 2D window coords to 3D space coords, there are a priori
       * infinitely many solutions, and one has to be choose. This is equivalent to
       * choosing a plane parallel to the screen.
       * @param p the point to unproject, expressed in window coordinates.
       * @param ref the reference point, determining the plane into which to unproject.
       *            If you don't know what to put here, see the other
       *            unProject(const QPoint&) method.
       * @return vector containing the unprojected space coordinates
       *
       * @sa unProject(const Eigen::Vector3d&), unProject(const QPoint&), project()
       */
      Eigen::Vector3d unProject(const QPoint& p, const Eigen::Vector3d& ref) const;

      /**
       * Performs an unprojection from window coordinates to space coordinates,
       * into the plane passing through the molecule's center and parallel to the screen.
       * Thus the returned vector is a point belonging to that plane. This is equivalent to
       * @code
         unProject( p, center() );
       * @endcode
       * @param p the point to unproject, expressed in window coordinates.
       * @return vector containing the unprojected space coordinates.
       *
       * @sa unProject(const Eigen::Vector3d&),
       *     unProject(const QPoint&, const Eigen::Vector3d&), project()
       */
      Eigen::Vector3d unProject(const QPoint& p) const;

      /**
       * Performs a projection from space coordinates to window coordinates.
       * @param v the vector to project, expressed in space coordinates.
       * @return vector containing the projected screen coordinates.
       *
       * @sa unProject(const Eigen::Vector3d&), unProject(const QPoint&),
       *     unProject(const QPoint&, const Eigen::Vector3d&)
       */
      Eigen::Vector3d project(const Eigen::Vector3d& v) const;

      /**
       * Returns a unit vector pointing toward the right, expressed in the
       * scene's coordinate system. This is simply the
       * unit vector that is mapped to (1,0,0) by the camera rotation.
       */
      Eigen::Vector3d backTransformedXAxis() const;

      /**
       * Returns a unit vector pointing upward, expressed in the
       * scene's coordinate system. This is simply the
       * unit vector that is mapped to (0,1,0) by the camera rotation.
       */
      Eigen::Vector3d backTransformedYAxis() const;

      /**
       * Returns a unit vector pointing toward the camera, expressed in the
       * scene's coordinate system. This is simply the
       * unit vector that is mapped to (0,0,1) by the camera rotation.
       */
      Eigen::Vector3d backTransformedZAxis() const;

      /**
       * Returns a unit vector pointing in the x direction, expressed in the
       * space coordinate system.
       */
      Eigen::Vector3d transformedXAxis() const;

      /**
       * Returns a unit vector pointing in the y direction, expressed in the
       * space coordinate system.
       */
      Eigen::Vector3d transformedYAxis() const;

      /**
       * Returns a unit vector pointing in the z direction, expressed in the
       * space coordinate system.
       */
      Eigen::Vector3d transformedZAxis() const;

      /**
       * Obtain a normal vector and point defining the plane of the
       * clipping plane nearest the camera.
       *
       * @param normal Overwritten with a normalized vector orthogonal
       * to the plane. All planes have two normal vectors -- this
       * vector is the one pointing into the viewing volume.
       * @param point Overwritten with a point that lies within the
       * plane. This is the point at (-1, -1, -1) in normalized device
       * coordinates (i.e. the bottom left corner of the viewport).
       * @return True if the clipping plane exists, false otherwise.
       */
      bool nearClippingPlane(Eigen::Vector3d *normal, Eigen::Vector3d *point);

      /** The linear component (ie the 3x3 topleft block) of the camera matrix must
        * always be a rotation. But after several hundreds of operations on it,
        * it can drift farther and farther away from being a rotation. This method
        * normalizes the camera matrix so that the linear component is guaranteed to be
        * a rotation. Concretely, it performs a Gram-Schmidt orthonormalization to
        * transform the linear component into a nearby rotation.
        *
        * The bottom row must always have entries 0, 0, 0, 1. This function overwrites
        * the bottom row with these values.
        */
      void normalize();

    private:
      CameraPrivate * const d;

      /**Given {x, y, z, w} returns {x, y, z} / w.
       *
       * @param v4 {x,y,z,x} vector
       * @return {x/w, y/w, z/w} vector
       */
      Eigen::Vector3d V4toV3DivW(const Eigen::Vector4d & v4) {
        return v4.start<3>()/v4.w();
      }
  };

} // end namespace Avogadro

#endif // __CAMERA_H
