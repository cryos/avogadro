/**********************************************************************
  GamessInputBuilder - GAMESS Input Deck Builder for Avogadro

  Copyright (C) 2006, 2007 by Donald Ephraim Curtis
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
#include <QFrame>
#include <QCheckBox>
#include <QGroupBox>
#include <QButtonGroup>
#include <QLineEdit>
#include <QRadioButton>
#include <QGridLayout>
#include <QStackedLayout>
#include <QTextEdit>

#include "gamessinputdata.h"

namespace Avogadro
{
  class GamessInputBuilder : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      GamessInputBuilder( GamessInputData *inputData, QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Desconstructor
      ~GamessInputBuilder();

      void setInputData ( GamessInputData *inputData );

    protected:
      //! Create dialog tabs
      void createModes();
      void createBasic();
      void createAdvanced();
      void createPreview();
      void createButtons();

      void createBasis();
      void createControl();
      void createData();
      void createSystem();
      void createMOGuess();
      void createMisc();
      void createSCF();
      void createMP2();
      void createDFT();
      void createHessian();
      void createStatPoint();


    private:
      Ui::GamessInputBuilder ui;
      GamessInputData *m_inputData;

//       GamessInputData *m_inputBackup;
      bool m_advancedChanged;
//       bool m_previewChanged;
//       QString m_previewBackup;

      QVBoxLayout *m_mainLayout;
      QStackedLayout *m_stackedLayout;
      QFrame *m_basicWidget;
      QTabWidget  *m_advancedWidget;

      QGridLayout *m_basisLayout;
      // QWidget *m_basisWidget;
      int m_basisIndex;
      QGridLayout *m_controlLayout;
      // QWidget *m_controlWidget;
      int m_controlIndex;
      QGridLayout *m_dataLayout;
      // QWidget *m_dataWidget;
      int m_dataIndex;
      QGridLayout *m_systemLayout;
      // QWidget *m_systemWidget;
      int m_systemIndex;
      QGridLayout *m_moGuessLayout;
      // QWidget *m_moGuessWidget;
      int m_moGuessIndex;
      QGridLayout *m_dftLayout;
      // QWidget *m_dftWidget;
      int m_dftIndex;
      QGridLayout *m_mp2Layout;
      // QWidget *m_mp2Widget;
      int m_mp2Index;
      QGridLayout *m_hessianLayout;
      // QWidget *m_hessianWidget;
      int m_hessianIndex;
      QGridLayout *m_statPointLayout;
      // QWidget *m_statPointWidget;
      int m_statPointIndex;
      QGridLayout *m_miscLayout;
      // QWidget *m_miscWidget;
      int m_miscIndex;
      QGridLayout *m_scfLayout;
      // QWidget *m_scfWidget;
      int m_scfIndex;

      QTextEdit   *m_previewText;

      //! Mode Pointers
      QHBoxLayout *m_modeLayout;
      QButtonGroup *m_modeButtons;
      QPushButton *m_modeBasicButton;
      QPushButton *m_modeAdvancedButton;

      QHBoxLayout *m_buttonLayout;

      //! Basic Widgets
      QLabel    *m_basicCalculateLabel;
      QComboBox *m_basicCalculateCombo;
      QLabel    *m_basicWithLabel;
      QComboBox *m_basicWithLeftCombo;
      QComboBox *m_basicWithRightCombo;
      QLabel    *m_basicInLabel;
      QComboBox *m_basicInCombo;
      QLabel    *m_basicOnLabel;
      QComboBox *m_basicOnLeftCombo;
      QComboBox *m_basicOnRightCombo;
      QLabel    *m_basicTimeLabel;
      QLineEdit *m_basicTimeLine;
      QLabel    *m_basicTimeUnitsLabel;
      QLabel    *m_basicMemoryLabel;
      QLineEdit *m_basicMemoryLine;
      QLabel    *m_basicMemoryUnitsLabel;

      //! Basis Widgets
      QLabel    *m_basisSetLabel;
      QComboBox *m_basisSetCombo;
      QLabel    *m_basisECPLabel;
      QComboBox *m_basisECPCombo;
      QLabel    *m_basisDLabel;
      QComboBox *m_basisDCombo;
      QLabel    *m_basisFLabel;
      QComboBox *m_basisFCombo;
      QLabel    *m_basisLightLabel;
      QComboBox *m_basisLightCombo;
      QLabel    *m_basisPolarLabel;
      QComboBox *m_basisPolarCombo;
      QCheckBox *m_basisDiffuseLCheck;
      QCheckBox *m_basisDiffuseSCheck;

      //! Control Widgets
      QLabel    *m_controlRunLabel;
      QComboBox *m_controlRunCombo;
      QLabel    *m_controlSCFLabel;
      QComboBox *m_controlSCFCombo;
      QLabel    *m_controlLocalizationLabel;
      QComboBox *m_controlLocalizationCombo;
      QLabel    *m_controlChargeLabel;
      QLineEdit *m_controlChargeLine;
      QLabel    *m_controlMultiplicityLabel;
      QLineEdit *m_controlMultiplicityLine;
      QLabel    *m_controlExecLabel;
      QComboBox *m_controlExecCombo;
      QLabel    *m_controlMaxSCFLabel;
      QLineEdit *m_controlMaxSCFLine;
      QLabel    *m_controlCILabel;
      QComboBox *m_controlCICombo;
      QLabel    *m_controlCCLabel;
      QComboBox *m_controlCCCombo;
      QCheckBox *m_controlMP2Check;
      QCheckBox *m_controlDFTCheck;

      //! Data Widgets
      QLabel    *m_dataTitleLabel;
      QLineEdit *m_dataTitleLine;
      QLabel    *m_dataCoordinateLabel;
      QComboBox *m_dataCoordinateCombo;
      QLabel    *m_dataUnitsLabel;
      QComboBox *m_dataUnitsCombo;
      QLabel    *m_dataZMatrixLabel;
      QLineEdit *m_dataZMatrixLine;
      QLabel    *m_dataPointLabel;
      QComboBox *m_dataPointCombo;
      QLabel    *m_dataOrderLabel;
      QComboBox *m_dataOrderCombo;
      QCheckBox *m_dataSymmetryCheck;

      //! System Widgets
      QLabel    *m_systemTimeLabel;
      QLineEdit *m_systemTimeLine;
      QComboBox *m_systemTimeUnitsCombo;
      QLabel    *m_systemMemoryLabel;
      QLineEdit *m_systemMemoryLine;
      QComboBox *m_systemMemoryUnitsCombo;
      QLabel    *m_systemDDILabel;
      QLineEdit *m_systemDDILine;
      QComboBox *m_systemDDICombo;
      QLabel    *m_systemDiagnalizationLabel;
      QComboBox *m_systemDiagnalizationCombo;
      QCheckBox *m_systemCoreCheck;

      QGroupBox *m_systemParallelGroup;
      QRadioButton *m_systemLoopRadio;
      QRadioButton *m_systemNextRadio;
      QCheckBox *m_systemExternalCheck;
      QCheckBox *m_systemForceCheck;

      //! MO Guess Widgets
      QLabel    *m_moGuessInitialLabel;
      QComboBox *m_moGuessInitialCombo;
      QCheckBox *m_moGuessPrintCheck;
      QCheckBox *m_moGuessRotateCheck;

      //! Misc Widgets
      QGroupBox    *m_miscInterfaceGroup;
      QCheckBox    *m_miscMolPltCheck;
      QCheckBox    *m_miscPltOrbCheck;
      QCheckBox    *m_miscAIMPACCheck;
      QCheckBox    *m_miscRPACKCheck;
      QGroupBox    *m_miscForceGroup;
      QButtonGroup *m_miscForceButtons;
      QRadioButton *m_miscNoneRadio;
      QRadioButton *m_miscHondoRadio;
      QRadioButton *m_miscMeldfRadio;
      QRadioButton *m_miscGamessRadio;
      QRadioButton *m_miscGaussianRadio;
      QRadioButton *m_miscAllRadio;
      QCheckBox    *m_miscWaterCheck;

      //! SCF Widgets
      QCheckBox   *m_scfDirectCheck;
      QCheckBox   *m_scfComputeCheck;
      QCheckBox   *m_scfGenerateCheck;

      //! DFT Widgets
      QLabel    *m_dftMethodLabel;
      QComboBox *m_dftMethodCombo;
      QLabel    *m_dftFunctionalLabel;
      QComboBox *m_dftFunctionalCombo;

      //! MP2 Widgets
      QLabel    *m_mp2ElectronsLabel;
      QLineEdit *m_mp2ElectronsLine;
      QLabel    *m_mp2MemoryLabel;
      QLineEdit *m_mp2MemoryLine;
      QLabel    *m_mp2IntegralLabel;
      QLineEdit *m_mp2IntegralLine;
      QLabel    *m_mp2LocalizedLabel;
      QCheckBox *m_mp2LocalizedCheck;
      QLabel    *m_mp2ComputeLabel;
      QCheckBox *m_mp2ComputeCheck;
      QGroupBox *m_mp2TransformationGroup;
      QLabel       *m_mp2SegmentedLabel;
      QRadioButton *m_mp2SegmentedRadio;
      QLabel       *m_mp2TwoLabel;
      QRadioButton *m_mp2TwoRadio;
      QGroupBox *m_mp2StorageGroup;
      QLabel       *m_mp2DuplicatedLabel;
      QRadioButton *m_mp2DuplicatedRadio;
      QLabel       *m_mp2DistributedLabel;
      QRadioButton *m_mp2DistributedRadio;

      //! Hessian Widgets
      QGroupBox    *m_hessianMethodGroup;
      QRadioButton *m_hessianAnalyticRadio;
      QRadioButton *m_hessianNumericRadio;
      QLabel       *m_hessianDisplacementLabel;
      QLineEdit    *m_hessianDisplacementLine;
      QCheckBox    *m_hessianDoubleCheck;
      QCheckBox    *m_hessianPurifyCheck;
      QCheckBox    *m_hessianPrintCheck;
      QCheckBox    *m_hessianVibrationalCheck;
      QLabel       *m_hessianScaleLabel;
      QLineEdit    *m_hessianScaleLine;

      //! Stat Point Widgets
      QLabel       *m_statPointStepsLabel;
      QLineEdit    *m_statPointStepsLine;
      QLabel       *m_statPointConvergenceLabel;
      QLineEdit    *m_statPointConvergenceLine;
      QLabel       *m_statPointOptimizationLabel;
      QComboBox    *m_statPointOptimizationCombo;
      QGroupBox    *m_statPointStepGroup;
      QLabel       *m_statPointInitialLabel;
      QLineEdit    *m_statPointInitialLine;
      QLabel       *m_statPointMinLabel;
      QLineEdit    *m_statPointMinLine;
      QLabel       *m_statPointMaxLabel;
      QLineEdit    *m_statPointMaxLine;
      QLabel       *m_statPointRecalculateLabel;
      QLineEdit    *m_statPointRecalculateLine;
      QLabel       *m_statPointFollowLabel;
      QLineEdit    *m_statPointFollowLine;
      QLabel       *m_statPointJumpLabel;
      QLineEdit    *m_statPointJumpLine;
      QGroupBox    *m_statPointHessianGroup;
      QButtonGroup *m_statPointHessianButtons;
      QRadioButton *m_statPointGuessRadio;
      QRadioButton *m_statPointReadRadio;
      QRadioButton *m_statPointCalculateRadio;
      QCheckBox    *m_statPointPrintCheck;
      QCheckBox    *m_statPointUpdateCheck;
      QCheckBox    *m_statPointStationaryCheck;


    public Q_SLOTS:
      void setMode( int mode );

      void updatePreviewText();

      //! Update Basic Interface Widgets
      /*!
       * These functions are used to take the settings from the GamessInputData
       * class and updated all the functions.  Caution should be taken.  There
       * may be a case where an update causes the callbacks to happen which in
       * tern causes another update to happen and we get a circular loop.  It
       * may be that we need to disable signals for these updates.  However, in
       * some cases that may mean that a change does not propogate back to our
       * input data.  It is a consistency problem relating to the nature of
       * GAMESS input.
       */
      void updateBasicWidgets();

      //! Update Adavnced Interface Widgets
      void updateAdvancedWidgets();

      //! Update Basis Widgets
      void updateBasisWidgets();

      //! Update Control Widgets
      void updateControlWidgets();

      //! Update Data Widgets
      void updateDataWidgets();

      //! Update System Widgets
      void updateSystemWidgets();

      //! Update Point Group Order Widgets
      void updatePointGroupOrderWidgets();

      //! Update MO Guess Widgets
      void updateMOGuessWidgets();

      //! Update Misc Widgets
      void updateMiscWidgets();

      //! Update SCF Widgets
      void updateSCFWidgets();

      //! Update DFT Widgets
      void updateDFTWidgets();

      //! Update MP2 Widgets
      void updateMP2Widgets();

      //! Update Hessian Widgets
      void updateHessianWidgets();

      //! Update Stat Point Widgets
      void updateStatPointWidgets();

    private Q_SLOTS:
      //! Button Slots
      void defaultsClicked();
      void resetClicked();
      void exportClicked();
      void closeClicked();

      //! Block all Advanced Widgets
      void blockAdvancedSignals( bool block );

      //! Block Child Signals
      void blockChildrenSignals( QLayout *layout, bool block );
      void blockChildrenSignals( QObject *object, bool block );

      void setBasicDefaults();
      void setAdvancedDefaults();
      void setBasisDefaults();
      void setControlDefaults();
      void setDataDefaults();
      void setSystemDefaults();
      void setMOGuessDefaults();
      void setMiscDefaults();
      void setSCFDefaults();
      void setDFTDefaults();
      void setMP2Defaults();
      void setHessianDefaults();
      void setStatPointDefaults();

      void basicChanged();
      void advancedChanged();
