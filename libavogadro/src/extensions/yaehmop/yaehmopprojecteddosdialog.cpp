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

#include "guessprojections.h"
#include "yaehmopprojecteddosdialog.h"
#include "yaehmopextension.h"
#include "ui_yaehmopprojecteddosdialog.h"

#include <QDebug>

#include <QMessageBox>

namespace Avogadro {

  YaehmopProjectedDOSDialog::YaehmopProjectedDOSDialog(QWidget *p) :
    QDialog(p),
    m_ui(new Ui::YaehmopProjectedDOSDialog),
    m_yext(NULL)
  {
    m_ui->setupUi(this);

    connect(m_ui->push_displayAtomProj, SIGNAL(clicked()),
            this, SLOT(displayAtomProjections()));
    connect(m_ui->push_displayOrbitalProj, SIGNAL(clicked()),
            this, SLOT(displayOrbitalProjections()));
    connect(m_ui->push_displayDetailedOrbitalProj, SIGNAL(clicked()),
            this, SLOT(displayDetailedOrbitalProjections()));
  }

  YaehmopProjectedDOSDialog::~YaehmopProjectedDOSDialog()
  {
    delete m_ui;
  }

  bool YaehmopProjectedDOSDialog::getUserOptions(YaehmopExtension* yext,
                                                 size_t& numValElectrons,
                                                 size_t& numKPoints,
                                                 bool& integratePDOS,
                                                 QString& kPoints,
                                                 QStringList& titles,
                                                 QString& projections,
                                                 bool& displayTotalDOS,
                                                 bool& displayDOSData,
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
    titles.clear();
    m_ui->spin_numValElectrons->setValue(numValElectrons);
    m_ui->edit_kpoints->setText(kPoints);
    m_ui->cb_integratePDOS->setChecked(integratePDOS);
    m_ui->cb_displayTotalDOS->setChecked(displayTotalDOS);
    m_ui->cb_displayData->setChecked(displayDOSData);
    m_ui->cb_useSmoothing->setChecked(useSmoothing);
    m_ui->spin_energyStep->setValue(stepE);
    m_ui->spin_broadening->setValue(broadening);
    m_ui->cb_limitY->setChecked(limitY);
    m_ui->spin_minY->setValue(minY);
    m_ui->spin_maxY->setValue(maxY);
    m_ui->cb_zeroFermi->setChecked(zeroFermi);
    m_ui->spin_numDim->setValue(numDimensions);

    displayAtomProjections();

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

    QString projectionsText = m_ui->edit_projections->toPlainText();
    QStringList pLines = projectionsText.split(QRegExp("[\r\n]"),
                                               QString::SkipEmptyParts);
    QStringList tmpTitles, projList;
    // Store whether or not we have a title for every projection
    bool titleFound = false;
    // Now, let's go through each line in the projections
    for (size_t i = 0; i < pLines.size(); ++i) {
      // If it is empty, skip over it
      if (pLines[i].trimmed().size() == 0)
        continue;
      // If it starts with a '#' or ';', store it as the title
      if (pLines[i].trimmed()[0] == '#' || pLines[i].trimmed()[0] == ';') {
        pLines[i].remove('#');
        pLines[i].remove(';');
        // If we already found a title, print a warning and just use the second
        // title.
        if (titleFound && tmpTitles.size() != 0) {
          qDebug() << "Warning: two titles were found in a row.";
          qDebug() << tmpTitles[tmpTitles.size() - 1]
                   << " will be replaced with "
                   << pLines[i].trimmed();
          tmpTitles[tmpTitles.size() - 1] = pLines[i].trimmed();
        }
        else {
          tmpTitles.append(pLines[i].trimmed());
          titleFound = true;
        }
        continue;
      }

      // If we made it here, it must be a projection line
      QString type = pLines[i].split(" ", QString::SkipEmptyParts)[0].trimmed();

      // 'type' must be 'atom', 'orbital', or 'FMO'
      if (type.toLower() != "atom" && type.toLower() != "orbital" &&
          type.toLower() != "fmo") {
        qDebug() << "Error: invalid type was entered in edit_projections:"
                 << type << "\nAborting.";
        displayInvalidProjectionsFormatMessage();
        return false;
      }

      // Split all the types by commas
      QStringList splitLine = pLines[i].split(",", QString::SkipEmptyParts);

      // Remove the type from the first item
      splitLine[0].remove(type);

      // The split items should now be: contributor (int) weight (double)
      for (size_t j = 0; j < splitLine.size(); ++j) {
        QStringList spaceSplit = splitLine[j].split(" ",
                                                    QString::SkipEmptyParts);
        // This should be of size 2, no more, no less
        if (spaceSplit.size() != 2) {
          qDebug() << "Error: invalid projection was entered in"
                   << "edit_projections:" << pLines[i] << "\nAborting.";
          displayInvalidProjectionsFormatMessage();
          return false;
        }

        // The first item should be an int, the second item should be a double
        bool ok;
        spaceSplit[0].toInt(&ok);
        if (!ok) {
          qDebug() << "Error: invalid projection was entered in"
                   << "edit_projections:" << splitLine[i] << "\nAborting.";
          displayInvalidProjectionsFormatMessage();
        }
        spaceSplit[1].toFloat(&ok);
        if (!ok) {
          qDebug() << "Error: invalid projection was entered in"
                   << "edit_projections:" << splitLine[i] << "\nAborting.";
          displayInvalidProjectionsFormatMessage();
        }
      }

      // If we didn't find a title, append an empty space
      if (!titleFound)
        tmpTitles.append(" ");

      titleFound = false;
      projList.append(pLines[i]);
    }

    // If the projList is of size 0, return a failure
    if (projList.size() == 0) {
        qDebug() << "Error: projections list is empty!"
                 << "\nAborting.";
        displayInvalidProjectionsFormatMessage();
        return false;
    }

    // The size of the list is the number of projections
    projList.prepend(QString::number(projList.size()));
    projList.prepend("projections");

    // If we made it here, we are good!
    numValElectrons = numValElectronsVal;
    numKPoints = numKPointsVal;
    kPoints = lines.join("\n");
    titles = tmpTitles;
    projections = projList.join("\n");
    integratePDOS = m_ui->cb_integratePDOS->isEnabled() &&
                    m_ui->cb_integratePDOS->isChecked();
    displayTotalDOS = m_ui->cb_displayTotalDOS->isChecked();
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

    // We have to set this in here so we can keep the "kpointsText"
    m_yext->setDOSKPoints(kpointsText);

    return true;
  }

