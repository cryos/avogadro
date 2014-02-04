/**********************************************************************
  Psi4InputDialog - Dialog for generating Psi4 input decks

  Copyright (C) 2012 Matthew R. Kennedy

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

#include "psi4inputdialog.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>

#include <QString>
#include <QTextStream>
//#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

using namespace OpenBabel;

namespace Avogadro
{
  //Defaults for the menus?
  Psi4InputDialog::Psi4InputDialog(QWidget *parent, Qt::WindowFlags f)
  : InputDialog(parent, f), m_calculationType(energy),
  m_theoryType(SAPT0), m_basisType(junDZ),
  m_output(), m_dirty(false), m_warned(false)
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
    connect(ui.enableFormButton, SIGNAL(clicked()),
        this, SLOT(enableFormClicked()));

    QSettings settings;
    readSettings(settings);

    // Generate an initial preview of the input deck
    updatePreviewText();
  }

  Psi4InputDialog::~Psi4InputDialog()
  {
    QSettings settings;
    writeSettings(settings);
  }

  void Psi4InputDialog::setMolecule(Molecule *molecule)
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

  void Psi4InputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();
  }

  void Psi4InputDialog::updatePreviewText()
  {
    if (!isVisible())
      return;
    // Generate the input deck and display it
    if (m_dirty && !m_warned) {
      m_warned = true;
      QMessageBox msgBox;

      msgBox.setWindowTitle(tr("Psi4 Input Deck Generator Warning"));
      msgBox.setText(tr("Would you like to update the preview text, losing all changes made in the Psi4 input deck preview pane?"));
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

  void Psi4InputDialog::resetClicked()
  {
    // Reset the form to defaults
    deckDirty(false);
    ui.calculationCombo->setCurrentIndex(0);
    ui.theoryCombo->setCurrentIndex(0);
    ui.basisCombo->setCurrentIndex(0);
    ui.multiplicitySpin->setValue(0);
    ui.chargeSpin->setValue(0);
    ui.previewText->setText(generateInputDeck());
    ui.previewText->document()->setModified(false);
  }

  void Psi4InputDialog::generateClicked()
  {
    /*QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty())
      defaultPath = QDir::homePath();

    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + ".in";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Psi4 Input Deck"),
                                defaultFileName, tr("Psi4 Input Deck (*.in)"));
    QFile file(fileName);
    // FIXME This really should pop up a warning if the file cannot be opened
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;

    QTextStream out(&file);
    out << ui.previewText->toPlainText();*/
    saveInputFile(ui.previewText->toPlainText(), tr("Psi4 Input Deck"), QString("in"));
  }

  void Psi4InputDialog::enableFormClicked()
  {
    updatePreviewText();
  }

  void Psi4InputDialog::previewEdited()
  {
    // Determine if the preview text has changed from the form generated
    if(ui.previewText->document()->isModified())
      deckDirty(true);
  }

  void Psi4InputDialog::setTitle()
  {
    m_title = ui.titleLine->text();
    updatePreviewText();
  }

  void Psi4InputDialog::setCalculation(int n)
  {
    m_calculationType = (Psi4InputDialog::calculationType) n;
    updatePreviewText();
  }

  void Psi4InputDialog::setTheory(int n)
  {
    m_theoryType = (Psi4InputDialog::theoryType) n;
    ui.basisCombo->setEnabled(true);

    updatePreviewText();
  }

  void Psi4InputDialog::setBasis(int n)
  {
    m_basisType = (Psi4InputDialog::basisType) n;
    updatePreviewText();
  }

  void Psi4InputDialog::setMultiplicity(int n)
  {
    m_multiplicity = n;
    updatePreviewText();
  }
  void Psi4InputDialog::setCharge(int n)
  {
    m_charge = n;
    updatePreviewText();
  }

  QString Psi4InputDialog::generateInputDeck()
  {
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);

    // Begin the job specification
    mol << "set basis " << getBasisType(m_basisType) << "\n";

    mol << "molecule {\n";
    mol << m_charge << " " << m_multiplicity << "\n";
    QList<Atom *> atoms = m_molecule->atoms();
    foreach (Atom *atom, atoms) {
      mol << qSetFieldWidth(4) << right
        << QString(OpenBabel::etab.GetSymbol(atom->atomicNumber()))
        << qSetFieldWidth(15) << qSetRealNumberPrecision(5) << forcepoint
        << fixed << right << atom->pos()->x() << atom->pos()->y()
        << atom->pos()->z()
        << qSetFieldWidth(0) << '\n';
    }
    mol << "}\n";
    if(getTheoryType(m_theoryType) == "sapt0" || getTheoryType(m_theoryType) == "sapt2")
      mol << "auto_fragments('')\n";
    mol << getCalculationType(m_calculationType) << "('" << getTheoryType(m_theoryType) << "')\n";

    return buffer;
  }

  QString Psi4InputDialog::getCalculationType(calculationType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case energy:
        return "energy";
      case optimize:
        return "optimize";
      case frequencies:
        return "frequencies";
      default:
        return "energy";
    }
  }

  QString Psi4InputDialog::getTheoryType(theoryType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case SCF:
        return "scf";
      case SAPT0:
        return "sapt0";
      case SAPT2:
        return "sapt2";
      case B3LYPD:
        return "B3LYP-D";
      case M052X:
        return "m05-2x";
      case B97D:
        return "B97-D";
      case MP2:
        return "MP2";
      case CCSD:
        return "CCSD";
      case CCSDT:
        return "CCSD(T)";
      default:
        return "sapt0";
    }
  }

  QString Psi4InputDialog::getBasisType(basisType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case STO3G:
        return " STO-3G";
      case junDZ:
        return " jun-cc-pVDZ";
      case ccpVDZ:
        return " cc-pVDZ";
      case augccpVDZ:
        return " aug-cc-pVDZ";
      case ccpVTZ:
        return " cc-pVTZ";
      default:
        return " jun-cc-pVDZ";
    }
  }

  void Psi4InputDialog::deckDirty(bool dirty)
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

  void Psi4InputDialog::readSettings(QSettings& settings)
  {
    m_savePath = settings.value("psi4/savepath").toString();
  }

  void Psi4InputDialog::writeSettings(QSettings& settings) const
  {
    settings.setValue("psi4/savepath", m_savePath);
  }
}

