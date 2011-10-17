/**********************************************************************
  AvoTubeGen -- interface between Avogadro and TubeGen

  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 **********************************************************************/

#ifndef AVOTUBEGEN_H
#define AVOTUBEGEN_H

namespace Avogadro {
  class Molecule;
}

namespace SWCNTBuilder
{

namespace AvoTubeGen
{

/**
  * Returns a new molecule containing a carbon nanotube.
  */
Avogadro::Molecule * getCarbonNanotube(unsigned int n, unsigned int m,
                                       unsigned int length,
                                       bool cap = false);

} // end namespace AvoTubeGen

} // end namespace SWCNTBuilder

#endif // AVOTUBEGEN_H
