/******************************************************************************

  This source file is part of the OpenQube project.

  Copyright (C) 2016 Kitware

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "yaehmopout.h"

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

YaehmopOut::YaehmopOut()
{
}

YaehmopOut::~YaehmopOut()
{
}

inline bool printAndReturnFalse(const QString& error)
{
  qDebug() << "Error in YaehmopOut::readBandData():" << error;
  return false;
}

bool YaehmopOut::readBandData(const QString& data, QVector<band>& bands,
                              QVector<kpoint>& kpoints,
                              QVector<specialKPoint>& specialKPoints)
{
  bands.clear();
  kpoints.clear();
  specialKPoints.clear();

  QStringList lines = data.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

  while (lines.size() != 0 && !lines[0].contains("#BAND_DATA"))
    lines.removeFirst();

  if (lines.size() == 0)
    return printAndReturnFalse("Band Data not found in readBandData()!");

  // These get printed from the status file and are not needed...
  foreach(const QString& line, lines) {
    if (line.contains("Error value from Diagonalization"))
      lines.removeOne(line);
  }

  // Right here, lines.size() must be at least 8
  if (lines.size() < 8)
    return printAndReturnFalse("Band data is too few lines!");

  size_t ind = 2;
  if (!lines[ind].contains("Special points"))
    return printAndReturnFalse("Special Points missing");
  size_t numSpecialPoints = lines[ind].split(" ")[0].toInt();

  // Now we know more detail about how many lines should be here
  if (lines.size() < 7 + numSpecialPoints)
    return printAndReturnFalse("Too few lines of data in band data!");

  ++ind;
  if (!lines[ind].contains("k points"))
    return printAndReturnFalse("k points missing");
  size_t numKPoints = lines[ind].split(" ")[0].toInt();

  ++ind;
  if (!lines[ind].contains("orbitals in"))
    return printAndReturnFalse("orbitals in missing");;
  size_t numOrbitals = lines[ind].split(" ")[0].toInt();

  for (size_t i = 0; i < numSpecialPoints; ++i) {
    ++ind;
    specialKPoint kp;
    if (lines[ind].split(" ").size() < 4)
      return printAndReturnFalse("Special points line too small");

    kp.label = lines[ind].split(" ")[0];
    kp.coords = Vector3(lines[ind].split(" ")[1].toDouble(),
                        lines[ind].split(" ")[2].toDouble(),
                        lines[ind].split(" ")[3].toDouble());
    specialKPoints.append(kp);
  }

  ++ind;
  if (!lines[ind].contains("Begin band data"))
    return printAndReturnFalse("Begin band data missing");

  // This should be equal in size to the number of orbitals
  bands.reserve(numOrbitals);
  for (size_t i = 0; i < numOrbitals; ++i) {
    bands.append(band());
    // This is how many points we should have in total
    bands[i].reserve(numKPoints * (numSpecialPoints - 1) + 1);
  }
  // This is how many points we should have in total
  kpoints.reserve(numKPoints * (numSpecialPoints - 1) + 1);

  ++ind;
  while (true) {
    // Did we make it to the end without finishing the band data?
    if (ind >= lines.size())
      return printAndReturnFalse("END_BAND_DATA is missing!");

    // Did we make it to the end?
    if (lines[ind].contains("END_BAND_DATA"))
      break;

    // Read the k-point info
    if (!lines[ind].contains("K point") ||
        lines[ind].split(" ").size() < 6) {
      return printAndReturnFalse("K point missing");
    }

    kpoint kp = Vector3(lines[ind].split(" ")[3].toDouble(),
                        lines[ind].split(" ")[4].toDouble(),
                        lines[ind].split(" ")[5].toDouble());
    kpoints.append(kp);

    // Get the orbital energies
    for (size_t j = 0; j < numOrbitals; ++j) {
      ++ind;
      bands[j].append(lines[ind].trimmed().toDouble());
    }
    ++ind;
  }

  // We made it!
  return true;
}
