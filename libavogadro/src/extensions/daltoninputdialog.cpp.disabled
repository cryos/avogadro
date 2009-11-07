/*************************************************************************
  DaltonInputDialog - Dialog for generating Dalton input files

  Jogvan Magnus Olsen

  Initial source code was shamelessly copied from gaussianinputdialog.cpp

 *************************************************************************/

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
    : QDialog(parent, f), m_molecule(0), m_title("Title"),
    m_calculationType(SP), m_theoryType(HF), m_functionalType(B3LYP),
    m_basisType(STOnG), m_stoBasis(STO2G), m_popleBasis(p321G),
    m_poplediffBasis(p321ppG), m_poplepolBasis(p321Gs),
    m_poplediffpolBasis(p321ppGs), m_pcBasis(pc0), m_apcBasis(apc0),
    m_ccpvxzBasis(ccpVDZ), m_accpvxzBasis(accpVDZ), m_ccpcvxzBasis(ccpCVDZ),
    m_accpcvxzBasis(accpCVDZ), m_dftGrid(normal), m_propType(polari),
    m_exci(1), m_coreBasis(false), m_diffBasis(false),m_polBasis(false),
    m_directCheck(false), m_parallelCheck(false), m_dirty(false),
    m_warned(false)
  {
    ui.setupUi(this);
    // Connect the GUI elements to the correct slots
    connect(ui.titleLine, SIGNAL(editingFinished()),
        this, SLOT(setTitle()));
    connect(ui.calculationCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setCalculation(int)));
    connect(ui.theoryCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setTheory(int)));
    connect(ui.functionalCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setFunctional(int)));
    connect(ui.basisCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setBasis(int)));
    connect(ui.stoCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setstoBasis(int)));
    connect(ui.popleCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setpopleBasis(int)));
    connect(ui.poplediffCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setpoplediffBasis(int)));
    connect(ui.poplepolCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setpoplepolBasis(int)));
    connect(ui.poplediffpolCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setpoplediffpolBasis(int)));
    connect(ui.pcCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setpcBasis(int)));
    connect(ui.apcCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setapcBasis(int)));
    connect(ui.ccpvxzCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setccpvxzBasis(int)));
    connect(ui.accpvxzCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setaccpvxzBasis(int)));
    connect(ui.ccpcvxzCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setccpcvxzBasis(int)));
    connect(ui.accpcvxzCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setaccpcvxzBasis(int)));
    connect(ui.basiscoreCheck, SIGNAL(stateChanged(int)),
        this, SLOT(setcoreBasis(int)));
    connect(ui.basisdiffCheck, SIGNAL(stateChanged(int)),
        this, SLOT(setdiffBasis(int)));
    connect(ui.basispolCheck, SIGNAL(stateChanged(int)),
        this, SLOT(setpolBasis(int)));
    connect(ui.directCheck, SIGNAL(stateChanged(int)),
        this, SLOT(setdirectCheck(int)));
    connect(ui.parallelCheck, SIGNAL(stateChanged(int)),
        this, SLOT(setparallelCheck(int)));
    connect(ui.dftgridCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setdftGrid(int)));
    connect(ui.propCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setProp(int)));
    connect(ui.exciSpin, SIGNAL(valueChanged(int)),
        this, SLOT(setExci(int)));
    connect(ui.previewText, SIGNAL(textChanged()),
        this, SLOT(previewEdited()));
    connect(ui.generateButton, SIGNAL(clicked()),
        this, SLOT(generateClicked()));
    connect(ui.resetButton, SIGNAL(clicked()),
        this, SLOT(resetClicked()));
    connect(ui.moreButton, SIGNAL(clicked()),
        this, SLOT(moreClicked()));

    // Generate an initial preview of the input deck
    resetClicked();
    updatePreviewText();
  }

  DaltonInputDialog::~DaltonInputDialog()
  {
  }

  void DaltonInputDialog::showEvent(QShowEvent *)
  {
    updatePreviewText();
  }

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
     ui.moreButton->hide();
     ui.enableFormButton->hide();
     if (m_theoryType != DFT) {
        ui.dftTab->setEnabled(false);
     } else {
        ui.dftTab->setEnabled(true);
     }
     if (m_calculationType != PROP) {
        ui.propsTab->setEnabled(false);
     } else {
        ui.propsTab->setEnabled(true);
     }
     if (m_propType != exci) {
        ui.excitations->hide();
        ui.exciSpin->hide();
     } else {
        ui.excitations->show();
        ui.exciSpin->show();
     }
     if (m_basisType == STOnG) {
       ui.pcCombo->hide();
       ui.apcCombo->hide();
       ui.ccpvxzCombo->hide();
       ui.ccpcvxzCombo->hide();
       ui.accpvxzCombo->hide();
       ui.accpcvxzCombo->hide();
       ui.popleCombo->hide();
       ui.poplediffCombo->hide();
       ui.poplepolCombo->hide();
       ui.poplediffpolCombo->hide();
       ui.stoCombo->show();
       ui.basiscoreCheck->hide();
       ui.basisdiffCheck->hide();
       ui.basispolCheck->hide();
    }
    else if (m_basisType == pople) {
       ui.basiscoreCheck->hide();
       ui.basisdiffCheck->show();
       ui.basispolCheck->show();
       ui.pcCombo->hide();
       ui.apcCombo->hide();
       ui.ccpvxzCombo->hide();
       ui.ccpcvxzCombo->hide();
       ui.accpvxzCombo->hide();
       ui.accpcvxzCombo->hide();
       ui.stoCombo->hide();
       if (m_polBasis == false && m_diffBasis == false) {
          ui.popleCombo->show();
          ui.poplediffCombo->hide();
          ui.poplepolCombo->hide();
          ui.poplediffpolCombo->hide();
       }
       else if (m_polBasis == true && m_diffBasis == false) {
          ui.popleCombo->hide();
          ui.poplediffCombo->hide();
          ui.poplepolCombo->show();
          ui.poplediffpolCombo->hide();
       }
       else if (m_polBasis == false && m_diffBasis == true) {
          ui.popleCombo->hide();
          ui.poplediffCombo->show();
          ui.poplepolCombo->hide();
          ui.poplediffpolCombo->hide();
       }
       else if (m_polBasis == true && m_diffBasis == true) {
          ui.popleCombo->hide();
          ui.poplediffCombo->hide();
          ui.poplepolCombo->hide();
          ui.poplediffpolCombo->show();
       }
    }
    else if (m_basisType == jensen) {
       ui.basiscoreCheck->hide();
       ui.basisdiffCheck->show();
       ui.basispolCheck->hide();
       ui.ccpvxzCombo->hide();
       ui.ccpcvxzCombo->hide();
       ui.accpvxzCombo->hide();
       ui.accpcvxzCombo->hide();
       ui.popleCombo->hide();
       ui.poplediffCombo->hide();
       ui.poplepolCombo->hide();
       ui.poplediffpolCombo->hide();
       ui.stoCombo->hide();
       if (m_diffBasis == false) {
          ui.pcCombo->show();
          ui.apcCombo->hide();
       }
       else if (m_diffBasis == true) {
          ui.pcCombo->hide();
          ui.apcCombo->show();
       }
    }
    else if (m_basisType == dunning) {
       ui.basiscoreCheck->show();
       ui.basisdiffCheck->show();
       ui.basispolCheck->hide();
       ui.pcCombo->hide();
       ui.apcCombo->hide();
       ui.ccpvxzCombo->show();
       ui.ccpcvxzCombo->hide();
       ui.accpvxzCombo->hide();
       ui.accpcvxzCombo->hide();
       ui.popleCombo->hide();
       ui.poplediffCombo->hide();
       ui.poplepolCombo->hide();
       ui.poplediffpolCombo->hide();
       ui.stoCombo->hide();
       if (m_coreBasis == false && m_diffBasis == false) {
          ui.ccpvxzCombo->show();
          ui.ccpcvxzCombo->hide();
          ui.accpvxzCombo->hide();
          ui.accpcvxzCombo->hide();
       }
       else if (m_coreBasis == true && m_diffBasis == false) {
          ui.ccpvxzCombo->hide();
          ui.ccpcvxzCombo->show();
          ui.accpvxzCombo->hide();
          ui.accpcvxzCombo->hide();
       }
       else if (m_coreBasis == false && m_diffBasis == true) {
          ui.ccpvxzCombo->hide();
          ui.ccpcvxzCombo->hide();
          ui.accpvxzCombo->show();
          ui.accpcvxzCombo->hide();
       }
       else if (m_coreBasis == true && m_diffBasis == true) {
          ui.ccpvxzCombo->hide();
          ui.ccpcvxzCombo->hide();
          ui.accpvxzCombo->hide();
          ui.accpcvxzCombo->show();
       }
    }

    if (!isVisible()) return;
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
    ui.functionalCombo->setCurrentIndex(1);
    ui.pcCombo->setCurrentIndex(0);
    ui.apcCombo->setCurrentIndex(0);
    ui.ccpvxzCombo->setCurrentIndex(0);
    ui.ccpcvxzCombo->setCurrentIndex(0);
    ui.accpvxzCombo->setCurrentIndex(0);
    ui.accpcvxzCombo->setCurrentIndex(0);
    ui.popleCombo->setCurrentIndex(0);
    ui.poplediffCombo->setCurrentIndex(0);
    ui.poplepolCombo->setCurrentIndex(0);
    ui.poplediffpolCombo->setCurrentIndex(0);
    ui.stoCombo->setCurrentIndex(0);
    ui.basiscoreCheck->setChecked(false);
    ui.basisdiffCheck->setChecked(false);
    ui.basispolCheck->setChecked(false);
    ui.directCheck->setChecked(false);
    ui.parallelCheck->setChecked(false);
    ui.dftgridCombo->setCurrentIndex(1);
    ui.propCombo->setCurrentIndex(0);
    ui.exciSpin->setValue(1);
  }

  QString DaltonInputDialog::saveInputFile()
  {
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty())
      defaultPath = QDir::homePath();

    QString defaultFileName = defaultPath + '/' + defaultFile.baseName()
       + ".dal";
    QString fileName = QFileDialog::getSaveFileName(this,
       tr("Save Dalton Input File"), defaultFileName,
       tr("Dalton Input File (*.dal)"));

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
        m_theoryType = HF;
        break;
      case 1:
        m_theoryType = DFT;
        break;
      case 2:
        m_theoryType = MP2;
        break;
      default:
        m_theoryType = HF;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setFunctional(int n)
  {
    switch (n)
    {
      case 0:
        m_functionalType = B2PLYP;
        break;
      case 1:
        m_functionalType = B3LYP;
        break;
      case 2:
        m_functionalType = B3LYPg;
        break;
      case 3:
        m_functionalType = B3P86;
        break;
      case 4:
        m_functionalType = B3P86g;
        break;
      case 5:
        m_functionalType = B3PW91;
        break;
      case 6:
        m_functionalType = B1LYP;
        break;
      case 7:
        m_functionalType = B1PW91;
        break;
      case 8:
        m_functionalType = BHandH;
        break;
      case 9:
        m_functionalType = BHandHLYP;
        break;
      case 10:
        m_functionalType = B86VWN;
        break;
      case 11:
        m_functionalType = B86LYP;
        break;
      case 12:
        m_functionalType = B86P86;
        break;
      case 13:
        m_functionalType = B86PW91;
        break;
      case 14:
        m_functionalType = BVWN;
        break;
      case 15:
        m_functionalType = BLYP;
        break;
      case 16:
        m_functionalType = BP86;
        break;
      case 17:
        m_functionalType = BPW91;
        break;
      case 18:
        m_functionalType = BW;
        break;
      case 19:
        m_functionalType = BFW;
        break;
      case 20:
        m_functionalType = CAMB3LYP;
        break;
      case 21:
        m_functionalType = DBLYP;
        break;
      case 22:
        m_functionalType = DBP86;
        break;
      case 23:
        m_functionalType = DBPW91;
        break;
      case 24:
        m_functionalType = EDF1;
        break;
      case 25:
        m_functionalType = EDF2;
        break;
      case 26:
        m_functionalType = G96VWN;
        break;
      case 27:
        m_functionalType = G96LYP;
        break;
      case 28:
        m_functionalType = G96P86;
        break;
      case 29:
        m_functionalType = G96PW91;
        break;
      case 30:
        m_functionalType = G961LYP;
        break;
      case 31:
        m_functionalType = KMLYP;
        break;
      case 32:
        m_functionalType = KT1;
        break;
      case 33:
        m_functionalType = KT2;
        break;
      case 34:
        m_functionalType = KT3;
        break;
      case 35:
        m_functionalType = LDA;
        break;
      case 36:
        m_functionalType = LG1LYP;
        break;
      case 37:
        m_functionalType = OVWN;
        break;
      case 38:
        m_functionalType = OLYP;
        break;
      case 39:
        m_functionalType = OP86;
        break;
      case 40:
        m_functionalType = OPW91;
        break;
      case 41:
        m_functionalType = mPWVWN;
        break;
      case 42:
        m_functionalType = mPWLYP;
        break;
      case 43:
        m_functionalType = mPWP86;
        break;
      case 44:
        m_functionalType = mPWPW91;
        break;
      case 45:
        m_functionalType = mPW91;
        break;
      case 46:
        m_functionalType = mPW1PW91;
        break;
      case 47:
        m_functionalType = mPW3PW91;
        break;
      case 48:
        m_functionalType = mPW1K;
        break;
      case 49:
        m_functionalType = mPW1N;
        break;
      case 50:
        m_functionalType = mPW1S;
        break;
      case 51:
        m_functionalType = PBE0;
        break;
      case 52:
        m_functionalType = PBE0PBE;
        break;
      case 53:
        m_functionalType = PBE1PBE;
        break;
      case 54:
        m_functionalType = PBE;
        break;
      case 55:
        m_functionalType = PBEPBE;
        break;
      case 56:
        m_functionalType = RPBE;
        break;
      case 57:
        m_functionalType = revPBE;
        break;
      case 58:
        m_functionalType = mPBE;
        break;
      case 59:
        m_functionalType = PW91;
        break;
      case 60:
        m_functionalType = PW91VWN;
        break;
      case 61:
        m_functionalType = PW91LYP;
        break;
      case 62:
        m_functionalType = PW91P86;
        break;
      case 63:
        m_functionalType = PW91PW91;
        break;
      case 64:
        m_functionalType = SVWN3;
        break;
      case 65:
        m_functionalType = SVWN5;
        break;
      case 66:
        m_functionalType = XLYP;
        break;
      case 67:
        m_functionalType = X3LYP;
        break;
      default:
        m_functionalType = B3LYP;
    }

    updatePreviewText();
  }

  void DaltonInputDialog::setBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_basisType = STOnG;
        break;
      case 1:
        m_basisType = pople;
        break;
      case 2:
        m_basisType = jensen;
        break;
      case 3:
        m_basisType = dunning;
        break;
      default:
        m_basisType = STOnG;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setstoBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_stoBasis = STO2G;
        break;
      case 1:
        m_stoBasis = STO3G;
        break;
      case 2:
        m_stoBasis = STO6G;
        break;
      default:
        m_stoBasis = STO2G;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setpopleBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_popleBasis = p321G;
        break;
      case 1:
        m_popleBasis = p431G;
        break;
      case 2:
        m_popleBasis = p631G;
        break;
      case 3:
        m_popleBasis = p6311G;
        break;
      default:
        m_popleBasis = p321G;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setpoplediffBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_poplediffBasis = p321ppG;
        break;
      case 1:
        m_poplediffBasis = p631pG;
        break;
      case 2:
        m_poplediffBasis = p631ppG;
        break;
      default:
        m_poplediffBasis = p321ppG;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setpoplepolBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_poplepolBasis = p321Gs;
        break;
      case 1:
        m_poplepolBasis = p631Gs;
        break;
      case 2:
        m_poplepolBasis = p631Gss;
        break;
      case 3:
        m_poplepolBasis = p631G33;
        break;
      case 4:
        m_poplepolBasis = p6311Gs;
        break;
      case 5:
        m_poplepolBasis = p6311Gss;
        break;
      case 6:
        m_poplepolBasis = p6311G22;
        break;
      default:
        m_poplepolBasis = p321Gs;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setpoplediffpolBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_poplediffpolBasis = p321ppGs;
        break;
      case 1:
        m_poplediffpolBasis = p631pGs;
        break;
      case 2:
        m_poplediffpolBasis = p631ppGs;
        break;
      case 3:
        m_poplediffpolBasis = p631ppGss;
        break;
      case 4:
        m_poplediffpolBasis = p6311pGs;
        break;
      case 5:
        m_poplediffpolBasis = p6311ppGss;
        break;
      case 6:
        m_poplediffpolBasis = p6311ppG22;
        break;
      case 7:
        m_poplediffpolBasis = p6311ppG33;
        break;
      default:
        m_poplediffpolBasis = p321ppGs;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setpcBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_pcBasis = pc0;
        break;
      case 1:
        m_pcBasis = pc1;
        break;
      case 2:
        m_pcBasis = pc2;
        break;
      case 3:
        m_pcBasis = pc3;
        break;
      case 4:
        m_pcBasis = pc4;
        break;
      default:
        m_pcBasis = pc0;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setapcBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_apcBasis = apc0;
        break;
      case 1:
        m_apcBasis = apc1;
        break;
      case 2:
        m_apcBasis = apc2;
        break;
      case 3:
        m_apcBasis = apc3;
        break;
      case 4:
        m_apcBasis = apc4;
        break;
      default:
        m_apcBasis = apc0;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setccpvxzBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_ccpvxzBasis = ccpVDZ;
        break;
      case 1:
        m_ccpvxzBasis = ccpVTZ;
        break;
      case 2:
        m_ccpvxzBasis = ccpVQZ;
        break;
      case 3:
        m_ccpvxzBasis = ccpV5Z;
        break;
      case 4:
        m_ccpvxzBasis = ccpV6Z;
        break;
      default:
        m_ccpvxzBasis = ccpVDZ;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setaccpvxzBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_accpvxzBasis = accpVDZ;
        break;
      case 1:
        m_accpvxzBasis = accpVTZ;
        break;
      case 2:
        m_accpvxzBasis = accpVQZ;
        break;
      case 3:
        m_accpvxzBasis = accpV5Z;
        break;
      case 4:
        m_accpvxzBasis = accpV6Z;
        break;
      default:
        m_accpvxzBasis = accpVDZ;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setccpcvxzBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_ccpcvxzBasis = ccpCVDZ;
        break;
      case 1:
        m_ccpcvxzBasis = ccpCVTZ;
        break;
      case 2:
        m_ccpcvxzBasis = ccpCVQZ;
        break;
      case 3:
        m_ccpcvxzBasis = ccpCV5Z;
        break;
      case 4:
        m_ccpcvxzBasis = ccpwCVDZ;
        break;
      case 5:
        m_ccpcvxzBasis = ccpwCVTZ;
        break;
      case 6:
        m_ccpcvxzBasis = ccpwCVQZ;
        break;
      case 7:
        m_ccpcvxzBasis = ccpwCV5Z;
        break;
      default:
        m_ccpcvxzBasis = ccpCVDZ;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setaccpcvxzBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_accpcvxzBasis = accpCVDZ;
        break;
      case 1:
        m_accpcvxzBasis = accpCVTZ;
        break;
      case 2:
        m_accpcvxzBasis = accpCVQZ;
        break;
      default:
        m_accpcvxzBasis = accpCVDZ;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setcoreBasis(int n)
  {
    if (n) m_coreBasis = true;
    else m_coreBasis = false;
    updatePreviewText();
  }

  void DaltonInputDialog::setdiffBasis(int n)
  {
    if (n) m_diffBasis = true;
    else m_diffBasis = false;
    updatePreviewText();
  }

  void DaltonInputDialog::setpolBasis(int n)
  {
    if (n) m_polBasis = true;
    else m_polBasis = false;
    updatePreviewText();
  }

  void DaltonInputDialog::setdirectCheck(int n)
  {
    if (n) m_directCheck = true;
    else m_directCheck = false;
    updatePreviewText();
  }

  void DaltonInputDialog::setparallelCheck(int n)
  {
    if (n) m_parallelCheck = true;
    else m_parallelCheck = false;
    updatePreviewText();
  }

  void DaltonInputDialog::setdftGrid(int n)
  {
    switch (n)
    {
      case 0:
        m_dftGrid = coarse;
        break;
      case 1:
        m_dftGrid = normal;
        break;
      case 2:
        m_dftGrid = fine;
        break;
      case 3:
        m_dftGrid = ultrafine;
        break;
      default:
        m_dftGrid = normal;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setProp(int n)
  {
    switch (n)
    {
      case 0:
        m_propType = polari;
        break;
      case 1:
        m_propType = exci;
        break;
      default:
        m_propType = polari;
    }
    updatePreviewText();
  }

  void DaltonInputDialog::setExci(int n)
  {
    m_exci = n;
    updatePreviewText();
  }

  QString DaltonInputDialog::generateInputDeck()
  {
    QString buffer;
    QTextStream mol(&buffer);

    int newatomtype;
    int oldatomtype;
    int natomtypes;
    int* natoms = NULL;

    mol << "BASIS" << '\n';
    if (m_basisType == STOnG) {
       mol << getstoBasis(m_stoBasis) << '\n';
    }
    else if (m_basisType == pople) {
       if (m_polBasis == false && m_diffBasis == false) {
          mol << getpopleBasis(m_popleBasis) << '\n';
       }
       else if (m_polBasis == true && m_diffBasis == false) {
          mol << getpoplepolBasis(m_poplepolBasis) << '\n';
       }
       else if (m_polBasis == false && m_diffBasis == true) {
          mol << getpoplediffBasis(m_poplediffBasis) << '\n';
       }
       else if (m_polBasis == true && m_diffBasis == true) {
          mol << getpoplediffpolBasis(m_poplediffpolBasis) << '\n';
       }
    }
    else if (m_basisType == jensen) {
       if (m_diffBasis == false) {
          mol << getpcBasis(m_pcBasis) << '\n';
       }
       else if (m_diffBasis == true) {
          mol << getapcBasis(m_apcBasis) << '\n';
       }
    }
    else if (m_basisType == dunning) {
       if (m_coreBasis == false && m_diffBasis == false) {
          mol << getccpvxzBasis(m_ccpvxzBasis) << '\n';
       }
       else if (m_coreBasis == true && m_diffBasis == false) {
          mol << getccpcvxzBasis(m_ccpcvxzBasis) << '\n';
       }
       else if (m_coreBasis == false && m_diffBasis == true) {
          mol << getaccpvxzBasis(m_accpvxzBasis) << '\n';
       }
       else if (m_coreBasis == true && m_diffBasis == true) {
          mol << getaccpcvxzBasis(m_accpcvxzBasis) << '\n';
       }
    }

    mol << " Generated by the Dalton Input File Plugin for Avogadro" << '\n';
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

    mol << "Atomtypes=" << natomtypes << " Angstrom";
    
    if (m_propType == exci) {
       mol << " Nosymm" << '\n';
    } else {
       mol << '\n';
    }

    newatomtype = 0;
    oldatomtype = 0;
    natomtypes = 0;

    foreach (Atom *atom, atoms) {
      newatomtype = atom->atomicNumber();
      if (newatomtype != oldatomtype) {
        natomtypes += 1;
        mol << "Charge=" << atom->atomicNumber() << ".0" << " Atoms="
            << natoms[natomtypes] << '\n';
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
    if (m_directCheck == true)
       mol << ".DIRECT" << '\n';
    if (m_parallelCheck == true)
       mol << ".PARALLEL" << '\n';
    mol << "**WAVE FUNCTIONS" << '\n';
    if (m_theoryType == MP2) {
       mol << ".HF" << '\n';
       mol << getTheoryType(m_theoryType) << '\n';
    } else if (m_theoryType == DFT) {
       mol << getTheoryType(m_theoryType) << '\n';
       mol << " " << getFunctionalType(m_functionalType) << '\n';
       if (m_dftGrid != normal) {
          mol << "*DFT INPUT" << '\n';
          mol << getdftGrid(m_dftGrid) << '\n';
       }
    } else {
      mol << getTheoryType(m_theoryType) << '\n';
    }
    if (m_calculationType == PROP) {
       mol << "**PROPERTIES" << '\n';
       mol << getPropType(m_propType) << '\n';
       if (m_propType == exci) {
          mol << " " << m_exci << '\n';
       }
    }
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
      case HF:
        return ".HF";
      case DFT:
        return ".DFT";
      case MP2:
        return ".MP2";
      default:
        return ".DFT";
    }
  }

  QString DaltonInputDialog::getFunctionalType(functionalType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case B2PLYP:
        return "B2PLYP";
      case B3LYP:
        return "B3LYP";
      case B3LYPg:
        return "B3LYPg";
      case B3P86:
        return "B3P86";
      case B3P86g:
        return "B3P86g";
      case B3PW91:
        return "B3PW91";
      case B1LYP:
        return "B1LYP";
      case B1PW91:
        return "B1PW91";
      case BHandH:
        return "BHandH";
      case BHandHLYP:
        return "BHandHLYP";
      case B86VWN:
        return "B86VWN";
      case B86LYP:
        return "B86LYP";
      case B86P86:
        return "B86P86";
      case B86PW91:
        return "B86PW91";
      case BVWN:
        return "BVWN";
      case BLYP:
        return "BLYP";
      case BP86:
        return "BP86";
      case BPW91:
        return "BPW91";
      case BW:
        return "BW";
      case BFW:
        return "BFW";
      case CAMB3LYP:
        return "CAMB3LYP";
      case DBLYP:
        return "DBLYP";
      case DBP86:
        return "DBP86";
      case DBPW91:
        return "DBPW91";
      case EDF1:
        return "EDF1";
      case EDF2:
        return "EDF2";
      case G96VWN:
        return "G96VWN";
      case G96LYP:
        return "G96LYP";
      case G96P86:
        return "G96P86";
      case G96PW91:
        return "G96PW91";
      case G961LYP:
        return "G961LYP";
      case KMLYP:
        return "KMLYP";
      case KT1:
        return "KT1";
      case KT2:
        return "KT2";
      case KT3:
        return "KT3";
      case LDA:
        return "LDA";
      case LG1LYP:
        return "LG1LYP";
      case OVWN:
        return "OVWN";
      case OLYP:
        return "OLYP";
      case OP86:
        return "OP86";
      case OPW91:
        return "OPW91";
      case mPWVWN:
        return "mPWVWN";
      case mPWLYP:
        return "mPWLYP";
      case mPWP86:
        return "mPWP86";
      case mPWPW91:
        return "mPWPW91";
      case mPW91:
        return "mPW91";
      case mPW1PW91:
        return "mPW1PW91";
      case mPW3PW91:
        return "mPW3PW91";
      case mPW1K:
        return "mPW1K";
      case mPW1N:
        return "mPW1N";
      case mPW1S:
        return "mPW1S";
      case PBE0:
        return "PBE0";
      case PBE0PBE:
        return "PBE0PBE";
      case PBE1PBE:
        return "PBE1PBE";
      case PBE:
        return "PBE";
      case PBEPBE:
        return "PBEPBE";
      case RPBE:
        return "RPBE";
      case revPBE:
        return "revPBE";
      case mPBE:
        return "mPBE";
      case PW91:
        return "PW91";
      case PW91VWN:
        return "PW91VWN";
      case PW91LYP:
        return "PW91LYP";
      case PW91P86:
        return "PW91P86";
      case PW91PW91:
        return "PW91PW91";
      case SVWN3:
        return "SVWN3";
      case SVWN5:
        return "SVWN5";
      case XLYP:
        return "XLYP";
      case X3LYP:
        return "X3LYP";
      default:
        return "B3LYP";
    }
  }

  QString DaltonInputDialog::getstoBasis(stoBasis t)
  {
// Translate the enum to text for the output generation
    switch (t)
    {
      case STO2G:
        return "STO-2G";
      case STO3G:
        return "STO-3G";
      case STO6G:
        return "STO-6G";
      default:
        return "STO-2G";
    }
  }

  QString DaltonInputDialog::getpopleBasis(popleBasis t)
  {
    switch (t)
    {
      case p321G:
        return "3-21G";
      case p431G:
        return "4-31G";
      case p631G:
        return "6-31G";
      case p6311G:
        return "6-311G";
      default:
        return "3-21G";
    }
  }

  QString DaltonInputDialog::getpoplediffBasis(poplediffBasis t)
  {
    switch (t)
    {
      case p321ppG:
        return "3-21++G";
      case p631pG:
        return "6-31+G";
      case p631ppG:
        return "6-31++G";
      default:
        return "3-21++G";
    }
  }

  QString DaltonInputDialog::getpoplepolBasis(poplepolBasis t)
  {
    switch (t)
    {
      case p321Gs:
        return "3-21G*";
      case p631Gs:
        return "6-31G*";
      case p631Gss:
        return "6-31G**";
      case p631G33:
        return "6-31G(3df,3pd)";
      case p6311Gs:
        return "6-311G*";
      case p6311Gss:
        return "6-311G**";
      case p6311G22:
        return "6-311G(2df,2pd)";
      default:
        return "3-21G*";
    }
  }

  QString DaltonInputDialog::getpoplediffpolBasis(poplediffpolBasis t)
  {
    switch (t)
    {
      case p321ppGs:
        return "3-21++G*";
      case p631pGs:
        return "6-31+G*";
      case p631ppGs:
        return "6-31++G*";
      case p631ppGss:
        return "6-31++G**";
      case p6311pGs:
        return "6-311+G*";
      case p6311ppGss:
        return "6-311++G**";
      case p6311ppG22:
        return "6-311++G(2d,2p)";
      case p6311ppG33:
        return "6-311++G(3df,3pd)";
      default:
        return "3-21++G*";
    }
  }

  QString DaltonInputDialog::getpcBasis(pcBasis t)
  {
    switch (t)
    {
      case pc0:
        return "pc-0";
      case pc1:
        return "pc-1";
      case pc2:
        return "pc-2";
      case pc3:
        return "pc-3";
      case pc4:
        return "pc-4";
      default:
        return "pc-0";
    }
  }

  QString DaltonInputDialog::getapcBasis(apcBasis t)
  {
    switch (t)
    {
      case apc0:
        return "apc-0";
      case apc1:
        return "apc-1";
      case apc2:
        return "apc-2";
      case apc3:
        return "apc-3";
      case apc4:
        return "apc-4";
      default:
        return "apc-0";
    }
  }

  QString DaltonInputDialog::getccpvxzBasis(ccpvxzBasis t)
  {
    switch (t)
    {
      case ccpVDZ:
        return "cc-pVDZ";
      case ccpVTZ:
        return "cc-pVTZ";
      case ccpVQZ:
        return "cc-pVQZ";
      case ccpV5Z:
        return "cc-pV5Z";
      case ccpV6Z:
        return "cc-pV6Z";
      default:
        return "cc-pVDZ";
    }
  }

  QString DaltonInputDialog::getaccpvxzBasis(accpvxzBasis t)
  {
    switch (t)
    {
      case accpVDZ:
        return "aug-cc-pVDZ";
      case accpVTZ:
        return "aug-cc-pVTZ";
      case accpVQZ:
        return "aug-cc-pVQZ";
      case accpV5Z:
        return "aug-cc-pV5Z";
      case accpV6Z:
        return "aug-cc-pV6Z";
      default:
        return "aug-cc-pVDZ";
    }
  }

  QString DaltonInputDialog::getccpcvxzBasis(ccpcvxzBasis t)
  {
    switch (t)
    {
      case ccpCVDZ:
        return "cc-pCVDZ";
      case ccpCVTZ:
        return "cc-pCVTZ";
      case ccpCVQZ:
        return "cc-pCVQZ";
      case ccpCV5Z:
        return "cc-pCV5Z";
      case ccpwCVDZ:
        return "cc-pwCVDZ";
      case ccpwCVTZ:
        return "cc-pwCVTZ";
      case ccpwCVQZ:
        return "cc-pwCVQZ";
      case ccpwCV5Z:
        return "cc-pwCV5Z";
      default:
        return "cc-pCVDZ";
    }
  }

  QString DaltonInputDialog::getaccpcvxzBasis(accpcvxzBasis t)
  {
    switch (t)
    {
      case accpCVDZ:
        return "aug-cc-pCVDZ";
      case accpCVTZ:
        return "aug-cc-pCVTZ";
      case accpCVQZ:
        return "aug-cc-pCVQZ";
      default:
        return "aug-cc-pCVDZ";
    }
  }

  QString DaltonInputDialog::getdftGrid(dftGrid t)
  {
    switch (t)
    {
      case coarse:
        return ".COARSE";
      case normal:
        return ".NORMAL";
      case fine:
        return ".FINE";
      case ultrafine:
        return ".ULTRAFINE";
      default:
        return ".NORMAL";
    }
  }

  QString DaltonInputDialog::getPropType(propType t)
  {
    switch (t)
    {
      case polari:
        return ".POLARI";
      case exci:
        return ".EXCITA";
      default:
        return ".POLARI";
    }
  }

 void DaltonInputDialog::deckDirty(bool dirty)
  {
    m_dirty = dirty;
    ui.titleLine->setEnabled(!dirty);
    ui.calculationCombo->setEnabled(!dirty);
    ui.theoryCombo->setEnabled(!dirty);
    ui.functionalCombo->setEnabled(!dirty);
    ui.basisCombo->setEnabled(!dirty);
    ui.pcCombo->setEnabled(!dirty);
    ui.apcCombo->setEnabled(!dirty);
    ui.ccpvxzCombo->setEnabled(!dirty);
    ui.ccpcvxzCombo->setEnabled(!dirty);
    ui.accpvxzCombo->setEnabled(!dirty);
    ui.accpcvxzCombo->setEnabled(!dirty);
    ui.popleCombo->setEnabled(!dirty);
    ui.poplediffCombo->setEnabled(!dirty);
    ui.poplepolCombo->setEnabled(!dirty);
    ui.poplediffpolCombo->setEnabled(!dirty);
    ui.stoCombo->setEnabled(!dirty);
    ui.basiscoreCheck->setEnabled(!dirty);
    ui.basisdiffCheck->setEnabled(!dirty);
    ui.basispolCheck->setEnabled(!dirty);
    ui.directCheck->setEnabled(!dirty);
    ui.parallelCheck->setEnabled(!dirty);
    ui.dftgridCombo->setEnabled(!dirty);
    ui.propCombo->setEnabled(!dirty);
  }

}

