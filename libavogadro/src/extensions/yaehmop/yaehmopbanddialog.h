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
  // numKPoints will be set to the number of kpoints, and kPointInfo will
  // be set to the string if it succeeds. They will be 0 and empty if
  // the parsing does not succeed.
  // If the user checks the box to display band data, displayBandData
  // will be set to be true
  bool getKPointInfo(size_t& numKPoints, QString& kPointInfo,
                     bool& displayBandData);

  void displayInvalidFormatMessage();

private:
  Ui::YaehmopBandDialog *m_ui;
};

} // namespace Avogadro
#endif // AVOGADRO_YAEHMOPBANDDIALOG_H