  void YaehmopProjectedDOSDialog::displayAtomProjections()
  {
    if (!m_yext)
      return;
    m_ui->edit_projections->setText(
      guessTypedAtomProjections(m_yext->getMolecule())
    );
  }

  void YaehmopProjectedDOSDialog::displayOrbitalProjections()
  {
    if (!m_yext)
      return;
    m_ui->edit_projections->setText(
      guessOrbitalProjections(m_yext->getMolecule())
    );
  }

  void YaehmopProjectedDOSDialog::displayDetailedOrbitalProjections()
  {
    if (!m_yext)
      return;
    m_ui->edit_projections->setText(
      guessDetailedOrbitalProjections(m_yext->getMolecule())
    );
  }

  void YaehmopProjectedDOSDialog::displayInvalidKPointsFormatMessage()
  {
    QMessageBox::critical
      (this,
       tr("Cannot Parse Text"),
       tr("There was an error parsing the k points. "
          "Please check your input and try again.\n\n"));
    reject();
    close();
  }

  void YaehmopProjectedDOSDialog::displayInvalidProjectionsFormatMessage()
  {
    QMessageBox::critical
      (this,
       tr("Cannot Parse Text"),
       tr("There was an error parsing the projections. "
          "Please check your input and try again.\n\n"));
    reject();
    close();
  }

} // namespace Avogadro
