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

#ifndef __QUATERNION_H
#define __QUATERNION_H

#include <eigen/vector.h>

namespace Avogadro {

  /*
   * @class Quaternion
   * @brief <TODO>
   * @author Shahzad Ali, Ross Braithwaite, James Bunt
   *
   * <TODO>
   */
  class Quaternion
  {
    protected:
      double m_W;
      Eigen::Vector3d m_V;

    public:
      //! Constructor
      /*
       * <TODO>
       *
       * @param w <TODO>
       * @param x <TODO>
       * @param y <TODO>
       * @param z <TODO>
       */
      Quaternion(double w, double x, double y, double z);

      //! Constructor
      /*
       * <TODO>
       *
       * @param w <TODO>
       * @param v <TODO>
       */
      Quaternion(double w, Eigen::Vector3d v);

      /*
       * <TODO>
       *
       * @param right <TODO>
       *
       * @return <TODO>
       */
      Quaternion multiply(Quaternion right);

      /*
       * <TODO>
       *
       * @param right <TODO>
       *
       * @return <TODO>
       */
      Eigen::Vector3d multiplyToVector(Quaternion right);

      /*
       * <TODO>
       *
       * @return <TODO>
       */
      Quaternion multiplicitiveInverse();

      /*
       * <TODO>
       *
       * @return <TODO>
       */
      double w();

      /*
       * <TODO>
       *
       * @return <TODO>
       */
      double x();

      /*
       * <TODO>
       *
       * @return <TODO>
       */
      double y();

      /*
       * <TODO>
       *
       * @return <TODO>
       */
      double z();

      /*
       * <TODO>
       *
       * @return <TODO>
       */
      Eigen::Vector3d v();

      /*
       * <TODO>
       *
       * @return <TODO>
       */
      double norm();

      /*
       * <TODO>
       *
       * @return <TODO>
       */
      double norm2();

      /*
       * <TODO>
       *
       * @param theta <TODO>
       * @param rotationVector <TODO>
       *
       * @return <TODO>
       */
      static Quaternion createRotationLeftHalf(double theta, Eigen::Vector3d rotationVector);

      /*
       * <TODO>
       *
       * @param rotationLeft <TODO>
       * @param direction <TODO>
       * @param rotationRight <TODO>
       *
       * @return <TODO>
       */
      static Eigen::Vector3d performRotationMultiplication(Quaternion rotationLeft,
          Eigen::Vector3d direction, Quaternion rotationRight);
  };
} // End namespace Avogadro

#endif /*__QUATERNION_H*/
