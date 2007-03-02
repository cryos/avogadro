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
#include <QDebug>

using namespace Avogadro;

GamessInputBuilder::GamessInputBuilder(GamessInputData *inputData, QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f), m_inputData(inputData)
{
  m_mainLayout = new QVBoxLayout;
  m_subLayout = new QHBoxLayout;
  setLayout(m_mainLayout);

  setWindowTitle(tr("GAMESS Input Deck Builder"));

  createTabs();
  createPreview();
  createButtons();

  m_mainLayout->addWidget(m_tabWidget);
  m_mainLayout->addWidget(m_previewText);
  m_mainLayout->addLayout(m_buttonLayout);

  updateWidgets();

  setDefaults();
}

GamessInputBuilder::~GamessInputBuilder()
{
}

void GamessInputBuilder::createTabs()
{
  QWidget *widget;

  // Tabs
  m_tabWidget = new QTabWidget(this);

  widget = new QWidget(this);
  m_basisLayout = new QGridLayout(widget);
  widget->setLayout(m_basisLayout);
  m_tabWidget->addTab(widget, tr("Basis"));
  createBasis();

  widget = new QWidget(this);
  m_controlLayout = new QGridLayout(widget);
  widget->setLayout(m_controlLayout);
  m_tabWidget->addTab(widget, tr("Control"));
  createControl();

  widget = new QWidget(this);
  m_dataLayout = new QGridLayout(widget);
  widget->setLayout(m_dataLayout);
  m_tabWidget->addTab(widget, tr("Data"));
  createData();

  widget = new QWidget(this);
  m_systemLayout = new QGridLayout(widget);
  widget->setLayout(m_systemLayout);
  m_tabWidget->addTab(widget, tr("System"));
  createSystem();

  widget = new QWidget(this);
  m_moGuessLayout = new QGridLayout(widget);
  widget->setLayout(m_moGuessLayout);
  m_tabWidget->addTab(widget, tr("MO Guess"));
  createMOGuess();

  widget = new QWidget(this);
  m_miscLayout = new QGridLayout(widget);
  widget->setLayout(m_miscLayout);
  m_tabWidget->addTab(widget, tr("misc"));
  createMisc();

  widget = new QWidget(this);
  m_scfLayout = new QGridLayout(widget);
  widget->setLayout(m_scfLayout);
  m_tabWidget->addTab(widget, tr("SCF"));
  createScf();

}

void GamessInputBuilder::createPreview()
{
  m_previewText = new QTextEdit(this);
}

