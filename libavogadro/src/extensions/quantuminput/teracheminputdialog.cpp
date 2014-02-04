/**********************************************************************
  TeraChemInputDialog - Dialog for generating TeraChem input decks

  Copyright (C) 2012 Albert DeFusco
     copied from qmcheminputdialog.cpp

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

#include "teracheminputdialog.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>

#include <QString>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

using namespace OpenBabel;

namespace Avogadro
{
  TeraChemInputDialog::TeraChemInputDialog(QWidget *parent, Qt::WindowFlags f)
    : InputDialog(parent, f),

    m_calculationType(SP),
    m_theoryType(HF),
    m_basisType(STO3G),
    m_coordType(PDB),
    m_dispType(NO),
    m_output(),
    m_unrestricted(false),

    m_dirty(false), m_warned(false)
  {
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
    connect(ui.multiplicitySpin, SIGNAL(valueChanged(int)),
        this, SLOT(setMultiplicity(int)));
    connect(ui.chargeSpin, SIGNAL(valueChanged(int)),
        this, SLOT(setCharge(int)));
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
    connect(ui.checkUnrestricted, SIGNAL(toggled(bool)),
        this, SLOT(setUnrestricted(bool)));
    connect(ui.comboCoord, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setCoordType(int)));
    connect(ui.comboDisp, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setDispType(int)));

    QSettings settings;
    readSettings(settings);

    // Generate an initial preview of the input deck
    updatePreviewText();
  }

  TeraChemInputDialog::~TeraChemInputDialog()
  {
      QSettings settings;
      writeSettings(settings);
  }

  void TeraChemInputDialog::setMolecule(Molecule *molecule)
  {
    // Disconnect the old molecule first...
    if (m_molecule)
      disconnect(m_molecule, 0, this, 0);

    m_molecule = molecule;
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

  void TeraChemInputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();
  }

  void TeraChemInputDialog::updatePreviewText()
  {
    if (!isVisible())
      return;
    // Generate the input deck and display it
    if (m_dirty && !m_warned) {
      m_warned = true;
      QMessageBox msgBox;

      msgBox.setWindowTitle(tr("TeraChem Input Deck Generator Warning"));
      msgBox.setText(tr("Would you like to update the preview text, losing all changes made in the TeraChem input deck preview pane?"));
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

  void TeraChemInputDialog::resetClicked()
  {
    // Reset the form to defaults
    deckDirty(false);
    ui.calculationCombo->setCurrentIndex(0);
    ui.theoryCombo->setCurrentIndex(0);
    ui.basisCombo->setCurrentIndex(0);
    ui.multiplicitySpin->setValue(1);
    ui.checkUnrestricted->setChecked(false);
    ui.chargeSpin->setValue(0);
    ui.previewText->setText(generateInputDeck());
    ui.previewText->document()->setModified(false);
  }

  void TeraChemInputDialog::generateClicked()
  {
    saveInputFile(ui.previewText->toPlainText(), tr("TeraChem Input Deck"), QString("tcin"));
  }

  void TeraChemInputDialog::moreClicked()
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

  void TeraChemInputDialog::enableFormClicked()
  {
    updatePreviewText();
  }

  void TeraChemInputDialog::previewEdited()
  {
    // Determine if the preview text has changed from the form generated
    if(ui.previewText->document()->isModified())
      deckDirty(true);
  }

  void TeraChemInputDialog::setTitle()
  {
    m_title = ui.titleLine->text();
    updatePreviewText();
  }

  void TeraChemInputDialog::setCalculation(int n)
  {
    m_calculationType = (TeraChemInputDialog::calculationType) n;
    updatePreviewText();
  }

  void TeraChemInputDialog::setTheory(int n)
  {
    m_theoryType = (TeraChemInputDialog::theoryType) n;
    ui.basisCombo->setEnabled(true);

    updatePreviewText();
  }

  void TeraChemInputDialog::setBasis(int n)
  {
    m_basisType = (TeraChemInputDialog::basisType) n;
    updatePreviewText();
  }

  void TeraChemInputDialog::setMultiplicity(int n)
  {
    m_multiplicity = n;
    if(m_multiplicity != 1) {
      m_unrestricted = true;
      ui.checkUnrestricted->setChecked(true);
      ui.checkUnrestricted->setEnabled(false);
    }
    else if (m_multiplicity == 1)
      ui.checkUnrestricted->setEnabled(true);

    updatePreviewText();
  }
  void TeraChemInputDialog::setCharge(int n)
  {
    m_charge = n;
    updatePreviewText();
  }

  void TeraChemInputDialog::setUnrestricted(bool n)
  {
    m_unrestricted = n;
    updatePreviewText();
  }

  void TeraChemInputDialog::setCoordType(int n)
  {
    m_coordType = (TeraChemInputDialog::coordType) n;
    updatePreviewText();
  }

  void TeraChemInputDialog::setDispType(int n)
  {
    m_dispType = (TeraChemInputDialog::dispType) n;
    updatePreviewText();
  }

  QString TeraChemInputDialog::generateInputDeck()
  {
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);

    // Title line
    mol << "#\n# " << m_title << "\n#\n\n";

    // Now for the calculation type
    mol << "run            " << getCalculationType(m_calculationType)
      << "\n\n";

    // Now specify the job type and basis set
    mol << "method         " << getTheoryType(m_theoryType) << "\n";
    if(m_dispType != NO)
      mol << "dispersion     " << getDispType(m_dispType) << "\n";
    mol << "basis          " << getBasisType(m_basisType) << "\n";
    // Now for the charge and multiplicity
    mol << "charge         " << m_charge << "\n";
    mol << "spinmul        " << m_multiplicity << "\n\n";

    //mol << "coordinates    " << m_coordFile << "\n\n";
    QFileInfo coordFile(m_molecule->fileName());
    QString coordFileName = coordFile.baseName();
    coordFileName = coordFileName+getCoordType(m_coordType);
    mol << "coordinates    " << coordFileName << "\n\n";

    // End the job spec section
    mol << "\nend\n";

    return buffer;
  }

  QString TeraChemInputDialog::getCalculationType(calculationType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case SP:
        return "energy";
      case GRAD:
        return "gradient";
      case OPT:
        return "minimize";
      default:
        return "energy";
    }
  }

  QString TeraChemInputDialog::getTheoryType(theoryType t)
  {
    // Translate the enum to text for the output generation
    //
    // unrestricted calculation prepend a u to the method
    QString restriction = "";
    QString mDefault = "rhf";
    if(m_unrestricted) {
      restriction = "u";
      mDefault = "uhf";
    }

    switch (t)
    {
      case HF:
        {
          if(m_unrestricted)
            return "uhf";
          else
            return "rhf";
        }
      case BLYP:
        return restriction+"blyp";
      case B3LYP:
        return restriction+"b3lyp";
      case B3LYP1:
        return restriction+"b3lyp1";
      case B3LYP5:
        return restriction+"b3lyp5";
      case PBE:
        return restriction+"pbe";
      case REVPBE:
        return restriction+"revpbe";
      default:
        return mDefault;
    }
  }

  QString TeraChemInputDialog::getBasisType(basisType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case STO3G:
        return "sto-3g";
      case B321G:
        return "3-21G";
      case B631Gd:
        return "6-31G(d)";
      case B631Gdp:
        return "6-31G(d,p)";
      case B631plusGd:
        return "6-31+G(d)";
      case B6311Gd:
        return "6-311G(d)";
      case ccpVDZ:
        return "cc-pVDZ";
      default:
        return "6-31G(d)";
    }
  }

  QString TeraChemInputDialog::getCoordType(coordType t)
  {
    switch (t)
    {
      case PDB:
        return ".pdb";
      case XYZ:
        return ".xyz";
      default:
        return ".pdb";
    }
  }

  QString TeraChemInputDialog::getDispType(dispType t)
  {
    switch (t)
    {
      case NO:
        return "no";
      case YES:
        return "yes";
      case D2:
        return "d2";
      case D3:
        return "d3";
      default:
	return "no";
    }
  }

  void TeraChemInputDialog::deckDirty(bool dirty)
  {
    m_dirty = dirty;
    ui.titleLine->setEnabled(!dirty);
    ui.calculationCombo->setEnabled(!dirty);
    ui.theoryCombo->setEnabled(!dirty);
    ui.basisCombo->setEnabled(!dirty);
    ui.multiplicitySpin->setEnabled(!dirty);
    ui.chargeSpin->setEnabled(!dirty);
    ui.comboCoord->setEnabled(!dirty);
    ui.enableFormButton->setEnabled(dirty);
  }

  void TeraChemInputDialog::readSettings(QSettings& settings)
  {
    m_savePath = settings.value("terachem/savepath").toString();
  }

  void TeraChemInputDialog::writeSettings(QSettings& settings) const
  {
    settings.setValue("terachem/savepath", m_savePath);
  }
}

