/**********************************************************************
  QChemInputDialog - Dialog for generating Q-Chem input decks

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

#include "qcheminputdialog.h"

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
  QChemInputDialog::QChemInputDialog(QWidget *parent, Qt::WindowFlags f)
    : InputDialog(parent, f), m_calculationType(OPT),
    m_theoryType(B3LYP), m_basisType(B631Gd),
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

    QSettings settings;
    readSettings(settings);
    
    // Generate an initial preview of the input deck
    updatePreviewText();
  }

  QChemInputDialog::~QChemInputDialog()
  {
      QSettings settings;
      writeSettings(settings);
  }

  void QChemInputDialog::setMolecule(Molecule *molecule)
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

  void QChemInputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();
  }

  void QChemInputDialog::updatePreviewText()
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

  void QChemInputDialog::resetClicked()
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

  void QChemInputDialog::generateClicked()
  {
    saveInputFile(ui.previewText->toPlainText(), tr("QChem Input Deck"), QString("qcin"));
  }

  void QChemInputDialog::moreClicked()
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

  void QChemInputDialog::enableFormClicked()
  {
    updatePreviewText();
  }

  void QChemInputDialog::previewEdited()
  {
    // Determine if the preview text has changed from the form generated
    if(ui.previewText->document()->isModified())
      deckDirty(true);
  }

  void QChemInputDialog::setTitle()
  {
    m_title = ui.titleLine->text();
    updatePreviewText();
  }

  void QChemInputDialog::setCalculation(int n)
  {
    m_calculationType = (QChemInputDialog::calculationType) n;
    updatePreviewText();
  }

  void QChemInputDialog::setTheory(int n)
  {
    m_theoryType = (QChemInputDialog::theoryType) n;
    ui.basisCombo->setEnabled(true);

    updatePreviewText();
  }

  void QChemInputDialog::setBasis(int n)
  {
    m_basisType = (QChemInputDialog::basisType) n;
    updatePreviewText();
  }

  void QChemInputDialog::setMultiplicity(int n)
  {
    m_multiplicity = n;
    updatePreviewText();
  }
  void QChemInputDialog::setCharge(int n)
  {
    m_charge = n;
    updatePreviewText();
  }

  void QChemInputDialog::setCoords(int n)
  {
    m_coordType = (QChemInputDialog::coordType) n;
    updatePreviewText();
  }

  QString QChemInputDialog::generateInputDeck()
  {
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);

    // Begin the job specification
    mol << "$rem\n";

    // Now for the calculation type
    mol << "   JOBTYPE " << getCalculationType(m_calculationType) << '\n';

    // Now specify the job type and basis set
    mol << "   EXCHANGE " << getTheoryType(m_theoryType) << '\n';
    mol << "   " << getBasisType(m_basisType) << '\n';

    // Output parameters for some programs
    mol << "   GUI=2\n";

    // End the job spec section
    mol << "$end\n\n";

    // Title line
    mol << "$comment\n" << m_title << "\n$end\n\n";

    // Begin the molecule specification
    mol << "$molecule\n";

    // Now for the charge and multiplicity
    mol << "   " << m_charge << ' ' << m_multiplicity << '\n';

    // Now to output the actual molecular coordinates
    // Cartesian coordinates
    if (m_molecule && m_coordType == CARTESIAN)
    {
      QTextStream mol(&buffer);
      QList<Atom *> atoms = m_molecule->atoms();
      foreach (Atom *atom, atoms) {
        mol << qSetFieldWidth(4) << right
            << QString(OpenBabel::etab.GetSymbol(atom->atomicNumber()))
            << qSetFieldWidth(15) << qSetRealNumberPrecision(5) << forcepoint
            << fixed << right << atom->pos()->x() << atom->pos()->y()
            << atom->pos()->z()
            << qSetFieldWidth(0) << '\n';
      }
    }
    // Z-matrix
    else if (m_molecule && m_coordType == ZMATRIX)
    {
      QTextStream mol(&buffer);
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

        mol << qSetFieldWidth(4) << right
            << QString(etab.GetSymbol(atom->GetAtomicNum())
                       + QString::number(atom->GetIdx()))
            << qSetFieldWidth(0);
        if (atom->GetIdx() > 1)
          mol << ' ' << QString(etab.GetSymbol(a->GetAtomicNum())
                                + QString::number(a->GetIdx()))
              << " r" << atom->GetIdx();
        if (atom->GetIdx() > 2)
          mol << ' ' << QString(etab.GetSymbol(b->GetAtomicNum())
                                + QString::number(b->GetIdx()))
              << " a" << atom->GetIdx();
        if (atom->GetIdx() > 3)
          mol << ' ' << QString(etab.GetSymbol(c->GetAtomicNum())
                                + QString::number(c->GetIdx()))
              << " d" << atom->GetIdx();
        mol << '\n';
      }

      mol << '\n';
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
      foreach (OpenBabel::OBInternalCoord *c, vic)
        delete c;
    }
    else if (m_molecule && m_coordType == ZMATRIX_COMPACT)
    {
      QTextStream mol(&buffer);
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
        r = vic[atom->GetIdx()]->_dst;
        w = vic[atom->GetIdx()]->_ang;
        if (w < 0.0)
          w += 360.0;
        t = vic[atom->GetIdx()]->_tor;
        if (t < 0.0)
          t += 360.0;

        mol << qSetFieldWidth(4) << right
            << QString(etab.GetSymbol(atom->GetAtomicNum())
                       + QString::number(atom->GetIdx()));
        if (atom->GetIdx() > 1)
          mol << qSetFieldWidth(6) << right
              << QString(etab.GetSymbol(a->GetAtomicNum())
                         + QString::number(a->GetIdx())) << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right << r;
        if (atom->GetIdx() > 2)
          mol << qSetFieldWidth(6) << right
                 << QString(etab.GetSymbol(b->GetAtomicNum())
                         + QString::number(b->GetIdx())) << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right << w;
        if (atom->GetIdx() > 3)
          mol << qSetFieldWidth(6) << right
              << QString(etab.GetSymbol(c->GetAtomicNum())
                         + QString::number(c->GetIdx())) << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right << t;
        mol << qSetFieldWidth(0) << '\n';
      }
      foreach (OpenBabel::OBInternalCoord *c, vic)
        delete c;
    }
    mol << "$end\n\n";

    return buffer;
  }

  QString QChemInputDialog::getCalculationType(calculationType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case SP:
        return "SP";
      case OPT:
        return "Opt";
      case FREQ:
        return "Freq";
      default:
        return "SP";
    }
  }

  QString QChemInputDialog::getTheoryType(theoryType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {//   enum theoryType{RHF, B3LYP, B3LYP5, EDF1, M062X, MP2, CCSD}
      case RHF:
        return "RHF";
      case B3LYP:
        return "B3LYP";
      case B3LYP5:
        return "B3LYP5";
      case EDF1:
        return "EDF1";
      case M062X:
        return "M062X";
      case MP2:
        return "HF\n   CORRELATION MP2";
      case CCSD:
        return "HF\n   CORRELATION CCSD";
      default:
        return "RHF";
    }
  }

  QString QChemInputDialog::getBasisType(basisType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case STO3G:
        return "BASIS STO-3G";
      case B321G:
        return "BASIS 3-21G";
      case B631Gd:
        return "BASIS 6-31G(d)";
      case B631Gdp:
        return "BASIS 6-31G(d,p)";
      case B631plusGd:
        return "BASIS 6-31+G(d)";
      case B6311Gd:
        return "BASIS 6-311G(d)";
      case ccpVDZ:
        return "BASIS cc-pVDZ";
      case ccpVTZ:
        return "BASIS cc-pVTZ";
      case LANL2DZ:
        return "ECP LANL2DZ";
      case LACVP:
        return "ECP LACVP";
      default:
        return "6-31G(d)";
    }
  }

  void QChemInputDialog::deckDirty(bool dirty)
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

  void QChemInputDialog::readSettings(QSettings& settings)
  {
    m_savePath = settings.value("qchem/savepath").toString();
  }
  
  void QChemInputDialog::writeSettings(QSettings& settings) const
  {
    settings.setValue("qchem/savepath", m_savePath);
  }
}

