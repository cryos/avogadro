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

#include "guessprojections.h"
#include "yaehmopcoopdialog.h"
#include "yaehmopextension.h"
#include "ui_yaehmopcoopdialog.h"

#include <QDebug>

#include <QMessageBox>

namespace Avogadro {

  YaehmopCOOPDialog::YaehmopCOOPDialog(QWidget *p) :
    QDialog(p),
    m_ui(new Ui::YaehmopCOOPDialog),
    m_yext(NULL)
  {
    m_ui->setupUi(this);

    connect(m_ui->push_viewAtomNumbers, SIGNAL(clicked()),
            this, SLOT(viewAtomNumbers()));
    connect(m_ui->push_viewOrbitalNumbers, SIGNAL(clicked()),
            this, SLOT(viewOrbitalNumbers()));
  }

  YaehmopCOOPDialog::~YaehmopCOOPDialog()
  {
    delete m_ui;
  }

  bool YaehmopCOOPDialog::getUserOptions(YaehmopExtension* yext,
                                                 size_t& numValElectrons,
                                                 size_t& numKPoints,
                                                 QString& kPoints,
                                                 QString& coops,
                                                 bool& displayData,
                                                 bool& useSmoothing,
                                                 double& stepE,
                                                 double& broadening,
                                                 bool& limitY,
                                                 double& minY, double& maxY,
                                                 bool& zeroFermi,
                                                 unsigned short& numDimensions)
  {
    m_yext = yext;
    numKPoints = 0;
    m_ui->spin_numValElectrons->setValue(numValElectrons);
    m_ui->edit_kpoints->setText(kPoints);
    m_ui->edit_coops->setText(coops);
    m_ui->cb_displayData->setChecked(displayData);
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
    QString kpointsText = m_ui->edit_kpoints->toPlainText();

    // Let's make sure the input is valid. Split it into lines first
    QStringList lines = kpointsText.split(QRegExp("[\r\n]"),
                                          QString::SkipEmptyParts);

    // Let's go through each line in the k points
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
          displayInvalidKPointsFormatMessage();
          return false;
        }

        // All three strings must be convertible to ints
        bool ok = true;
        size_t grid[3];
        for (size_t j = 0; j < 3; ++j) {
          grid[j] = splitLine[j].toInt(&ok);
          if (!ok) {
            displayInvalidKPointsFormatMessage();
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
          displayInvalidKPointsFormatMessage();
          return false;
        }

        // First, second, and third strings must be convertible to floats
        // Fourth string must be convertible to an int
        bool ok = true;
        for (size_t j = 0; j < 4; ++j) {
          if (j < 3) {
            splitLine[j].toFloat(&ok);
            if (!ok) {
              displayInvalidKPointsFormatMessage();
              return false;
            }
          }
          else {
            splitLine[j].toInt(&ok);
            if (!ok) {
              displayInvalidKPointsFormatMessage();
              return false;
            }
          }
        }

        // We have another valid k point!
        ++numKPointsVal;
      }
    }

    QString coopsText = m_ui->edit_coops->toPlainText();
    QStringList cLines = coopsText.split(QRegExp("[\r\n]"),
                                         QString::SkipEmptyParts);
    QStringList coopList;
    // Now, let's go through each line in the coops
    for (size_t i = 0; i < cLines.size(); ++i) {
      // If it is empty, skip over it
      if (cLines[i].trimmed().size() == 0)
        continue;
      // If it starts with a '#' or ';', it is a comment line
      if (cLines[i].trimmed()[0] == '#' || cLines[i].trimmed()[0] == ';') {
        cLines[i].remove('#');
        cLines[i].remove(';');
        continue;
      }

      // If we made it here, it must be a coop line
      QString type = cLines[i].split(" ", QString::SkipEmptyParts)[0].trimmed();

      // 'type' must be 'atom', 'orbital', or 'FMO', or 'h-*'
      if (type.toLower() != "atom" && type.toLower() != "orbital" &&
          type.toLower() != "fmo" && type.toLower() != "h-atom" &&
          type.toLower() != "h-orbital" && type.toLower() != "h-fmo") {
        qDebug() << "Error: invalid type was entered in edit_coops:"
                 << type << "\nAborting.";
        displayInvalidCOOPSFormatMessage();
        return false;
      }

      // Split all the types by spaces
      QStringList splitLine = cLines[i].split(" ", QString::SkipEmptyParts);

      if (splitLine.size() != 7) {
        qDebug() << "Error: invalid COOP was entered in"
                 << "edit_coops:" << cLines[i] << "\nAborting.";
        displayInvalidCOOPSFormatMessage();
        return false;
      }

      // All items other than the first should be an int
      bool ok;
      for (size_t i = 1; i < 7; ++i) {
        splitLine[i].toInt(&ok);
        if (!ok) {
          qDebug() << "Error: invalid COOP was entered in"
                   << "edit_coops:" << splitLine[i] << "\nAborting.";
          displayInvalidCOOPSFormatMessage();
        }
      }

      coopList.append(cLines[i]);
    }

    // If the coopList is of size 0, return a failure
    if (coopList.size() == 0) {
        qDebug() << "Error: COOPs list is empty!"
                 << "\nAborting.";
        displayInvalidCOOPSFormatMessage();
        return false;
    }

    // The size of the list is the number of COOPs
    coopList.prepend(QString::number(coopList.size()));
    coopList.prepend("coop");

    // If we made it here, we are good!
    numValElectrons = numValElectronsVal;
    numKPoints = numKPointsVal;
    kPoints = lines.join("\n");
    coops = coopList.join("\n");
    displayData = m_ui->cb_displayData->isChecked();
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

    // We have to set this in here so we can keep the "kpointsText"
    yext->setDOSKPoints(kpointsText);

    return true;
  }

  void YaehmopCOOPDialog::viewAtomNumbers()
  {
    if (!m_yext)
      return;

    QDialog* dialog = new QDialog(this);
    QVBoxLayout* layout = new QVBoxLayout;
    dialog->setLayout(layout);
    dialog->setWindowTitle(tr("View Atom Numbers"));
    QTextEdit* edit = new QTextEdit;
    layout->addWidget(edit);
    dialog->resize(500, 500);

    edit->setText(displayAtomNumbers(m_yext->getMolecule()));

    // Make sure this gets deleted upon closing
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
  }

  void YaehmopCOOPDialog::viewOrbitalNumbers()
  {
    if (!m_yext)
      return;

    QDialog* dialog = new QDialog(this);
    QVBoxLayout* layout = new QVBoxLayout;
    dialog->setLayout(layout);
    dialog->setWindowTitle(tr("View Orbital Numbers"));
    QTextEdit* edit = new QTextEdit;
    layout->addWidget(edit);
    dialog->resize(500, 500);

    // Show the user the output
    edit->setText(displayOrbitalNumbers(m_yext->getMolecule()));

    // Make sure this gets deleted upon closing
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
  }

  void YaehmopCOOPDialog::displayInvalidKPointsFormatMessage()
  {
    QMessageBox::critical
      (this,
       tr("Cannot Parse Text"),
       tr("There was an error parsing the k points. "
          "Please check your input and try again.\n\n"));
    reject();
    close();
  }

  void YaehmopCOOPDialog::displayInvalidCOOPSFormatMessage()
  {
    QMessageBox::critical
      (this,
       tr("Cannot Parse Text"),
       tr("There was an error parsing the COOPS. "
          "Please check your input and try again.\n\n"));
    reject();
    close();
  }

} // namespace Avogadro
