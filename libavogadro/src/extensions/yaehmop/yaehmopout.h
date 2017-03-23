/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2016 Kitware

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef YAEHMOP_OUT_H
#define YAEHMOP_OUT_H

#include <QtCore/QIODevice>
#include <QVector>

#include <Eigen/Dense>

typedef Eigen::Matrix<double, 3, 1> Vector3;

class QString;

typedef struct {
  QString label;
  Vector3 coords;
} specialKPoint;

typedef Vector3 kpoint;

typedef QVector<double> band;

// Static class for Yaehmop output
class YaehmopOut
{
public:
  YaehmopOut();
  ~YaehmopOut();
  // Pass the yaehmop output in as 'data'. It would be faster if this only
  // included the section from BAND_DATA to END_BAND_DATA, but it is not
  // necessary. This sets bands, kpoints, and specialKPoints to be the
  // bands, the kpoints, and the special k points. Returns true if the
  // read was successful, and false if the read failed
  static bool readBandData(const QString& data,
                           QVector<band>& bands,
                           QVector<kpoint>& kpoints,
                           QVector<specialKPoint>& specialKPoints);

  // Pass in the yaehmop DOS data as 'data'. This searches the data for
  // the fermi level and sets it to @param fermi. If the fermi energy
  // is found, this returns true. Otherwise, it returns false.
  static bool getFermiLevelFromDOSData(const QString& data, double& fermi);


  // Pass the yaehmop output in as 'data'. It would be faster if this only
  // included the section from TOTAL DENSITY OF STATES and END OF DOS, but
  // it is not necessary. This sets densities and energies to be the
  // densities and energies for the DOS plot. Returns true if the read
  // was successful, and false if the read failed.
  static bool readTotalDOSData(const QString& data,
                               QVector<double>& densities,
                               QVector<double>& energies);
};

#endif
