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

#ifndef __MOLGEOMINFO_H
#define __MOLGEOMINFO_H

#include <avogadro/global.h>
#include <avogadro/primitives.h>

#include <eigen/vector.h>

namespace Avogadro {

  class MolGeomInfoPrivate;
  class A_EXPORT MolGeomInfo
  {
    public:
      MolGeomInfo(const Molecule *molecule = 0);
      virtual ~MolGeomInfo();
      void compute(const Molecule *molecule);

      const Eigen::Vector3d & center() const;
      const Eigen::Vector3d & normalVector() const;
      const double & radius() const;
      const Atom *farthestAtom() const;

    private:

      MolGeomInfoPrivate * const d;

  };

} // end namespace Avogadro

#endif // __MOLGEOMINFO_H

