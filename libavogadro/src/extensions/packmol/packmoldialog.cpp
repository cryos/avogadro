/**********************************************************************
  PackmolDialog - Dialog for generating cubes and meshes

  Copyright (C) 2010 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "packmoldialog.h"
#include "highlighter.h"
#include "structuresmodel.h"

#include <Eigen/Core>

#include <avogadro/atom.h>
#include <avogadro/molecule.h>
#include <avogadro/moleculefile.h>

#include <openbabel/mol.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QCheckBox>
#include <QDesktopServices>
#include <QUrl>
#include <QSharedPointer>
#include <QDebug>

namespace Avogadro {

  double calcNumberOfMolecules(double mw, double density, double volume)
  {
    // mass [g/mol]
    // density [g/ml]
    // volume [A^3]
    // 1 ml = 1 cm^3 = 10^24A^3
    // 1 A^3 = 10^-24cm^3

    double mass = volume * density;
    double moles = mass / mw;
    double number = moles * 6.022 * 10e-2;
    return number;
  }

  double calcNumberOfMolecules(const QString &fileName, double density, double volume)
  {
    QSharedPointer<Molecule> molecule(MoleculeFile::readMolecule(fileName));
    if (!molecule)
      return 0.0;
    return calcNumberOfMolecules(molecule->OBMol().GetMolWt(), density, volume);
  }

  double calcVolumeOfMolecules(double mw, double density, double number)
  {
    double moles = number / (6.022 * 10e+23);
    double mass = moles * mw;
    double volume = mass / density;
    return volume * 10e+24;
  }









  PackmolDialog::PackmolDialog(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_process(0)
  {
    ui.setupUi(this);
    new Highlighter(ui.textEdit->document());

    m_model = new StructuresModel;
    m_model->addDefaultStructures();
    ui.bilayerTableView->setModel(m_model);
    ui.bilayerTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.bilayerTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.bilayerTableView->setItemDelegateForColumn(0, new FileDelegate);
    ui.bilayerTableView->setItemDelegateForColumn(1, new ComboBoxDelegate);
    ui.bilayerTableView->setItemDelegateForColumn(2, new SpinBoxDelegate);
    ui.bilayerTableView->setItemDelegateForColumn(3, new DensityDelegate);
    ui.bilayerTableView->setColumnWidth(0, 250);
    ui.bilayerTableView->setColumnWidth(1, 200);
    ui.bilayerTableView->setColumnWidth(2, 100);

    // Connect up some signals and slots
    connect(ui.solvSoluteBrowse, SIGNAL(clicked()), this, SLOT(solvSoluteBrowseClicked()));
    connect(ui.solvSolventBrowse, SIGNAL(clicked()), this, SLOT(solvSolventBrowseClicked()));
    connect(ui.solvGenerate, SIGNAL(clicked()), this, SLOT(solvGenerateClicked()));
    connect(ui.solvAdjustShape, SIGNAL(stateChanged(int)), this, SLOT(solvAdjustShapeClicked(int)));
    connect(ui.solvAddCounterIons, SIGNAL(stateChanged(int)), this, SLOT(solvAddCounterIonsClicked(int)));
    connect(ui.solvGuessSolventNumber, SIGNAL(stateChanged(int)), this, SLOT(solvGuessSolventNumberClicked(int)));
    connect(ui.solvSpacing, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));
    connect(ui.solvMinX, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));
    connect(ui.solvMinY, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));
    connect(ui.solvMinZ, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));
    connect(ui.solvMaxX, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));
    connect(ui.solvMaxY, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));
    connect(ui.solvMaxZ, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));
    connect(ui.solvCenterX, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));
    connect(ui.solvCenterY, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));
    connect(ui.solvCenterZ, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));
    connect(ui.solvRadius, SIGNAL(valueChanged(double)), this, SLOT(solvVolumeChanged(double)));

    connect(ui.bilayerGenerate, SIGNAL(clicked()), this, SLOT(bilayerGenerateClicked()));
    connect(ui.bilayerGuessNumber, SIGNAL(clicked()), this, SLOT(bilayerUpdateNumber()));
    connect(ui.bilayerNew, SIGNAL(clicked()), this, SLOT(bilayerNewClicked()));
    connect(ui.bilayerRemove, SIGNAL(clicked()), this, SLOT(bilayerRemoveClicked()));

    connect(ui.runButton, SIGNAL(clicked()), this, SLOT(runButtonClicked()));
    connect(ui.abortButton, SIGNAL(clicked()), this, SLOT(abortButtonClicked()));
    connect(ui.visitWebsite, SIGNAL(clicked()), this, SLOT(visitWebsite()));
  }

  PackmolDialog::~PackmolDialog()
  {
  }

  void PackmolDialog::solvSoluteBrowseClicked()
  {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Molecule"));
    ui.solvSoluteFilename->setText(fileName);
    solvUpdateVolume();

    // keep track of files
    QFileInfo fileInfo(fileName);
    QString fileNameInInputFile = fileInfo.baseName() + "." + ui.filetype->currentText();
    m_fileLookup[fileNameInInputFile] = fileName;
  }

  void PackmolDialog::solvSolventBrowseClicked()
  {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Molecule"));
    ui.solvSolventFilename->setText(fileName);

    // keep track of files
    QFileInfo fileInfo(fileName);
    QString fileNameInInputFile = fileInfo.baseName() + "." + ui.filetype->currentText();
    m_fileLookup[fileNameInInputFile] = fileName;
  }

  void PackmolDialog::solvAdjustShapeClicked(int state)
  {
    switch (state) {
      default:
      case Qt::Unchecked:
        ui.solvMinX->setEnabled(true);
        ui.solvMinY->setEnabled(true);
        ui.solvMinZ->setEnabled(true);
        ui.solvMaxX->setEnabled(true);
        ui.solvMaxY->setEnabled(true);
        ui.solvMaxZ->setEnabled(true);
        ui.solvCenterX->setEnabled(true);
        ui.solvCenterY->setEnabled(true);
        ui.solvCenterZ->setEnabled(true);
        ui.solvRadius->setEnabled(true);
        ui.solvSpacing->setEnabled(false);
        break;
      case Qt::Checked:
        ui.solvMinX->setEnabled(false);
        ui.solvMinY->setEnabled(false);
        ui.solvMinZ->setEnabled(false);
        ui.solvMaxX->setEnabled(false);
        ui.solvMaxY->setEnabled(false);
        ui.solvMaxZ->setEnabled(false);
        ui.solvCenterX->setEnabled(false);
        ui.solvCenterY->setEnabled(false);
        ui.solvCenterZ->setEnabled(false);
        ui.solvRadius->setEnabled(false);
        ui.solvSpacing->setEnabled(true);
        break;
    }

    solvUpdateVolume();
  }

  void PackmolDialog::solvUpdateVolume()
  {
    if (!ui.solvAdjustShape->isChecked())
      return;

    QString soluteFilename = ui.solvSoluteFilename->text();
    if (soluteFilename.length()) {
      QSharedPointer<Molecule> molecule(MoleculeFile::readMolecule(soluteFilename));
      if (molecule) {
        double spacing = ui.solvSpacing->value();

        // Box
        double minX, minY, minZ;
        double maxX, maxY, maxZ;
        minX = minY = minZ = 1000.0;
        maxX = maxY = maxZ = -1000.0;

        Eigen::Vector3d center(Eigen::Vector3d::Zero());
        foreach(Atom *atom, molecule->atoms()) {
          Eigen::Vector3d pos = *(atom->pos());
          center += pos;
          if (pos.x() < minX) minX = pos.x();
          if (pos.y() < minY) minY = pos.y();
          if (pos.z() < minZ) minZ = pos.z();
          if (pos.x() > maxX) maxX = pos.x();
          if (pos.y() > maxY) maxY = pos.y();
          if (pos.z() > maxZ) maxZ = pos.z();
        }
        center /= molecule->numAtoms();

        ui.solvMinX->setValue(minX - spacing);
        ui.solvMinY->setValue(minY - spacing);
        ui.solvMinZ->setValue(minZ - spacing);
        ui.solvMaxX->setValue(maxX + spacing);
        ui.solvMaxY->setValue(maxY + spacing);
        ui.solvMaxZ->setValue(maxZ + spacing);

        // Sphere
        double maxR = 0.0;
        foreach(Atom *atom, molecule->atoms()) {
          double r = (center - *(atom->pos())).norm();
          if (r > maxR)
            maxR = r;
        }

        ui.solvCenterX->setValue(center.x());
        ui.solvCenterY->setValue(center.y());
        ui.solvCenterZ->setValue(center.z());
        ui.solvRadius->setValue(maxR + spacing);
      }
    }
  }

  double PackmolDialog::solvCalcVolume()
  {
    if (ui.solvShape->currentIndex() == 0) {
      // Box
      double dx = ui.solvMaxX->value() - ui.solvMinX->value();
      double dy = ui.solvMaxY->value() - ui.solvMinY->value();
      double dz = ui.solvMaxZ->value() - ui.solvMinZ->value();
      return dx * dy * dz;
    } else {
      // Sphere
      double r = ui.solvRadius->value();
      return 4.0 / 3.0 * M_PI * r * r * r;
    }
  }

  void PackmolDialog::solvVolumeChanged(double value)
  {
    solvUpdateVolume();
    solvUpdateSoluteNumber();
  }

  void PackmolDialog::solvUpdateSoluteNumber()
  {
    if (!ui.solvGuessSolventNumber->isChecked())
      return;

    double volume = solvCalcVolume();
    int number = calcNumberOfMolecules(ui.solvSolventFilename->text(), ui.solvDensity->value(), volume);

    ui.solvSolventNumber->setValue(number);
  }

  void PackmolDialog::solvAddCounterIonsClicked(int state)
  {
  }

  void PackmolDialog::solvGuessSolventNumberClicked(int state)
  {
    switch (state) {
      default:
      case Qt::Unchecked:
        ui.solvSolventNumber->setEnabled(true);
        ui.solvDensity->setEnabled(false);
        break;
      case Qt::Checked:
        ui.solvSolventNumber->setEnabled(false);
        ui.solvDensity->setEnabled(true);
        break;
    }

    solvUpdateSoluteNumber();
  }

  QString PackmolDialog::solvContraintString()
  {
    QString text;
    if (ui.solvShape->currentIndex() == 0) {
      // Box
      text += "inside box " + QString::number(ui.solvMinX->value(), 'f', 1) + " "
                            + QString::number(ui.solvMinY->value(), 'f', 1) + " "
                            + QString::number(ui.solvMinZ->value(), 'f', 1) + " "
                            + QString::number(ui.solvMaxX->value(), 'f', 1) + " "
                            + QString::number(ui.solvMaxY->value(), 'f', 1) + " "
                            + QString::number(ui.solvMaxZ->value(), 'f', 1);
    } else {
      // Sphere
      text += "inside sphere " + QString::number(ui.solvCenterX->value(), 'f', 1) + " "
                               + QString::number(ui.solvCenterY->value(), 'f', 1) + " "
                               + QString::number(ui.solvCenterZ->value(), 'f', 1) + " "
                               + QString::number(ui.solvRadius->value(), 'f', 1);
    }
    return text;
  }

  QString PackmolDialog::headerString()
  {
    QString filetype = ui.filetype->currentText();
    QString text;
    text += "tolerance " + QString::number(ui.tolerance->value(), 'f', 1) + "\n";
    text += "filetype " +  filetype + "\n";
    text += "output " + ui.output->text() + "\n";
    if (ui.addBoxSides->isChecked())
      text += "add_box_sides\n";
    if (ui.addAmberTer->isChecked())
      text += "add_amber_ter\n";
    text += "\n";
    return text;
  }

  void PackmolDialog::solvGenerateClicked()
  {
    if (ui.solvSolventFilename->text().length() == 0) {
      QMessageBox::information(this, tr("No solvent"), tr("No solvent filename specified."));
      return;
    }
    if (ui.solvSoluteFilename->text().length() == 0) {
      QMessageBox::StandardButton result = QMessageBox::question(this, tr("No solute"),
          tr("No solute filename specified. Continue?"), QMessageBox::Yes | QMessageBox::No);
      if (result == QMessageBox::No)
        return;
    }

    ui.tabWidget->setCurrentIndex(1); // change to text mode

    QString filetype = ui.filetype->currentText();
    QString contraint = solvContraintString();
    QString text;
    text += headerString();
    if (ui.solvSoluteFilename->text().length() > 0) {
      // solute
      text += "# solute\n";
      QFileInfo soluteFileInfo(ui.solvSoluteFilename->text());
      text += "structure " + soluteFileInfo.baseName() + "." + filetype + "\n";
      text += "  number " + QString::number(ui.solvSoluteNumber->value()) + "\n";
      if (ui.solvSoluteNumber->value() == 1)
        text += "  fixed 0. 0. 0. 0. 0. 0.\n";
      else
        text += "  " + contraint + "\n";
      text += "end structure\n";
      text += "\n";

      if (ui.solvAddCounterIons->isChecked()) {
        // compute solute charge
        int soluteCharge = 0.0;
        QSharedPointer<Molecule> molecule(MoleculeFile::readMolecule(ui.solvSoluteFilename->text()));
        if (molecule) {
          foreach (Atom *atom, molecule->atoms())
            soluteCharge += atom->formalCharge();
        }
        soluteCharge *= ui.solvSoluteNumber->value();

        // coutner ions
        if (soluteCharge) {
          text += "# counter ions\n";
          if (soluteCharge < 0) {
            // add Na ions...
            text += "structure sodium." + filetype + "\n";
            text += "  number " + QString::number(-soluteCharge) + "\n";
            text += "  " + contraint + "\n";
            text += "end structure\n";
          } else {
            // Add Cl ions...
            text += "structure chlorine." + filetype + "\n";
            text += "  number " + QString::number(soluteCharge) + "\n";
            text += "  " + contraint + "\n";
            text += "end structure\n";
          }
          text += "\n";
        }
      }
    }

    // solvent
    text += "# solvent\n";
    QFileInfo solventFileInfo(ui.solvSolventFilename->text());
    text += "structure " + solventFileInfo.baseName() + "." + filetype + "\n";
    text += "  number " + QString::number(ui.solvSolventNumber->value()) + "\n";
    text += "  " + contraint + "\n";
    text += "end structure\n";
    text += "\n";

    ui.textEdit->setText(text);
  }

  void PackmolDialog::bilayerUpdateNumber()
  {
    double L = bilayerCalculateL();
    if (!L)
      return;

    QList<Structure> structures;
    foreach (Structure structure, m_model->structures()) {
      double volume;
      if (structure.type == Structure::Lipid) {
        volume = ui.bilayerDimX->value() * ui.bilayerDimY->value() * (L + 1.0);
      } else
      if (structure.type == Structure::PolarSolvent) {
        double thickness = 0.5 * (ui.bilayerDimZ->value() - 2.0 * L) + 3.0;
        volume = ui.bilayerDimX->value() * ui.bilayerDimY->value() * thickness;
      }
      structure.number = calcNumberOfMolecules(structure.fileName, structure.density, volume);
      structures.append(structure);
    }

    m_model->setStructures(structures);
  }

  double PackmolDialog::bilayerCalculateL()
  {
    // make sure we have at least one polar solvent + lipid
    bool foundLipid = false;
    bool foundPolarSolvent = false;
    double L = 0.0;
    foreach (const Structure &structure, m_model->structures()) {
      if (structure.type == Structure::Lipid) {
        foundLipid = true;
        QFileInfo fileInfo(structure.fileName);
        m_fileLookup[fileInfo.fileName()] = structure.fileName;
        // find the longest lipid
        QSharedPointer<Molecule> molecule(MoleculeFile::readMolecule(structure.fileName));
        if (molecule) {
          foreach (Atom *a, molecule->atoms())
            foreach (Atom *b, molecule->atoms()) {
              double dist = (*(a->pos()) - *(b->pos())).norm();
              if (dist > L)
                L = dist;
            }
        }
      }
      if (structure.type == Structure::PolarSolvent)
        foundPolarSolvent = true;
    }

    qDebug() << "longest lipid = " << L;

    if (!foundLipid) {
      QMessageBox::information(this, tr("No Lipid"), tr("There must be at least one lipid."));
      return 0.0;
    }

    if (!foundPolarSolvent) {
      QMessageBox::information(this, tr("No Polar Solvent"), tr("There must be at least one polar solvent."));
      return 0.0;
    }

    return L;
  }

  void PackmolDialog::bilayerNewClicked()
  {
    m_model->addStructure();
  }

  void PackmolDialog::bilayerRemoveClicked()
  {
    QModelIndex index = ui.bilayerTableView->currentIndex();
    if (!index.isValid())
      return;
    int row = index.row();
    m_model->removeStructure(row);
  }

  void PackmolDialog::bilayerGenerateClicked()
  {
    QString filetype = ui.filetype->currentText();
    QString text;
    text += headerString();

    double L = bilayerCalculateL();

    ui.tabWidget->setCurrentIndex(1); // change to text mode

    // lipid polar head with polar solvent overlap
    double overlap = 3.0;
    foreach (const Structure &structure, m_model->structures()) {
      QFileInfo fileInfo(structure.fileName);
      // zShift: make sure center of bilayer is at coordinates origin...
      double zShift = - ui.bilayerDimZ->value() / 2.0;
      double xMin = - ui.bilayerDimX->value() / 2.0;
      double xMax = ui.bilayerDimX->value() / 2.0;
      double yMin = - ui.bilayerDimY->value() / 2.0;
      double yMax = ui.bilayerDimY->value() / 2.0;
      if (structure.type == Structure::Lipid) {
        double thickness = 0.5 * (ui.bilayerDimZ->value() - 2.0 * L);
        text += "structure " + fileInfo.baseName() + "." + filetype + "\n";
        text += "  number " + QString::number(structure.number) + "\n";
        text += "  inside box " + QString::number(xMin, 'f', 1) + " "
                                + QString::number(yMin, 'f', 1) + " "
                                + QString::number(thickness + zShift, 'f', 1) + " "
                                + QString::number(xMax, 'f', 1) + " "
                                + QString::number(yMax, 'f', 1) + " "
                                + QString::number(thickness + L + 1.0 + zShift, 'f', 1) + "\n";
        text += "  atoms # list polar head atoms here\n";
        text += "    below plane 0.0 0.0 1.0 " + QString::number(thickness + overlap + 2.0 + zShift, 'f', 1) + "\n";
        text += "  end atoms\n";
        text += "  atoms # list lipophilic tail atoms here\n";
        text += "    over plane 0.0 0.0 1.0 " + QString::number(thickness + overlap + L - 3.0 + zShift, 'f', 1) + "\n";
        text += "  end atoms\n";
        text += "end structure\n";
        text += "\n";
        text += "structure " + fileInfo.baseName() + "." + filetype + "\n";
        text += "  number " + QString::number(structure.number) + "\n";
        text += "  inside box " + QString::number(xMin, 'f', 1) + " "
                                + QString::number(yMin, 'f', 1) + " "
                                + QString::number(ui.bilayerDimZ->value() - thickness - L - 1.0 + zShift, 'f', 1) + " "
                                + QString::number(xMax, 'f', 1) + " "
                                + QString::number(yMax, 'f', 1) + " "
                                + QString::number(ui.bilayerDimZ->value() - thickness + zShift, 'f', 1) + "\n";
        text += "  atoms # list polar head atoms here\n";
        text += "    over plane 0.0 0.0 1.0 " + QString::number(ui.bilayerDimZ->value() -
            thickness - overlap - 2.0 + zShift, 'f', 1) + "\n";
        text += "  end atoms\n";
        text += "  atoms # list lipophilic tail atoms here\n";
        text += "    below plane 0.0 0.0 1.0 " + QString::number(ui.bilayerDimZ->value() -
            thickness - overlap - L + 3.0 + zShift, 'f', 1) + "\n";
        text += "  end atoms\n";
        text += "end structure\n";
        text += "\n";

      } else
      if (structure.type == Structure::PolarSolvent) {
        double thickness = 0.5 * (ui.bilayerDimZ->value() - 2.0 * L) + 3.0;
        text += "structure " + fileInfo.baseName() + "." + filetype + "\n";
        text += "  number " + QString::number(structure.number) + "\n";
        text += "  inside box " + QString::number(xMin, 'f', 1) + " "
                                + QString::number(yMin, 'f', 1) + " "
                                + QString::number(zShift, 'f', 1) + " "
                                + QString::number(xMax, 'f', 1) + " "
                                + QString::number(yMax, 'f', 1) + " "
                                + QString::number(thickness + zShift, 'f', 1) + "\n";
        text += "end structure\n";
        text += "\n";
        text += "structure " + fileInfo.baseName() + "." + filetype + "\n";
        text += "  number " + QString::number(structure.number) + "\n";
        text += "  inside box " + QString::number(xMin, 'f', 1) + " "
                                + QString::number(yMin, 'f', 1) + " "
                                + QString::number(ui.bilayerDimZ->value() - thickness + zShift, 'f', 1) + " "
                                + QString::number(xMax, 'f', 1) + " "
                                + QString::number(yMax, 'f', 1) + " "
                                + QString::number(ui.bilayerDimZ->value() + zShift, 'f', 1) + "\n";
        text += "end structure\n";
        text += "\n";
      }
    }

    ui.textEdit->setText(text);
  }

  void PackmolDialog::createSodiumFile()
  {
    bool formatIsPdb = (ui.filetype->currentText() == "pdb") ? true : false;
    QString tmpdir = QDesktopServices::storageLocation(QDesktopServices::TempLocation);
    QString fileName = tmpdir + QDir::separator() + "sodium." + ui.filetype->currentText();
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    QTextStream stream(&file);
    if (formatIsPdb) {
      stream << "HETATM    1 NA   LIG     1       0.000   0.000   0.000  1.00  0.00          Na  ";
    } else {
      // xyz
      stream << "1\n";
      stream << "sodium.xyz\n";
      stream << "Na         0.00000        0.00000        0.00000";
    }
    file.close();
  }

  void PackmolDialog::createChlorineFile()
  {
    bool formatIsPdb = (ui.filetype->currentText() == "pdb") ? true : false;
    QString tmpdir = QDesktopServices::storageLocation(QDesktopServices::TempLocation);
    QString fileName = tmpdir + QDir::separator() + "chlorine." + ui.filetype->currentText();
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    QTextStream stream(&file);
    if (formatIsPdb) {
      stream << "HETATM    1 CL   LIG     1       0.000   0.000   0.000  1.00  0.00          Cl  ";
    } else {
      // xyz
      stream << "1\n";
      stream << "chlorine.xyz\n";
      stream << "Cl         0.00000        0.00000        0.00000";
    }
    file.close();
  }

  void PackmolDialog::runButtonClicked()
  {
    /*
    if (m_process) {
      m_process->deleteLater();
      ui.tabWidget->setCurrentIndex(2); // change to output mode
      return;
    }
    */
    QString program = "/usr/local/bin/packmol"; // FIXME: should be option

    ui.runButton->setEnabled(false);
    ui.abortButton->setEnabled(true);

    QString tmpdir = QDesktopServices::storageLocation(QDesktopServices::TempLocation);

    // Write the input file
    QFile inputFile(tmpdir + QDir::separator() + "input.inp");
    if (!inputFile.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    QTextStream stream(&inputFile);
    stream << ui.textEdit->toPlainText().toAscii();
    inputFile.close();

    // Make sure we know where all files are
    QStringList files;
    QStringList lines = ui.textEdit->toPlainText().split("\n");
    foreach (const QString &line, lines) {
      if (line.contains("end"))
        continue;
      if (!line.contains("structure"))
        continue;

      QStringList tokens = line.split(QRegExp("\\s+"));
      if (tokens.length() != 2)
        continue;

      files.append(tokens[1]);
    }

    foreach (const QString &file, files) {
      if (!m_fileLookup.contains(file)) {
        QMessageBox::StandardButton result = QMessageBox::question(this, tr("File not found"),
            tr("File %1 not found. Look forit now?").arg(file), QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No)
          return;
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Molecule"));
        m_fileLookup[file] = fileName;
      }
    }

    // Now we know where all files are, move/convert them to the temp location
    foreach (const QString &shortFileName, m_fileLookup.keys()) {
      QString fullFileName = m_fileLookup.value(shortFileName);
      QFileInfo fileInfo(fullFileName);

      QString tmpFile = tmpdir + QDir::separator() + fileInfo.baseName() + "." + ui.filetype->currentText();

      QSharedPointer<Molecule> molecule(MoleculeFile::readMolecule(fullFileName));
      if (!molecule)
        return;
      MoleculeFile::writeMolecule(molecule.data(), tmpFile);
    }

    // Create & setup the process
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
        this, SLOT(processFinished(int,QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(updateStandardOutput()));
    m_process->setStandardInputFile(tmpdir + QDir::separator() + "input.inp");
    m_process->setWorkingDirectory(tmpdir);
    m_process->start(program);

    ui.tabWidget->setCurrentIndex(2); // change to output mode
    ui.outputEdit->append(tr("Running...\n"));
  }

  void PackmolDialog::abortButtonClicked()
  {
    m_process->deleteLater();
    m_process = 0;
    ui.outputEdit->append(tr("Aborting...\n"));
  }

  void PackmolDialog::updateStandardOutput()
  {
    ui.outputEdit->append(m_process->read(10000));
  }

  void PackmolDialog::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
  {
    ui.runButton->setEnabled(true);
    ui.abortButton->setEnabled(false);

    if (!m_process)
      return;

    QString tmpdir = QDesktopServices::storageLocation(QDesktopServices::TempLocation);
    QString resultFileName = tmpdir + QDir::separator() + ui.output->text();

    Molecule *molecule = MoleculeFile::readMolecule(resultFileName);
    if (molecule)
      emit resultReady(molecule);

    m_process->deleteLater();
    m_process = 0;
  }

  void PackmolDialog::visitWebsite()
  {
      QDesktopServices::openUrl(QUrl("http://www.ime.unicamp.br/~martinez/packmol/"));
  }

  void PackmolDialog::writeSettings(QSettings &settings) const
  {
    settings.setValue("packmolExecutable", ui.packmolExecutable->text());
    settings.setValue("packmolTolerance", ui.tolerance->value());
    settings.setValue("packmolFiletype", ui.filetype->currentIndex());
    settings.setValue("packmolOutput", ui.output->text());
    settings.setValue("packmolSeed", ui.seed->value());
    settings.setValue("packmolMaxit", ui.maxit->value());
    settings.setValue("packmolNloop", ui.nloop->value());
    settings.setValue("packmolWriteout", ui.writeout->value());
    settings.setValue("packmolAddAmberTer", ui.addAmberTer->isChecked());
    settings.setValue("packmolAddBoxSides", ui.addBoxSides->isChecked());
    settings.setValue("packmolRandomInitialPoint", ui.randomInitialPoint->isChecked());
  }

  void PackmolDialog::readSettings(QSettings &settings)
  {
    // executable
    QString executable = settings.value("packmolExecutable").toString();
    if (executable.isEmpty()) {
      // Figure out reasonable defaults
#ifdef Q_WS_WIN
      executable = QCoreApplication::applicationDirPath() + QDir::separator() + "packmol.exe";
#else
      executable = "/usr/local/bin/packmol";
#endif
    }
    ui.packmolExecutable->setText(executable);
    // tolarance, filetype, output
    ui.tolerance->setValue(settings.value("packmolTolerance", 2.0).toDouble());
    ui.filetype->setCurrentIndex(settings.value("packmolFiletype", 0).toInt());
    ui.output->setText(settings.value("packmolOutput", "result.pdb").toString());
    ui.seed->setValue(settings.value("packmolSeed", 0).toInt());
  }




}

