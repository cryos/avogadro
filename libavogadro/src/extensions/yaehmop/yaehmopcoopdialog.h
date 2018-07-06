/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2017 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef AVOGADRO_YAEHMOPCOOPDIALOG_H
#define AVOGADRO_YAEHMOPCOOPDIALOG_H

#include <QDialog>

namespace Avogadro {

class YaehmopExtension;

namespace Ui {
class YaehmopCOOPDialog;
}

/**
 * @brief The YaehmopCOOPDialog class provides a dialog for setting
 * k-point options for plotting band structures with Yaehmop.
 */

class YaehmopCOOPDialog : public QDialog
{
  Q_OBJECT
public:
  YaehmopCOOPDialog(QWidget *p = 0);
  ~YaehmopCOOPDialog();

  // Returns true if it was able to parse the given input successfully and
  // if the user did not cancel. The string should already be ready to be
  // added to the yaehmop input.
  // @param yext The yaehmop extension caller (needed to set a member variable
  //             in it.
  // @param numValElectrons Will be set to the number of valence electrons.
  // @param numKPoints Will be set to the number of kpoints on success. It
  //                   will be zero on failure.
  // @param kpoints Will be set to the input string (x, y, z, weight for each
  //                k points) if it succeeds. It will be empty if the parsing
  //                does not succeed.
  // @param projections The projections information exactly as it is to be
  //                    entered into YAeHMOP.
  // @param displayData This will be set to true if we are to
  //                    display the data for the user.
  // @param useSmoothing This will be set to true if we are to
  //                     use Gaussian smoothing on the data.
  // @param stepE If useSmoothing is true, this will contain
  //              the energy step size to be used for Gaussian smoothing.
  // @param broadening If useSmoothing is true, this will contain
  //                   the broadening to be used for Gaussian smoothing.
  // @param limitY Should we limit the y-range?
  // @param minY MinY if we are limiting the y-range.
  // @param maxY MaxY if we are limiting the y-range.
  // @param zeroFermi Whether or not to zero the Fermi level.
  // @param numDimensions The number of dimensions. If this is 1, then
  //                      periodicity is only along the A vector. If this
  //                      is 2, then periodicity is along both the A and
  //                      the B vectors. If it is 3, then periodicity is
  //                      along all three vectors.
  // @return True if the parse was successful and the user did not cancel.
  //         False otherwise.

  bool getUserOptions(YaehmopExtension* yext,
                      size_t& numValElectrons, size_t& numKPoints,
                      QString& kPoints, QString& projections,
                      bool& displayData,
                      bool& useSmoothing, double& stepE,
                      double& broadening, bool& limitY,
                      double& minY, double& maxY, bool& zeroFermi,
                      unsigned short& numDimensions);

  void displayInvalidKPointsFormatMessage();
  void displayInvalidCOOPSFormatMessage();

public slots:
  // Pops up a dialog box that shows the user the atom numbers
  void viewAtomNumbers();
  // Pops up a dialog box that shows the user the orbital numbers
  void viewOrbitalNumbers();

private:
  Ui::YaehmopCOOPDialog *m_ui;
  YaehmopExtension *m_yext;
};

} // namespace Avogadro
#endif // AVOGADRO_YAEHMOPCOOPDIALOG_H