void GamessInputBuilder::createButtons()
{
  // Button Bar
  m_buttonLayout = new QHBoxLayout();
  m_buttonLayout->setMargin(0);
  QPushButton *button = new QPushButton(tr("&Export"));
  connect(button, SIGNAL(clicked()), this, SLOT(exportClicked()));
  m_buttonLayout->addWidget(button);
  button = new QPushButton(tr("&Ok"));
  connect(button, SIGNAL(clicked()), this, SLOT(okClicked()));
  m_buttonLayout->addWidget(button);
  button = new QPushButton(tr("&Cancel"));
  connect(button, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  m_buttonLayout->addWidget(button);
}

void GamessInputBuilder::setDefaults()
{
  m_basisSetCombo->setCurrentIndex(3);
  m_timeLimitCombo->setCurrentIndex(2);
  m_timeLimitLine->setText(tr("10"));
  m_memoryCombo->setCurrentIndex(3);
  m_memoryLine->setText(tr("50"));
}

void GamessInputBuilder::updatePreviewText()
{
  stringstream str;
  m_inputData->WriteInputFile(str);
  m_previewText->setText(QString::fromAscii(str.str().c_str()));

}

void GamessInputBuilder::updateWidgets()
{
  updateBasisWidgets();
  updateControlWidgets();
  updateDataWidgets();
  updateSystemWidgets();
}

void GamessInputBuilder::updateBasisWidgets()
{
  int basis = m_inputData->Basis->GetBasis();
  int gauss = m_inputData->Basis->GetNumGauss();
  int itemValue = 0;
  int testValue = 0;

  // basisChoice
  itemValue = basis;
  if(itemValue == 0) itemValue = 1;
  else if(itemValue == 3) itemValue = gauss + 1;
  else if(itemValue == 4) {
    itemValue += 4;
    if(gauss == 6) itemValue++;
  }
  else if(itemValue == 5) itemValue = gauss + 6;
  else if(itemValue > 5) itemValue += 7;
  m_basisSetCombo->setCurrentIndex(itemValue - 1);

  //TODO: Implement this.
//   CheckBasisMenu();

  // m_ecpTypeCombo
  if(basis == 12 || basis == 13) {
    m_ecpTypeCombo->setEnabled(true);
    m_ecpTypeLabel->setEnabled(true);
    itemValue = m_inputData->Basis->GetECPPotential();
    if(itemValue == 0) {
      if(basis == 12) itemValue = 2;
      else itemValue = 3;
    }
    m_ecpTypeCombo->setCurrentIndex(itemValue);
  }
  else {
    m_ecpTypeCombo->setEnabled(false);
    m_ecpTypeLabel->setEnabled(false);
  }

  // polarChoice
  testValue = m_inputData->Basis->GetNumPFuncs() +
    m_inputData->Basis->GetNumDFuncs() +
    m_inputData->Basis->GetNumFFuncs();
  if(testValue) {
    m_polarCombo->setEnabled(true);
    m_polarLabel->setEnabled(true);
    itemValue = m_inputData->Basis->GetPolar();
    if(itemValue == 0) {
      if(basis == 6 || basis == 11) itemValue = 2;
      else if(basis == 7 || basis == 8) itemValue = 3;
      else if(basis < 3) itemValue = 4;
      else if(basis == 10) itemValue = 5;
      else itemValue = 1;
    }
    m_polarCombo->setCurrentIndex(itemValue - 1);
  }
  else {
    m_polarCombo->setEnabled(false);
    m_polarLabel->setEnabled(false);
  }

  // diffuseLCheck
  m_diffuseLShellCheck->setChecked(m_inputData->Basis->GetDiffuseSP());

  // diffuseSCheck
  m_diffuseSShellCheck->setChecked(m_inputData->Basis->GetDiffuseS());

  // numDChoice
  m_dHeavyAtomCombo->setCurrentIndex(m_inputData->Basis->GetNumDFuncs());

  // numFChoice
  m_fHeavyAtomCombo->setCurrentIndex(m_inputData->Basis->GetNumFFuncs());

  // numPChoice
  m_lightAtomCombo->setCurrentIndex(m_inputData->Basis->GetNumPFuncs());
}

void GamessInputBuilder::updateControlWidgets()
{
    short mp2 = m_inputData->Control->GetMPLevel();
    bool dft = m_inputData->Control->UseDFT();
    short ci = m_inputData->Control->GetCIType();
    CCRunType cc = m_inputData->Control->GetCCType();
    long scft = m_inputData->Control->GetSCFType();
    long NumElectrons = m_inputData->GetNumElectrons();
    
    int itemValue = 0;
    
    itemValue = m_inputData->Control->GetRunType();
    if(itemValue == 0) itemValue = 1;
    m_runTypeCombo->setCurrentIndex(itemValue - 1);
    
    if(scft == 0) {
        if(NumElectrons & 1) {
            scft = 3;
        }
        else {
            scft = 1;
        }
    }
    m_scfTypeCombo->setCurrentIndex(scft - 1);

    // mp2Check
    if(ci || cc || dft || (mp2 < 0)) {
        m_useMP2Check->setChecked(false);
        m_useMP2Check->setEnabled(false);
    }
    else {
        m_useMP2Check->setEnabled(true);
        if(mp2 < 0) mp2 = 0;
        if(mp2 == 2) mp2 = 1;
        m_useMP2Check->setChecked(mp2);
    }
    
    // dftCheck
    if(ci || cc || (mp2 > 0) || (scft > 3)) {
        m_useDFTCheck->setChecked(false);
        m_useDFTCheck->setEnabled(false);
    }
    else {
        m_useDFTCheck->setEnabled(true);
        m_useDFTCheck->setChecked(dft);
    // TODO: enable DFT
//         if(dft) setPaneVisible(DFT_PANE, true);
//         else setPaneVisible(DFT_PANE, false);
    }
    
    // ciCombo
    if((mp2 > 0) || dft || cc || scft == 2) {
        m_ciCombo->setCurrentIndex(0);
        m_ciCombo->setEnabled(false);
        m_ciLabel->setEnabled(false);
    }
    else {
        m_ciLabel->setEnabled(true);
        m_ciCombo->setEnabled(true);
        m_ciCombo->setCurrentIndex(ci);
    }
    
    // ccCombo
    if((mp2 > 0) || dft || ci || scft > 1) {
        m_ccCombo->setCurrentIndex(0);
        m_ccCombo->setEnabled(false);
        m_ccLabel->setEnabled(false);
    }
    else {
        m_ccLabel->setEnabled(true);
        m_ccCombo->setEnabled(true);
        m_ccCombo->setCurrentIndex(cc);
    }
    
    // scfIterText
    itemValue = m_inputData->Control->GetMaxIt();
    if(itemValue <= 0) itemValue = 30;
    m_maxSCFLine->setText(QString::number(itemValue));
    
    // exeCombo
    m_execTypeCombo->setCurrentIndex(m_inputData->Control->GetExeType());
    if(m_inputData->Control->GetFriend() != Friend_None) {
		//The friend keyword choices force a check run type
        m_execTypeCombo->setEnabled(false);
		m_execTypeCombo->setCurrentIndex(1);
    }
    else {
        m_execTypeCombo->setEnabled(true);
    }
    
    // mchargeText
    m_moleculeChargeLine->setText(QString::number(m_inputData->Control->GetCharge()));
    
    // multText
    itemValue = m_inputData->Control->GetMultiplicity();
    if(itemValue <= 0) {
        if(NumElectrons & 1) itemValue == 2;
        else itemValue = 1;
    }
    m_multiplicityLine->setText(QString::number(itemValue));
    
    // localCombo
    m_localizationMethodCombo->setCurrentIndex(m_inputData->Control->GetLocal());
}

void GamessInputBuilder::updateDataWidgets()
{
    int itemValue;
	
	//Title
	if (m_inputData->Data->GetTitle())
		m_titleLine->setText(tr(m_inputData->Data->GetTitle()));
	else
		m_titleLine->setText(tr("Title"));
    
    // coordTypeCombo
    itemValue = m_inputData->Data->GetCoordType();
    if(itemValue == 0) itemValue = 1;
    m_coordinateTypeCombo->setCurrentIndex(itemValue - 1);
    
    // unitCombo
    m_unitsCombo->setCurrentIndex(m_inputData->Data->GetUnits());
	
	//# Z-Matrix vars
	m_numZMatrixLine->setText(QString::number(m_inputData->Data->GetNumZVar()));
    
		//Point Group
	itemValue = m_inputData->Data->GetPointGroup();
	if (itemValue == 0) itemValue = 1;
	m_pointGroupCombo->setCurrentIndex(itemValue-1);

		//Point group order
	updatePointGroupOrderWidgets();
    
    // symmetryCheck
    m_useSymmetryCheck->setChecked(m_inputData->Data->GetUseSym());
}

void GamessInputBuilder::updateSystemWidgets()
{
    
    // timeLimitText
	m_timeLimitLine->setText(QString::number(m_inputData->System->GetConvertedTime()));
	
    // timeLimitUnitCombo
	m_timeLimitCombo->setCurrentIndex(m_inputData->System->GetTimeUnits() - 1);
	
    // memoryText
	m_memoryLine->setText(QString::number(m_inputData->System->GetConvertedMem()));
	
    // memoryUnitCombo
	m_memoryCombo->setCurrentIndex(m_inputData->System->GetMemUnits() - 1);
	
    // memDDI edit
	m_memDDILine->setText(QString::number(m_inputData->System->GetConvertedMemDDI()));
	
    // memDDIUnitCombo
	m_memDDICombo->setCurrentIndex(m_inputData->System->GetMemDDIUnits() - megaWordsUnit);

    // diagCombo
	m_diagonalizationCombo->setCurrentIndex(m_inputData->System->GetDiag());
	
	m_produceCoreCheck->setChecked(m_inputData->System->GetCoreFlag());
	
    if(m_inputData->System->GetBalanceType())
      m_nextRadio->setChecked(true);
    else
      m_loopRadio->setChecked(true);
	
	m_useExternalDataCheck->setChecked(m_inputData->System->GetXDR());

    // Parall check
	m_forceParallelCheck->setChecked(m_inputData->System->GetParallel());

}

void GamessInputBuilder::updatePointGroupOrderWidgets()
{
  //Point group order - only applicable to certain point groups
  int itemValue = m_inputData->Data->GetPointGroup();
  if (itemValue == 0) itemValue = 1;
  if ((itemValue>3)&&(itemValue<11)) {
    m_orderPrincipleAxisCombo->setEnabled(true);
    itemValue = m_inputData->Data->GetPointGroupOrder()-1;
    if (itemValue <= 0) {
      itemValue = 1;
      m_inputData->Data->SetPointGroupOrder(2);
    }
    m_orderPrincipleAxisCombo->setCurrentIndex(itemValue-1);
  } else m_orderPrincipleAxisCombo->setEnabled(false);
}

// Basis Tab
void GamessInputBuilder::createBasis()
{
  QHBoxLayout *layout;
  int row = 0;

  // Basis Set
  layout = new QHBoxLayout();
  layout->addStretch(40);
  m_basisSetLabel = new QLabel();
  m_basisSetLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_basisSetLabel->setText(tr("Basis Set"));
  layout->addWidget(m_basisSetLabel);

  m_basisSetCombo = new QComboBox();
  m_basisSetCombo->addItem(tr("MINI"));
  m_basisSetCombo->addItem(tr("MIDI"));
  m_basisSetCombo->addItem(tr("STO-2G"));
  m_basisSetCombo->addItem(tr("STO-3G"));
  m_basisSetCombo->addItem(tr("STO-4G"));
  m_basisSetCombo->addItem(tr("STO-5G"));
  m_basisSetCombo->addItem(tr("STO-6G"));
  m_basisSetCombo->addItem(tr("3-21G"));
  m_basisSetCombo->addItem(tr("6-21G"));
  m_basisSetCombo->addItem(tr("4-31G"));
  m_basisSetCombo->addItem(tr("5-31G"));
  m_basisSetCombo->addItem(tr("6-31G"));
  m_basisSetCombo->addItem(tr("6-311G"));
  m_basisSetCombo->addItem(tr("Double Zeta Valance"));
  m_basisSetCombo->addItem(tr("Dunning/Hay DZ"));
  m_basisSetCombo->addItem(tr("Binning/Curtiss DZ"));
  m_basisSetCombo->addItem(tr("Triple Zeta Valence"));
  m_basisSetCombo->addItem(tr("McLean/Chandler"));
  m_basisSetCombo->addItem(tr("SBKJC Valance"));
  m_basisSetCombo->addItem(tr("Hay/Wadt Valance"));
  m_basisSetCombo->addItem(tr("MNDO"));
  m_basisSetCombo->addItem(tr("AM1"));
  m_basisSetCombo->addItem(tr("PM3"));
  connect(m_basisSetCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setBasisSetIndex(int)));
  connect(m_basisSetCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));

  layout->addWidget(m_basisSetCombo);
  layout->addStretch(40);

  m_basisLayout->addLayout(layout,row,0);

  // ECP Type
  layout = new QHBoxLayout();
  m_ecpTypeLabel = new QLabel(tr("ECP Type"));
  m_ecpTypeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  layout->addWidget(m_ecpTypeLabel);

  m_ecpTypeCombo = new QComboBox();
  m_ecpTypeCombo->addItem(tr("None"));
  m_ecpTypeCombo->addItem(tr("Read"));
  m_ecpTypeCombo->addItem(tr("SBKJC"));
  m_ecpTypeCombo->addItem(tr("Hay-Wadt"));
  connect(m_ecpTypeCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setECPTypeIndex(int)));
  connect(m_ecpTypeCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));

  layout->addWidget(m_ecpTypeCombo);
  layout->addStretch(40);

  m_basisLayout->addLayout(layout,row,1);

  // D Heavy Atom Polarization
  row++;
  m_dHeavyAtomLabel = new QLabel(tr("#D Heavy Atom Polarization Functions"));
  m_dHeavyAtomLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_basisLayout->addWidget(m_dHeavyAtomLabel, row, 0);

  m_dHeavyAtomCombo = new QComboBox();
  m_dHeavyAtomCombo->addItem(tr("0"));
  m_dHeavyAtomCombo->addItem(tr("1"));
  m_dHeavyAtomCombo->addItem(tr("2"));
  m_dHeavyAtomCombo->addItem(tr("3"));
  connect(m_dHeavyAtomCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setDHeavyAtomIndex(int)));
  connect(m_dHeavyAtomCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));

  m_basisLayout->addWidget(m_dHeavyAtomCombo, row, 1);

  // F Heavy Atom Polarization
  row++;
  m_fHeavyAtomLabel = new QLabel(tr("#F Heavy Atom Polarization Functions"));
  m_fHeavyAtomLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_basisLayout->addWidget(m_fHeavyAtomLabel, row, 0);

  m_fHeavyAtomCombo = new QComboBox();
  m_fHeavyAtomCombo->addItem(tr("0"));
  m_fHeavyAtomCombo->addItem(tr("1"));
  m_fHeavyAtomCombo->addItem(tr("2"));
  m_fHeavyAtomCombo->addItem(tr("3"));
  connect(m_fHeavyAtomCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setFHeavyAtomIndex(int)));
  connect(m_fHeavyAtomCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));

  m_basisLayout->addWidget(m_fHeavyAtomCombo, row, 1);

  // Light Atom Polarization
  row++;
  m_lightAtomLabel = new QLabel(tr("#light Atom Polarization Functions"));
  m_lightAtomLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_basisLayout->addWidget(m_lightAtomLabel, row, 0);

  m_lightAtomCombo = new QComboBox();
  m_lightAtomCombo->addItem(tr("0"));
  m_lightAtomCombo->addItem(tr("1"));
  m_lightAtomCombo->addItem(tr("2"));
  m_lightAtomCombo->addItem(tr("3"));
  connect(m_lightAtomCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setLightAtomIndex(int)));
  connect(m_lightAtomCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));

  m_basisLayout->addWidget(m_lightAtomCombo, row, 1);

  // Polar 
  row++;
  m_polarLabel = new QLabel(tr("Polar"));
  m_polarLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_basisLayout->addWidget(m_polarLabel, row, 0);

  m_polarCombo = new QComboBox();
  m_polarCombo->addItem(tr("Default"));
  m_polarCombo->addItem(tr("Pople"));
  m_polarCombo->addItem(tr("Pople N311"));
  m_polarCombo->addItem(tr("Dunning"));
  m_polarCombo->addItem(tr("Huzinaga"));
  m_polarCombo->addItem(tr("Hondo7"));
  connect(m_polarCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setPolarIndex(int)));
  connect(m_polarCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  m_basisLayout->addWidget(m_polarCombo, row, 1);


  // Diffuse L-Shell on Heavy Atoms
  row++;
  m_diffuseLShellCheck = new QCheckBox(tr("Diffuse L-Shell on Heavy Atoms"));
  connect(m_diffuseLShellCheck, SIGNAL(toggled(bool)),
      this, SLOT(setDiffuseLShellState(bool)));
  connect(m_diffuseLShellCheck, SIGNAL(toggled(bool)),
      this, SLOT(updatePreviewText()));
  m_basisLayout->addWidget(m_diffuseLShellCheck, row, 0, Qt::AlignCenter);

  // Diffuse S-Shell on Heavy Atoms
  m_diffuseSShellCheck = new QCheckBox(tr("Diffuse S-Shell on Heavy Atoms"));
  connect(m_diffuseSShellCheck, SIGNAL(toggled(bool)),
      this, SLOT(setDiffuseSShellState(bool)));
  connect(m_diffuseSShellCheck, SIGNAL(toggled(bool)),
      this, SLOT(updatePreviewText()));
  m_basisLayout->addWidget(m_diffuseSShellCheck, row, 1, Qt::AlignLeft);

}

