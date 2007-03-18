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

#ifndef __CAMERA_H
#define __CAMERA_H

#include <avogadro/global.h>

#include <eigen/projective.h>
#include <eigen/matrix.h>
#include <eigen/vector.h>

namespace Avogadro {

  class A_EXPORT GLWidget;

  /**
   * This class represents a camera's position and orientation
   *
   * @class Camera
   * @author Benoit Jacob
   */
  class CameraPrivate;
  class A_EXPORT Camera
  {
    protected:
      friend class GLWidget;
      void setParent(const GLWidget *glwidget);

    public:
      Camera(const GLWidget *parent = 0, double angleOfViewY = 40.0);
      virtual ~Camera();
      const GLWidget *parent() const;
      void setAngleOfViewY(double angleOfViewY);
      double angleOfViewY() const;
      void setMatrix(const Eigen::MatrixP3d &matrix);
      const Eigen::MatrixP3d & matrix() const;
      Eigen::MatrixP3d & matrix();
      const double *matrixArray() const;
      double *matrixArray();
      
      void applyPerspective() const;
      void applyModelview() const;
      void initializeViewPoint();
      void translate(const Eigen::Vector3d &vector);
      void pretranslate(const Eigen::Vector3d &vector);
      void rotate(const double &angle, const Eigen::Vector3d &axis);
      void prerotate(const double &angle, const Eigen::Vector3d &axis);
      const Eigen::Vector3d translationVector() const;

    private:
      CameraPrivate * const d;

  };

} // end namespace Avogadro

#endif
