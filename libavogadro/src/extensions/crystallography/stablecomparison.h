/**********************************************************************
  StableComp - Functions for comparing numbers with a tolerance

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#ifndef CE_STABLECOMPARISON
#define CE_STABLECOMPARISON

#define STABLE_COMP_TOL 1e-5

namespace Avogadro {
  namespace StableComp {

    bool lt(double v1,
            double v2,
            double prec =
            STABLE_COMP_TOL)
    {
      return ( v1 < (v2 - prec) );
    }

    bool gt(double v1,
            double v2,
            double prec =
            STABLE_COMP_TOL)
    {
      return ( v2 < (v1 - prec) );
    }

    bool eq(double v1,
            double v2,
            double prec =
            STABLE_COMP_TOL)
    {
      return (!(lt(v1,v2,prec) ||
                gt(v1,v2,prec) ) );
    }

    bool neq(double v1,
             double v2,
             double prec =
             STABLE_COMP_TOL)
    {
      return (!(eq(v1,v2,prec)));
    }

    bool leq(double v1,
             double v2,
             double prec =
             STABLE_COMP_TOL)
    {
      return (!gt(v1, v2, prec));
    }

    bool geq(double v1,
             double v2,
             double prec =
             STABLE_COMP_TOL)
    {
      return (!lt(v1, v2, prec));
    }

    double sign(double v)
    {
      // consider 0 to be positive
      if (v >= 0) return 1.0;
      else return -1.0;
    }

    double round(double v,
                 int dec)
    {
      const double shift = pow(10.0, dec);
      const double shifted = v * shift;
      return floor(shifted+0.5) / shift;
    }
  }
}

#endif