void GamessInputBuilder::createControl()
{
  // general use variable for newly created layouts
  QHBoxLayout *layout;
  QFrame *divider;
  int row = 0;

  // Run Type
  layout = new QHBoxLayout();
  m_runTypeLabel = new QLabel(tr("Run Type:"));
  m_runTypeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_controlLayout->addWidget(m_runTypeLabel,row,0,1,2,Qt::AlignRight);
  m_runTypeCombo = new QComboBox();
  m_runTypeCombo->addItem(tr("Energy"));
  m_runTypeCombo->addItem(tr("Gradient"));
  m_runTypeCombo->addItem(tr("Hessian"));
  m_runTypeCombo->addItem(tr("Optimization"));
  m_runTypeCombo->addItem(tr("Trudge"));
  m_runTypeCombo->addItem(tr("Saddle Point"));
  m_runTypeCombo->addItem(tr("IRC"));
  m_runTypeCombo->addItem(tr("Gradient Extremal"));
  m_runTypeCombo->addItem(tr("DRC"));
  m_runTypeCombo->addItem(tr("Energy Surface"));
  m_runTypeCombo->addItem(tr("Properties"));
  m_runTypeCombo->addItem(tr("Morokuma"));
  m_runTypeCombo->addItem(tr("Radiative Transition mom."));
  m_runTypeCombo->addItem(tr("Spin Orbit"));
  m_runTypeCombo->addItem(tr("Finite Electric Field"));
  m_runTypeCombo->addItem(tr("TDHF"));
  m_runTypeCombo->addItem(tr("Global Optimization"));
  m_runTypeCombo->addItem(tr("VSCF"));
  m_runTypeCombo->addItem(tr("FMO Optimization"));
  m_runTypeCombo->addItem(tr("Raman Intensities"));
  m_runTypeCombo->addItem(tr("NMR"));
  m_runTypeCombo->addItem(tr("Make EFP"));
  connect(m_runTypeCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setRunTypeIndex(int)));
  connect(m_runTypeCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  layout->addWidget(m_runTypeCombo);

  // SCF Type
  layout->addSpacing(20);
  m_scfTypeLabel = new QLabel(tr("SCF Type:"));
  m_scfTypeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  layout->addWidget(m_scfTypeLabel);
  m_controlLayout->addLayout(layout, row, 2, 1, 2);
  m_scfTypeCombo = new QComboBox();
  m_scfTypeCombo->addItem(tr("RHF"));
  m_scfTypeCombo->addItem(tr("UHF"));
  m_scfTypeCombo->addItem(tr("ROHF"));
  m_scfTypeCombo->addItem(tr("GVB"));
  m_scfTypeCombo->addItem(tr("MCSCF"));
  m_scfTypeCombo->addItem(tr("None (CI)"));
  connect(m_scfTypeCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setSCFTypeIndex(int)));
  connect(m_scfTypeCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  m_controlLayout->addWidget(m_scfTypeCombo, row, 4);

  // Localization Method
  row++;
  m_localizationMethodLabel = new QLabel(tr("Localization Method:"));
  m_localizationMethodLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_controlLayout->addWidget(m_localizationMethodLabel, row, 0, 1, 2);
  m_localizationMethodCombo = new QComboBox();
  m_localizationMethodCombo->addItem(tr("None"));
  m_localizationMethodCombo->addItem(tr("Foster-Boys"));
  m_localizationMethodCombo->addItem(tr("Edmiston-Ruedenberg"));
  m_localizationMethodCombo->addItem(tr("Pipek-Mezey"));
  connect(m_localizationMethodCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setLocalizationMethodIndex(int)));
  connect(m_localizationMethodCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  m_controlLayout->addWidget(m_localizationMethodCombo, row, 2, 1, 2);

  // Divider
  row++;
  divider = new QFrame();
  divider->setFrameShape(QFrame::HLine);
  divider->setFrameShadow(QFrame::Sunken);
  divider->setLineWidth(0);
  divider->setMidLineWidth(0);
  m_controlLayout->addWidget(divider, row, 1, 1, 5);

  row++;
  // Exec Type
  layout = new QHBoxLayout();
  m_execTypeLabel = new QLabel(tr("Exec Type:"));
  m_execTypeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_controlLayout->addWidget(m_execTypeLabel, row, 0, 1, 2);
  m_execTypeCombo = new QComboBox();
  m_execTypeCombo->addItem(tr("Normal Run"));
  m_execTypeCombo->addItem(tr("Check"));
  m_execTypeCombo->addItem(tr("Debug"));
  m_execTypeCombo->addItem(tr("Other..."));
  connect(m_execTypeCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setExecTypeIndex(int)));
  connect(m_execTypeCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  layout->addWidget(m_execTypeCombo);

  // Molecule Charge
  layout->addStretch(20);
  m_moleculeChargeLabel = new QLabel(tr("Molecule Charge:"));
  m_moleculeChargeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  layout->addWidget(m_moleculeChargeLabel);
  m_controlLayout->addLayout(layout, row, 2, 1, 2);
  m_moleculeChargeLine = new QLineEdit();
  m_moleculeChargeLine->setText(tr("0"));
  m_moleculeChargeLine->setToolTip(tr("$CONTRL:ICHARG - Enter an integer value for the molecular charge."));
  connect(m_moleculeChargeLine, SIGNAL(textChanged(QString)),
      this, SLOT(setMoleculeChargeText(QString)));
  connect(m_moleculeChargeLine, SIGNAL(textChanged(QString)),
      this, SLOT(updatePreviewText()));
  m_controlLayout->addWidget(m_moleculeChargeLine, row, 4);

  row++;
  layout = new QHBoxLayout();
  // Maximum SCF Iterations
  m_maxSCFLabel = new QLabel(tr("Max SCF Iterations:"));
  m_maxSCFLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_controlLayout->addWidget(m_maxSCFLabel, row, 0, 1, 2);
  m_maxSCFLine = new QLineEdit();
  m_maxSCFLine->setText(tr("30"));
  m_maxSCFLine->setToolTip(tr("$CONTRL:MAXIT - Enter the maximum number of SCF iterations. If the wavefunction is not converged at this point the run will be aborted."));
  connect(m_maxSCFLine, SIGNAL(textChanged(QString)),
      this, SLOT(setMaxSCFText(QString)));
  connect(m_maxSCFLine, SIGNAL(textChanged(QString)),
      this, SLOT(updatePreviewText()));
  layout->addWidget(m_maxSCFLine);

  // Multiplicity
  layout->addStretch(20);
  m_multiplicityLabel = new QLabel(tr("Multiplicity:"));
  m_multiplicityLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  layout->addWidget(m_multiplicityLabel);
  m_controlLayout->addLayout(layout, row, 2, 1, 2);
  m_multiplicityLine = new QLineEdit();
  m_multiplicityLine->setText(tr("1"));
  m_multiplicityLine->setToolTip(tr("$CONTRL:MULT - Enter an integer value for the spin state."));
  connect(m_multiplicityLine, SIGNAL(textChanged(QString)),
      this, SLOT(setMultiplicityText(QString)));
  connect(m_multiplicityLine, SIGNAL(textChanged(QString)),
      this, SLOT(updatePreviewText()));
  m_controlLayout->addWidget(m_multiplicityLine, row, 4);

  // Divider
  row++;
  divider = new QFrame();
  divider->setFrameShape(QFrame::HLine);
  divider->setFrameShadow(QFrame::Sunken);
  divider->setLineWidth(0);
  divider->setMidLineWidth(0);
  m_controlLayout->addWidget(divider, row, 0, 1, 5);

  row++;
  // Use MP2
  m_useMP2Check = new QCheckBox(tr("Use MP2"));
  m_useMP2Check->setToolTip(tr("$CONTRL:MPLEVL - Click to use 2nd order Moller-Plesset perturbation theory. Implemented for RHF, UHF, ROHF and MCSCF energies and RHF, UHF, and ROHF gradients."));
  connect(m_useMP2Check, SIGNAL(toggled(bool)),
      this, SLOT(setUseMP2State(bool)));
  connect(m_useMP2Check, SIGNAL(toggled(bool)),
      this, SLOT(updatePreviewText()));
  m_controlLayout->addWidget(m_useMP2Check, row, 1);

  // CI
  QGridLayout *glayout = new QGridLayout();
  m_ciLabel = new QLabel(tr("CI:"));
  m_ciLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  glayout->addWidget(m_ciLabel, 0, 0);
  m_ciCombo = new QComboBox();
  m_ciCombo->addItem(tr("None"));
  m_ciCombo->addItem(tr("GUGA"));
  m_ciCombo->addItem(tr("Ames Lab. Determinant"));
  m_ciCombo->addItem(tr("Occupation Restricted Multiple Active Space"));
  m_ciCombo->addItem(tr("CI Singles"));
  m_ciCombo->addItem(tr("Full Second Order CI"));
  m_ciCombo->addItem(tr("General CI"));
  m_ciCombo->setToolTip(tr("$CONTRL:CITYP Choose the type of CI to perform on top of the base wavefunction or on the supplied $VEC group for SCFTYP=NONE."));
  connect(m_ciCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setCIIndex(int)));
  connect(m_ciCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  glayout->addWidget(m_ciCombo, 0, 1);
  
  // CC
  m_ccLabel = new QLabel(tr("CC:"));
  m_ccLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  glayout->addWidget(m_ccLabel, 1, 0);
  m_ccCombo = new QComboBox();
  m_ccCombo->addItem(tr("None"));
  m_ccCombo->addItem(tr("LCCD: linearized CC"));
  m_ccCombo->addItem(tr("CCD: CC with doubles"));
  m_ccCombo->addItem(tr("CCSD: CC with singles and doubles"));
  m_ccCombo->addItem(tr("CCSD(T)"));
  m_ccCombo->addItem(tr("R-CC"));
  m_ccCombo->addItem(tr("CR-CC"));
  m_ccCombo->addItem(tr("EOM-CCSD"));
  m_ccCombo->addItem(tr("CR-EOM"));
  connect(m_ccCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setCCIndex(int)));
  connect(m_ccCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  glayout->addWidget(m_ccCombo, 1, 1);
  m_controlLayout->addLayout(glayout, row, 2,2,3);

  row++;
  // Use DFT
  m_useDFTCheck = new QCheckBox(tr("Use DFT"));
  connect(m_useDFTCheck, SIGNAL(toggled(bool)),
      this, SLOT(setUseDFTState(bool)));
  connect(m_useDFTCheck, SIGNAL(toggled(bool)),
      this, SLOT(updatePreviewText()));
  m_controlLayout->addWidget(m_useDFTCheck, row, 1);

}

