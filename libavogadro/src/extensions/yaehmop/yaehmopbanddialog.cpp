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

#include "specialkpoints.h"
#include "yaehmopbanddialog.h"
#include "ui_yaehmopbanddialog.h"

#include <QDebug>

#include <QMessageBox>

namespace Avogadro {

  YaehmopBandDialog::YaehmopBandDialog(QWidget *p) :
    QDialog(p),
    m_ui(new Ui::YaehmopBandDialog)
  {
    m_ui->setupUi(this);
  }

  YaehmopBandDialog::~YaehmopBandDialog()
  {
    delete m_ui;
  }

  bool YaehmopBandDialog::getKPointInfo(Molecule* mol, size_t& numKPoints,
                                        QString& kPointInfo,
                                        bool& displayBandData, bool& limitY,
                                        double& minY, double& maxY,
                                        bool& plotFermi, double& fermi,
                                        bool& zeroFermi)
  {
    m_ui->spin_numKPoints->setValue(numKPoints);
    m_ui->cb_displayBandData->setChecked(displayBandData);
    m_ui->cb_limitY->setChecked(limitY);
    m_ui->spin_minY->setValue(minY);
    m_ui->spin_maxY->setValue(maxY);
    m_ui->cb_plotFermi->setChecked(plotFermi);
    m_ui->spin_fermi->setValue(fermi);
    m_ui->cb_zeroFermi->setChecked(zeroFermi);

    kPointInfo = "";
    QString specialKPoints = SpecialKPoints::getSpecialKPoints(mol);
    if (!specialKPoints.isEmpty())
      m_ui->edit_specialKPoints->setText(specialKPoints);

    if (this->exec() == QDialog::Rejected)
      return false;

    size_t numKPointsVal = m_ui->spin_numKPoints->value();
    QString text = m_ui->edit_specialKPoints->toPlainText();

    // Let's make sure the input is valid. Split it into lines first
    QStringList lines = text.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

    // Let's go through each line
    for (size_t i = 0; i < lines.size(); ++i) {
      // Split the line by spaces
      QStringList splitLine = lines[i].split(" ", QString::SkipEmptyParts);
      // Skip it if it is of size 0
      if (splitLine.size() == 0)
        continue;
      // Otherwise, it must be of size 4
      if (splitLine.size() != 4) {
        displayInvalidFormatMessage();
        return false;
      }

      // First string can be anything. We'll ignore it
      // Second, third, and fourth strings must be convertable to floats
      bool ok = true;
      for (size_t j = 1; j < 4; ++j) {
        splitLine[j].toFloat(&ok);
        if (!ok) {
          displayInvalidFormatMessage();
          return false;
        }
      }
    }

    // If we made it here, we are good!
    numKPoints = numKPointsVal;
    kPointInfo = text;
    displayBandData = m_ui->cb_displayBandData->isChecked();
    limitY = m_ui->cb_limitY->isChecked();
    minY = m_ui->spin_minY->value();
    maxY = m_ui->spin_maxY->value();
    plotFermi = m_ui->cb_plotFermi->isChecked();
    fermi = m_ui->spin_fermi->value();
    zeroFermi = (plotFermi ? m_ui->cb_zeroFermi->isChecked() : false);
    return true;
  }

  void YaehmopBandDialog::displayInvalidFormatMessage()
  {
    QMessageBox::critical
      (this,
       tr("Cannot Parse Text"),
       tr("There was an error parsing the special k-points. "
          "Please check your input and try again.\n\n"));
    reject();
    close();
  }

} // namespace Avogadro
