/******************************************************************************

  This source file is part of the OpenQube project.

  Copyright 2010 Geoffrey R. Hutchison

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MOLDEN_H
#define MOLDEN_H

#include "config.h"

#include <Eigen/Core>
#include <vector>

#include "gaussianset.h"

namespace OpenQube
{

class MoldenFile
{
  // Parsing mode: section of the file currently being parsed
  enum mode { NotParsing, Atoms, GTO, STO, MO, SCF };
public:
  MoldenFile(const QString &filename, GaussianSet *basis);
  ~MoldenFile();
  void outputAll();
private:
  QIODevice *m_in;
  void processLine(GaussianSet* basis);
  void load(GaussianSet* basis);
  void unnormalizeBasis();

  double m_coordFactor;
  mode m_currentMode;
  int m_electrons;

  bool m_sphericalD;
  bool m_sphericalG;

  unsigned int m_numBasisFunctions;
  std::vector<int> m_aNums;
  std::vector<double> m_aPos;
  std::vector<orbital> m_shellTypes;
  std::vector<int> m_shellNums;
  std::vector<int> m_shelltoAtom;
  std::vector<double> m_a;
  std::vector<double> m_c;
  std::vector<double> m_csp;
  std::vector<double> m_orbitalEnergy;
  std::vector<double> m_MOcoeffs;
};

} // End namespace

#endif
