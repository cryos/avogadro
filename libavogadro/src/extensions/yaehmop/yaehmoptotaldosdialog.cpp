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

#include "yaehmoptotaldosdialog.h"
#include "yaehmopextension.h"
#include "ui_yaehmoptotaldosdialog.h"

#include <QDebug>

#include <QMessageBox>

namespace Avogadro {

  YaehmopTotalDOSDialog::YaehmopTotalDOSDialog(QWidget *p) :
    QDialog(p),
    m_ui(new Ui::YaehmopTotalDOSDialog)
  {
    m_ui->setupUi(this);
  }

  YaehmopTotalDOSDialog::~YaehmopTotalDOSDialog()
  {
    delete m_ui;
  }

  bool YaehmopTotalDOSDialog::getUserOptions(YaehmopExtension* yext,
                                             size_t& numValElectrons,
                                             size_t& numKPoints,
                                             QString& kPoints,
                                             bool& displayDOSData,
                                             bool& useSmoothing,
                                             double& stepE,
                                             double& broadening,
                                             bool& limitY,
                                             double& minY, double& maxY,
                                             bool& zeroFermi,
                                             unsigned short& numDimensions)
  {
    m_ui->spin_numValElectrons->setValue(numValElectrons);
    numKPoints = 0;
    m_ui->edit_kpoints->setText(kPoints);
    m_ui->cb_displayData->setChecked(displayDOSData);
    m_ui->cb_useSmoothing->setChecked(useSmoothing);
    m_ui->spin_energyStep->setValue(stepE);
    m_ui->spin_broadening->setValue(broadening);
    m_ui->cb_limitY->setChecked(limitY);
    m_ui->spin_minY->setValue(minY);
    m_ui->spin_maxY->setValue(maxY);
    m_ui->cb_zeroFermi->setChecked(zeroFermi);
    m_ui->spin_numDim->setValue(numDimensions);

    if (this->exec() == QDialog::Rejected)
      return false;

    size_t numValElectronsVal = m_ui->spin_numValElectrons->value();
    size_t numKPointsVal = 0;
    QString text = m_ui->edit_kpoints->toPlainText();

    // Let's make sure the input is valid. Split it into lines first
    QStringList lines = text.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

    // Let's go through each line
    for (size_t i = 0; i < lines.size(); ++i) {
      // Split the line by spaces
      QStringList splitLine = lines[i].split(" ", QString::SkipEmptyParts);
      // Skip it if it is of size 0
      if (splitLine.size() == 0)
        continue;

      // Check to see if an 'x' is in the line to indicate a grid.
      if (lines[i].contains("x")) {
        // Split it by the x values instead
        splitLine = lines[i].split("x", QString::SkipEmptyParts);
        // It must be of size 3
        if (splitLine.size() != 3) {
          displayInvalidFormatMessage();
          return false;
        }

        // All three strings must be convertible to ints
        bool ok = true;
        size_t grid[3];
        for (size_t j = 0; j < 3; ++j) {
          grid[j] = splitLine[j].toInt(&ok);
          if (!ok) {
            displayInvalidFormatMessage();
            return false;
          }
        }

        // Now, let's construct the grid
        QStringList newLines;
        for (size_t j = 0; j < grid[0]; ++j) {
          for (size_t k = 0; k < grid[1]; ++k) {
            for (size_t l = 0; l < grid[2]; ++l) {
              QString newLine = QString::number(static_cast<double>(j /
                                                static_cast<double>(grid[0])),
                                                'f') + " " +
                                QString::number(static_cast<double>(k /
                                                static_cast<double>(grid[1])),
                                                'f') + " " +
                                QString::number(static_cast<double>(l /
                                                static_cast<double>(grid[2])),
                                                'f') + " 1";
              newLines.append(newLine);
            }
          }
        }
        // Remove the last new line character
        if (newLines.size() != 0) {
          newLines[newLines.size() - 1] =
            newLines[newLines.size() - 1].trimmed();
        }
        // Set "lines[i]" to be the new set of lines we have made
        lines[i] = newLines.join("\n");
        numKPointsVal += (grid[0] * grid[1] * grid[2]);
      }
      // Otherwise, we have a regular k point
      else {
        // Otherwise, the split line must be of size 4
        if (splitLine.size() != 4) {
          displayInvalidFormatMessage();
          return false;
        }

        // First, second, and third strings must be convertible to floats
        // Fourth string must be convertible to an int
        bool ok = true;
        for (size_t j = 0; j < 4; ++j) {
          if (j < 3) {
            splitLine[j].toFloat(&ok);
            if (!ok) {
              displayInvalidFormatMessage();
              return false;
            }
          }
          else {
            splitLine[j].toInt(&ok);
            if (!ok) {
              displayInvalidFormatMessage();
              return false;
            }
          }
        }

        // We have another valid k point!
        ++numKPointsVal;
      }
    }

    // If we made it here, we are good!
    numValElectrons = numValElectronsVal;
    numKPoints = numKPointsVal;
    kPoints = lines.join("\n");
    displayDOSData = m_ui->cb_displayData->isChecked();
    useSmoothing = m_ui->cb_useSmoothing->isChecked();
    if (useSmoothing) {
      stepE = m_ui->spin_energyStep->value();
      broadening = m_ui->spin_broadening->value();
    }
    else {
      stepE = 0.0;
      broadening = 0.0;
    }
    limitY = m_ui->cb_limitY->isChecked();
    minY = m_ui->spin_minY->value();
    maxY = m_ui->spin_maxY->value();
    zeroFermi = m_ui->cb_zeroFermi->isChecked();
    numDimensions = m_ui->spin_numDim->value();

    // We have to set this in here so we can keep the "text"
    yext->setDOSKPoints(text);

    return true;
  }

  void YaehmopTotalDOSDialog::displayInvalidFormatMessage()
  {
    QMessageBox::critical
      (this,
       tr("Cannot Parse Text"),
       tr("There was an error parsing the k points. "
          "Please check your input and try again.\n\n"));
    reject();
    close();
  }

} // namespace Avogadro
