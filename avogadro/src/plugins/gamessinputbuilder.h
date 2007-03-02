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

#ifndef __GAMESINPUTBUILDER_H
#define __GAMESINPUTBUILDER_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QGridLayout>
#include <QTextEdit>

#include "gamessinputdata.h"

namespace Avogadro {
  class GamessInputBuilder : public QDialog
  {
    Q_OBJECT

    public:
      //! Constructor
      GamessInputBuilder(GamessInputData *inputData, QWidget *parent = 0, Qt::WindowFlags f = 0);
      //! Desconstructor
      ~GamessInputBuilder();

    protected:
      //! Create dialog tabs
      void createTabs();
      void createPreview();
      void createButtons();

      void createBasis();
      void createControl();
      void createData();
      void createSystem();
      void createMOGuess();
      void createMisc();
      void createScf();

    private:
      GamessInputData *m_inputData;

      QVBoxLayout *m_mainLayout;
      QHBoxLayout *m_subLayout;
      QTabWidget *m_tabWidget;

      QGridLayout *m_basisLayout;
      QGridLayout *m_controlLayout;
      QGridLayout *m_dataLayout;
      QGridLayout *m_systemLayout;
      QGridLayout *m_moGuessLayout;
      QGridLayout *m_miscLayout;
      QGridLayout *m_scfLayout;

      QTextEdit   *m_previewText;

      QHBoxLayout *m_buttonLayout;

      // Basis Widgets
      QLabel    *m_basisSetLabel; 
      QComboBox *m_basisSetCombo;
      QLabel    *m_ecpTypeLabel; 
      QComboBox *m_ecpTypeCombo;
      QLabel    *m_dHeavyAtomLabel; 
      QComboBox *m_dHeavyAtomCombo;
      QLabel    *m_fHeavyAtomLabel; 
      QComboBox *m_fHeavyAtomCombo;
      QLabel    *m_lightAtomLabel; 
      QComboBox *m_lightAtomCombo;
      QLabel    *m_polarLabel; 
      QComboBox *m_polarCombo;
      QCheckBox *m_diffuseLShellCheck;
      QCheckBox *m_diffuseSShellCheck;

      // Control Widgets
      QLabel    *m_runTypeLabel;
      QComboBox *m_runTypeCombo;
      QLabel    *m_scfTypeLabel;
      QComboBox *m_scfTypeCombo;
      QLabel    *m_localizationMethodLabel;
      QComboBox *m_localizationMethodCombo;
      QLabel    *m_moleculeChargeLabel;
      QLineEdit *m_moleculeChargeLine;
      QLabel    *m_multiplicityLabel;
      QLineEdit *m_multiplicityLine;
      QLabel    *m_execTypeLabel;
      QComboBox *m_execTypeCombo;
      QLabel    *m_maxSCFLabel;
      QLineEdit *m_maxSCFLine;
      QLabel    *m_ciLabel;
      QComboBox *m_ciCombo;
      QLabel    *m_ccLabel;
      QComboBox *m_ccCombo;
      QCheckBox *m_useMP2Check;
      QCheckBox *m_useDFTCheck;

      // Data Widgets
      QLabel    *m_titleLabel;
      QLineEdit *m_titleLine;
      QLabel    *m_coordinateTypeLabel;
      QComboBox *m_coordinateTypeCombo;
      QLabel    *m_unitsLabel;
      QComboBox *m_unitsCombo;
      QLabel    *m_numZMatrixLabel;
      QLineEdit *m_numZMatrixLine;
      QLabel    *m_pointGroupLabel;
      QComboBox *m_pointGroupCombo;
      QLabel    *m_orderPrincipleAxisLabel;
      QComboBox *m_orderPrincipleAxisCombo;
      QCheckBox *m_useSymmetryCheck;

      // System Widgets
      QLabel    *m_timeLimitLabel;
      QLineEdit *m_timeLimitLine;
      QComboBox *m_timeLimitCombo;
      QLabel    *m_memoryLabel;
      QLineEdit *m_memoryLine;
      QComboBox *m_memoryCombo;
      QLabel    *m_memDDILabel;
      QLineEdit *m_memDDILine;
      QComboBox *m_memDDICombo;
      QLabel    *m_diagonalizationLabel;
      QComboBox *m_diagonalizationCombo;
      QCheckBox *m_produceCoreCheck;

      QGroupBox *m_parallelLoadGroup;
      QRadioButton *m_loopRadio;
      QRadioButton *m_nextRadio;
      QCheckBox *m_useExternalDataCheck;
      QCheckBox *m_forceParallelCheck;

    public Q_SLOTS:
      // Button Slots
      void okClicked();
      void exportClicked();
      void cancelClicked();

      void setDefaults();

      void updatePreviewText();

      void updateWidgets();
      void updateBasisWidgets();
      void updateControlWidgets();
      void updateDataWidgets();
      void updateSystemWidgets();
      void updatePointGroupOrderWidgets();

      // Basis Slots
      void setBasisSetIndex( int index );
      void setECPTypeIndex( int index );
      void setDHeavyAtomIndex( int index );
      void setFHeavyAtomIndex( int index );
      void setLightAtomIndex( int index );
      void setPolarIndex( int index );
      void setDiffuseLShellState( bool state );
      void setDiffuseSShellState( bool state );
      
      // Control Slots;
      void setRunTypeIndex( int index );
      void setSCFTypeIndex( int index );
      void setLocalizationMethodIndex( int index );
      void setExecTypeIndex( int index );
      void setMaxSCFText( const QString &text );
      void setMoleculeChargeText( const QString &text );
      void setMultiplicityText( const QString &text );
      void setUseMP2State ( bool state );
      void setUseDFTState ( bool state );
      void setCIIndex( int index );
      void setCCIndex( int index );
      
      // Data Slots;
      void setTitleText( const QString &text );
      void setCoordinateTypeIndex( int index );
      void setUnitsIndex( int index );
      void setNumZMatrixText( const QString &text );
      void setPointGroupIndex( int index );
      void setOrderPrincipleAxisIndex( int index );
      void setUseSymmetryState ( bool state );
      
      // System Slots;
      void setTimeLimitText( const QString &text );
      void setTimeLimitIndex( int index );
      void setMemoryText( const QString &text );
      void setMemoryIndex( int index );
      void setMemDDIText( const QString &text );
      void setMemDDIIndex( int index );
      void setProduceCoreState ( bool state );
      void setForceParallelState ( bool state );
      void setDiagonalizationIndex( int index );
      void setUseExternalDataState ( bool state );
      void setLoopState ( bool state );
      void setNextState ( bool state );

  };
}

#endif
