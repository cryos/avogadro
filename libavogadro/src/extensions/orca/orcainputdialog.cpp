/**********************************************************************
  OrcaInputDialog - ORCA Input Deck Dialog for Avogadro

  Copyright (C) 2014 Dagmar Lenk

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

#include "orcainputdialog.h"
#include "orcaextension.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/periodictableview.h>

#include <openbabel/mol.h>

#include <Eigen/Geometry>

#include <vector>

#include <QtGui>

#include <QString>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>

using namespace OpenBabel;
using namespace Eigen;
using namespace std;

namespace Avogadro {

OrcaInputDialog::OrcaInputDialog(QWidget *parent, Qt::WindowFlags f ) :
    QDialog( parent, f ), m_molecule(NULL), m_scfConvButtons(NULL), m_scfConv2ndButtons(NULL),
    m_output(), m_savePath(), m_dirty(false), m_warned(false)
{
    basicData = new OrcaBasicData;
    basisData = new OrcaBasisData;
    controlData = new OrcaControlData;
    scfData = new OrcaSCFData;
    dftData = new OrcaDFTData;
    dataData = new OrcaDataData;
    cosXData = new OrcaCosXData;

    // This initializes the ui member function to contain pointers to
    // all GUI elements in the orcainputdialog.ui file
    ui.setupUi(this);

    // write items into comboboxes

    initComboboxes();

    // init dialog boxes

    initBasicData();
    initBasisData();
    initControlData();
    initSCFData();
    initDFTData();
    initCosXData();
    initDataData();


    ui.modeTabWidget->setCurrentIndex(0);
    ui.advancedStacked->setCurrentIndex(0);
    ui.advancedTree->expandAll();
    ui.advancedTree->setCurrentItem(ui.advancedTree->topLevelItem(0));

    // Connect the GUI elements to the correct slots

    connectModes();
    connectBasic();
    connectAdvanced();
    connectPreview();
    connectButtons();

    QSettings settings;
    readSettings(settings);

    // Enable/Disable Widgets

    bool auxNeeded;
    if (controlData->cosXEnabled() || controlData->dftEnabled()) {
        auxNeeded = true;
    } else {
        auxNeeded = false;
    }
    ui.basisAuxBasisSetCombo->setEnabled(auxNeeded);
    ui.basisAuxECPCheck->setEnabled(false);

    bool auxCorrNeeded;
    if (controlData->mp2Enabled() || controlData->ccsdEnabled()) {
        auxCorrNeeded = true;
    } else {
        auxCorrNeeded = false;
    }
    ui.basisAuxCorrBasisSetCombo->setEnabled(auxCorrNeeded);
    ui.basisAuxCorrECPCheck->setEnabled(false);
    m_basic = true;
    m_advanced = false;
}

  OrcaInputDialog::~OrcaInputDialog()
  {
      QSettings settings;
      writeSettings(settings);   
      delete basicData;
      delete basisData;
      delete controlData;
      delete scfData;
      delete dftData;
      delete dataData;
      delete cosXData;
  }
  void OrcaInputDialog::showEvent(QShowEvent *)
  {
    // Generate an initial preview of the input deck
    updatePreviewText();
  }

  void OrcaInputDialog::connectModes()
  {
      connect( ui.modeTabWidget, SIGNAL( currentChanged( int ) ), this, SLOT( setMode( int ) ) );
  }

  void OrcaInputDialog::connectAdvanced()
  {
      // Advanced Basis Set Slots
      connect (ui.basisBasisSetCombo, SIGNAL(currentIndexChanged(int)), this,  SLOT (setBasisBasisSet(int )));
      connect (ui.basisAuxBasisSetCombo, SIGNAL(currentIndexChanged(int)), this, SLOT( setBasisAuxBasisSet (int )));
      connect (ui.basisAuxCorrBasisSetCombo, SIGNAL(currentIndexChanged(int)), this, SLOT( setBasisAuxCorrBasisSet (int )));
      connect (ui.basisECPCheck, SIGNAL(toggled(bool)), this, SLOT(setBasisUseEPC (bool )));
      connect (ui.basisAuxECPCheck, SIGNAL(toggled (bool)), this, SLOT( setBasisUseAuxEPC (bool )));
      connect (ui.basisAuxCorrECPCheck, SIGNAL(toggled (bool)), this, SLOT( setBasisUseAuxCorrEPC (bool )));
      connect (ui.basisRelativisticGroup, SIGNAL(toggled(bool)), this, SLOT(setBasisUseRel(bool)));
      connect (ui.basisRelativisticCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setBasisRel(int )));
      connect (ui.basisDKHSpin, SIGNAL(valueChanged(int)), this, SLOT(setBasisDKHOrder(int)));

      // Advanced Control Slots
      connect(ui.advancedTree, SIGNAL(clicked(QModelIndex)), this, SLOT(advancedItemClicked(QModelIndex)));

      connect(ui.controlRunTypeCombo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setControlCalculation(int)));
      connect(ui.controlMultiplicitySpin, SIGNAL(valueChanged(int)),
              this, SLOT(setControlMultiplicity(int)));
      connect(ui.controlChargeSpin, SIGNAL(valueChanged(int)),
              this, SLOT(setControlCharge(int)));



      connect( ui.controlCosXCheck, SIGNAL( toggled( bool ) ), this, SLOT( setControlUseCosX( bool ) ) );
      connect( ui.controlDFTCheck, SIGNAL( toggled( bool ) ), this, SLOT( setControlUseDFT( bool ) ) );
      connect( ui.controlMP2Check, SIGNAL( toggled( bool ) ),this, SLOT( setControlUseMP2( bool ) ) );
      connect( ui.controlCCSDCheck, SIGNAL( toggled( bool ) ),this, SLOT( setControlUseCCSD( bool ) ) );

      // Advanced SCF Slots

      connect (ui.scfDampingGroup, SIGNAL(toggled(bool)), this, SLOT( setSCFUseDamping(bool)));
      connect (ui.scfLevelShiftGroup, SIGNAL(toggled(bool)), this, SLOT(setSCFUseLevelShift(bool)));
      connect (ui.scfDampFactorDSpin, SIGNAL(valueChanged(double)), this, SLOT(setSCFDampFactor (double)));
      connect (ui.scfLevelShiftDSpin, SIGNAL(valueChanged(double)), this, SLOT(setSCFLevelShift(double)));
      connect (ui.scfDampErrorDSpin, SIGNAL(valueChanged(double)), this, SLOT(setSCFDampError (double)));
      connect (ui.scfLevelErrorDSpin, SIGNAL(valueChanged(double)), this, SLOT(setSCFLevelError(double)));
      connect (ui.scfAccCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setSCFAccuracy(int)));

      connect (ui.scfTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setSCFType(int)));
      connect (ui.scfMaxIterSpin, SIGNAL(valueChanged(int)), this , SLOT (setSCFMaxIter(int)));

      connect (m_scfConvButtons, SIGNAL(buttonClicked (int)), this, SLOT(setSCFConverger(int)));
      connect (m_scfConv2ndButtons, SIGNAL(buttonClicked (int)), this, SLOT(setSCF2ndConverger(int)));

      // Advanced DFT Slots

//      connect (ui.dftFinalGridcheck, SIGNAL(toggled(bool)), this, SLOT(setDFTUseFinalGrid(bool)));
      connect (ui.dftFinalGridCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setDFTFinalGrid(int)));
      connect (ui.dftGridCombo, SIGNAL(currentIndexChanged(int)), this,SLOT(setDFTGrid(int)));
      connect (ui.dftFunctionalCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setDFTFunctional(int)));

      // Advanced CosX Slots

//      connect (ui.cosXFinalGridCheck, SIGNAL(toggled(bool)), this, SLOT(setCosXUseFinalGrid(bool)));
      connect (ui.cosXFinalGridCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCosXFinalGrid(int)));
      connect (ui.cosXGridCombo, SIGNAL(currentIndexChanged(int)), this,SLOT(setCosXGrid(int)));
      connect (ui.cosXSFittingCheck, SIGNAL(toggled(bool)), this, SLOT(setCosXSFitting(bool)));

      // Advanced Data Slots
      connect(ui.dataFormatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT (setDataFormat(int)));
      connect(ui.dataCommentLine, SIGNAL(editingFinished()), this, SLOT(setDataComment()));
      connect(ui.dataPrintCombo, SIGNAL(currentIndexChanged(int)), this, SLOT (setPrintLevel(int)));
      connect(ui.MOPrintCheck, SIGNAL(toggled(bool)), this, SLOT(setMOPrint(bool)));
      connect(ui.basisPrintCheck, SIGNAL(toggled(bool)),this, SLOT(setBasisPrint(bool)));
  }


  void OrcaInputDialog::connectPreview()
  {
      connect(ui.previewText, SIGNAL(cursorPositionChanged()),
              this, SLOT(previewEdited()));

      connect(ui.hideButton, SIGNAL(clicked()),
              this, SLOT(hideClicked()));
  }

  void OrcaInputDialog::connectButtons()
  {
      connect(ui.generateButton, SIGNAL(clicked()),
              this, SLOT(generateClicked()));
      connect(ui.resetButton, SIGNAL(clicked()),
              this, SLOT(resetClicked()));
  }

  void OrcaInputDialog::connectBasic()
  {
      connect(ui.basicCommentLine, SIGNAL(editingFinished()),
              this, SLOT(setBasicComment()));
      connect(ui.basicCalculationCombo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setBasicCalculation(int)));
      connect(ui.basicMethodCombo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setBasicMethod(int)));
      connect(ui.basicBasisSetCombo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setBasicBasis(int)));
      connect(ui.basicMultiplicitySpin, SIGNAL(valueChanged(int)),
              this, SLOT(setBasicMultiplicity(int)));

      connect(ui.basicChargeSpin, SIGNAL(valueChanged(int)),
              this, SLOT(setBasicCharge(int)));

      connect(ui.basicFormatCombo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setBasicCoordsFormat(int)));
  }

  void OrcaInputDialog::advancedItemClicked(const QModelIndex &index )
  {
      int i = index.row();

      QModelIndex parent = index.parent();
      if(parent.isValid())
      {
        i += ui.advancedTree->topLevelItemCount();
      }

      ui.advancedStacked->setCurrentIndex(i);
  }

  void OrcaInputDialog::initComboboxes()
  {
      meta = new QMetaObject (OrcaExtension::staticMetaObject);
      QStringList items;
      for (int i=0; i < meta->enumeratorCount(); ++i) {
          items.clear();
          QMetaEnum m = meta->enumerator(i);
          qDebug() << "//" << m.name() << "//";
          QString enumType = m.name();
          if (enumType == "DFTFunctionalType") {
              dftData->setEnumDFT(m);
              for (int j=0; j<m.keyCount();j++) {
                  items += QLatin1String(m.valueToKey(j));
              }
              ui.dftFunctionalCombo->addItems(items);
          } else if (enumType == "basisType") {
              basicData->setEnumBasis(m);
              basisData->setEnumBasis(m);

              for (int j=0; j<m.keyCount();j++) {
                  items += QLatin1String(m.valueToKey(j));
                  items[j].prepend("def2-");
              }
              ui.basicBasisSetCombo->addItems(items);
              ui.basisBasisSetCombo->addItems(items);
              for (int j=0; j<items.size();j++) {
                  items[j].append("/J");
              }
              ui.basisAuxBasisSetCombo->addItems(items);

              items.replaceInStrings("/J", "/C");

              ui.basisAuxCorrBasisSetCombo->addItems(items);
          } else if (enumType == "gridType") {

              dftData->setEnumGrid(m);
              cosXData->setEnumGridX(m);
              for (int j=0; j<m.keyCount();j++) {
                  items += QLatin1String(m.valueToKey(j));
              }

              items.replaceInStrings("Grid", "");

              ui.dftGridCombo->addItems(items);
              ui.cosXGridCombo->addItems(items);
          } else if (enumType == "finalgridType") {

              dftData->setEnumFinalGrid(m);
              cosXData->setEnumFinalGridX(m);

              for (int j=0; j<m.keyCount();j++) {
                  items += QLatin1String(m.valueToKey(j));
              }

              items.replaceInStrings("fNone", "None");
              items.replaceInStrings("fDefault", "Default");
              items.replaceInStrings("fGrid", "");
              ui.dftFinalGridCombo->addItems(items);
              ui.cosXFinalGridCombo->addItems(items);
          }
      }
  }
  void OrcaInputDialog::initBasicData()
  {
      ui.basicCalculationCombo->setCurrentIndex(basicData->getCalculation());
      ui.basicMethodCombo->setCurrentIndex(basicData->getMethod ());
      ui.basicBasisSetCombo->setCurrentIndex(basicData->getBasis ());
      ui.basicChargeSpin->setValue(basicData->getCharge());
      ui.basicMultiplicitySpin->setValue(basicData->getMultiplicity());
      ui.basicFormatCombo->setCurrentIndex(basicData->getFormat());

  }

  void OrcaInputDialog::initBasisData()
  {
      ui.basisBasisSetCombo->setCurrentIndex(basisData->getBasis());
      ui.basisAuxBasisSetCombo->setCurrentIndex(basisData->getAuxBasis());
      ui.basisAuxCorrBasisSetCombo->setCurrentIndex(basisData->getAuxCorrBasis());

      ui.basisECPCheck->setChecked(basisData->EPCEnabled());
      ui.basisAuxECPCheck->setChecked(basisData->auxEPCEnabled());
      ui.basisAuxCorrECPCheck->setChecked(basisData->auxCorrEPCEnabled());

      ui.basisRelativisticGroup->setChecked(basisData->relEnabled());
      ui.basisRelativisticCombo->setEnabled(basisData->relEnabled());
      ui.basisRelativisticCombo->setCurrentIndex(basisData->getRel());
      ui.basisDKHSpin->setVisible(basisData->dkhEnabled());
      ui.basisDKHLabel->setVisible(basisData->dkhEnabled());
  }

  void OrcaInputDialog::initControlData()
  {
      ui.controlChargeSpin->setValue(controlData->getCharge());
      ui.controlMultiplicitySpin->setValue((controlData->getMultiplicity()));

      ui.controlRunTypeCombo->setCurrentIndex(controlData->getCalculation());
      ui.controlCosXCheck->setChecked(controlData->cosXEnabled());
      ui.controlDFTCheck->setChecked(controlData->dftEnabled());
      ui.controlMP2Check->setChecked(controlData->mp2Enabled());
      ui.controlCCSDCheck->setChecked(controlData->ccsdEnabled());
  }

  void OrcaInputDialog::initSCFData()
  {
      ui.scfAccCombo->setCurrentIndex(scfData->getAccuracy());
      ui.scfTypeCombo->setCurrentIndex(scfData->getType());
      ui.scfMaxIterSpin->setValue(scfData->getMaxIter());

      ui.scfLevelShiftGroup->setChecked(scfData->levelShiftEnabled());
      ui.scfDampingGroup->setChecked(scfData->dampingEnabled());

      ui.scfLevelShiftDSpin->setValue(scfData->getLevelShift());
      ui.scfLevelErrorDSpin->setValue(scfData->getLevelError());

      ui.scfDampFactorDSpin->setValue(scfData->getDampFactor());
      ui.scfDampErrorDSpin->setValue(scfData->getDampError());

      if (m_scfConvButtons == NULL) {
          m_scfConvButtons = new QButtonGroup;
          m_scfConvButtons->addButton(ui.scfDIISRadio, 0);
          m_scfConvButtons->addButton(ui.scfKDIISRadio, 1);
      }
      if (m_scfConv2ndButtons == NULL) {
          m_scfConv2ndButtons = new QButtonGroup;
          m_scfConv2ndButtons->addButton(ui.scfSOSCFRadio, 0);
          m_scfConv2ndButtons->addButton(ui.scfNRSCFRadio, 1);
          m_scfConv2ndButtons->addButton(ui.scfAHSCFRadio, 2);
      }

      QRadioButton *convButton =
        qobject_cast<QRadioButton *>( m_scfConvButtons->button( scfData->getConv() ));

      if ( convButton ) {
          convButton->setChecked( true );
      }

      QRadioButton *conv2ndButton =
        qobject_cast<QRadioButton *>( m_scfConv2ndButtons->button(scfData->getConv2nd()) );

      if ( conv2ndButton ) {
          conv2ndButton->setChecked( true );
      }
  }

  void OrcaInputDialog::initDFTData()
  {
      ui.dftGridCombo->setCurrentIndex(dftData->getGrid());
      ui.dftFinalGridCombo->setCurrentIndex(dftData->getFinalGrid());
      ui.dftFunctionalCombo->setCurrentIndex(dftData->getDFTFunctional());

  }

  void OrcaInputDialog::initCosXData()
  {
      ui.cosXGridCombo->setCurrentIndex(cosXData->getGrid());
      ui.cosXFinalGridCombo->setCurrentIndex(cosXData->getFinalGrid());

      if (cosXData->sFittingEnabled()) {
          ui.cosXSFittingCheck->setChecked(true);
      }

  }

  void OrcaInputDialog::initDataData()
  {
      ui.dataPrintCombo->setCurrentIndex(dataData->getPrintLevel());
      if (dataData->MOPrintEnabled()) {
          ui.MOPrintCheck->setChecked(true);
      } else {
          ui.MOPrintCheck->setChecked(false);
      }
      if (dataData->basisPrintEnabled()) {
          ui.basisPrintCheck->setChecked(true);
      } else {
          ui.basisPrintCheck->setChecked(false);
      }
  }

  void OrcaInputDialog::updateAdvancedSetup()
  {
      initBasisData();
      initControlData();
      initSCFData();
      initDFTData();
      initDataData();
      initCosXData();

      QTreeWidgetItem *controlItem = ui.advancedTree->topLevelItem(1);

      bool cosXEnabled = controlData->cosXEnabled();
      ui.cosXPage->setEnabled( cosXEnabled );
      controlItem->child(1)->setHidden(!cosXEnabled);

      bool dftEnabled = controlData->dftEnabled();
      ui.dftPage->setEnabled( dftEnabled );
      controlItem->child(2)->setHidden(!dftEnabled);

//      bool mp2Enabled = controlData->mp2Enabled();
//      ui.mp2Page->setEnabled( mp2Enabled );
//      controlItem->child(3)->setHidden(!mp2Enabled);

      updatePreviewText();
  }

  void OrcaInputDialog::updateBasicSetup()
  {
      initBasicData();
      updatePreviewText();
  }

  void OrcaInputDialog::setMode( int mode )
  {
      if (mode == 0) {
          m_basic = true;
          m_advanced = false;
          updateBasicSetup();
      } else if (mode == 1){
          m_basic = false;
          m_advanced = true;
          updateAdvancedSetup();
      }

    ui.modeTabWidget->setCurrentIndex( mode );
  }
  void OrcaInputDialog::setMolecule(Molecule *molecule)
  {
      // Disconnect the old molecule first...

      if (m_molecule)
        disconnect(m_molecule, 0, this, 0);

      m_molecule = molecule;

      // Set multiplicity to the OB value

      OpenBabel::OBMol obmol = m_molecule->OBMol();

      setBasicMultiplicity(obmol.GetTotalSpinMultiplicity());
      setControlMultiplicity(obmol.GetTotalSpinMultiplicity());
      setBasicCharge(obmol.GetTotalCharge());
      setControlCharge(obmol.GetTotalCharge());

      if (m_molecule){
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
  }

  void OrcaInputDialog::resetClicked()
  {
      if (m_advanced) {
          basisData->reset();
          controlData->reset();
          scfData->reset();
          dftData->reset();
          cosXData->reset();
          dataData->reset();
          updateAdvancedSetup();
      } else {
          basicData->reset();
          updateBasicSetup();
      }

      updatePreviewText();
  }

  void OrcaInputDialog::generateClicked()
  {
    saveInputFile(ui.previewText->toPlainText(), tr("ORCA Input Deck"), QString("inp"));
  }

  void OrcaInputDialog::hideClicked()
  {
      // If the hide preview button is clicked : hide/show the preview text and change button label

      if (ui.previewText->isVisible()) {
        ui.previewText->hide();
        ui.hideButton->setText(tr("Show Preview"));

      }
      else {
          ui.previewText->show();
          ui.hideButton->setText(tr("Hide Preview"));
      }
  }

  void OrcaInputDialog::previewEdited()
  {
      qDebug() <<"ui.previewText->document " <<  ui.previewText->document()->isModified() << "\n";
  }

//
// Set Basic Widgets
//
  void OrcaInputDialog::setBasicComment()
  {
      basicData->setComment(ui.basicCommentLine->text());
      updateBasicSetup();
  }

  void OrcaInputDialog::setBasicCalculation(int n)
  {
      basicData->setCalculation(n);
      updateBasicSetup();
  }

  void OrcaInputDialog::setBasicMethod(int n)
  {
      basicData->setMethod(n);
      updateBasicSetup();
  }

  void OrcaInputDialog::setBasicBasis(int n)
  {
      basicData->setBasis(n);
      updateBasicSetup();
  }

  void OrcaInputDialog::setBasicMultiplicity(int n)
  {
      basicData->setMultiplicity(n);
      if (ui.basicMultiplicitySpin->value() != n) {
          ui.basicMultiplicitySpin->setValue(n);
      }
      updateBasicSetup();
  }

  void OrcaInputDialog::setBasicCharge(int n)
  {
      basicData->setCharge (n);
      updateBasicSetup();
  }

  void OrcaInputDialog::setBasicCoordsFormat(int n)
  {
      basicData->setFormat(n);
      updateBasicSetup();
  }
//
// Set Advanced Basis Widgets
//
  void OrcaInputDialog::setBasisBasisSet(int n)
  {
      basisData->setBasis(n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setBasisAuxBasisSet(int n)
  {
      basisData->setAuxBasis(n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setBasisAuxCorrBasisSet(int n)
  {
      basisData->setAuxCorrBasis(n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setBasisUseEPC(bool value)
  {
      basisData->setEPCChecked(value);
      basisData->setAuxEPCChecked (value);
      basisData->setAuxCorrEPCChecked (value);
      updateAdvancedSetup();

  }
  void OrcaInputDialog::setBasisUseAuxEPC (bool value)
  {
      basisData->setAuxEPCChecked (value);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setBasisUseAuxCorrEPC (bool value)
  {
      basisData->setAuxCorrEPCChecked (value);
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setBasisUseRel(bool value)
  {
      basisData->setRelChecked(value);
      if (value) {
          ui.basisRelativisticCombo->setEnabled(true);
          if (basisData->dkhEnabled()) {
              ui.basisDKHSpin->setVisible(true);
              ui.basisDKHLabel->setVisible(true);
          }
      } else {
          ui.basisDKHSpin->setVisible(false);
          ui.basisDKHLabel->setVisible(false);
      }
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setBasisRel(int n)
  {
      basisData->setRel(n);
      if (n == DKH) {
          basisData->setDKHChecked(true);
          ui.basisDKHSpin->setVisible(true);
          ui.basisDKHLabel->setVisible(true);
      } else {
          basisData->setDKHChecked(false);
          ui.basisDKHSpin->setVisible(false);
          ui.basisDKHLabel->setVisible(false);
      }

      updateAdvancedSetup();
  }


  void OrcaInputDialog::setBasisDKHOrder(int n)
  {
      basisData->setDKHOrder(n);
      updateAdvancedSetup();
  }

//
// Set Advanced Control Widgets
//

  void OrcaInputDialog::setControlCalculation(int n)
  {
      controlData->setCalculation(n);
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setControlCharge(int n)
  {
      controlData->setCharge(n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setControlMultiplicity(int n)
  {
      controlData->setMultiplicity(n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setControlUseCosX(bool value )
  {
      if (value && controlData->dftEnabled()) {
          bool dftSuccess = checkDFTforRijCosX();
          if (dftSuccess) {
              controlData->setCosXChecked(value);
          } else {
              QMessageBox msgBox( QMessageBox::Warning, tr( " Selection failure" ),
                                  tr( "RijCosX option not available for the selected DFT functional!" ),
                                  QMessageBox::Ok);
              msgBox.exec();
          }
          qDebug () << dftSuccess;
      } else {
          controlData->setCosXChecked(value);
          enableAllDFTFunctionals();
      }
      if (value || controlData->dftEnabled()) {
          ui.basisAuxBasisSetCombo->setEnabled(true);
      } else {
          ui.basisAuxBasisSetCombo->setEnabled(false);

      }
      updateAdvancedSetup();
  }

  bool OrcaInputDialog::checkDFTforRijCosX()
  {
      // find the row numbers of all DFT functionals where NO RijCosX ist possible
      std::vector<int> rows;
      rows.resize(0);

      QMetaObject meta = OrcaExtension::staticMetaObject;
      QStringList noRijCosXDFT;
      for (int i=0; i < meta.enumeratorCount(); ++i) {
          QMetaEnum m = meta.enumerator(i);
          qDebug() << "//" << m.name() << "//";
          QString enumType = m.name();
          if (enumType == "DFTNoCosXType") {
              for (int j=0; j<m.keyCount();j++) {
                  noRijCosXDFT += QLatin1String(m.valueToKey(j));
              }
              break;
          }
      }
      noRijCosXDFT.replaceInStrings("No","");
//      noRijCosXDFT << "BP" << "TPSS";
      for (int i=0; i<noRijCosXDFT.size();i++) {
          rows.push_back(ui.dftFunctionalCombo->findText(noRijCosXDFT.at(i)));

          if (rows.at(i) == dftData->getDFTFunctional())
              return false;
      }
      // Set the flag of the items within the combobox model
      for (uint i=0; i<rows.size();i++)
          if (rows.at(i) >= 0)
              qobject_cast<QStandardItemModel *>(ui.dftFunctionalCombo->model())->item( rows.at(i) )->setEnabled( false );

      return true;
  }
  void OrcaInputDialog::enableAllDFTFunctionals()
  {
      // be sure that all DFT functionals can be selected

      for (int i=0; i<ui.dftFunctionalCombo->count();i++) {
              qobject_cast<QStandardItemModel *>(ui.dftFunctionalCombo->model())->item(i)->setEnabled(true);
      }

      return;
  }

  void OrcaInputDialog::setControlUseDFT(bool value )
  {
      controlData->setDFTChecked(value);
      if (value) {
          ui.controlMP2Check->setEnabled(false);
          ui.controlCCSDCheck->setEnabled(false);
          ui.basisAuxCorrBasisSetCombo->setEnabled(false);
          //          ui.basisAuxCorrECPCheck->setEnabled(false);
          ui.basisAuxBasisSetCombo->setEnabled(value);
          //          ui.basisAuxECPCheck->setEnabled(value);
          if (controlData->cosXEnabled()) {
              bool dftSuccess = checkDFTforRijCosX();
              if (!dftSuccess) {
                  controlData->setCosXChecked(false);
                  QMessageBox msgBox( QMessageBox::Warning, tr( " Selection failure" ),
                                      tr( "RijCosX not available for the selected DFT functional! \nRijCosX option reset!" ),
                                      QMessageBox::Ok);
                  msgBox.exec();
              }
          }
      } else {
          ui.controlMP2Check->setEnabled(true);
          ui.controlCCSDCheck->setEnabled(true);
          if (!controlData->cosXEnabled()) {
              ui.basisAuxBasisSetCombo->setEnabled(value);
              //              ui.basisAuxECPCheck->setEnabled(value);
          }
      }
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setControlUseMP2(bool value )
  {
      controlData->setMP2Checked(value);

      if (value) {
          ui.controlDFTCheck->setEnabled(!value);
          ui.controlCCSDCheck->setEnabled(false);
          ui.basisAuxCorrBasisSetCombo->setEnabled(value);
          //          ui.basisAuxCorrECPCheck->setEnabled(value);

          if (!controlData->cosXEnabled()) {
              ui.basisAuxBasisSetCombo->setEnabled(!value);
//              ui.basisAuxECPCheck->setEnabled(!value);
          }

      } else {
          ui.controlDFTCheck->setEnabled(true);
          ui.controlCCSDCheck->setEnabled(true);
          ui.basisAuxCorrBasisSetCombo->setEnabled(value);
      }
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setControlUseCCSD(bool value )
  {
      controlData->setCCSDChecked(value);

      if (value) {
          ui.controlDFTCheck->setEnabled(!value);
          ui.controlMP2Check->setEnabled(!value);
          ui.basisAuxCorrBasisSetCombo->setEnabled(value);
//          ui.basisAuxCorrECPCheck->setEnabled(value);

          if (!controlData->cosXEnabled()) {
              ui.basisAuxBasisSetCombo->setEnabled(!value);
//              ui.basisAuxECPCheck->setEnabled(!value);
          }

      } else {
          ui.controlDFTCheck->setEnabled(true);
          ui.controlMP2Check->setEnabled(true);
          ui.basisAuxCorrBasisSetCombo->setEnabled(value);
//          ui.basisAuxCorrECPCheck->setEnabled(value);
      }
      updateAdvancedSetup();
  }
//
// Advanced SCF Widgets
//
  void OrcaInputDialog::setSCFAccuracy(int n)
  {
      scfData->setAccuracy(n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setSCFType(int n)
  {
      scfData->setType (n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setSCFMaxIter(int n)
  {
      scfData->setMaxIter(n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setSCFDampFactor(double f)
  {
      scfData->setDampFactor( f);
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setSCFDampError(double f)
  {
      scfData->setDampError( f);
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setSCFLevelShift(double f)
  {
      scfData->setLevelShift(f);
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setSCFLevelError(double f)
  {
      scfData->setLevelError(f);
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setSCFUseDamping(bool value)
  {
      scfData->setDampingChecked(value);
      if (value) {
          scfData->setDampFactor(ui.scfDampFactorDSpin->value());
          scfData->setDampError(ui.scfDampErrorDSpin->value());
      }
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setSCFUseLevelShift(bool value)
  {
      scfData->setLevelShiftChecked(value);
      if (value) {
          scfData->setLevelShift(ui.scfLevelShiftDSpin->value());
          scfData->setLevelError(ui.scfLevelErrorDSpin->value());
      }
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setSCFConverger(int n)
  {
      scfData->setConv(n);
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setSCF2ndConverger(int n)
  {
      scfData->setConv2nd(n);
      updateAdvancedSetup();
  }

//
// Advanced DFT WIdgets
//

  void OrcaInputDialog::setDFTFinalGrid(int n)
  {
      dftData->setFinalGrid(n);
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setDFTGrid(int n)
  {
      dftData->setGrid(n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setDFTFunctional(int n)
  {
      dftData->setDFTFunctional(n);
      updateAdvancedSetup();
  }
//
// Advanced CosX WIdgets
//
  void OrcaInputDialog::setCosXFinalGrid(int n)
  {
      cosXData->setFinalGrid(n);
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setCosXGrid(int n)
  {
      cosXData->setGrid(n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setCosXSFitting(bool value)
  {
      cosXData->setSFittingChecked(value);
      updateAdvancedSetup();
  }

//
// Set Advanced Data Widgets
//
  void OrcaInputDialog::setDataFormat(int n)
  {
      dataData->setFormat(n);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setDataComment()
  {
      dataData->setComment(ui.dataCommentLine->text());
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setPrintLevel(int n)
  {
      dataData->setPrintLvl(n);
      updateAdvancedSetup();
  }
  void OrcaInputDialog::setMOPrint(bool value)
  {
      dataData->setMOPrintChecked(value);
      updateAdvancedSetup();
  }

  void OrcaInputDialog::setBasisPrint(bool value)
  {
      dataData->setBasisPrintChecked(value);
      updateAdvancedSetup();
  }
//
// Handle PreviewText and Output
//
  void OrcaInputDialog::updatePreviewText ()
  {
      if (!isVisible())
          return;

      // Generate the input deck and display it

      if (ui.previewText->document()->isModified()) {

          QMessageBox msgBox;

          msgBox.setWindowTitle(tr("OrcaExtension Warning"));
          msgBox.setText(tr("Unsaved changes are made in the actual preview text! \n Generating a new preview will lose all changes! \n Would you like to override them anyway?"));
          msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

          switch (msgBox.exec()) {

          case QMessageBox::Yes:
              // yes was clicked
              ui.previewText->setText(generateInputDeck());
              ui.previewText->document()->setModified(false);
              break;
          case QMessageBox::No:
              // no was clicked
              break;
          default:
              // should never be reached
              break;
          }
      } else {
          ui.previewText->setText(generateInputDeck());
          ui.previewText->document()->setModified(false);
      }
  }

  QString OrcaInputDialog::generateInputDeck()
  {
      // Generate an input deck based on the settings of the dialog

      QString buffer;
      QTextStream mol(&buffer);

      int charge, multiplicity;

      // start here generating the output stream for the preview text box

      // here - if Basic Mode is used

      if (m_basic){

          charge = basicData->getCharge();
          multiplicity = basicData->getMultiplicity();

          mol << "# avogadro generated ORCA input file \n# Basic Mode\n";

          // write the comment / comment

          // Comment
          mol << "# " << basicData->getComment() << "\n";

          // create inline command

          mol << "! " << basicData->getMethodTxt() << " " << basicData->getCalculationTxt() << " " << basicData->getBasisTxt();
          if (basicData->getMethod() == DFT) {
              mol << " " << basicData->getBasisTxt() <<"/J";
          }
          mol << " \n";
      }

      // here - if Advanced Mode is used

      else {

          charge = controlData->getCharge();
          multiplicity = controlData->getMultiplicity();

          mol << "## avogadro generated ORCA input file \n# Advanced Mode\n";

          // write the comment / comment

          // Comment
          mol << "# " << dataData->getComment() << "\n";

          // create inline command
          mol << "! ";

          // Method

          if (controlData->dftEnabled()) {
              if (dftData->getDFTFunctional() == OrcaExtension::BP) {
                  mol <<  dftData->getDFTFunctionalTxt() << " RI " ;
              } else {
                  mol <<  dftData->getDFTFunctionalTxt() << " " ;
              }
          } else if ( controlData->mp2Enabled()) {
              mol << "RI-MP2 ";
          } else if ( controlData->ccsdEnabled()) {
              mol << "CCSD ";
          } else {
              mol << scfData->getTypeTxt() << " ";
          }

          // Calculation

          mol << controlData->getCalculationTxt() << " ";

          // Basis Set(s)

          mol <<  basisData->getBasisTxt() << " ";

          if (controlData->cosXEnabled() || controlData->dftEnabled()) {
              mol << basisData->getAuxBasisTxt() << " ";
          }
          if (controlData->mp2Enabled()) {
              mol << basisData->getAuxCorrBasisTxt() << " ";
          }
          if (basisData->EPCEnabled()) {
              mol << "EPC{" << basisData->getBasisTxt();
              if (controlData->cosXEnabled() || controlData->dftEnabled()) {
                  mol  << "," << basisData->getAuxBasisTxt();
              }
              if (controlData->mp2Enabled()) {
                  mol << "," << basisData->getAuxCorrBasisTxt();
              }
              mol << "} ";
          }
          if (dataData->getPrintLevel() != 0) {
              mol << dataData->getPrintLevelTxt() << " ";
          }
          if (controlData->dftEnabled()) {
              mol << dftData->getGridTxt() << " " << dftData->getFinalGridTxt() << " ";
          }
          if (controlData->cosXEnabled()) {
              mol << "RijCosX ";
              mol << cosXData->getGridTxt() << " " << cosXData->getFinalGridTxt() << " ";
          }
          mol << scfData->getAccuracyTxt() << " ";
          if (basisData->relEnabled()) {
              if (basisData->dkhEnabled()){
                  mol << basisData->getRelTxt() << basisData->getDKHOrder() << " ";
              } else {
                  mol << basisData->getRelTxt() << " ";
              }
          }
          // SCF Block Infos

          mol << "\n";
          mol << "%scf\n";
          mol << "\tMaxIter " << scfData->getMaxIter() << "\n";
          if (scfData->dampingEnabled()){
              mol << "\tCNVDamp 1 \n";
              mol << "\tDampFac " << scfData->getDampFactor() << "\n";
              mol << "\tDampErr " << scfData->getDampError() << "\n";
          }
          if(scfData->levelShiftEnabled()){
              mol << "\tCNVShift 1 \n";
              mol << "\tLevelShift " << scfData->getLevelShift() << "\n";
              mol << "\tShiftErr " << scfData->getLevelError() << "\n";
          }
          if (scfData->getConv() == DIIS) {
              mol << "\tCNVDIIS 1\n";
          } else {
              mol << "\tCNVKDIIS 1\n";
          }
          if (scfData->getConv2nd() == SOSCF) {
              mol << "\tCNVSOSCF 1 \n";
          } else if (scfData->getConv2nd() == NRSCF) {
              mol << "\tCNVNR 1 \n";
          } else {
//              mol << "\tCNVAH 1 \n";   // not yet implemented
          }
          mol << "end\n";

//          // Output Block Info -this keyword is not working correctly

          if (dataData->basisPrintEnabled() || dataData->MOPrintEnabled()) {
              mol << "%output\n";
              if (dataData->MOPrintEnabled())   mol << "\tprint[p_mos] true\n";
              if (dataData->basisPrintEnabled())    mol << "\tprint[p_basis] 5\n";

              //          mol << "\t PrintLevel " << dataData->getPrintLevelTxt() << "\n";
              mol << "end\n";
          }

      }
      mol << "\n";

      // Coordinates
      coordType formatCheck;
      if (m_basic) {
          formatCheck = basicData->getFormat();
      } else {
          formatCheck = dataData->getFormat();
      }
      if (m_molecule && (formatCheck == CARTESIAN)) {


          // Cartesian coordinates

          QTextStream mol(&buffer);

          mol << "* xyz " << charge << " " << multiplicity << "\n";

          QList<Atom *> atoms = m_molecule->atoms();

          foreach (Atom *atom, atoms) {
              mol << qSetFieldWidth(4) << right
                  << QString(OpenBabel::etab.GetSymbol(atom->atomicNumber()))
                  << qSetFieldWidth(15) << qSetRealNumberPrecision(5) << forcepoint
                  << fixed << right << atom->pos()->x() << atom->pos()->y()
                  << atom->pos()->z()
                  << qSetFieldWidth(0) << '\n';
          }
          mol << "*\n";

      } else if (m_molecule && (formatCheck == ZMATRIX)) {

          // Z-matrix

          QTextStream mol(&buffer);
          mol.setFieldAlignment(QTextStream::AlignAccountingStyle);
          mol << "* int " << charge << " " << multiplicity << "\n";
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

              mol << qSetFieldWidth(3) << QString(etab.GetSymbol(atom->GetAtomicNum()));

              if (atom->GetIdx() > 1)
                  mol << qSetFieldWidth(0) << "  " << qSetFieldWidth(3) << QString::number(a->GetIdx())
                      << qSetFieldWidth(0) << "  "<< qSetFieldWidth(4) << QString("r") + QString::number(atom->GetIdx());

              if (atom->GetIdx() > 2)
                  mol << qSetFieldWidth(0) << "  " << qSetFieldWidth(3) << QString::number(b->GetIdx())
                      << qSetFieldWidth(0) << "  "<< qSetFieldWidth(4) << QString("a") + QString::number(atom->GetIdx());

              if (atom->GetIdx() > 3)
                  mol << qSetFieldWidth(0) << "  " << qSetFieldWidth(3) << QString::number(c->GetIdx())
                      << qSetFieldWidth(0) << "  "<< qSetFieldWidth(4) << QString("d") + QString::number(atom->GetIdx());

              mol << qSetFieldWidth(0) << '\n';
          }

          mol << " variables\n";
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
                  mol << "   r" << atom->GetIdx() << qSetFieldWidth(15)
                      << qSetRealNumberPrecision(5) << forcepoint << fixed << right
                      << r << qSetFieldWidth(0) << '\n';
              if (atom->GetIdx() > 2)
                  mol << "   a" << atom->GetIdx() << qSetFieldWidth(15)
                      << qSetRealNumberPrecision(5) << forcepoint << fixed << right
                      << w << qSetFieldWidth(0) << '\n';
              if (atom->GetIdx() > 3)
                  mol << "   d" << atom->GetIdx() << qSetFieldWidth(15)
                      << qSetRealNumberPrecision(5) << forcepoint << fixed << right
                      << t << qSetFieldWidth(0) << '\n';
          }
          mol << " end\n";
          foreach (OpenBabel::OBInternalCoord *c, vic)
              delete c;
      } else if (m_molecule && (formatCheck == ZMATRIX_COMPACT)) {

          // Compact ZMatrix

          QTextStream mol(&buffer);

          mol << "* int " << charge << " " << multiplicity << "\n";

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


      mol << endl;

      return buffer;
  }

  QString OrcaInputDialog::saveInputFile(QString inputDeck, QString fileType, QString ext)
  {
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

    file.write(inputDeck.toLocal8Bit()); // prevent troubles in Windows
    file.close(); // flush buffer!
    m_savePath = QFileInfo(file).absolutePath();
    return fileName;
  }

  void OrcaInputDialog::readSettings(QSettings& settings)
  {
    m_savePath = settings.value("orca/savepath").toString();
  }

  void OrcaInputDialog::writeSettings(QSettings& settings) const
  {
    settings.setValue("orca/savepath", m_savePath);
  }
}

// This includes the files generated by Qt's moc at compile time to
// ensure that signals/slots work. If you ever see errors about
// missing vtables with gcc, check that you haven't forgotten one of
// these:
//#include "orcainputdialog.moc"
