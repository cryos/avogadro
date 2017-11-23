/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2016 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef AVOGADRO_YAEHMOPBANDDIALOG_H
#define AVOGADRO_YAEHMOPBANDDIALOG_H

#include <avogadro/molecule.h>

#include <QDialog>

namespace Avogadro {

namespace Ui {
class YaehmopBandDialog;
}

/**
 * @brief The YaehmopBandDialog class provides a dialog for setting
 * k-point options for plotting band structures with Yaehmop.
 */

class YaehmopBandDialog : public QDialog
{
  Q_OBJECT
public:
  YaehmopBandDialog(QWidget *p = 0);
  ~YaehmopBandDialog();

  // Returns true if it was able to parse the given input successfully and
  // if the user did not cancel. The string should already be ready to be
  // added to the yaehmop input.
  // @param mol is the molecule - used for determining special kpoints.
  // @param numKPoints will be set to the number of kpoints, and
  // @param kPointInfo will be set to the string if it succeeds. They will be
  //                   0 and empty if the parsing does not succeed.
  // @param limitY Should we limit the y-range?
  // @param minY MinY if we are limiting the y-range.
  // @param maxY MaxY if we are limiting the y-range.
  // @param plotFermi Whether or not to plot the Fermi level. It must be known
  //                  from a density of states calculation.
  // @param fermi The Fermi level. The user should know this from a density
  //              of states calculation. This only takes effect if plotFermi is
  //              true.
  // @param zeroFermi Whether or not to zero the Fermi level. This will only
  //                  take effect if plotFermi is true.
  // @param numDimensions The number of dimensions. If this is 1, then
  //                      periodicity is only along the A vector. If this
  //                      is 2, then periodicity is along both the A and
  //                      the B vectors. If it is 3, then periodicity is
  //                      along all three vectors.
  // If the user checks the box to display band data, displayBandData
  // will be set to be true
  bool getUserOptions(Molecule* mol, size_t& numKPoints,
                      QString& kPointInfo, bool& displayBandData,
                      bool& limitY, double& minY, double& maxY, bool& plotFermi,
                      double& fermi, bool& zeroFermi,
                      unsigned short& numDimensions);

  void displayInvalidFormatMessage();

private:
  Ui::YaehmopBandDialog *m_ui;
};

} // namespace Avogadro
#endif // AVOGADRO_YAEHMOPBANDDIALOG_H
