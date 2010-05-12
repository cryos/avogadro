/*************************************************************************
  DaltonInputDialog - Dialog for generating Dalton input files

  Jogvan Magnus Olsen

  Initial source code was shamelessly copied from gaussianinputdialog.cpp

 *************************************************************************/

#ifndef DALTONINPUTDIALOG_H
#define DALTONINPUTDIALOG_H

#include "inputdialog.h"
#include "ui_daltoninputdialog.h"

namespace Avogadro
{
  class Molecule;
  class DaltonInputDialog : public InputDialog
  {
    Q_OBJECT

  public:
    explicit DaltonInputDialog(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~DaltonInputDialog();

    void setMolecule(Molecule *molecule);
    void readSettings(QSettings&);
    void writeSettings(QSettings&) const;

    enum calculationType{SP, PROP};
    enum theoryType{HF, DFT, MP2};
    enum functionalType{B2PLYP, B3LYP, B3LYPg, B3P86, B3P86g, B3PW91, B1LYP,
                        B1PW91, BHandH, BHandHLYP, B86VWN, B86LYP, B86P86,
                        B86PW91, BVWN, BLYP, BP86, BPW91, BW, BFW, CAMB3LYP,
                        DBLYP, DBP86, DBPW91, EDF1, EDF2, G96VWN, G96LYP,
                        G96P86, G96PW91, G961LYP, KMLYP, KT1, KT2, KT3, LDA,
                        LG1LYP, OVWN, OLYP, OP86, OPW91, mPWVWN, mPWLYP, mPWP86,
                        mPWPW91, mPW91, mPW1PW91, mPW3PW91, mPW1K, mPW1N, mPW1S,
                        PBE0, PBE0PBE, PBE1PBE, PBE, PBEPBE, RPBE, revPBE, mPBE,
                        PW91, PW91VWN, PW91LYP, PW91P86, PW91PW91, SVWN3, SVWN5,
                        XLYP, X3LYP};
    enum basisType{STOnG, pople, jensen, dunning};
    enum stoBasis{STO2G, STO3G, STO6G};
    enum popleBasis{p321G, p431G, p631G, p6311G};
    enum poplediffBasis{p321ppG, p631pG, p631ppG};
    enum poplepolBasis{p321Gs, p631Gs, p631Gss, p631G33, p6311Gs, p6311Gss,
                       p6311G22};
    enum poplediffpolBasis{p321ppGs, p631pGs, p631ppGs, p631ppGss, p6311pGs,
                           p6311ppGss, p6311ppG22, p6311ppG33};
    enum pcBasis{pc0, pc1, pc2, pc3, pc4};
    enum apcBasis{apc0, apc1, apc2, apc3, apc4};
    enum ccpvxzBasis{ccpVDZ, ccpVTZ, ccpVQZ, ccpV5Z, ccpV6Z};
    enum accpvxzBasis{accpVDZ, accpVTZ, accpVQZ, accpV5Z, accpV6Z};
    enum ccpcvxzBasis{ccpCVDZ, ccpCVTZ, ccpCVQZ, ccpCV5Z, ccpwCVDZ, ccpwCVTZ, 
                     ccpwCVQZ, ccpwCV5Z};
    enum accpcvxzBasis{accpCVDZ, accpCVTZ, accpCVQZ, accpCV5Z};
    enum xaugccBasis{sing, doub, trip, quad};
    enum dftGrid{coarse, normal, fine, ultrafine};
    enum propType{polari, exci};

  protected:
    /**
     * Reimplemented to update the dialog when it is shown
     */
    void showEvent(QShowEvent *event);

  private:
    Ui::DaltonInputDialog ui;
//    Molecule* m_molecule;

    // Internal data structure for the calculation
//    QString m_title;
    calculationType m_calculationType;
    theoryType m_theoryType;
    functionalType m_functionalType;
    basisType m_basisType;
    stoBasis m_stoBasis;
    popleBasis m_popleBasis;
    poplediffBasis m_poplediffBasis;
    poplepolBasis m_poplepolBasis;
    poplediffpolBasis m_poplediffpolBasis;
    pcBasis m_pcBasis;
    apcBasis m_apcBasis;
    ccpvxzBasis m_ccpvxzBasis;
    accpvxzBasis m_accpvxzBasis;
    ccpcvxzBasis m_ccpcvxzBasis;
    accpcvxzBasis m_accpcvxzBasis;
    xaugccBasis m_xaugccBasis;
    dftGrid m_dftGrid;
    propType m_propType;
    int m_exci;
    bool m_coreBasis;
    bool m_diffBasis;
    bool m_polBasis;
    bool m_directCheck;
    bool m_parallelCheck;
    bool m_dirty;
    bool m_warned;
    QString m_inputFile;

    // Generate an input deck as a string
    QString generateInputDeck();
    // Translate enums to strings
    QString getCalculationType(calculationType t);
    QString getTheoryType(theoryType t);
    QString getFunctionalType(functionalType t);
    QString getstoBasis(stoBasis t);
    QString getpopleBasis(popleBasis t);
    QString getpoplediffBasis(poplediffBasis t);
    QString getpoplepolBasis(poplepolBasis t);
    QString getpoplediffpolBasis(poplediffpolBasis t);
    QString getpcBasis(pcBasis t);
    QString getapcBasis(apcBasis t);
    QString getccpvxzBasis(ccpvxzBasis t);
    QString getaccpvxzBasis(accpvxzBasis t);
    QString getccpcvxzBasis(ccpcvxzBasis t);
    QString getaccpcvxzBasis(accpcvxzBasis t);
    QString getxaugccBasis(xaugccBasis t);
    QString getdftGrid(dftGrid t);
    QString getPropType(propType t);
    // Enable/disable form elements
    void deckDirty(bool);
    
    //QString saveInputFile();

  public Q_SLOTS:
    void updatePreviewText();

  private Q_SLOTS:
    //! Button Slots
    void resetClicked();
    void generateClicked();
    void moreClicked();
    void previewEdited();
    void setTitle();
    void setCalculation(int);
    void setTheory(int);
    void setFunctional(int);
    void setBasis(int);
    void setstoBasis(int);
    void setpopleBasis(int);
    void setpoplediffBasis(int);
    void setpoplepolBasis(int);
    void setpoplediffpolBasis(int);
    void setpcBasis(int);
    void setapcBasis(int);
    void setccpvxzBasis(int);
    void setaccpvxzBasis(int);
    void setccpcvxzBasis(int);
    void setaccpcvxzBasis(int);
    void setxaugccBasis(int);
    void setcoreBasis(int);
    void setdiffBasis(int);
    void setpolBasis(int);
    void setdirectCheck(int);
    void setparallelCheck(int);
    void setdftGrid(int);
    void setProp(int);
    void setExci(int);
  };
}

#endif
