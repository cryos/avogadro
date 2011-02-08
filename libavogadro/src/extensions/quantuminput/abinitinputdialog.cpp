/**********************************************************************
  AbinitInputDialog - Dialog for generating Abinit input decks

  Copyright (C) 2010 Matthieu Verstraete
  copied from GaussianInputDialog

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

#include "abinitinputdialog.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>

#include <QString>
#include <QTextStream>
#include <QTextBrowser>
#include <QFileDialog>
#include <QMessageBox>
#include <QShowEvent>
#include <QSettings>
#include <QDebug>
#include <QProcess>

using namespace OpenBabel;

namespace Avogadro
{

  AbinitInputDialog::AbinitInputDialog(QWidget *parent, Qt::WindowFlags f)
    : InputDialog(parent, f),
    // arguments, with default options?
    // Basic Tab
    m_title("Title"),
    m_ecut(10.0),
    m_coordType(XANGST),
    m_occopt(SEMICOND),
    m_tsmear(0.01),
    m_procs(1),
    m_ngkpt1(1),
    m_ngkpt2(1),
    m_ngkpt3(1),
    m_ionmov(NOMOV),
    m_nband(1),
    // Advanced Tab
    m_tolmxf(5.e-5),
    m_dilatmx(1.05),
    m_ecutsm(0.5),
    m_ntime(20),
    m_nshiftk(1),
    m_shiftk1(0.0),
    m_shiftk2(0.0),
    m_shiftk3(0.0),
    m_toleranceType(TOLDFE),
    m_tolXXX(1.e-10),
    // PAW Tab
    m_pawecutdg(20.0),
    // Rest
    m_dirty(false),
    m_warned(false),
    m_process(0),
    m_progress(0),
    m_logWindow(0)
  {
    ui.setupUi(this);
    // Connect the GUI elements to the correct slots
    // Basic Tab
    connect(ui.titleLine, SIGNAL(editingFinished()),
        this, SLOT(setTitle()));
    connect(ui.ecutLineEdit, SIGNAL(editingFinished()),
        this, SLOT(setEcut()));
    connect(ui.coordCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setCoordType(int)));
    connect(ui.occoptCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setOccopt(int)));
    connect(ui.tsmearLineEdit, SIGNAL(editingFinished()),
        this, SLOT(setTsmear()));
    connect(ui.procSpin, SIGNAL(valueChanged(int)),
        this, SLOT(setProcs(int)));
    connect(ui.ngkpt1Spin, SIGNAL(valueChanged(int)),
        this, SLOT(setNgkpt1(int)));
    connect(ui.ngkpt2Spin, SIGNAL(valueChanged(int)),
        this, SLOT(setNgkpt2(int)));
    connect(ui.ngkpt3Spin, SIGNAL(valueChanged(int)),
        this, SLOT(setNgkpt3(int)));
    connect(ui.ionmovCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setIonmov(int)));
    connect(ui.nbandSpin, SIGNAL(valueChanged(int)),
        this, SLOT(setNband(int)));
    // Advanced Tab
    connect(ui.tolmxfLineEdit, SIGNAL(editingFinished()),
        this, SLOT(setTolmxf()));
    connect(ui.dilatmxLineEdit, SIGNAL(editingFinished()),
        this, SLOT(setDilatmx()));
    connect(ui.ecutsmLineEdit, SIGNAL(editingFinished()),
        this, SLOT(setEcutsm()));
    connect(ui.ntimeSpin, SIGNAL(valueChanged(int)),
        this, SLOT(setNtime(int)));
    connect(ui.nshiftkSpin, SIGNAL(valueChanged(int)),
        this, SLOT(setNshiftk(int)));
    connect(ui.shiftk1LineEdit, SIGNAL(editingFinished()),
        this, SLOT(setShiftk1()));
    connect(ui.shiftk2LineEdit, SIGNAL(editingFinished()),
        this, SLOT(setShiftk2()));
    connect(ui.shiftk3LineEdit, SIGNAL(editingFinished()),
        this, SLOT(setShiftk3()));
    connect(ui.toleranceTypeCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setToleranceType(int)));
    connect(ui.tolXXXLineEdit, SIGNAL(editingFinished()),
        this, SLOT(setTolXXX()));
    // PAW Tab
    connect(ui.pawecutdgLineEdit, SIGNAL(editingFinished()),
        this, SLOT(setPawecutdg()));

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

#ifdef WIN32
    // Current code won't execute compute process on Windows. FIXME
    ui.computeButton->setVisible(false);
#endif // WIN32

    QSettings settings;
    readSettings(settings);

    // Generate an initial preview of the input deck
    updatePreviewText();
  }

  AbinitInputDialog::~AbinitInputDialog()
  {
      QSettings settings;
      writeSettings(settings);
  }

  void AbinitInputDialog::writeSettings(QSettings &settings) const
  {
    // Basic Tab
    settings.setValue("abinit/Title", ui.titleLine->text());
    settings.setValue("abinit/Ecut", ui.ecutLineEdit->text());
    settings.setValue("abinit/CoordType", ui.coordCombo->currentIndex());
    settings.setValue("abinit/Occopt", ui.occoptCombo->currentIndex());
    settings.setValue("abinit/Tsmear", ui.tsmearLineEdit->text());
    settings.setValue("abinit/Procs", ui.procSpin->value());
    settings.setValue("abinit/Ngkpt1", ui.ngkpt1Spin->value());
    settings.setValue("abinit/Ngkpt2", ui.ngkpt2Spin->value());
    settings.setValue("abinit/Ngkpt3", ui.ngkpt3Spin->value());
    settings.setValue("abinit/Ionmov", ui.ionmovCombo->currentIndex());
    settings.setValue("abinit/Nband", ui.nbandSpin->value());
    // Advanced Tab
    settings.setValue("abinit/Tolmxf", ui.tolmxfLineEdit->text());
    settings.setValue("abinit/Dilatmx", ui.dilatmxLineEdit->text());
    settings.setValue("abinit/Ecutsm", ui.ecutsmLineEdit->text());
    settings.setValue("abinit/Ntime", ui.ntimeSpin->value());
    settings.setValue("abinit/Nshiftk", ui.nshiftkSpin->value());
    settings.setValue("abinit/Shiftk1", ui.shiftk1LineEdit->text());
    settings.setValue("abinit/Shiftk2", ui.shiftk2LineEdit->text());
    settings.setValue("abinit/Shiftk3", ui.shiftk3LineEdit->text());
    settings.setValue("abinit/ToleranceType", ui.toleranceTypeCombo->currentIndex());
    settings.setValue("abinit/TolXXX", ui.tolXXXLineEdit->text());
    // PAW Tab
    settings.setValue("abinit/Pawecutdg", ui.pawecutdgLineEdit->text());

    settings.setValue("abinit/savepath", m_savePath);
  }

  void AbinitInputDialog::readSettings(QSettings &settings)
  {
    // Basic Tab
    ui.titleLine->setText(settings.value("abinit/Title", "").toString());
    setTitle();

    ui.ecutLineEdit->setText(settings.value("abinit/Ecut", "10.0").toString());
    setEcut();
    ui.coordCombo->setCurrentIndex(settings.value("abinit/CoordType", 0).toInt());
    setCoordType(settings.value("abinit/CoordType", 0).toInt());
    ui.occoptCombo->setCurrentIndex(settings.value("abinit/Occopt", 0).toInt());
    setOccopt(settings.value("abinit/Occopt", 0).toInt());
    ui.tsmearLineEdit->setText(settings.value("abinit/Tsmear", "0.01").toString());
    setTsmear();
    ui.procSpin->setValue(settings.value("abinit/Procs", 1).toInt());
    setProcs(settings.value("abinit/Procs", 1).toInt());
    ui.ngkpt1Spin->setValue(settings.value("abinit/Ngkpt1", 1).toInt());
    setNgkpt1(settings.value("abinit/Ngkpt1", 1).toInt());
    ui.ngkpt2Spin->setValue(settings.value("abinit/Ngkpt2", 1).toInt());
    setNgkpt2(settings.value("abinit/Ngkpt2", 1).toInt());
    ui.ngkpt3Spin->setValue(settings.value("abinit/Ngkpt3", 1).toInt());
    setNgkpt3(settings.value("abinit/Ngkpt3", 1).toInt());
    ui.ionmovCombo->setCurrentIndex(settings.value("abinit/Ionmov", 0).toInt());
    setIonmov(settings.value("abinit/Ionmov", 0).toInt());
    ui.nbandSpin->setValue(settings.value("abinit/Nband", 0).toInt());
    setNband(settings.value("abinit/Nband", 0).toInt());
    // Advanced Tab
    ui.tolmxfLineEdit->setText(settings.value("abinit/Tolmxf", "5.e-5").toString());
    setTolmxf();
    ui.dilatmxLineEdit->setText(settings.value("abinit/Dilatmx", "1.05").toString());
    setDilatmx();
    ui.ecutsmLineEdit->setText(settings.value("abinit/Ecutsm", "0.5").toString());
    setEcutsm();
    ui.ntimeSpin->setValue(settings.value("abinit/Ntime", 10).toInt());
    setNtime(settings.value("abinit/Ntime", 10).toInt());
    ui.nshiftkSpin->setValue(settings.value("abinit/Nshiftk", 1).toInt());
    setNshiftk(settings.value("abinit/Nshiftk", 1).toInt());
    ui.shiftk1LineEdit->setText(settings.value("abinit/Shiftk1", "0.0").toString());
    setShiftk1();
    ui.shiftk2LineEdit->setText(settings.value("abinit/Shiftk2", "0.0").toString());
    setShiftk2();
    ui.shiftk3LineEdit->setText(settings.value("abinit/Shiftk3", "0.0").toString());
    setShiftk3();
    ui.toleranceTypeCombo->setCurrentIndex(settings.value("abinit/ToleranceType", 0).toInt());
    setToleranceType(settings.value("abinit/ToleranceType", 0).toInt());
    ui.tolXXXLineEdit->setText(settings.value("abinit/TolXXX", "1.e-10").toString());
    setTolXXX();
    // PAW Tab
    ui.pawecutdgLineEdit->setText(settings.value("abinit/Pawecutdg", "20.0").toString());
    setPawecutdg();

    m_savePath = settings.value("abinit/savepath").toString();
  }

  void AbinitInputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();

    if (pathToAbinit().isEmpty())
      ui.computeButton->hide();
    else
      ui.computeButton->show();
  }

  QString AbinitInputDialog::pathToAbinit() const
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

    // Add default abinit directories here...
    pathList << "/opt/etsf/bin";

    // I don't know how this works for Windows -- probably need a different
    // method
    foreach(const QString &path, pathList) {
      QFileInfo abinit(path + '/' + "abinit");
      if (abinit.exists() && abinit.isExecutable())
        returnPath = abinit.canonicalFilePath();
    }

    return returnPath;
  }

  void AbinitInputDialog::setMolecule(Molecule *molecule)
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

  void AbinitInputDialog::updatePreviewText()
  {
    if (!isVisible())
      return;
    // Generate the input deck and display it
    if (m_dirty && !m_warned) {
      m_warned = true;
      QMessageBox msgBox;

      msgBox.setWindowTitle(tr("Abinit Input Deck Generator Warning"));
      msgBox.setText(tr("Would you like to update the preview text, losing all changes made in the Abinit input deck preview pane?"));
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

  void AbinitInputDialog::resetClicked()
  {

    // Reset the form to defaults
    deckDirty(false);
    // Basic Tab
    ui.titleLine->setText("");
    setTitle();
    ui.ecutLineEdit->setText("10.0");
    setEcut();
    // DLonie: An enum would be clearer and type safe for these integer args (occopt, coordtype, ionmov, etc).
    setCoordType(0); // XANGST
    setOccopt(0);    // SEMICOND
    ui.tsmearLineEdit->setText("0.01");
    setTsmear();
    setProcs(1);
    setNgkpt1(1);
    setNgkpt2(1);
    setNgkpt3(1);
    setIonmov(0);         // NOMOV
    setNband(0);
    // Advanced Tab
    ui.tolmxfLineEdit->setText("5.e-5");
    setTolmxf();
    ui.dilatmxLineEdit->setText("1.05");
    setDilatmx();
    ui.ecutsmLineEdit->setText("0.5");
    setEcutsm();
    setNtime(10);
    setNshiftk(1);
    ui.shiftk1LineEdit->setText("0.0");
    setShiftk1();
    ui.shiftk2LineEdit->setText("0.0");
    setShiftk2();
    ui.shiftk3LineEdit->setText("0.0");
    setShiftk3();
    setToleranceType(0);   // TOLDFE
    ui.tolXXXLineEdit->setText("1.e-10");
    setTolXXX();
    // PAW Tab
    ui.pawecutdgLineEdit->setText("20.0");
    setPawecutdg();

    ui.procSpin->setValue(1);
  }

  QString AbinitInputDialog::saveInputFile(QString inputDeck, QString fileType,
                                           QString ext)
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

    QString defaultFileName = defaultPath + '/' + defaultFile.baseName();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Input Deck"),
        defaultFileName, fileType + " (*." + ext + ")");

    if(fileName == "")
      return fileName;

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) return QString();
// end of copied

// Fragment copied from InputDialog
    file.write(inputDeck.toLocal8Bit()); // prevent troubles in Windows
    file.close(); // flush buffer!
    m_savePath = QFileInfo(file).absolutePath();
    return fileName;
  }

  void AbinitInputDialog::generateClicked()
  {
    saveInputFile(ui.previewText->toPlainText(),
                          tr("Abinit_Input_Deck"), QString("in"));
  }

  void AbinitInputDialog::computeClicked()
  {
    if (m_process != 0) {
      QMessageBox::warning(this, tr("Abinit Running."),
                           tr("Abinit is already running. Wait until the previous calculation is finished."));
      return;
    }

    QString fileName = saveInputFile(ui.previewText->toPlainText(),
                          tr("Abinit_Input_Deck"), QString("in"));
    if (fileName.isEmpty())
      return;

    QFileInfo info(pathToAbinit());
    if (!info.exists() || !info.isExecutable()) {
      QMessageBox::warning(this, tr("Abinit Not Installed."),
                           tr("The abinit executable cannot be found."));
      return;
    }

    m_process = new QProcess(this);
    QFileInfo input(fileName);
    m_process->setWorkingDirectory(input.absolutePath());


    // write file names to filesFileName
    QFileInfo inputFile(fileName);
    QString defaultPath = inputFile.canonicalPath();
    QString outputFile = defaultPath + '/' + inputFile.baseName() + ".out";
    QString filesFile = \
         fileName              + "\n" + \
         inputFile.canonicalPath() + '/' + inputFile.baseName() + ".o"  + "\n" + \
         inputFile.canonicalPath() + '/' + inputFile.baseName() + ".inp" + "\n" + \
         inputFile.canonicalPath() + '/' + inputFile.baseName() + ".out" + "\n" + \
         inputFile.canonicalPath() + '/' + inputFile.baseName() + ".tmp" + "\n";

    QString defaultFileName = defaultPath + '/' + inputFile.baseName();
    QString pseudoFileName;
    QString pseudoDialogTitle;
    int iz = 0;
    foreach (int zz, m_znucl) {
      pseudoDialogTitle = "Pseudopotential for Z= " + QString::number(zz,10);
      pseudoFileName = QFileDialog::getOpenFileName(this, pseudoDialogTitle,
          defaultFileName, tr("Pseudos (*.hgh *.gth *.pspgth *.fhi *.pspnc *.paw *.mt *.UPF *.upf *.psp)"));
      defaultFileName = pseudoFileName; // this makes sure the next psp will be searched for in the same directory
      filesFile = filesFile + pseudoFileName + "\n";
      iz++;
    }

    QString filesFileName = inputFile.canonicalPath() + '/' + inputFile.baseName() + ".files";
    QFile file(filesFileName);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
      //complain ;
    }
    file.write(filesFile.toLocal8Bit()); // prevent troubles in Windows
    file.close();

    QStringList arguments;
    m_process->setStandardInputFile(filesFileName);
    m_logFileName = inputFile.canonicalPath() + '/' + inputFile.baseName() + ".log";
    m_process->setStandardOutputFile(m_logFileName);

    m_process->start(pathToAbinit(), arguments);
    if (!m_process->waitForStarted()) {
      QMessageBox::warning(this, tr("abinit failed to start."),
                           tr("abinit did not start. Perhaps it is not installed correctly."));
    }
    connect(m_process, SIGNAL(finished(int)), this, SLOT(finished(int)));
    m_progress = new QProgressDialog(this);
    m_progress->setRange(0,0); // indeterminate progress
    m_progress->setLabelText(tr("Running Abinit calculation..."));
    m_progress->show();
    connect(m_progress, SIGNAL(canceled()), this, SLOT(stopProcess()));

    // print out log std output in a window... does not work for the moment - do I need a window object to embed this in?
    // DLonie: Yes -- a QDialog with a QTextBrowser would be simplest.
    //m_logWindow = new QTextBrowser(this);
    //m_logWindow->setSource(m_logFileName);
    //connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(printLogOutput()));
    //connect(m_logWindow, SIGNAL(canceled()), this, SLOT(stoplogWindow()));


  }

  void AbinitInputDialog::printLogOutput()
  {
    if (m_logWindow) {
      m_logWindow->reload();
    }
  }

  void AbinitInputDialog::stopProcess()
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

  void AbinitInputDialog::finished(int exitCode)
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
      QMessageBox::warning(this, tr("Abinit Crashed."),
                           tr("Abinit did not run correctly. Perhaps it is not installed correctly."));
     return;
    }

    // we have a successful run.
    // Now, read in the results and close the dialog
    QFileInfo inputFile(m_logFileName);
    QString outputFile = inputFile.canonicalPath() + '/' + inputFile.baseName() + ".o";
    // for a relaxation run, need to add the _TIM10_ string too
    QString cmlFile = inputFile.canonicalPath() + '/' + inputFile.baseName() + "out.cml";
    emit readOutput(cmlFile);

    close();
  }

  void AbinitInputDialog::moreClicked()
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

  void AbinitInputDialog::enableFormClicked()
  {
    deckDirty(false);
    updatePreviewText();
  }

  void AbinitInputDialog::previewEdited()
  {
    // Determine if the preview text has changed from the form generated
    if(ui.previewText->toPlainText() != generateInputDeck())
      deckDirty(true);
    else
      deckDirty(false);
  }

  void AbinitInputDialog::setTitle()
  {
    m_title = ui.titleLine->text();
    updatePreviewText();
  }

  void AbinitInputDialog::setEcut()
  {
    // TODO : need some error checking that the string is not garbage - does toFloat do this?
    double d = ui.ecutLineEdit->text().toFloat();
    if (d > 1.e-10)
      m_ecut = d;
    updatePreviewText();
  }

  void AbinitInputDialog::setCoordType(int n)
  {
    switch (n)
    {
      case 0:
        m_coordType = XANGST;
        break;
      case 1:
        m_coordType = XRED;
        break;
      default:
        m_coordType = XANGST;
    }
    updatePreviewText();
  }

  void AbinitInputDialog::setOccopt(int n)
  {
    switch (n)
    {
      case 0:
        m_occopt = SEMICOND;
        break;
      case 1:
        m_occopt = FERMI;
        break;
      case 2:
        m_occopt = COLD;
        break;
      case 3:
        m_occopt = GAUSSIAN;
        break;
      default:
        m_occopt = SEMICOND;
    }
    updatePreviewText();
  }

  void AbinitInputDialog::setTsmear()
  {
    double d = ui.tsmearLineEdit->text().toFloat();
    if (d > 1.e-10)
      m_tsmear = d;
    updatePreviewText();
  }

  void AbinitInputDialog::setProcs(int n)
  {
    if (n > 0)
      m_procs = n;
    updatePreviewText();
  }

  void AbinitInputDialog::setNgkpt1(int n)
  {
    if (n > 0)
      m_ngkpt1 = n;
    updatePreviewText();
  }

  void AbinitInputDialog::setNgkpt2(int n)
  {
    if (n > 0)
      m_ngkpt2 = n;
    updatePreviewText();
  }

  void AbinitInputDialog::setNgkpt3(int n)
  {
    if (n > 0)
      m_ngkpt3 = n;
    updatePreviewText();
  }

  void AbinitInputDialog::setIonmov(int n)
  {
    switch (n)
    {
      case 0:
        m_ionmov = NOMOV;
        break;
      case 1:
        m_ionmov = VISCOUS;
        break;
      case 2:
        m_ionmov = BFGS;
        break;
      case 3:
        m_ionmov = BGFSE;
        break;
      default:
        m_ionmov = NOMOV;
    }
    updatePreviewText();
  }

  // TODO : check that nban 0 gives default behavior in abinit of getting nelec/2 + 10% or so
  void AbinitInputDialog::setNband(int n)
  {
    if (n >= 0)
      m_nband = n;
    updatePreviewText();
  }

  // Advanced Tab
  void AbinitInputDialog::setTolmxf()
  {
    double d = ui.tolmxfLineEdit->text().toFloat();
    if (d > 0.0)
      m_tolmxf = d;
    updatePreviewText();
  }

  void AbinitInputDialog::setDilatmx()
  {
    double d = ui.dilatmxLineEdit->text().toFloat();
    if (d > 1.0)
      m_dilatmx = d;
    updatePreviewText();
  }

  void AbinitInputDialog::setEcutsm()
  {
    double d = ui.ecutsmLineEdit->text().toFloat();
    if (d > 0.0)
      m_ecutsm = d;
    updatePreviewText();
  }

  void AbinitInputDialog::setNtime(int n)
  {
    if (n > 0)
      m_ntime = n;
    updatePreviewText();
  }

  // TODO : this should influence the number of lines of shiftk we accept. Do not know how to do this in Qt
  void AbinitInputDialog::setNshiftk(int n)
  {
    if (n > 0)
      m_nshiftk = n;
    updatePreviewText();
  }

  void AbinitInputDialog::setShiftk1()
  {
    double d = ui.shiftk1LineEdit->text().toFloat();
    if (abs(d) <= 1.0)
      m_shiftk1 = d;
    updatePreviewText();
  }

  void AbinitInputDialog::setShiftk2()
  {
    double d = ui.shiftk2LineEdit->text().toFloat();
    if (abs(d) <= 1.0)
      m_shiftk2 = d;
    updatePreviewText();
  }

  void AbinitInputDialog::setShiftk3()
  {
    double d = ui.shiftk3LineEdit->text().toFloat();
    if (abs(d) <= 1.0)
      m_shiftk3 = d;
    updatePreviewText();
  }

  void AbinitInputDialog::setToleranceType(int n)
  {
    switch (n)
    {
      case 0:
        m_toleranceType = TOLDFE;
        break;
      case 1:
        m_toleranceType = TOLWFR;
        break;
      case 2:
        m_toleranceType = TOLVRS;
        break;
      case 3:
        m_toleranceType = TOLDFF;
        break;
      default:
        m_toleranceType = TOLRFF;
    }
    updatePreviewText();
  }

  void AbinitInputDialog::setTolXXX()
  {
    double d = ui.tolXXXLineEdit->text().toFloat();
    if (d > 0.0)
      m_tolXXX = d;
    updatePreviewText();
  }

  // PAW Tab
  void AbinitInputDialog::setPawecutdg()
  {
    double d = ui.pawecutdgLineEdit->text().toFloat();
    if (d > 0.0)
      m_pawecutdg = d;
    updatePreviewText();
  }



  QString AbinitInputDialog::generateInputDeck()
  {
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);

    // Title line
    mol << "# " << m_title << "\n";
    mol << "# abinit input generated by Avogadro plugin\n";
    //  for information: this should be used when calling abinit with mpirun or something
    mol << "# abinit found at " << pathToAbinit() << "\n";
    if (m_procs > 1)
      mol << "#  Number of processors should be =" << m_procs << '\n';

    mol << "\n\n";
    mol << "# basis set, bands, k-points, SCF tolerance\n";
    mol << "ecut " << m_ecut << '\n';
    if (m_pawecutdg > 0.0)
      mol << "pawecutdg  " << m_pawecutdg << '\n';
    // nband is only output if it is actually set to something > 1 (default)
    // otherwise abinit will get it automatically
    if (m_nband > 1)
      mol << "nband " << m_nband << '\n';
    mol << "occopt " << getOccopt(m_occopt) << '\n';
    mol << "tsmear " << m_tsmear << '\n';
    mol << "ngkpt " << m_ngkpt1 << " " << m_ngkpt2 << " " << m_ngkpt3 << '\n';
    mol << "nshiftk " << m_nshiftk << '\n';
    mol << "shiftk " << '\n';
    mol << "  " << m_shiftk1 << " " << m_shiftk2 << " " << m_shiftk3 << '\n';
    mol << getToleranceType(m_toleranceType) << " " << m_tolXXX << '\n';


    mol << "# unit cell\n";
    OBUnitCell *uc = m_molecule->OBUnitCell();
    if (uc) {
      mol << "acell "
        << uc->GetA() << " "
        << uc->GetB() << " "
        << uc->GetC() << " Angstr \n";

      mol << "angdeg "
        << uc->GetAlpha() << " "
        << uc->GetBeta()  << " "
        << uc->GetGamma() << "\n";
    }
    // If we have a molecule, invent the box it will live in
    // TODO: check the box is big enough: get molecule size and add a bit in
    // each direction, or make it a ui input.
    else {
      mol << "acell 10.0 10.0 10.0 Angstr\n"; // defaults to a rectangular box
    }


    // Now to output the actual molecular coordinates
    // could also print an auxiliary xyz file and just give the name to abinit...

    // Cartesian coordinates
    mol << "# Atomic positions and species\n";
    mol << "natom " << m_molecule->numAtoms() << "\n";

    QList<Atom *> atoms = m_molecule->atoms();

    if (m_molecule && m_coordType == XANGST) {
      // positions of all atoms
      mol << "xangst\n";
      foreach (Atom *atom, atoms) {
        mol << qSetFieldWidth(3) << left
            << qSetFieldWidth(15) << qSetRealNumberPrecision(5) << forcepoint
            << fixed << right << atom->pos()->x() << atom->pos()->y()
            << atom->pos()->z()
            << qSetFieldWidth(0) << '\n';
      }
    }
    // Reduced coordinates
    else if (m_molecule && m_coordType == XRED) {
      mol << "Not implemented yet\n";
/*
      QTextStream mol(&buffer);
      OpenBabel::OBAtom *a, *b, *c;
      double r, w, t;

      std::vector<OpenBabel::OBInternalCoord*> vic;
      vic.push_back((OpenBabel::OBInternalCoord*)NULL);
      OpenBabel::OBMol obmol = m_molecule->OBMol();
      FOR_ATOMS_OF_MOL(atom, &obmol)
        vic.push_back(new OpenBabel::OBInternalCoord);
      CartesianToInternal(vic, obmol);
*/

    }


    // Z of each atom
    QList<int> Zatom, Zatom_sorted;
    foreach (Atom *atom, atoms) {
       Zatom.append(atom->atomicNumber());
       Zatom_sorted.append(atom->atomicNumber());
    }
    qSort(Zatom_sorted);

    // number of types of atoms
    unsigned int iatom, iz;
    int ntypat = 1;
    int TypatofZ[200];
    for (iz = 0; iz < 200; iz++) { TypatofZ[iz] = 0; };
    QList<int> ZofTypat;
    ZofTypat.append(Zatom_sorted[0]);
    TypatofZ[int(Zatom_sorted[0])] = ntypat;
    // start with second atom (have at least 1)
    for (iatom = 1; iatom < m_molecule->numAtoms(); iatom++) {
      if(Zatom_sorted[iatom] != Zatom_sorted[iatom-1]) {
        ntypat++;
        ZofTypat.append(Zatom_sorted[iatom]);
        TypatofZ[int(Zatom_sorted[iatom])] = ntypat;
      }
    }
    mol << "ntypat " << ntypat << '\n';
    mol << "znucl ";
    m_znucl.clear();
    foreach (int Z, ZofTypat) {
      mol << Z << "  " ;
      m_znucl << Z;
    }
    mol << "\n" ;

    mol << "typat ";
    for (iatom = 0; iatom < m_molecule->numAtoms(); iatom++) {
      mol << TypatofZ[int(Zatom[iatom])] << "  ";
    }

    mol << "\n\n";
    mol << "# Geometrical relaxation\n";
    mol << "ionmov " << getIonmov(m_ionmov) << '\n';
    mol << "ntime " << m_ntime << '\n';
    mol << "tolmxf " << m_tolmxf << '\n';
    mol << "dilatmx " << m_dilatmx << '\n';
    mol << "ecutsm " << m_ecutsm << '\n';

    mol << "\n\n";
    mol << "# Other stuff\n";
    mol << "prtcml 1 \n";

    return buffer;
  }

  int AbinitInputDialog::getOccopt(occoptType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case SEMICOND:
        return 1;
      case FERMI:
        return 3;
      case COLD:
        return 4;
      case GAUSSIAN:
        return 7;
      default:
        return 0;
    }
  }

  int AbinitInputDialog::getIonmov(ionmovType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case NOMOV:
        return 0;
      case VISCOUS:
        return 1;
      case BFGS:
        return 2;
      case BGFSE:
        return 3;
      default:
        return 0;
    }
  }

  QString AbinitInputDialog::getCoordType(coordType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case XANGST:
        return "xangst";
      case XRED:
        return "xred";
      default:
        return "xangst";
    }
  }

  QString AbinitInputDialog::getToleranceType(toleranceType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case TOLDFE:
        return "toldfe";
      case TOLWFR:
        return "tolwfr";
      case TOLVRS:
        return "tolvrs";
      case TOLDFF:
        return "toldff";
      case TOLRFF:
        return "tolrff";
      default:
        return "toldfe";
    }
  }

  void AbinitInputDialog::deckDirty(bool dirty)
  {
    m_dirty = dirty;

    // Basic Tab
    ui.titleLine->setEnabled(!dirty);
    ui.ecutLineEdit->setEnabled(!dirty);
    ui.coordCombo->setEnabled(!dirty);
    ui.occoptCombo->setEnabled(!dirty);
    ui.tsmearLineEdit->setEnabled(!dirty);
    ui.procSpin->setEnabled(!dirty);
    ui.ngkpt1Spin->setEnabled(!dirty);
    ui.ngkpt2Spin->setEnabled(!dirty);
    ui.ngkpt3Spin->setEnabled(!dirty);
    ui.ionmovCombo->setEnabled(!dirty);
    ui.nbandSpin->setEnabled(!dirty);
    // Advanced Tab
    ui.tolmxfLineEdit->setEnabled(!dirty);
    ui.dilatmxLineEdit->setEnabled(!dirty);
    ui.ecutsmLineEdit->setEnabled(!dirty);
    ui.ntimeSpin->setEnabled(!dirty);
    ui.nshiftkSpin->setEnabled(!dirty);
    ui.shiftk1LineEdit->setEnabled(!dirty);
    ui.shiftk2LineEdit->setEnabled(!dirty);
    ui.shiftk3LineEdit->setEnabled(!dirty);
    ui.toleranceTypeCombo->setEnabled(!dirty);
    ui.tolXXXLineEdit->setEnabled(!dirty);
    // PAW Tab
    ui.pawecutdgLineEdit->setEnabled(!dirty);

    ui.enableFormButton->setEnabled(dirty);
  }

}

