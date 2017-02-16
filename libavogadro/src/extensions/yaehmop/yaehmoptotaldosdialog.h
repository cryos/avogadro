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

#ifndef AVOGADRO_YAEHMOPTOTALDOSDIALOG_H
#define AVOGADRO_YAEHMOPTOTALDOSDIALOG_H

#include <QDialog>

namespace Avogadro {

class YaehmopExtension;

namespace Ui {
class YaehmopTotalDOSDialog;
}

/**
 * @brief The YaehmopTotalDOSDialog class provides a dialog for setting
 * k-point options for plotting band structures with Yaehmop.
 */

class YaehmopTotalDOSDialog : public QDialog
{
  Q_OBJECT
public:
  YaehmopTotalDOSDialog(QWidget *p = 0);
  ~YaehmopTotalDOSDialog();

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
  // @param displayDOSData This will be set to true if we are to
  //                       display the DOS data for the user.
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
  // @return True if the parse was successful and the user did not cancel.
  //         False otherwise.

  bool getNumValAndKPoints(YaehmopExtension* yext,
                           size_t& numValElectrons, size_t& numKPoints,
                           QString& kPoints, bool& displayDOSData,
                           bool& useSmoothing, double& stepE,
                           double& broadening, bool& limitY,
                           double& minY, double& maxY, bool& zeroFermi);

  void displayInvalidFormatMessage();

private:
  Ui::YaehmopTotalDOSDialog *m_ui;
};

} // namespace Avogadro
#endif // AVOGADRO_YAEHMOPTOTALDOSDIALOG_H
