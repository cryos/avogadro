/**********************************************************************
  MOPACInputDialog - Dialog for generating MOPAC input decks

  Copyright (C) 2009 Geoffrey Hutchison
  Some portions Copyright (C) 2008-2009 Marcus Hanwell

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

#include "mopacinputdialog.h"

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
  MOPACInputDialog::MOPACInputDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_molecule(0), m_title("Title"), m_calculationType(OPT),
      m_theoryType(PM6), m_multiplicity(1), m_charge(0),
      m_coordType(CARTESIAN), m_dirty(false), m_warned(false)
  {
    ui.setupUi(this);
    // Connect the GUI elements to the correct slots
    connect(ui.titleLine, SIGNAL(editingFinished()),
            this, SLOT(setTitle()));
    connect(ui.calculationCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setCalculation(int)));
    connect(ui.theoryCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setTheory(int)));
    connect(ui.multiplicityCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setMultiplicity(int)));
    connect(ui.chargeSpin, SIGNAL(valueChanged(int)),
            this, SLOT(setCharge(int)));
    connect(ui.coordCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setCoords(int)));
    connect(ui.previewText, SIGNAL(textChanged()),
            this, SLOT(previewEdited()));
    connect(ui.generateButton, SIGNAL(clicked()),
            this, SLOT(generateClicked()));
    connect(ui.computeButton, SIGNAL(clicked()),
            this, SLOT(computeClicked()));
    connect(ui.resetButton, SIGNAL(clicked()),
            this, SLOT(resetClicked()));
    connect(ui.moreButton, SIGNAL(clicked()),
            this, SLOT(moreClicked()));
    connect(ui.enableFormButton, SIGNAL(clicked()),
            this, SLOT(enableFormClicked()));

    // Generate an initial preview of the input deck
    updatePreviewText();
    ui.previewText->hide();
  }

  MOPACInputDialog::~MOPACInputDialog()
  {
    if (m_molecule)
      disconnect(m_molecule, 0, this, 0);
  }

  void MOPACInputDialog::setMolecule(Molecule *molecule)
  {
    // Disconnect the old molecule first...
    if (m_molecule)
      disconnect(m_molecule, 0, this, 0);

    m_molecule = molecule;
    // Update the preview text whenever primitives are changed
    connect(m_molecule, SIGNAL(primitiveRemoved(Primitive *)),
            this, SLOT(updatePreviewText()));
    connect(m_molecule, SIGNAL(primitiveAdded(Primitive *)),
            this, SLOT(updatePreviewText()));
    connect(m_molecule, SIGNAL(primitiveUpdated(Primitive *)),
            this, SLOT(updatePreviewText()));
    // Add atom coordinates
    updatePreviewText();
  }

  void MOPACInputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();
  }

  void MOPACInputDialog::updatePreviewText()
  {
    if (!isVisible())
      return;
    // Generate the input deck and display it
    if (m_dirty && !m_warned) {
      m_warned = true;
      QMessageBox msgBox;

      msgBox.setWindowTitle(tr("MOPAC Input Warning"));
      msgBox.setText(tr("Would you like to update the preview text, losing all changes made in the MOPAC input deck preview pane?"));
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

      switch (msgBox.exec()) {
      case QMessageBox::Yes:
        // yes was clicked
        m_dirty = false;
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

    if (!m_dirty)
      ui.previewText->setText(generateInputDeck());
  }

  void MOPACInputDialog::resetClicked()
  {
    // Reset the form to defaults
    deckDirty(false);
    QSettings emptySettings;
    readSettings(emptySettings);
  }

  void MOPACInputDialog::generateClicked()
  {
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultFileName = defaultFile.canonicalPath() + "/" + defaultFile.baseName() + ".mop";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save MOPAC Input Deck"),
                                                    defaultFileName, tr("MOPAC Input Deck (*.mop)"));
    QFile file(fileName);
    // FIXME This really should pop up a warning if the file cannot be opened
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;

    QTextStream out(&file);
    out << ui.previewText->toPlainText();
  }

  void MOPACInputDialog::computeClicked()
  {
  }

  void MOPACInputDialog::moreClicked()
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

  void MOPACInputDialog::enableFormClicked()
  {
    updatePreviewText();
  }

  void MOPACInputDialog::previewEdited()
  {
    // Determine if the preview text has changed from the form generated
    if(ui.previewText->toPlainText() != generateInputDeck())
      deckDirty(true);
    else
      deckDirty(false);
  }

  void MOPACInputDialog::setTitle()
  {
    m_title = ui.titleLine->text();
    updatePreviewText();
  }

  void MOPACInputDialog::setCalculation(int n)
  {
    switch (n)
      {
      case 0:
        m_calculationType = SP;
        break;
      case 2:
        m_calculationType = FREQ;
        break;
      case 1:
      default:
        m_calculationType = OPT;
      }
    updatePreviewText();
  }

  void MOPACInputDialog::setTheory(int n)
  {
    switch (n)
      {
      case 0:
        m_theoryType = AM1;
        break;
      case 1:
        m_theoryType = PM3;
        break;
      default:
        m_theoryType = PM6;
      }

    updatePreviewText();
  }

  void MOPACInputDialog::setMultiplicity(int n)
  {
    m_multiplicity = n + 1;
    updatePreviewText();
  }
  void MOPACInputDialog::setCharge(int n)
  {
    m_charge = n;
    updatePreviewText();
  }

  void MOPACInputDialog::setCoords(int n)
  {
    switch (n)
      {
      case 0:
        m_coordType = CARTESIAN;
        break;
      case 1:
        m_coordType = ZMATRIX;
        break;
      default:
        m_coordType = CARTESIAN;
      }
    updatePreviewText();
  }

  QString MOPACInputDialog::generateInputDeck()
  {
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);

    // Begin the job specification
    mol << "$rem\n";

    // Now for the calculation type
    mol << "   JOBTYPE " << getCalculationType(m_calculationType) << "\n";

    // Now specify the job type etc
    mol << "   EXCHANGE " << getTheoryType(m_theoryType) << "\n";

    // End the job spec section
    mol << "$end\n\n";

    // Title line
    mol << "$comment\n" << m_title << "\n$end\n\n";

    // Begin the molecule specification
    mol << "$molecule\n";

    // Now for the charge and multiplicity
    mol << "   " << m_charge << " " << m_multiplicity << "\n";

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
              << qSetFieldWidth(0) << "\n";
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
        vic.push_back((OBInternalCoord*)NULL);
        OpenBabel::OBMol obmol = m_molecule->OBMol();
        FOR_ATOMS_OF_MOL(atom, &obmol)
          vic.push_back(new OBInternalCoord);
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
            mol << qSetFieldWidth(0) << "\n";
          }
      }
    mol << "$end\n\n";

    return buffer;
  }

  QString MOPACInputDialog::getCalculationType(calculationType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
      {
      case SP:
        return "NOOPT";
      case FREQ:
        return "FORCES";
      case OPT:
      default:
        return "";
      }
  }

  QString MOPACInputDialog::getTheoryType(theoryType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
      {
      case AM1:
        return "AM1";
      case MNDO:
        return "MNDO";
      case MNDOD:
        return "MNDOD";
      case PM3:
        return "PM3";
      case RM1:
        return "RM1";
      case PM6:
      default:
        return "PM6";
      }
  }

  void MOPACInputDialog::deckDirty(bool dirty)
  {
    m_dirty = dirty;
    ui.titleLine->setEnabled(!dirty);
    ui.calculationCombo->setEnabled(!dirty);
    ui.theoryCombo->setEnabled(!dirty);
    ui.multiplicityCombo->setEnabled(!dirty);
    ui.chargeSpin->setEnabled(!dirty);
    ui.coordCombo->setEnabled(!dirty);
    ui.enableFormButton->setEnabled(dirty);
  }

  void MOPACInputDialog::writeSettings(QSettings &settings) const
  {
    settings.setValue("MOPACCalcType", ui.calculationCombo->currentIndex());
    settings.setValue("MOPACTheory", ui.theoryCombo->currentIndex());
    settings.setValue("MOPACCoord", ui.coordCombo->currentIndex());
  }

  void MOPACInputDialog::readSettings(QSettings &settings)
  {
    setCalculation(settings.value("MOPACCalcType", 1).toInt());
    ui.calculationCombo->setCurrentIndex(m_calculationType);
    setTheory(settings.value("MOPACTheory", 4).toInt());
    ui.theoryCombo->setCurrentIndex(m_theoryType);
    setCoords(settings.value("MOPACCoord", 0).toInt());
    ui.coordCombo->setCurrentIndex(m_coordType);
  }

}

#include "mopacinputdialog.moc"
