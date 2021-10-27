/**********************************************************************
  GAMESSUKInputDialog - Dialog for generating GAMESS-UK input decks

  Copyright (C) 2010 Jens Thomas
  Copyright (C) 2008-2009 Marcus D. Hanwell
  Copyright (C) 2009 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#include "gamessukinputdialog.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/elements.h>
#include <openbabel/mol.h>

#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

using namespace OpenBabel;

namespace Avogadro
{
  GAMESSUKInputDialog::GAMESSUKInputDialog(QWidget *parent, Qt::WindowFlags f)
    : InputDialog(parent, f), m_calculationType(SP), m_theoryType(RHF),
      m_basisType(p321G), m_dftFunctionalType(B3LYP), m_direct(false),
      m_coordType(CARTESIAN), m_output(), m_dirty(false), m_warned(false)
  {

    // Initialise title here as is in base class
    m_title = "GAMESS-UK input generated by Avogadro";

    ui.setupUi(this);

    // Connect the GUI elements to the correct slots
    connect(ui.titleLine, SIGNAL(editingFinished()),
        this, SLOT(setTitle()));
    connect(ui.calculationCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setCalculation(int)));
    connect(ui.theoryCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setTheory(int)));
    connect(ui.basisCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setBasis(int)));
    connect(ui.dftFunctionalCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setDftFunctional(int)));
    connect(ui.multiplicitySpin, SIGNAL(valueChanged(int)),
        this, SLOT(setMultiplicity(int)));
    connect(ui.chargeSpin, SIGNAL(valueChanged(int)),
        this, SLOT(setCharge(int)));
    connect(ui.coordCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setCoords(int)));
    connect(ui.directCheckBox, SIGNAL(stateChanged(int)),
        this, SLOT(setDirect(int)));
    connect(ui.previewText, SIGNAL(cursorPositionChanged()),
        this, SLOT(previewEdited()));
    connect(ui.generateButton, SIGNAL(clicked()),
        this, SLOT(generateClicked()));
    connect(ui.resetButton, SIGNAL(clicked()),
        this, SLOT(resetClicked()));
    connect(ui.moreButton, SIGNAL(clicked()),
        this, SLOT(moreClicked()));
    connect(ui.enableFormButton, SIGNAL(clicked()),
        this, SLOT(enableFormClicked()));

    QSettings settings;
    readSettings(settings);

    // Generate an initial preview of the input deck
    updatePreviewText();
  }

  GAMESSUKInputDialog::~GAMESSUKInputDialog()
  {
      QSettings settings;
      writeSettings(settings);
  }

  void GAMESSUKInputDialog::setMolecule(Molecule *molecule)
  {
    // Disconnect the old molecule first...
    if (m_molecule)
      disconnect(m_molecule, 0, this, 0);

    m_molecule = molecule;

    // Set multiplicity to the OB value
    OpenBabel::OBMol obmol = m_molecule->OBMol();
    setMultiplicity(obmol.GetTotalSpinMultiplicity());

    // Update the preview text whenever primitives are changed
    connect(m_molecule, SIGNAL(atomRemoved(Atom *)),
            this, SLOT(updatePreviewText()));
    connect(m_molecule, SIGNAL(atomAdded(Atom *)),
            this, SLOT(updatePreviewText()));
    connect(m_molecule, SIGNAL(atomUpdated(Atom *)),
            this, SLOT(updatePreviewText()));
    // Add atom coordinates
    updatePreviewText();
  }

  void GAMESSUKInputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();
  }

  void GAMESSUKInputDialog::updatePreviewText()
  {
    if (!isVisible())
      return;
    // Generate the input deck and display it
    if (m_dirty && !m_warned) {
      m_warned = true;
      QMessageBox msgBox;

      msgBox.setWindowTitle(tr("GAMESS-UK Input Deck Generator Warning"));
      msgBox.setText(tr("Would you like to update the preview text, losing all changes made in the GAMESS-UK input deck preview pane?"));
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

      switch (msgBox.exec()) {
        case QMessageBox::Yes:
          // yes was clicked
          deckDirty(false);
          ui.previewText->setText(generateInputDeck());
          ui.previewText->document()->setModified(false);
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

  void GAMESSUKInputDialog::resetClicked()
  {
    // Reset the form to defaults
    deckDirty(false);
    ui.calculationCombo->setCurrentIndex(0);
    ui.theoryCombo->setCurrentIndex(0);
    ui.coordCombo->setCurrentIndex(0);
    ui.basisCombo->setCurrentIndex(1);
    ui.dftFunctionalCombo->setCurrentIndex(2);
    ui.directCheckBox->setChecked(false);
    ui.multiplicitySpin->setValue(0);
    ui.chargeSpin->setValue(0);
    ui.previewText->setText(generateInputDeck());
    ui.previewText->document()->setModified(false);
  }

  void GAMESSUKInputDialog::generateClicked()
  {
    saveInputFile(ui.previewText->toPlainText(), tr("GAMESS-UK Input Deck"), QString("gukin"));
  }

  void GAMESSUKInputDialog::moreClicked()
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

  void GAMESSUKInputDialog::enableFormClicked()
  {
    updatePreviewText();
  }

  void GAMESSUKInputDialog::previewEdited()
  {
    // Determine if the preview text has changed from the form generated
    if(ui.previewText->document()->isModified())
      deckDirty(true);
  }

  void GAMESSUKInputDialog::setTitle()
  {
    m_title = ui.titleLine->text();
    updatePreviewText();
  }

  void GAMESSUKInputDialog::setCalculation(int n)
  {
    m_calculationType = (GAMESSUKInputDialog::calculationType) n;
    updatePreviewText();
  }

  void GAMESSUKInputDialog::setTheory(int n)
  {
    m_theoryType = (GAMESSUKInputDialog::theoryType) n;
    if (m_theoryType != DFT) {
      ui.dftFunctionalCombo->setEnabled(false);
      ui.dftFunctionalLabel->setEnabled(false);
    } else {
      ui.dftFunctionalCombo->setEnabled(true);
      ui.dftFunctionalLabel->setEnabled(true);
    }
    updatePreviewText();
  }

  void GAMESSUKInputDialog::setDftFunctional(int n)
  {
    m_dftFunctionalType = (GAMESSUKInputDialog::dftFunctionalType) n;
    updatePreviewText();
  }

  void GAMESSUKInputDialog::setBasis(int n)
  {
    m_basisType = (GAMESSUKInputDialog::basisType) n;
    updatePreviewText();
  }

  void GAMESSUKInputDialog::setMultiplicity(int n)
  {
    m_multiplicity = n;
    if (ui.multiplicitySpin->value() != n) {
      ui.multiplicitySpin->setValue(n);
    }
    updatePreviewText();
  }

  void GAMESSUKInputDialog::setCharge(int n)
  {
    m_charge = n;
    updatePreviewText();
  }

  void GAMESSUKInputDialog::setCoords(int n)
  {
    m_coordType = (GAMESSUKInputDialog::coordType) n;
    updatePreviewText();
  }

  void GAMESSUKInputDialog::setDirect(int n)
  {
    if (n){
      m_direct=true;
    } else {
      m_direct=false;
    }
    updatePreviewText();
  }

  QString GAMESSUKInputDialog::generateInputDeck()
  {
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);


    // Give some additional info
    mol << "# This file was generated by Avogadro\n";
    mol << "# For more GAMESS-UK input options consult the manual at:\n";
    mol << "# http://www.cfs.dl.ac.uk/docs/index.shtml\n\n";

    // Title
    mol << "title \n" << m_title << "\n\n";

    // Ensure orbital vectors printed for an optimization
    if (m_calculationType == OPT || m_calculationType == TSS){
      mol << "# Ensure orbital vectors printed after optimisation\n";
      mol << "iprint vectors\n\n";
    }

    // Now for the charge & multiplicty
    mol << "mult " << m_multiplicity << "\n";
    mol << "charge " << m_charge << "\n\n";

    // Geometry specification
    // Now to output the actual molecular coordinates
    // Cartesian coordinates
    if (m_molecule && m_coordType == CARTESIAN)
    {
      // Ensure automatic z-matrix generation is used if we are doing a transiation state search
      if (m_calculationType == TSS)
        mol << "geometry angstrom all\n";
      else
        mol << "geometry angstrom\n";
      QList<Atom *> atoms = m_molecule->atoms();
      foreach (Atom *atom, atoms) {
        mol << qSetFieldWidth(12) << qSetRealNumberPrecision(8) << forcepoint << fixed << right
            << atom->pos()->x() << atom->pos()->y()<< atom->pos()->z()
            << qSetFieldWidth(4) << right << atom->atomicNumber()
            << qSetFieldWidth(4) << right << QString(OpenBabel::OBElements::GetSymbol(atom->atomicNumber()))
            << qSetFieldWidth(0) << '\n';
      }
      // End
      mol << "end\n\n";
    }
    // Z-matrix
    else if (m_molecule && m_coordType == ZMATRIX)
    {
      QTextStream mol(&buffer);
      mol.setFieldAlignment(QTextStream::AlignAccountingStyle);
      mol << "zmatrix angstrom\n";
      OBAtom *a, *b, *c;
      double r, w, t;

      /* Taken from OpenBabel's gzmat file format converter */
      std::vector<OBInternalCoord*> vic;
      vic.push_back((OpenBabel::OBInternalCoord*)NULL);
      OpenBabel::OBMol obmol = m_molecule->OBMol();
      FOR_ATOMS_OF_MOL(atom, &obmol)
        vic.push_back(new OpenBabel::OBInternalCoord);
      CartesianToInternal(vic, obmol);

      FOR_ATOMS_OF_MOL(atom, &obmol)
      {
        a = vic[atom->GetIdx()]->_a;
        b = vic[atom->GetIdx()]->_b;
        c = vic[atom->GetIdx()]->_c;

        mol << qSetFieldWidth(3) << QString(OBElements::GetSymbol(atom->GetAtomicNum()));

        if (atom->GetIdx() > 1)
          mol << qSetFieldWidth(0) << "  " << qSetFieldWidth(3) << QString::number(a->GetIdx())
              << qSetFieldWidth(0) << "  "<< qSetFieldWidth(4) << QString("r") + QString::number(atom->GetIdx());

        if (atom->GetIdx() > 2)
          mol << qSetFieldWidth(0) << "  " << qSetFieldWidth(3) << QString::number(b->GetIdx())
              << qSetFieldWidth(0) << "  "<< qSetFieldWidth(4) << QString("a") + QString::number(atom->GetIdx());

        if (atom->GetIdx() > 3)
          mol << qSetFieldWidth(0) << "  " << qSetFieldWidth(3) << QString::number(c->GetIdx())
              << qSetFieldWidth(0) << "  "<< qSetFieldWidth(4) << QString("d") + QString::number(atom->GetIdx());

        mol << qSetFieldWidth(0) << '\n';
      }

      mol << " variables\n";
      FOR_ATOMS_OF_MOL(atom, &obmol)
      {
        r = vic[atom->GetIdx()]->_dst;
        w = vic[atom->GetIdx()]->_ang;
        if (w < 0.0)
          w += 360.0;
        t = vic[atom->GetIdx()]->_tor;
        if (t < 0.0)
          t += 360.0;
        if (atom->GetIdx() > 1)
          mol << "   r" << atom->GetIdx() << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right
              << r << qSetFieldWidth(0) << '\n';
        if (atom->GetIdx() > 2)
          mol << "   a" << atom->GetIdx() << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right
              << w << qSetFieldWidth(0) << '\n';
        if (atom->GetIdx() > 3)
          mol << "   d" << atom->GetIdx() << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right
              << t << qSetFieldWidth(0) << '\n';
      }
      foreach (OpenBabel::OBInternalCoord *c, vic)
        delete c;
      // End
      mol << "end\n\n";
    }

    // Basis set
    mol << "basis  " << getBasisType(m_basisType) << endl << endl;

    // Set runtype
    mol << getRunType(m_calculationType) << endl;

    // Set scftype
    mol << getScfType(m_theoryType) << endl;

    mol << endl;
    mol << "enter" << endl;

    return buffer;
  }

  QString GAMESSUKInputDialog::getRunType(calculationType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
      {
      case SP:
        return "runtype scf";
      case OPT:
        if ( m_coordType == ZMATRIX )
          return "runtype optimze";
        else
          return "runtype optxyz";
      case TSS:
        return "runtype saddle";
      case FREQ:
        return "runtype hessian";
      default:
        return "runtype scf";
      }
  }

  QString GAMESSUKInputDialog::getScfType(theoryType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
      {
      case RHF:
        {
          if (m_direct)
            return "scftype direct rhf";
          else
            return "scftype rhf";
        }
      case DFT:
        {
          QString dftstr="";
          switch ( m_dftFunctionalType )
            {
            case SVWN:
              dftstr="dft svwn";
              break;
            case BLYP:
              dftstr="dft blyp";
              break;
            case B3LYP:
              dftstr="dft b3lyp";
              break;
            case B97:
              dftstr="dft b97";
              break;
            case HCTH:
              dftstr="dft hcth";
              break;
            case FT97:
              dftstr="dft ft97";
              break;
            default:
              dftstr="dft b3lyp";
            }
          if (m_direct)
            return "scftype direct\n"+dftstr;
          else
            return dftstr;
        }
      case MP2:
        {
          if (m_direct)
            return "scftype direct mp2";
          else
            return "scftype mp2";
        }
      default:
        return "scftype rhf";
      } // End switch
  } // End getScfType

  QString GAMESSUKInputDialog::getBasisType(basisType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case STO3G:
        return "sto3g";
      case p321G:
        return "3-21G";
      case p631G:
        return "6-31G";
      case p631Gs:
        return "6-31G*";
      case ccpVDZ:
        return "cc-pVDZ";
      case ccpVTZ:
        return "cc-pVTZ";
      default:
        return "3-21G";
    }
  }

  void GAMESSUKInputDialog::deckDirty(bool dirty)
  {
    m_dirty = dirty;
    ui.titleLine->setEnabled(!dirty);
    ui.calculationCombo->setEnabled(!dirty);
    ui.theoryCombo->setEnabled(!dirty);
    ui.basisCombo->setEnabled(!dirty);
    ui.multiplicitySpin->setEnabled(!dirty);
    ui.chargeSpin->setEnabled(!dirty);
    ui.enableFormButton->setEnabled(dirty);
  }

  void GAMESSUKInputDialog::readSettings(QSettings& settings)
  {
    m_savePath = settings.value("gamessuk/savepath").toString();
  }

  void GAMESSUKInputDialog::writeSettings(QSettings& settings) const
  {
    settings.setValue("gamessuk/savepath", m_savePath);
  }
}

