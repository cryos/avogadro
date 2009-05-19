/**********************************************************************
  MolproInputDialog - Dialog for generating MOLPRO input decks

  Copyright (C) 2008-2009 Marcus D. Hanwell

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

#include "molproinputdialog.h"

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
  MolproInputDialog::MolproInputDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_molecule(0), m_title("Title"), m_calculationType(OPT),
    m_theoryType(RHF), m_basisType(B631Gd), m_multiplicity(1), m_charge(0),
    m_output(), m_coordType(CARTESIAN), m_dirty(false), m_warned(false)
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
    connect(ui.coordCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setCoords(int)));
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

    // Generate an initial preview of the input deck
    updatePreviewText();
  }

  MolproInputDialog::~MolproInputDialog()
  {
  }

  void MolproInputDialog::setMolecule(Molecule *molecule)
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

  void MolproInputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();
  }

  void MolproInputDialog::updatePreviewText()
  {
    if (!isVisible())
      return;
    // Generate the input deck and display it
    if (m_dirty && !m_warned) {
      m_warned = true;
      QMessageBox msgBox;

      msgBox.setWindowTitle(tr("Q-Chem Input Deck Generator Warning"));
      msgBox.setText(tr("Would you like to update the preview text, losing all changes made in the Q-Chem input deck preview pane?"));
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

  void MolproInputDialog::resetClicked()
  {
    // Reset the form to defaults
    deckDirty(false);
    ui.calculationCombo->setCurrentIndex(1);
    ui.theoryCombo->setCurrentIndex(3);
    ui.basisCombo->setCurrentIndex(2);
    ui.multiplicitySpin->setValue(0);
    ui.chargeSpin->setValue(0);
    ui.previewText->setText(generateInputDeck());
    ui.previewText->document()->setModified(false);
  }

  void MolproInputDialog::generateClicked()
  {
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty())
      defaultPath = QDir::homePath();

    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + ".qcin";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Molpro Input Deck"),
                                defaultFileName, tr("Molpro Input Deck (*.qcin)"));
    QFile file(fileName);
    // FIXME This really should pop up a warning if the file cannot be opened
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;

    QTextStream out(&file);
    out << ui.previewText->toPlainText();
  }

  void MolproInputDialog::moreClicked()
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

  void MolproInputDialog::enableFormClicked()
  {
    updatePreviewText();
  }

  void MolproInputDialog::previewEdited()
  {
    // Determine if the preview text has changed from the form generated
    if(ui.previewText->document()->isModified())
      deckDirty(true);
  }

  void MolproInputDialog::setTitle()
  {
    m_title = ui.titleLine->text();
    updatePreviewText();
  }

  void MolproInputDialog::setCalculation(int n)
  {
    m_calculationType = (MolproInputDialog::calculationType) n;
    updatePreviewText();
  }

  void MolproInputDialog::setTheory(int n)
  {
    m_theoryType = (MolproInputDialog::theoryType) n;
    ui.basisCombo->setEnabled(true);

    updatePreviewText();
  }

  void MolproInputDialog::setBasis(int n)
  {
    m_basisType = (MolproInputDialog::basisType) n;
    updatePreviewText();
  }

  void MolproInputDialog::setMultiplicity(int n)
  {
    m_multiplicity = n;
    updatePreviewText();
  }
  void MolproInputDialog::setCharge(int n)
  {
    m_charge = n;
    updatePreviewText();
  }

  void MolproInputDialog::setCoords(int n)
  {
    m_coordType = (MolproInputDialog::coordType) n;
    updatePreviewText();
  }

  QString MolproInputDialog::generateInputDeck()
  {
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);

    // Begin the job specification, including title
    mol << "*** " << m_title << "\n\n";

    // Default output parameters
    mol << "gprint,basis" << '\n';
    mol << "gprint,orbital" << '\n';

    mol << '\n';

    // Now for the basis set
    mol << "basis, " << getBasisType(m_basisType) << '\n';

    mol << '\n';

    // Now to output the actual molecular coordinates
    // Cartesian coordinates
    if (m_molecule && m_coordType == CARTESIAN)
    {
      mol << "geomtyp=xyz" << '\n';
      mol << "geometry={" << '\n';
      mol << m_molecule->numAtoms() << '\n';
      mol << '\n';
      QList<Atom *> atoms = m_molecule->atoms();
      foreach (Atom *atom, atoms) {
        mol << qSetFieldWidth(2) << left
            << QString(OpenBabel::etab.GetSymbol(atom->atomicNumber()))
            << qSetFieldWidth(15) << qSetRealNumberPrecision(5) << forcepoint
            << fixed << right << atom->pos()->x() << atom->pos()->y()
            << atom->pos()->z()
            << qSetFieldWidth(0) << '\n';
      }
      mol << "}" << '\n';
    }
    // Z-matrix
    else if (m_molecule && m_coordType == ZMATRIX)
    {
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
        r = vic[atom->GetIdx()]->_dst;
        w = vic[atom->GetIdx()]->_ang;
        if (w < 0.0)
          w += 360.0;
        t = vic[atom->GetIdx()]->_tor;
        if (t < 0.0)
          t += 360.0;
        if (atom->GetIdx() > 1)
          mol << "   r" << atom->GetIdx() << " = " << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right
              << r << qSetFieldWidth(0) << '\n';
        if (atom->GetIdx() > 2)
          mol << "   a" << atom->GetIdx() << " = " << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right
              << w << qSetFieldWidth(0) << '\n';
        if (atom->GetIdx() > 3)
          mol << "   d" << atom->GetIdx() << " = " << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right
              << t << qSetFieldWidth(0) << '\n';
      }
      mol << "geometry={" << '\n';
      mol << '\n';
      mol << "nosym" << '\n'; /* FIXME */
      mol << "ang" << '\n';
      FOR_ATOMS_OF_MOL(atom, &obmol)
      {
        a = vic[atom->GetIdx()]->_a;
        b = vic[atom->GetIdx()]->_b;
        c = vic[atom->GetIdx()]->_c;

        mol << QString(etab.GetSymbol(atom->GetAtomicNum()));
        if (atom->GetIdx() > 1)
          mol << ", " << QString::number(a->GetIdx())
              << ", r" << atom->GetIdx();
        if (atom->GetIdx() > 2)
          mol << ", " << QString::number(b->GetIdx())
              << ", a" << atom->GetIdx();
        if (atom->GetIdx() > 3)
          mol << ", " << QString::number(c->GetIdx())
              << ", d" << atom->GetIdx();
        mol << '\n';
      }
      mol << "}" << '\n';
      foreach (OpenBabel::OBInternalCoord *c, vic)
        delete c;
    }
    else if (m_molecule && m_coordType == ZMATRIX_COMPACT)
    {
      QTextStream mol(&buffer);
      OBAtom *a, *b, *c;
      double r, w, t;

      mol << "geometry={" << '\n';
      mol << "nosym" << '\n'; /* FIXME */
      mol << "ang" << '\n';
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
        r = vic[atom->GetIdx()]->_dst;
        w = vic[atom->GetIdx()]->_ang;
        if (w < 0.0)
          w += 360.0;
        t = vic[atom->GetIdx()]->_tor;
        if (t < 0.0)
          t += 360.0;

        mol << QString(etab.GetSymbol(atom->GetAtomicNum()));
        if (atom->GetIdx() > 1)
          mol << ", " << QString::number(a->GetIdx()) << ", "
              << qSetRealNumberPrecision(5) << forcepoint 
	      << fixed << right << r;
        if (atom->GetIdx() > 2)
          mol << ", " << QString::number(b->GetIdx()) << ", "
              << qSetRealNumberPrecision(5) << forcepoint 
	      << fixed << right << w;
        if (atom->GetIdx() > 3)
          mol << ", " << QString::number(c->GetIdx()) << ", " 
              << qSetRealNumberPrecision(5) << forcepoint 
              << fixed << right << t;
        mol << qSetFieldWidth(0) << '\n';
      }
      mol << "}" << '\n';
      foreach (OpenBabel::OBInternalCoord *c, vic)
        delete c;
    }

    mol << '\n';

    // Now specify the job type
    if (m_theoryType != B3LYP) {
      mol << "{" << "rhf" << '\n';
      mol << getWavefunction() << "}\n";
    }
    if (m_theoryType != RHF) {
      mol << "{" << getTheoryType(m_theoryType) << '\n';
      mol << getWavefunction() << "}\n";
    }
    
    mol << '\n';

    // Now for the calculation type
    mol << getCalculationType(m_calculationType);

    mol << "---\n";

    return buffer;
  }

  QString MolproInputDialog::getWavefunction(void)
  {
    QString buffer;
    QTextStream wf(&buffer);
    OpenBabel::OBMol obmol = m_molecule->OBMol();
    int num_electrons;
    int spin;

    num_electrons = -m_charge;
    FOR_ATOMS_OF_MOL(atom, &obmol)
      num_electrons += atom->GetAtomicNum();
    spin = m_multiplicity - 1;

    //  TODO: space symmetry
    wf << "wf," << num_electrons << ",1," << spin;
    return buffer;
  }

  QString MolproInputDialog::getCalculationType(calculationType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case SP:
        return "";
      case OPT:
        return "{optg}\n\n";
      case FREQ:
        return "{optg}\n{frequencies}\n\n";
      default:
        return "";
    }
  }

  QString MolproInputDialog::getTheoryType(theoryType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {//   enum theoryType{RHF, B3LYP, B3LYP5, EDF1, M062X, MP2, CCSD}
      case RHF:
        return "rhf";
      case MP2:
        return "mp2";
      case B3LYP:
        return "uks,b3lyp";
      case CCSD:
        return "ccsd";
      case CCSDT:
        return "ccsd(t)";
      default:
        return "rhf";
    }
  }

  QString MolproInputDialog::getBasisType(basisType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case STO3G:
        return "STO-3G";
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
        return "vdz";
      case ccpVTZ:
        return "vtz";
      case AUGccpVDZ:
        return "avdz";
      case AUGccpVTZ:
        return "avtz";
      default:
        return "6-31G(d)";
    }
  }

  void MolproInputDialog::deckDirty(bool dirty)
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

}

