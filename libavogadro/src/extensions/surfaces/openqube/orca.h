/******************************************************************************

  This source file is part of the OpenQube project.

  Copyright 2014 Dagmar Lenk

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef ORCA_H
#define ORCA_H

#include <QStringList>

#include <QtCore/QIODevice>
#include <Eigen/Core>
#include <vector>

#include "gaussianset.h"

class QString;

namespace OpenQube
{

class OPENQUBE_EXPORT ORCAOutput
{
  // Parsing mode: section of the file currently being parsed
  enum mode { NotParsing, Atoms, GTO, MO};


public:
  ORCAOutput(const QString &filename, GaussianSet *basis);
  ~ORCAOutput();
  void outputAll();
  bool success() {return (m_orcaSuccess);}
private:
  QIODevice *m_in;
  void processLine(GaussianSet *basis);
  void load(GaussianSet *basis);
  void calculateDensityMatrix();

  // OrcaStuff

  void orcaWarningMessage(const QString &m);
  orbital orbitalIdx(QString txt);
  bool m_orcaSuccess;

  QStringList m_atomLabel;
  QStringList m_basisAtomLabel;

  std::vector<int> shellFunctions;
  std::vector<orbital> shellTypes;
  std::vector<std::vector<int> > m_orcaNumShells;
  std::vector<std::vector<orbital> > m_orcaShellTypes;
  int m_nGroups;


  std::vector<std::vector<std::vector<Eigen::Vector2d> *> *> m_basisFunctions;

  //
  double m_coordFactor;
  mode m_currentMode;
  int m_electrons;

  bool m_openShell;
  bool m_useBeta;

  int m_homo;

  int m_currentAtom;
  unsigned int m_numBasisFunctions;
  std::vector<orbital> m_shellTypes;
  std::vector<int> m_shellNums;
  std::vector<int> m_shelltoAtom;
  std::vector<double> m_a;
  std::vector<double> m_c;
  std::vector<double> m_csp;
  std::vector<double> m_orbitalEnergy;
  std::vector<double> m_MOcoeffs;

  Eigen::MatrixXd m_density;     /// Total density matrix
};

} // End namespace

#endif
