/**********************************************************************
  SurfaceMeshGenerator - Class to generate meshes from volumetric data

  Copyright (C) 2009 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
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

#ifndef SURFACEMESHGENERATOR_H
#define SURFACEMESHGENERATOR_H

#include <avogadro/global.h>

#include <QThread>
#include <Eigen/Core>
#include <vector>


namespace Avogadro {

  class Cube;
  class Atom;
  class Mesh;

} // End namespace Avogadro

#endif

