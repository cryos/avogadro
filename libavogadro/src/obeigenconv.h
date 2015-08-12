/**********************************************************************
  obeigenconv - Collection of functions to convert between Eigen and
  OpenBabel vectors / matrices

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#ifndef OBEIGENCONV_H
#define OBEIGENCONV_H

#include "config.h"

#include <openbabel/math/vector3.h>
#include <openbabel/math/matrix3x3.h>

#include <Eigen/Core>

namespace Avogadro {

  inline Eigen::Matrix3d OB2Eigen(const OpenBabel::matrix3x3 &obm)
  {
    Eigen::Matrix3d mat;
    mat(0,0)=obm.Get(0,0);mat(0,1)=obm.Get(0,1);mat(0,2)=obm.Get(0,2);
    mat(1,0)=obm.Get(1,0);mat(1,1)=obm.Get(1,1);mat(1,2)=obm.Get(1,2);
    mat(2,0)=obm.Get(2,0);mat(2,1)=obm.Get(2,1);mat(2,2)=obm.Get(2,2);
    return mat;
  }

  inline OpenBabel::matrix3x3 Eigen2OB(const Eigen::Matrix3d &mat)
  {
    OpenBabel::matrix3x3 obm;
    obm.Set(0,0,mat(0,0));obm.Set(0,1,mat(0,1));obm.Set(0,2,mat(0,2));
    obm.Set(1,0,mat(1,0));obm.Set(1,1,mat(1,1));obm.Set(1,2,mat(1,2));
    obm.Set(2,0,mat(2,0));obm.Set(2,1,mat(2,1));obm.Set(2,2,mat(2,2));
    return obm;
  }

  inline Eigen::Vector3d OB2Eigen(const OpenBabel::vector3 &obv)
  {
    Eigen::Vector3d vec (obv.x(), obv.y(), obv.z());
    return vec;
  }

  inline OpenBabel::vector3 Eigen2OB(const Eigen::Vector3d &vec)
  {
    OpenBabel::vector3 obv (vec.x(), vec.y(), vec.z());
    return obv;
  }
}
#endif