void GamessInputBuilder::createData()
{
  QHBoxLayout *layout;
  QFrame *divider;
  int row = 0;

  m_dataLayout->setColumnStretch(0, 50);
  m_dataLayout->setColumnStretch(1, 50);

  // Title
  m_titleLabel = new QLabel(tr("Title:"));
  m_dataLayout->addWidget(m_titleLabel, row, 0);
  m_titleLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_titleLine = new QLineEdit;
  m_titleLine->setText(tr("Title"));
  m_titleLine->setToolTip(tr("$DATA - You may enter a one line title which may help you identify this input deck in the future."));
  connect(m_titleLine, SIGNAL(textChanged(QString)),
      this, SLOT(setTitleText(QString)));
  connect(m_titleLine, SIGNAL(textChanged(QString)),
      this, SLOT(updatePreviewText()));
  layout = new QHBoxLayout;
  layout->addWidget(m_titleLine);
  layout->addStretch(40);
  m_dataLayout->addLayout(layout, row, 1);

  row++;
  // Coordinate Type
  m_coordinateTypeLabel = new QLabel(tr("Coordinate Type:"));
  m_dataLayout->addWidget(m_coordinateTypeLabel, row, 0);
  m_coordinateTypeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_coordinateTypeCombo = new QComboBox;
  m_coordinateTypeCombo->addItem(tr("Unique cartesian Coords."));
  m_coordinateTypeCombo->addItem(tr("Hilderbrant internals"));
  m_coordinateTypeCombo->addItem(tr("Cartesian coordinates"));
  m_coordinateTypeCombo->addItem(tr("Z-Matrix"));
  m_coordinateTypeCombo->addItem(tr("MOPAC Z-Matrix"));
  connect(m_coordinateTypeCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setCoordinateTypeIndex(int)));
  connect(m_coordinateTypeCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  layout = new QHBoxLayout;
  layout->addWidget(m_coordinateTypeCombo);
  layout->addStretch(40);
  m_dataLayout->addLayout(layout, row, 1);

  row++;
  // Units
  m_unitsLabel = new QLabel(tr("Units:"));
  m_dataLayout->addWidget(m_unitsLabel, row, 0);
  m_unitsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_unitsCombo = new QComboBox;
  m_unitsCombo->addItem(tr("Angstroms"));
  m_unitsCombo->addItem(tr("Bohr"));
  connect(m_unitsCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setUnitsIndex(int)));
  connect(m_unitsCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  layout = new QHBoxLayout;
  layout->addWidget(m_unitsCombo);
  layout->addStretch(40);
  m_dataLayout->addLayout(layout, row, 1);

  row++;
  // Number of Z-Matrix Variables
  m_numZMatrixLabel = new QLabel(tr("# if Z-Matrix Variables:"));
  m_dataLayout->addWidget(m_numZMatrixLabel, row, 0);
  m_numZMatrixLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_numZMatrixLine = new QLineEdit;
  m_numZMatrixLine->setText(tr("0"));
  m_numZMatrixLine->setToolTip(tr("$CONTRL:NZVAR - Enter an integer number representing the number of internal coordinates for your molecule. Normally this will be 3N-6 (3N-5 for linear molecules) where N is the number of atoms. A value of 0 selects cartesian coordinates. If set and a set of internal coordinates are defined a $ZMAT group will be punched out."));
  connect(m_numZMatrixLine, SIGNAL(textChanged(QString)),
      this, SLOT(setNumZMatrixText(QString)));
  connect(m_numZMatrixLine, SIGNAL(textChanged(QString)),
      this, SLOT(updatePreviewText()));
  layout = new QHBoxLayout;
  layout->addWidget(m_numZMatrixLine);
  layout->addStretch(40);
  m_dataLayout->addLayout(layout, row, 1);

  row++;
  // Point Group
  m_pointGroupLabel = new QLabel(tr("Point Group:"));
  m_dataLayout->addWidget(m_pointGroupLabel, row, 0);
  m_pointGroupLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_pointGroupCombo = new QComboBox;
  m_pointGroupCombo->addItem(tr("C1"));
  m_pointGroupCombo->addItem(tr("CS"));
  m_pointGroupCombo->addItem(tr("CI"));
  m_pointGroupCombo->addItem(tr("CnH"));
  m_pointGroupCombo->addItem(tr("CnV"));
  m_pointGroupCombo->addItem(tr("Cn"));
  m_pointGroupCombo->addItem(tr("S2n"));
  m_pointGroupCombo->addItem(tr("DnD"));
  m_pointGroupCombo->addItem(tr("DnH"));
  m_pointGroupCombo->addItem(tr("Dn"));
  m_pointGroupCombo->addItem(tr("TD"));
  m_pointGroupCombo->addItem(tr("TH"));
  m_pointGroupCombo->addItem(tr("T"));
  m_pointGroupCombo->addItem(tr("OH"));
  m_pointGroupCombo->addItem(tr("O"));
  connect(m_pointGroupCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setPointGroupIndex(int)));
  connect(m_pointGroupCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  layout = new QHBoxLayout;
  layout->addWidget(m_pointGroupCombo);
  layout->addStretch(40);
  m_dataLayout->addLayout(layout, row, 1);

  row++;
  // Order of Principle Axis
  m_orderPrincipleAxisLabel = new QLabel(tr("Order of Principle Axis:"));
  m_dataLayout->addWidget(m_orderPrincipleAxisLabel, row, 0);
  m_orderPrincipleAxisLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_orderPrincipleAxisCombo = new QComboBox;
  m_orderPrincipleAxisCombo->addItem(tr("2"));
  m_orderPrincipleAxisCombo->addItem(tr("3"));
  m_orderPrincipleAxisCombo->addItem(tr("4"));
  m_orderPrincipleAxisCombo->setToolTip(tr("Replaces the 'n' above."));
  connect(m_orderPrincipleAxisCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setOrderPrincipleAxisIndex(int)));
  connect(m_orderPrincipleAxisCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  layout = new QHBoxLayout;
  layout->addWidget(m_orderPrincipleAxisCombo);
  layout->addStretch(40);
  m_dataLayout->addLayout(layout, row, 1);
  
  // Divider
  row++;
  divider = new QFrame();
  divider->setFrameShape(QFrame::HLine);
  divider->setFrameShadow(QFrame::Sunken);
  divider->setLineWidth(0);
  divider->setMidLineWidth(0);
  m_dataLayout->addWidget(divider, row, 0, 1, 1);

          
  row++;
  // Use Symmetry During Calculation
  m_useSymmetryCheck = new QCheckBox(tr("Use Symmetry During Calculation"));
  m_useSymmetryCheck->setToolTip(tr("$CONTRL:NOSYM - When checked symmetry will be used as much as possible in the caluclation of integrals, gradients, etc. (This is the normal setting)"));
  connect(m_useSymmetryCheck, SIGNAL(toggled(bool)),
      this, SLOT(setUseSymmetryState(bool)));
  connect(m_useSymmetryCheck, SIGNAL(toggled(bool)),
      this, SLOT(updatePreviewText()));
  m_dataLayout->addWidget(m_useSymmetryCheck, row, 0, 1, 2, Qt::AlignCenter);

}

