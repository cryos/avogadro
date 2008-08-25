/**********************************************************************
  BondCentricTool - Bond Centric Manipulation Tool for Avogadro

  Copyright (C) 2007 by Shahzad Ali
  Copyright (C) 2007 by Ross Braithwaite
  Copyright (C) 2007 by James Bunt

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

#ifndef QUATERNION_H
#define QUATERNION_H

#include <Eigen/Core>

namespace Avogadro {

  /**
   * @class Quaternion
   * @brief A minimal implementation of Quaternions to allow for rotation in 3D space.
   * @author Shahzad Ali, Ross Braithwaite, James Bunt
   *
   * A minimal implementation of Quaternions to allow for rotation in 3D space.
   * The implementation only implements multiplication and finding the multiplicative
   * inverse.  Extra static functions for creating a rotation quaternion and
   * applying it in the smallest amount of multiplications possible have been added.
   * The Quaternion is of the form W + Xi + Yj + Zk,
   * where i*i = j*j = k*k = -1 and i*j = k, j*k = i, i*k = j.
   * The X, Y and Z components are represented by a Vector V.
   */
  class Quaternion
  {
    protected:
      double m_W;
      Eigen::Vector3d m_V;

    public:
      //! Constructor
      /**
       * Create a Quaternion with the given components.
       *
       * @param w W component of Quaternion
       * @param x X component of Quaternion
       * @param y Y component of Quaternion
       * @param z Z component of Quaternion
       */
      Quaternion(double w, double x, double y, double z);

      //! Constructor
      /**
       * Create a Quaternion with the given components.
       *
       * @param w W component of Quaternion
       * @param v Vector V, the X, Y and Z components of Quaternion.
       */
      Quaternion(double w, Eigen::Vector3d v);

      /**
       * Function to multiply two Quaternions and return the result.
       *
       * @param right Quaternion to multiply this by.
       *
       * @return Quaternion result of this * right.
       */
      Quaternion multiply(Quaternion right);

      /**
       * Function to multiply two Quaternions and return the
       * Vector component only, does not calculate extra W component.
       *
       * @param right Quaternion to multiply this by.
       *
       * @return Vector result of this * right.
       */
      Eigen::Vector3d multiplyToVector(Quaternion right);

      /**
       * Function to get the multiplicitive inverse of a Quaternion.
       *
       * @return Quaternion result of multiplicitive inverse of this.
       */
      Quaternion multiplicitiveInverse();

      /**
       * Returns W component of Quaternion.
       *
       * @return W component of Quaternion.
       */
      double w();

      /**
       * Returns X component of Quaternion.
       *
       * @return X component of Quaternion.
       */
      double x();

      /**
       * Returns Y component of Quaternion.
       *
       * @return Y component of Quaternion.
       */
      double y();

      /**
       * Returns Z component of Quaternion.
       *
       * @return Z component of Quaternion.
       */
      double z();

      /**
       * Returns Vector V, the X, Y and Z components of Quaternion.
       *
       * @return Vector V, the X, Y and Z components of Quaternion.
       */
      Eigen::Vector3d v();

      /**
       * Returns the length of the Quaternion.
       * sqrt(w*w + x*x + y*y + z*z)
       *
       * @return Length of the Quaternion.
       */
      double norm();

      /**
       * Returns the length of the Quaternion squared.
       * w*w + x*x + y*y +z*z
       *
       * @return Length of the Quaternnion squared.
       */
      double norm2();

      /**
       * Function to create the left hand half of a pair of Rotation Quaternions.
       *
       * @param theta Angle of rotation in radians
       * @param rotationVector Unit vector defining the axis of rotation.
       *
       * @return Quaternion of the left half of a pair of rotation quaternions.
       */
      static Quaternion createRotationLeftHalf(double theta, Eigen::Vector3d rotationVector);

      /**
       * Function to apply a pair of Rotation Quaternions and return the result in 3D space.
       * rotationLeft must be produced by createRotationLeftHalf, and rotationRight must be
       * its multiplicitive inverse.
       *
       * @param rotationLeft Left half of rotation Quaternion Pair
       * @param direction Vector representing vector to be rotated.
       * @param rotationRight Right half of rotation Quaternion Pair.
       *
       * @return Vector representing the location after rotation is applied.
       */
      static Eigen::Vector3d performRotationMultiplication(Quaternion rotationLeft,
          Eigen::Vector3d direction, Quaternion rotationRight);
  };
} // End namespace Avogadro

#endif /*__QUATERNION_H*/
