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

using namespace Avogadro;

GamessInputBuilder::GamessInputBuilder(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
  m_mainLayout = new QVBoxLayout;
  setLayout(m_mainLayout);

  setWindowTitle(tr("GAMESS Input Deck Builder"));

  createTabs();
  createPreview();
  createButtons();
}

GamessInputBuilder::~GamessInputBuilder()
{
}

void GamessInputBuilder::createTabs()
{
  QWidget *widget;

  m_tabWidget = new QTabWidget(this);

  // Basis Set
  widget = new QWidget(this);
  m_basisLayout = new QGridLayout(this);
  widget->setLayout(m_basisLayout);
  m_tabWidget->addTab(widget, tr("Basis"));
  createBasis();

  widget = new QWidget(this);
  m_controlLayout = new QGridLayout(this);
  widget->setLayout(m_controlLayout);
  m_tabWidget->addTab(widget, tr("Control"));
  createControl();

  widget = new QWidget(this);
  m_dataLayout = new QGridLayout(this);
  widget->setLayout(m_dataLayout);
  m_tabWidget->addTab(widget, tr("Data"));
  createData();

  widget = new QWidget(this);
  m_systemLayout = new QGridLayout(this);
  widget->setLayout(m_systemLayout);
  m_tabWidget->addTab(widget, tr("System"));
  createSystem();

  widget = new QWidget(this);
  m_moGuessLayout = new QGridLayout(this);
  widget->setLayout(m_moGuessLayout);
  m_tabWidget->addTab(widget, tr("MO Guess"));
  createMOGuess();

  widget = new QWidget(this);
  m_miscLayout = new QGridLayout(this);
  widget->setLayout(m_miscLayout);
  m_tabWidget->addTab(widget, tr("misc"));
  createMisc();

  widget = new QWidget(this);
  m_scfLayout = new QGridLayout(this);
  widget->setLayout(m_scfLayout);
  m_tabWidget->addTab(widget, tr("SCF"));
  createScf();

  m_mainLayout->addWidget(m_tabWidget);
}

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
  m_basisLayout->addWidget(m_polarCombo, row, 1);


  // Diffuse L-Shell on Heavy Atoms
  row++;
  m_diffuseLShellCheck = new QCheckBox(tr("Diffuse L-Shell on Heavy Atoms"));
  m_basisLayout->addWidget(m_diffuseLShellCheck, row, 0, Qt::AlignCenter);

  // Diffuse S-Shell on Heavy Atoms
  m_diffuseSShellCheck = new QCheckBox(tr("Diffuse S-Shell on Heavy Atoms"));
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
  glayout->addWidget(m_ccCombo, 1, 1);
  m_controlLayout->addLayout(glayout, row, 2,2,3);

  row++;
  // Use DFT
  m_useDFTCheck = new QCheckBox(tr("Use DFT"));
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
  m_timeLimitCombo->setCurrentIndex(1);
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
  m_systemLayout->addWidget(m_memoryLine, row, 1);
  layout = new QHBoxLayout;
  m_memoryCombo = new QComboBox;
  m_memoryCombo->addItem(tr("Words"));
  m_memoryCombo->addItem(tr("Bytes"));
  m_memoryCombo->addItem(tr("MegaWords"));
  m_memoryCombo->addItem(tr("MegaBytes"));
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
  m_systemLayout->addWidget(m_memDDILine, row, 1);
  layout = new QHBoxLayout;
  m_memDDICombo = new QComboBox;
  m_memDDICombo->addItem(tr("MegaWords"));
  m_memDDICombo->addItem(tr("MegaBytes"));
  m_memDDICombo->addItem(tr("GigaWords"));
  m_memDDICombo->addItem(tr("GigaBytes"));
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
  layout = new QHBoxLayout;
  layout->addWidget(m_diagonalizationCombo);
  layout->addStretch(40);
  m_systemLayout->addLayout(layout, row, 2);
  
  row++;
  // Use External Data Representation for Messages
  m_useExternalDataCheck = new QCheckBox(tr("Use External Data Representation for Messages"));
  m_systemLayout->addWidget(m_useExternalDataCheck, row, 1, 1, 3, Qt::AlignCenter);

  row = 0;
  // Produce "core" file upon abort.
  m_produceCoreCheck  = new QCheckBox(tr("Produce \"core\" file upon abort"));
  m_systemLayout->addWidget(m_produceCoreCheck, row, 3, 1, 1);

  row++;
  // Force Parallel Methods
  m_forceParallelCheck = new QCheckBox(tr("Force Parallel Methods"));
  m_systemLayout->addWidget(m_forceParallelCheck, row, 3, 1, 1);
  row++;
  // Parallel Load Balance Type
  m_parallelLoadGroup = new QGroupBox(tr("Parallel Load Balance Type"));
  m_parallelLoadGroup->setFlat(true);
  m_loopRadio = new QRadioButton(tr("Loop"));
  m_nextRadio = new QRadioButton(tr("Next Value"));
  QHBoxLayout *parallelLoadLayout = new QHBoxLayout;
  parallelLoadLayout->addWidget(m_loopRadio);
  parallelLoadLayout->addWidget(m_nextRadio);
  parallelLoadLayout->setMargin(4);
  m_parallelLoadGroup->setLayout(parallelLoadLayout);
  m_systemLayout->addWidget(m_parallelLoadGroup, row, 3, 2, 1);
  row++;


//   // Divider
//   row++;
//   divider = new QFrame();
//   divider->setFrameShape(QFrame::HLine);
//   divider->setFrameShadow(QFrame::Sunken);
//   divider->setLineWidth(0);
//   divider->setMidLineWidth(0);
//   m_systemLayout->addWidget(divider, row, 0, 1, 4);

//   // Divider
//   row++;
//   divider = new QFrame();
//   divider->setFrameShape(QFrame::HLine);
//   divider->setFrameShadow(QFrame::Sunken);
//   divider->setLineWidth(0);
//   divider->setMidLineWidth(0);
//   m_systemLayout->addWidget(divider, row, 0, 1, 4);


//   // Divider
//   row++;
//   divider = new QFrame();
//   divider->setFrameShape(QFrame::HLine);
//   divider->setFrameShadow(QFrame::Sunken);
//   divider->setLineWidth(0);
//   divider->setMidLineWidth(0);
//   m_systemLayout->addWidget(divider, row, 0, 0, 4);


//
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

void GamessInputBuilder::createPreview()
{
}

void GamessInputBuilder::createButtons()
{
}
