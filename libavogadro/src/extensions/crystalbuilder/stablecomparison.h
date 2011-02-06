/**********************************************************************
  StableComp - Functions for comparing numbers with a tolerance

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#define STABLE_COMP_TOL 1e-5

namespace Avogadro {
  namespace StableComp {

    inline bool lt(const double &v1,
                          const double &v2,
                          const double &prec =
                          STABLE_COMP_TOL)
    {
      return ( v1 < (v2 - prec) );
    }

    inline bool gt(const double &v1,
                          const double &v2,
                          const double &prec =
                          STABLE_COMP_TOL)
    {
      return ( v2 < (v1 - prec) );
    }

    inline bool eq(const double &v1,
                          const double &v2,
                          const double &prec =
                          STABLE_COMP_TOL)
    {
      return (!(lt(v1,v2,prec) ||
                gt(v1,v2,prec) ) );
    }

    inline bool neq(const double &v1,
                           const double &v2,
                           const double &prec =
                           STABLE_COMP_TOL)
    {
      return (!(eq(v1,v2,prec)));
    }

    inline bool leq(const double &v1,
                           const double &v2,
                           const double &prec =
                           STABLE_COMP_TOL)
    {
      return (!gt(v1, v2, prec));
    }

    inline bool geq(const double &v1,
                           const double &v2,
                           const double &prec =
                           STABLE_COMP_TOL)
    {
      return (!lt(v1, v2, prec));
    }

    inline double sign(const double &v)
    {
      // consider 0 to be positive
      if (v >= 0) return 1.0;
      else return -1.0;
    }
  }
}
