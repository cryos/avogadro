/**********************************************************************
  LeastSquares - Least squares functions removed from Eigen2 codebase

  Copyright (C) 2006-2009 Benoit Jacob <jacob.benoit.1@gmail.com>

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#ifndef LEASTSQUARES_H
#define LEASTSQUARES_H

#include <Eigen/Eigenvalues>

namespace Avogadro
{

template<typename VectorType, typename HyperplaneType>
void fitHyperplane(int numPoints,
                   VectorType **points,
                   HyperplaneType *result,
                   typename Eigen::NumTraits<typename VectorType::Scalar>::Real* soundness = 0)
{
  typedef typename VectorType::Scalar Scalar;
  typedef Eigen::Matrix<Scalar,VectorType::SizeAtCompileTime,VectorType::SizeAtCompileTime> CovMatrixType;
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(VectorType)
  assert(numPoints >= 1);
  int size = points[0]->size();
  assert(size+1 == result->coeffs().size());

  // compute the mean of the data
  VectorType mean = VectorType::Zero(size);
  for(int i = 0; i < numPoints; ++i)
    mean += *(points[i]);
  mean /= numPoints;

  // compute the covariance matrix
  CovMatrixType covMat = CovMatrixType::Zero(size, size);
  VectorType remean = VectorType::Zero(size);
  for(int i = 0; i < numPoints; ++i)
  {
    VectorType diff = (*(points[i]) - mean).conjugate();
    covMat += diff * diff.adjoint();
  }

  // now we just have to pick the eigen vector with smallest eigen value
  Eigen::SelfAdjointEigenSolver<CovMatrixType> eig(covMat);
  result->normal() = eig.eigenvectors().col(0);
  if (soundness)
    *soundness = eig.eigenvalues().coeff(0)/eig.eigenvalues().coeff(1);

  // let's compute the constant coefficient such that the
  // plane pass trough the mean point:
  result->offset() = - (result->normal().array() * mean.array()).sum();
}

} // end namespace Avogadro

#endif
