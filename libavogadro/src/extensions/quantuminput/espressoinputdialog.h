/**********************************************************************
  EspressoInputDialog - Dialog for generating Espresso input decks

  Copyright (C) 2012 Albert DeFusco

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

#ifndef QESPRESSOINPUTDIALOG_H
#define QESPRESSOINPUTDIALOG_H

#include <QProcess>
#include <QProgressDialog>
#include <QTextBrowser>
#include <QHash>
#include <QSignalMapper>
#include <QFileDialog>

#include "inputdialog.h"
#include "ui_espressoinputdialog.h"

namespace Avogadro
{
  class Molecule;
  class EspressoInputDialog : public InputDialog
  {
    Q_OBJECT

  public:
    explicit EspressoInputDialog(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~EspressoInputDialog();

    void setMolecule(Molecule *molecule);

    //crystal structure tab
    enum crystalType{Display, Primitive, Manual};
    enum bravaisType{free, cubic, fcc, bcc,
      hexP, trigR, tetP, tetI, ortP, bco, 
      ortF, ortB, monP, monoB, tri};
    enum unitType{alat, bohr, ang};

    //calculation tab
    enum calcType{scf, nscf, bands, relax, md,
      vcrelax, vcmd};
    enum verboseType{high, def, low, min};
    enum restartType{scr, restart};
    enum smearType{gauss, mp, mv, fd};

    //pseudo tab
    //The pseudopotential file names are from the full
    //acrhive at http://www.quantum-espresso.org/pseudo/espresso_pp.tar.gz
    //and are listed in espressopseudo.h

    //k point tab
    enum kpointType{automatic, gamma};

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

    inline OpenBabel::OBUnitCell* currentCell() const {
      return (m_molecule) ? m_molecule->OBUnitCell() : 0 ;}

  protected:
    /**
     * Reimplemented to update the dialog when it is shown
     */
    void showEvent(QShowEvent *event);

  private:
    Ui::EspressoInputDialog ui;

    // Internal data structure for the calculation
    // Basic Tab
    QString m_title;

    //local types

    //crystal structure tab
    crystalType m_crystalType;
    bravaisType m_bravaisType;
    unitType m_unitType;
    int m_ibrav;
    double m_latticeA;
    double m_latticeB;
    double m_latticeC;
    double m_latticeAlpha;
    double m_latticeBeta;
    double m_latticeGamma;
    int numAtoms;
    QHash<int, int> atomTypes;
    QHash<int, int>::iterator itr;
    QHash<int, double> atomMass;

    //calculation tab
    calcType m_calcType;
    verboseType m_verboseType;
    restartType m_restartType;
    QString m_outdir;
    QString m_prefix;
    bool m_symm;
    bool m_autoBand;
    int m_bands;
    double m_charge;
    double m_cutoff;
    smearType m_smearType;
    double m_dgauss;
    bool m_stress;
    bool m_forces;
    double m_timeStep;
    int m_nSteps;

    //pseudo tab
    bool m_pseudoEnv;
    QString m_pseudodir;
    QHash<QString, QString> atomPseudo;
    QHash<QString, QStringList> atomPseudoFiles;

    //k point tab
    kpointType m_kpointType;
    int m_nKA;
    int m_nKB;
    int m_nKC;
    bool m_offKA;
    bool m_offKB;
    bool m_offKC;

    bool m_dirty;
    bool m_warned;

    // Generate an input deck as a string
    QString generateInputDeck();

    //crystal sctructure tab
    QString getCrystalStructure(crystalType t);
    QString generatePrimitiveLattice();
    QString getDisplayedCrystal();
    QString getManualLattice();
    QString getUnits(unitType t);
    double getUnitConv(unitType t);
    //calculation tab
    QString getCalcType(calcType t);
    QString getVerboseType(verboseType t);
    QString getRestartType(restartType t);
    QString getSmearType(smearType t);
    QString getNoSymm(bool t);
    QString getStress(bool t);
    QString getForces(bool t);
    //pseudo tab
    QSignalMapper *signalMapper;
    void setPseudoFiles();
    void initializePseudo(QString);
    void preparePseudoInput();

    //k point tab
    QString getKPointInput(kpointType t);

    QString getSystemNameList();
    QString getControlNameList();

    // Enable/disable form elements
    void deckDirty(bool);

  public Q_SLOTS:
    void updatePreviewText();

  private Q_SLOTS:
    //! Button Slots
    void resetClicked();
    void generateClicked();
    void enableFormClicked();
    void moreClicked();
    void previewEdited();

    void setTitle();

    //crystal structure tab
    void setCrystalStructure(int);
    void setLatticeA(double);
    void setLatticeB(double);
    void setLatticeC(double);
    void setLatticeAlpha(double);
    void setLatticeBeta(double);
    void setLatticeGamma(double);
    void setBravaisLattice(int);
    void setUnits(int);

    //calculation tab
    void setCalcType(int);
    void setVerbosity(int);
    void setRestartType(int);
    void setOutDir();
    void setPrefix();
    void setAutoBands(bool);
    void setBands(int);
    void setCharge(double);
    void setCutoff(double);
    void setSymmetry(bool);
    void setSmearType(int);
    void setDGauss(double);
    void setStress(bool);
    void setForces(bool);
    void setTimeStep(double);
    void setNSteps(int);

    //pseudo tab
    void setPseudoDir();
    void setPseudoType(int);
    void setPseudoEnv(bool);

    //k point tab
    void setKPointType(int);
    void setNKA(int);
    void setNKB(int);
    void setNKC(int);
    void setOffKA(bool);
    void setOffKB(bool);
    void setOffKC(bool);

  };
}

#endif