void GamessInputBuilder::createSystem()
{
  QHBoxLayout *layout;
  QSpacerItem *spacer;
  QFrame *divider;
  int row = 0;

  spacer = new QSpacerItem(0,0);

  m_systemLayout->setColumnMinimumWidth(2, 130);
  m_systemLayout->setColumnStretch(0, 1);
  m_systemLayout->setColumnStretch(4, 1);

  // Time Limit
  m_timeLimitLabel = new QLabel(tr("Time Limit:"));
  m_timeLimitLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_systemLayout->addWidget(m_timeLimitLabel, row, 0, 1, 1);
  m_timeLimitLine = new QLineEdit();
  m_timeLimitLine->setText(tr("525600"));
  m_timeLimitLine->setToolTip(tr("$SYSTEM:TIMLIM - Enter a value for the time limit. When the time limit is reached GAMESS will stop the run. The number entered here will have the units given at the right."));
  connect(m_timeLimitLine, SIGNAL(textChanged(QString)),
      this, SLOT(setTimeLimitText(QString)));
  connect(m_timeLimitLine, SIGNAL(textChanged(QString)),
      this, SLOT(updatePreviewText()));
  m_systemLayout->addWidget(m_timeLimitLine, row, 1);
  layout = new QHBoxLayout;
  m_timeLimitCombo = new QComboBox;
  m_timeLimitCombo->addItem(tr("Seconds"));
  m_timeLimitCombo->addItem(tr("Minutes"));
  m_timeLimitCombo->addItem(tr("Hours"));
  m_timeLimitCombo->addItem(tr("Days"));
  m_timeLimitCombo->addItem(tr("Weeks"));
  m_timeLimitCombo->addItem(tr("Years"));
  m_timeLimitCombo->addItem(tr("Millenia"));
  connect(m_timeLimitCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setTimeLimitIndex(int)));
  connect(m_timeLimitCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  layout->addWidget(m_timeLimitCombo);
  layout->addStretch(40);
  m_systemLayout->addLayout(layout, row, 2, 1, 1);

  row++;
  // Memory
  m_memoryLabel = new QLabel(tr("Memory:"));
  m_memoryLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_systemLayout->addWidget(m_memoryLabel, row, 0, 1, 1);
  m_memoryLine = new QLineEdit();
  m_memoryLine->setText(tr("1000000"));
  m_memoryLine->setToolTip(tr("$SYSTEM:MEMORY - Enter the amount of memory (in the units at the right) that GAMESS will request for its dynamic memory pool. You should not normally request more memory than the RAM size."));
  connect(m_memoryLine, SIGNAL(textChanged(QString)),
      this, SLOT(setMemoryText(QString)));
  connect(m_memoryLine, SIGNAL(textChanged(QString)),
      this, SLOT(updatePreviewText()));
  m_systemLayout->addWidget(m_memoryLine, row, 1);
  layout = new QHBoxLayout;
  m_memoryCombo = new QComboBox;
  m_memoryCombo->addItem(tr("Words"));
  m_memoryCombo->addItem(tr("Bytes"));
  m_memoryCombo->addItem(tr("MegaWords"));
  m_memoryCombo->addItem(tr("MegaBytes"));
  connect(m_memoryCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setMemoryIndex(int)));
  connect(m_memoryCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  layout->addWidget(m_memoryCombo);
  layout->addStretch(40);
  m_systemLayout->addLayout(layout, row, 2, 1, 1);

  row++;
  // MemDDI
  m_memDDILabel = new QLabel(tr("MemDDI:"));
  m_memDDILabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_systemLayout->addWidget(m_memDDILabel, row, 0, 1, 1);
  m_memDDILine = new QLineEdit();
  m_memDDILine->setText(tr("0.00"));
  m_memDDILine->setToolTip(tr("$SYSTEM:MEMDDI - The size of the pseudo global shared memory pool. This is most often needed for certain parallel computations, but certain sequential algorithms also use it (such as ROMP2). Default is 0."));
  connect(m_memDDILine, SIGNAL(textChanged(QString)),
      this, SLOT(setMemDDIText(QString)));
  connect(m_memDDILine, SIGNAL(textChanged(QString)),
      this, SLOT(updatePreviewText()));
  m_systemLayout->addWidget(m_memDDILine, row, 1);
  layout = new QHBoxLayout;
  m_memDDICombo = new QComboBox;
  m_memDDICombo->addItem(tr("MegaWords"));
  m_memDDICombo->addItem(tr("MegaBytes"));
  m_memDDICombo->addItem(tr("GigaWords"));
  m_memDDICombo->addItem(tr("GigaBytes"));
  connect(m_memDDICombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setMemDDIIndex(int)));
  connect(m_memDDICombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  layout->addWidget(m_memDDICombo);
  layout->addStretch(40);
  m_systemLayout->addLayout(layout, row, 2, 1, 1);

  row++;
  // Diagonalization Method
  m_diagonalizationLabel = new QLabel(tr("Diagonalization Method:"));
  m_diagonalizationLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_systemLayout->addWidget(m_diagonalizationLabel, row, 0, 1, 2);
  m_diagonalizationCombo = new QComboBox;
  m_diagonalizationCombo->addItem(tr("Default"));
  m_diagonalizationCombo->addItem(tr("EVVRSP"));
  m_diagonalizationCombo->addItem(tr("GIVEIS"));
  m_diagonalizationCombo->addItem(tr("JACOBI"));
  connect(m_diagonalizationCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setDiagonalizationIndex(int)));
  connect(m_diagonalizationCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(updatePreviewText()));
  layout = new QHBoxLayout;
  layout->addWidget(m_diagonalizationCombo);
  layout->addStretch(40);
  m_systemLayout->addLayout(layout, row, 2);
  
  row++;
  // Use External Data Representation for Messages
  m_useExternalDataCheck = new QCheckBox(tr("Use External Data Representation for Messages"));
  connect(m_useExternalDataCheck, SIGNAL(toggled(bool)),
      this, SLOT(setUseExternalDataState(bool)));
  connect(m_useExternalDataCheck, SIGNAL(toggled(bool)),
      this, SLOT(updatePreviewText()));
  m_systemLayout->addWidget(m_useExternalDataCheck, row, 1, 1, 3, Qt::AlignCenter);

  row = 0;
  // Produce "core" file upon abort.
  m_produceCoreCheck  = new QCheckBox(tr("Produce \"core\" file upon abort"));
  connect(m_produceCoreCheck, SIGNAL(toggled(bool)),
      this, SLOT(setProduceCoreState(bool)));
  connect(m_produceCoreCheck, SIGNAL(toggled(bool)),
      this, SLOT(updatePreviewText()));
  m_systemLayout->addWidget(m_produceCoreCheck, row, 3, 1, 1);

  row++;
  // Force Parallel Methods
  m_forceParallelCheck = new QCheckBox(tr("Force Parallel Methods"));
  connect(m_forceParallelCheck, SIGNAL(toggled(bool)),
      this, SLOT(setForceParallelState(bool)));
  connect(m_forceParallelCheck, SIGNAL(toggled(bool)),
      this, SLOT(updatePreviewText()));
  m_systemLayout->addWidget(m_forceParallelCheck, row, 3, 1, 1);
  row++;
  // Parallel Load Balance Type
  m_parallelLoadGroup = new QGroupBox(tr("Parallel Load Balance Type"));
  m_parallelLoadGroup->setFlat(true);
  m_loopRadio = new QRadioButton(tr("Loop"));
  connect(m_loopRadio, SIGNAL(toggled(bool)), this, SLOT(setLoopState(bool)));
  connect(m_loopRadio, SIGNAL(toggled(bool)), this, SLOT(updatePreviewText()));
  m_nextRadio = new QRadioButton(tr("Next Value"));
  connect(m_nextRadio, SIGNAL(toggled(bool)), this, SLOT(setNextState(bool)));
  connect(m_nextRadio, SIGNAL(toggled(bool)), this, SLOT(updatePreviewText()));
  QHBoxLayout *parallelLoadLayout = new QHBoxLayout;
  parallelLoadLayout->addWidget(m_loopRadio);
  parallelLoadLayout->addWidget(m_nextRadio);
  parallelLoadLayout->setMargin(4);
  m_parallelLoadGroup->setLayout(parallelLoadLayout);
  m_systemLayout->addWidget(m_parallelLoadGroup, row, 3, 2, 1);
  row++;
}

