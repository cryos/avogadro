/**********************************************************************
  GamessInputBuilder - GAMESS Input Deck Builder for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "gamessinputbuilder.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <QMessageBox>

using namespace Avogadro;

GamessInputBuilder::GamessInputBuilder( GamessInputData *inputData, QWidget *parent, Qt::WindowFlags f ) : QDialog( parent, f ), m_inputData( inputData ),
    m_advancedChanged( false )
{
  m_mainLayout = new QVBoxLayout;
  m_stackedLayout = new QStackedLayout;
  setLayout( m_mainLayout );

  setWindowTitle( tr( "GAMESS Input Deck Builder" ) );

  createModes();
  createBasic();
  createAdvanced();
  createPreview();
  createButtons();

  m_stackedLayout->addWidget( m_basicWidget );
  m_stackedLayout->addWidget( m_advancedWidget );
  m_mainLayout->addLayout( m_modeLayout );
  m_mainLayout->addLayout( m_stackedLayout );
  m_mainLayout->addWidget( m_previewText );
  m_mainLayout->setStretchFactor( m_previewText, 1 );
  m_mainLayout->addLayout( m_buttonLayout );

  setBasicDefaults();
  updatePreviewText();
}

GamessInputBuilder::~GamessInputBuilder()
{}

void GamessInputBuilder::createBasic()
{
  int row=0;
  QHBoxLayout *layout;

  m_basicWidget = new QFrame( this );
  m_basicWidget->setFrameShape( QFrame::StyledPanel );
  m_basicWidget->setFrameShadow( QFrame::Raised );
  QGridLayout *basicLayout = new QGridLayout( m_basicWidget );
  m_basicWidget->setLayout( basicLayout );

  basicLayout->setColumnMinimumWidth( 0, 120 );
  basicLayout->setColumnStretch( 1, 1 );
  basicLayout->setColumnStretch( 2, 1 );

  // Calculate
  m_basicCalculateLabel = new QLabel( tr( "Calculate:" ) );
  m_basicCalculateLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  basicLayout->addWidget( m_basicCalculateLabel, row, 0 );
  m_basicCalculateCombo = new QComboBox;
  m_basicCalculateCombo->addItem( tr( "Single Point Entry" ) );
  m_basicCalculateCombo->addItem( tr( "Equilibrium Geometry" ) );
  m_basicCalculateCombo->addItem( tr( "Transition State" ) );
  m_basicCalculateCombo->addItem( tr( "Frequencies" ) );
  connect( m_basicCalculateCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasicCalculateIndex( int ) ) );
  connect( m_basicCalculateCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basicCalculateCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( basicChanged() ) );
  basicLayout->addWidget( m_basicCalculateCombo, row, 1 );

  row++;
  // With
  m_basicWithLabel = new QLabel( tr( "With:" ) );
  m_basicWithLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  basicLayout->addWidget( m_basicWithLabel, row, 0 );
  m_basicWithLeftCombo = new QComboBox;
  m_basicWithLeftCombo->addItem( tr( "AM1" ) );
  m_basicWithLeftCombo->addItem( tr( "PM3" ) );
  m_basicWithLeftCombo->addItem( tr( "RHF" ) );
  m_basicWithLeftCombo->addItem( tr( "B3LYP" ) );
  m_basicWithLeftCombo->addItem( tr( "MP2" ) );
  m_basicWithLeftCombo->addItem( tr( "CCSD(T)" ) );
  connect( m_basicWithLeftCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasicWithLeftIndex( int ) ) );
  connect( m_basicWithLeftCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basicWithLeftCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( basicChanged() ) );
  basicLayout->addWidget( m_basicWithLeftCombo, row, 1 );
  layout = new QHBoxLayout;
  m_basicWithRightCombo = new QComboBox;
  m_basicWithRightCombo->addItem( tr( "STO-3G" ) );
  m_basicWithRightCombo->addItem( tr( "MINI" ) );
  m_basicWithRightCombo->addItem( tr( "3-21G" ) );
  m_basicWithRightCombo->addItem( tr( "6-31G(d)" ) );
  m_basicWithRightCombo->addItem( tr( "6-31G(d,p)" ) );
  m_basicWithRightCombo->addItem( tr( "6-31+G(d,p)" ) );
  m_basicWithRightCombo->addItem( tr( "6-31+G(2d,p)" ) );
  m_basicWithRightCombo->addItem( tr( "6-311++G(2d,p)" ) );
  m_basicWithRightCombo->addItem( tr( "Core Potential" ) );
  connect( m_basicWithRightCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasicWithRightIndex( int ) ) );
  connect( m_basicWithRightCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basicWithRightCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( basicChanged() ) );
  layout->addWidget( m_basicWithRightCombo );
  layout->addStretch();
  basicLayout->addLayout( layout, row, 2 );

  row++;
  // In
  m_basicInLabel = new QLabel( tr( "In:" ) );
  m_basicInLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  basicLayout->addWidget( m_basicInLabel, row, 0 );
  m_basicInCombo = new QComboBox;
  m_basicInCombo->addItem( tr( "Gas Phase" ) );
  m_basicInCombo->addItem( tr( "Water" ) );
  connect( m_basicInCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasicInIndex( int ) ) );
  connect( m_basicInCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basicInCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( basicChanged() ) );
  basicLayout->addWidget( m_basicInCombo, row, 1 );

  row++;
  // On
  m_basicOnLabel = new QLabel( tr( "On:" ) );
  m_basicOnLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  basicLayout->addWidget( m_basicOnLabel, row, 0 );
  m_basicOnLeftCombo = new QComboBox;
  m_basicOnLeftCombo->addItem( tr( "Singlet" ) );
  m_basicOnLeftCombo->addItem( tr( "Doublet" ) );
  m_basicOnLeftCombo->addItem( tr( "Triplet" ) );
  connect( m_basicOnLeftCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasicOnLeftIndex( int ) ) );
  connect( m_basicOnLeftCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basicOnLeftCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( basicChanged() ) );
  basicLayout->addWidget( m_basicOnLeftCombo, row, 1 );
  layout = new QHBoxLayout;
  m_basicOnRightCombo = new QComboBox;
  m_basicOnRightCombo->addItem( tr( "Dication" ) );
  m_basicOnRightCombo->addItem( tr( "Cation" ) );
  m_basicOnRightCombo->addItem( tr( "Neutral" ) );
  m_basicOnRightCombo->addItem( tr( "Anion" ) );
  m_basicOnRightCombo->addItem( tr( "Dianion" ) );
  connect( m_basicOnRightCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasicOnRightIndex( int ) ) );
  connect( m_basicOnRightCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basicOnRightCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( basicChanged() ) );
  layout->addWidget( m_basicOnRightCombo );
  layout->addStretch();
  basicLayout->addLayout( layout, row, 2 );

  row++;
  // Time Limit
  m_basicTimeLabel = new QLabel( tr( "Time Limit:" ) );
  m_basicTimeLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  basicLayout->addWidget( m_basicTimeLabel, row, 0 );
  m_basicTimeLine = new QLineEdit;
  connect( m_basicTimeLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setBasicTimeLimitText( QString ) ) );
  connect( m_basicTimeLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basicTimeLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  basicLayout->addWidget( m_basicTimeLine, row, 1 );
  m_basicTimeUnitsLabel = new QLabel( tr( "Hour(s)" ) );
  basicLayout->addWidget( m_basicTimeUnitsLabel, row, 2 );

  row++;
  // Memory
  m_basicMemoryLabel = new QLabel( tr( "Memory:" ) );
  m_basicMemoryLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  basicLayout->addWidget( m_basicMemoryLabel, row, 0 );
  m_basicMemoryLine = new QLineEdit;
  connect( m_basicMemoryLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setBasicMemoryText( QString ) ) );
  connect( m_basicMemoryLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basicMemoryLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  basicLayout->addWidget( m_basicMemoryLine, row, 1 );
  m_basicMemoryUnitsLabel = new QLabel( tr( "MegaBytes" ) );
  basicLayout->addWidget( m_basicMemoryUnitsLabel, row, 2 );
}

void GamessInputBuilder::createAdvanced()
{
  QWidget *widget;


  // Tabs
  m_advancedWidget= new QTabWidget( this );
  m_advancedWidget->setUsesScrollButtons( false );

  createBasis();
  widget = new QWidget( this );
  widget->setLayout( m_basisLayout );
  m_basisIndex = m_advancedWidget->addTab( widget, tr( "Basis" ) );

  createControl();
  widget = new QWidget( this );
  widget->setLayout( m_controlLayout );
  m_controlIndex = m_advancedWidget->addTab( widget, tr( "Control" ) );

  createData();
  widget = new QWidget( this );
  widget->setLayout( m_dataLayout );
  m_dataIndex = m_advancedWidget->addTab( widget, tr( "Data" ) );

  createSystem();
  widget = new QWidget( this );
  widget->setLayout( m_systemLayout );
  m_systemIndex = m_advancedWidget->addTab( widget, tr( "System" ) );

  createMOGuess();
  widget = new QWidget( this );
  widget->setLayout( m_moGuessLayout );
  m_moGuessIndex = m_advancedWidget->addTab( widget, tr( "MO Guess" ) );

  createMisc();
  widget = new QWidget( this );
  widget->setLayout( m_miscLayout );
  m_miscIndex = m_advancedWidget->addTab( widget, tr( "Misc" ) );

  createSCF();
  widget = new QWidget( this );
  widget->setLayout( m_scfLayout );
  m_scfIndex = m_advancedWidget->addTab( widget, tr( "SCF" ) );

  createDFT();
  widget = new QWidget( this );
  widget->setLayout( m_dftLayout );
  m_dftIndex = m_advancedWidget->addTab( widget, tr( "DFT" ) );

  createMP2();
  widget = new QWidget( this );
  widget->setLayout( m_mp2Layout );
  m_mp2Index = m_advancedWidget->addTab( widget, tr( "MP2" ) );

  createHessian();
  widget = new QWidget( this );
  widget->setLayout( m_hessianLayout );
  m_hessianIndex = m_advancedWidget->addTab( widget, tr( "Hessian" ) );

  createStatPoint();
  widget = new QWidget( this );
  widget->setLayout( m_statPointLayout );
  m_statPointIndex = m_advancedWidget->addTab( widget, tr( "Stat Point" ) );

}

void GamessInputBuilder::createPreview()
{
  m_previewText = new QTextEdit( this );
}

void GamessInputBuilder::createModes()
{
  m_modeButtons = new QButtonGroup( this );
  m_modeButtons->setExclusive( true );
  connect( m_modeButtons, SIGNAL( buttonClicked( int ) ), this, SLOT( setMode( int ) ) );

  // Modes Button Bar
  m_modeLayout = new QHBoxLayout();
  m_modeLayout->setMargin( 0 );
  m_modeBasicButton = new QPushButton( tr( "&Basic" ) );
  m_modeBasicButton->setCheckable( true );
  m_modeBasicButton->setChecked( true );
  m_modeButtons->addButton( m_modeBasicButton, 0 );
  m_modeLayout->addWidget( m_modeBasicButton );
  m_modeAdvancedButton = new QPushButton( tr( "&Advanced" ) );
  m_modeAdvancedButton->setCheckable( true );
  m_modeButtons->addButton( m_modeAdvancedButton, 1 );
  m_modeLayout->addWidget( m_modeAdvancedButton );
  m_modeLayout->addStretch();
}

void GamessInputBuilder::createButtons()
{
  QPushButton *button;
  // Button Bar
  m_buttonLayout = new QHBoxLayout();
  m_buttonLayout->setMargin( 0 );
  button = new QPushButton( tr( "Reset All" ) );
  connect( button, SIGNAL( clicked() ), this, SLOT( resetClicked() ) );
  m_buttonLayout->addWidget( button );
  button = new QPushButton( tr( "Defaults" ) );
  connect( button, SIGNAL( clicked() ), this, SLOT( defaultsClicked() ) );
  m_buttonLayout->addWidget( button );
  m_buttonLayout->addStretch();
  button = new QPushButton( tr( "&Export" ) );
  connect( button, SIGNAL( clicked() ), this, SLOT( exportClicked() ) );
  m_buttonLayout->addWidget( button );
  button = new QPushButton( tr( "&Close" ) );
  connect( button, SIGNAL( clicked() ), this, SLOT( closeClicked() ) );
  m_buttonLayout->addWidget( button );
}

void GamessInputBuilder::basicChanged()
{
  m_basicWidget->setEnabled( true );
  m_advancedChanged = false;
}

void GamessInputBuilder::advancedChanged()
{
  m_basicWidget->setEnabled( false );
  m_advancedChanged = true;
}

void GamessInputBuilder::setBasicDefaults()
{
  // These rely on the advanced defaults
  setAdvancedDefaults();

  blockChildrenSignals(m_basicWidget, true);

  m_basicCalculateCombo->setCurrentIndex( 0 );
  setBasicCalculateIndex(0);
  m_basicWithLeftCombo->setCurrentIndex( 2 );
  setBasicWithRightIndex(2);
  m_basicWithRightCombo->setCurrentIndex( 0 );
  setBasicWithRightIndex(0);
  m_basicInCombo->setCurrentIndex( 0 );
  setBasicInIndex(0);
  m_basicOnLeftCombo->setCurrentIndex( 0 );
  setBasicOnLeftIndex(0);
  m_basicOnRightCombo->setCurrentIndex( 2 );
  setBasicOnRightIndex(2);
  m_basicTimeLine->setText( tr( "10" ) );
  setBasicTimeLimitText(tr("10"));
  m_basicMemoryLine->setText( tr( "50" ) );
  setBasicMemoryText(tr("50"));

  blockChildrenSignals(m_basicWidget, false);

  m_advancedChanged = false;
  m_basicWidget->setEnabled( true );

//   updateAdvancedWidgets();

}
void GamessInputBuilder::setAdvancedDefaults()
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
}

void GamessInputBuilder::setBasisDefaults()
{
  m_inputData->Basis->InitData();
  updateBasisWidgets();
//   m_basisSetCombo->setCurrentIndex( 3 );
//   m_basisECPCombo->setCurrentIndex( 0 );
//   m_basisDCombo->setCurrentIndex( 0 );
//   m_basisFCombo->setCurrentIndex( 0 );
//   m_basisLightCombo->setCurrentIndex( 0 );
//   m_basisPolarCombo->setCurrentIndex( 0 );
//   m_basisDiffuseLCheck->setChecked( false );
//   m_basisDiffuseSCheck->setChecked( false );
}

void GamessInputBuilder::setControlDefaults()
{
  m_inputData->Control->InitControlPaneData();
  updateControlWidgets();
//   m_controlRunCombo->setCurrentIndex( 0 );
//   m_controlSCFCombo->setCurrentIndex( 0 );
//   m_controlLocalizationCombo->setCurrentIndex( 0 );
//   m_controlExecCombo->setCurrentIndex( 0 );
//   m_controlMaxSCFLine->setText( tr( "30" ) );
//   m_controlChargeLine->setText( tr( "0" ) );
//   m_controlMultiplicityLine->setText( tr( "1" ) );
//   m_controlMP2Check->setChecked( false );
//   m_controlDFTCheck->setChecked( false );
//   m_controlCICombo->setCurrentIndex( 0 );
//   m_controlCCCombo->setCurrentIndex( 0 );
}

void GamessInputBuilder::setDataDefaults()
{
  m_inputData->Data->InitData();
  updateDataWidgets();
//   m_dataTitleLine->setText( "Title" );
//   m_dataCoordinateCombo->setCurrentIndex( 0 );
//   m_dataUnitsCombo->setCurrentIndex( 0 );
//   m_dataZMatrixLine->setText( tr( "0" ) );
//   m_dataPointCombo->setCurrentIndex( 0 );
//   m_dataOrderCombo->setCurrentIndex( 0 );
//   m_dataSymmetryCheck->setChecked( true );
}

void GamessInputBuilder::setSystemDefaults()
{
  m_inputData->System->InitData();
  updateSystemWidgets();
//   m_systemTimeUnitsCombo->setCurrentIndex( 2 );
//   m_systemTimeLine->setText( tr( "10" ) );
//   m_systemMemoryUnitsCombo->setCurrentIndex( 3 );
//   m_systemMemoryLine->setText( tr( "50" ) );
//   m_systemDDICombo->setCurrentIndex( 0 );
//   m_systemDDILine->setText( tr( "0" ) );
//   m_systemDiagnalizationCombo->setCurrentIndex( 0 );
//   m_systemCoreCheck->setChecked( false );
//   m_systemForceCheck->setChecked( false );
//   m_systemLoopRadio->setChecked( false );
//   m_systemExternalCheck->setChecked( false );
}

void GamessInputBuilder::setMOGuessDefaults()
{
  m_inputData->Guess->InitData();
  updateMOGuessWidgets();
//   m_moGuessInitialCombo->setCurrentIndex( 0 );
//   m_moGuessPrintCheck->setChecked( false );
//   m_moGuessRotateCheck->setChecked( false );
}

void GamessInputBuilder::setMiscDefaults()
{
  m_inputData->Control->InitProgPaneData();
  m_inputData->Basis->SetWaterSolvate(false);
  updateMiscWidgets();
//   m_miscMolPltCheck->setChecked( false );
//   m_miscPltOrbCheck->setChecked( false );
//   m_miscAIMPACCheck->setChecked( false );
//   m_miscRPACKCheck->setChecked( false );
//   m_miscForceButtons->button( 0 )->setChecked( true );
}

void GamessInputBuilder::setSCFDefaults()
{
  m_inputData->SCF->InitData();
  updateSCFWidgets();
//   m_scfDirectCheck->setChecked( false );
//   m_scfComputeCheck->setChecked( false );
//   m_scfGenerateCheck->setChecked( false );
}

void GamessInputBuilder::setDFTDefaults()
{
  m_inputData->DFT->InitData();
  updateDFTWidgets();
//   m_dftMethodCombo->setCurrentIndex(0);
//   m_dftFunctionalCombo->setCurrentIndex(0);
}

void GamessInputBuilder::setMP2Defaults()
{
  m_inputData->MP2->InitData();
  updateMP2Widgets();
//   m_mp2ElectronsLine->setText(tr("0"));
//   m_mp2MemoryLine->setText(tr("All"));
//   m_mp2IntegralLine->setText(tr("1e-09"));
// 
//   m_mp2SegmentedRadio->setChecked(true);
//   m_mp2DistributedRadio->setChecked(true);
// 
//   m_mp2LocalizedCheck->setChecked(false);
//   m_mp2ComputeCheck->setChecked(false);
}

void GamessInputBuilder::setHessianDefaults()
{
  m_inputData->Hessian->InitData();
  updateHessianWidgets();
//   m_hessianNumericRadio->setChecked(true);
//   m_hessianDoubleCheck->setChecked(false);
//   m_hessianPurifyCheck->setChecked(false);
//   m_hessianPrintCheck->setChecked(false);
//   m_hessianVibrationalCheck->setChecked(false);
// 
//   m_hessianDisplacementLine->setText(tr("0.01"));
//   m_hessianScaleLine->setText(tr("1"));
}

void GamessInputBuilder::setStatPointDefaults()
{
  m_inputData->StatPt->InitData();
  updateStatPointWidgets();
//   m_statPointStepsLine->setText(tr("20"));
//   m_statPointConvergenceLine->setText(tr("0.0001"));
//   m_statPointRecalculateLine->setText(tr("0"));
//   m_statPointInitialLine->setText(tr("0.2"));
//   m_statPointMinLine->setText(tr("0.05"));
//   m_statPointMaxLine->setText(tr("0"));
// 
//   m_statPointOptimizationCombo->setCurrentIndex(2);
//   m_statPointHessianButtons->button(1)->setChecked(true);
// 
//   m_statPointPrintCheck->setChecked(false);
// 
//   m_statPointUpdateCheck->setChecked(true);
//   m_statPointStationaryCheck->setChecked(false);
// 
//   m_statPointJumpLine->setText(tr("1"));
}

void GamessInputBuilder::blockChildrenSignals( QLayout *layout, bool block )
{
  blockChildrenSignals( layout->parentWidget(), block );
}

void GamessInputBuilder::blockChildrenSignals( QObject *object, bool block )
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

void GamessInputBuilder::blockAdvancedSignals( bool block )
{
  int count = m_advancedWidget->count();
  for ( int i = 0; i < count; i++ )
  {
    blockChildrenSignals( m_advancedWidget->widget( i ), block );
  }
}

void GamessInputBuilder::updatePreviewText()
{
  stringstream str;
  m_inputData->WriteInputFile( str );
  m_previewText->setText( QString::fromAscii( str.str().c_str() ) );
}

void GamessInputBuilder::updateBasicWidgets()
{
  // These rely on the advanced defaults
  setAdvancedDefaults();

  setBasicCalculateIndex( m_basicCalculateCombo->currentIndex() );
  setBasicWithLeftIndex( m_basicWithLeftCombo->currentIndex() );
  setBasicWithRightIndex( m_basicWithRightCombo->currentIndex() );
  setBasicInIndex( m_basicInCombo->currentIndex() );
  setBasicOnLeftIndex( m_basicOnLeftCombo->currentIndex() );
  setBasicOnRightIndex( m_basicOnRightCombo->currentIndex() );
  setBasicTimeLimitText( m_basicTimeLine->text() );
  setBasicMemoryText( m_basicMemoryLine->text() );

  m_advancedChanged = false;
  m_basicWidget->setEnabled( true );
}

void GamessInputBuilder::updateAdvancedWidgets()
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

  int runType = m_inputData->Control->GetRunType();
  m_advancedWidget->setTabEnabled( m_statPointIndex, ( runType == 4 ||runType == 6 ) );
  m_advancedWidget->setTabEnabled( m_mp2Index, m_inputData->Control->GetMPLevel() == 2 );
  m_advancedWidget->setTabEnabled( m_hessianIndex, ( runType == 3 ) ||
                                   (( runType == 3 || runType == 6 ) &&
                                    m_inputData->StatPt->GetHessMethod() == 3 ) );
  m_advancedWidget->setTabEnabled( m_dftIndex, m_inputData->Control->UseDFT() );
  m_advancedWidget->setTabEnabled( m_scfIndex, m_inputData->Control->GetSCFType() <= 4 );
}

void GamessInputBuilder::updateBasisWidgets()
{
  blockChildrenSignals( m_basisLayout, true );
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
  m_basisSetCombo->setCurrentIndex( itemValue - 1 );

  //TODO: Implement this.
  //   CheckBasisMenu();

  // m_basisECPCombo
  if ( basis == 12 || basis == 13 )
  {
    m_basisECPCombo->setEnabled( true );
    m_basisECPLabel->setEnabled( true );
    itemValue = m_inputData->Basis->GetECPPotential();
    if ( basis == 12 && ( itemValue == 3 || itemValue == 0 ) ) itemValue=2;
    else if ( basis == 13 && ( itemValue == 2 || itemValue == 0 ) ) itemValue=3;
    m_basisECPCombo->setCurrentIndex( itemValue );
  }
  else
  {
    m_basisECPCombo->setCurrentIndex( 0 );
    m_basisECPCombo->setEnabled( false );
    m_basisECPLabel->setEnabled( false );
  }

  // polarChoice
  testValue = m_inputData->Basis->GetNumPFuncs() +
              m_inputData->Basis->GetNumDFuncs() +
              m_inputData->Basis->GetNumFFuncs();
  if ( testValue )
  {
    m_basisPolarCombo->setEnabled( true );
    m_basisPolarLabel->setEnabled( true );
    itemValue = m_inputData->Basis->GetPolar();
    if ( itemValue == 0 )
    {
      if ( basis == 6 || basis == 11 ) itemValue = 2;
      else if ( basis == 7 || basis == 8 ) itemValue = 3;
      else if ( basis < 3 ) itemValue = 4;
      else if ( basis == 10 ) itemValue = 5;
      else itemValue = 1;
    }
    m_basisPolarCombo->setCurrentIndex( itemValue - 1 );
  }
  else
  {
    m_basisPolarCombo->setEnabled( false );
    m_basisPolarLabel->setEnabled( false );
  }

  // diffuseLCheck
  m_basisDiffuseLCheck->setChecked( m_inputData->Basis->GetDiffuseSP() );

  // diffuseSCheck
  m_basisDiffuseSCheck->setChecked( m_inputData->Basis->GetDiffuseS() );

  // numDChoice
  m_basisDCombo->setCurrentIndex( m_inputData->Basis->GetNumDFuncs() );

  // numFChoice
  m_basisFCombo->setCurrentIndex( m_inputData->Basis->GetNumFFuncs() );

  // numPChoice
  m_basisLightCombo->setCurrentIndex( m_inputData->Basis->GetNumPFuncs() );
  blockChildrenSignals( m_basisLayout, false );
}

void GamessInputBuilder::updateControlWidgets()
{
  blockChildrenSignals( m_controlLayout, true );
  short mp2 = m_inputData->Control->GetMPLevel();
  bool dft = m_inputData->Control->UseDFT();
  short ci = m_inputData->Control->GetCIType();
  CCRunType cc = m_inputData->Control->GetCCType();
  long scft = m_inputData->Control->GetSCFType();
  long NumElectrons = m_inputData->GetNumElectrons();

  int itemValue = 0;

  itemValue = m_inputData->Control->GetRunType();
  if ( itemValue == 0 ) itemValue = 1;
  m_controlRunCombo->setCurrentIndex( itemValue - 1 );

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
  m_controlSCFCombo->setCurrentIndex( scft - 1 );

  // mp2Check
  if ( ci || cc || dft || ( mp2 < 0 ) )
  {
    m_controlMP2Check->setChecked( false );
    m_controlMP2Check->setEnabled( false );
  }
  else
  {
    m_controlMP2Check->setEnabled( true );
    if ( mp2 < 0 ) mp2 = 0;
    if ( mp2 == 2 ) mp2 = 1;
    m_controlMP2Check->setChecked( mp2 );
  }

  // dftCheck
  if ( ci || cc || ( mp2 > 0 ) || ( scft > 3 ) )
  {
    m_controlDFTCheck->setChecked( false );
    m_controlDFTCheck->setEnabled( false );
  }
  else
  {
    m_controlDFTCheck->setEnabled( true );
    m_controlDFTCheck->setChecked( dft );
  }

  // ciCombo
  if (( mp2 > 0 ) || dft || cc || scft == 2 )
  {
    m_controlCICombo->setCurrentIndex( 0 );
    m_controlCICombo->setEnabled( false );
    m_controlCILabel->setEnabled( false );
  }
  else
  {
    m_controlCILabel->setEnabled( true );
    m_controlCICombo->setEnabled( true );
    m_controlCICombo->setCurrentIndex( ci );
  }

  // ccCombo
  if (( mp2 > 0 ) || dft || ci || scft > 1 )
  {
    m_controlCCCombo->setCurrentIndex( 0 );
    m_controlCCCombo->setEnabled( false );
    m_controlCCLabel->setEnabled( false );
  }
  else
  {
    m_controlCCLabel->setEnabled( true );
    m_controlCCCombo->setEnabled( true );
    m_controlCCCombo->setCurrentIndex( cc );
  }

  // scfIterText
  itemValue = m_inputData->Control->GetMaxIt();
  if ( itemValue <= 0 ) itemValue = 30;
  m_controlMaxSCFLine->setText( QString::number( itemValue ) );

  // exeCombo
  m_controlExecCombo->setCurrentIndex( m_inputData->Control->GetExeType() );
  if ( m_inputData->Control->GetFriend() != Friend_None )
  {
    //The friend keyword choices force a check run type
    m_controlExecCombo->setEnabled( false );
    m_controlExecCombo->setCurrentIndex( 1 );
  }
  else
  {
    m_controlExecCombo->setEnabled( true );
  }

  // mchargeText
  m_controlChargeLine->setText( QString::number( m_inputData->Control->GetCharge() ) );

  // multText
  itemValue = m_inputData->Control->GetMultiplicity();
  if ( itemValue <= 0 )
  {
    if ( NumElectrons & 1 ) itemValue == 2;
    else itemValue = 1;
  }
  m_controlMultiplicityLine->setText( QString::number( itemValue ) );

  // localCombo
  m_controlLocalizationCombo->setCurrentIndex( m_inputData->Control->GetLocal() );
  blockChildrenSignals( m_controlLayout, false );
}

void GamessInputBuilder::updateDataWidgets()
{
  blockChildrenSignals( m_dataLayout, true );
  int itemValue;

  //Title
  if ( m_inputData->Data->GetTitle() )
    m_dataTitleLine->setText( tr( m_inputData->Data->GetTitle() ) );
  else
    m_dataTitleLine->setText( tr( "Title" ) );

  // coordTypeCombo
  itemValue = m_inputData->Data->GetCoordType();
  if ( itemValue == 0 ) itemValue = 1;
  m_dataCoordinateCombo->setCurrentIndex( itemValue - 1 );

  // unitCombo
  m_dataUnitsCombo->setCurrentIndex( m_inputData->Data->GetUnits() );

  //# Z-Matrix vars
  m_dataZMatrixLine->setText( QString::number( m_inputData->Data->GetNumZVar() ) );

  //Point Group
  itemValue = m_inputData->Data->GetPointGroup();
  if ( itemValue == 0 ) itemValue = 1;
  m_dataPointCombo->setCurrentIndex( itemValue-1 );

  //Point group order
  updatePointGroupOrderWidgets();

  // symmetryCheck
  m_dataSymmetryCheck->setChecked( m_inputData->Data->GetUseSym() );
  blockChildrenSignals( m_dataLayout, false );
}

void GamessInputBuilder::updateSystemWidgets()
{
  blockChildrenSignals( m_systemLayout, true );

  // timeLimitText
  m_systemTimeLine->setText( QString::number( m_inputData->System->GetConvertedTime() ) );

  // timeLimitUnitCombo
  m_systemTimeUnitsCombo->setCurrentIndex( m_inputData->System->GetTimeUnits() - 1 );

  // memoryText
  m_systemMemoryLine->setText( QString::number( m_inputData->System->GetConvertedMem() ) );

  // memoryUnitCombo
  m_systemMemoryUnitsCombo->setCurrentIndex( m_inputData->System->GetMemUnits() - 1 );

  // memDDI edit
  m_systemDDILine->setText( QString::number( m_inputData->System->GetConvertedMemDDI() ) );

  // memDDIUnitCombo
  m_systemDDICombo->setCurrentIndex( m_inputData->System->GetMemDDIUnits() - megaWordsUnit );

  // diagCombo
  m_systemDiagnalizationCombo->setCurrentIndex( m_inputData->System->GetDiag() );

  m_systemCoreCheck->setChecked( m_inputData->System->GetCoreFlag() );

  if ( m_inputData->System->GetBalanceType() )
    m_systemNextRadio->setChecked( true );
  else
    m_systemLoopRadio->setChecked( true );

  m_systemExternalCheck->setChecked( m_inputData->System->GetXDR() );

  // Parall check
  m_systemForceCheck->setChecked( m_inputData->System->GetParallel() );

  blockChildrenSignals( m_systemLayout, false );
}

void GamessInputBuilder::updatePointGroupOrderWidgets()
{
  m_dataOrderCombo->blockSignals( true );
  //Point group order - only applicable to certain point groups
  int itemValue = m_inputData->Data->GetPointGroup();
  if ( itemValue == 0 ) itemValue = 1;
  if (( itemValue>3 )&&( itemValue<11 ) )
  {
    m_dataOrderCombo->setEnabled( true );
    itemValue = m_inputData->Data->GetPointGroupOrder()-1;
    if ( itemValue <= 0 )
    {
      itemValue = 1;
      m_inputData->Data->SetPointGroupOrder( 2 );
    }
    m_dataOrderCombo->setCurrentIndex( itemValue-1 );
  }
  else m_dataOrderCombo->setEnabled( false );
  m_dataOrderCombo->blockSignals( false );
}

void GamessInputBuilder::updateMOGuessWidgets()
{
  blockChildrenSignals( m_moGuessLayout, true );
  long guess = m_inputData->Guess->GetGuess();
  if ( guess < 1 )
  {
    guess = 1;
  }
  m_moGuessInitialCombo->setCurrentIndex( guess-1 );

  int multiplicity = m_inputData->Control->GetMultiplicity();
  m_moGuessRotateCheck->setEnabled(( m_inputData->Control->GetSCFType() == 2 ) &&
                                   ( multiplicity == 1 || multiplicity == 2 ) );
  m_moGuessRotateCheck->setChecked( m_inputData->Guess->GetMix() );
  m_moGuessPrintCheck->setChecked( m_inputData->Guess->GetPrintMO() );

  blockChildrenSignals( m_moGuessLayout, false );
}

void GamessInputBuilder::updateMiscWidgets()
{
  blockChildrenSignals( m_miscLayout, true );

  m_miscMolPltCheck->setChecked( m_inputData->Control->GetMolPlot() );
  m_miscPltOrbCheck->setChecked( m_inputData->Control->GetPlotOrb() );

  m_miscAIMPACCheck->setChecked( m_inputData->Control->GetAIMPAC() );
  m_miscRPACKCheck->setChecked( m_inputData->Control->GetRPAC() );

  int force = m_inputData->Control->GetFriend();
  if (( m_inputData->Control->GetExeType() != 1 ) &&
      ( force == Friend_None ) )
  {
    m_miscAIMPACCheck->setEnabled( true );
    m_miscRPACKCheck->setEnabled( true );
  }
  else
  {
    m_miscAIMPACCheck->setEnabled( true );
    m_miscRPACKCheck->setEnabled( true );
  }

  QRadioButton *forceButton =
    qobject_cast<QRadioButton *>( m_miscForceButtons->button( force ) );

  if ( forceButton )
    forceButton->setChecked( true );

  m_miscWaterCheck->setChecked( m_inputData->Basis->GetWaterSolvate() );

  blockChildrenSignals( m_miscLayout, false );
}

void GamessInputBuilder::updateSCFWidgets()
{
  blockChildrenSignals( m_scfLayout, true );

  bool direct = m_inputData->SCF->GetDirectSCF();
  int scf = m_inputData->Control->GetSCFType();

  m_scfDirectCheck->setChecked( direct );

  m_scfComputeCheck->setChecked( m_inputData->SCF->GetFockDiff() );
  m_scfComputeCheck->setEnabled( direct && 3>=scf );

  m_scfGenerateCheck->setChecked( m_inputData->SCF->GetUHFNO() );
  m_scfGenerateCheck->setEnabled( scf == GAMESS_UHF );

  blockChildrenSignals( m_scfLayout, false );
}

void GamessInputBuilder::updateDFTWidgets()
{
  blockChildrenSignals( m_scfLayout, true );

  blockChildrenSignals( m_scfLayout, false );
}

void GamessInputBuilder::updateMP2Widgets()
{
  blockChildrenSignals( m_mp2Layout, true );

  int electrons = m_inputData->MP2->GetNumCoreElectrons();
  if(electrons > -1)
  {
    m_mp2ElectronsLine->setText(QString::number(electrons));
  } else {
    m_mp2ElectronsLine->setText(tr(""));
  }

  long mem = m_inputData->MP2->GetMemory();
  QString memory;
  if( mem == 0 ) {
    memory = tr("All");
  } else {
    memory = QString::number(mem);
  }
  m_mp2MemoryLine->setText(memory);

  double cutoff = m_inputData->MP2->GetIntCutoff();
  if(cutoff == 0.0) cutoff = 1.0e-09;
  m_mp2IntegralLine->setText(QString::number(cutoff));

  m_mp2LocalizedCheck->setEnabled(m_inputData->Control->GetSCFType() <= 1);
  bool lmomp2 = m_inputData->MP2->GetLMOMP2();
  m_mp2LocalizedCheck->setChecked(lmomp2);

  m_mp2ComputeCheck->setEnabled(m_inputData->Control->GetRunType());
  m_mp2ComputeCheck->setChecked(m_inputData->MP2->GetMP2Prop());

  m_mp2TransformationGroup->setEnabled(!lmomp2);
  if(m_inputData->MP2->GetMethod() == 3)
    m_mp2TwoRadio->setChecked(true);
  else
    m_mp2SegmentedRadio->setChecked(true);

  int ao = m_inputData->MP2->GetAOIntMethod();
  if( ao == 2 || ao == 0)
    m_mp2DistributedRadio->setChecked(true);
  else
    m_mp2DuplicatedRadio->setChecked(true);

  blockChildrenSignals( m_mp2Layout, false );
}

void GamessInputBuilder::updateHessianWidgets()
{
  blockChildrenSignals( m_hessianLayout, true );

  int scfType = m_inputData->Control->GetSCFType();

  bool analyticEnable = (( scfType == 1 || scfType == 3 || scfType == 4 || scfType == 0) && m_inputData->Control->GetMPLevel() == 0);
  bool analyticSelect = analyticEnable && m_inputData->Hessian->GetAnalyticMethod();

  m_hessianAnalyticRadio->setChecked(analyticSelect);
  m_hessianNumericRadio->setChecked(!analyticSelect);
  m_hessianAnalyticRadio->setEnabled(analyticEnable);

  m_hessianDoubleCheck->setEnabled(!analyticSelect);
  m_hessianDoubleCheck->setChecked(m_inputData->Hessian->GetDoubleDiff());

  int numZVar = m_inputData->Data->GetNumZVar();
  m_hessianPurifyCheck->setEnabled(numZVar > 0);
  m_hessianPurifyCheck->setChecked(m_inputData->Hessian->GetPurify());

  m_hessianPrintCheck->setEnabled(numZVar > 0);
  m_hessianPrintCheck->setChecked(m_inputData->Hessian->GetPrintFC());

  bool vibrational = m_inputData->Hessian->GetVibAnalysis();
  m_hessianVibrationalCheck->setChecked(vibrational);

  m_hessianScaleLine->setEnabled(vibrational);
  m_hessianScaleLine->setText(QString::number(m_inputData->Hessian->GetFreqScale()));

  m_hessianDisplacementLine->setEnabled(!analyticSelect);
  m_hessianDisplacementLine->setText(QString::number(m_inputData->Hessian->GetDisplacementSize()));

  blockChildrenSignals( m_hessianLayout, false );
}

void GamessInputBuilder::updateStatPointWidgets()
{
  blockChildrenSignals( m_statPointLayout, true );

  int runType = m_inputData->Control->GetRunType();

  m_statPointStepsLine->setText(QString::number(m_inputData->StatPt->GetMaxSteps()));

  m_statPointConvergenceLine->setText(QString::number(m_inputData->StatPt->GetOptConvergance()));

  int method = m_inputData->StatPt->GetMethod();
  m_statPointOptimizationCombo->setCurrentIndex(method - 1);

  m_statPointInitialLine->setEnabled(method != 1);

  float initial = m_inputData->StatPt->GetInitRadius();
  if(initial == 0.0)
  {
    if(method == 5) initial = 0.1;
    else if (runType == 6) initial = 0.2;
    else initial = 0.3;
  }
  m_statPointInitialLine->setText(QString::number(initial));

  float min = m_inputData->StatPt->GetMinRadius();
  m_statPointMinLine->setText(QString::number(min));

  float max = m_inputData->StatPt->GetMaxRadius();
  m_statPointMaxLine->setText(QString::number(max));

  m_statPointRecalculateLine->setText(QString::number(m_inputData->StatPt->GetHessRecalcInterval()));

  int hessianType = m_inputData->StatPt->GetHessMethod();
  if(!hessianType)
  {
    if(runType == 4) hessianType = 1;
    else hessianType = 2;
  }

  m_statPointHessianButtons->button(hessianType-1)->setChecked(true);

  m_statPointUpdateCheck->setChecked(m_inputData->StatPt->GetRadiusUpdate());

  bool stationary = m_inputData->StatPt->GetStatPoint();
  m_statPointStationaryCheck->setChecked(stationary);

  m_statPointPrintCheck->setChecked(m_inputData->StatPt->AlwaysPrintOrbs());

  m_statPointJumpLine->setEnabled(stationary);
  m_statPointJumpLine->setText(QString::number(m_inputData->StatPt->GetModeFollow()));

  blockChildrenSignals( m_statPointLayout, false );
}

// Basis Tab
void GamessInputBuilder::createBasis()
{
  m_basisLayout = new QGridLayout;

  QHBoxLayout *layout;
  int row = 0;

  m_basisLayout = new QGridLayout;
  // Basis Set
  layout = new QHBoxLayout();
  layout->addStretch( 40 );
  m_basisSetLabel = new QLabel( tr( "Basis Set" ) );
  m_basisSetLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  layout->addWidget( m_basisSetLabel );

  m_basisSetCombo = new QComboBox();
  m_basisSetCombo->addItem( tr( "MINI" ) );
  m_basisSetCombo->addItem( tr( "MIDI" ) );
  m_basisSetCombo->addItem( tr( "STO-2G" ) );
  m_basisSetCombo->addItem( tr( "STO-3G" ) );
  m_basisSetCombo->addItem( tr( "STO-4G" ) );
  m_basisSetCombo->addItem( tr( "STO-5G" ) );
  m_basisSetCombo->addItem( tr( "STO-6G" ) );
  m_basisSetCombo->addItem( tr( "3-21G" ) );
  m_basisSetCombo->addItem( tr( "6-21G" ) );
  m_basisSetCombo->addItem( tr( "4-31G" ) );
  m_basisSetCombo->addItem( tr( "5-31G" ) );
  m_basisSetCombo->addItem( tr( "6-31G" ) );
  m_basisSetCombo->addItem( tr( "6-311G" ) );
  m_basisSetCombo->addItem( tr( "Double Zeta Valance" ) );
  m_basisSetCombo->addItem( tr( "Dunning/Hay DZ" ) );
  m_basisSetCombo->addItem( tr( "Binning/Curtiss DZ" ) );
  m_basisSetCombo->addItem( tr( "Triple Zeta Valence" ) );
  m_basisSetCombo->addItem( tr( "McLean/Chandler" ) );
  m_basisSetCombo->addItem( tr( "SBKJC Valance" ) );
  m_basisSetCombo->addItem( tr( "Hay/Wadt Valance" ) );
  m_basisSetCombo->addItem( tr( "MNDO" ) );
  m_basisSetCombo->addItem( tr( "AM1" ) );
  m_basisSetCombo->addItem( tr( "PM3" ) );
  connect( m_basisSetCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasisSetIndex( int ) ) );
  connect( m_basisSetCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basisSetCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );

  layout->addWidget( m_basisSetCombo );
  layout->addStretch( 40 );

  m_basisLayout->addLayout( layout,row,0 );

  // ECP Type
  layout = new QHBoxLayout();
  m_basisECPLabel = new QLabel( tr( "ECP Type" ) );
  m_basisECPLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  layout->addWidget( m_basisECPLabel );

  m_basisECPCombo = new QComboBox();
  m_basisECPCombo->addItem( tr( "None" ) );
  m_basisECPCombo->addItem( tr( "Read" ) );
  m_basisECPCombo->addItem( tr( "SBKJC" ) );
  m_basisECPCombo->addItem( tr( "Hay-Wadt" ) );
  connect( m_basisECPCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasisECP( int ) ) );
  connect( m_basisECPCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basisECPCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );

  layout->addWidget( m_basisECPCombo );
  layout->addStretch( 40 );

  m_basisLayout->addLayout( layout,row,1 );

  // D Heavy Atom Polarization
  row++;
  m_basisDLabel = new QLabel( tr( "#D Heavy Atom Polarization Functions" ) );
  m_basisDLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_basisLayout->addWidget( m_basisDLabel, row, 0 );

  m_basisDCombo = new QComboBox();
  m_basisDCombo->addItem( tr( "0" ) );
  m_basisDCombo->addItem( tr( "1" ) );
  m_basisDCombo->addItem( tr( "2" ) );
  m_basisDCombo->addItem( tr( "3" ) );
  connect( m_basisDCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasisD( int ) ) );
  connect( m_basisDCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basisDCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );

  m_basisLayout->addWidget( m_basisDCombo, row, 1 );

  // F Heavy Atom Polarization
  row++;
  m_basisFLabel = new QLabel( tr( "#F Heavy Atom Polarization Functions" ) );
  m_basisFLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_basisLayout->addWidget( m_basisFLabel, row, 0 );

  m_basisFCombo = new QComboBox();
  m_basisFCombo->addItem( tr( "0" ) );
  m_basisFCombo->addItem( tr( "1" ) );
  m_basisFCombo->addItem( tr( "2" ) );
  m_basisFCombo->addItem( tr( "3" ) );
  connect( m_basisFCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasisF( int ) ) );
  connect( m_basisFCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basisFCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );

  m_basisLayout->addWidget( m_basisFCombo, row, 1 );

  // Light Atom Polarization
  row++;
  m_basisLightLabel = new QLabel( tr( "#light Atom Polarization Functions" ) );
  m_basisLightLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_basisLayout->addWidget( m_basisLightLabel, row, 0 );

  m_basisLightCombo = new QComboBox();
  m_basisLightCombo->addItem( tr( "0" ) );
  m_basisLightCombo->addItem( tr( "1" ) );
  m_basisLightCombo->addItem( tr( "2" ) );
  m_basisLightCombo->addItem( tr( "3" ) );
  connect( m_basisLightCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasisLight( int ) ) );
  connect( m_basisLightCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basisLightCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );

  m_basisLayout->addWidget( m_basisLightCombo, row, 1 );

  // Polar
  row++;
  m_basisPolarLabel = new QLabel( tr( "Polar" ) );
  m_basisPolarLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_basisLayout->addWidget( m_basisPolarLabel, row, 0 );

  m_basisPolarCombo = new QComboBox();
  m_basisPolarCombo->addItem( tr( "Default" ) );
  m_basisPolarCombo->addItem( tr( "Pople" ) );
  m_basisPolarCombo->addItem( tr( "Pople N311" ) );
  m_basisPolarCombo->addItem( tr( "Dunning" ) );
  m_basisPolarCombo->addItem( tr( "Huzinaga" ) );
  m_basisPolarCombo->addItem( tr( "Hondo7" ) );
  connect( m_basisPolarCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setBasisPolar( int ) ) );
  connect( m_basisPolarCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basisPolarCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  m_basisLayout->addWidget( m_basisPolarCombo, row, 1 );


  // Diffuse L-Shell on Heavy Atoms
  row++;
  m_basisDiffuseLCheck = new QCheckBox( tr( "Diffuse L-Shell on Heavy Atoms" ) );
  connect( m_basisDiffuseLCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setBasisDiffuseL( bool ) ) );
  connect( m_basisDiffuseLCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basisDiffuseLCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  m_basisLayout->addWidget( m_basisDiffuseLCheck, row, 0, Qt::AlignCenter );

  // Diffuse S-Shell on Heavy Atoms
  m_basisDiffuseSCheck = new QCheckBox( tr( "Diffuse S-Shell on Heavy Atoms" ) );
  connect( m_basisDiffuseSCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setBasisDiffuseS( bool ) ) );
  connect( m_basisDiffuseSCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_basisDiffuseSCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  m_basisLayout->addWidget( m_basisDiffuseSCheck, row, 1, Qt::AlignLeft );

}

void GamessInputBuilder::createControl()
{
  m_controlLayout = new QGridLayout;

  // general use variable for newly created layouts
  QHBoxLayout *layout;
  QFrame *divider;
  int row = 0;

  // Run Type
  layout = new QHBoxLayout();
  m_controlRunLabel = new QLabel( tr( "Run Type:" ) );
  m_controlRunLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_controlLayout->addWidget( m_controlRunLabel,row,0,1,2,Qt::AlignRight );
  m_controlRunCombo = new QComboBox();
  m_controlRunCombo->addItem( tr( "Energy" ) );
  m_controlRunCombo->addItem( tr( "Gradient" ) );
  m_controlRunCombo->addItem( tr( "Hessian" ) );
  m_controlRunCombo->addItem( tr( "Optimization" ) );
  m_controlRunCombo->addItem( tr( "Trudge" ) );
  m_controlRunCombo->addItem( tr( "Saddle Point" ) );
  m_controlRunCombo->addItem( tr( "IRC" ) );
  m_controlRunCombo->addItem( tr( "Gradient Extremal" ) );
  m_controlRunCombo->addItem( tr( "DRC" ) );
  m_controlRunCombo->addItem( tr( "Energy Surface" ) );
  m_controlRunCombo->addItem( tr( "Properties" ) );
  m_controlRunCombo->addItem( tr( "Morokuma" ) );
  m_controlRunCombo->addItem( tr( "Radiative Transition mom." ) );
  m_controlRunCombo->addItem( tr( "Spin Orbit" ) );
  m_controlRunCombo->addItem( tr( "Finite Electric Field" ) );
  m_controlRunCombo->addItem( tr( "TDHF" ) );
  m_controlRunCombo->addItem( tr( "Global Optimization" ) );
  m_controlRunCombo->addItem( tr( "VSCF" ) );
  m_controlRunCombo->addItem( tr( "FMO Optimization" ) );
  m_controlRunCombo->addItem( tr( "Raman Intensities" ) );
  m_controlRunCombo->addItem( tr( "NMR" ) );
  m_controlRunCombo->addItem( tr( "Make EFP" ) );
  connect( m_controlRunCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setControlRun( int ) ) );
  connect( m_controlRunCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlRunCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_controlRunCombo );

  // SCF Type
  layout->addSpacing( 20 );
  m_controlSCFLabel = new QLabel( tr( "SCF Type:" ) );
  m_controlSCFLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  layout->addWidget( m_controlSCFLabel );
  m_controlLayout->addLayout( layout, row, 2, 1, 2 );
  m_controlSCFCombo = new QComboBox();
  m_controlSCFCombo->addItem( tr( "RHF" ) );
  m_controlSCFCombo->addItem( tr( "UHF" ) );
  m_controlSCFCombo->addItem( tr( "ROHF" ) );
  m_controlSCFCombo->addItem( tr( "GVB" ) );
  m_controlSCFCombo->addItem( tr( "MCSCF" ) );
  m_controlSCFCombo->addItem( tr( "None (CI)" ) );
  connect( m_controlSCFCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setControlSCF( int ) ) );
  connect( m_controlSCFCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlSCFCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  m_controlLayout->addWidget( m_controlSCFCombo, row, 4 );

  // Localization Method
  row++;
  m_controlLocalizationLabel = new QLabel( tr( "Localization Method:" ) );
  m_controlLocalizationLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_controlLayout->addWidget( m_controlLocalizationLabel, row, 0, 1, 2 );
  m_controlLocalizationCombo = new QComboBox();
  m_controlLocalizationCombo->addItem( tr( "None" ) );
  m_controlLocalizationCombo->addItem( tr( "Foster-Boys" ) );
  m_controlLocalizationCombo->addItem( tr( "Edmiston-Ruedenberg" ) );
  m_controlLocalizationCombo->addItem( tr( "Pipek-Mezey" ) );
  connect( m_controlLocalizationCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setControlLocalization( int ) ) );
  connect( m_controlLocalizationCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlLocalizationCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  m_controlLayout->addWidget( m_controlLocalizationCombo, row, 2, 1, 2 );

  // Divider
  row++;
  divider = new QFrame();
  divider->setFrameShape( QFrame::HLine );
  divider->setFrameShadow( QFrame::Sunken );
  divider->setLineWidth( 0 );
  divider->setMidLineWidth( 0 );
  m_controlLayout->addWidget( divider, row, 1, 1, 5 );

  row++;
  // Exec Type
  layout = new QHBoxLayout();
  m_controlExecLabel = new QLabel( tr( "Exec Type:" ) );
  m_controlExecLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_controlLayout->addWidget( m_controlExecLabel, row, 0, 1, 2 );
  m_controlExecCombo = new QComboBox();
  m_controlExecCombo->addItem( tr( "Normal Run" ) );
  m_controlExecCombo->addItem( tr( "Check" ) );
  m_controlExecCombo->addItem( tr( "Debug" ) );
  m_controlExecCombo->addItem( tr( "Other..." ) );
  connect( m_controlExecCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setControlExec( int ) ) );
  connect( m_controlExecCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlExecCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_controlExecCombo );

  // Molecule Charge
  layout->addStretch( 20 );
  m_controlChargeLabel = new QLabel( tr( "Molecule Charge:" ) );
  m_controlChargeLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  layout->addWidget( m_controlChargeLabel );
  m_controlLayout->addLayout( layout, row, 2, 1, 2 );
  m_controlChargeLine = new QLineEdit();
  m_controlChargeLine->setText( tr( "0" ) );
  m_controlChargeLine->setToolTip( tr( "$CONTRL:ICHARG - Enter an integer value for the molecular charge." ) );
  connect( m_controlChargeLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setControlCharge( QString ) ) );
  connect( m_controlChargeLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlChargeLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( advancedChanged() ) );
  m_controlLayout->addWidget( m_controlChargeLine, row, 4 );

  row++;
  layout = new QHBoxLayout();
  // Maximum SCF Iterations
  m_controlMaxSCFLabel = new QLabel( tr( "Max SCF Iterations:" ) );
  m_controlMaxSCFLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_controlLayout->addWidget( m_controlMaxSCFLabel, row, 0, 1, 2 );
  m_controlMaxSCFLine = new QLineEdit();
  m_controlMaxSCFLine->setText( tr( "30" ) );
  m_controlMaxSCFLine->setToolTip( tr( "$CONTRL:MAXIT - Enter the maximum number of SCF iterations. If the wavefunction is not converged at this point the run will be aborted." ) );
  connect( m_controlMaxSCFLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setControlMaxSCF( QString ) ) );
  connect( m_controlMaxSCFLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlMaxSCFLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_controlMaxSCFLine );

  // Multiplicity
  layout->addStretch( 20 );
  m_controlMultiplicityLabel = new QLabel( tr( "Multiplicity:" ) );
  m_controlMultiplicityLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  layout->addWidget( m_controlMultiplicityLabel );
  m_controlLayout->addLayout( layout, row, 2, 1, 2 );
  m_controlMultiplicityLine = new QLineEdit();
  m_controlMultiplicityLine->setText( tr( "0" ) );
  m_controlMultiplicityLine->setToolTip( tr( "$CONTRL:MULT - Enter an integer value for the spin state." ) );
  connect( m_controlMultiplicityLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setControlMultiplicity( QString ) ) );
  connect( m_controlMultiplicityLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlMultiplicityLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( advancedChanged() ) );
  m_controlLayout->addWidget( m_controlMultiplicityLine, row, 4 );

  // Divider
  row++;
  divider = new QFrame();
  divider->setFrameShape( QFrame::HLine );
  divider->setFrameShadow( QFrame::Sunken );
  divider->setLineWidth( 0 );
  divider->setMidLineWidth( 0 );
  m_controlLayout->addWidget( divider, row, 0, 1, 5 );

  row++;
  // Use MP2
  m_controlMP2Check = new QCheckBox( tr( "Use MP2" ) );
  m_controlMP2Check->setToolTip( tr( "$CONTRL:MPLEVL - Click to use 2nd order Moller-Plesset perturbation theory. Implemented for RHF, UHF, ROHF and MCSCF energies and RHF, UHF, and ROHF gradients." ) );
  connect( m_controlMP2Check, SIGNAL( toggled( bool ) ),
           this, SLOT( setControlMP2( bool ) ) );
  connect( m_controlMP2Check, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlMP2Check, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  m_controlLayout->addWidget( m_controlMP2Check, row, 1 );

  // CI
  QGridLayout *glayout = new QGridLayout();
  m_controlCILabel = new QLabel( tr( "CI:" ) );
  m_controlCILabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  glayout->addWidget( m_controlCILabel, 0, 0 );
  m_controlCICombo = new QComboBox();
  m_controlCICombo->addItem( tr( "None" ) );
  m_controlCICombo->addItem( tr( "GUGA" ) );
  m_controlCICombo->addItem( tr( "Ames Lab. Determinant" ) );
  m_controlCICombo->addItem( tr( "Occupation Restricted Multiple Active Space" ) );
  m_controlCICombo->addItem( tr( "CI Singles" ) );
  m_controlCICombo->addItem( tr( "Full Second Order CI" ) );
  m_controlCICombo->addItem( tr( "General CI" ) );
  m_controlCICombo->setToolTip( tr( "$CONTRL:CITYP Choose the type of CI to perform on top of the base wavefunction or on the supplied $VEC group for SCFTYP=NONE." ) );
  connect( m_controlCICombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setControlCI( int ) ) );
  connect( m_controlCICombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlCICombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  glayout->addWidget( m_controlCICombo, 0, 1 );

  // CC
  m_controlCCLabel = new QLabel( tr( "CC:" ) );
  m_controlCCLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  glayout->addWidget( m_controlCCLabel, 1, 0 );
  m_controlCCCombo = new QComboBox();
  m_controlCCCombo->addItem( tr( "None" ) );
  m_controlCCCombo->addItem( tr( "LCCD: linearized CC" ) );
  m_controlCCCombo->addItem( tr( "CCD: CC with doubles" ) );
  m_controlCCCombo->addItem( tr( "CCSD: CC with singles and doubles" ) );
  m_controlCCCombo->addItem( tr( "CCSD(T)" ) );
  m_controlCCCombo->addItem( tr( "R-CC" ) );
  m_controlCCCombo->addItem( tr( "CR-CC" ) );
  m_controlCCCombo->addItem( tr( "EOM-CCSD" ) );
  m_controlCCCombo->addItem( tr( "CR-EOM" ) );
  connect( m_controlCCCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setControlCC( int ) ) );
  connect( m_controlCCCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlCCCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  glayout->addWidget( m_controlCCCombo, 1, 1 );
  m_controlLayout->addLayout( glayout, row, 2,2,3 );

  row++;
  // Use DFT
  m_controlDFTCheck = new QCheckBox( tr( "Use DFT" ) );
  connect( m_controlDFTCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setControlDFT( bool ) ) );
  connect( m_controlDFTCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_controlDFTCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  m_controlLayout->addWidget( m_controlDFTCheck, row, 1 );

}

void GamessInputBuilder::createData()
{
  m_dataLayout = new QGridLayout;

  QHBoxLayout *layout;
  QFrame *divider;
  int row = 0;

  m_dataLayout->setColumnStretch( 0, 50 );
  m_dataLayout->setColumnStretch( 1, 50 );

  // Title
  m_dataTitleLabel = new QLabel( tr( "Title:" ) );
  m_dataLayout->addWidget( m_dataTitleLabel, row, 0 );
  m_dataTitleLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_dataTitleLine = new QLineEdit;
  m_dataTitleLine->setText( tr( "Title" ) );
  m_dataTitleLine->setToolTip( tr( "$DATA - You may enter a one line title which may help you identify this input deck in the future." ) );
  connect( m_dataTitleLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setDataTitle( QString ) ) );
  connect( m_dataTitleLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_dataTitleLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( advancedChanged() ) );
  layout = new QHBoxLayout;
  layout->addWidget( m_dataTitleLine );
  layout->addStretch( 40 );
  m_dataLayout->addLayout( layout, row, 1 );

  row++;
  // Coordinate Type
  m_dataCoordinateLabel = new QLabel( tr( "Coordinate Type:" ) );
  m_dataLayout->addWidget( m_dataCoordinateLabel, row, 0 );
  m_dataCoordinateLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_dataCoordinateCombo = new QComboBox;
  m_dataCoordinateCombo->addItem( tr( "Unique cartesian Coords." ) );
  m_dataCoordinateCombo->addItem( tr( "Hilderbrant internals" ) );
  m_dataCoordinateCombo->addItem( tr( "Cartesian coordinates" ) );
  m_dataCoordinateCombo->addItem( tr( "Z-Matrix" ) );
  m_dataCoordinateCombo->addItem( tr( "MOPAC Z-Matrix" ) );
  connect( m_dataCoordinateCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setDataCoordinate( int ) ) );
  connect( m_dataCoordinateCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_dataCoordinateCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout = new QHBoxLayout;
  layout->addWidget( m_dataCoordinateCombo );
  layout->addStretch( 40 );
  m_dataLayout->addLayout( layout, row, 1 );

  row++;
  // Units
  m_dataUnitsLabel = new QLabel( tr( "Units:" ) );
  m_dataLayout->addWidget( m_dataUnitsLabel, row, 0 );
  m_dataUnitsLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_dataUnitsCombo = new QComboBox;
  m_dataUnitsCombo->addItem( tr( "Angstroms" ) );
  m_dataUnitsCombo->addItem( tr( "Bohr" ) );
  connect( m_dataUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setDataUnits( int ) ) );
  connect( m_dataUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_dataUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout = new QHBoxLayout;
  layout->addWidget( m_dataUnitsCombo );
  layout->addStretch( 40 );
  m_dataLayout->addLayout( layout, row, 1 );

  row++;
  // Number of Z-Matrix Variables
  m_dataZMatrixLabel = new QLabel( tr( "# if Z-Matrix Variables:" ) );
  m_dataLayout->addWidget( m_dataZMatrixLabel, row, 0 );
  m_dataZMatrixLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_dataZMatrixLine = new QLineEdit;
  m_dataZMatrixLine->setText( tr( "0" ) );
  m_dataZMatrixLine->setToolTip( tr( "$CONTRL:NZVAR - Enter an integer number representing the number of internal coordinates for your molecule. Normally this will be 3N-6 (3N-5 for linear molecules) where N is the number of atoms. A value of 0 selects cartesian coordinates. If set and a set of internal coordinates are defined a $ZMAT group will be punched out." ) );
  connect( m_dataZMatrixLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setDataZMatrix( QString ) ) );
  connect( m_dataZMatrixLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_dataZMatrixLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( advancedChanged() ) );
  layout = new QHBoxLayout;
  layout->addWidget( m_dataZMatrixLine );
  layout->addStretch( 40 );
  m_dataLayout->addLayout( layout, row, 1 );

  row++;
  // Point Group
  m_dataPointLabel = new QLabel( tr( "Point Group:" ) );
  m_dataLayout->addWidget( m_dataPointLabel, row, 0 );
  m_dataPointLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_dataPointCombo = new QComboBox;
  m_dataPointCombo->addItem( tr( "C1" ) );
  m_dataPointCombo->addItem( tr( "CS" ) );
  m_dataPointCombo->addItem( tr( "CI" ) );
  m_dataPointCombo->addItem( tr( "CnH" ) );
  m_dataPointCombo->addItem( tr( "CnV" ) );
  m_dataPointCombo->addItem( tr( "Cn" ) );
  m_dataPointCombo->addItem( tr( "S2n" ) );
  m_dataPointCombo->addItem( tr( "DnD" ) );
  m_dataPointCombo->addItem( tr( "DnH" ) );
  m_dataPointCombo->addItem( tr( "Dn" ) );
  m_dataPointCombo->addItem( tr( "TD" ) );
  m_dataPointCombo->addItem( tr( "TH" ) );
  m_dataPointCombo->addItem( tr( "T" ) );
  m_dataPointCombo->addItem( tr( "OH" ) );
  m_dataPointCombo->addItem( tr( "O" ) );
  connect( m_dataPointCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setDataPoint( int ) ) );
  connect( m_dataPointCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_dataPointCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout = new QHBoxLayout;
  layout->addWidget( m_dataPointCombo );
  layout->addStretch( 40 );
  m_dataLayout->addLayout( layout, row, 1 );

  row++;
  // Order of Principle Axis
  m_dataOrderLabel = new QLabel( tr( "Order of Principle Axis:" ) );
  m_dataLayout->addWidget( m_dataOrderLabel, row, 0 );
  m_dataOrderLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_dataOrderCombo = new QComboBox;
  m_dataOrderCombo->addItem( tr( "2" ) );
  m_dataOrderCombo->addItem( tr( "3" ) );
  m_dataOrderCombo->addItem( tr( "4" ) );
  m_dataOrderCombo->setToolTip( tr( "Replaces the 'n' above." ) );
  connect( m_dataOrderCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setDataOrder( int ) ) );
  connect( m_dataOrderCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_dataOrderCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout = new QHBoxLayout;
  layout->addWidget( m_dataOrderCombo );
  layout->addStretch( 40 );
  m_dataLayout->addLayout( layout, row, 1 );

  // Divider
  row++;
  divider = new QFrame();
  divider->setFrameShape( QFrame::HLine );
  divider->setFrameShadow( QFrame::Sunken );
  divider->setLineWidth( 0 );
  divider->setMidLineWidth( 0 );
  m_dataLayout->addWidget( divider, row, 0, 1, 1 );


  row++;
  // Use Symmetry During Calculation
  m_dataSymmetryCheck = new QCheckBox( tr( "Use Symmetry During Calculation" ) );
  m_dataSymmetryCheck->setToolTip( tr( "$CONTRL:NOSYM - When checked symmetry will be used as much as possible in the caluclation of integrals, gradients, etc. (This is the normal setting)" ) );
  connect( m_dataSymmetryCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setDataSymmetry( bool ) ) );
  connect( m_dataSymmetryCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_dataSymmetryCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  m_dataLayout->addWidget( m_dataSymmetryCheck, row, 0, 1, 2, Qt::AlignCenter );

}

void GamessInputBuilder::createSystem()
{
  m_systemLayout = new QGridLayout;

  QHBoxLayout *layout;
  QSpacerItem *spacer;
  QFrame *divider;
  int row = 0;

  spacer = new QSpacerItem( 0,0 );

  m_systemLayout->setColumnMinimumWidth( 2, 130 );
  m_systemLayout->setColumnStretch( 0, 1 );
  m_systemLayout->setColumnStretch( 4, 1 );

  // Time Limit
  m_systemTimeLabel = new QLabel( tr( "Time Limit:" ) );
  m_systemTimeLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_systemLayout->addWidget( m_systemTimeLabel, row, 0, 1, 1 );
  m_systemTimeLine = new QLineEdit();
  m_systemTimeLine->setToolTip( tr( "$SYSTEM:TIMLIM - Enter a value for the time limit. When the time limit is reached GAMESS will stop the run. The number entered here will have the units given at the right." ) );
  connect( m_systemTimeLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setSystemTime( QString ) ) );
  connect( m_systemTimeLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_systemTimeLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( advancedChanged() ) );
  m_systemLayout->addWidget( m_systemTimeLine, row, 1 );
  layout = new QHBoxLayout;
  m_systemTimeUnitsCombo = new QComboBox;
  m_systemTimeUnitsCombo->addItem( tr( "Seconds" ) );
  m_systemTimeUnitsCombo->addItem( tr( "Minutes" ) );
  m_systemTimeUnitsCombo->addItem( tr( "Hours" ) );
  m_systemTimeUnitsCombo->addItem( tr( "Days" ) );
  m_systemTimeUnitsCombo->addItem( tr( "Weeks" ) );
  m_systemTimeUnitsCombo->addItem( tr( "Years" ) );
  m_systemTimeUnitsCombo->addItem( tr( "Millenia" ) );
  connect( m_systemTimeUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setSystemTimeUnits( int ) ) );
  connect( m_systemTimeUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_systemTimeUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_systemTimeUnitsCombo );
  layout->addStretch( 40 );
  m_systemLayout->addLayout( layout, row, 2, 1, 1 );

  row++;
  // Memory
  m_systemMemoryLabel = new QLabel( tr( "Memory:" ) );
  m_systemMemoryLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_systemLayout->addWidget( m_systemMemoryLabel, row, 0, 1, 1 );
  m_systemMemoryLine = new QLineEdit();
  m_systemMemoryLine->setText( tr( "1000000" ) );
  m_systemMemoryLine->setToolTip( tr( "$SYSTEM:MEMORY - Enter the amount of memory (in the units at the right) that GAMESS will request for its dynamic memory pool. You should not normally request more memory than the RAM size." ) );
  connect( m_systemMemoryLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setSystemMemory( QString ) ) );
  connect( m_systemMemoryLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_systemMemoryLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( advancedChanged() ) );
  m_systemLayout->addWidget( m_systemMemoryLine, row, 1 );
  layout = new QHBoxLayout;
  m_systemMemoryUnitsCombo = new QComboBox;
  m_systemMemoryUnitsCombo->addItem( tr( "Words" ) );
  m_systemMemoryUnitsCombo->addItem( tr( "Bytes" ) );
  m_systemMemoryUnitsCombo->addItem( tr( "MegaWords" ) );
  m_systemMemoryUnitsCombo->addItem( tr( "MegaBytes" ) );
  connect( m_systemMemoryUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setSystemMemoryUnits( int ) ) );
  connect( m_systemMemoryUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_systemMemoryUnitsCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_systemMemoryUnitsCombo );
  layout->addStretch( 40 );
  m_systemLayout->addLayout( layout, row, 2, 1, 1 );

  row++;
  // MemDDI
  m_systemDDILabel = new QLabel( tr( "MemDDI:" ) );
  m_systemDDILabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_systemLayout->addWidget( m_systemDDILabel, row, 0, 1, 1 );
  m_systemDDILine = new QLineEdit();
  m_systemDDILine->setText( tr( "0.00" ) );
  m_systemDDILine->setToolTip( tr( "$SYSTEM:MEMDDI - The size of the pseudo global shared memory pool. This is most often needed for certain parallel computations, but certain sequential algorithms also use it (such as ROMP2). Default is 0." ) );
  connect( m_systemDDILine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setSystemDDI( QString ) ) );
  connect( m_systemDDILine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_systemDDILine, SIGNAL( textChanged( QString ) ),
           this, SLOT( advancedChanged() ) );
  m_systemLayout->addWidget( m_systemDDILine, row, 1 );
  layout = new QHBoxLayout;
  m_systemDDICombo = new QComboBox;
  m_systemDDICombo->addItem( tr( "MegaWords" ) );
  m_systemDDICombo->addItem( tr( "MegaBytes" ) );
  m_systemDDICombo->addItem( tr( "GigaWords" ) );
  m_systemDDICombo->addItem( tr( "GigaBytes" ) );
  connect( m_systemDDICombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setSystemDDIUnits( int ) ) );
  connect( m_systemDDICombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_systemDDICombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_systemDDICombo );
  layout->addStretch( 40 );
  m_systemLayout->addLayout( layout, row, 2, 1, 1 );

  row++;
  // Diagonalization Method
  m_systemDiagnalizationLabel = new QLabel( tr( "Diagonalization Method:" ) );
  m_systemDiagnalizationLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  m_systemLayout->addWidget( m_systemDiagnalizationLabel, row, 0, 1, 2 );
  m_systemDiagnalizationCombo = new QComboBox;
  m_systemDiagnalizationCombo->addItem( tr( "Default" ) );
  m_systemDiagnalizationCombo->addItem( tr( "EVVRSP" ) );
  m_systemDiagnalizationCombo->addItem( tr( "GIVEIS" ) );
  m_systemDiagnalizationCombo->addItem( tr( "JACOBI" ) );
  connect( m_systemDiagnalizationCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setSystemDiagonalization( int ) ) );
  connect( m_systemDiagnalizationCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_systemDiagnalizationCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout = new QHBoxLayout;
  layout->addWidget( m_systemDiagnalizationCombo );
  layout->addStretch( 40 );
  m_systemLayout->addLayout( layout, row, 2 );

  row++;
  // Use External Data Representation for Messages
  m_systemExternalCheck = new QCheckBox( tr( "Use External Data Representation for Messages" ) );
  connect( m_systemExternalCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setSystemExternal( bool ) ) );
  connect( m_systemExternalCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_systemExternalCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  m_systemLayout->addWidget( m_systemExternalCheck, row, 1, 1, 3, Qt::AlignCenter );

  row = 0;
  // Produce "core" file upon abort.
  m_systemCoreCheck  = new QCheckBox( tr( "Produce \"core\" file upon abort" ) );
  connect( m_systemCoreCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setSystemProduce( bool ) ) );
  connect( m_systemCoreCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_systemCoreCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  m_systemLayout->addWidget( m_systemCoreCheck, row, 3, 1, 1 );

  row++;
  // Force Parallel Methods
  m_systemForceCheck = new QCheckBox( tr( "Force Parallel Methods" ) );
  connect( m_systemForceCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setSystemForce( bool ) ) );
  connect( m_systemForceCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_systemForceCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  m_systemLayout->addWidget( m_systemForceCheck, row, 3, 1, 1 );
  row++;
  // Parallel Load Balance Type
  m_systemParallelGroup = new QGroupBox( tr( "Parallel Load Balance Type" ) );
  m_systemParallelGroup->setFlat( true );
  m_systemLoopRadio = new QRadioButton( tr( "Loop" ) );
  connect( m_systemLoopRadio, SIGNAL( toggled( bool ) ), this, SLOT( setSystemLoop( bool ) ) );
  connect( m_systemLoopRadio, SIGNAL( toggled( bool ) ), this, SLOT( updatePreviewText() ) );
  connect( m_systemLoopRadio, SIGNAL( toggled( bool ) ), this, SLOT( advancedChanged() ) );
  m_systemNextRadio = new QRadioButton( tr( "Next Value" ) );
  connect( m_systemNextRadio, SIGNAL( toggled( bool ) ), this, SLOT( setSystemNext( bool ) ) );
  connect( m_systemNextRadio, SIGNAL( toggled( bool ) ), this, SLOT( updatePreviewText() ) );
  connect( m_systemNextRadio, SIGNAL( toggled( bool ) ), this, SLOT( advancedChanged() ) );
  QHBoxLayout *parallelLoadLayout = new QHBoxLayout;
  parallelLoadLayout->addWidget( m_systemLoopRadio );
  parallelLoadLayout->addWidget( m_systemNextRadio );
  parallelLoadLayout->setMargin( 4 );
  m_systemParallelGroup->setLayout( parallelLoadLayout );
  m_systemLayout->addWidget( m_systemParallelGroup, row, 3, 2, 1 );
  row++;
}

void GamessInputBuilder::createMOGuess()
{
  m_moGuessLayout = new QGridLayout;

  QHBoxLayout *layout;
  int row = 0;

  layout = new QHBoxLayout;
  layout->addStretch();
  // Initial Guess
  m_moGuessInitialLabel = new QLabel( tr( "Initial Guess:" ) );
  m_moGuessInitialLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  layout->addWidget( m_moGuessInitialLabel );
  m_moGuessInitialCombo = new QComboBox;
  m_moGuessInitialCombo->addItem( tr( "Huckel" ) );
  m_moGuessInitialCombo->addItem( tr( "HCore" ) );
  m_moGuessInitialCombo->addItem( tr( "MO Read ($VEC)" ) );
  m_moGuessInitialCombo->addItem( tr( "MO Saved (DICTNRY)" ) );
  m_moGuessInitialCombo->addItem( tr( "Skip" ) );
  connect( m_moGuessInitialCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setMOGuessInitial( int ) ) );
  connect( m_moGuessInitialCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_moGuessInitialCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_moGuessInitialCombo );
  layout->addStretch();
  m_moGuessLayout->addLayout( layout, row, 0 );

  row++;
  layout = new QHBoxLayout;
  layout->addStretch();
  // Print Initial Guess
  m_moGuessPrintCheck = new QCheckBox( tr( "Print the Initial Guess" ) );
  connect( m_moGuessPrintCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setMOGuessPrint( bool ) ) );
  connect( m_moGuessPrintCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_moGuessPrintCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_moGuessPrintCheck );

  layout->addStretch();
  // Rotate Alpha and Beta Orbitals
  m_moGuessRotateCheck = new QCheckBox( tr( "Rotate Alpha and Beta Orbitals" ) );
  connect( m_moGuessRotateCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setMOGuessRotate( bool ) ) );
  connect( m_moGuessRotateCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_moGuessRotateCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_moGuessRotateCheck );
  layout->addStretch();
  m_moGuessLayout->addLayout( layout, row, 0, 1, 2 );

}

void GamessInputBuilder::createMisc()
{
  m_miscLayout = new QGridLayout;

  QHBoxLayout *layout;
  int row = 0;

  // GAMESS Interface to Other Codes
  m_miscInterfaceGroup = new QGroupBox( tr( "GAMESS Interface to Other Codes" ) );
  m_miscInterfaceGroup->setFlat( true );
  QHBoxLayout *gamessInterfaceLayout = new QHBoxLayout;
  m_miscMolPltCheck = new QCheckBox( tr( "MolPlt" ) );
  connect( m_miscMolPltCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setMiscMolPlt( bool ) ) );
  connect( m_miscMolPltCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_miscMolPltCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  gamessInterfaceLayout->addWidget( m_miscMolPltCheck );
  m_miscPltOrbCheck = new QCheckBox( tr( "PltOrb" ) );
  connect( m_miscPltOrbCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setMiscPltOrb( bool ) ) );
  connect( m_miscPltOrbCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_miscPltOrbCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  gamessInterfaceLayout->addWidget( m_miscPltOrbCheck );
  m_miscAIMPACCheck = new QCheckBox( tr( "AIMPAC" ) );
  connect( m_miscAIMPACCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setMiscAIMPAC( bool ) ) );
  connect( m_miscAIMPACCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_miscAIMPACCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  gamessInterfaceLayout->addWidget( m_miscAIMPACCheck );
  m_miscRPACKCheck = new QCheckBox( tr( "RPAC" ) );
  connect( m_miscRPACKCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setMiscRPAC( bool ) ) );
  connect( m_miscRPACKCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_miscRPACKCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  gamessInterfaceLayout->addWidget( m_miscRPACKCheck );
  m_miscInterfaceGroup->setLayout( gamessInterfaceLayout );
  m_miscLayout->addWidget( m_miscInterfaceGroup, row, 0 );

  row++;
  // Force a Check Run Type
  m_miscForceGroup = new QGroupBox( tr( "Force a Check Run Type" ) );
  m_miscForceGroup->setFlat( true );
  QHBoxLayout *forceRunTypeHLayout = new QHBoxLayout;
  QVBoxLayout *forceRunTypeVLayout[2];
  forceRunTypeVLayout[0] = new QVBoxLayout;
  forceRunTypeVLayout[1] = new QVBoxLayout;
  m_miscForceButtons = new QButtonGroup;
  connect( m_miscForceButtons, SIGNAL( buttonClicked( int ) ),
           this, SLOT( setMiscForce( int ) ) );
  connect( m_miscForceButtons, SIGNAL( buttonClicked( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_miscForceButtons, SIGNAL( buttonClicked( int ) ),
           this, SLOT( advancedChanged() ) );
  int id = 0;
  m_miscNoneRadio = new QRadioButton( tr( "None" ) );
  m_miscForceButtons->addButton( m_miscNoneRadio, id++ );
  forceRunTypeVLayout[0]->addWidget( m_miscNoneRadio );
  m_miscHondoRadio = new QRadioButton( tr( "Hondo 8.2" ) );
  m_miscForceButtons->addButton( m_miscHondoRadio, id++ );
  forceRunTypeVLayout[0]->addWidget( m_miscHondoRadio );
  m_miscMeldfRadio = new QRadioButton( tr( "MELDF" ) );
  m_miscForceButtons->addButton( m_miscMeldfRadio, id++ );
  forceRunTypeVLayout[0]->addWidget( m_miscMeldfRadio );
  m_miscGamessRadio = new QRadioButton( tr( "GAMESS (UK Version)" ) );
  m_miscForceButtons->addButton( m_miscGamessRadio, id++ );
  forceRunTypeVLayout[1]->addWidget( m_miscGamessRadio );
  m_miscGaussianRadio = new QRadioButton( tr( "Gaussian 9x" ) );
  m_miscForceButtons->addButton( m_miscGaussianRadio, id++ );
  forceRunTypeVLayout[1]->addWidget( m_miscGaussianRadio );
  m_miscAllRadio = new QRadioButton( tr( "All" ) );
  m_miscForceButtons->addButton( m_miscAllRadio, id++ );
  forceRunTypeVLayout[1]->addWidget( m_miscAllRadio );
  forceRunTypeHLayout->addLayout( forceRunTypeVLayout[0] );
  forceRunTypeHLayout->addLayout( forceRunTypeVLayout[1] );
  m_miscForceGroup->setLayout( forceRunTypeHLayout );
  m_miscLayout->addWidget( m_miscForceGroup, row, 0 );

  row++;
  m_miscWaterCheck = new QCheckBox(tr("Solvate with Water"));
  connect( m_miscWaterCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setMiscWater( bool ) ) );
  connect( m_miscWaterCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_miscWaterCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  m_miscLayout->addWidget( m_miscWaterCheck, row, 0, Qt::AlignCenter);
}

void GamessInputBuilder::createSCF()
{
  m_scfLayout = new QGridLayout;

  m_scfLayout->setMargin( 20 );

  QVBoxLayout *layout = new QVBoxLayout;
  m_scfDirectCheck = new QCheckBox( tr( "Direct SCF" ) );
  connect( m_scfDirectCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setSCFDirect( bool ) ) );
  connect( m_scfDirectCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_scfDirectCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_scfDirectCheck );
  m_scfComputeCheck = new QCheckBox( tr( "Comput Only Changed in Fock Matrix" ) );
  connect( m_scfComputeCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setSCFCompute( bool ) ) );
  connect( m_scfComputeCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_scfComputeCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_scfComputeCheck );
  m_scfGenerateCheck = new QCheckBox( tr( "Generate UHF Natural Orbitals" ) );
  connect( m_scfGenerateCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setSCFGenerate( bool ) ) );
  connect( m_scfGenerateCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_scfGenerateCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget( m_scfGenerateCheck );
  layout->addStretch();
  m_scfLayout->addLayout( layout, 0, 0, Qt::AlignCenter );
}

void GamessInputBuilder::createDFT()
{
  m_dftLayout = new QGridLayout;

  QHBoxLayout *layout;
  int row = 0;

  m_dftLayout->setColumnStretch( 0, 50 );
  m_dftLayout->setColumnStretch( 1, 50 );

  // Method
  m_dftMethodLabel = new QLabel(tr("Method:"));
  m_dftMethodLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_dftLayout->addWidget(m_dftMethodLabel, row, 0);
  layout = new QHBoxLayout;
  m_dftMethodCombo = new QComboBox;
  m_dftMethodCombo->addItem(tr("Grid"));
  m_dftMethodCombo->addItem(tr("Grid-Free"));
  connect( m_dftMethodCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setDFTMethod( int ) ) );
  connect( m_dftMethodCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_dftMethodCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget(m_dftMethodCombo);
  layout->addStretch();
  m_dftLayout->addLayout(layout, row, 1);

  row++;
  // DFT Functional
  m_dftFunctionalLabel = new QLabel(tr("DFT Functional:"));
  m_dftFunctionalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_dftLayout->addWidget(m_dftFunctionalLabel, row, 0);
  layout = new QHBoxLayout;
  m_dftFunctionalCombo = new QComboBox;
  m_dftFunctionalCombo->addItem(tr("Slater exchange"));
  m_dftFunctionalCombo->addItem(tr("Becke 1988 exchange"));
  m_dftFunctionalCombo->addItem(tr("VWN: Vosko-Wilk-Nusair (VWN5) correlation"));
  m_dftFunctionalCombo->addItem(tr("LYP: Lee-Yang-Parr correlation"));
  m_dftFunctionalCombo->addItem(tr("SVWN: Slater exhange + VWN correlation"));
  m_dftFunctionalCombo->addItem(tr("BVWN: BECKE exchange + VWN5 correlation"));
  m_dftFunctionalCombo->addItem(tr("BLYP: BECKE exchange + LYP correlation"));
  m_dftFunctionalCombo->addItem(tr("B3LYP"));
  m_dftFunctionalCombo->addItem(tr("Gill 1996 exchange"));
  m_dftFunctionalCombo->addItem(tr("Perdew-Burke-Ernzerhof (PBE) exchange"));
  m_dftFunctionalCombo->addItem(tr("OP: One-parameter Progressive correlation"));
  m_dftFunctionalCombo->addItem(tr("SLYP: SLATER + Lee-Yang-Parr (LYP) correlation"));
  m_dftFunctionalCombo->addItem(tr("SOP: SLATER + OP correlation"));
  m_dftFunctionalCombo->addItem(tr("BOP: BECKE exchange + OP correlation"));
  m_dftFunctionalCombo->addItem(tr("BVWN: GILL exchange + VWN5 correlation"));
  m_dftFunctionalCombo->addItem(tr("GLYP: GILL exchange + LYP correlation"));
  m_dftFunctionalCombo->addItem(tr("GOP: GILL exchange + OP correlation"));
  m_dftFunctionalCombo->addItem(tr("PBEVWN: PBE exchange + VWN6 correlation"));
  m_dftFunctionalCombo->addItem(tr("PBELYP: PBE exchange +LYP correlation"));
  m_dftFunctionalCombo->addItem(tr("PBEOP: PBE exchange + OP correlation"));
  m_dftFunctionalCombo->addItem(tr("BHHLYP: HF and BECKE exchange + LYP correlation"));
  connect( m_dftFunctionalCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setDFTFunctional( int ) ) );
  connect( m_dftFunctionalCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_dftFunctionalCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  layout->addWidget(m_dftFunctionalCombo);
  layout->addStretch();
  m_dftLayout->addLayout(layout, row, 1);

}

void GamessInputBuilder::createMP2()
{
  m_mp2Layout = new QGridLayout;
  QHBoxLayout *layout;

  m_mp2Layout->setColumnStretch( 0, 50 );
  m_mp2Layout->setColumnStretch( 1, 50 );

  int row = 0;
  // # of Core Electrons
  m_mp2ElectronsLabel = new QLabel(tr("# of Core Electrons:"));
  m_mp2ElectronsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_mp2Layout->addWidget(m_mp2ElectronsLabel, row, 0);
  m_mp2ElectronsLine = new QLineEdit;
  connect( m_mp2ElectronsLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setMP2Electrons( QString ) ) );
  connect( m_mp2ElectronsLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_mp2ElectronsLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  layout = new QHBoxLayout;
  layout->addWidget(m_mp2ElectronsLine);
  layout->addStretch();
  m_mp2Layout->addLayout(layout, row, 1);

  row++;
  m_mp2MemoryLabel = new QLabel(tr("Memory:"));
  m_mp2MemoryLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_mp2Layout->addWidget(m_mp2MemoryLabel, row, 0);
  m_mp2MemoryLine = new QLineEdit;
  connect( m_mp2MemoryLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setMP2Memory( QString ) ) );
  connect( m_mp2MemoryLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_mp2MemoryLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  layout = new QHBoxLayout;
  layout->addWidget(m_mp2MemoryLine);
  QLabel *label = new QLabel(tr("words"));
  layout->addWidget(label);
  layout->addStretch();
  m_mp2Layout->addLayout(layout, row, 1);

  row++;
  // Integral Retention Cutoff
  m_mp2IntegralLabel = new QLabel(tr("Integral Retention Cutoff:"));
  m_mp2IntegralLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_mp2Layout->addWidget(m_mp2IntegralLabel, row, 0);
  m_mp2IntegralLine = new QLineEdit;
  connect( m_mp2IntegralLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setMP2Integral( QString ) ) );
  connect( m_mp2IntegralLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_mp2IntegralLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  layout = new QHBoxLayout;
  layout->addWidget(m_mp2IntegralLine);
  layout->addStretch();
  m_mp2Layout->addLayout(layout, row, 1);

  QVBoxLayout *vlayout;
  row++;
  // Use Localized Orbitals
  vlayout = new QVBoxLayout;
  m_mp2LocalizedCheck = new QCheckBox(tr("Use Localized Orbitals"));
  connect( m_mp2LocalizedCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setMP2Localized( bool ) ) );
  connect( m_mp2LocalizedCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_mp2LocalizedCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_mp2LocalizedCheck);
//   m_mp2Layout->addWidget(m_mp2LocalizedCheck, row, 0, Qt::AlignCenter);

  // Compute MP2 Properties
  m_mp2ComputeCheck = new QCheckBox(tr("Compute MP2 Properties"));
  connect( m_mp2ComputeCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setMP2Compute( bool ) ) );
  connect( m_mp2ComputeCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_mp2ComputeCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_mp2ComputeCheck);
  m_mp2Layout->addLayout(vlayout, row, 0, 3, 2, Qt::AlignCenter | Qt::AlignVCenter);
//   m_mp2Layout->addWidget(m_mp2ComputeCheck, row, 1, Qt::AlignCenter);

  row = 0;
  // Transformation Method
  m_mp2TransformationGroup = new QGroupBox(tr("Transformation Method"));
  vlayout = new QVBoxLayout;
  m_mp2TransformationGroup->setLayout(vlayout);
  m_mp2SegmentedRadio = new QRadioButton(tr("Segmented Transformation"));
  connect( m_mp2SegmentedRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( setMP2Segmented( bool ) ) );
  connect( m_mp2SegmentedRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_mp2SegmentedRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_mp2SegmentedRadio);
  m_mp2TwoRadio = new QRadioButton(tr("Two Phase Bin Sort"));
  connect( m_mp2TwoRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( setMP2Two( bool ) ) );
  connect( m_mp2TwoRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_mp2TwoRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_mp2TwoRadio);
  m_mp2Layout->addWidget(m_mp2TransformationGroup, row, 2, 3, 1, Qt::AlignCenter);

  row += 3;
  // AO Integral Storage
  m_mp2StorageGroup = new QGroupBox(tr("AO Integral Storage"));
  vlayout = new QVBoxLayout;
  m_mp2StorageGroup->setLayout(vlayout);
  m_mp2DuplicatedRadio = new QRadioButton(tr("Duplicated on Each Node"));
  connect( m_mp2DuplicatedRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( setMP2Duplicated( bool ) ) );
  connect( m_mp2DuplicatedRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_mp2DuplicatedRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_mp2DuplicatedRadio);
  m_mp2DistributedRadio = new QRadioButton(tr("Distributed Across All Nodes"));
  connect( m_mp2DistributedRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( setMP2Distributed( bool ) ) );
  connect( m_mp2DistributedRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_mp2DistributedRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_mp2DistributedRadio);
  m_mp2Layout->addWidget(m_mp2StorageGroup, row, 2, 3, 1, Qt::AlignCenter);

}

void GamessInputBuilder::createHessian()
{
  m_hessianLayout = new QGridLayout;
  QHBoxLayout *hlayout;
  QVBoxLayout *vlayout;

  m_hessianLayout->setColumnStretch( 0, 1 );
  m_hessianLayout->setColumnMinimumWidth(2, 175);
  m_hessianLayout->setColumnStretch( 4, 1 );

  int row = 0;
  // Method
  m_hessianMethodGroup = new QGroupBox(tr("Method"));
  hlayout = new QHBoxLayout;
  m_hessianMethodGroup->setLayout(hlayout);
  m_hessianAnalyticRadio = new QRadioButton(tr("Analytic"));
  connect( m_hessianAnalyticRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( setHessianAnalytic( bool ) ) );
  connect( m_hessianAnalyticRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_hessianAnalyticRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  hlayout->addWidget(m_hessianAnalyticRadio);
  m_hessianNumericRadio = new QRadioButton(tr("Numeric"));
  connect( m_hessianNumericRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( setHessianNumeric( bool ) ) );
  connect( m_hessianNumericRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_hessianNumericRadio, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  hlayout->addWidget(m_hessianNumericRadio);
  m_hessianLayout->addWidget(m_hessianMethodGroup, row, 1, 1, 1);

  row++;
  // Double Differenced Hessian
  vlayout = new QVBoxLayout;
  m_hessianDoubleCheck = new QCheckBox(tr("Double Differenced Hessian"));
  connect( m_hessianDoubleCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setHessianDouble( bool ) ) );
  connect( m_hessianDoubleCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_hessianDoubleCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_hessianDoubleCheck);
  // Purify Hessian
  m_hessianPurifyCheck = new QCheckBox(tr("Purify Hessian"));
  connect( m_hessianPurifyCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setHessianPurify( bool ) ) );
  connect( m_hessianPurifyCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_hessianPurifyCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_hessianPurifyCheck);
  //Print Internal Force Constants
  m_hessianPrintCheck = new QCheckBox(tr("Print Internal Force Constants"));
  connect( m_hessianPrintCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setHessianPrint( bool ) ) );
  connect( m_hessianPrintCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_hessianPrintCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_hessianPrintCheck);
  // Vibrational Analysis
  m_hessianVibrationalCheck = new QCheckBox(tr("Vibrational Analysis"));
  connect( m_hessianVibrationalCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setHessianVibrational( bool ) ) );
  connect( m_hessianVibrationalCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_hessianVibrationalCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_hessianVibrationalCheck);
  m_hessianLayout->addLayout(vlayout, row, 1, 1, 1 );

  row = 0;
  // Displacement Size
  m_hessianDisplacementLabel = new QLabel(tr("Displacement Size:"));
  m_hessianDisplacementLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_hessianLayout->addWidget(m_hessianDisplacementLabel, row, 2);
  m_hessianDisplacementLine = new QLineEdit;
  hlayout = new QHBoxLayout;
  connect( m_hessianDisplacementLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setHessianDisplacement( QString ) ) );
  connect( m_hessianDisplacementLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_hessianDisplacementLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  hlayout->addWidget(m_hessianDisplacementLine);
  QLabel *label = new QLabel(tr("bohrs"));
  hlayout->addWidget(label);
  hlayout->addStretch();
  m_hessianLayout->addLayout(hlayout, row, 3);

  row++;
  // Frequency Scale Factor
  m_hessianScaleLabel = new QLabel(tr("Frequency Scale Factor:"));
  m_hessianScaleLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_hessianLayout->addWidget(m_hessianScaleLabel, row, 2);
  m_hessianScaleLine = new QLineEdit;
  connect( m_hessianScaleLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setHessianScale( QString ) ) );
  connect( m_hessianScaleLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_hessianScaleLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  hlayout = new QHBoxLayout;
  hlayout->addWidget(m_hessianScaleLine);
  hlayout->addStretch();
  m_hessianLayout->addLayout(hlayout, row, 3);
  
}

void GamessInputBuilder::createStatPoint()
{
  m_statPointLayout = new QGridLayout;
  QVBoxLayout *vlayout;
  QHBoxLayout *hlayout;
  QGridLayout *glayout;

  int row = 0;
  m_statPointStepsLabel = new QLabel(tr("Maximum Steps:"));
  m_statPointStepsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_statPointLayout->addWidget(m_statPointStepsLabel, row, 0);
  m_statPointStepsLine = new QLineEdit;
  connect( m_statPointStepsLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setStatPointSteps( QString ) ) );
  connect( m_statPointStepsLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointStepsLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  m_statPointLayout->addWidget(m_statPointStepsLine, row, 1, 1, 1);

  row++;
  m_statPointConvergenceLabel = new QLabel(tr("Gradient Convergence Criteria:"));
  m_statPointConvergenceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_statPointLayout->addWidget(m_statPointConvergenceLabel, row, 0);
  m_statPointConvergenceLine = new QLineEdit;
  connect( m_statPointConvergenceLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setStatPointConvergence( QString ) ) );
  connect( m_statPointConvergenceLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointConvergenceLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  m_statPointLayout->addWidget(m_statPointConvergenceLine, row, 1, 1, 1);

  row++;
  m_statPointRecalculateLabel = new QLabel(tr("Recalculateulate Hessian Every:"));
  m_statPointRecalculateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_statPointLayout->addWidget(m_statPointRecalculateLabel, row, 0);
  m_statPointRecalculateLine = new QLineEdit;
  connect( m_statPointRecalculateLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setStatPointRecalculate( QString ) ) );
  connect( m_statPointRecalculateLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointRecalculateLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  m_statPointLayout->addWidget(m_statPointRecalculateLine, row, 1, 1, 1);
  
  row++;
  m_statPointOptimizationLabel = new QLabel(tr("Optimization Method:"));
  m_statPointOptimizationLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_statPointLayout->addWidget(m_statPointOptimizationLabel, row, 0);
  m_statPointOptimizationCombo = new QComboBox;
  m_statPointOptimizationCombo->addItem(tr("Newton-Raphson"));
  m_statPointOptimizationCombo->addItem(tr("Rational Function Optimization"));
  m_statPointOptimizationCombo->addItem(tr("Quadratic Approximation"));
  m_statPointOptimizationCombo->addItem(tr("Schlegel (quasi-NR)"));
  m_statPointOptimizationCombo->addItem(tr("Constrained Optimization"));
  connect( m_statPointOptimizationCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setStatPointOptimization( int ) ) );
  connect( m_statPointOptimizationCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointOptimizationCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( advancedChanged() ) );
  hlayout = new QHBoxLayout;
  hlayout->addWidget(m_statPointOptimizationCombo);
  hlayout->addStretch();
  m_statPointLayout->addLayout(hlayout, row, 1, 1, 2);
  
  row++;
  QHBoxLayout *rlayout = new QHBoxLayout;
  rlayout->addStretch();
  m_statPointHessianGroup = new QGroupBox(tr("Initial Hessian"));
  m_statPointHessianGroup->setFlat(true);
  hlayout = new QHBoxLayout;
  m_statPointHessianGroup->setLayout(hlayout);
  m_statPointHessianButtons = new QButtonGroup;
  connect( m_statPointHessianButtons, SIGNAL( buttonClicked( int ) ), 
      this, SLOT( setStatPointHessian( int ) ) );
  connect( m_statPointHessianButtons, SIGNAL( buttonClicked( int ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointHessianButtons, SIGNAL( buttonClicked( int ) ),
           this, SLOT( advancedChanged() ) );
  int index = 0;
  m_statPointGuessRadio = new QRadioButton(tr("Guess (+ definite)"));
  m_statPointHessianButtons->addButton(m_statPointGuessRadio, index++);
  hlayout->addWidget(m_statPointGuessRadio);
  m_statPointReadRadio = new QRadioButton(tr("Read (from $HESS)"));
  m_statPointHessianButtons->addButton(m_statPointReadRadio, index++);
  hlayout->addWidget(m_statPointReadRadio);
  m_statPointCalculateRadio = new QRadioButton(tr("Calculate"));
  m_statPointHessianButtons->addButton(m_statPointCalculateRadio, index++);
  hlayout->addWidget(m_statPointCalculateRadio);
  rlayout->addWidget(m_statPointHessianGroup);
  rlayout->addStretch();
  
  m_statPointJumpLabel = new QLabel(tr("Jump Size:"));
  m_statPointJumpLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  rlayout->addWidget(m_statPointJumpLabel);
  m_statPointJumpLine = new QLineEdit;
  connect( m_statPointJumpLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setStatPointJump( QString ) ) );
  connect( m_statPointJumpLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointJumpLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  rlayout->addWidget(m_statPointJumpLine);
  m_statPointLayout->addLayout(rlayout, row, 0, 1, 4);
  
  row++;
  hlayout = new QHBoxLayout;
  hlayout->addStretch();
  m_statPointPrintCheck = new QCheckBox(tr("Print Orbs at Each Iteration"));
  connect( m_statPointPrintCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setStatPointPrint( bool ) ) );
  connect( m_statPointPrintCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointPrintCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  hlayout->addWidget(m_statPointPrintCheck);

  hlayout->addStretch();
  // temporarily disabled as it doesn't do anything.
//   m_statPointFollowLabel = new QLabel(tr("Follow Mode:"));
//   m_statPointFollowLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
//   hlayout->addWidget(m_statPointFollowLabel);
//   m_statPointFollowLine = new QLineEdit;
//   connect( m_statPointFollowLine, SIGNAL( textChanged( QString ) ),
//            this, SLOT( setStatPointFollow( QString ) ) );
//   connect( m_statPointFollowLine, SIGNAL( textChanged( QString ) ),
//            this, SLOT( updatePreviewText() ) );
//   connect( m_statPointFollowLine, SIGNAL( textChanged( QString ) ),
//            this, SLOT( basicChanged() ) );
//   hlayout->addWidget(m_statPointFollowLine);
  m_statPointLayout->addLayout(hlayout, row, 0, 1, 4, Qt::AlignCenter);

  row=0;
  m_statPointInitialLabel = new QLabel(tr("Initial Step Size:"));
  m_statPointInitialLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_statPointLayout->addWidget(m_statPointInitialLabel, row, 2);
  m_statPointInitialLine = new QLineEdit;
  connect( m_statPointInitialLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setStatPointInitial( QString ) ) );
  connect( m_statPointInitialLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointInitialLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  m_statPointLayout->addWidget(m_statPointInitialLine, row, 3);
  
  row++;
  m_statPointMinLabel = new QLabel(tr("Minimum Step Size:"));
  m_statPointMinLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_statPointLayout->addWidget(m_statPointMinLabel, row, 2);
  m_statPointMinLine = new QLineEdit;
  connect( m_statPointMinLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setStatPointMin( QString ) ) );
  connect( m_statPointMinLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointMinLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  m_statPointLayout->addWidget(m_statPointMinLine, row, 3);
  
  row++;
  m_statPointMaxLabel = new QLabel(tr("Maximum Step Size:"));
  m_statPointMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_statPointLayout->addWidget(m_statPointMaxLabel, row, 2);
  m_statPointMaxLine = new QLineEdit;
  connect( m_statPointMaxLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( setStatPointMax( QString ) ) );
  connect( m_statPointMaxLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointMaxLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( basicChanged() ) );
  m_statPointLayout->addWidget(m_statPointMaxLine, row, 3);
  
  row+=1;
  vlayout = new QVBoxLayout;
  m_statPointUpdateCheck = new QCheckBox(tr("Update Step Size"));
  connect( m_statPointUpdateCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setStatPointUpdate( bool ) ) );
  connect( m_statPointUpdateCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointUpdateCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_statPointUpdateCheck);
  m_statPointStationaryCheck = new QCheckBox(tr("Stationary Point"));
  connect( m_statPointStationaryCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( setStatPointStationary( bool ) ) );
  connect( m_statPointStationaryCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( updatePreviewText() ) );
  connect( m_statPointStationaryCheck, SIGNAL( toggled( bool ) ),
           this, SLOT( advancedChanged() ) );
  vlayout->addWidget(m_statPointStationaryCheck);
  m_statPointLayout->addLayout(vlayout, row, 3, 1, 1, Qt::AlignCenter);

  row++;
}

void GamessInputBuilder::setMode( int mode )
{
  if ( mode == 0 && m_advancedChanged )
  {
    QMessageBox msgbox( QMessageBox::Warning, tr( "Advanced Settings Changed" ),
                        tr( "Advanced settings have changed.\nDiscard?" ), QMessageBox::Abort | QMessageBox::Discard );
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
  m_stackedLayout->setCurrentIndex( mode );
}

void GamessInputBuilder::resetClicked()
{
  int current = m_stackedLayout->currentIndex();
  if( current )
  {
    QMessageBox msgbox( QMessageBox::Warning, tr( "Advanced Settings Reset" ),
                        tr( "Are you sure you wish to reset advanced settings?\n"
                            "All changes will be lost!" ), QMessageBox::Yes | QMessageBox::No );
    int response = msgbox.exec();
    if(response = QMessageBox::Yes)
    {
      m_advancedWidget->setCurrentIndex(0);
      setAdvancedDefaults();
      updateAdvancedWidgets();
      updatePreviewText();
      advancedChanged();
    }
  } else {
    QMessageBox msgbox( QMessageBox::Warning, tr( "Basic Settings Reset" ),
                        tr( "Are you sure you wish to reset basic settings?\n"
                            "All changes will be lost!" ), QMessageBox::Yes | QMessageBox::No );
    int response = msgbox.exec();
    if(response = QMessageBox::Yes)
    {
      setBasicDefaults();
      updatePreviewText();
    }
  }
}

void GamessInputBuilder::defaultsClicked()
{
  int current = m_stackedLayout->currentIndex();
  if( current )
  {
    // Advanced Page Defaults
    int tab = m_advancedWidget->currentIndex();
    switch(tab) {
      case 0:
        setBasisDefaults();
        break;
      case 1:
        setControlDefaults();
        break;
      case 2:
        setDataDefaults();
        break;
      case 3:
        setSystemDefaults();
        break;
      case 4:
        setMOGuessDefaults();
        break;
      case 5:
        setMiscDefaults();
        break;
      case 6:
        setSCFDefaults();
        break;
      case 7:
        setDFTDefaults();
        break;
      case 8:
        setMP2Defaults();
        break;
      case 9:
        setHessianDefaults();
        break;
      case 10:
        setStatPointDefaults();
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

void GamessInputBuilder::exportClicked()
{
  ofstream file( "/tmp/gamessoutput.test", ios_base::out );
  m_inputData->WriteInputFile( file );
}

void GamessInputBuilder::closeClicked()
{
  close();
}

// Basic Slots
void GamessInputBuilder::setBasicCalculateIndex( int index )
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

void GamessInputBuilder::setBasicWithLeftIndex( int index )
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
    setBasicWithRightIndex( 1 );
    m_inputData->Basis->SetBasis( basis );
  }
  else
  {
    setBasicWithRightIndex( m_basicWithRightCombo->currentIndex() );
  }

  m_inputData->Control->SetCCType( cc );
  m_inputData->Control->SetMPLevel( mp2 ? 2 : 0 );
  m_inputData->Control->UseDFT(dft);

  m_inputData->DFT->SetFunctional(func);

  m_basicWithRightCombo->setEnabled( er );
}

void GamessInputBuilder::setBasicWithRightIndex( int index )
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
  m_inputData->Basis->SetNumPFuncs( nd );

  m_inputData->Basis->SetDiffuseSP( dsp );
  m_inputData->Basis->SetDiffuseS( ds );

  m_inputData->Basis->SetECPPotential( ecp );
}

void GamessInputBuilder::setBasicInIndex( int index )
{
  m_inputData->Basis->SetWaterSolvate(index);
}

void GamessInputBuilder::setBasicOnLeftIndex( int index )
{
  GAMESS_SCFType scf = GAMESSDefaultSCFType;
  int mult = 0;
  switch ( index )
  {
    case 0:
      // Nothing
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

void GamessInputBuilder::setBasicOnRightIndex( int index )
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

void GamessInputBuilder::setBasicTimeLimitText( const QString &text )
{
  m_inputData->System->SetConvertedTime( text.toDouble() );
}

void GamessInputBuilder::setBasicMemoryText( const QString &text )
{
  // this should be the default
  // m_inputData->System->SetMemUnits(megaBytesUnit);
  m_inputData->System->SetConvertedMem( text.toDouble() );
}

// Basis Slots
void GamessInputBuilder::setBasisSetIndex( int index )
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

void GamessInputBuilder::setBasisECP( int index )
{
  m_inputData->Basis->SetECPPotential( index );
}

void GamessInputBuilder::setBasisD( int index )
{
  m_inputData->Basis->SetNumDFuncs( index );
  updateAdvancedWidgets();
}

void GamessInputBuilder::setBasisF( int index )
{
  m_inputData->Basis->SetNumFFuncs( index );
  updateAdvancedWidgets();
}

void GamessInputBuilder::setBasisLight( int index )
{
  m_inputData->Basis->SetNumPFuncs( index );
  updateAdvancedWidgets();
}

void GamessInputBuilder::setBasisPolar( int index )
{
  m_inputData->Basis->SetPolar(( GAMESS_BS_Polarization )( index ) );
}

void GamessInputBuilder::setBasisDiffuseL( bool state )
{
  m_inputData->Basis->SetDiffuseSP( state );
}

void GamessInputBuilder::setBasisDiffuseS( bool state )
{
  m_inputData->Basis->SetDiffuseS( state );
}


// Control Slots
void GamessInputBuilder::setControlRun( int index )
{
  m_inputData->Control->SetRunType(( TypeOfRun )( index+1 ) );
  updateAdvancedWidgets();
}

void GamessInputBuilder::setControlSCF( int index )
{
  m_inputData->Control->SetSCFType(( GAMESS_SCFType )( index +1 ) );
  updateAdvancedWidgets();
}

void GamessInputBuilder::setControlLocalization( int index )
{
  m_inputData->Control->SetLocal(( GAMESS_Localization )index );
  updateAdvancedWidgets();
}

void GamessInputBuilder::setControlExec( int index )
{
  m_inputData->Control->SetExeType( index );
}

void GamessInputBuilder::setControlMaxSCF( const QString &text )
{
  m_inputData->Control->SetMaxIt( text.toInt() );
}

void GamessInputBuilder::setControlCharge( const QString &text )
{
  m_inputData->Control->SetCharge( text.toInt() );
}

void GamessInputBuilder::setControlMultiplicity( const QString &text )
{
  m_inputData->Control->SetMultiplicity( text.toInt() );
  updateMOGuessWidgets();
}

void GamessInputBuilder::setControlMP2( bool state )
{
  if ( state )
    m_inputData->Control->SetMPLevel( 2 );
  else
    m_inputData->Control->SetMPLevel( 0 );

  updateAdvancedWidgets();
}

void GamessInputBuilder::setControlDFT( bool state )
{
  m_inputData->Control->UseDFT( state );
  updateAdvancedWidgets();
}

void GamessInputBuilder::setControlCI( int index )
{
  m_inputData->Control->SetCIType(( CIRunType )index );
  updateAdvancedWidgets();
}

void GamessInputBuilder::setControlCC( int index )
{
  m_inputData->Control->SetCCType(( CCRunType )index );
  updateAdvancedWidgets();
}


// Data Slots
void GamessInputBuilder::setDataTitle( const QString &text )
{
  m_inputData->Data->SetTitle( text.toAscii().constData() );
}

void GamessInputBuilder::setDataCoordinate( int index )
{
  m_inputData->Data->SetCoordType(( CoordinateType )( index+1 ) );
  updateAdvancedWidgets();
}

void GamessInputBuilder::setDataUnits( int index )
{
  m_inputData->Data->SetUnits( index );
}

void GamessInputBuilder::setDataZMatrix( const QString &text )
{
  m_inputData->Data->SetNumZVar( text.toInt() );
}

void GamessInputBuilder::setDataPoint( int index )
{
  m_inputData->Data->SetPointGroup(( GAMESSPointGroup )( index+1 ) );
  updatePointGroupOrderWidgets();
}

void GamessInputBuilder::setDataOrder( int index )
{
  m_inputData->Data->SetPointGroupOrder( index+2 );
}

void GamessInputBuilder::setDataSymmetry( bool state )
{
  m_inputData->Data->SetUseSym( state );
}


// System Slots
void GamessInputBuilder::setSystemTime( const QString &text )
{
  m_inputData->System->SetConvertedTime( text.toDouble() );
}

void GamessInputBuilder::setSystemTimeUnits( int index )
{
  m_inputData->System->SetTimeUnits(( TimeUnit )( index+1 ) );
  QString str = QString::number( m_inputData->System->GetConvertedTime() );
  m_systemTimeLine->setText( str );

}

void GamessInputBuilder::setSystemMemory( const QString &text )
{
  m_inputData->System->SetConvertedMem( text.toDouble() );
}

void GamessInputBuilder::setSystemMemoryUnits( int index )
{
  m_inputData->System->SetMemUnits(( MemoryUnit )( index+1 ) );
  QString str = QString::number( m_inputData->System->GetConvertedMem() );
  m_systemMemoryLine->setText( str );
}

void GamessInputBuilder::setSystemDDI( const QString &text )
{
  m_inputData->System->SetConvertedMemDDI( text.toDouble() );
}

void GamessInputBuilder::setSystemDDIUnits( int index )
{
  m_inputData->System->SetMemDDIUnits(( MemoryUnit )( index + megaWordsUnit ) );
}

void GamessInputBuilder::setSystemProduce( bool state )
{
  m_inputData->System->SetCoreFlag( state );
}

void GamessInputBuilder::setSystemForce( bool state )
{
  m_inputData->System->SetParallel( state );
}

void GamessInputBuilder::setSystemDiagonalization( int index )
{
  m_inputData->System->SetDiag( index );
}

void GamessInputBuilder::setSystemExternal( bool state )
{
  m_inputData->System->SetXDR( state );
}

void GamessInputBuilder::setSystemLoop( bool state )
{
  m_inputData->System->SetBalanceType( 0 );
}

void GamessInputBuilder::setSystemNext( bool state )
{
  m_inputData->System->SetBalanceType( 1 );
}

void GamessInputBuilder::setMOGuessInitial( int index )
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

void GamessInputBuilder::setMOGuessPrint( bool state )
{
  m_inputData->Guess->SetPrintMO( state );
}

void GamessInputBuilder::setMOGuessRotate( bool state )
{
  m_inputData->Guess->SetMix( state );
}

// Misc Slots
void GamessInputBuilder::setMiscMolPlt( bool state )
{
  m_inputData->Control->SetMolPlot( state );
}

void GamessInputBuilder::setMiscPltOrb( bool state )
{
  m_inputData->Control->SetPlotOrb( state );
}

void GamessInputBuilder::setMiscAIMPAC( bool state )
{
  m_inputData->Control->SetAIMPAC( state );
}

void GamessInputBuilder::setMiscRPAC( bool state )
{
  m_inputData->Control->SetRPAC( state );
}

void GamessInputBuilder::setMiscForce( int id )
{
  m_inputData->Control->SetFriend(( FriendType ) id );
  updateMiscWidgets();
  updateControlWidgets();

}

void GamessInputBuilder::setMiscWater( bool state )
{
  m_inputData->Basis->SetWaterSolvate(state);
}

// SCF Slots
void GamessInputBuilder::setSCFDirect( bool state )
{
  m_inputData->SCF->SetDirectSCF( state );
  updateSCFWidgets();
}

void GamessInputBuilder::setSCFCompute( bool state )
{
  m_inputData->SCF->SetFockDiff( state );
}

void GamessInputBuilder::setSCFGenerate( bool state )
{
  m_inputData->SCF->SetUHFNO( state );
}

// DFT Slots
void GamessInputBuilder::setDFTMethod( int index )
{
  m_inputData->DFT->SetMethodGrid(!index);
}

void GamessInputBuilder::setDFTFunctional( int index )
{
  qDebug() << "Distributed";
  m_inputData->DFT->SetFunctional(index+1);
}

// MP2 Slots
void GamessInputBuilder::setMP2Electrons( const QString &text )
{
  long val = -1;
  if( text != "" )
  {
    val = text.toLong();
  }
  m_inputData->MP2->SetNumCoreElectrons(val);
}

void GamessInputBuilder::setMP2Memory( const QString &text )
{
  m_inputData->MP2->SetMemory(text.toLong());
}

void GamessInputBuilder::setMP2Integral( const QString &text )
{
  double val = text.toDouble();
  if(fabs(val - 1.0e-9) < 1.0e-20) val = 0.0;
  m_inputData->MP2->SetIntCutoff(val);
}

void GamessInputBuilder::setMP2Localized( bool state )
{
  m_inputData->MP2->SetLMOMP2(state);
  updateMP2Widgets();
}

void GamessInputBuilder::setMP2Compute( bool state )
{
  m_inputData->MP2->SetMP2Prop(state);
}

void GamessInputBuilder::setMP2Segmented( bool state )
{
  m_inputData->MP2->SetMethod(2);
}

void GamessInputBuilder::setMP2Two( bool state )
{
  m_inputData->MP2->SetMethod(3);
}

void GamessInputBuilder::setMP2Duplicated( bool state )
{
  m_inputData->MP2->SetAOIntMethod(1);
}

void GamessInputBuilder::setMP2Distributed( bool state )
{
  m_inputData->MP2->SetAOIntMethod(2);
}

//! Hessian Slots
void GamessInputBuilder::setHessianAnalytic( bool state )
{
  m_inputData->Hessian->SetAnalyticMethod(1);
  updateHessianWidgets();
}

void GamessInputBuilder::setHessianNumeric( bool state )
{
  m_inputData->Hessian->SetAnalyticMethod(0);
  updateHessianWidgets();
}

void GamessInputBuilder::setHessianDisplacement( const QString &text )
{
  m_inputData->Hessian->SetDisplacementSize(text.toDouble());
}

void GamessInputBuilder::setHessianDouble( bool state )
{
  m_inputData->Hessian->SetPrintFC(state);
}

void GamessInputBuilder::setHessianPurify( bool state )
{
  m_inputData->Hessian->SetPurify(state);
}

void GamessInputBuilder::setHessianPrint( bool state )
{
  m_inputData->Hessian->SetPrintFC(state);
}

void GamessInputBuilder::setHessianVibrational( bool state )
{
  m_inputData->Hessian->SetVibAnalysis(state);
}

void GamessInputBuilder::setHessianScale( const QString &text )
{
  m_inputData->Hessian->SetFreqScale(text.toDouble());
}


//! Stat Point Slots
void GamessInputBuilder::setStatPointSteps( const QString &text )
{
  m_inputData->StatPt->SetMaxSteps(text.toLong());
}

void GamessInputBuilder::setStatPointRecalculate( const QString &text )
{
  m_inputData->StatPt->SetHessRecalcInterval(text.toLong());
}

void GamessInputBuilder::setStatPointConvergence( const QString &text )
{
  m_inputData->StatPt->SetOptConvergance(text.toDouble());
}

void GamessInputBuilder::setStatPointInitial( const QString &text )
{
  double val = text.toDouble();
  double def = 0.3;

  if(m_inputData->StatPt->GetMethod() == 5) def = 0.1;
  else if(m_inputData->Control->GetRunType() == 6) def = 0.2;
  if(fabs(val-def) < 1.0e-10) val = 0.0;
  m_inputData->StatPt->SetInitRadius(val);
}

void GamessInputBuilder::setStatPointMin( const QString &text )
{
  m_inputData->StatPt->SetMinRadius(text.toDouble());
}

void GamessInputBuilder::setStatPointMax( const QString &text )
{
  double val = text.toDouble();
  double def = 0.5;

  if(m_inputData->Control->GetRunType() == 6) def = 0.3;
  if(fabs(val-def) < 1.0e-10) val = 0.0;
  m_inputData->StatPt->SetMaxRadius(val);
}

void GamessInputBuilder::setStatPointOptimization( int index )
{
  m_inputData->StatPt->SetMethod(index+1);
  updateStatPointWidgets();
}

void GamessInputBuilder::setStatPointUpdate( bool state )
{
  m_inputData->StatPt->SetRadiusUpdate(state);
}

void GamessInputBuilder::setStatPointStationary( bool state )
{
  m_inputData->StatPt->SetStatPoint(state);
  updateStatPointWidgets();
}

void GamessInputBuilder::setStatPointHessian( int value )
{
  m_inputData->StatPt->SetHessMethod(value+1);
  updateStatPointWidgets();
}

void GamessInputBuilder::setStatPointPrint( bool state )
{
  m_inputData->StatPt->SetAlwaysPrintOrbs(state);
}

void GamessInputBuilder::setStatPointJump( const QString &text )
{
  m_inputData->StatPt->SetStatJump(text.toDouble());
}

void GamessInputBuilder::setStatPointFollow( const QString &text )
{
  m_inputData->StatPt->SetModeFollow(text.toLong());
}

