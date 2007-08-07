/**********************************************************************
  BondCentricTool - Bond Centric Manipulation Tool for Avogadro

  Copyright (C) 2007 by Shahzad Ali
  Copyright (C) 2007 by Ross Braithwaite
  Copyright (C) 2007 by James Bunt
  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2006,2007 by Benoit Jacob

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

#include "quaternion.h"

using namespace Avogadro;
using namespace Eigen;

// ##########  Constructor  ##########

Quaternion::Quaternion(double w, double x, double y, double z)
{
  m_W = w;
  m_V = Vector3d(x,y,z);
}

// ##########  Constructor  ##########

Quaternion::Quaternion(double w, Vector3d v)
{
  m_W = w;
  m_V = v;
}

// ##########  w  ##########

double Quaternion::w()
{
  return m_W;
}

// ##########  x  ##########

double Quaternion::x()
{
  return m_V.x();
}

// ##########  y  ##########

double Quaternion::y()
{
  return m_V.y();
}

// ##########  z  ##########

double Quaternion::z()
{
  return m_V.z();
}

// ##########  v  ##########

Vector3d Quaternion::v()
{
  return m_V;
}

// ##########  norm  ##########

double Quaternion::norm()
{
  return sqrt(norm2());
}

// ##########  norm2  ##########

double Quaternion::norm2()
{
  return w() * w() + v().norm2();
}

// ##########  multiply  ##########

Quaternion Quaternion::multiply(Quaternion right)
{
  return Quaternion(w() * right.w() - v().dot(right.v()),
                    w() * right.x() + x() * right.w() + y() * right.z() - z() * right.y(),
                    w() * right.y() - x() * right.z() + y() * right.w() + z() * right.x(),
                    w() * right.z() + x() * right.y() - y() * right.x() + z() * right.w());
}

// ##########  multiplyToVector  ##########

Vector3d Quaternion::multiplyToVector(Quaternion right)
{
  return Vector3d(w() * right.x() + x() * right.w() + y() * right.z() - z() * right.y(),
                  w() * right.y() - x() * right.z() + y() * right.w() + z() * right.x(),
                  w() * right.z() + x() * right.y() - y() * right.x() + z() * right.w());
}

// ##########  multiplicitiveInverse  ##########

Quaternion Quaternion::multiplicitiveInverse()
{
  double divisor = norm2();
  return Quaternion(w() / divisor, - v() / divisor);
}

// ##########  createRotationLeftHalf  ##########

Quaternion Quaternion::createRotationLeftHalf(double theta, Vector3d rotationVector)
{
  double angleHalf = theta/2.0;
  double sinAngle = sin(angleHalf);

  return Quaternion(cos(angleHalf), rotationVector.x() * sinAngle, 
                    rotationVector.y() * sinAngle, rotationVector.z() * sinAngle);
}

// ##########  performRotationMultiplication  ##########

Vector3d Quaternion::performRotationMultiplication(Quaternion left, 
    Vector3d direction, Quaternion right)
{
  return left.multiply(Quaternion(0,direction)).multiplyToVector(right);
}