void GamessInputBuilder::createMOGuess()
{
}

void GamessInputBuilder::createMisc()
{
}

void GamessInputBuilder::createScf()
{
}

void GamessInputBuilder::okClicked()
{
}

void GamessInputBuilder::exportClicked()
{
  ofstream file("/tmp/gamessoutput.test", ios_base::out);
  m_inputData->WriteInputFile(file);
}

void GamessInputBuilder::cancelClicked()
{
}

// Basis Slots
void GamessInputBuilder::setBasisSetIndex( int index )
{
  int basis = 0;
  int gauss = 0;

  if(index < 2) {
    basis = index + 1;
  }
  else if(index < 7) {
    basis = 3;
    gauss = index;
  }
  else if(index < 9) {
    basis = 4;
    gauss = ((index==7) ? 3 : 6);
  }
  else if(index < 12) {
    basis = 5;
    gauss = index - 5;
  }
  else if(index < 18) {
    basis = index - 6;
    if(index == 12) gauss = 6;
  }
  else {
    basis = index - 6;
  }

  m_inputData->Basis->SetBasis(basis);
  m_inputData->Basis->SetNumGauss(gauss);

  updateWidgets();
}

void GamessInputBuilder::setECPTypeIndex( int index )
{
  m_inputData->Basis->SetECPPotential(index);
}

