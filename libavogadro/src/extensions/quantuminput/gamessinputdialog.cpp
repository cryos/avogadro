/**********************************************************************
  GamessInputDialog - GAMESS Input Deck Dialog for Avogadro

  Copyright (C) 2006, 2007 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "gamessinputdialog.h"
#include "gamesshighlighter.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

#include <QtGui/QPushButton>
#include <QtGui/QButtonGroup>
#include <QtGui/QMessageBox>
//#include <QtGui/QFileDialog>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QSettings>

namespace Avogadro {

  GamessInputDialog::GamessInputDialog( GamessInputData *inputData,
    QWidget *parent, Qt::WindowFlags f ) : InputDialog( parent, f ),
    m_inputData(NULL), m_advancedChanged( false )
  {
    setInputData(inputData);

    ui.setupUi(this);
    ui.navigationTree->expandAll();
    ui.navigationTree->headerItem()->setHidden(true);

    // FIXME: Check for GAMESS and show as needed.
    //    ui.computeButton->hide();

    m_highlighter = new GamessHighlighter(ui.previewText->document());

    connectModes();
    connectBasic();
    connectAdvanced();
    connectPreview();
    connectButtons();

    setBasicDefaults();
    updateBasicWidgets();
    updateAdvancedWidgets();
    updatePreviewText();
    
    QSettings settings;
    readSettings(settings);
  }

  // TODO on SHOW we need to update the current view.

  GamessInputDialog::~GamessInputDialog()
  {
      QSettings settings;
      writeSettings(settings);
  }

  void GamessInputDialog::setInputData(GamessInputData *inputData)
  {
    if(!inputData) return;

    if(m_inputData)
      disconnect(m_inputData->m_molecule, 0, this, 0);

    m_inputData = inputData;
    m_molecule = m_inputData->m_molecule;

    connect(m_inputData->m_molecule, SIGNAL(primitiveAdded( Primitive* )),
        this, SLOT(updatePreviewText()));
    connect(m_inputData->m_molecule, SIGNAL(primitiveUpdated( Primitive* )),
        this, SLOT(updatePreviewText()));
    connect(m_inputData->m_molecule, SIGNAL(primitiveRemoved( Primitive* )),
        this, SLOT(updatePreviewText()));
    connect(m_inputData->m_molecule, SIGNAL(updated()), this, SLOT(updatePreviewText()));
  }

  void GamessInputDialog::connectBasic()
  {
    // Calculate
    connect( ui.basicCalculateCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setBasicCalculate( int ) ) );
    connect( ui.basicCalculateCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basicCalculateCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( basicChanged() ) );

    // With Left
    connect( ui.basicWithLeftCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setBasicWithLeft( int ) ) );
    connect( ui.basicWithLeftCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basicWithLeftCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( basicChanged() ) );

    // With Right
    connect( ui.basicWithRightCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setBasicWithRight( int ) ) );
    connect( ui.basicWithRightCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basicWithRightCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( basicChanged() ) );

    // In
    connect( ui.basicInCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setBasicIn( int ) ) );
    connect( ui.basicInCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basicInCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( basicChanged() ) );

    // On Left
    connect( ui.basicOnLeftCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setBasicOnLeft( int ) ) );
    connect( ui.basicOnLeftCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basicOnLeftCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( basicChanged() ) );

    // On Right
    connect( ui.basicOnRightCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setBasicOnRight( int ) ) );
    connect( ui.basicOnRightCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basicOnRightCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( basicChanged() ) );
  }

  void GamessInputDialog::connectAdvanced()
  {

    connectBasis();
    connectControl();
    connectData();
    connectSystem();
    connectMOGuess();
    connectMisc();
    connectSCF();
    connectDFT();
    connectMP2();
    connectHessian();
    connectStatPoint();

    connect(ui.navigationTree, SIGNAL(clicked(QModelIndex)), this,
        SLOT(navigationItemClicked(QModelIndex)));
  }

  void GamessInputDialog::navigationItemClicked( const QModelIndex &index )
  {
    int i = index.row();

    QModelIndex parent = index.parent();
    if(parent.isValid())
    {
      i += ui.navigationTree->topLevelItemCount();
    }

    ui.advancedStacked->setCurrentIndex(i);
  }

  void GamessInputDialog::connectPreview()
  {
  }

  void GamessInputDialog::connectModes()
  {
    connect( ui.modeTab, SIGNAL( currentChanged( int ) ), this, SLOT( setMode( int ) ) );
  }

  void GamessInputDialog::connectButtons()
  {
    connect( ui.resetAllButton, SIGNAL( clicked() ), this, SLOT( resetClicked() ) );
    connect( ui.defaultsButton, SIGNAL( clicked() ), this, SLOT( defaultsClicked() ) );
    connect( ui.generateButton, SIGNAL( clicked() ), this, SLOT( generateClicked() ) );
    connect( ui.closeButton, SIGNAL( clicked() ), this, SLOT( close() ) );
  }

  void GamessInputDialog::basicChanged()
  {
    ui.basicWidget->setEnabled( true );
    m_advancedChanged = false;
  }

  void GamessInputDialog::advancedChanged()
  {
    ui.basicWidget->setEnabled( false );
    m_advancedChanged = true;
  }

  void GamessInputDialog::setBasicDefaults()
  {
    // These rely on the advanced defaults
    setAdvancedDefaults();

    blockChildrenSignals(ui.basicWidget, true);

    ui.basicCalculateCombo->setCurrentIndex( 0 );
    setBasicCalculate(0);
    ui.basicWithLeftCombo->setCurrentIndex( 2 );
    setBasicWithRight(2);
    ui.basicWithRightCombo->setCurrentIndex( 0 );
    setBasicWithRight(0);
    ui.basicInCombo->setCurrentIndex( 0 );
    setBasicIn(0);
    ui.basicOnLeftCombo->setCurrentIndex( 0 );
    setBasicOnLeft(0);
    ui.basicOnRightCombo->setCurrentIndex( 2 );
    setBasicOnRight(2);
    updateBasicWidgets();

    blockChildrenSignals(ui.basicWidget, false);

    m_advancedChanged = false;
    ui.basicWidget->setEnabled( true );

    //   updateAdvancedWidgets();

  }
  void GamessInputDialog::setAdvancedDefaults()
  {
    setBasisDefaults();
    setControlDefaults();
    setDataDefaults();
    setSystemDefaults();
    setMOGuessDefaults();
    setMiscDefaults();
    setSCFDefaults();
    setDFTDefaults();
    setMP2Defaults();
    setHessianDefaults();
    setStatPointDefaults();

    ui.navigationTree->setCurrentItem(ui.navigationTree->topLevelItem(0));
    ui.advancedStacked->setCurrentIndex(0);
  }

  void GamessInputDialog::setBasisDefaults()
  {
    m_inputData->Basis->InitData();
    updateBasisWidgets();
  }

  void GamessInputDialog::setControlDefaults()
  {
    m_inputData->Control->InitControlPaneData();
    updateControlWidgets();
  }

  void GamessInputDialog::setDataDefaults()
  {
    m_inputData->Data->InitData();
    updateDataWidgets();
  }

  void GamessInputDialog::setSystemDefaults()
  {
    m_inputData->System->InitData();
    updateSystemWidgets();
  }

  void GamessInputDialog::setMOGuessDefaults()
  {
    m_inputData->Guess->InitData();
    updateMOGuessWidgets();
  }

  void GamessInputDialog::setMiscDefaults()
  {
    m_inputData->Control->InitProgPaneData();
    m_inputData->Basis->SetWaterSolvate(false);
    updateMiscWidgets();
  }

  void GamessInputDialog::setSCFDefaults()
  {
    m_inputData->SCF->InitData();
    updateSCFWidgets();
  }

  void GamessInputDialog::setDFTDefaults()
  {
    m_inputData->DFT->InitData();
    updateDFTWidgets();
  }

  void GamessInputDialog::setMP2Defaults()
  {
    m_inputData->MP2->InitData();
    updateMP2Widgets();
  }

  void GamessInputDialog::setHessianDefaults()
  {
    m_inputData->Hessian->InitData();
    updateHessianWidgets();
  }

  void GamessInputDialog::setStatPointDefaults()
  {
    m_inputData->StatPt->InitData();
    updateStatPointWidgets();
  }

  void GamessInputDialog::blockChildrenSignals( QLayout *layout, bool block )
  {
    blockChildrenSignals( layout->parentWidget(), block );
  }

  void GamessInputDialog::blockChildrenSignals( QObject *object, bool block )
  {
    if ( !object ) return;

    QList<QObject *> children = object->children();

    Q_FOREACH( QObject *child, children )
    {
      //     qDebug() << child->metaObject()->className();
      blockChildrenSignals(child, block);
      child->blockSignals( block );
    }
  }

  void GamessInputDialog::blockAdvancedSignals( bool block )
  {
    int count = ui.advancedStacked->count();
    for ( int i = 0; i < count; i++ )
    {
      blockChildrenSignals( ui.advancedStacked->widget( i ), block );
    }
  }

  void GamessInputDialog::updatePreviewText()
  {
    stringstream str;
    m_inputData->WriteInputFile( str );
    ui.previewText->setText( QString::fromAscii( str.str().c_str() ) );
  }

  void GamessInputDialog::updateBasicWidgets()
  {
    // These rely on the advanced defaults
    setAdvancedDefaults();

    setBasicCalculate( ui.basicCalculateCombo->currentIndex() );
    setBasicWithLeft( ui.basicWithLeftCombo->currentIndex() );
    setBasicWithRight( ui.basicWithRightCombo->currentIndex() );
    setBasicIn( ui.basicInCombo->currentIndex() );

    // multText
    int itemValue = m_inputData->Control->GetMultiplicity();
    if ( itemValue <= 0 )
    {
      long NumElectrons = m_inputData->GetNumElectrons();
      if ( NumElectrons & 1 ) {
        ui.basicOnLeftCombo->setCurrentIndex(1);
      }
    }
    else
    {
      setBasicOnLeft( ui.basicOnLeftCombo->currentIndex() );
    }
    setBasicOnRight( ui.basicOnRightCombo->currentIndex() );

    m_advancedChanged = false;
    ui.basicWidget->setEnabled( true );
  }

  void GamessInputDialog::updateAdvancedWidgets()
  {
    updateBasisWidgets();
    updateControlWidgets();
    updateDataWidgets();
    updateSystemWidgets();
    updateMOGuessWidgets();
    updateMiscWidgets();
    updateSCFWidgets();
    updateDFTWidgets();
    updateMP2Widgets();
    updateHessianWidgets();
    updateStatPointWidgets();

    QTreeWidgetItem *controlItem = ui.navigationTree->topLevelItem(1);

    int runType = m_inputData->Control->GetRunType();

    bool scfEnabled = (m_inputData->Control->GetSCFType() <= 4);
    ui.scfWidget->setEnabled( scfEnabled );
    controlItem->child(0)->setHidden(!scfEnabled);

    bool dftEnabled = m_inputData->Control->UseDFT();
    ui.dftWidget->setEnabled( dftEnabled );
    controlItem->child(1)->setHidden(!dftEnabled);

    bool mp2Enabled = ( m_inputData->Control->GetMPLevel() == 2 );
    ui.mp2Widget->setEnabled( mp2Enabled );
    controlItem->child(2)->setHidden(!mp2Enabled);

    bool hessianEnabled = (( runType == 3 ) || (( runType == 3 || runType == 6 ) &&
          m_inputData->StatPt->GetHessMethod() == 3 ));
    ui.hessianWidget->setEnabled( hessianEnabled );
    controlItem->child(3)->setHidden(!hessianEnabled);

    bool statPointEnabled = (runType == 4 || runType == 6);
    ui.statPointWidget->setEnabled( statPointEnabled );
    controlItem->child(4)->setHidden(!statPointEnabled);

  }

  void GamessInputDialog::updateBasisWidgets()
  {
    blockChildrenSignals( ui.basisWidget, true );
    int basis = m_inputData->Basis->GetBasis();
    int gauss = m_inputData->Basis->GetNumGauss();
    int itemValue = 0;
    int testValue = 0;

    // basisChoice
    itemValue = basis;
    if ( itemValue == 0 ) itemValue = 1;
    else if ( itemValue == 3 ) itemValue = gauss + 1;
    else if ( itemValue == 4 )
    {
      itemValue += 4;
      if ( gauss == 6 ) itemValue++;
    }
    else if ( itemValue == 5 ) itemValue = gauss + 6;
    else if ( itemValue > 5 ) itemValue += 7;
    ui.basisSetCombo->setCurrentIndex( itemValue - 1 );

    //TODO: Implement this.
    //   CheckBasisMenu();

    // ui.basisECPCombo
    if ( basis == 12 || basis == 13 )
    {
      ui.basisECPCombo->setEnabled( true );
      itemValue = m_inputData->Basis->GetECPPotential();
      if ( basis == 12 && ( itemValue == 3 || itemValue == 0 ) ) itemValue=2;
      else if ( basis == 13 && ( itemValue == 2 || itemValue == 0 ) ) itemValue=3;
      ui.basisECPCombo->setCurrentIndex( itemValue );
    }
    else
    {
      ui.basisECPCombo->setCurrentIndex( 0 );
      ui.basisECPCombo->setEnabled( false );
    }

    // polarChoice
    testValue = m_inputData->Basis->GetNumPFuncs() +
      m_inputData->Basis->GetNumDFuncs() +
      m_inputData->Basis->GetNumFFuncs();
    if ( testValue )
    {
      ui.basisPolarCombo->setEnabled( true );
      itemValue = m_inputData->Basis->GetPolar();
      if ( itemValue == 0 )
      {
        if ( basis == 6 || basis == 11 ) itemValue = 2;
        else if ( basis == 7 || basis == 8 ) itemValue = 3;
        else if ( basis < 3 ) itemValue = 4;
        else if ( basis == 10 ) itemValue = 5;
        else itemValue = 1;
      }
      ui.basisPolarCombo->setCurrentIndex( itemValue - 1 );
    }
    else
    {
      ui.basisPolarCombo->setEnabled( false );
    }

    // diffuseLCheck
    ui.basisLCheck->setChecked( m_inputData->Basis->GetDiffuseSP() );

    // diffuseSCheck
    ui.basisSCheck->setChecked( m_inputData->Basis->GetDiffuseS() );

    // numDChoice
    ui.basisDSpin->setValue( m_inputData->Basis->GetNumDFuncs() );

    // numFChoice
    ui.basisFSpin->setValue( m_inputData->Basis->GetNumFFuncs() );

    // numPChoice
    ui.basisLightSpin->setValue( m_inputData->Basis->GetNumPFuncs() );
    blockChildrenSignals( ui.basisWidget, false );
  }

  void GamessInputDialog::updateControlWidgets()
  {
    blockChildrenSignals( ui.controlWidget, true );
    short mp2 = m_inputData->Control->GetMPLevel();
    bool dft = m_inputData->Control->UseDFT();
    short ci = m_inputData->Control->GetCIType();
    CCRunType cc = m_inputData->Control->GetCCType();
    long scft = m_inputData->Control->GetSCFType();
    long NumElectrons = m_inputData->GetNumElectrons();

    int itemValue = 0;

    itemValue = m_inputData->Control->GetRunType();
    if ( itemValue == 0 ) itemValue = 1;
    ui.controlRunCombo->setCurrentIndex( itemValue - 1 );

    if ( scft == 0 )
    {
      if ( NumElectrons & 1 )
      {
        scft = 3;
      }
      else
      {
        scft = 1;
      }
    }
    ui.controlSCFCombo->setCurrentIndex( scft - 1 );

    // mp2Check
    if ( ci || cc || dft || ( mp2 < 0 ) )
    {
      ui.controlMP2Check->setChecked( false );
      ui.controlMP2Check->setEnabled( false );
    }
    else
    {
      ui.controlMP2Check->setEnabled( true );
      if ( mp2 < 0 ) mp2 = 0;
      if ( mp2 == 2 ) mp2 = 1;
      ui.controlMP2Check->setChecked( mp2 );
    }

    // dftCheck
    if ( ci || cc || ( mp2 > 0 ) || ( scft > 3 ) )
    {
      ui.controlDFTCheck->setChecked( false );
      ui.controlDFTCheck->setEnabled( false );
    }
    else
    {
      ui.controlDFTCheck->setEnabled( true );
      ui.controlDFTCheck->setChecked( dft );
    }

    // ciCombo
    if (( mp2 > 0 ) || dft || cc || scft == 2 )
    {
      ui.controlCICombo->setCurrentIndex( 0 );
      ui.controlCICombo->setEnabled( false );
    }
    else
    {
      ui.controlCICombo->setEnabled( true );
      ui.controlCICombo->setCurrentIndex( ci );
    }

    // ccCombo
    if (( mp2 > 0 ) || dft || ci || scft > 1 )
    {
      ui.controlCCCombo->setCurrentIndex( 0 );
      ui.controlCCCombo->setEnabled( false );
    }
    else
    {
      ui.controlCCCombo->setEnabled( true );
      ui.controlCCCombo->setCurrentIndex( cc );
    }

    // scfIterText
    itemValue = m_inputData->Control->GetMaxIt();
    if ( itemValue <= 0 ) itemValue = 30;
    ui.controlMaxSCFSpin->setValue( itemValue );

    // exeCombo
    ui.controlExecCombo->setCurrentIndex( m_inputData->Control->GetExeType() );
    if ( m_inputData->Control->GetFriend() != Friend_None )
    {
      //The friend keyword choices force a check run type
      ui.controlExecCombo->setEnabled( false );
      ui.controlExecCombo->setCurrentIndex( 1 );
    }
    else
    {
      ui.controlExecCombo->setEnabled( true );
    }

    // mchargeText
    ui.controlChargeSpin->setValue( m_inputData->Control->GetCharge() );

    // multText
    itemValue = m_inputData->Control->GetMultiplicity();
    if ( itemValue <= 0 )
    {
      if ( NumElectrons & 1 ) itemValue = 2;
      else itemValue = 1;
    }
    ui.controlMultiplicitySpin->setValue( itemValue );

    // localCombo
    ui.controlLocalizationCombo->setCurrentIndex( m_inputData->Control->GetLocal() );
    blockChildrenSignals( ui.controlWidget, false );
  }

  void GamessInputDialog::updateDataWidgets()
  {
    blockChildrenSignals( ui.dataWidget, true );
    int itemValue;

    //Title
    if ( m_inputData->Data->GetTitle() )
      ui.dataTitleLine->setText( tr( m_inputData->Data->GetTitle() ) );
    else
      ui.dataTitleLine->setText( tr( "Title" ) );

    // coordTypeCombo
    itemValue = m_inputData->Data->GetCoordType();
    if ( itemValue == 0 ) itemValue = 1;
    ui.dataCoordinateCombo->setCurrentIndex( itemValue - 1 );

    // unitCombo
    ui.dataUnitsCombo->setCurrentIndex( m_inputData->Data->GetUnits() );

    //# Z-Matrix vars
    ui.dataZMatrixSpin->setValue( m_inputData->Data->GetNumZVar() );

    //Point Group
    itemValue = m_inputData->Data->GetPointGroup();
    if ( itemValue == 0 ) itemValue = 1;
    ui.dataPointCombo->setCurrentIndex( itemValue-1 );

    //Point group order
    updatePointGroupOrderWidgets();

    // symmetryCheck
    ui.dataSymmetryCheck->setChecked( m_inputData->Data->GetUseSym() );
    blockChildrenSignals( ui.dataWidget, false );
  }

  void GamessInputDialog::updateSystemWidgets()
  {
    blockChildrenSignals( ui.systemWidget, true );

    ui.systemTimeDouble->setValue( m_inputData->System->GetConvertedTime() );
    ui.systemTimeCombo->setCurrentIndex( m_inputData->System->GetTimeUnits() - minuteUnit );

    // Standard Memory Handler
    ui.systemMemoryDouble->setValue( m_inputData->System->GetConvertedMem() );
    ui.systemMemoryCombo->setCurrentIndex( m_inputData->System->GetMemUnits() - megaWordsUnit );

    // memDDI edit
    ui.systemDDIDouble->setValue( m_inputData->System->GetConvertedMemDDI() );

    // memDDIUnitCombo
    ui.systemDDICombo->setCurrentIndex( m_inputData->System->GetMemDDIUnits() - megaWordsUnit );

    // diagCombo
    ui.systemDiagonalizationCombo->setCurrentIndex( m_inputData->System->GetDiag() );

    ui.systemCoreCheck->setChecked( m_inputData->System->GetCoreFlag() );

    if ( m_inputData->System->GetBalanceType() )
      ui.systemNextRadio->setChecked( true );
    else
      ui.systemLoopRadio->setChecked( true );

    ui.systemExternalCheck->setChecked( m_inputData->System->GetXDR() );

    // Parall check
    ui.systemForceCheck->setChecked( m_inputData->System->GetParallel() );

    blockChildrenSignals( ui.systemWidget, false );
  }

  void GamessInputDialog::updatePointGroupOrderWidgets()
  {
    ui.dataOrderSpin->blockSignals( true );
    //Point group order - only applicable to certain point groups
    int itemValue = m_inputData->Data->GetPointGroup();
    if ( itemValue == 0 ) itemValue = 1;
    if (( itemValue>3 )&&( itemValue<11 ) )
    {
      ui.dataOrderSpin->setEnabled( true );
      itemValue = m_inputData->Data->GetPointGroupOrder()-1;
      if ( itemValue <= 0 )
      {
        itemValue = 1;
        m_inputData->Data->SetPointGroupOrder( 2 );
      }
      ui.dataOrderSpin->setValue( itemValue + 1 );
    }
    else ui.dataOrderSpin->setEnabled( false );
    ui.dataOrderSpin->blockSignals( false );
  }

  void GamessInputDialog::updateMOGuessWidgets()
  {
    blockChildrenSignals( ui.moGuessWidget, true );
    long guess = m_inputData->Guess->GetGuess();
    if ( guess < 1 )
    {
      guess = 1;
    }
    ui.moGuessInitialCombo->setCurrentIndex( guess-1 );

    int multiplicity = m_inputData->Control->GetMultiplicity();
    ui.moGuessRotateCheck->setEnabled(( m_inputData->Control->GetSCFType() == 2 ) &&
        ( multiplicity == 1 || multiplicity == 2 ) );
    ui.moGuessRotateCheck->setChecked( m_inputData->Guess->GetMix() );
    ui.moGuessPrintCheck->setChecked( m_inputData->Guess->GetPrintMO() );

    blockChildrenSignals( ui.moGuessWidget, false );
  }

  void GamessInputDialog::updateMiscWidgets()
  {
    blockChildrenSignals( ui.miscWidget, true );

    ui.miscMolPltCheck->setChecked( m_inputData->Control->GetMolPlot() );
    ui.miscPltOrbCheck->setChecked( m_inputData->Control->GetPlotOrb() );

    ui.miscAIMPACCheck->setChecked( m_inputData->Control->GetAIMPAC() );
    ui.miscRPACCheck->setChecked( m_inputData->Control->GetRPAC() );

    int force = m_inputData->Control->GetFriend();
    if (( m_inputData->Control->GetExeType() != 1 ) &&
        ( force == Friend_None ) )
    {
      ui.miscAIMPACCheck->setEnabled( true );
      ui.miscRPACCheck->setEnabled( true );
    }
    else
    {
      ui.miscAIMPACCheck->setEnabled( true );
      ui.miscRPACCheck->setEnabled( true );
    }

    QRadioButton *forceButton =
      qobject_cast<QRadioButton *>( m_miscForceButtons->button( force ) );

    if ( forceButton )
      forceButton->setChecked( true );

    ui.miscWaterCheck->setChecked( m_inputData->Basis->GetWaterSolvate() );

    blockChildrenSignals( ui.miscWidget, false );
  }

  void GamessInputDialog::updateSCFWidgets()
  {
    blockChildrenSignals( ui.scfWidget, true );

    bool direct = m_inputData->SCF->GetDirectSCF();
    int scf = m_inputData->Control->GetSCFType();

    ui.scfDirectCheck->setChecked( direct );

    ui.scfComputeCheck->setChecked( m_inputData->SCF->GetFockDiff() );
    ui.scfComputeCheck->setEnabled( direct && 3>=scf );

    ui.scfGenerateCheck->setChecked( m_inputData->SCF->GetUHFNO() );
    ui.scfGenerateCheck->setEnabled( scf == GAMESS_UHF );

    blockChildrenSignals( ui.scfWidget, false );
  }

  void GamessInputDialog::updateDFTWidgets()
  {
    blockChildrenSignals( ui.scfWidget, true );

    blockChildrenSignals( ui.scfWidget, false );
  }

  void GamessInputDialog::updateMP2Widgets()
  {
    blockChildrenSignals( ui.mp2Widget, true );

    int electrons = m_inputData->MP2->GetNumCoreElectrons();
    if(electrons > -1)
    {
      ui.mp2ElectronsSpin->setValue(electrons);
    } else {
      ui.mp2ElectronsSpin->setValue(0);
    }

    long mem = m_inputData->MP2->GetMemory();
    ui.mp2MemorySpin->setValue(mem);

    double cutoff = m_inputData->MP2->GetIntCutoff();
    if(cutoff == 0.0) cutoff = 1.0e-09;
    ui.mp2IntegralLine->setText(QString::number(cutoff));

    ui.mp2LocalizedCheck->setEnabled(m_inputData->Control->GetSCFType() <= 1);
    bool lmomp2 = m_inputData->MP2->GetLMOMP2();
    ui.mp2LocalizedCheck->setChecked(lmomp2);

    ui.mp2ComputeCheck->setEnabled(m_inputData->Control->GetRunType());
    ui.mp2ComputeCheck->setChecked(m_inputData->MP2->GetMP2Prop());

    ui.mp2TransformationGroup->setEnabled(!lmomp2);
    if(m_inputData->MP2->GetMethod() == 3)
      ui.mp2TwoRadio->setChecked(true);
    else
      ui.mp2SegmentedRadio->setChecked(true);

    int ao = m_inputData->MP2->GetAOIntMethod();
    if( ao == 2 || ao == 0)
      ui.mp2DistributedRadio->setChecked(true);
    else
      ui.mp2DuplicatedRadio->setChecked(true);

    blockChildrenSignals( ui.mp2Widget, false );
  }

  void GamessInputDialog::updateHessianWidgets()
  {
    blockChildrenSignals( ui.hessianWidget, true );

    int scfType = m_inputData->Control->GetSCFType();

    bool analyticEnable = (( scfType == 1 || scfType == 3 || scfType == 4 || scfType == 0) && m_inputData->Control->GetMPLevel() == 0);
    bool analyticSelect = analyticEnable && m_inputData->Hessian->GetAnalyticMethod();

    ui.hessianAnalyticRadio->setChecked(analyticSelect);
    ui.hessianNumericRadio->setChecked(!analyticSelect);
    ui.hessianAnalyticRadio->setEnabled(analyticEnable);

    ui.hessianDoubleCheck->setEnabled(!analyticSelect);
    ui.hessianDoubleCheck->setChecked(m_inputData->Hessian->GetDoubleDiff());

    int numZVar = m_inputData->Data->GetNumZVar();
    ui.hessianPurifyCheck->setEnabled(numZVar > 0);
    ui.hessianPurifyCheck->setChecked(m_inputData->Hessian->GetPurify());

    ui.hessianPrintCheck->setEnabled(numZVar > 0);
    ui.hessianPrintCheck->setChecked(m_inputData->Hessian->GetPrintFC());

    bool vibrational = m_inputData->Hessian->GetVibAnalysis();
    ui.hessianVibrationalCheck->setChecked(vibrational);

    ui.hessianScaleDouble->setEnabled(vibrational);
    ui.hessianScaleDouble->setValue(m_inputData->Hessian->GetFreqScale());

    ui.hessianDisplacementDouble->setEnabled(!analyticSelect);
    ui.hessianDisplacementDouble->setValue(m_inputData->Hessian->GetDisplacementSize());

    blockChildrenSignals( ui.hessianWidget, false );
  }

  void GamessInputDialog::updateStatPointWidgets()
  {
    blockChildrenSignals( ui.statPointWidget, true );

    int runType = m_inputData->Control->GetRunType();

    ui.statPointStepsSpin->setValue(m_inputData->StatPt->GetMaxSteps());

    ui.statPointConvergenceDouble->setValue(m_inputData->StatPt->GetOptConvergance());

    int method = m_inputData->StatPt->GetMethod();
    ui.statPointOptimizationCombo->setCurrentIndex(method - 1);

    ui.statPointInitialDouble->setEnabled(method != 1);

    float initial = m_inputData->StatPt->GetInitRadius();
    if(initial == 0.0)
    {
      if(method == 5) initial = 0.1;
      else if (runType == 6) initial = 0.2;
      else initial = 0.3;
    }
    ui.statPointInitialDouble->setValue(initial);

    float min = m_inputData->StatPt->GetMinRadius();
    ui.statPointMinDouble->setValue(min);

    float max = m_inputData->StatPt->GetMaxRadius();
    ui.statPointMaxDouble->setValue(max);

    ui.statPointRecalculateSpin->setValue(m_inputData->StatPt->GetHessRecalcInterval());

    int hessianType = m_inputData->StatPt->GetHessMethod();
    if(!hessianType)
    {
      if(runType == 4) hessianType = 1;
      else hessianType = 2;
    }

    m_statPointHessianButtons->button(hessianType-1)->setChecked(true);

    ui.statPointUpdateCheck->setChecked(m_inputData->StatPt->GetRadiusUpdate());

    bool stationary = m_inputData->StatPt->GetStatPoint();
    ui.statPointStationaryCheck->setChecked(stationary);

    ui.statPointPrintCheck->setChecked(m_inputData->StatPt->AlwaysPrintOrbs());

    ui.statPointJumpDouble->setEnabled(stationary);
    ui.statPointJumpDouble->setValue(m_inputData->StatPt->GetStatJump());

    ui.statPointFollowSpin->setEnabled(stationary);
    ui.statPointFollowSpin->setValue(m_inputData->StatPt->GetModeFollow());

    blockChildrenSignals( ui.statPointWidget, false );
  }

  // Basis Tab
  void GamessInputDialog::connectBasis()
  {
    connect( ui.basisSetCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setBasisSet( int ) ) );
    connect( ui.basisSetCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basisSetCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.basisECPCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setBasisECP( int ) ) );
    connect( ui.basisECPCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basisECPCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.basisDSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( setBasisD( int ) ) );
    connect( ui.basisDSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basisDSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.basisFSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( setBasisF( int ) ) );
    connect( ui.basisFSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basisFSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.basisLightSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( setBasisLight( int ) ) );
    connect( ui.basisLightSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basisLightSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.basisPolarCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setBasisPolar( int ) ) );
    connect( ui.basisPolarCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basisPolarCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.basisLCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setBasisDiffuseL( bool ) ) );
    connect( ui.basisLCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basisLCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.basisSCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setBasisDiffuseS( bool ) ) );
    connect( ui.basisSCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.basisSCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

  }

  void GamessInputDialog::connectControl()
  {
    connect( ui.controlRunCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setControlRun( int ) ) );
    connect( ui.controlRunCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlRunCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.controlSCFCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setControlSCF( int ) ) );
    connect( ui.controlSCFCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlSCFCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.controlLocalizationCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setControlLocalization( int ) ) );
    connect( ui.controlLocalizationCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlLocalizationCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.controlExecCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setControlExec( int ) ) );
    connect( ui.controlExecCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlExecCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.controlChargeSpin, SIGNAL( valueChanged( QString ) ),
        this, SLOT( setControlCharge( QString ) ) );
    connect( ui.controlChargeSpin, SIGNAL( valueChanged( QString ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlChargeSpin, SIGNAL( valueChanged( QString ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.controlMultiplicitySpin, SIGNAL( valueChanged( QString ) ),
        this, SLOT( setControlMultiplicity( QString ) ) );
    connect( ui.controlMultiplicitySpin, SIGNAL( valueChanged( QString ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlMultiplicitySpin, SIGNAL( valueChanged( QString ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.controlMaxSCFSpin, SIGNAL( valueChanged( QString ) ),
        this, SLOT( setControlMaxSCF( QString ) ) );
    connect( ui.controlMaxSCFSpin, SIGNAL( valueChanged( QString ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlMaxSCFSpin, SIGNAL( valueChanged( QString ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.controlMP2Check, SIGNAL( toggled( bool ) ),
        this, SLOT( setControlMP2( bool ) ) );
    connect( ui.controlMP2Check, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlMP2Check, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.controlCICombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setControlCI( int ) ) );
    connect( ui.controlCICombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlCICombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.controlCCCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setControlCC( int ) ) );
    connect( ui.controlCCCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlCCCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.controlDFTCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setControlDFT( bool ) ) );
    connect( ui.controlDFTCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.controlDFTCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );
  }

  void GamessInputDialog::connectData()
  {
    connect( ui.dataTitleLine, SIGNAL( textChanged( QString ) ),
        this, SLOT( setDataTitle( QString ) ) );
    connect( ui.dataTitleLine, SIGNAL( textChanged( QString ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.dataTitleLine, SIGNAL( textChanged( QString ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.dataCoordinateCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setDataCoordinate( int ) ) );
    connect( ui.dataCoordinateCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.dataCoordinateCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.dataUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setDataUnits( int ) ) );
    connect( ui.dataUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.dataUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.dataZMatrixSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( setDataZMatrix( int ) ) );
    connect( ui.dataZMatrixSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.dataZMatrixSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.dataPointCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setDataPoint( int ) ) );
    connect( ui.dataPointCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.dataPointCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.dataOrderSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( setDataOrder( int ) ) );
    connect( ui.dataOrderSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.dataOrderSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.dataSymmetryCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setDataSymmetry( bool ) ) );
    connect( ui.dataSymmetryCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.dataSymmetryCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

  }

  void GamessInputDialog::connectSystem()
  {
    connect( ui.systemTimeDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( setTimeLimit( double ) ) );
    connect( ui.systemTimeDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.systemTimeDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.systemTimeCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setTimeLimitUnits( int ) ) );
    connect( ui.systemTimeCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.systemTimeCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.systemMemoryDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( setSystemMemory( double ) ) );
    connect( ui.systemMemoryDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.systemMemoryDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.systemMemoryCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setSystemMemoryUnits( int ) ) );
    connect( ui.systemMemoryCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.systemMemoryCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.systemDDIDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( setSystemDDI( double ) ) );
    connect( ui.systemDDIDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.systemDDIDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.systemDDICombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setSystemDDIUnits( int ) ) );
    connect( ui.systemDDICombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.systemDDICombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.systemDiagonalizationCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setSystemDiagonalization( int ) ) );
    connect( ui.systemDiagonalizationCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.systemDiagonalizationCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.systemExternalCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setSystemExternal( bool ) ) );
    connect( ui.systemExternalCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.systemExternalCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.systemCoreCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setSystemProduce( bool ) ) );
    connect( ui.systemCoreCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.systemCoreCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.systemForceCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setSystemForce( bool ) ) );
    connect( ui.systemForceCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.systemForceCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.systemLoopRadio, SIGNAL( toggled( bool ) ), this, SLOT( setSystemLoop( bool ) ) );
    connect( ui.systemLoopRadio, SIGNAL( toggled( bool ) ), this, SLOT( updatePreviewText() ) );
    connect( ui.systemLoopRadio, SIGNAL( toggled( bool ) ), this, SLOT( advancedChanged() ) );

    connect( ui.systemNextRadio, SIGNAL( toggled( bool ) ), this, SLOT( setSystemNext( bool ) ) );
    connect( ui.systemNextRadio, SIGNAL( toggled( bool ) ), this, SLOT( updatePreviewText() ) );
    connect( ui.systemNextRadio, SIGNAL( toggled( bool ) ), this, SLOT( advancedChanged() ) );
  }

  void GamessInputDialog::connectMOGuess()
  {
    connect( ui.moGuessInitialCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setMOGuessInitial( int ) ) );
    connect( ui.moGuessInitialCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.moGuessInitialCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.moGuessPrintCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setMOGuessPrint( bool ) ) );
    connect( ui.moGuessPrintCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.moGuessPrintCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.moGuessRotateCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setMOGuessRotate( bool ) ) );
    connect( ui.moGuessRotateCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.moGuessRotateCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );
  }

  void GamessInputDialog::connectMisc()
  {
    connect( ui.miscMolPltCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setMiscMolPlt( bool ) ) );
    connect( ui.miscMolPltCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.miscMolPltCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.miscPltOrbCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setMiscPltOrb( bool ) ) );
    connect( ui.miscPltOrbCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.miscPltOrbCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.miscAIMPACCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setMiscAIMPAC( bool ) ) );
    connect( ui.miscAIMPACCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.miscAIMPACCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.miscRPACCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setMiscRPAC( bool ) ) );
    connect( ui.miscRPACCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.miscRPACCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    m_miscForceButtons = new QButtonGroup;
    m_miscForceButtons->addButton(ui.miscNoneRadio, 0);
    m_miscForceButtons->addButton(ui.miscHondoRadio, 1);
    m_miscForceButtons->addButton(ui.miscMELDFRadio, 2);
    m_miscForceButtons->addButton(ui.miscGamessRadio, 3);
    m_miscForceButtons->addButton(ui.miscGaussianRadio, 4);
    m_miscForceButtons->addButton(ui.miscAllRadio, 5);
    connect( m_miscForceButtons, SIGNAL( buttonClicked( int ) ),
        this, SLOT( setMiscForce( int ) ) );
    connect( m_miscForceButtons, SIGNAL( buttonClicked( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( m_miscForceButtons, SIGNAL( buttonClicked( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.miscWaterCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setMiscWater( bool ) ) );
    connect( ui.miscWaterCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.miscWaterCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );
  }

  void GamessInputDialog::connectSCF()
  {
    connect( ui.scfDirectCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setSCFDirect( bool ) ) );
    connect( ui.scfDirectCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.scfDirectCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.scfComputeCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setSCFCompute( bool ) ) );
    connect( ui.scfComputeCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.scfComputeCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.scfGenerateCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setSCFGenerate( bool ) ) );
    connect( ui.scfGenerateCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.scfGenerateCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );
  }

  void GamessInputDialog::connectDFT()
  {
    connect( ui.dftMethodCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setDFTMethod( int ) ) );
    connect( ui.dftMethodCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.dftMethodCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.dftFunctionalCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setDFTFunctional( int ) ) );
    connect( ui.dftFunctionalCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.dftFunctionalCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );
  }

  void GamessInputDialog::connectMP2()
  {
    connect( ui.mp2ElectronsSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( setMP2Electrons( int ) ) );
    connect( ui.mp2ElectronsSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.mp2ElectronsSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.mp2MemorySpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( setMP2Memory( int ) ) );
    connect( ui.mp2MemorySpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.mp2MemorySpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.mp2IntegralLine, SIGNAL( textChanged( QString ) ),
        this, SLOT( setMP2Integral( QString ) ) );
    connect( ui.mp2IntegralLine, SIGNAL( textChanged( QString ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.mp2IntegralLine, SIGNAL( textChanged( QString ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.mp2LocalizedCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setMP2Localized( bool ) ) );
    connect( ui.mp2LocalizedCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.mp2LocalizedCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.mp2ComputeCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setMP2Compute( bool ) ) );
    connect( ui.mp2ComputeCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.mp2ComputeCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.mp2SegmentedRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( setMP2Segmented( bool ) ) );
    connect( ui.mp2SegmentedRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.mp2SegmentedRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.mp2TwoRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( setMP2Two( bool ) ) );
    connect( ui.mp2TwoRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.mp2TwoRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.mp2DuplicatedRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( setMP2Duplicated( bool ) ) );
    connect( ui.mp2DuplicatedRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.mp2DuplicatedRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.mp2DistributedRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( setMP2Distributed( bool ) ) );
    connect( ui.mp2DistributedRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.mp2DistributedRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );
  }

  void GamessInputDialog::connectHessian()
  {
    connect( ui.hessianAnalyticRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( setHessianAnalytic( bool ) ) );
    connect( ui.hessianAnalyticRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.hessianAnalyticRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.hessianNumericRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( setHessianNumeric( bool ) ) );
    connect( ui.hessianNumericRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.hessianNumericRadio, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.hessianDoubleCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setHessianDouble( bool ) ) );
    connect( ui.hessianDoubleCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.hessianDoubleCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.hessianPurifyCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setHessianPurify( bool ) ) );
    connect( ui.hessianPurifyCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.hessianPurifyCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.hessianPrintCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setHessianPrint( bool ) ) );
    connect( ui.hessianPrintCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.hessianPrintCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.hessianVibrationalCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setHessianVibrational( bool ) ) );
    connect( ui.hessianVibrationalCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.hessianVibrationalCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.hessianDisplacementDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( setHessianDisplacement( double ) ) );
    connect( ui.hessianDisplacementDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.hessianDisplacementDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.hessianScaleDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( setHessianScale( double ) ) );
    connect( ui.hessianScaleDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.hessianScaleDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( basicChanged() ) );
  }

  void GamessInputDialog::connectStatPoint()
  {
    connect( ui.statPointStepsSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( setStatPointSteps( int ) ) );
    connect( ui.statPointStepsSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointStepsSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.statPointConvergenceDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( setStatPointConvergence( double ) ) );
    connect( ui.statPointConvergenceDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointConvergenceDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.statPointRecalculateSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( setStatPointRecalculate( int ) ) );
    connect( ui.statPointRecalculateSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointRecalculateSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.statPointOptimizationCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( setStatPointOptimization( int ) ) );
    connect( ui.statPointOptimizationCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointOptimizationCombo, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( advancedChanged() ) );

    m_statPointHessianButtons = new QButtonGroup;
    m_statPointHessianButtons->addButton(ui.statPointGuessButton, 0);
    m_statPointHessianButtons->addButton(ui.statPointReadButton, 1);
    m_statPointHessianButtons->addButton(ui.statPointCalculateButton, 2);
    connect( m_statPointHessianButtons, SIGNAL( buttonClicked( int ) ),
        this, SLOT( setStatPointHessian( int ) ) );
    connect( m_statPointHessianButtons, SIGNAL( buttonClicked( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( m_statPointHessianButtons, SIGNAL( buttonClicked( int ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.statPointJumpDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( setStatPointJump( double ) ) );
    connect( ui.statPointJumpDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointJumpDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.statPointPrintCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setStatPointPrint( bool ) ) );
    connect( ui.statPointPrintCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointPrintCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.statPointFollowSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( setStatPointFollow( int ) ) );
    connect( ui.statPointFollowSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointFollowSpin, SIGNAL( valueChanged( int ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.statPointInitialDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( setStatPointInitial( double ) ) );
    connect( ui.statPointInitialDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointInitialDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.statPointMinDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( setStatPointMin( double ) ) );
    connect( ui.statPointMinDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointMinDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.statPointMaxDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( setStatPointMax( double ) ) );
    connect( ui.statPointMaxDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointMaxDouble, SIGNAL( valueChanged( double ) ),
        this, SLOT( basicChanged() ) );

    connect( ui.statPointUpdateCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setStatPointUpdate( bool ) ) );
    connect( ui.statPointUpdateCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointUpdateCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );

    connect( ui.statPointStationaryCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( setStatPointStationary( bool ) ) );
    connect( ui.statPointStationaryCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( updatePreviewText() ) );
    connect( ui.statPointStationaryCheck, SIGNAL( toggled( bool ) ),
        this, SLOT( advancedChanged() ) );
  }

  void GamessInputDialog::setMode( int mode )
  {
    if ( mode == 0 && m_advancedChanged )
    {
      QMessageBox msgbox( QMessageBox::Warning, tr( "Advanced Settings Changed" ),
          tr( "Advanced settings have changed.\nDiscard?" ),
          QMessageBox::Abort | QMessageBox::Discard, this );
      int response = msgbox.exec();
      if ( response == QMessageBox::Discard )
      {
        updateBasicWidgets();
        updatePreviewText();
      }
    }
    else if ( mode == 1 )
    {
      updateAdvancedWidgets();
    }
    ui.modeTab->setCurrentIndex( mode );
  }

  void GamessInputDialog::resetClicked()
  {
    int current = ui.modeTab->currentIndex();
    if( current )
    {
      QMessageBox msgbox( QMessageBox::Warning, tr( "Advanced Settings Reset" ),
          tr( "Are you sure you wish to reset advanced settings?\n"
            "All changes will be lost!" ),
          QMessageBox::Yes | QMessageBox::No, this );
      int response = msgbox.exec();
      if(response == QMessageBox::Yes)
      {
        setAdvancedDefaults();
        updateAdvancedWidgets();
        updatePreviewText();
        advancedChanged();
      }
    } else {
      QMessageBox msgbox( QMessageBox::Warning, tr( "Basic Settings Reset" ),
          tr( "Are you sure you wish to reset basic settings?\n"
            "All changes will be lost!" ),
          QMessageBox::Yes | QMessageBox::No, this );
      int response = msgbox.exec();
      if(response == QMessageBox::Yes)
      {
        setBasicDefaults();
        updatePreviewText();
      }
    }
  }

  void GamessInputDialog::defaultsClicked()
  {
    int current = ui.modeTab->currentIndex();
    if( current )
    {
      // Advanced Page Defaults
      int tab = ui.advancedStacked->currentIndex();
      switch(tab) {
        case 0:
          setBasisDefaults();
          break;
        case 1:
          setControlDefaults();
          break;
        case 2:
          setSCFDefaults();
          break;
        case 3:
          setDFTDefaults();
          break;
        case 4:
          setMP2Defaults();
          break;
        case 5:
          setHessianDefaults();
          break;
        case 6:
          setStatPointDefaults();
          break;
        case 7:
          setDataDefaults();
          break;
        case 8:
          setSystemDefaults();
          break;
        case 9:
          setMOGuessDefaults();
          break;
        case 10:
          setMiscDefaults();
          break;
      }
      updateAdvancedWidgets();
      advancedChanged();
    }
    else
    {
      setBasicDefaults();
    }
    updatePreviewText();
  }

  void GamessInputDialog::generateClicked()
  {
    saveInputFile(ui.previewText->toPlainText(), tr("GAMESS Input Deck"), QString("inp"));
  }

  // Basic Slots
  void GamessInputDialog::setBasicCalculate( int index )
  {

    TypeOfRun rti = Energy;
    int hessian = 0;
    switch ( index )
    {
      case 1:
        rti = OptimizeRun;
        break;
      case 2:
        rti = SadPointRun;
        break;
      case 3:
        rti = HessianRun;
        hessian = 3;
        break;
    }
    m_inputData->StatPt->SetHessMethod(hessian);
    m_inputData->Control->SetRunType( rti );
  }

  void GamessInputDialog::setBasicWithLeft( int index )
  {
    int basis = 0;
    CCRunType cc = CC_None;
    bool mp2 = false;
    bool dft = false;
    DFTFunctionalsGrid func = DFT_Grid_Slater;
    bool er = true;

    switch ( index )
    {
      case 0:
        basis = 15;
        er = false;
        break;
      case 1:
        basis = 16;
        er = false;
        break;
      case 2:
        break;
      case 3:
        dft = true;
        func = DFT_Grid_B3LYP;
        break;
      case 4:
        mp2 = true;
        break;
      case 5:
        cc = CC_CCSDT;
        break;
    }

    if ( basis != 0 )
    {
      // Clear everything else.
      setBasicWithRight( 1 );
      m_inputData->Basis->SetBasis( basis );
    }
    else
    {
      setBasicWithRight( ui.basicWithRightCombo->currentIndex() );
    }

    m_inputData->Control->SetCCType( cc );
    m_inputData->Control->SetMPLevel( mp2 ? 2 : 0 );
    m_inputData->Control->UseDFT(dft);

    m_inputData->DFT->SetFunctional(func);

    ui.basicWithRightCombo->setEnabled( er );
  }

  void GamessInputDialog::setBasicWithRight( int index )
  {
    GAMESS_BasisSet basis = GAMESS_BS_STO;
    int gauss = 3;
    int nd = 0;
    int np = 0;
    int ecp = 0;
    bool dsp = false;
    bool ds = false;

    switch ( index )
    {
      case 0:
        // $BASIS GBASIS=STO NGAUSS=3 $END
        break;
      case 1:
        // $BASIS GBASIS=MINI $END
        basis = GAMESS_BS_MINI;
        gauss = 0;
        break;
      case 2:
        // $BASIS GBASIS=N21 NGAUSS=3 $END
        basis = GAMESS_BS_N21;
        gauss = 3;
        break;
      case 3:
        // $BASIS GBASIS=N31 NGAUSS=6 NDFUNC=1 $END
        basis = GAMESS_BS_N31;
        gauss = 6;
        nd = 1;
        break;
      case 4:
        // $BASIS GBASIS=N31 NGAUSS=6 NDFUNC=1 NPFUNC=1 $END
        basis = GAMESS_BS_N31;
        gauss = 6;
        nd = 1;
        np = 1;
        break;
      case 5:
        // $BASIS GBASIS=N31 NGAUSS=6 NDFUNC=1 NPFUNC=1 DIFFSP=.T. $END
        basis = GAMESS_BS_N31;
        gauss = 6;
        nd = 1;
        np = 1;
        dsp = true;
        break;
      case 6:
        // $BASIS GBASIS=N31 NGAUSS=6 NDFUNC=2 NPFUNC=1 DIFFSP=.T. $END
        basis = GAMESS_BS_N31;
        gauss = 6;
        nd = 2;
        np = 1;
        dsp = true;
        break;
      case 7:
        // $BASIS GBASIS=N311 NGAUSS=6 NDFUNC=1 NPFUNC=1 DIFFSP=.T. DIFFS=.T. $END
        basis = GAMESS_BS_N311;
        gauss = 2;
        nd = 1;
        np = 1;
        dsp = true;
        ds = true;
        break;
      case 8:
        // $BASIS GBASIS=SBKJC NDFUNC=1 $END\n $CONTRL ECP=SBKJC $END
        basis = GAMESS_BS_SBK;
        nd = 1;
        ecp = 2;
        break;
    }

    m_inputData->Basis->SetBasis( basis );
    m_inputData->Basis->SetNumGauss( gauss );
    m_inputData->Basis->SetNumDFuncs( nd );
    m_inputData->Basis->SetNumPFuncs( np );

    m_inputData->Basis->SetDiffuseSP( dsp );
    m_inputData->Basis->SetDiffuseS( ds );

    m_inputData->Basis->SetECPPotential( ecp );
  }

  void GamessInputDialog::setBasicIn( int index )
  {
    m_inputData->Basis->SetWaterSolvate(index);
  }

  void GamessInputDialog::setBasicOnLeft( int index )
  {
    GAMESS_SCFType scf = GAMESSDefaultSCFType;
    int mult = 1;
    long numElectrons = m_inputData->GetNumElectrons();
    int charge = m_inputData->Control->GetCharge();
    switch ( index )
    {
      case 0:
        // Make sure we can select this.
        if ( (numElectrons + charge) & 1 ) {
          ui.basicOnLeftCombo->setCurrentIndex(1);
          scf = GAMESS_ROHF;
          mult = 2;
        }
        break;
      case 1:
        // $CONTRL SCFTYP=ROHF MULT=2 $END
        scf = GAMESS_ROHF;
        mult = 2;
        break;
      case 2:
        // $CONTRL SCFTYP=ROHF MULT=2 $END
        scf = GAMESS_ROHF;
        mult = 3;
        break;
    }
    m_inputData->Control->SetSCFType( scf );
    m_inputData->Control->SetMultiplicity( mult );
  }

  void GamessInputDialog::setBasicOnRight( int index )
  {
    int charge = 0;
    switch ( index )
    {
      case 0:
        // $CONTRL ICHARG=2 $END
        charge = 2;
        break;
      case 1:
        // $CONTRL ICHARG=1 $END
        charge = 1;
        break;
      case 2:
        // $CONTRL ICHARD=0 $END
        charge = 0;
        break;
      case 3:
        // $CONTRL ICHARG=-1 $END
        charge = -1;
        break;
      case 4:
        // $CONTRL ICHARG=-2 $END
        charge = -2;
        break;
    }

    m_inputData->Control->SetCharge( charge );
  }

  // Basis Slots
  void GamessInputDialog::setBasisSet( int index )
  {
    int basis = 0;
    int gauss = 0;

    if ( index < 2 )
    {
      basis = index + 1;
    }
    else if ( index < 7 )
    {
      basis = 3;
      gauss = index;
    }
    else if ( index < 9 )
    {
      basis = 4;
      gauss = (( index==7 ) ? 3 : 6 );
    }
    else if ( index < 12 )
    {
      basis = 5;
      gauss = index - 5;
    }
    else if ( index < 18 )
    {
      basis = index - 6;
      if ( index == 12 ) gauss = 6;
    }
    else
    {
      basis = index - 6;
    }

    m_inputData->Basis->SetBasis( basis );
    m_inputData->Basis->SetNumGauss( gauss );

    updateAdvancedWidgets();
  }

  void GamessInputDialog::setBasisECP( int index )
  {
    m_inputData->Basis->SetECPPotential( index );
  }

  void GamessInputDialog::setBasisD( int val )
  {
    m_inputData->Basis->SetNumDFuncs( val );
    updateAdvancedWidgets();
  }

  void GamessInputDialog::setBasisF( int val )
  {
    m_inputData->Basis->SetNumFFuncs( val );
    updateAdvancedWidgets();
  }

  void GamessInputDialog::setBasisLight( int val )
  {
    m_inputData->Basis->SetNumPFuncs( val );
    updateAdvancedWidgets();
  }

  void GamessInputDialog::setBasisPolar( int index )
  {
    m_inputData->Basis->SetPolar(( GAMESS_BS_Polarization )( index ) );
  }

  void GamessInputDialog::setBasisDiffuseL( bool state )
  {
    m_inputData->Basis->SetDiffuseSP( state );
  }

  void GamessInputDialog::setBasisDiffuseS( bool state )
  {
    m_inputData->Basis->SetDiffuseS( state );
  }


  // Control Slots
  void GamessInputDialog::setControlRun( int index )
  {
    m_inputData->Control->SetRunType(( TypeOfRun )( index+1 ) );
    updateAdvancedWidgets();
  }

  void GamessInputDialog::setControlSCF( int index )
  {
    m_inputData->Control->SetSCFType(( GAMESS_SCFType )( index +1 ) );
    updateAdvancedWidgets();
  }

  void GamessInputDialog::setControlLocalization( int index )
  {
    m_inputData->Control->SetLocal(( GAMESS_Localization )index );
    updateAdvancedWidgets();
  }

  void GamessInputDialog::setControlExec( int index )
  {
    m_inputData->Control->SetExeType( index );
  }

  void GamessInputDialog::setControlMaxSCF( const QString &text )
  {
    m_inputData->Control->SetMaxIt( text.toInt() );
  }

  void GamessInputDialog::setControlCharge( const QString &text )
  {
    m_inputData->Control->SetCharge( text.toInt() );
  }

  void GamessInputDialog::setControlMultiplicity( const QString &text )
  {
    m_inputData->Control->SetMultiplicity( text.toInt() );
    updateMOGuessWidgets();
  }

  void GamessInputDialog::setControlMP2( bool state )
  {
    if ( state )
      m_inputData->Control->SetMPLevel( 2 );
    else
      m_inputData->Control->SetMPLevel( 0 );

    updateAdvancedWidgets();
  }

  void GamessInputDialog::setControlDFT( bool state )
  {
    m_inputData->Control->UseDFT( state );
    updateAdvancedWidgets();
  }

  void GamessInputDialog::setControlCI( int index )
  {
    m_inputData->Control->SetCIType(( CIRunType )index );
    updateAdvancedWidgets();
  }

  void GamessInputDialog::setControlCC( int index )
  {
    m_inputData->Control->SetCCType(( CCRunType )index );
    updateAdvancedWidgets();
  }


  // Data Slots
  void GamessInputDialog::setDataTitle( const QString &text )
  {
    m_inputData->Data->SetTitle( text.toAscii().constData() );
  }

  void GamessInputDialog::setDataCoordinate( int index )
  {
    m_inputData->Data->SetCoordType(( CoordinateType )( index+1 ) );
    updateAdvancedWidgets();
  }

  void GamessInputDialog::setDataUnits( int index )
  {
    m_inputData->Data->SetUnits( index );
  }

  void GamessInputDialog::setDataZMatrix( int val )
  {
    m_inputData->Data->SetNumZVar( val );
  }

  void GamessInputDialog::setDataPoint( int index )
  {
    m_inputData->Data->SetPointGroup(( GAMESSPointGroup )( index+1 ) );
    updatePointGroupOrderWidgets();
  }

  void GamessInputDialog::setDataOrder( int index )
  {
    m_inputData->Data->SetPointGroupOrder( index+2 );
  }

  void GamessInputDialog::setDataSymmetry( bool state )
  {
    m_inputData->Data->SetUseSym( state );
  }


  void GamessInputDialog::setTimeLimit( double val )
  {
    m_inputData->System->SetConvertedTime( val );
  }

  void GamessInputDialog::setTimeLimitUnits( int index )
  {
    long oldTimeLimit, newTimeLimit;
    double oldFactor, newFactor;

    // get current information
    oldTimeLimit = m_inputData->System->GetTimeLimit();
    oldFactor = m_inputData->System->GetConvertedTime();

    m_inputData->System->SetTimeUnits((TimeUnit)( index + minuteUnit ) );

    // get updated coefficient
    newFactor = m_inputData->System->GetConvertedTime();

    newTimeLimit = ( long ) round((oldTimeLimit * oldFactor / newFactor));
    m_inputData->System->SetTimeLimit(newTimeLimit);
  }

  void GamessInputDialog::setSystemMemory( double val )
  {
    m_inputData->System->SetConvertedMem( val );
  }

  void GamessInputDialog::setSystemMemoryUnits( int index )
  {
    double oldMemory, newMemory, oldFactor, newFactor;

    // get current information
    oldMemory = m_inputData->System->GetMemory();
    oldFactor = m_inputData->System->GetConvertedMem();

    m_inputData->System->SetMemUnits(( MemoryUnit )( index + megaWordsUnit ) );

    // get updated coefficient
    newFactor = m_inputData->System->GetConvertedMem();

    newMemory = oldMemory * (oldFactor / newFactor);
    m_inputData->System->SetMemory( newMemory );
  }

  void GamessInputDialog::setSystemDDI( double val )
  {
    m_inputData->System->SetConvertedMemDDI( val );
  }

  void GamessInputDialog::setSystemDDIUnits( int index )
  {
    double oldMemory, newMemory, oldFactor, newFactor;

    // get current information
    oldMemory = m_inputData->System->GetMemDDI();
    oldFactor = m_inputData->System->GetConvertedMemDDI();

    m_inputData->System->SetMemDDIUnits(( MemoryUnit )( index + megaWordsUnit ) );

    // get updated coefficient
    newFactor = m_inputData->System->GetConvertedMemDDI();

    newMemory = oldMemory * (oldFactor / newFactor);
    m_inputData->System->SetMemDDI( newMemory );
  }

  void GamessInputDialog::setSystemProduce( bool state )
  {
    m_inputData->System->SetCoreFlag( state );
  }

  void GamessInputDialog::setSystemForce( bool state )
  {
    m_inputData->System->SetParallel( state );
  }

  void GamessInputDialog::setSystemDiagonalization( int index )
  {
    m_inputData->System->SetDiag( index );
  }

  void GamessInputDialog::setSystemExternal( bool state )
  {
    m_inputData->System->SetXDR( state );
  }

  void GamessInputDialog::setSystemLoop( bool )
  {
    m_inputData->System->SetBalanceType( 0 );
  }

  void GamessInputDialog::setSystemNext( bool )
  {
    m_inputData->System->SetBalanceType( 1 );
  }

  void GamessInputDialog::setMOGuessInitial( int index )
  {
    int i = index + 1;

    if ( i == 1 )
      i = 0;

    if ( i != m_inputData->Guess->GetGuess() )
    {
      m_inputData->Guess->SetGuess( i );
      updateMOGuessWidgets();
    }
  }

  void GamessInputDialog::setMOGuessPrint( bool state )
  {
    m_inputData->Guess->SetPrintMO( state );
  }

  void GamessInputDialog::setMOGuessRotate( bool state )
  {
    m_inputData->Guess->SetMix( state );
  }

  // Misc Slots
  void GamessInputDialog::setMiscMolPlt( bool state )
  {
    m_inputData->Control->SetMolPlot( state );
  }

  void GamessInputDialog::setMiscPltOrb( bool state )
  {
    m_inputData->Control->SetPlotOrb( state );
  }

  void GamessInputDialog::setMiscAIMPAC( bool state )
  {
    m_inputData->Control->SetAIMPAC( state );
  }

  void GamessInputDialog::setMiscRPAC( bool state )
  {
    m_inputData->Control->SetRPAC( state );
  }

  void GamessInputDialog::setMiscForce( int id )
  {
    m_inputData->Control->SetFriend(( FriendType ) id );
    updateMiscWidgets();
    updateControlWidgets();

  }

  void GamessInputDialog::setMiscWater( bool state )
  {
    m_inputData->Basis->SetWaterSolvate(state);
  }

  // SCF Slots
  void GamessInputDialog::setSCFDirect( bool state )
  {
    m_inputData->SCF->SetDirectSCF( state );
    updateSCFWidgets();
  }

  void GamessInputDialog::setSCFCompute( bool state )
  {
    m_inputData->SCF->SetFockDiff( state );
  }

  void GamessInputDialog::setSCFGenerate( bool state )
  {
    m_inputData->SCF->SetUHFNO( state );
  }

  // DFT Slots
  void GamessInputDialog::setDFTMethod( int index )
  {
    m_inputData->DFT->SetMethodGrid(!index);
  }

  void GamessInputDialog::setDFTFunctional( int index )
  {
    m_inputData->DFT->SetFunctional(index+1);
  }

  // MP2 Slots
  void GamessInputDialog::setMP2Electrons( int val )
  {
    m_inputData->MP2->SetNumCoreElectrons(val);
  }

  void GamessInputDialog::setMP2Memory( int val )
  {
    m_inputData->MP2->SetMemory(val);
  }

  void GamessInputDialog::setMP2Integral( const QString &text )
  {
    double val = text.toDouble();
    if(fabs(val - 1.0e-9) < 1.0e-20) val = 0.0;
    m_inputData->MP2->SetIntCutoff(val);
  }

  void GamessInputDialog::setMP2Localized( bool state )
  {
    m_inputData->MP2->SetLMOMP2(state);
    updateMP2Widgets();
  }

  void GamessInputDialog::setMP2Compute( bool state )
  {
    m_inputData->MP2->SetMP2Prop(state);
  }

  void GamessInputDialog::setMP2Segmented( bool )
  {
    m_inputData->MP2->SetMethod(2);
  }

  void GamessInputDialog::setMP2Two( bool )
  {
    m_inputData->MP2->SetMethod(3);
  }

  void GamessInputDialog::setMP2Duplicated( bool )
  {
    m_inputData->MP2->SetAOIntMethod(1);
  }

  void GamessInputDialog::setMP2Distributed( bool )
  {
    m_inputData->MP2->SetAOIntMethod(2);
  }

  //! Hessian Slots
  void GamessInputDialog::setHessianAnalytic( bool )
  {
    m_inputData->Hessian->SetAnalyticMethod(1);
    updateHessianWidgets();
  }

  void GamessInputDialog::setHessianNumeric( bool )
  {
    m_inputData->Hessian->SetAnalyticMethod(0);
    updateHessianWidgets();
  }

  void GamessInputDialog::setHessianDisplacement( double val )
  {
    m_inputData->Hessian->SetDisplacementSize(val);
  }

  void GamessInputDialog::setHessianDouble( bool state )
  {
    m_inputData->Hessian->SetPrintFC(state);
  }

  void GamessInputDialog::setHessianPurify( bool state )
  {
    m_inputData->Hessian->SetPurify(state);
  }

  void GamessInputDialog::setHessianPrint( bool state )
  {
    m_inputData->Hessian->SetPrintFC(state);
  }

  void GamessInputDialog::setHessianVibrational( bool state )
  {
    m_inputData->Hessian->SetVibAnalysis(state);
  }

  void GamessInputDialog::setHessianScale( double val )
  {
    m_inputData->Hessian->SetFreqScale(val);
  }


  //! Stat Point Slots
  void GamessInputDialog::setStatPointSteps( int val )
  {
    m_inputData->StatPt->SetMaxSteps(val);
  }

  void GamessInputDialog::setStatPointRecalculate( int val )
  {
    m_inputData->StatPt->SetHessRecalcInterval(val);
  }

  void GamessInputDialog::setStatPointConvergence( double val )
  {
    m_inputData->StatPt->SetOptConvergance(val);
  }

  void GamessInputDialog::setStatPointInitial( double val )
  {
    double def = 0.3;

    if(m_inputData->StatPt->GetMethod() == 5) def = 0.1;
    else if(m_inputData->Control->GetRunType() == 6) def = 0.2;
    if(fabs(val-def) < 1.0e-10) val = 0.0;
    m_inputData->StatPt->SetInitRadius(val);
  }

  void GamessInputDialog::setStatPointMin( double val )
  {
    m_inputData->StatPt->SetMinRadius(val);
  }

  void GamessInputDialog::setStatPointMax( double val )
  {
    double def = 0.5;

    if(m_inputData->Control->GetRunType() == 6) def = 0.3;
    if(fabs(val-def) < 1.0e-10) val = 0.0;
    m_inputData->StatPt->SetMaxRadius(val);
  }

  void GamessInputDialog::setStatPointOptimization( int index )
  {
    m_inputData->StatPt->SetMethod(index+1);
    updateStatPointWidgets();
  }

  void GamessInputDialog::setStatPointUpdate( bool state )
  {
    m_inputData->StatPt->SetRadiusUpdate(state);
  }

  void GamessInputDialog::setStatPointStationary( bool state )
  {
    m_inputData->StatPt->SetStatPoint(state);
    updateStatPointWidgets();
  }

  void GamessInputDialog::setStatPointHessian( int value )
  {
    m_inputData->StatPt->SetHessMethod(value+1);
    updateStatPointWidgets();
  }

  void GamessInputDialog::setStatPointPrint( bool state )
  {
    m_inputData->StatPt->SetAlwaysPrintOrbs(state);
  }

  void GamessInputDialog::setStatPointJump( double val )
  {
    m_inputData->StatPt->SetStatJump(val);
  }

  void GamessInputDialog::setStatPointFollow( int val )
  {
    m_inputData->StatPt->SetModeFollow(val);
  }
  
  void GamessInputDialog::readSettings(QSettings& settings)
  {
    m_savePath = settings.value("gamess/savepath").toString();
  }
  
  void GamessInputDialog::writeSettings(QSettings& settings) const
  {
    settings.setValue("gamess/savepath", m_savePath);
  }
}

