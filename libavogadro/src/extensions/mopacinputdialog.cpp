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
using namespace std;

namespace Avogadro
{

#ifdef Q_WS_WIN
    QString mopacPath("C:\Program Files\MOPAC\MOPAC2009.exe");
#else
    QString mopacPath("/opt/mopac/MOPAC2009.exe");
#endif

  MOPACInputDialog::MOPACInputDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_molecule(0), m_title("Title"),
      m_calculationType(OPT),
      m_theoryType(PM6), m_multiplicity(1), m_charge(0),
      m_coordType(CARTESIAN), m_dirty(false), m_warned(false), m_process(0),
      m_progress(0)
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

    QFileInfo info(mopacPath);
    if (!info.exists() || !info.isExecutable()) {
      ui.computeButton->hide();
    } else
      ui.computeButton->show();
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

    if (!m_dirty) {
      ui.previewText->setText(generateInputDeck());
    }
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
    saveInputFile();
  }

  void MOPACInputDialog::computeClicked()
  {
    if (m_process != 0) {
      QMessageBox::warning(this, tr("MOPAC Running."),
                           tr("MOPAC is already running. Wait until the previous calculation is finished."));      
      return;
    }
    
    QString fileName = saveInputFile();
    
    QFileInfo info(mopacPath);
    if (!info.exists() || !info.isExecutable()) {
      QMessageBox::warning(this, tr("MOPAC Not Installed."),
                           tr("The MOPAC executable, cannot be found."));
      return;
    }
    
    m_process = new QProcess(this);
    QFileInfo input(fileName);
    m_process->setWorkingDirectory(input.absolutePath());

    QStringList arguments;
    arguments << fileName;
    m_inputFile = fileName; // save for reading in output

    m_process->start(mopacPath, arguments);
    if (!m_process->waitForStarted()) {
      QMessageBox::warning(this, tr("MOPAC failed to start."),
                           tr("MOPAC did not start. Perhaps it is not installed correctly."));
    }
    connect(m_process, SIGNAL(finished(int)), this, SLOT(finished(int)));
    m_progress = new QProgressDialog(this);
    m_progress->setRange(0,0); // indeterminate progress
    m_progress->setLabelText(tr("Running MOPAC calculation..."));
    m_progress->show();
    connect(m_progress, SIGNAL(canceled()), this, SLOT(stopProcess()));
  }

  void MOPACInputDialog::stopProcess()
  {
    if (m_progress) {
      m_progress->deleteLater();
      m_progress = 0;
    } 

    disconnect(m_process, 0, this, 0); // don't send a "finished" signal
    m_process->close();
    m_process->deleteLater();
    m_process = 0;
  }

  void MOPACInputDialog::finished(int exitCode)
  {
    if (m_progress) {
      m_progress->cancel();
      m_progress->deleteLater();
      m_progress = 0;
    } 

    if (m_process) {
      disconnect(m_process, 0, this, 0);
      m_process->deleteLater();
      m_process = 0;
    } else {
      return; // we probably cancelled
    }

    if (exitCode) {
      QMessageBox::warning(this, tr("MOPAC Crashed."),
                           tr("MOPAC did not run correctly. Perhaps it is not installed correctly."));
     return;
    }
    
    if (!m_molecule)
      return;

    // we have a successful run. Read in the results and close the dialog
    QFileInfo inputFile(m_inputFile);
    QString outputFile = inputFile.canonicalPath() + "/" + inputFile.baseName() + ".out";
    emit readOutput(outputFile);
    
    close();
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
  
  QString MOPACInputDialog::saveInputFile()
  {
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty())
      defaultPath = QDir::homePath();
    
    QString defaultFileName = defaultPath + "/" + defaultFile.baseName() + ".mop";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save MOPAC Input Deck"),
                                                    defaultFileName, tr("MOPAC Input Deck (*.mop)"));
    QFile file(fileName);
    // FIXME This really should pop up a warning if the file cannot be opened
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return "";

    QTextStream out(&file);
    out << ui.previewText->toPlainText();
    
    return fileName;
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
        m_theoryType = MNDO;
        break;
      case 2:
        m_theoryType = MNDOD;
        break;
      case 3:
        m_theoryType = PM3;
        break;
      case 5:
        m_theoryType = RM1;
        break;
      case 4:
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

    mol << " AUX ";
    mol << "CHARGE=" << m_charge << " ";
    switch (m_multiplicity)
      {
      case 2:
        mol << "DOUBLET ";
        break;
      case 3:
        mol << "TRIPLET ";
        break;
      case 4:
        mol << "QUARTET ";
        break;
      case 5:
        mol << "QUINTET ";
        break;
      case 1:
      default:
        mol << "SINGLET ";
      }

    mol << getCalculationType(m_calculationType) << " ";
    mol << getTheoryType(m_theoryType) << "\n";

    mol << m_title << "\n\n";

    // Now to output the actual molecular coordinates
    QString optimizationFlag;
    if (m_calculationType == SP)
      optimizationFlag = " 0 "; // we could actually obey constraints easily
    else
      optimizationFlag = " 1 ";

    // Cartesian coordinates
    if (m_molecule && m_coordType == CARTESIAN)
      {
        QList<Atom *> atoms = m_molecule->atoms();
        foreach (Atom *atom, atoms) {
          mol << qSetFieldWidth(4) << right
              << QString(OpenBabel::etab.GetSymbol(atom->atomicNumber()))
              << qSetFieldWidth(15) << qSetRealNumberPrecision(5) << forcepoint
              << fixed << right 
              << atom->pos()->x() << optimizationFlag 
              << atom->pos()->y() << optimizationFlag 
              << atom->pos()->z() << optimizationFlag 
              << qSetFieldWidth(0) << "\n";
        }
      }
    // Z-matrix
    else if (m_molecule && m_coordType == ZMATRIX)
      {
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
                << QString(etab.GetSymbol(atom->GetAtomicNum()));

            QString buffer = QString("%1 %2 %3 %4 %5 %6")
              .arg(r, 10, 'f', 6)
              .arg(optimizationFlag)
              .arg(w, 10, 'f', 6)
              .arg(optimizationFlag)
              .arg(t, 10, 'f', 6)
              .arg(optimizationFlag);

            mol << buffer;

            int aIndex, bIndex, cIndex;
            aIndex = bIndex = cIndex = 0;

            if (atom->GetIdx() > 1)
              aIndex = a->GetIdx();
            if (atom->GetIdx() > 2)
              bIndex = b->GetIdx();
            if (atom->GetIdx() > 3)
              cIndex = c->GetIdx();

            mol << ' ' << aIndex << ' ' << bIndex << ' ' << cIndex << '\n';
          }
      }
    mol << "\n\n";

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
        return "FORCE";
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