void GamessInputBuilder::setDHeavyAtomIndex( int index )
{
  m_inputData->Basis->SetNumDFuncs(index);
  updateWidgets();
}

void GamessInputBuilder::setFHeavyAtomIndex( int index )
{
  m_inputData->Basis->SetNumFFuncs(index);
  updateWidgets();
}

void GamessInputBuilder::setLightAtomIndex( int index )
{
  m_inputData->Basis->SetNumPFuncs(index);
  updateWidgets();
}

void GamessInputBuilder::setPolarIndex( int index )
{
  m_inputData->Basis->SetPolar((GAMESS_BS_Polarization)(index));
}

void GamessInputBuilder::setDiffuseLShellState ( bool state )
{
  m_inputData->Basis->SetDiffuseSP(state);
}

void GamessInputBuilder::setDiffuseSShellState ( bool state )
{
  m_inputData->Basis->SetDiffuseS(state);
}


// Control Slots
void GamessInputBuilder::setRunTypeIndex( int index )
{
  m_inputData->Control->SetRunType((TypeOfRun)(index+1));
  updateWidgets();
}

void GamessInputBuilder::setSCFTypeIndex( int index )
{
  m_inputData->Control->SetSCFType((GAMESS_SCFType)(index +1));
  //TODO: Enable SCF Tab
  updateWidgets();
}

