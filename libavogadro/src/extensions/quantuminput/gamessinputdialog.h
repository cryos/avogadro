#ifndef GAMESSINPUTDIALOG_H
#define GAMESSINPUTDIALOG_H
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

#include <QDialog>
#include <QButtonGroup>
#include <QModelIndex>

#include "ui_gamessinputdialog.h"
#include "gamessinputdata.h"

namespace Avogadro
{
  class GamessHighlighter;

  class GamessInputDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit GamessInputDialog( GamessInputData *inputData, QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Desconstructor
      ~GamessInputDialog();

      void setInputData ( GamessInputData *inputData );
      void readSettings();
      void writeSettings();

    protected:
      //! Create dialog tabs
      void connectModes();
      void connectBasic();
      void connectAdvanced();
      void connectPreview();
      void connectButtons();

      void connectBasis();
      void connectControl();
      void connectData();
      void connectSystem();
      void connectMOGuess();
      void connectMisc();
      void connectSCF();
      void connectMP2();
      void connectDFT();
      void connectHessian();
      void connectStatPoint();


    private:
      Ui::GamessInputDialog ui;
      QString m_savePath;
      GamessInputData *m_inputData;
      GamessHighlighter *m_highlighter;

      bool m_advancedChanged;

      QButtonGroup *m_miscForceButtons;
      QButtonGroup *m_statPointHessianButtons;
      //! Mode Pointers

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
       * some cases that may mean that a change does not propigate back to our
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
      void generateClicked();

      void navigationItemClicked( const QModelIndex &index );
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
      void setBasicCalculate( int index );
      void setBasicWithLeft( int index );
      void setBasicWithRight( int index );
      void setBasicIn( int index );
      void setBasicOnLeft( int index );
      void setBasicOnRight( int index );

      //! Basis Slots
      void setBasisSet( int index );
      void setBasisECP( int index );
      void setBasisD( int val );
      void setBasisF( int val );
      void setBasisLight( int val );
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
      void setDataZMatrix( int val );
      void setDataPoint( int index );
      void setDataOrder( int index );
      void setDataSymmetry( bool state );

      //! System Slots;
      void setTimeLimit( double val );
      void setTimeLimitUnits( int index );
      void setSystemMemory( double val );
      void setSystemMemoryUnits( int index );
      void setSystemDDI( double val );
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
      void setMP2Electrons( int val );
      void setMP2Memory( int val );
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
      void setHessianDisplacement( double val );
      void setHessianDouble( bool state );
      void setHessianPurify( bool state );
      void setHessianPrint( bool state );
      void setHessianVibrational( bool state );
      void setHessianScale( double val );

      //! Stat Point Slots
      void setStatPointSteps( int val );
      void setStatPointRecalculate( int val );
      void setStatPointConvergence( double val );
      void setStatPointInitial( double val );
      void setStatPointMin( double val );
      void setStatPointMax( double val );
      void setStatPointOptimization( int index );
      void setStatPointUpdate( bool state );
      void setStatPointStationary( bool state );
      void setStatPointHessian( int value );
      void setStatPointPrint( bool state );
      void setStatPointJump( double val );
      void setStatPointFollow( int val );
  };
}

#endif // GAMESSINPUTDIALOG_H