//       void previewChanged();

      //! Basic Slots
      void setBasicCalculateIndex( int index );
      void setBasicWithLeftIndex( int index );
      void setBasicWithRightIndex( int index );
      void setBasicInIndex( int index );
      void setBasicOnLeftIndex( int index );
      void setBasicOnRightIndex( int index );
      void setBasicTimeLimitText( const QString &text );
      void setBasicMemoryText( const QString &text );

      //! Basis Slots
      void setBasisSetIndex( int index );
      void setBasisECP( int index );
      void setBasisD( int index );
      void setBasisF( int index );
      void setBasisLight( int index );
      void setBasisPolar( int index );
      void setBasisDiffuseL( bool state );
      void setBasisDiffuseS( bool state );

      //! Control Slots;
      void setControlRun( int index );
      void setControlSCF( int index );
      void setControlLocalization( int index );
      void setControlExec( int index );
      void setControlMaxSCF( const QString &text );
      void setControlCharge( const QString &text );
      void setControlMultiplicity( const QString &text );
      void setControlMP2( bool state );
      void setControlDFT( bool state );
      void setControlCI( int index );
      void setControlCC( int index );

      //! Data Slots;
      void setDataTitle( const QString &text );
      void setDataCoordinate( int index );
      void setDataUnits( int index );
      void setDataZMatrix( const QString &text );
      void setDataPoint( int index );
      void setDataOrder( int index );
      void setDataSymmetry( bool state );

      //! System Slots;
      void setSystemTime( const QString &text );
      void setSystemTimeUnits( int index );
      void setSystemMemory( const QString &text );
      void setSystemMemoryUnits( int index );
      void setSystemDDI( const QString &text );
      void setSystemDDIUnits( int index );
      void setSystemProduce( bool state );
      void setSystemForce( bool state );
      void setSystemDiagonalization( int index );
      void setSystemExternal( bool state );
      void setSystemLoop( bool state );
      void setSystemNext( bool state );


      //! MO Guess Slots
      void setMOGuessInitial( int index );
      void setMOGuessPrint( bool state );
      void setMOGuessRotate( bool state );

      //!! Misc Slots
      void setMiscMolPlt( bool state );
      void setMiscPltOrb( bool state );
      void setMiscAIMPAC( bool state );
      void setMiscRPAC( bool state );
      void setMiscForce( int id );
      void setMiscWater( bool state );

      //! SCF Slots
      void setSCFDirect( bool state );
      void setSCFCompute( bool state );
      void setSCFGenerate( bool state );

      //! DFT Slots
      void setDFTMethod( int index );
      void setDFTFunctional( int index );

      //! MP2 Slots
      void setMP2Electrons( const QString &text );
      void setMP2Memory( const QString &text );
      void setMP2Integral( const QString &text );
      void setMP2Localized( bool state );
      void setMP2Compute( bool state );
      void setMP2Segmented( bool state );
      void setMP2Two( bool state );
      void setMP2Duplicated( bool state );
      void setMP2Distributed( bool state );

      //! Hessian Slots
      void setHessianAnalytic( bool state );
      void setHessianNumeric( bool state );
      void setHessianDisplacement( const QString &text );
      void setHessianDouble( bool state );
      void setHessianPurify( bool state );
      void setHessianPrint( bool state );
      void setHessianVibrational( bool state );
      void setHessianScale( const QString &text );

      //! Stat Point Slots
      void setStatPointSteps( const QString &text );
      void setStatPointRecalculate( const QString &text );
      void setStatPointConvergence( const QString &text );
      void setStatPointInitial( const QString &text );
      void setStatPointMin( const QString &text );
      void setStatPointMax( const QString &text );
      void setStatPointOptimization( int index );
      void setStatPointUpdate( bool state );
      void setStatPointStationary( bool state );
      void setStatPointHessian( int value );
      void setStatPointPrint( bool state );
      void setStatPointJump( const QString &text );
      void setStatPointFollow( const QString &text );
  };
}

#endif