void GamessInputBuilder::setLocalizationMethodIndex( int index )
{
  m_inputData->Control->SetLocal((GAMESS_Localization)index);
  updateWidgets();
}

void GamessInputBuilder::setExecTypeIndex( int index )
{
  m_inputData->Control->SetExeType(index);
}

void GamessInputBuilder::setMaxSCFText( const QString &text )
{
  m_inputData->Control->SetMaxIt(text.toInt());
}

void GamessInputBuilder::setMoleculeChargeText( const QString &text )
{
  m_inputData->Control->SetCharge(text.toInt());
}

void GamessInputBuilder::setMultiplicityText( const QString &text )
{
  m_inputData->Control->SetMultiplicity(text.toInt());
}

void GamessInputBuilder::setUseMP2State ( bool state )
{
  if(state)
    m_inputData->Control->SetMPLevel(2);
  else
    m_inputData->Control->SetMPLevel(0);

  updateWidgets();
}

void GamessInputBuilder::setUseDFTState ( bool state )
{
  m_inputData->Control->UseDFT(state);
  updateWidgets();
}

void GamessInputBuilder::setCIIndex( int index )
{
  m_inputData->Control->SetCIType((CIRunType)index);
  updateWidgets();
}

void GamessInputBuilder::setCCIndex( int index )
{
  m_inputData->Control->SetCCType((CCRunType)index);
  updateWidgets();
}


// Data Slots
void GamessInputBuilder::setTitleText( const QString &text )
{
  m_inputData->Data->SetTitle(text.toAscii().constData());
}

void GamessInputBuilder::setCoordinateTypeIndex( int index )
{
  m_inputData->Data->SetCoordType((CoordinateType)(index+1));
  updateWidgets();
}

void GamessInputBuilder::setUnitsIndex( int index )
{
  m_inputData->Data->SetUnits(index);
}

void GamessInputBuilder::setNumZMatrixText( const QString &text )
{
  m_inputData->Data->SetNumZVar(text.toInt());
}

void GamessInputBuilder::setPointGroupIndex( int index )
{
  m_inputData->Data->SetPointGroup((GAMESSPointGroup)(index+1));
  updatePointGroupOrderWidgets();
}

void GamessInputBuilder::setOrderPrincipleAxisIndex( int index )
{
  m_inputData->Data->SetPointGroupOrder(index+2);
}

void GamessInputBuilder::setUseSymmetryState ( bool state )
{
  m_inputData->Data->SetUseSym(state);
}


// System Slots
void GamessInputBuilder::setTimeLimitText( const QString &text )
{
  m_inputData->System->SetConvertedTime(text.toDouble());
}

void GamessInputBuilder::setTimeLimitIndex( int index )
{
  m_inputData->System->SetTimeUnits((TimeUnit)(index+1));
  QString str = QString::number(m_inputData->System->GetConvertedTime());
  m_timeLimitLine->setText(str);

}

void GamessInputBuilder::setMemoryText( const QString &text )
{
  m_inputData->System->SetConvertedMem(text.toFloat());
}

void GamessInputBuilder::setMemoryIndex( int index )
{
  m_inputData->System->SetMemUnits((MemoryUnit)(index+1));
  QString str = QString::number(m_inputData->System->GetConvertedMem());
  m_memoryLine->setText(str);
}

void GamessInputBuilder::setMemDDIText( const QString &text )
{
  m_inputData->System->SetConvertedMemDDI(text.toDouble());
}

void GamessInputBuilder::setMemDDIIndex( int index )
{
  m_inputData->System->SetMemDDIUnits((MemoryUnit)(index + megaWordsUnit));
}

void GamessInputBuilder::setProduceCoreState ( bool state )
{
  m_inputData->System->SetCoreFlag(state);
}

void GamessInputBuilder::setForceParallelState ( bool state )
{
  m_inputData->System->SetParallel(state);
}

void GamessInputBuilder::setDiagonalizationIndex( int index )
{
  m_inputData->System->SetDiag(index);
}

void GamessInputBuilder::setUseExternalDataState ( bool state )
{
  m_inputData->System->SetXDR(state);
}

void GamessInputBuilder::setLoopState ( bool state )
{
  m_inputData->System->SetBalanceType(0);
}

void GamessInputBuilder::setNextState ( bool state )
{
  m_inputData->System->SetBalanceType(1);
}

