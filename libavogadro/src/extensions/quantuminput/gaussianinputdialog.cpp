/**********************************************************************
  GaussianInputDialog - Dialog for generating Gaussian input decks

  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Michael Banck

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

#include "gaussianinputdialog.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/elements.h>
#include <openbabel/mol.h>

#include <QString>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QShowEvent>
#include <QSettings>
#include <QDebug>
#include <QProcess>

using namespace OpenBabel;

namespace Avogadro
{

  GaussianInputDialog::GaussianInputDialog(QWidget *parent, Qt::WindowFlags f)
    : InputDialog(parent, f), m_calculationType(OPT),
    m_theoryType(B3LYP), m_basisType(B631Gd),
    m_procs(1), m_output(""), m_chk(false), m_coordType(CARTESIAN),
    m_dirty(false), m_warned(false), m_process(0), m_progress(0)
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
    connect(ui.procSpin, SIGNAL(valueChanged(int)),
        this, SLOT(setProcs(int)));
    connect(ui.outputCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setOutput(int)));
    connect(ui.checkpointCheck, SIGNAL(stateChanged(int)),
        this, SLOT(setChk(int)));
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

    QSettings settings;
    readSettings(settings);

    // Generate an initial preview of the input deck
    updatePreviewText();
  }

  GaussianInputDialog::~GaussianInputDialog()
  {
      QSettings settings;
      writeSettings(settings);
  }

  void GaussianInputDialog::writeSettings(QSettings &settings) const
  {
    settings.setValue("gaussian/CalcType", ui.calculationCombo->currentIndex());
    settings.setValue("gaussian/Procs", ui.procSpin->value());
    settings.setValue("gaussian/Theory", ui.theoryCombo->currentIndex());
    settings.setValue("gaussian/Basis", ui.basisCombo->currentIndex());
    settings.setValue("gaussian/Output", ui.outputCombo->currentIndex());
    settings.setValue("gaussian/Chk", ui.checkpointCheck->isChecked());
    settings.setValue("gaussian/Coord", ui.coordCombo->currentIndex());
    settings.setValue("gaussian/savepath", m_savePath);
  }

  void GaussianInputDialog::readSettings(QSettings &settings)
  {
    setProcs(settings.value("gaussian/Procs", 2).toInt());
    ui.procSpin->setValue(settings.value("gaussian/Procs", 1).toInt());
    setCalculation(settings.value("gaussian/CalcType", 1).toInt());
    ui.calculationCombo->setCurrentIndex(settings.value("gaussian/CalcType", 1).toInt());
    setTheory(settings.value("gaussian/Theory", 3).toInt());
    ui.theoryCombo->setCurrentIndex(settings.value("gaussian/Theory", 3).toInt());
    setBasis(settings.value("gaussian/Basis", 2).toInt());
    ui.basisCombo->setCurrentIndex(settings.value("gaussian/Basis", 2).toInt());
    setOutput(settings.value("gaussian/Output", 0).toInt());
    ui.outputCombo->setCurrentIndex(settings.value("gaussian/Output", 0).toInt());
    setChk(settings.value("gaussian/Chk", false).toBool());
    ui.checkpointCheck->setChecked(settings.value("gaussian/Chk", false).toBool());
    setCoords(settings.value("gaussian/Coord", 0).toInt());
    ui.coordCombo->setCurrentIndex(settings.value("gaussian/Coord", 0).toInt());
    m_savePath = settings.value("gaussian/savepath").toString();
  }

  void GaussianInputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();

    if (pathToG03().isEmpty())
      ui.computeButton->hide();
    else
      ui.computeButton->show();
  }

  QString GaussianInputDialog::pathToG03() const
  {
    QString returnPath;
    QStringList pathList;

    QStringList environment = QProcess::systemEnvironment();
    // This is a pain
    // Each item in the list is a key-value pair
    // so we match PATH
    // and then we split out the value (the bit after the =)
    // and split the PATH by ':' characters
    foreach(const QString &key, environment) {
      if (key.startsWith(QLatin1String("PATH")))
        pathList = key.split('=').at(1).split(':');
    }

    // Add default G03 and G09 directories
    pathList << "/usr/local/g03" << "/usr/local/g09";

    // I don't know how this works for Windows -- probably need a different method
    foreach(const QString &path, pathList) {
      QFileInfo g03(path + '/' + "g03");
      if (g03.exists() && g03.isExecutable())
        returnPath = g03.canonicalFilePath();
      QFileInfo g09(path + '/' + "g09");
      if (g09.exists() && g09.isExecutable())
        returnPath = g09.canonicalFilePath();
    }

    return returnPath;
  }

  void GaussianInputDialog::setMolecule(Molecule *molecule)
  {
    // Disconnect the old molecule first...
    if (m_molecule) {
      disconnect(m_molecule, 0, this, 0);
    }

    m_molecule = molecule;
    // Update the preview text whenever atoms are changed
    connect(m_molecule, SIGNAL(atomRemoved(Atom *)),
            this, SLOT(updatePreviewText()));
    connect(m_molecule, SIGNAL(atomAdded(Atom *)),
            this, SLOT(updatePreviewText()));
    connect(m_molecule, SIGNAL(atomUpdated(Atom *)),
            this, SLOT(updatePreviewText()));
    // Add atom coordinates
    updatePreviewText();
  }

  void GaussianInputDialog::updatePreviewText()
  {
    if (!isVisible())
      return;
    // Generate the input deck and display it
    if (m_dirty && !m_warned) {
      m_warned = true;
      QMessageBox msgBox;

      msgBox.setWindowTitle(tr("Gaussian Input Deck Generator Warning"));
      msgBox.setText(tr("Would you like to update the preview text, losing all changes made in the Gaussian input deck preview pane?"));
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
    else if (!m_dirty)
      ui.previewText->setText(generateInputDeck());
  }

  void GaussianInputDialog::resetClicked()
  {
    // Reset the form to defaults
    deckDirty(false);
    ui.calculationCombo->setCurrentIndex(1);
    ui.theoryCombo->setCurrentIndex(3);
    ui.basisCombo->setCurrentIndex(2);
    ui.multiplicitySpin->setValue(0);
    ui.chargeSpin->setValue(0);
    ui.procSpin->setValue(1);
  }

  QString GaussianInputDialog::saveInputFile(QString inputDeck, QString fileType, QString ext)
  {
// Fragment copied from InputDialog
    // Try to set default save path for dialog using the next sequence:
    //  1) directory of current file (if any);
    //  2) directory where previous deck was saved;
    //  3) $HOME
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if(m_savePath == "") {
      if (defaultPath.isEmpty())
        defaultPath = QDir::homePath();
    } else {
      defaultPath = m_savePath;
    }

    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + "." + ext;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Input Deck"),
        defaultFileName, fileType + " (*." + ext + ")");

    if(fileName == "")
      return fileName;

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) return QString();
// end of copied

    // checkpoint
    QString checkpointName = QFileInfo(fileName).baseName();
    checkpointName.prepend("%Chk=");
    checkpointName.append(".chk");
    inputDeck.replace(QLatin1String("%Chk=checkpoint.chk"), checkpointName, Qt::CaseInsensitive);

// Fragment copied from InputDialog
    file.write(inputDeck.toLocal8Bit()); // prevent troubles in Windows
    file.close(); // flush buffer!
    m_savePath = QFileInfo(file).absolutePath();
    return fileName;
  }

  void GaussianInputDialog::generateClicked()
  {
    saveInputFile(ui.previewText->toPlainText(),
                          tr("Gaussian Input Deck"), QString("com"));
  }

  void GaussianInputDialog::computeClicked()
  {
    if (m_process != 0) {
      QMessageBox::warning(this, tr("Gaussian Running."),
                           tr("Gaussian is already running. Wait until the previous calculation is finished."));
      return;
    }

    QString fileName = saveInputFile(ui.previewText->toPlainText(),
                          tr("Gaussian Input Deck"), QString("com"));
    if (fileName.isEmpty())
      return;

    QFileInfo info(pathToG03());
    if (!info.exists() || !info.isExecutable()) {
      QMessageBox::warning(this, tr("Gaussian Not Installed."),
                           tr("The G03 executable, cannot be found."));
      return;
    }

    m_process = new QProcess(this);
    QFileInfo input(fileName);
    m_process->setWorkingDirectory(input.absolutePath());

    QStringList arguments;
    arguments << fileName;
    m_inputFile = fileName; // save for reading in output

    m_process->start(pathToG03(), arguments);
    if (!m_process->waitForStarted()) {
      QMessageBox::warning(this, tr("G03 failed to start."),
                           tr("G03 did not start. Perhaps it is not installed correctly."));
    }
    connect(m_process, SIGNAL(finished(int)), this, SLOT(finished(int)));
    m_progress = new QProgressDialog(this);
    m_progress->setRange(0,0); // indeterminate progress
    m_progress->setLabelText(tr("Running Gaussian calculation..."));
    m_progress->show();
    connect(m_progress, SIGNAL(canceled()), this, SLOT(stopProcess()));
  }

  void GaussianInputDialog::stopProcess()
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

  void GaussianInputDialog::finished(int exitCode)
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
      QMessageBox::warning(this, tr("G03 Crashed."),
                           tr("Gaussian did not run correctly. Perhaps it is not installed correctly."));
     return;
    }

    if (!m_molecule)
      return;

    // we have a successful run.
    // try to run formchk
    QFileInfo inputFile(m_inputFile);
    QString checkpointFileName = inputFile.canonicalPath() + '/' + inputFile.baseName() + ".chk";
    QFileInfo checkpointFile(checkpointFileName);
    if (checkpointFile.exists() && checkpointFile.isReadable()) {
      // let's see if formchk exists
      QString formchkFilePath = QFileInfo(pathToG03()).canonicalPath() + '/' + "formchk";
      QFileInfo formchkInfo(formchkFilePath);
      if (formchkInfo.exists() && formchkInfo.isExecutable()) {
        QStringList arguments;
        arguments << checkpointFileName;
        QProcess::execute(formchkFilePath, arguments); // if this fails, it's not a big deal
      }
    }

    // Now, read in the results and close the dialog
    QString outputFile = inputFile.canonicalPath() + '/' + inputFile.baseName() + ".log";
    emit readOutput(outputFile);

    close();
  }

  void GaussianInputDialog::moreClicked()
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

  void GaussianInputDialog::enableFormClicked()
  {
    deckDirty(false);
    updatePreviewText();
  }

  void GaussianInputDialog::previewEdited()
  {
    // Determine if the preview text has changed from the form generated
    if(ui.previewText->toPlainText() != generateInputDeck())
      deckDirty(true);
    else
      deckDirty(false);
  }

  void GaussianInputDialog::setTitle()
  {
    m_title = ui.titleLine->text();
    updatePreviewText();
  }

  void GaussianInputDialog::setCalculation(int n)
  {
    switch (n)
    {
      case 0:
        m_calculationType = SP;
        break;
      case 1:
        m_calculationType = OPT;
        break;
      case 2:
        m_calculationType = FREQ;
        break;
      default:
        m_calculationType = SP;
    }
    updatePreviewText();
  }

  void GaussianInputDialog::setTheory(int n)
  {
    switch (n)
    {
      case 0:
        m_theoryType = AM1;
        break;
      case 1:
        m_theoryType = PM3;
        break;
      case 2:
        m_theoryType = RHF;
        break;
      case 3:
        m_theoryType = B3LYP;
        break;
      case 4:
        m_theoryType = MP2;
        break;
      case 5:
        m_theoryType = CCSD;
        break;
      default:
        m_theoryType = RHF;
    }

    if (m_theoryType == AM1 || m_theoryType == PM3)
      ui.basisCombo->setEnabled(false);
    else
      ui.basisCombo->setEnabled(true);

    updatePreviewText();
  }

  void GaussianInputDialog::setBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_basisType = STO3G;
        break;
      case 1:
        m_basisType = B321G;
        break;
      case 2:
        m_basisType = B631Gd;
        break;
      case 3:
        m_basisType = B631Gdp;
        break;
      case 4:
        m_basisType = LANL2DZ;
        break;
      default:
        m_basisType = B631Gd;
    }
    updatePreviewText();
  }

  void GaussianInputDialog::setMultiplicity(int n)
  {
    m_multiplicity = n;
    updatePreviewText();
  }
  void GaussianInputDialog::setCharge(int n)
  {
    m_charge = n;
    updatePreviewText();
  }

  void GaussianInputDialog::setProcs(int n)
  {
    if (n > 0)
      m_procs = n;
    updatePreviewText();
  }

  void GaussianInputDialog::setOutput(int n)
  {
    switch (n)
    {
      case 1:
        m_output = " gfprint pop=full";
        break;
      case 2:
        m_output = " gfoldprint pop=full";
        break;
      default:
        m_output = "";
    }
    updatePreviewText();
  }

  void GaussianInputDialog::setChk(int n)
  {
    if (n) m_chk = true;
    else m_chk = false;
    updatePreviewText();
  }

  void GaussianInputDialog::setCoords(int n)
  {
    switch (n)
    {
      case 0:
        m_coordType = CARTESIAN;
        break;
      case 1:
        m_coordType = ZMATRIX;
        break;
      case 2:
        m_coordType = ZMATRIX_COMPACT;
        break;
      default:
        m_coordType = CARTESIAN;
    }
    updatePreviewText();
  }

  QString GaussianInputDialog::generateInputDeck()
  {
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);

    // These directives are required before the job specification
    if (m_procs > 1)
      mol << "%NProcShared=" << m_procs << '\n';
    if (m_chk) {
      mol << "%Chk=checkpoint.chk\n";
    }

    // Now specify the job type etc
    mol << "#n " << getTheoryType(m_theoryType);

    // Not all theories have a basis set
    if (m_theoryType != AM1 && m_theoryType != PM3)
      mol << '/' << getBasisType(m_basisType);

    // Now for the calculation type
    mol << ' ' << getCalculationType(m_calculationType);

    // Output parameters for some programs
    mol << m_output;

    // Title line
    mol << "\n\n " << m_title << "\n\n";

    // Now for the charge and multiplicity
    mol << m_charge << ' ' << m_multiplicity << '\n';

    // Now to output the actual molecular coordinates
    // Cartesian coordinates
    if (m_molecule && m_coordType == CARTESIAN) {
      QTextStream mol(&buffer);
      QList<Atom *> atoms = m_molecule->atoms();
      foreach (Atom *atom, atoms) {
        mol << qSetFieldWidth(3) << left
            << QString(OpenBabel::OBElements::GetSymbol(atom->atomicNumber()))
            << qSetFieldWidth(15) << qSetRealNumberPrecision(5) << forcepoint
            << fixed << right << atom->pos()->x() << atom->pos()->y()
            << atom->pos()->z()
            << qSetFieldWidth(0) << '\n';
      }
      mol << '\n';
    }
    // Z-matrix
    else if (m_molecule && m_coordType == ZMATRIX) {
      QTextStream mol(&buffer);
      OpenBabel::OBAtom *a, *b, *c;
      double r, w, t;

      /* Taken from OpenBabel's gzmat file format converter */
      std::vector<OpenBabel::OBInternalCoord*> vic;
      vic.push_back((OpenBabel::OBInternalCoord*)NULL);
      OpenBabel::OBMol obmol = m_molecule->OBMol();
      FOR_ATOMS_OF_MOL(atom, &obmol)
        vic.push_back(new OpenBabel::OBInternalCoord);
      CartesianToInternal(vic, obmol);

      foreach (Atom *atom, m_molecule->atoms()) {
        a = vic[atom->index()+1]->_a;
        b = vic[atom->index()+1]->_b;
        c = vic[atom->index()+1]->_c;

        mol << qSetFieldWidth(3) << left
            << QString(OpenBabel::OBElements::GetSymbol(atom->atomicNumber()))
            << qSetFieldWidth(0);
        if (atom->index() > 0)
          mol << ' ' << a->GetIdx() << " B" << atom->index();
        if (atom->index() > 1)
          mol << ' ' << b->GetIdx() << " A" << atom->index();
        if (atom->index() > 2)
          mol << ' ' << c->GetIdx() << " D" << atom->index();
        mol << '\n';
      }

      mol << "Variables:" << endl;
      foreach (Atom *atom, m_molecule->atoms()) {
        r = vic[atom->index()+1]->_dst;
        w = vic[atom->index()+1]->_ang;
        if (w < 0.0)
          w += 360.0;
        t = vic[atom->index()+1]->_tor;
        if (t < 0.0)
          t += 360.0;
        if (atom->index() > 0)
          mol << "B" << atom->index() << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right
              << r << qSetFieldWidth(0) << '\n';
        if (atom->index() > 1)
          mol << "A" << atom->index() << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right
              << w << qSetFieldWidth(0) << '\n';
        if (atom->index() > 2)
          mol << "D" << atom->index() << qSetFieldWidth(15)
              << qSetRealNumberPrecision(5) << forcepoint << fixed << right
              << t << qSetFieldWidth(0) << '\n';
      }
      mol << '\n';
      foreach(OpenBabel::OBInternalCoord *c, vic)
        delete c;
    }
    else if (m_molecule && m_coordType == ZMATRIX_COMPACT)
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

        mol << qSetFieldWidth(3) << left << QString(OBElements::GetSymbol(atom->GetAtomicNum()))
            << qSetFieldWidth(6) << right;
        if (atom->GetIdx() > 1)
          mol << a->GetIdx() << qSetFieldWidth(15)
          << qSetRealNumberPrecision(5) << forcepoint << fixed << right << r;
        if (atom->GetIdx() > 2)
          mol << qSetFieldWidth(6) << right << b->GetIdx() << qSetFieldWidth(15)
          << qSetRealNumberPrecision(5) << forcepoint << fixed << right << w;
        if (atom->GetIdx() > 3)
          mol << qSetFieldWidth(6) << right << c->GetIdx() << qSetFieldWidth(15)
          << qSetRealNumberPrecision(5) << forcepoint << fixed << right << t;
        mol << qSetFieldWidth(0) << '\n';
      }
      mol << '\n';
      foreach(OpenBabel::OBInternalCoord *c, vic)
        delete c;
    }

    return buffer;
  }

  QString GaussianInputDialog::getCalculationType(calculationType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case SP:
        return "SP";
      case OPT:
        return "Opt";
      case FREQ:
        return "Opt Freq";
      default:
        return "SP";
    }
  }

  QString GaussianInputDialog::getTheoryType(theoryType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case AM1:
        return "AM1";
      case PM3:
        return "PM3";
      case RHF:
        return "RHF";
      case B3LYP:
        return "B3LYP";
      case MP2:
        return "MP2";
      case CCSD:
        return "CCSD";
      default:
        return "RHF";
    }
  }

  QString GaussianInputDialog::getBasisType(basisType t)
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
      case LANL2DZ:
        return "LANL2DZ";
      default:
        return "6-31G(d)";
    }
  }

  void GaussianInputDialog::deckDirty(bool dirty)
  {
    m_dirty = dirty;
    ui.titleLine->setEnabled(!dirty);
    ui.calculationCombo->setEnabled(!dirty);
    ui.theoryCombo->setEnabled(!dirty);
    ui.basisCombo->setEnabled(!dirty);
    ui.multiplicitySpin->setEnabled(!dirty);
    ui.chargeSpin->setEnabled(!dirty);
    ui.procSpin->setEnabled(!dirty);
    ui.outputCombo->setEnabled(!dirty);
    ui.checkpointCheck->setEnabled(!dirty);
    ui.enableFormButton->setEnabled(dirty);
  }

}

