/**********************************************************************
  GaussianInputDialog - Dialog for generating Gaussian input decks

  Copyright (C) 2010 Matthieu Verstraete

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef ABINITINPUTDIALOG_H
#define ABINITINPUTDIALOG_H

#include <QProcess>
#include <QProgressDialog>
#include <QTextBrowser>

#include "inputdialog.h"
#include "ui_abinitinputdialog.h"

namespace Avogadro
{
  class Molecule;    
  class AbinitInputDialog : public InputDialog
  {
    Q_OBJECT

  public:
    explicit AbinitInputDialog(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~AbinitInputDialog();

    void setMolecule(Molecule *molecule);

// which types do we want here?
// DFPT?
// geometry optimization ionmov?
// 
// gotten from Avogadro:
//  xred
//  acell 1 1 1 Angstr + rprim
//  znucl
//  typat ntypat
//  natom
//
    /* These are lists of symbols for use below */ 
    enum coordType{XANGST, XRED};
    enum toleranceType{TOLDFE, TOLWFR, TOLVRS, TOLDFF, TOLRFF};
    enum occoptType{SEMICOND, FERMI, COLD, GAUSSIAN};
    enum ionmovType{NOMOV, VISCOUS, BFGS, BGFSE};


    /**
     * Save the settings for this extension.
     * @param settings Settings variable to write settings to.
     */
    virtual void writeSettings(QSettings &settings) const;

    /**
     * Read the settings for this extension.
     * @param settings Settings variable to read settings from.
     */
    virtual void readSettings(QSettings &settings);

    QString pathToAbinit() const;

  protected:
    /**
     * Reimplemented to update the dialog when it is shown
     */
    void showEvent(QShowEvent *event);
    QString saveInputFile(QString inputDeck, QString fileType, QString ext);

  private:
    Ui::AbinitInputDialog ui;

    // Internal data structure for the calculation
    // Basic Tab
    QString m_title;
    double m_ecut;
    coordType  m_coordType;
    occoptType m_occopt;
    double m_tsmear;
    int m_procs;
    int m_ngkpt1, m_ngkpt2, m_ngkpt3;
    ionmovType m_ionmov;
    int m_nband;

    // Advanced Tab
    double m_tolmxf;
    double m_dilatmx;
    double m_ecutsm;
    int m_ntime;
    int m_nshiftk;
    double m_shiftk1;
    double m_shiftk2;
    double m_shiftk3;
    toleranceType m_toleranceType;
    double m_tolXXX;
    // int m_charge;
    // PAW Tab
    double m_pawecutdg;

    QList<int> m_znucl;

    bool m_dirty;
    bool m_warned;
    QProcess *m_process;
    QProgressDialog *m_progress;
    QTextBrowser *m_logWindow;
    QString m_inputFile;
    QString m_logFileName;

    // Generate an input deck as a string
    QString generateInputDeck();

    // Translate enums to strings
    int getOccopt(occoptType t);
    int getIonmov(ionmovType t);
    QString getCoordType(coordType t);
    QString getToleranceType(toleranceType t);

    // Enable/disable form elements
    void deckDirty(bool);

  public Q_SLOTS:
    void updatePreviewText();

  private Q_SLOTS:
    //! Button Slots
    void resetClicked();
    void generateClicked();
    void computeClicked();
    void enableFormClicked();
    void moreClicked();
    void previewEdited();
    // Basic tab
    void setTitle();
    void setEcut();
    void setCoordType(int);
    void setOccopt(int);
    void setTsmear();
    void setProcs(int);
    void setNgkpt1(int);
    void setNgkpt2(int);
    void setNgkpt3(int);
    void setIonmov(int);
    void setNband(int);

    // Advanced tab
    void setTolmxf();
    void setDilatmx();
    void setEcutsm();
    void setNtime(int);
    void setNshiftk(int);
    void setShiftk1();
    void setShiftk2();
    void setShiftk3();
    void setToleranceType(int);
    void setTolXXX();

    // PAW tab
    void setPawecutdg();

    void printLogOutput();
    void finished(int);
    void stopProcess();
  };
}

#endif
