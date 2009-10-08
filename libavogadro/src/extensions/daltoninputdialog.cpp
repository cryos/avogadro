/**********************************************************************
  DaltonInputDialog - Dialog for generating Dalton input files

  Source code shamelessly copied from gaussianinputdialog.cpp

 **********************************************************************/

#include "daltoninputdialog.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>

#include <QString>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

using namespace OpenBabel;

namespace Avogadro
{

  DaltonInputDialog::DaltonInputDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_molecule(0), m_title("Title"), m_calculationType(SP),
    m_theoryType(B3LYP), m_basisType(ccpVDZ), m_dirty(false), m_warned(false)
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
    connect(ui.previewText, SIGNAL(textChanged()),
        this, SLOT(previewEdited()));
    connect(ui.generateButton, SIGNAL(clicked()),
        this, SLOT(generateClicked()));
    connect(ui.resetButton, SIGNAL(clicked()),
        this, SLOT(resetClicked()));
    connect(ui.moreButton, SIGNAL(clicked()),
        this, SLOT(moreClicked()));

    // Generate an initial preview of the input deck
    updatePreviewText();
  }

  DaltonInputDialog::~DaltonInputDialog()
  {
  }

  void DaltonInputDialog::writeSettings(QSettings &settings) const
  {
    settings.setValue("daltonCalcType", ui.calculationCombo->currentIndex());
    settings.setValue("daltonTheory", ui.theoryCombo->currentIndex());
    settings.setValue("daltonBasis", ui.basisCombo->currentIndex());
  }

  void DaltonInputDialog::readSettings(QSettings &settings)
  {
    setCalculation(settings.value("daltonCalcType", 0).toInt());
    ui.calculationCombo->setCurrentIndex(settings.value("daltonCalcType", 0).toInt());
    setTheory(settings.value("daltonTheory", 0).toInt());
    ui.theoryCombo->setCurrentIndex(settings.value("daltonTheory", 0).toInt());
    setBasis(settings.value("daltonBasis", 0).toInt());
    ui.basisCombo->setCurrentIndex(settings.value("daltonBasis", 0).toInt());
  }


  void DaltonInputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();

//    if (pathToDalton().isEmpty())
//      ui.computeButton->hide();
//    else
//      ui.computeButton->show();
  }

/*
  QString DaltonInputDialog::pathToDalton() const
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

    // I don't know how this works for Windows -- probably need a different method
    foreach(const QString &path, pathList) {
      QFileInfo info(path + '/' + "dalton");
      if (info.exists() && info.isExecutable())
        returnPath = info.canonicalPath();
    }

    return returnPath;
  }
*/
  void DaltonInputDialog::setMolecule(Molecule *molecule)
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

  void DaltonInputDialog::updatePreviewText()
  {
    if (!isVisible())
      return;
    // Generate the input deck and display it
    if (m_dirty && !m_warned) {
      m_warned = true;
      QMessageBox msgBox;

      msgBox.setWindowTitle(tr("Dalton Input File Generator Warning"));
      msgBox.setText(tr("Would you like to update the preview text, losing all changes made in the Dalton input file preview pane?"));
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

  void DaltonInputDialog::resetClicked()
  {
    // Reset the form to defaults
    deckDirty(false);
    ui.calculationCombo->setCurrentIndex(0);
    ui.theoryCombo->setCurrentIndex(0);
    ui.basisCombo->setCurrentIndex(0);
  }

  QString DaltonInputDialog::saveInputFile()
  {
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty())
      defaultPath = QDir::homePath();

    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + ".dal";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Dalton Input File"),
                                defaultFileName, tr("Dalton Input File (*.dal)"));

    QFile file(fileName);
    // FIXME This really should pop up a warning if the file cannot be opened
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return QString();

    QString previewText = ui.previewText->toPlainText();

    QTextStream out(&file);
    out << previewText;

    return fileName;
  }

  void DaltonInputDialog::generateClicked()
  {
    saveInputFile();
  }

  void DaltonInputDialog::moreClicked()
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

  void DaltonInputDialog::previewEdited()
  {
    // Determine if the preview text has changed from the form generated
    if(ui.previewText->toPlainText() != generateInputDeck())
      deckDirty(true);
    else
      deckDirty(false);
  }

  void DaltonInputDialog::setTitle()
  {
    m_title = ui.titleLine->text();
    updatePreviewText();
  }

  void DaltonInputDialog::setCalculation(int n)
  {
    switch (n)
    {
      case 0:
        m_calculationType = SP;
        break;
      case 1:
        m_calculationType = PROP;
        break;
      default:
        m_calculationType = SP;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setTheory(int n)
  {
    switch (n)
    {
      case 0:
        m_theoryType = B3LYP;
        break;
      case 1:
        m_theoryType = PBE0;
        break;
      default:
        m_theoryType = B3LYP;
    }

/*
    if (m_theoryType == AM1 || m_theoryType == PM3)
      ui.basisCombo->setEnabled(false);
    else
*/
      ui.basisCombo->setEnabled(true);

    updatePreviewText();
  }

  void DaltonInputDialog::setBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_basisType = ccpVDZ;
        break;
      case 1:
        m_basisType = ccpVTZ;
        break;
      default:
        m_basisType = ccpVDZ;
    }
    updatePreviewText();
  }

