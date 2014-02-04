/**********************************************************************
  AvoTubeGen -- interface between Avogadro and TubeGen

  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 **********************************************************************/

#ifndef AVOTUBEGEN_H
#define AVOTUBEGEN_H

#include <QtCore/QObject>

namespace Avogadro {
  class Molecule;
}

namespace SWCNTBuilder
{

class AvoTubeGen : public QObject
{
  Q_OBJECT
public:
  explicit AvoTubeGen(QObject *parent = NULL);
  virtual ~AvoTubeGen();

  // This class will not clean up the molecule, be sure to retrieve and
  // delete it if you build one
  Avogadro::Molecule * getMolecule() {return m_molecule;}

public Q_SLOTS:
  // length is the number of periodic cell lengths if periodicCell==true.
  // Otherwise, length is in Angstrom.
  // dbonds determines whether or not to search for double bonds (slow)
  void buildCarbonNanotube(unsigned int n, unsigned int m,
                           bool periodicCell,
                           double length,
                           bool cap = true, bool dbonds = false);

Q_SIGNALS:
  void buildFinished();

protected:
  void addTranslationalUnits(unsigned int num, double maxz);
  void trimTube(double maxz);
  void capTube();
  void perceiveSingleBonds();
  void perceiveDoubleBonds();
  Avogadro::Molecule *m_molecule;
};

} // end namespace SWCNTBuilder

#endif // AVOTUBEGEN_H
