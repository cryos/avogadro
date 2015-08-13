/**********************************************************************
  EspressoInputDialog - Dialog for generating Espresso input decks

  Copyright (C) 2012 Albert DeFusco

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

#include "espressoinputdialog.h"

#include <avogadro/avospglib.h>
#include <avogadro/obeigenconv.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>

#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QTextBrowser>
#include <QFileDialog>
#include <QMessageBox>
#include <QShowEvent>
#include <QSettings>
#include <QDebug>
#include <QProcess>
#include <QHash>
#include <QTableWidget>
#include <QSignalMapper>

#define ANG2BOHR 1.889725989
#define PI       3.141592654

using namespace OpenBabel;

namespace Avogadro
{

  EspressoInputDialog::EspressoInputDialog(QWidget *parent, Qt::WindowFlags f)
    : InputDialog(parent, f),
    //main
    m_title("Title"),

    //crystal structure tab
    m_crystalType(Display),
    m_bravaisType(free),
    m_unitType(alat),
    m_ibrav(0),
    m_latticeA(1.0),
    m_latticeB(1.0),
    m_latticeC(1.0),
    m_latticeAlpha(90.0),
    m_latticeBeta(90.0),
    m_latticeGamma(90.0),

    //calculation tab
    m_calcType(scf),
    m_verboseType(high),
    m_restartType(scr),
    m_outdir("tmp"),
    m_symm(true),
    m_autoBand(true),
    m_bands(0),
    m_charge(0.0),
    m_cutoff(20.0),
    m_smearType(gauss),
    m_dgauss(0.0),
    m_stress(false),
    m_forces(false),
    m_timeStep(20.0),
    m_nSteps(100),

    //pseudotab
    m_pseudoEnv(true),

    //k point tab
    m_kpointType(automatic),
    m_nKA(4),
    m_nKB(4),
    m_nKC(4),
    m_offKA(true),
    m_offKB(true),
    m_offKC(true),

    // Rest
    m_dirty(false),
    m_warned(false)
  {
    ui.setupUi(this);
    // Connect the GUI elements to the correct slots
    // Basic Tab
    connect(ui.titleLine, SIGNAL(editingFinished()),
        this, SLOT(setTitle()));

    connect(ui.previewText, SIGNAL(textChanged()),
        this, SLOT(previewEdited()));
    connect(ui.generateButton, SIGNAL(clicked()),
        this, SLOT(generateClicked()));
    connect(ui.resetButton, SIGNAL(clicked()),
        this, SLOT(resetClicked()));
    connect(ui.moreButton, SIGNAL(clicked()),
        this, SLOT(moreClicked()));
    connect(ui.enableFormButton, SIGNAL(clicked()),
        this, SLOT(enableFormClicked()));
    connect(ui.closeButton, SIGNAL(clicked()),
        this, SLOT(close()));

    //crystal structure tab
    connect(ui.crystalCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setCrystalStructure(int)));
    connect(ui.spinLatticeA, SIGNAL(valueChanged(double)),
        this, SLOT(setLatticeA(double)));
    connect(ui.spinLatticeB, SIGNAL(valueChanged(double)),
        this, SLOT(setLatticeB(double)));
    connect(ui.spinLatticeC, SIGNAL(valueChanged(double)),
        this, SLOT(setLatticeC(double)));
    connect(ui.spinLatticeAlpha, SIGNAL(valueChanged(double)),
        this, SLOT(setLatticeAlpha(double)));
    connect(ui.spinLatticeBeta, SIGNAL(valueChanged(double)),
        this, SLOT(setLatticeBeta(double)));
    connect(ui.spinLatticeGamma, SIGNAL(valueChanged(double)),
        this, SLOT(setLatticeGamma(double)));
    connect(ui.comboBravais, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setBravaisLattice(int)));
    connect(ui.comboUnits, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setUnits(int)));

    //calculation tab
    connect(ui.comboCalc, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setCalcType(int)));
    connect(ui.comboVerbose, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setVerbosity(int)));
    connect(ui.comboRestart, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setRestartType(int)));
    connect(ui.lineOutput, SIGNAL(editingFinished()),
        this, SLOT(setOutDir()));
    connect(ui.linePrefix, SIGNAL(editingFinished()),
        this, SLOT(setPrefix()));
    connect(ui.checkSymm, SIGNAL(toggled(bool)),
        this, SLOT(setSymmetry(bool)));
    connect(ui.checkAutoBands, SIGNAL(toggled(bool)),
        this, SLOT(setAutoBands(bool)));
    connect(ui.spinBands, SIGNAL(valueChanged(int)),
        this, SLOT(setBands(int)));
    connect(ui.spinCharge, SIGNAL(valueChanged(double)),
        this, SLOT(setCharge(double)));
    connect(ui.spinCutoff, SIGNAL(valueChanged(double)),
        this, SLOT(setCutoff(double)));
    connect(ui.comboSmear, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setSmearType(int)));
    connect(ui.spinDGauss, SIGNAL(valueChanged(double)),
        this, SLOT(setDGauss(double)));
    connect(ui.checkStress, SIGNAL(toggled(bool)),
        this, SLOT(setStress(bool)));
    connect(ui.checkForces, SIGNAL(toggled(bool)),
        this, SLOT(setForces(bool)));
    connect(ui.spinTimeStep, SIGNAL(valueChanged(double)),
        this, SLOT(setTimeStep(double)));
    connect(ui.spinSteps, SIGNAL(valueChanged(int)),
        this, SLOT(setNSteps(int)));

    //pseudo tab
    connect(ui.linePseudo, SIGNAL(editingFinished()),
        this, SLOT(setPseudoDir()));
    connect(ui.checkPseudoEnv, SIGNAL(toggled(bool)),
        this, SLOT(setPseudoEnv(bool)));

    //k point tab
    connect(ui.comboKPoint, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setKPointType(int)));
    connect(ui.spinKA, SIGNAL(valueChanged(int)),
        this, SLOT(setNKA(int)));
    connect(ui.spinKB, SIGNAL(valueChanged(int)),
        this, SLOT(setNKB(int)));
    connect(ui.spinKC, SIGNAL(valueChanged(int)),
        this, SLOT(setNKC(int)));
    connect(ui.checkOffA, SIGNAL(toggled(bool)),
        this, SLOT(setOffKA(bool)));
    connect(ui.checkOffB, SIGNAL(toggled(bool)),
        this, SLOT(setOffKB(bool)));
    connect(ui.checkOffC, SIGNAL(toggled(bool)),
        this, SLOT(setOffKC(bool)));

    //prepare the pseudopotential table
    ui.pseudoTableWidget->setColumnCount(2);
    ui.pseudoTableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("Atom"));
    ui.pseudoTableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("Pseudopotential"));
    ui.pseudoTableWidget->setColumnWidth(1,220);

    QSettings settings;
    readSettings(settings);

    //there may be a better way to initialize
    //hash tables.
    setPseudoFiles();

    // Generate an initial preview of the input deck
    updatePreviewText();
  }


  EspressoInputDialog::~EspressoInputDialog()
  {
      QSettings settings;
      writeSettings(settings);
  }

  void EspressoInputDialog::setMolecule(Molecule *molecule)
  {
    // Disconnect the old molecule first...
    if (m_molecule) {
      disconnect(m_molecule, 0, this, 0);
    }

    m_molecule = molecule;
    // Update the preview text whenever atoms are changed
    // this is done by first updating the hash tables that
    // control pseudopotential input
    connect(m_molecule, SIGNAL(atomRemoved(Atom *)),
            this, SLOT(updatePreviewText()));
    connect(m_molecule, SIGNAL(atomAdded(Atom *)),
            this, SLOT(updatePreviewText()));
    connect(m_molecule, SIGNAL(atomUpdated(Atom *)),
            this, SLOT(updatePreviewText()));
    // Add atom coordinates
    if(currentCell()) {
      m_latticeA=currentCell()->GetA();
      ui.spinLatticeA->setValue(m_latticeA);
    }
    updatePreviewText();
  }

  void EspressoInputDialog::preparePseudoInput()
  {
    //-----------------------------------------------------
    // The pseudopotential selection works by assigning
    // a pseudopotential file to an atom type.  I have
    // implemented pull down menus through generation
    // of a QTableWidget each time the molecule is updated.
    //

    //Step 1: determine unique atom types based on atomicNumber
    //but the atomic mass is required for the espresso input
    QList<Atom *> atoms = m_molecule->atoms();
    int count=0;
    atomTypes.clear();
    atomMass.clear();
    foreach(Atom *atom, atoms) {
      int ThisAtom = atom->atomicNumber();
      count++;
      atomTypes[ThisAtom] = count;
      atomMass[ThisAtom] = atom->OBAtom().GetAtomicMass();
    }
    numAtoms=atoms.size();
    //Step 2: Fill the QTableWidget with number of atomTypes
    //first, clear it!
    //ui.pseudoTableWidget->clear();
    for (int i=ui.pseudoTableWidget->rowCount()-1;i>=0;--i)
      ui.pseudoTableWidget->removeRow(i);
    //now, make a new one
    ui.pseudoTableWidget->setRowCount(atomTypes.size());
    signalMapper = new QSignalMapper(this);
    int rowCount=0;
    for(itr=atomTypes.begin();itr!=atomTypes.end();++itr)
    {
      QString label=OpenBabel::etab.GetSymbol(itr.key());
      ui.pseudoTableWidget->setItem(rowCount,0,new QTableWidgetItem(label));

      //if a pseudopotential was found for this atom, fill the comboBox.
      //pseudopotential names are hardcoded at the bottom of this file.
      QComboBox* combo = new QComboBox();
      if(atomPseudoFiles.contains(label))
        combo->addItems(atomPseudoFiles[label]);
      else
      {
        combo->addItems((QStringList() << "No pseudopotentials found"));
        combo->setEnabled(false);
      }
      //signalMapper was used so that each comboBox can determine its
      //origin and print the pseudopotential name for the
      //particular atom type.
      ui.pseudoTableWidget->setCellWidget(rowCount,1,combo);
      connect(combo, SIGNAL(currentIndexChanged(int)),
          signalMapper, SLOT(map()));
      signalMapper->setMapping(combo, rowCount);

      rowCount++;
    }
    connect(signalMapper, SIGNAL(mapped(int)),
        this, SLOT(setPseudoType(int)));

  }

  //signalMapper is used because I do not know how many
  //unique atom types will be loaded
  void EspressoInputDialog::setPseudoType(int row)
  {
    //first, get the correct combo and its text
    QComboBox *combo = qobject_cast<QComboBox *>(ui.pseudoTableWidget->cellWidget(row,1));
    QString pseudo = combo->currentText();
    //now, find the atom type
    QString atomSymbol = ui.pseudoTableWidget->item(row,0)->text();

    //Finally, set the pseudo potential for the atom type and update the preview
    atomPseudo[atomSymbol] = pseudo;

    updatePreviewText();

  }

  void EspressoInputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();
  }

  void EspressoInputDialog::updatePreviewText()
  {
    if (!isVisible())
      return;
    // Generate the input deck and display it
    if (m_dirty && !m_warned) {
      m_warned = true;
      QMessageBox msgBox;

      msgBox.setWindowTitle(tr("Espresso Input Deck Generator Warning"));
      msgBox.setText(tr("Would you like to update the preview text, "
            "losing all changes made in the Espresso input "
            "deck preview pane?"));
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

      switch (msgBox.exec()) {
        case QMessageBox::Yes:
          // yes was clicked
          deckDirty(false);
          ui.previewText->setText(generateInputDeck());
          m_warned = false;
          break;
        case QMessageBox::No:
          // no was clicked
          m_warned = false;
          break;
        default:
          // should never be reached
          break;
      }
    }
    else if (!m_dirty) {
      ui.previewText->setText(generateInputDeck());
      ui.previewText->document()->setModified(false);
    }
  }

  void EspressoInputDialog::resetClicked()
  {

    // Reset the form to defaults
    deckDirty(false);
    // Basic Tab
    //ui.titleLine->setText("");
    //setTitle();
    ui.crystalCombo->setCurrentIndex(0);
    ui.previewText->document()->setModified(false);
  }

  void EspressoInputDialog::generateClicked()
  {
    saveInputFile(ui.previewText->toPlainText(),
                          tr("Espresso Input Deck"), QString("in"));
  }

  void EspressoInputDialog::moreClicked()
  {
    // If the more button is clicked hide/show the preview text
    if (ui.previewText->isVisible()) {
      ui.previewText->hide();
      ui.moreButton->setText(tr("Show Preview"));
    }
    else {
      ui.previewText->show();
      ui.moreButton->setText(tr("Hide Preview"));
    }
  }

  void EspressoInputDialog::enableFormClicked()
  {
    updatePreviewText();
  }

  void EspressoInputDialog::previewEdited()
  {
    // Determine if the preview text has changed from the form generated
    if(ui.previewText->toPlainText() != generateInputDeck())
      deckDirty(true);
    else
      deckDirty(false);
  }

  void EspressoInputDialog::setTitle()
  {
    m_title = ui.titleLine->text();
    updatePreviewText();
  }

  //crystal structure tab slots
  void EspressoInputDialog::setCrystalStructure(int n)
  {
    m_crystalType = (EspressoInputDialog::crystalType) n;
    updatePreviewText();
  }

  void EspressoInputDialog::setBravaisLattice(int n)
  {
    m_bravaisType = (EspressoInputDialog::bravaisType) n;
    m_ibrav = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setUnits(int n)
  {
    m_unitType = (EspressoInputDialog::unitType) n;
    updatePreviewText();
  }

  void EspressoInputDialog::setLatticeA(double n)
  {
    m_latticeA = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setLatticeB(double n)
  {
    m_latticeB = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setLatticeC(double n)
  {
    m_latticeC = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setLatticeAlpha(double n)
  {
    m_latticeAlpha = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setLatticeBeta(double n)
  {
    m_latticeBeta = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setLatticeGamma(double n)
  {
    m_latticeGamma = n;
    updatePreviewText();
  }

  //calculation tab slots
  void EspressoInputDialog::setCalcType(int n)
  {
    m_calcType = (EspressoInputDialog::calcType) n;

    if(m_calcType == vcmd || m_calcType == vcrelax)
      ui.checkStress->setChecked(true);
    if(m_calcType == md || m_calcType == relax || m_calcType == vcmd)
      ui.checkForces->setChecked(true);
    if(m_calcType == md || m_calcType == vcmd)
    {
      ui.spinTimeStep->setEnabled(true);
      ui.spinSteps->setEnabled(true);
    }
    else
    {
      ui.spinTimeStep->setEnabled(false);
      ui.spinSteps->setEnabled(false);
    }

    updatePreviewText();
  }

  void EspressoInputDialog::setVerbosity(int n)
  {
    m_verboseType = (EspressoInputDialog::verboseType) n;
    updatePreviewText();
  }

  void EspressoInputDialog::setRestartType(int n)
  {
    m_restartType = (EspressoInputDialog::restartType) n;
    updatePreviewText();
  }

  void EspressoInputDialog::setOutDir()
  {
    m_outdir = ui.lineOutput->text();
    updatePreviewText();
  }

  void EspressoInputDialog::setPrefix()
  {
    m_prefix = ui.linePrefix->text();
    updatePreviewText();
  }

  void EspressoInputDialog::setSymmetry(bool n)
  {
    m_symm = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setAutoBands(bool n)
  {
    m_autoBand = n;
    if(n)
    {
      ui.spinBands->setValue(0);
      ui.spinBands->setEnabled(false);
    }
    else
      ui.spinBands->setEnabled(true);
    updatePreviewText();
  }

  void EspressoInputDialog::setBands(int n)
  {
    m_bands = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setCharge(double n)
  {
    m_charge = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setCutoff(double n)
  {
    m_cutoff = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setSmearType(int n)
  {
    m_smearType = (EspressoInputDialog::smearType) n;
    updatePreviewText();
  }

  void EspressoInputDialog::setDGauss(double n)
  {
    m_dgauss = n;
    updatePreviewText();
  }
  void EspressoInputDialog::setStress(bool n)
  {
    m_stress = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setForces(bool n)
  {
    m_forces = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setTimeStep(double n)
  {
    m_timeStep = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setNSteps(int n)
  {
    m_nSteps = n;
    updatePreviewText();
  }

  //pseudo tab
  void EspressoInputDialog::setPseudoDir()
  {
    m_pseudodir = ui.linePseudo->text();
    updatePreviewText();
  }

  void EspressoInputDialog::setPseudoEnv(bool n)
  {
    m_pseudoEnv = n;
    if(n)
      ui.linePseudo->setEnabled(false);
    else
      ui.linePseudo->setEnabled(true);
    updatePreviewText();
  }

  //k point tab
  void EspressoInputDialog::setKPointType(int n)
  {
    m_kpointType = (EspressoInputDialog::kpointType) n;
    if(m_kpointType == gamma)
    {
      ui.spinKA->setEnabled(false);
      ui.spinKB->setEnabled(false);
      ui.spinKC->setEnabled(false);
      ui.checkOffA->setEnabled(false);
      ui.checkOffB->setEnabled(false);
      ui.checkOffC->setEnabled(false);
    }
    else
    {
      ui.spinKA->setEnabled(true);
      ui.spinKB->setEnabled(true);
      ui.spinKC->setEnabled(true);
      ui.checkOffA->setEnabled(true);
      ui.checkOffB->setEnabled(true);
      ui.checkOffC->setEnabled(true);
    }
    updatePreviewText();
  }

  void EspressoInputDialog::setNKA(int n)
  {
    m_nKA = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setNKB(int n)
  {
    m_nKB = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setNKC(int n)
  {
    m_nKC = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setOffKA(bool n)
  {
    m_offKA = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setOffKB(bool n)
  {
    m_offKB = n;
    updatePreviewText();
  }

  void EspressoInputDialog::setOffKC(bool n)
  {
    m_offKC = n;
    updatePreviewText();
  }


  //crystal structure tab output
  QString EspressoInputDialog::getCrystalStructure(crystalType t)
  {
    switch (t)
    {
      case Display:
        {
          QString buffer = getDisplayedCrystal();
          return buffer;
        }
      case Primitive:
        {
          QString buffer = generatePrimitiveLattice();
          return buffer;
        }
      case Manual:
        {
          QString buffer = getManualLattice();
          return buffer;
        }
      default:
        {
          QString buffer = getDisplayedCrystal();
          return buffer;
        }
    }
  }

  QString EspressoInputDialog::getDisplayedCrystal()
  {
    //First, disable unnecessary input keywords
    ui.spinLatticeB->setEnabled(false);
    ui.spinLatticeC->setEnabled(false);
    ui.spinLatticeAlpha->setEnabled(false);
    ui.spinLatticeBeta->setEnabled(false);
    ui.spinLatticeGamma->setEnabled(false);
    ui.comboUnits->setEnabled(false);
    ui.comboBravais->setCurrentIndex(0);
    ui.comboBravais->setEnabled(false);

    //prepare memory
    QList<Eigen::Vector3d> fcoords;
    QList<unsigned int> atomicNums;
    Eigen::Matrix3d cellMatrix,fracMatrix;
    Spglib::prepareMolecule(m_molecule, currentCell(), &fcoords, &atomicNums, &cellMatrix);
    QString buffer,tmp;

    buffer = "ATOMIC_SPECIES\n";
    for (itr=atomTypes.begin();itr!=atomTypes.end();++itr)
    {
      QString symbol= OpenBabel::etab.GetSymbol( itr.key() );
      if(!atomPseudo.contains(symbol))
        initializePseudo(symbol);
      tmp.sprintf("%3s %9.4f %s\n",
          symbol.toStdString().c_str(),
          atomMass[itr.key()],
          atomPseudo[symbol].toStdString().c_str());
      buffer.append(tmp);
    }

    //Crystal means x_{frac} = L^-1*x_{cart},
    //which I think corresponds to other people's definition
    //of fractional coordinates.
    numAtoms = atomicNums.size();
    buffer.append("ATOMIC_POSITIONS crystal\n");
    for(int iatom = 0;iatom<atomicNums.size();iatom++)
    {
      Eigen::Vector3d fracCoord;
      fracCoord=fcoords[iatom];
      tmp.sprintf("%3s %12.6f %12.6f %12.6f\n",
          OpenBabel::etab.GetSymbol( atomicNums.at(iatom) ),
          fracCoord.x(),
          fracCoord.y(),
          fracCoord.z());
      buffer.append(tmp);
    }
    //cubic is default
    //hexagonal is the only other option and needs to be tested
    buffer.append("CELL_PARAMETERS cubic\n");
    for(int i=0;i<3;i++)
    {
      tmp.sprintf("%12.6f %12.6f %12.6f\n",
          cellMatrix(i,0)/currentCell()->GetA(),
          cellMatrix(i,1)/currentCell()->GetA(),
          cellMatrix(i,2)/currentCell()->GetA());
      buffer.append(tmp);
    }
    return buffer;
  }

  QString EspressoInputDialog::generatePrimitiveLattice()
  {
    //First, disable unnecessary input keywords
    ui.spinLatticeB->setEnabled(false);
    ui.spinLatticeC->setEnabled(false);
    ui.spinLatticeAlpha->setEnabled(false);
    ui.spinLatticeBeta->setEnabled(false);
    ui.spinLatticeGamma->setEnabled(false);
    ui.comboUnits->setEnabled(false);
    ui.comboBravais->setCurrentIndex(0);
    ui.comboBravais->setEnabled(false);


    QString buffer;

    //prepare memory to get primitive lattice and basis
    QList<Eigen::Vector3d> fcoords;
    QList<unsigned int> atomicNums;
    Eigen::Matrix3d cellMatrix;
    //It would be great to fill the unit cell first
    //uca->FillUnitCell(*obmol);
    Spglib::prepareMolecule(m_molecule, currentCell(), &fcoords, &atomicNums, &cellMatrix);

    const double tolerance=1e-5;
    const bool refine=true;
    unsigned int spg=Spglib::getPrimitive(&fcoords, &atomicNums, &cellMatrix,tolerance,refine);
    if(spg == 0) {
      qDebug() << "crap, no space group.";
      buffer = "---- ERROR: Spacegroup symmetry not found. ----\n";
      return buffer;
    }
    else
    {
      buffer = "ATOMIC_SPECIES\n";
      for (itr=atomTypes.begin();itr!=atomTypes.end();++itr)
      {
        QString tmp;
        QString symbol= OpenBabel::etab.GetSymbol( itr.key() );
        if(!atomPseudo.contains(symbol))
          initializePseudo(symbol);
        tmp.sprintf("%3s %9.4f %s\n",
            symbol.toStdString().c_str(),
            atomMass[itr.key()],
            atomPseudo[symbol].toStdString().c_str());
        buffer.append(tmp);
      }
      //Crystal means x_{frac} = L^-1*x_{cart},
      //which I think corresponds to other people's definition
      //of fractional coordinates.
      numAtoms = atomicNums.size();
      buffer.append("ATOMIC_POSITIONS crystal\n");
      for(int iatom = 0;iatom<atomicNums.size();iatom++)
      {
        Eigen::Vector3d fracCoord;
        fracCoord=fcoords[iatom];
        QString tmp;
        tmp.sprintf("%3s %12.6f %12.6f %12.6f\n",
            OpenBabel::etab.GetSymbol( atomicNums.at(iatom) ),
            fracCoord.x(),
            fracCoord.y(),
            fracCoord.z());
        buffer.append(tmp);
      }
      //cubic is default
      //hexagonal is the only other option and needs to be tested
      buffer.append("CELL_PARAMETERS cubic\n");
      for(int i=0;i<3;i++)
      {
        QString tmp;
        tmp.sprintf("%12.6f %12.6f %12.6f\n",
            cellMatrix(i,0)/currentCell()->GetA(),
            cellMatrix(i,1)/currentCell()->GetA(),
            cellMatrix(i,2)/currentCell()->GetA());
        buffer.append(tmp);
      }
      return buffer;
    }
  }

  QString EspressoInputDialog::getManualLattice()
  {
    QString buffer;

    //First, enable necessary input keywords
    //spinLatticeA is always enabled
    ui.comboBravais->setEnabled(true);
    ui.comboUnits->setEnabled(true);

    //Lattice definitions
    //http://en.wikipedia.org/wiki/Bravais_lattice

    //cubic latices: P, F and I (free is included as well)
    if(m_bravaisType == free || m_bravaisType == cubic
        || m_bravaisType == fcc || m_bravaisType == bcc)
    {
      ui.spinLatticeB->setValue(m_latticeA);
      ui.spinLatticeC->setValue(m_latticeA);
      ui.spinLatticeAlpha->setValue(90);
      ui.spinLatticeBeta->setValue(90);
      ui.spinLatticeGamma->setValue(90);

      ui.spinLatticeB->setEnabled(false);
      ui.spinLatticeC->setEnabled(false);
      ui.spinLatticeAlpha->setEnabled(false);
      ui.spinLatticeBeta->setEnabled(false);
      ui.spinLatticeGamma->setEnabled(false);

    }
    //tetragonal lattices: P and I
    else if(m_bravaisType == tetP || m_bravaisType == tetI)
    {
      ui.spinLatticeB->setValue(m_latticeA);
      ui.spinLatticeAlpha->setValue(90);
      ui.spinLatticeBeta->setValue(90);
      ui.spinLatticeGamma->setValue(90);

      ui.spinLatticeB->setEnabled(false);
      ui.spinLatticeC->setEnabled(true);
      ui.spinLatticeAlpha->setEnabled(false);
      ui.spinLatticeBeta->setEnabled(false);
      ui.spinLatticeGamma->setEnabled(false);
    }
    else if(m_bravaisType == ortP || m_bravaisType == bco
        || m_bravaisType == ortF || m_bravaisType == ortB)
    {
      ui.spinLatticeB->setValue(m_latticeB);
      ui.spinLatticeC->setValue(m_latticeC);
      ui.spinLatticeAlpha->setValue(90);
      ui.spinLatticeBeta->setValue(90);
      ui.spinLatticeGamma->setValue(90);

      ui.spinLatticeB->setEnabled(true);
      ui.spinLatticeC->setEnabled(true);
      ui.spinLatticeAlpha->setEnabled(false);
      ui.spinLatticeBeta->setEnabled(false);
      ui.spinLatticeGamma->setEnabled(false);

    }
    //Monoclinic lattices: P and B (C)
    else if(m_bravaisType == monP || m_bravaisType == monoB)
    {
      ui.spinLatticeB->setValue(m_latticeA);
      ui.spinLatticeC->setValue(m_latticeA);
      ui.spinLatticeAlpha->setValue(m_latticeAlpha);
      ui.spinLatticeBeta->setValue(90);
      ui.spinLatticeGamma->setValue(90);

      ui.spinLatticeB->setEnabled(true);
      ui.spinLatticeC->setEnabled(true);
      ui.spinLatticeAlpha->setEnabled(true);
      ui.spinLatticeBeta->setEnabled(false);
      ui.spinLatticeGamma->setEnabled(false);
    }
    //hexagonal and Trigonal P
    else if(m_bravaisType == hexP)
    {
      ui.spinLatticeB->setValue(m_latticeA);
      ui.spinLatticeC->setValue(m_latticeC);
      ui.spinLatticeAlpha->setValue(90);
      ui.spinLatticeBeta->setValue(90);
      ui.spinLatticeGamma->setValue(120);

      ui.spinLatticeB->setEnabled(false);
      ui.spinLatticeC->setEnabled(true);
      ui.spinLatticeAlpha->setEnabled(false);
      ui.spinLatticeBeta->setEnabled(false);
      ui.spinLatticeGamma->setEnabled(false);
    }
    //triclinic
    else if(m_bravaisType == tri)
    {
      ui.spinLatticeB->setValue(m_latticeB);
      ui.spinLatticeC->setValue(m_latticeC);
      ui.spinLatticeAlpha->setValue(m_latticeAlpha);
      ui.spinLatticeBeta->setValue(m_latticeBeta);
      ui.spinLatticeGamma->setValue(m_latticeGamma);

      ui.spinLatticeB->setEnabled(true);
      ui.spinLatticeC->setEnabled(true);
      ui.spinLatticeAlpha->setEnabled(true);
      ui.spinLatticeBeta->setEnabled(true);
      ui.spinLatticeGamma->setEnabled(true);
    }
    //Trigonal R
    else if(m_bravaisType == trigR)
    {
      ui.spinLatticeB->setValue(m_latticeA);
      ui.spinLatticeC->setValue(m_latticeA);
      ui.spinLatticeAlpha->setValue(m_latticeBeta);
      ui.spinLatticeBeta->setValue(m_latticeBeta);
      ui.spinLatticeGamma->setValue(m_latticeBeta);

      ui.spinLatticeB->setEnabled(false);
      ui.spinLatticeC->setEnabled(false);
      ui.spinLatticeAlpha->setEnabled(false);
      ui.spinLatticeBeta->setEnabled(true);
      ui.spinLatticeGamma->setEnabled(false);
    }
    else {
      //Just in case.
      ui.spinLatticeB->setValue(m_latticeB);
      ui.spinLatticeC->setValue(m_latticeC);
      ui.spinLatticeAlpha->setValue(m_latticeAlpha);
      ui.spinLatticeBeta->setValue(m_latticeBeta);
      ui.spinLatticeGamma->setValue(m_latticeGamma);

      ui.spinLatticeB->setEnabled(true);
      ui.spinLatticeC->setEnabled(true);
      ui.spinLatticeAlpha->setEnabled(true);
      ui.spinLatticeBeta->setEnabled(true);
      ui.spinLatticeGamma->setEnabled(true);
    }

    buffer = "ATOMIC_SPECIES\n";
    for (itr=atomTypes.begin();itr!=atomTypes.end();++itr)
    {
      QString tmp;
      QString symbol= OpenBabel::etab.GetSymbol( itr.key() );
      if(!atomPseudo.contains(symbol))
        initializePseudo(symbol);
      tmp.sprintf("%3s %9.4f %s\n",
          symbol.toStdString().c_str(),
          atomMass[itr.key()],
          atomPseudo[symbol].toStdString().c_str());
      buffer.append(tmp);
    }
    buffer.append("ATOMIC_POSITIONS "+getUnits(m_unitType)+"\n");
    QList<Atom *> atoms = m_molecule->atoms();
    numAtoms = atoms.size();
    foreach (Atom *atom, atoms)
    {
      //Cartesian coordiantes
      //alat means that cartesian coordiantes
      //are scaled by lattice parameter A
      QString tmp;
      tmp.sprintf("%3s %12.6f %12.6f %12.6f\n",
          OpenBabel::etab.GetSymbol(atom->atomicNumber()),
          atom->pos()->x()*getUnitConv(m_unitType),
          atom->pos()->y()*getUnitConv(m_unitType),
          atom->pos()->z()*getUnitConv(m_unitType));
      buffer.append(tmp);
    }
    return buffer;
  }

  double EspressoInputDialog::getUnitConv(unitType t)
  {
    switch(t)
    {
      //alat means that cartesian coordiantes
      //are scaled by lattice parameter A
      //not real fractional (crystal) units
      case alat:
        return 1.0/currentCell()->GetA();
      case ang:
        return 1.0;
      case bohr:
        return ANG2BOHR;
      default:
        return 1.0/currentCell()->GetA();
    }
  }

  QString EspressoInputDialog::getUnits(unitType t)
  {
    switch(t)
    {
      case alat:
        return "alat";
      case bohr:
        return "bohr";
      case ang:
        return "angstrom";
      default:
        return "alat";
    }
  }

  //calculation tab output
  QString EspressoInputDialog::getCalcType(calcType t)
  {
    switch(t)
    {
      case scf:
        return "'scf'";
      case nscf:
        return "'nscf'";
      case bands:
        return "'bands'";
      case relax:
        return "'relax'";
      case md:
        return "'md'";
      case vcrelax:
        return "'vc-relax'";
      case vcmd:
        return "'vc-md'";
      default:
        return "'scf'";
    }
  }

  QString EspressoInputDialog::getVerboseType(verboseType t)
  {
    switch(t)
    {
      case high:
        return "'high'";
      case def:
        return "'default'";
      case low:
        return "'low'";
      case min:
        return "'minimal'";
      default:
        return "'high'";
    }
  }

  QString EspressoInputDialog::getRestartType(restartType t)
  {
    switch(t)
    {
      case scr:
        return "'from_scratch'";
      case restart:
        return "'restart'";
      default:
        return "'from_scratch'";
    }
  }

  QString EspressoInputDialog::getSmearType(smearType t)
  {
    switch(t)
    {
      case gauss:
        return "'gauss'";
      case mp:
        return "'mp'";
      case mv:
        return "'mv'";
      case fd:
        return "'fd'";
      default:
        return "'gauss'";
    }
  }
  QString EspressoInputDialog::getNoSymm(bool t)
  {
    if(t)
      return ".false.";
    else
      return ".true.";
  }

  QString EspressoInputDialog::getStress(bool t)
  {
    if(t)
      return ".true.";
    else
      return ".false.";
  }

  QString EspressoInputDialog::getForces(bool t)
  {
    if(t)
      return ".true.";
    else
      return ".false.";
  }

  //k point tab output
  QString EspressoInputDialog::getKPointInput(kpointType t)
  {
    QString buffer;
    QTextStream tmp(&buffer);
    switch(t)
    {
      case automatic:
        {
          tmp << "K_POINTS automatic\n";
          tmp << m_nKA << " " << m_nKB << " " << m_nKC << " "
            << m_offKA << " " << m_offKB << " " << m_offKC;
          return buffer;
        }
      case gamma:
        {
          tmp << "K_POINTS gamma\n";
          return buffer;
        }
      default:
        {
          tmp << "K_POINTS automatice\n4 4 4 1 1 1\n";
          return buffer;
        }
    }
  }

  //general output
  QString EspressoInputDialog::getSystemNameList()
  {
    QString buffer;
    QTextStream tmp(&buffer);
    tmp.setRealNumberNotation(QTextStream::FixedNotation);
    tmp.setRealNumberPrecision(3);
    tmp << " &system\n";
    tmp << "        nat = " << numAtoms << "\n";
    tmp << "       ntyp = " << atomTypes.size() << "\n";
    tmp << "      ibrav = " << m_ibrav << "\n";
    if(m_bands > 0)
      tmp << "       nbnd = " << m_bands << "\n";
    tmp << " tot_charge = " << m_charge << "\n";
    tmp << "    ecutwfc = " << m_cutoff << "\n";
    tmp << "      nosym = " << getNoSymm(m_symm) << "\n";
    tmp << "   smearing = " << getSmearType(m_smearType) << "\n";
    if(m_dgauss > 0)
      tmp << "    degauss = " << m_dgauss << "\n";
    if(m_bravaisType == free)
      tmp << "          A = " << m_latticeA << "\n";
    else
    {
      //a, b and c must be in angstrom
      tmp << "          A = " << m_latticeA << "\n";
      tmp << "          B = " << m_latticeB << "\n";
      tmp << "          C = " << m_latticeC << "\n";
      tmp << "      cosAB = " << cos(m_latticeGamma*2.0*PI/360.0)
        << " !gamma = " << m_latticeGamma << "\n";
      tmp << "      cosAC = " << cos(m_latticeBeta*2.0*PI/360.0)
        << " !beta  = " << m_latticeBeta << "\n";
      tmp << "      cosBC = " << cos(m_latticeAlpha*2.0*PI/360.0)
        << " !alpha = " << m_latticeAlpha << "\n";
    }
    tmp << " /\n";
    //The next two name lists are required even
    //if no keywords are specified.
    tmp << " &electrons\n /\n &ions\n /\n";


    return buffer;
  }

  QString EspressoInputDialog::getControlNameList()
  {
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream tmp(&buffer);
    QString buffer2,buffer3;

    tmp << " &control\n";
    tmp << "    calculation = " << getCalcType(m_calcType) << "\n";
    tmp << "          title = " << "'" << m_title << "'" << "\n";
    tmp << "      verbosity = " << getVerboseType(m_verboseType) << "\n";
    tmp << "   restart_mode = " << getRestartType(m_restartType) << "\n";
    tmp << "        tstress = " << getStress(m_stress) << "\n";
    tmp << "        tprnfor = " << getForces(m_forces) << "\n";
    if(m_calcType == md || m_calcType == vcmd)
    {
      tmp << "             dt = " << m_timeStep << "\n";
      tmp << "          nstep = " << m_nSteps << "\n";
    }
    tmp << "         outdir = " << "'" << m_outdir << "'" << "\n";
    tmp << "         prefix = " << "'" << m_prefix << "'" << "\n";
    if(!m_pseudoEnv)
      tmp << "      pseudodir = " << "'" << m_pseudodir << "'" << "\n";
    tmp << " /\n";

    return buffer;

  }

  QString EspressoInputDialog::generateInputDeck()
  {
    if (!m_molecule || m_molecule->numAtoms() == 0) {
      return QString("");
    }
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);
    QString buffer2,buffer3;

    // Title line
    mol << "# " << m_title << "\n";
    mol << "# espresso input generated by Avogadro\n";

    // If we have a molecule, invent the box it will live in
    // TODO: check the box is big enough: get molecule size and add a bit in
    // each direction, or make it a ui input.

    preparePseudoInput();
    mol << getControlNameList();

    //because &system needs to know the number and types of the atoms,
    //it must be called after the crystal sctructure selector, but
    //it might be better to print it second.
    buffer2 = getCrystalStructure(m_crystalType);
    buffer3 = getSystemNameList();
    mol << buffer3;
    mol << buffer2;

    mol << getKPointInput(m_kpointType);

    return buffer;
  }

  void EspressoInputDialog::deckDirty(bool dirty)
  {
    m_dirty = dirty;

    ui.titleLine->setEnabled(!dirty);

    ui.crystalCombo->setEnabled(!dirty);
    ui.enableFormButton->setEnabled(dirty);
  }

  void EspressoInputDialog::readSettings(QSettings& settings)
  {
    m_savePath = settings.value("espresso/savepath").toString();
  }

  void EspressoInputDialog::writeSettings(QSettings& settings) const
  {
    settings.setValue("espresso/savepath", m_savePath);
  }
  void EspressoInputDialog::initializePseudo(QString label)
  {
    if(atomPseudoFiles.contains(label))
      atomPseudo[label] = atomPseudoFiles[label].at(0);
    else
      atomPseudo[label] = "NotFound";
  }
  void EspressoInputDialog::setPseudoFiles()
  {
    //the following hash table was derived from the
    //pseudopotential archive obtained from
    //http://www.quantum-espresso.org/wp-content/uploads/upf_files/upf_files.tar
    //Downloaded on August 13, 2015
    atomPseudoFiles["Ac"] = QStringList()
      << "Ac.pbe-mt_fhi.UPF"
      << "Ac.pz-mt_fhi.UPF";
    atomPseudoFiles["Ag"] = QStringList()
      << "Ag.pbe-d-rrkjus.UPF"
      << "Ag.pbe-dn-kjpaw_psl.0.1.UPF"
      << "Ag.pbe-dn-rrkjus_psl.0.1.UPF"
      << "Ag.pbe-mt_fhi.UPF"
      << "Ag.pbesol-dn-kjpaw_psl.0.1.UPF"
      << "Ag.pbesol-dn-rrkjus_psl.0.1.UPF"
      << "Ag.pz-d-rrkjus.UPF"
      << "Ag.pz-dn-kjpaw_psl.0.1.UPF"
      << "Ag.pz-dn-rrkjus_psl.0.1.UPF"
      << "Ag.pz-mt_fhi.UPF"
      << "Ag.rel-pbe-dn-kjpaw_psl.0.1.UPF"
      << "Ag.rel-pbe-dn-rrkjus_psl.0.1.UPF"
      << "Ag.rel-pbesol-dn-kjpaw_psl.0.1.UPF"
      << "Ag.rel-pbesol-dn-rrkjus_psl.0.1.UPF"
      << "Ag.rel-pz-dn-kjpaw_psl.0.1.UPF"
      << "Ag.rel-pz-dn-rrkjus_psl.0.1.UPF";
    atomPseudoFiles["Al"] = QStringList()
      << "Al.blyp-n-van_ak.UPF"
      << "Al.bp-n-van_ak.UPF"
      << "Al.pbe-mt_fhi.UPF"
      << "Al.pbe-n-kjpaw_psl.0.1.UPF"
      << "Al.pbe-n-rrkjus_psl.0.1.UPF"
      << "Al.pbe-n-van.UPF"
      << "Al.pbe-rrkj.UPF"
      << "Al.pbe-sp-van.UPF"
      << "Al.pbesol-n-kjpaw_psl.0.1.UPF"
      << "Al.pbesol-n-rrkjus_psl.0.1.UPF"
      << "Al.pw91-n-van.UPF"
      << "Al.pz-mt_fhi.UPF"
      << "Al.pz-n-kjpaw_psl.0.1.UPF"
      << "Al.pz-n-rrkjus_psl.0.1.UPF"
      << "Al.pz-vbc.UPF"
      << "Al.rel-pbe-n-kjpaw_psl.0.2.2.UPF"
      << "Al.rel-pbe-n-rrkjus_psl.0.2.2.UPF"
      << "Al.rel-pbesol-n-kjpaw_psl.0.2.2.UPF"
      << "Al.rel-pbesol-n-rrkjus_psl.0.2.2.UPF"
      << "Al.rel-pz-n-kjpaw_psl.0.2.2.UPF"
      << "Al.rel-pz-n-rrkjus_psl.0.2.2.UPF";
    atomPseudoFiles["Am"] = QStringList()
      << "Am.pbe-mt_fhi.UPF"
      << "Am.pz-mt_fhi.UPF";
    atomPseudoFiles["Ar"] = QStringList()
      << "Ar.pbe-mt_fhi.UPF"
      << "Ar.pbe-n-kjpaw.psl.0.2.3.UPF"
      << "Ar.pbe-n-rrkjus_psl.0.2.3.UPF"
      << "Ar.pbesol-n-kjpaw.psl.0.2.3.UPF"
      << "Ar.pbesol-n-rrkjus_psl.0.2.3.UPF"
      << "Ar.pz-mt_fhi.UPF"
      << "Ar.pz-n-kjpaw.psl.0.2.3.UPF"
      << "Ar.pz-n-rrkjus_psl.0.2.3.UPF"
      << "Ar.pz-rrkj.UPF"
      << "Ar.rel-pbe-n-kjpaw.psl.0.2.3.UPF"
      << "Ar.rel-pbe-n-rrkjus_psl.0.2.3.UPF"
      << "Ar.rel-pbesol-n-kjpaw.psl.0.2.3.UPF"
      << "Ar.rel-pbesol-n-rrkjus_psl.0.2.3.UPF"
      << "Ar.rel-pz-n-kjpaw.psl.0.2.3.UPF"
      << "Ar.rel-pz-n-rrkjus_psl.0.2.3.UPF";
    atomPseudoFiles["As"] = QStringList()
      << "As.pbe-mt_fhi.UPF"
      << "As.pbe-n-kjpaw_psl.0.2.UPF"
      << "As.pbe-n-rrkjus_psl.0.2.UPF"
      << "As.pbe-n-van.UPF"
      << "As.pbesol-n-kjpaw_psl.0.2.UPF"
      << "As.pbesol-n-rrkjus_psl.0.2.UPF"
      << "As.pw91-n-van.UPF"
      << "As.pz-bhs.UPF"
      << "As.pz-mt_fhi.UPF"
      << "As.pz-n-kjpaw_psl.0.2.UPF"
      << "As.pz-n-rrkjus_psl.0.2.UPF"
      << "As.rel-pbe-n-kjpaw_psl.0.2.UPF"
      << "As.rel-pbe-n-rrkjus_psl.0.2.UPF"
      << "As.rel-pbesol-n-kjpaw_psl.0.2.UPF"
      << "As.rel-pbesol-n-rrkjus_psl.0.2.UPF"
      << "As.rel-pz-n-kjpaw_psl.0.2.UPF"
      << "As.rel-pz-n-rrkjus_psl.0.2.UPF";
    atomPseudoFiles["At"] = QStringList()
      << "At.pbe-mt_fhi.UPF"
      << "At.pz-mt_fhi.UPF";
    atomPseudoFiles["Au"] = QStringList()
      << "Au.blyp-van_ak.UPF"
      << "Au.bp-van_ak.UPF"
      << "Au.pbe-dn-kjpaw_psl.0.1.UPF"
      << "Au.pbe-dn-rrkjus_psl.0.1.UPF"
      << "Au.pbe-mt_fhi.UPF"
      << "Au.pbe-nd-rrkjus.UPF"
      << "Au.pbe-nd-van.UPF"
      << "Au.pbe-van_ak.UPF"
      << "Au.pbesol-dn-kjpaw_psl.0.1.UPF"
      << "Au.pbesol-dn-rrkjus_psl.0.1.UPF"
      << "Au.pw91-d-van.UPF"
      << "Au.pw91-van_ak.UPF"
      << "Au.pz-d-rrkjus.UPF"
      << "Au.pz-d-van.UPF"
      << "Au.pz-dn-kjpaw_psl.0.1.UPF"
      << "Au.pz-dn-rrkjus_psl.0.1.UPF"
      << "Au.pz-mt_fhi.UPF"
      << "Au.pz-van_ak.UPF"
      << "Au.rel-pbe-dn-kjpaw_psl.0.1.UPF"
      << "Au.rel-pbe-dn-rrkjus_psl.0.1.UPF"
      << "Au.rel-pbesol-dn-kjpaw_psl.0.1.UPF"
      << "Au.rel-pbesol-dn-rrkjus_psl.0.1.UPF"
      << "Au.rel-pz-dn-kjpaw_psl.0.1.UPF"
      << "Au.rel-pz-dn-rrkjus_psl.0.1.UPF"
      << "Au.rel-pz-kjpaw.UPF"
      << "Au.rel-pz-rrkjus.UPF";
    atomPseudoFiles["B"] = QStringList()
      << "B.blyp-n-van_ak.UPF"
      << "B.bp-n-van_ak.UPF"
      << "B.pbe-mt_fhi.UPF"
      << "B.pbe-n-kjpaw_psl.0.1.UPF"
      << "B.pbe-n-rrkjus_psl.0.1.UPF"
      << "B.pbe-n-van_ak.UPF"
      << "B.pbesol-n-kjpaw_psl.0.1.UPF"
      << "B.pbesol-n-rrkjus_psl.0.1.UPF"
      << "B.pw91-n-van_ak.UPF"
      << "B.pz-bhs.UPF"
      << "B.pz-mt_fhi.UPF"
      << "B.pz-n-kjpaw_psl.0.1.UPF"
      << "B.pz-n-rrkjus_psl.0.1.UPF"
      << "B.pz-vbc.UPF"
      << "B.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "B.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "B.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "B.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "B.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "B.rel-pz-n-rrkjus_psl.0.1.UPF";
    atomPseudoFiles["Ba"] = QStringList()
      << "Ba.pbe-mt_fhi.UPF"
      << "Ba.pbe-nsp-van.UPF"
      << "Ba.pw91-nsp-van.UPF"
      << "Ba.pz-mt_fhi.UPF";
    atomPseudoFiles["Be"] = QStringList()
      << "Be.pbe-mt_fhi.UPF"
      << "Be.pbe-n-kjpaw_psl.0.2.UPF"
      << "Be.pbe-n-rrkjus_psl.0.2.UPF"
      << "Be.pbe-rrkjus.UPF"
      << "Be.pbesol-n-kjpaw_psl.0.2.UPF"
      << "Be.pbesol-n-rrkjus_psl.0.2.UPF"
      << "Be.pz-mt_fhi.UPF"
      << "Be.pz-n-kjpaw_psl.0.2.UPF"
      << "Be.pz-n-rrkjus_psl.0.2.UPF"
      << "Be.pz-n-vbc.UPF"
      << "Be.rel-pbe-n-kjpaw_psl.0.2.UPF"
      << "Be.rel-pbe-n-rrkjus_psl.0.2.UPF"
      << "Be.rel-pbesol-n-kjpaw_psl.0.2.UPF"
      << "Be.rel-pbesol-n-rrkjus_psl.0.2.UPF"
      << "Be.rel-pz-n-kjpaw_psl.0.2.UPF"
      << "Be.rel-pz-n-rrkjus_psl.0.2.UPF"
      << "Be.star1s-pz-n-vbc.UPF";
    atomPseudoFiles["Bh"] = QStringList()
      << "Bh.pbe-mt_fhi.UPF"
      << "Bh.pz-mt_fhi.UPF";
    atomPseudoFiles["Bi"] = QStringList()
      << "Bi.pbe-d-mt.UPF"
      << "Bi.pbe-mt_fhi.UPF"
      << "Bi.pz-mt_fhi.UPF";
    atomPseudoFiles["Bk"] = QStringList()
      << "Bk.pbe-mt_fhi.UPF"
      << "Bk.pz-mt_fhi.UPF";
    atomPseudoFiles["Br"] = QStringList()
      << "Br.pbe-mt_fhi.UPF"
      << "Br.pbe-n-kjpaw_psl.0.2.UPF"
      << "Br.pbe-n-rrkjus_psl.0.2.UPF"
      << "Br.pbe-van_mit.UPF"
      << "Br.pbesol-n-kjpaw_psl.0.2.UPF"
      << "Br.pbesol-n-rrkjus_psl.0.2.UPF"
      << "Br.pw91-mt.UPF"
      << "Br.pz-mt_fhi.UPF"
      << "Br.pz-n-kjpaw_psl.0.2.UPF"
      << "Br.pz-n-rrkjus_psl.0.2.UPF"
      << "Br.pz-vbc.UPF"
      << "Br.rel-pbe-n-kjpaw_psl.0.2.UPF"
      << "Br.rel-pbe-n-rrkjus_psl.0.2.UPF"
      << "Br.rel-pbesol-n-kjpaw_psl.0.2.UPF"
      << "Br.rel-pbesol-n-rrkjus_psl.0.2.UPF"
      << "Br.rel-pz-n-kjpaw_psl.0.2.UPF"
      << "Br.rel-pz-n-rrkjus_psl.0.2.UPF";
    atomPseudoFiles["C"] = QStringList()
      << "C.blyp-mt.UPF"
      << "C.blyp-van_ak.UPF"
      << "C.bp-van_ak.UPF"
      << "C.pbe-mt_fhi.UPF"
      << "C.pbe-mt_gipaw.UPF"
      << "C.pbe-n-kjpaw_psl.0.1.UPF"
      << "C.pbe-n-rrkjus_psl.0.1.UPF"
      << "C.pbe-rrkjus.UPF"
      << "C.pbe-van_ak.UPF"
      << "C.pbe-van_bm.UPF"
      << "C.pbesol-n-kjpaw_psl.0.1.UPF"
      << "C.pbesol-n-rrkjus_psl.0.1.UPF"
      << "C.pw91-van_ak.UPF"
      << "C.pz-kjpaw.UPF"
      << "C.pz-mt_fhi.UPF"
      << "C.pz-n-kjpaw_psl.0.1.UPF"
      << "C.pz-n-rrkjus_psl.0.1.UPF"
      << "C.pz-rrkjus.UPF"
      << "C.pz-van_ak.UPF"
      << "C.pz-vbc.UPF"
      << "C.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "C.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "C.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "C.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "C.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "C.rel-pz-n-rrkjus_psl.0.1.UPF"
      << "C.rel-pz-rrkjus.UPF"
      << "C.star1s-blyp-mt.UPF"
      << "C.star1s-pbe-mt_gipaw.UPF"
      << "C.star1s-pbe-rrkjus.UPF"
      << "C.tpss-mt.UPF";
    atomPseudoFiles["Ca"] = QStringList()
      << "Ca.pbe-mt_fhi.UPF"
      << "Ca.pbe-nsp-van.UPF"
      << "Ca.pw91-nsp-van.UPF"
      << "Ca.pz-mt_fhi.UPF"
      << "Ca.pz-n-vbc.UPF";
    atomPseudoFiles["Cd"] = QStringList()
      << "Cd.pbe-dn-kjpaw_psl.0.2.UPF"
      << "Cd.pbe-dn-rrkjus_psl.0.2.UPF"
      << "Cd.pbe-mt_EM.UPF"
      << "Cd.pbe-mt_fhi.UPF"
      << "Cd.pbe-n-van.UPF"
      << "Cd.pbesol-dn-kjpaw_psl.0.2.UPF"
      << "Cd.pbesol-dn-rrkjus_psl.0.2.UPF"
      << "Cd.pw91-n-van.UPF"
      << "Cd.pz-dn-kjpaw_psl.0.2.UPF"
      << "Cd.pz-dn-rrkjus_psl.0.2.UPF"
      << "Cd.pz-mt_fhi.UPF"
      << "Cd.rel-pbe-dn-kjpaw_psl.0.2.UPF"
      << "Cd.rel-pbe-dn-rrkjus_psl.0.2.UPF"
      << "Cd.rel-pbesol-dn-kjpaw_psl.0.2.UPF"
      << "Cd.rel-pbesol-dn-rrkjus_psl.0.2.UPF"
      << "Cd.rel-pz-dn-kjpaw_psl.0.2.UPF"
      << "Cd.rel-pz-dn-rrkjus_psl.0.2.UPF";
    atomPseudoFiles["Ce"] = QStringList()
      << "Ce.pbe-mt_fhi.UPF"
      << "Ce.pz-mt_fhi.UPF";
    atomPseudoFiles["Cf"] = QStringList()
      << "Cf.pbe-mt_fhi.UPF"
      << "Cf.pz-mt_fhi.UPF";
    atomPseudoFiles["Cl"] = QStringList()
      << "Cl.blyp-mt.UPF"
      << "Cl.pbe-mt_fhi.UPF"
      << "Cl.pbe-n-kjpaw_psl.0.1.UPF"
      << "Cl.pbe-n-rrkjus_psl.0.1.UPF"
      << "Cl.pbe-n-van.UPF"
      << "Cl.pbesol-n-kjpaw_psl.0.1.UPF"
      << "Cl.pbesol-n-rrkjus_psl.0.1.UPF"
      << "Cl.pw91-mt.UPF"
      << "Cl.pz-bhs.UPF"
      << "Cl.pz-mt_fhi.UPF"
      << "Cl.pz-n-kjpaw_psl.0.1.UPF"
      << "Cl.pz-n-rrkjus_psl.0.1.UPF"
      << "Cl.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "Cl.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "Cl.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "Cl.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "Cl.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "Cl.rel-pz-n-rrkjus_psl.0.1.UPF";
    atomPseudoFiles["Cm"] = QStringList()
      << "Cm.pbe-mt_fhi.UPF"
      << "Cm.pz-mt_fhi.UPF";
    atomPseudoFiles["Co"] = QStringList()
      << "Co.pbe-mt_fhi.UPF"
      << "Co.pbe-n-van_gipaw.UPF"
      << "Co.pbe-nd-rrkjus.UPF"
      << "Co.pbe-sp-mt_gipaw.UPF"
      << "Co.pz-mt_fhi.UPF"
      << "Co.pz-nd-rrkjus.UPF"
      << "Co.rel-pz-n-rrkjus.UPF"
      << "Co.star1s-pbe-n-van_gipaw.UPF"
      << "Co.star1s-pbe-sp-mt_gipaw.UPF";
    atomPseudoFiles["Cr"] = QStringList()
      << "Cr.pbe-mt_fhi.UPF"
      << "Cr.pbe-sp-van.UPF"
      << "Cr.pw91-sp-van.UPF"
      << "Cr.pz-mt_fhi.UPF";
    atomPseudoFiles["Cs"] = QStringList()
      << "Cs.pbe-mt_bw.UPF"
      << "Cs.pbe-mt_fhi.UPF"
      << "Cs.pz-mt_fhi.UPF";
    atomPseudoFiles["Cu"] = QStringList()
      << "Cu.blyp-n-van_ak.UPF"
      << "Cu.pbe-d-rrkjus.UPF"
      << "Cu.pbe-dn-kjpaw_psl.0.2.UPF"
      << "Cu.pbe-dn-rrkjus_psl.0.2.UPF"
      << "Cu.pbe-kjpaw.UPF"
      << "Cu.pbe-mt_fhi.UPF"
      << "Cu.pbe-n-van_ak.UPF"
      << "Cu.pbe-n-van_gipaw.UPF"
      << "Cu.pbesol-dn-kjpaw_psl.0.2.UPF"
      << "Cu.pbesol-dn-rrkjus_psl.0.2.UPF"
      << "Cu.pw91-n-van_ak.UPF"
      << "Cu.pz-d-rrkjus.UPF"
      << "Cu.pz-dn-kjpaw_psl.0.2.UPF"
      << "Cu.pz-dn-rrkjus_psl.0.2.UPF"
      << "Cu.pz-mt_fhi.UPF"
      << "Cu.pz-n-van_ak.UPF"
      << "Cu.rel-pbe-dn-kjpaw_psl.0.2.UPF"
      << "Cu.rel-pbe-dn-rrkjus_psl.0.2.UPF"
      << "Cu.rel-pbesol-dn-kjpaw_psl.0.2.UPF"
      << "Cu.rel-pbesol-dn-rrkjus_psl.0.2.UPF"
      << "Cu.rel-pz-dn-kjpaw_psl.0.2.UPF"
      << "Cu.rel-pz-dn-rrkjus_psl.0.2.UPF"
      << "Cu.star1s-pbe-n-van_gipaw.UPF";
    atomPseudoFiles["Db"] = QStringList()
      << "Db.pbe-mt_fhi.UPF"
      << "Db.pz-mt_fhi.UPF";
    atomPseudoFiles["Er"] = QStringList()
      << "Er.pbe-mt_fhi.UPF"
      << "Er.pz-mt_fhi.UPF";
    atomPseudoFiles["Es"] = QStringList()
      << "Es.pbe-mt_fhi.UPF"
      << "Es.pz-mt_fhi.UPF";
    atomPseudoFiles["F"] = QStringList()
      << "F.pbe-mt_fhi.UPF"
      << "F.pbe-n-kjpaw_psl.0.1.UPF"
      << "F.pbe-n-rrkjus_psl.0.1.UPF"
      << "F.pbe-n-van.UPF"
      << "F.pbesol-n-kjpaw_psl.0.1.UPF"
      << "F.pbesol-n-rrkjus_psl.0.1.UPF"
      << "F.pw91-n-van.UPF"
      << "F.pz-mt_fhi.UPF"
      << "F.pz-n-kjpaw_psl.0.1.UPF"
      << "F.pz-n-rrkjus_psl.0.1.UPF"
      << "F.pz-van_asa.UPF"
      << "F.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "F.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "F.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "F.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "F.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "F.rel-pz-n-rrkjus_psl.0.1.UPF";
    atomPseudoFiles["Fe"] = QStringList()
      << "Fe.blyp-sp-van.UPF"
      << "Fe.blyp-sp-van_ak.UPF"
      << "Fe.bp-sp-van_ak.UPF"
      << "Fe.pbe-mt_fhi.UPF"
      << "Fe.pbe-nd-rrkjus.UPF"
      << "Fe.pbe-sp-mt_gipaw.UPF"
      << "Fe.pbe-sp-van.UPF"
      << "Fe.pbe-sp-van_ak.UPF"
      << "Fe.pbe-sp-van_mit.UPF"
      << "Fe.pbe-spn-kjpaw_psl.0.2.1.UPF"
      << "Fe.pbe-spn-rrkjus_psl.0.2.1.UPF"
      << "Fe.pbesol-spn-kjpaw_psl.0.2.1.UPF"
      << "Fe.pbesol-spn-rrkjus_psl.0.2.1.UPF"
      << "Fe.pw91-sp-van_ak.UPF"
      << "Fe.pz-mt_fhi.UPF"
      << "Fe.pz-nd-rrkjus.UPF"
      << "Fe.pz-sp-van_ak.UPF"
      << "Fe.pz-spn-kjpaw_psl.0.2.1.UPF"
      << "Fe.pz-spn-rrkjus_psl.0.2.1.UPF"
      << "Fe.rel-pbe-kjpaw.UPF"
      << "Fe.rel-pbe-spn-kjpaw_psl.0.2.1.UPF"
      << "Fe.rel-pbe-spn-rrkjus_psl.0.2.1.UPF"
      << "Fe.rel-pbesol-spn-kjpaw_psl.0.2.1.UPF"
      << "Fe.rel-pbesol-spn-rrkjus_psl.0.2.1.UPF"
      << "Fe.rel-pz-spn-kjpaw_psl.0.2.1.UPF"
      << "Fe.rel-pz-spn-rrkjus_psl.0.2.1.UPF"
      << "Fe.star1s-pbe-sp-mt_gipaw.UPF";
    atomPseudoFiles["Fm"] = QStringList()
      << "Fm.pbe-mt_fhi.UPF"
      << "Fm.pz-mt_fhi.UPF";
    atomPseudoFiles["Fr"] = QStringList()
      << "Fr.pbe-mt_fhi.UPF"
      << "Fr.pz-mt_fhi.UPF";
    atomPseudoFiles["Ga"] = QStringList()
      << "Ga.pbe-d-mt_fhi.UPF"
      << "Ga.pbe-dn-kjpaw_psl.0.2.UPF"
      << "Ga.pbe-dn-rrkjus_psl.0.2.UPF"
      << "Ga.pbe-mt_fhi.UPF"
      << "Ga.pbe-n-van.UPF"
      << "Ga.pbesol-dn-kjpaw_psl.0.2.UPF"
      << "Ga.pbesol-dn-rrkjus_psl.0.2.UPF"
      << "Ga.pw91-n-van.UPF"
      << "Ga.pz-bhs.UPF"
      << "Ga.pz-d-mt_fhi.UPF"
      << "Ga.pz-dn-kjpaw_psl.0.2.UPF"
      << "Ga.pz-dn-rrkjus_psl.0.2.UPF"
      << "Ga.pz-mt_fhi.UPF"
      << "Ga.rel-pbe-dn-kjpaw_psl.0.2.UPF"
      << "Ga.rel-pbe-dn-rrkjus_psl.0.2.UPF"
      << "Ga.rel-pbesol-dn-kjpaw_psl.0.2.UPF"
      << "Ga.rel-pbesol-dn-rrkjus_psl.0.2.UPF"
      << "Ga.rel-pz-dn-kjpaw_psl.0.2.UPF"
      << "Ga.rel-pz-dn-rrkjus_psl.0.2.UPF";
    atomPseudoFiles["Gd"] = QStringList()
      << "Gd.pbe-mt_fhi.UPF"
      << "Gd.pz-mt_fhi.UPF";
    atomPseudoFiles["Ge"] = QStringList()
      << "Ge.pbe-dn-kjpaw_psl.0.2.2.UPF"
      << "Ge.pbe-dn-rrkjus_psl.0.2.2.UPF"
      << "Ge.pbe-kjpaw.UPF"
      << "Ge.pbe-mt_fhi.UPF"
      << "Ge.pbesol-dn-kjpaw_psl.0.2.2.UPF"
      << "Ge.pbesol-dn-rrkjus_psl.0.2.2.UPF"
      << "Ge.pw91-n-van.UPF"
      << "Ge.pz-bhs.UPF"
      << "Ge.pz-dn-kjpaw_psl.0.2.2.UPF"
      << "Ge.pz-dn-rrkjus_psl.0.2.2.UPF"
      << "Ge.pz-mt_fhi.UPF"
      << "Ge.rel-pbe-dn-kjpaw_psl.0.2.2.UPF"
      << "Ge.rel-pbe-dn-rrkjus_psl.0.2.2.UPF"
      << "Ge.rel-pbesol-dn-kjpaw_psl.0.2.2.UPF"
      << "Ge.rel-pbesol-dn-rrkjus_psl.0.2.2.UPF"
      << "Ge.rel-pz-dn-kjpaw_psl.0.2.2.UPF"
      << "Ge.rel-pz-dn-rrkjus_psl.0.2.2.UPF"
      << "Ge.tpss-n-mt.UPF";
    atomPseudoFiles["H"] = QStringList()
      << "H.blyp-van_ak.UPF"
      << "H.blyp-vbc.UPF"
      << "H.bp-van_ak.UPF"
      << "H.coulomb-ae.UPF"
      << "H.pbe-kjpaw.UPF"
      << "H.pbe-kjpaw_psl.0.1.UPF"
      << "H.pbe-mt_fhi.UPF"
      << "H.pbe-rrkjus.UPF"
      << "H.pbe-rrkjus_psl.0.1.UPF"
      << "H.pbe-van_ak.UPF"
      << "H.pbe-van_bm.UPF"
      << "H.pbe-vbc.UPF"
      << "H.pbesol-kjpaw_psl.0.1.UPF"
      << "H.pbesol-rrkjus_psl.0.1.UPF"
      << "H.pw91-van_ak.UPF"
      << "H.pz-kjpaw.UPF"
      << "H.pz-kjpaw_psl.0.1.UPF"
      << "H.pz-mt_fhi.UPF"
      << "H.pz-rrkjus.UPF"
      << "H.pz-rrkjus_psl.0.1.UPF"
      << "H.pz-van_ak.UPF"
      << "H.pz-vbc.UPF"
      << "H.pz-vbc_050.UPF"
      << "H.pz-vbc_075.UPF"
      << "H.pz-vbc_125.UPF"
      << "H.pz-vbc_150.UPF"
      << "H.rel-pbe-kjpaw_psl.0.1.UPF"
      << "H.rel-pbe-rrkjus_psl.0.1.UPF"
      << "H.rel-pbesol-kjpaw_psl.0.1.UPF"
      << "H.rel-pbesol-rrkjus_psl.0.1.UPF"
      << "H.rel-pz-kjpaw_psl.0.1.UPF"
      << "H.rel-pz-rrkjus_psl.0.1.UPF"
      << "H.tpss-mt.UPF";
    atomPseudoFiles["He"] = QStringList()
      << "He.pbe-mt.UPF"
      << "He.pbe-mt_fhi.UPF"
      << "He.pz-mt.UPF"
      << "He.pz-mt_fhi.UPF";
    atomPseudoFiles["Hf"] = QStringList()
      << "Hf.pbe-mt_fhi.UPF"
      << "Hf.pz-mt_fhi.UPF";
    atomPseudoFiles["Hg"] = QStringList()
      << "Hg.pbe-dn-kjpaw_psl.0.2.2.UPF"
      << "Hg.pbe-dn-rrkjus_psl.0.2.2.UPF"
      << "Hg.pbe-mt_fhi.UPF"
      << "Hg.pbesol-dn-kjpaw_psl.0.2.2.UPF"
      << "Hg.pbesol-dn-rrkjus_psl.0.2.2.UPF"
      << "Hg.pz-dn-kjpaw_psl.0.2.2.UPF"
      << "Hg.pz-dn-rrkjus_psl.0.2.2.UPF"
      << "Hg.pz-mt_fhi.UPF"
      << "Hg.rel-pbe-dn-kjpaw_psl.0.2.2.UPF"
      << "Hg.rel-pbe-dn-rrkjus_psl.0.2.2.UPF"
      << "Hg.rel-pbesol-dn-kjpaw_psl.0.2.2.UPF"
      << "Hg.rel-pbesol-dn-rrkjus_psl.0.2.2.UPF"
      << "Hg.rel-pz-dn-kjpaw_psl.0.2.2.UPF"
      << "Hg.rel-pz-dn-rrkjus_psl.0.2.2.UPF";
    atomPseudoFiles["Hs"] = QStringList()
      << "Hs.pbe-mt_fhi.UPF"
      << "Hs.pz-mt_fhi.UPF";
    atomPseudoFiles["I"] = QStringList()
      << "I.pbe-mt_bw.UPF"
      << "I.pbe-mt_fhi.UPF"
      << "I.pbe-n-kjpaw_psl.0.2.UPF"
      << "I.pbe-n-rrkjus_psl.0.2.UPF"
      << "I.pbesol-n-kjpaw_psl.0.2.UPF"
      << "I.pbesol-n-rrkjus_psl.0.2.UPF"
      << "I.pz-mt_fhi.UPF"
      << "I.pz-n-kjpaw_psl.0.2.UPF"
      << "I.pz-n-rrkjus_psl.0.2.UPF"
      << "I.rel-pbe-n-kjpaw_psl.0.2.2.UPF"
      << "I.rel-pbe-n-rrkjus_psl.0.2.2.UPF"
      << "I.rel-pbesol-n-kjpaw_psl.0.2.2.UPF"
      << "I.rel-pbesol-n-rrkjus_psl.0.2.2.UPF"
      << "I.rel-pz-n-kjpaw_psl.0.2.2.UPF"
      << "I.rel-pz-n-rrkjus_psl.0.2.2.UPF";
    atomPseudoFiles["In"] = QStringList()
      << "In.pbe-d-mt_fhi.UPF"
      << "In.pbe-d-rrkjus.UPF"
      << "In.pbe-dn-kjpaw_psl.0.2.2.UPF"
      << "In.pbe-dn-rrkjus_psl.0.2.2.UPF"
      << "In.pbe-mt_fhi.UPF"
      << "In.pbesol-dn-kjpaw_psl.0.2.2.UPF"
      << "In.pbesol-dn-rrkjus_psl.0.2.2.UPF"
      << "In.pz-bhs.UPF"
      << "In.pz-d-mt_fhi.UPF"
      << "In.pz-d-rrkjus.UPF"
      << "In.pz-dn-kjpaw_psl.0.2.2.UPF"
      << "In.pz-dn-rrkjus_psl.0.2.2.UPF"
      << "In.pz-mt_fhi.UPF"
      << "In.pz-n-bhs.UPF"
      << "In.rel-pbe-dn-kjpaw_psl.0.2.2.UPF"
      << "In.rel-pbe-dn-rrkjus_psl.0.2.2.UPF"
      << "In.rel-pbesol-dn-kjpaw_psl.0.2.2.UPF"
      << "In.rel-pbesol-dn-rrkjus_psl.0.2.2.UPF"
      << "In.rel-pz-dn-kjpaw_psl.0.2.2.UPF"
      << "In.rel-pz-dn-rrkjus_psl.0.2.2.UPF";
    atomPseudoFiles["Ir"] = QStringList()
      << "Ir.pbe-mt_fhi.UPF"
      << "Ir.pbe-n-kjpaw_psl.0.2.3.UPF"
      << "Ir.pbe-n-rrkjus.UPF"
      << "Ir.pbe-n-rrkjus_psl.0.2.3.UPF"
      << "Ir.pbesol-n-kjpaw_psl.0.2.3.UPF"
      << "Ir.pbesol-n-rrkjus_psl.0.2.3.UPF"
      << "Ir.pz-mt_fhi.UPF"
      << "Ir.pz-n-kjpaw_psl.0.2.3.UPF"
      << "Ir.pz-n-rrkjus_psl.0.2.3.UPF"
      << "Ir.rel-pbe-n-kjpaw_psl.0.2.3.UPF"
      << "Ir.rel-pbe-n-rrkjus_psl.0.2.3.UPF"
      << "Ir.rel-pbesol-n-kjpaw_psl.0.2.3.UPF"
      << "Ir.rel-pbesol-n-rrkjus_psl.0.2.3.UPF"
      << "Ir.rel-pz-n-kjpaw_psl.0.2.3.UPF"
      << "Ir.rel-pz-n-rrkjus_psl.0.2.3.UPF";
    atomPseudoFiles["K"] = QStringList()
      << "K.blyp-n-mt.UPF"
      << "K.pbe-mt_fhi.UPF"
      << "K.pbe-n-mt.UPF"
      << "K.pz-mt_fhi.UPF"
      << "K.pz-n-vbc.UPF"
      << "K.pz-sp-van.UPF";
    atomPseudoFiles["Kr"] = QStringList()
      << "Kr.pbe-mt_fhi.UPF"
      << "Kr.pw91-n-van.UPF"
      << "Kr.pz-mt_fhi.UPF";
    atomPseudoFiles["La"] = QStringList()
      << "La.pbe-n-bpaw.UPF"
      << "La.pbe-nsp-van.UPF"
      << "La.pw91-nsp-van.UPF";
    atomPseudoFiles["Li"] = QStringList()
      << "Li.blyp-s-mt.UPF"
      << "Li.blyp-s-van_ak.UPF"
      << "Li.bp-s-mt.UPF"
      << "Li.bp-s-van_ak.UPF"
      << "Li.pbe-mt_fhi.UPF"
      << "Li.pbe-n-van.UPF"
      << "Li.pbe-s-kjpaw_psl.0.2.1.UPF"
      << "Li.pbe-s-mt.UPF"
      << "Li.pbe-s-rrkjus_psl.0.2.1.UPF"
      << "Li.pbe-s-van_ak.UPF"
      << "Li.pbesol-s-kjpaw_psl.0.2.1.UPF"
      << "Li.pbesol-s-rrkjus_psl.0.2.1.UPF"
      << "Li.pw91-n-van.UPF"
      << "Li.pw91-s-mt.UPF"
      << "Li.pw91-s-van_ak.UPF"
      << "Li.pz-mt_fhi.UPF"
      << "Li.pz-n-vbc.UPF"
      << "Li.pz-s-kjpaw_psl.0.2.1.UPF"
      << "Li.pz-s-mt.UPF"
      << "Li.pz-s-rrkjus_psl.0.2.1.UPF"
      << "Li.pz-s-van_ak.UPF"
      << "Li.rel-pbe-s-kjpaw_psl.0.2.1.UPF"
      << "Li.rel-pbe-s-rrkjus_psl.0.2.1.UPF"
      << "Li.rel-pbesol-s-kjpaw_psl.0.2.1.UPF"
      << "Li.rel-pbesol-s-rrkjus_psl.0.2.1.UPF"
      << "Li.rel-pz-s-kjpaw_psl.0.2.1.UPF"
      << "Li.rel-pz-s-rrkjus_psl.0.2.1.UPF";
    atomPseudoFiles["Lr"] = QStringList()
      << "Lr.pbe-mt_fhi.UPF"
      << "Lr.pz-mt_fhi.UPF";
    atomPseudoFiles["Lu"] = QStringList()
      << "Lu.pbe-mt_fhi.UPF"
      << "Lu.pz-mt_fhi.UPF";
    atomPseudoFiles["Md"] = QStringList()
      << "Md.pbe-mt_fhi.UPF"
      << "Md.pz-mt_fhi.UPF";
    atomPseudoFiles["Mg"] = QStringList()
      << "Mg.pbe-mt_fhi.UPF"
      << "Mg.pbe-nsp-bpaw.UPF"
      << "Mg.pw91-np-van.UPF"
      << "Mg.pz-bhs.UPF"
      << "Mg.pz-mt_fhi.UPF"
      << "Mg.pz-n-vbc.UPF";
    atomPseudoFiles["Mn"] = QStringList()
      << "Mn.pbe-mt_fhi.UPF"
      << "Mn.pbe-sp-van.UPF"
      << "Mn.pbe-sp-van_mit.UPF"
      << "Mn.pz-mt_fhi.UPF";
    atomPseudoFiles["Mo"] = QStringList()
      << "Mo.pbe-mt_fhi.UPF"
      << "Mo.pbe-spn-kjpaw_psl.0.2.UPF"
      << "Mo.pbe-spn-rrkjus_psl.0.2.UPF"
      << "Mo.pbesol-spn-kjpaw_psl.0.2.UPF"
      << "Mo.pbesol-spn-rrkjus_psl.0.2.UPF"
      << "Mo.pw91-n-van.UPF"
      << "Mo.pz-mt_fhi.UPF"
      << "Mo.pz-spn-kjpaw_psl.0.2.UPF"
      << "Mo.pz-spn-rrkjus_psl.0.2.UPF"
      << "Mo.rel-pbe-spn-kjpaw_psl.0.2.UPF"
      << "Mo.rel-pbe-spn-rrkjus_psl.0.2.UPF"
      << "Mo.rel-pbesol-spn-kjpaw_psl.0.2.UPF"
      << "Mo.rel-pbesol-spn-rrkjus_psl.0.2.UPF"
      << "Mo.rel-pz-spn-kjpaw_psl.0.2.UPF"
      << "Mo.rel-pz-spn-rrkjus_psl.0.2.UPF";
    atomPseudoFiles["Mt"] = QStringList()
      << "Mt.pbe-mt_fhi.UPF"
      << "Mt.pz-mt_fhi.UPF";
    atomPseudoFiles["N"] = QStringList()
      << "N.blyp-mt.UPF"
      << "N.blyp-van_ak.UPF"
      << "N.bp-van_ak.UPF"
      << "N.pbe-kjpaw.UPF"
      << "N.pbe-mt_fhi.UPF"
      << "N.pbe-n-kjpaw_psl.0.1.UPF"
      << "N.pbe-n-rrkjus_psl.0.1.UPF"
      << "N.pbe-rrkjus.UPF"
      << "N.pbe-van_ak.UPF"
      << "N.pbe-van_bm.UPF"
      << "N.pbesol-n-kjpaw_psl.0.1.UPF"
      << "N.pbesol-n-rrkjus_psl.0.1.UPF"
      << "N.pw91-van_ak.UPF"
      << "N.pz-mt_fhi.UPF"
      << "N.pz-n-kjpaw_psl.0.1.UPF"
      << "N.pz-n-rrkjus_psl.0.1.UPF"
      << "N.pz-rrkjus.UPF"
      << "N.pz-van_ak.UPF"
      << "N.pz-vbc.UPF"
      << "N.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "N.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "N.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "N.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "N.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "N.rel-pz-n-rrkjus_psl.0.1.UPF"
      << "N.star1s-blyp-mt.UPF"
      << "N.star1s-pbe-rrkjus.UPF"
      << "N.star1s-pbe-van.UPF";
    atomPseudoFiles["Na"] = QStringList()
      << "Na.blyp-sp-van_ak.UPF"
      << "Na.bp-sp-van_ak.UPF"
      << "Na.pbe-mt_fhi.UPF"
      << "Na.pbe-n-mt_bw.UPF"
      << "Na.pbe-sp-van_ak.UPF"
      << "Na.pbe-spn-kjpaw_psl.0.2.UPF"
      << "Na.pbe-spn-rrkjus_psl.0.2.UPF"
      << "Na.pbesol-spn-kjpaw_psl.0.2.UPF"
      << "Na.pbesol-spn-rrkjus_psl.0.2.UPF"
      << "Na.pw91-n-mt.UPF"
      << "Na.pw91-sp-van_ak.UPF"
      << "Na.pz-mt_fhi.UPF"
      << "Na.pz-n-vbc.UPF"
      << "Na.pz-sp-van_ak.UPF"
      << "Na.pz-spn-kjpaw_psl.0.2.UPF"
      << "Na.pz-spn-rrkjus_psl.0.2.UPF"
      << "Na.rel-pbe-spn-kjpaw_psl.0.2.UPF"
      << "Na.rel-pbe-spn-rrkjus_psl.0.2.UPF"
      << "Na.rel-pbesol-spn-kjpaw_psl.0.2.UPF"
      << "Na.rel-pbesol-spn-rrkjus_psl.0.2.UPF"
      << "Na.rel-pz-spn-kjpaw_psl.0.2.UPF"
      << "Na.rel-pz-spn-rrkjus_psl.0.2.UPF";
    atomPseudoFiles["Nb"] = QStringList()
      << "Nb.pbe-mt_fhi.UPF"
      << "Nb.pbe-nsp-van.UPF"
      << "Nb.pw91-nsp-van.UPF"
      << "Nb.pz-mt_fhi.UPF"
      << "Nb.pz-sp-van.UPF";
    atomPseudoFiles["Nd"] = QStringList()
      << "Nd.pbe-mt_fhi.UPF"
      << "Nd.pz-mt_fhi.UPF";
    atomPseudoFiles["Ne"] = QStringList()
      << "Ne.pbe-mt_fhi.UPF"
      << "Ne.pz-mt_fhi.UPF";
    atomPseudoFiles["Ni"] = QStringList()
      << "Ni.pbe-mt_fhi.UPF"
      << "Ni.pbe-n-kjpaw_psl.0.1.UPF"
      << "Ni.pbe-n-rrkjus_psl.0.1.UPF"
      << "Ni.pbe-nd-rrkjus.UPF"
      << "Ni.pbe-sp-mt_gipaw.UPF"
      << "Ni.pbesol-n-kjpaw_psl.0.1.UPF"
      << "Ni.pbesol-n-rrkjus_psl.0.1.UPF"
      << "Ni.pz-mt_fhi.UPF"
      << "Ni.pz-n-kjpaw_psl.0.1.UPF"
      << "Ni.pz-n-rrkjus_psl.0.1.UPF"
      << "Ni.pz-nd-rrkjus.UPF"
      << "Ni.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "Ni.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "Ni.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "Ni.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "Ni.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "Ni.rel-pz-n-rrkjus_psl.0.1.UPF"
      << "Ni.star1s-pbe-sp-mt_gipaw.UPF";
    atomPseudoFiles["Np"] = QStringList()
      << "Np.pbe-mt_fhi.UPF"
      << "Np.pz-mt_fhi.UPF";
    atomPseudoFiles["O"] = QStringList()
      << "O.blyp-mt.UPF"
      << "O.blyp-van_ak.UPF"
      << "O.bp-van_ak.UPF"
      << "O.pbe-kjpaw.UPF"
      << "O.pbe-mt.UPF"
      << "O.pbe-mt_fhi.UPF"
      << "O.pbe-n-kjpaw_psl.0.1.UPF"
      << "O.pbe-n-rrkjus_psl.0.1.UPF"
      << "O.pbe-rrkjus.UPF"
      << "O.pbe-van_ak.UPF"
      << "O.pbe-van_bm.UPF"
      << "O.pbe-van_gipaw.UPF"
      << "O.pbesol-n-kjpaw_psl.0.1.UPF"
      << "O.pbesol-n-rrkjus_psl.0.1.UPF"
      << "O.pw91-van_ak.UPF"
      << "O.pz-kjpaw.UPF"
      << "O.pz-mt.UPF"
      << "O.pz-mt_fhi.UPF"
      << "O.pz-n-kjpaw_psl.0.1.UPF"
      << "O.pz-n-rrkjus_psl.0.1.UPF"
      << "O.pz-rrkjus.UPF"
      << "O.pz-van_ak.UPF"
      << "O.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "O.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "O.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "O.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "O.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "O.rel-pz-n-rrkjus_psl.0.1.UPF"
      << "O.rel-pz-rrkjus.UPF"
      << "O.star1s-pbe-van_gipaw.UPF"
      << "O.tpss-mt.UPF";
    atomPseudoFiles["Os"] = QStringList()
      << "Os.pbe-mt_fhi.UPF"
      << "Os.pbe-n-van.UPF"
      << "Os.pw91-n-van.UPF"
      << "Os.pz-mt_fhi.UPF";
    atomPseudoFiles["P"] = QStringList()
      << "P.blyp-mt.UPF"
      << "P.blyp-van_ak.UPF"
      << "P.bp-van_ak.UPF"
      << "P.pbe-mt_fhi.UPF"
      << "P.pbe-n-kjpaw_psl.0.1.UPF"
      << "P.pbe-n-rrkjus_psl.0.1.UPF"
      << "P.pbe-n-van.UPF"
      << "P.pbe-van_ak.UPF"
      << "P.pbesol-n-kjpaw_psl.0.1.UPF"
      << "P.pbesol-n-rrkjus_psl.0.1.UPF"
      << "P.pw91-n-van.UPF"
      << "P.pw91-van_ak.UPF"
      << "P.pz-bhs.UPF"
      << "P.pz-mt_fhi.UPF"
      << "P.pz-n-kjpaw_psl.0.1.UPF"
      << "P.pz-n-rrkjus_psl.0.1.UPF"
      << "P.pz-van_ak.UPF"
      << "P.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "P.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "P.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "P.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "P.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "P.rel-pz-n-rrkjus_psl.0.1.UPF";
    atomPseudoFiles["Pa"] = QStringList()
      << "Pa.pbe-mt_fhi.UPF"
      << "Pa.pz-mt_fhi.UPF";
    atomPseudoFiles["Pb"] = QStringList()
      << "Pb.pbe-d-van.UPF"
      << "Pb.pbe-dn-kjpaw_psl.0.2.2.UPF"
      << "Pb.pbe-dn-rrkjus_psl.0.2.2.UPF"
      << "Pb.pbe-mt_fhi.UPF"
      << "Pb.pbesol-dn-kjpaw_psl.0.2.2.UPF"
      << "Pb.pbesol-dn-rrkjus_psl.0.2.2.UPF"
      << "Pb.pz-bhs.UPF"
      << "Pb.pz-d-van.UPF"
      << "Pb.pz-dn-kjpaw_psl.0.2.2.UPF"
      << "Pb.pz-dn-rrkjus_psl.0.2.2.UPF"
      << "Pb.pz-mt_fhi.UPF"
      << "Pb.rel-pbe-dn-kjpaw_psl.0.2.2.UPF"
      << "Pb.rel-pbe-dn-rrkjus_psl.0.2.2.UPF"
      << "Pb.rel-pbesol-dn-kjpaw_psl.0.2.2.UPF"
      << "Pb.rel-pbesol-dn-rrkjus_psl.0.2.2.UPF"
      << "Pb.rel-pz-dn-kjpaw_psl.0.2.2.UPF"
      << "Pb.rel-pz-dn-rrkjus_psl.0.2.2.UPF";
    atomPseudoFiles["Pd"] = QStringList()
      << "Pd.pbe-mt_fhi.UPF"
      << "Pd.pbe-n-kjpaw_psl.0.2.2.UPF"
      << "Pd.pbe-n-rrkjus_psl.0.2.2.UPF"
      << "Pd.pbe-nd-rrkjus.UPF"
      << "Pd.pbe-rrkjus.UPF"
      << "Pd.pbesol-n-kjpaw_psl.0.2.2.UPF"
      << "Pd.pbesol-n-rrkjus_psl.0.2.2.UPF"
      << "Pd.pz-mt_fhi.UPF"
      << "Pd.pz-n-kjpaw_psl.0.2.2.UPF"
      << "Pd.pz-n-rrkjus_psl.0.2.2.UPF"
      << "Pd.pz-nd-rrkjus.UPF"
      << "Pd.pz-rrkjus.UPF"
      << "Pd.rel-pbe-n-kjpaw_psl.0.2.2.UPF"
      << "Pd.rel-pbe-n-rrkjus_psl.0.2.2.UPF"
      << "Pd.rel-pbesol-n-kjpaw_psl.0.2.2.UPF"
      << "Pd.rel-pbesol-n-rrkjus_psl.0.2.2.UPF"
      << "Pd.rel-pz-n-kjpaw_psl.0.2.2.UPF"
      << "Pd.rel-pz-n-rrkjus_psl.0.2.2.UPF";
    atomPseudoFiles["Pm"] = QStringList()
      << "Pm.pbe-mt_fhi.UPF"
      << "Pm.pz-mt_fhi.UPF";
    atomPseudoFiles["Po"] = QStringList()
      << "Po.pbe-mt_fhi.UPF"
      << "Po.pz-mt_fhi.UPF";
    atomPseudoFiles["Pt"] = QStringList()
      << "Pt.pbe-mt_fhi.UPF"
      << "Pt.pbe-n-kjpaw_psl.0.1.UPF"
      << "Pt.pbe-n-rrkjus_psl.0.1.UPF"
      << "Pt.pbe-n-van.UPF"
      << "Pt.pbe-nd-rrkjus.UPF"
      << "Pt.pbesol-n-kjpaw_psl.0.1.UPF"
      << "Pt.pbesol-n-rrkjus_psl.0.1.UPF"
      << "Pt.pw91-n-van.UPF"
      << "Pt.pz-mt_fhi.UPF"
      << "Pt.pz-n-kjpaw_psl.0.1.UPF"
      << "Pt.pz-n-rrkjus_psl.0.1.UPF"
      << "Pt.pz-nd-rrkjus.UPF"
      << "Pt.pz-rrkjus.UPF"
      << "Pt.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "Pt.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "Pt.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "Pt.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "Pt.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "Pt.rel-pz-n-rrkjus.UPF"
      << "Pt.rel-pz-n-rrkjus_psl.0.1.UPF";
    atomPseudoFiles["Ra"] = QStringList()
      << "Ra.pbe-mt_fhi.UPF"
      << "Ra.pz-mt_fhi.UPF";
    atomPseudoFiles["Rb"] = QStringList()
      << "Rb.pbe-mt_fhi.UPF"
      << "Rb.pz-mt_fhi.UPF";
    atomPseudoFiles["Re"] = QStringList()
      << "Re.pbe-mt_fhi.UPF"
      << "Re.pw91-n-van.UPF"
      << "Re.pz-mt_fhi.UPF";
    atomPseudoFiles["Rf"] = QStringList()
      << "Rf.pbe-mt_fhi.UPF"
      << "Rf.pz-mt_fhi.UPF";
    atomPseudoFiles["Rh"] = QStringList()
      << "Rh.pbe-mt_fhi.UPF"
      << "Rh.pbe-nd-rrkjus.UPF"
      << "Rh.pbe-rrkjus.UPF"
      << "Rh.pbe-spn-kjpaw_psl.0.2.3.UPF"
      << "Rh.pbe-spn-rrkjus_psl.0.2.3.UPF"
      << "Rh.pbesol-spn-kjpaw_psl.0.2.3.UPF"
      << "Rh.pbesol-spn-rrkjus_psl.0.2.3.UPF"
      << "Rh.pz-mt_fhi.UPF"
      << "Rh.pz-rrkjus.UPF"
      << "Rh.pz-spn-kjpaw_psl.0.2.3.UPF"
      << "Rh.pz-spn-rrkjus_psl.0.2.3.UPF"
      << "Rh.rel-pbe-spn-kjpaw_psl.0.2.3.UPF"
      << "Rh.rel-pbe-spn-rrkjus_psl.0.2.3.UPF"
      << "Rh.rel-pbesol-spn-kjpaw_psl.0.2.3.UPF"
      << "Rh.rel-pbesol-spn-rrkjus_psl.0.2.3.UPF"
      << "Rh.rel-pz-spn-kjpaw_psl.0.2.3.UPF"
      << "Rh.rel-pz-spn-rrkjus_psl.0.2.3.UPF";
    atomPseudoFiles["Rn"] = QStringList()
      << "Rn.pbe-mt_fhi.UPF"
      << "Rn.pz-mt_fhi.UPF";
    atomPseudoFiles["Ru"] = QStringList()
      << "Ru.pbe-mt_fhi.UPF"
      << "Ru.pbe-n-van.UPF"
      << "Ru.pw91-n-van.UPF"
      << "Ru.pz-mt_fhi.UPF";
    atomPseudoFiles["S"] = QStringList()
      << "S.blyp-mt.UPF"
      << "S.blyp-van_ak.UPF"
      << "S.bp-van_ak.UPF"
      << "S.pbe-mt_fhi.UPF"
      << "S.pbe-n-kjpaw_psl.0.1.UPF"
      << "S.pbe-n-rrkjus_psl.0.1.UPF"
      << "S.pbe-van_bm.UPF"
      << "S.pbesol-n-kjpaw_psl.0.1.UPF"
      << "S.pbesol-n-rrkjus_psl.0.1.UPF"
      << "S.pw91-n-mt.UPF"
      << "S.pw91-van_ak.UPF"
      << "S.pz-bhs.UPF"
      << "S.pz-mt_fhi.UPF"
      << "S.pz-n-kjpaw_psl.0.1.UPF"
      << "S.pz-n-rrkjus_psl.0.1.UPF"
      << "S.pz-van_ak.UPF"
      << "S.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "S.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "S.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "S.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "S.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "S.rel-pz-n-rrkjus_psl.0.1.UPF";
    atomPseudoFiles["Sb"] = QStringList()
      << "Sb.pbe-mt_fhi.UPF"
      << "Sb.pw91-n-van.UPF"
      << "Sb.pz-bhs.UPF"
      << "Sb.pz-mt_fhi.UPF";
    atomPseudoFiles["Sc"] = QStringList()
      << "Sc.pbe-mt_fhi.UPF"
      << "Sc.pbe-nsp-van.UPF"
      << "Sc.pw91-nsp-van.UPF"
      << "Sc.pz-mt_fhi.UPF";
    atomPseudoFiles["Se"] = QStringList()
      << "Se.pbe-mt_fhi.UPF"
      << "Se.pbe-n-kjpaw_psl.0.2.UPF"
      << "Se.pbe-n-rrkjus_psl.0.2.UPF"
      << "Se.pbe-van.UPF"
      << "Se.pbesol-n-kjpaw_psl.0.2.UPF"
      << "Se.pbesol-n-rrkjus_psl.0.2.UPF"
      << "Se.pz-bhs.UPF"
      << "Se.pz-mt_fhi.UPF"
      << "Se.pz-n-kjpaw_psl.0.2.UPF"
      << "Se.pz-n-rrkjus_psl.0.2.UPF"
      << "Se.rel-pbe-n-kjpaw_psl.0.2.UPF"
      << "Se.rel-pbe-n-rrkjus_psl.0.2.UPF"
      << "Se.rel-pbesol-n-kjpaw_psl.0.2.UPF"
      << "Se.rel-pbesol-n-rrkjus_psl.0.2.UPF"
      << "Se.rel-pz-n-kjpaw_psl.0.2.UPF"
      << "Se.rel-pz-n-rrkjus_psl.0.2.UPF";
    atomPseudoFiles["Sg"] = QStringList()
      << "Sg.pbe-mt_fhi.UPF"
      << "Sg.pz-mt_fhi.UPF";
    atomPseudoFiles["Si"] = QStringList()
      << "Si.pbe-mt_fhi.UPF"
      << "Si.pbe-mt_gipaw.UPF"
      << "Si.pbe-n-kjpaw_psl.0.1.UPF"
      << "Si.pbe-n-rrkjus_psl.0.1.UPF"
      << "Si.pbe-n-van.UPF"
      << "Si.pbe-rrkj.UPF"
      << "Si.pbe-van_gipaw.UPF"
      << "Si.pbesol-n-kjpaw_psl.0.1.UPF"
      << "Si.pbesol-n-rrkjus_psl.0.1.UPF"
      << "Si.pw91-n-van.UPF"
      << "Si.pz-mt_fhi.UPF"
      << "Si.pz-n-kjpaw_psl.0.1.UPF"
      << "Si.pz-n-rrkjus_psl.0.1.UPF"
      << "Si.pz-vbc.UPF"
      << "Si.rel-pbe-n-kjpaw_psl.0.1.UPF"
      << "Si.rel-pbe-n-rrkjus_psl.0.1.UPF"
      << "Si.rel-pbesol-n-kjpaw_psl.0.1.UPF"
      << "Si.rel-pbesol-n-rrkjus_psl.0.1.UPF"
      << "Si.rel-pz-n-kjpaw_psl.0.1.UPF"
      << "Si.rel-pz-n-rrkjus_psl.0.1.UPF"
      << "Si.star1s-pbe-mt_gipaw.UPF"
      << "Si.star1s-pbe-van_gipaw.UPF"
      << "Si.tpss-mt.UPF";
    atomPseudoFiles["Sm"] = QStringList()
      << "Sm.pbe-mt_fhi.UPF"
      << "Sm.pz-mt_fhi.UPF";
    atomPseudoFiles["Sn"] = QStringList()
      << "Sn.pbe-dn-kjpaw_psl.0.2.UPF"
      << "Sn.pbe-dn-rrkjus_psl.0.2.UPF"
      << "Sn.pbe-mt_fhi.UPF"
      << "Sn.pbesol-dn-kjpaw_psl.0.2.UPF"
      << "Sn.pbesol-dn-rrkjus_psl.0.2.UPF"
      << "Sn.pw91-n-van.UPF"
      << "Sn.pz-bhs.UPF"
      << "Sn.pz-dn-kjpaw_psl.0.2.UPF"
      << "Sn.pz-dn-rrkjus_psl.0.2.UPF"
      << "Sn.pz-mt_fhi.UPF"
      << "Sn.rel-pbe-dn-kjpaw_psl.0.2.UPF"
      << "Sn.rel-pbe-dn-rrkjus_psl.0.2.UPF"
      << "Sn.rel-pbesol-dn-kjpaw_psl.0.2.UPF"
      << "Sn.rel-pbesol-dn-rrkjus_psl.0.2.UPF"
      << "Sn.rel-pz-dn-kjpaw_psl.0.2.UPF"
      << "Sn.rel-pz-dn-rrkjus_psl.0.2.UPF";
    atomPseudoFiles["Sr"] = QStringList()
      << "Sr.pbe-mt_fhi.UPF"
      << "Sr.pbe-nsp-van.UPF"
      << "Sr.pbe-sp-van.UPF"
      << "Sr.pw91-nsp-van.UPF"
      << "Sr.pz-mt_fhi.UPF";
    atomPseudoFiles["Ta"] = QStringList()
      << "Ta.pbe-mt_fhi.UPF"
      << "Ta.pbe-nsp-van.UPF"
      << "Ta.pbe-spn-kjpaw_psl.0.2.UPF"
      << "Ta.pbe-spn-rrkjus_psl.0.2.UPF"
      << "Ta.pbesol-spn-kjpaw_psl.0.2.UPF"
      << "Ta.pbesol-spn-rrkjus_psl.0.2.UPF"
      << "Ta.pw91-nsp-van.UPF"
      << "Ta.pz-mt_fhi.UPF"
      << "Ta.pz-spn-kjpaw_psl.0.2.UPF"
      << "Ta.pz-spn-rrkjus_psl.0.2.UPF"
      << "Ta.rel-pbe-spn-kjpaw_psl.0.2.UPF"
      << "Ta.rel-pbe-spn-rrkjus_psl.0.2.UPF"
      << "Ta.rel-pbesol-spn-kjpaw_psl.0.2.UPF"
      << "Ta.rel-pbesol-spn-rrkjus_psl.0.2.UPF"
      << "Ta.rel-pz-spn-kjpaw_psl.0.2.UPF"
      << "Ta.rel-pz-spn-rrkjus_psl.0.2.UPF";
    atomPseudoFiles["Tc"] = QStringList()
      << "Tc.pbe-mt_fhi.UPF"
      << "Tc.pz-mt_fhi.UPF";
    atomPseudoFiles["Te"] = QStringList()
      << "Te.pbe-dn-kjpaw_psl.0.2.2.UPF"
      << "Te.pbe-dn-rrkjus_psl.0.2.2.UPF"
      << "Te.pbe-mt_fhi.UPF"
      << "Te.pbe-rrkj.UPF"
      << "Te.pbesol-dn-kjpaw_psl.0.2.2.UPF"
      << "Te.pbesol-dn-rrkjus_psl.0.2.2.UPF"
      << "Te.pz-bhs.UPF"
      << "Te.pz-dn-kjpaw_psl.0.2.2.UPF"
      << "Te.pz-dn-rrkjus_psl.0.2.2.UPF"
      << "Te.pz-mt_fhi.UPF"
      << "Te.rel-pbe-dn-kjpaw_psl.0.2.2.UPF"
      << "Te.rel-pbe-dn-rrkjus_psl.0.2.2.UPF"
      << "Te.rel-pbesol-dn-kjpaw_psl.0.2.2.UPF"
      << "Te.rel-pbesol-dn-rrkjus_psl.0.2.2.UPF"
      << "Te.rel-pz-dn-kjpaw_psl.0.2.2.UPF"
      << "Te.rel-pz-dn-rrkjus_psl.0.2.2.UPF";
    atomPseudoFiles["Ti"] = QStringList()
      << "Ti.blyp-sp-van_ak.UPF"
      << "Ti.bp-sp-van_ak.UPF"
      << "Ti.pbe-mt_fhi.UPF"
      << "Ti.pbe-sp-van_ak.UPF"
      << "Ti.pw91-nsp-van.UPF"
      << "Ti.pw91-sp-van_ak.UPF"
      << "Ti.pz-mt_fhi.UPF"
      << "Ti.pz-sp-van.UPF"
      << "Ti.pz-sp-van_ak.UPF";
    atomPseudoFiles["Tl"] = QStringList()
      << "Tl.pbe-mt_fhi.UPF"
      << "Tl.pz-d-van.UPF"
      << "Tl.pz-mt_fhi.UPF";
    atomPseudoFiles["Tm"] = QStringList()
      << "Tm.pbe-mt_fhi.UPF"
      << "Tm.pz-mt_fhi.UPF";
    atomPseudoFiles["U"] = QStringList()
      << "U.pbe-mt_fhi.UPF"
      << "U.pz-mt_fhi.UPF";
    atomPseudoFiles["V"] = QStringList()
      << "V.pbe-mt_fhi.UPF"
      << "V.pbe-n-van.UPF"
      << "V.pbe-sp-van.UPF"
      << "V.pz-mt_fhi.UPF";
    atomPseudoFiles["W"] = QStringList()
      << "W.pbe-mt_fhi.UPF"
      << "W.pbe-nsp-van.UPF"
      << "W.pw91-nsp-van.UPF"
      << "W.pz-bhs.UPF"
      << "W.pz-mt_fhi.UPF";
    atomPseudoFiles["Xe"] = QStringList()
      << "Xe.pbe-mt_fhi.UPF"
      << "Xe.pw91-n-van.UPF"
      << "Xe.pz-mt_fhi.UPF";
    atomPseudoFiles["Y"] = QStringList()
      << "Y.pbe-mt_fhi.UPF"
      << "Y.pbe-nsp-van.UPF"
      << "Y.pw91-nsp-van.UPF"
      << "Y.pz-mt_fhi.UPF";
    atomPseudoFiles["Yb"] = QStringList()
      << "Yb.pbe-mt_fhi.UPF"
      << "Yb.pz-mt_fhi.UPF";
    atomPseudoFiles["Zn"] = QStringList()
      << "Zn.blyp-van_ak.UPF"
      << "Zn.bp-van_ak.UPF"
      << "Zn.pbe-mt_fhi.UPF"
      << "Zn.pbe-van.UPF"
      << "Zn.pw91-n-van.UPF"
      << "Zn.pw91-van_ak.UPF"
      << "Zn.pz-mt_fhi.UPF"
      << "Zn.pz-van_ak.UPF";
    atomPseudoFiles["Zr"] = QStringList()
      << "Zr.pbe-mt_fhi.UPF"
      << "Zr.pbe-nsp-van.UPF"
      << "Zr.pw91-nsp-van.UPF"
      << "Zr.pw91-sp-rrkjus.UPF"
      << "Zr.pz-mt_fhi.UPF";
  }
}