/*
  void DaltonInputDialog::setMultiplicity(int n)
  {
    m_multiplicity = n;
    updatePreviewText();
  }
  void DaltonInputDialog::setCharge(int n)
  {
    m_charge = n;
    updatePreviewText();
  }

  void DaltonInputDialog::setProcs(int n)
  {
    if (n > 0)
      m_procs = n;
    updatePreviewText();
  }

  void DaltonInputDialog::setCoords(int n)
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
*/

  QString DaltonInputDialog::generateInputDeck()
  {
    QString buffer;
    QTextStream mol(&buffer);

    int newatomtype;
    int oldatomtype;
    int natomtypes;
    int* natoms = NULL;

    mol << "BASIS" << '\n';
    mol << getBasisType(m_basisType) << '\n';

    mol << " Input file generated by the Dalton Input File Plugin for Avogadro" << '\n';
    mol << " " << m_title << '\n';

    natoms = new int[120];

    for (int i=0; i<120; i++) {
      natoms[i] = 0;
    }

    newatomtype = 0;
    oldatomtype = 0;
    natomtypes = 0;

    QList<Atom *> atoms = m_molecule->atoms();

    foreach (Atom *atom, atoms) {
      newatomtype = atom->atomicNumber();
      if (newatomtype != oldatomtype) {
        natomtypes += 1;
        oldatomtype = newatomtype;
      }
      natoms[natomtypes] += 1;
    }

    mol << "Atomtypes=" << natomtypes << " Angstrom" << '\n';

    newatomtype = 0;
    oldatomtype = 0;
    natomtypes = 0;

    foreach (Atom *atom, atoms) {
      newatomtype = atom->atomicNumber();
      if (newatomtype != oldatomtype) {
        natomtypes += 1;
        mol << "Charge=" << atom->atomicNumber() << ".0" << " Atoms=" << natoms[natomtypes] << '\n';
        oldatomtype = newatomtype;
      }
      mol << qSetFieldWidth(3) << left
          << QString(OpenBabel::etab.GetSymbol(atom->atomicNumber()))
          << qSetFieldWidth(15) << qSetRealNumberPrecision(5) << forcepoint
          << fixed << right << atom->pos()->x() << atom->pos()->y()
          << atom->pos()->z()
          << qSetFieldWidth(0) << '\n';
    }

    delete [] natoms;
    natoms = NULL;

    mol << '\n';

    mol << "**DALTON INPUT" << '\n';
    mol << getCalculationType(m_calculationType) << '\n';
    mol << "**WAVE FUNCTIONS" << '\n';
    mol << ".DFT" << '\n';
    mol << " " << getTheoryType(m_theoryType) << '\n';
    mol << "**END OF DALTON INPUT";

    return buffer;
  }

  QString DaltonInputDialog::getCalculationType(calculationType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case SP:
        return ".RUN WAVE FUNCTIONS";
      case PROP:
        return ".RUN PROPERTIES";
      default:
        return ".RUN WAVE FUNCTIONS";
    }
  }

  QString DaltonInputDialog::getTheoryType(theoryType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case B3LYP:
        return "B3LYP";
      case PBE0:
        return "PBE0";
      default:
        return "B3LYP";
    }
  }

  QString DaltonInputDialog::getBasisType(basisType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case ccpVDZ:
        return "cc-pVDZ";
      case ccpVTZ:
        return "cc-pVTZ";
      default:
        return "cc-pVDZ";
    }
  }

  void DaltonInputDialog::deckDirty(bool dirty)
  {
    m_dirty = dirty;
    ui.titleLine->setEnabled(!dirty);
    ui.calculationCombo->setEnabled(!dirty);
    ui.theoryCombo->setEnabled(!dirty);
    ui.basisCombo->setEnabled(!dirty);
  }

}

