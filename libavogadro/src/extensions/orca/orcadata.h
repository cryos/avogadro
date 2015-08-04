/**********************************************************************
  OrcaData - Data Class Definition

  Copyright (C) 2014 Dagmar Lenk

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#ifndef ORCADATA_H
#define ORCADATA_H

#include <QtGui/QDialog>
#include <QtCore/QSettings>

#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>

#include <QString>
#include <QTextStream>
#include <QMetaEnum>

#include "orcaextension.h"

// Forward declaration of Avogadro::Molecule
namespace Avogadro {
class Molecule;
class OrcaExtension;
}

namespace Avogadro {

enum calculationType {SP, OPT, FREQ};
enum methodType {RHF, DFT, MP2, CCSD};
enum relType {ZORA, IORA, DKH};
enum accType {NORMALSCF, TIGHTSCF, VERYTIGHTSCF, EXTREMESCF};
enum scfType {RKS, UKS};
enum convType {DIIS, KDIIS};
enum conv2ndType {SOSCF, NRSCF, AHSCF};


enum coordType {CARTESIAN, ZMATRIX, ZMATRIX_COMPACT };
enum printType {NOTHING, MINI, SMALL, NORMAL, LARGE};


// Ocra Vibration class

class OrcaVibrations {
public:
    OrcaVibrations();
    ~OrcaVibrations();

    void setFrequencies(std::vector<double> vec);
    std::vector<double> frequencies();

    void setIntensities (std::vector<double> vec);
    std::vector<double> intensities();

    void setRaman (std::vector<double> vec);
    std::vector<double> raman();

    void setModes (std::vector<int> nvec);
    std::vector<int> modes();

    void setDisplacement (std::vector<std::vector<Eigen::Vector3d> *> vec);
    const std::vector<std::vector<Eigen::Vector3d> *> &displacement() const;

    void setOK(bool toggle) { m_dataOK = toggle;}
    bool checkOK () {return m_dataOK;}
private:

    std::vector<int> m_modes;
    std::vector<double> m_intensities;
    std::vector<double> m_frequencies;
    std::vector<double> m_raman;
    std::vector<std::vector<Eigen::Vector3d> *> m_displacement;

    bool m_dataOK;

};


// Orca Basic Data Class

class OrcaBasicData {
public:
    OrcaBasicData();
    ~OrcaBasicData () {}

    // Comment

    void setComment (QString comment);
    QString getComment ();

    // Calculation Type

    void setCalculation (int n) {m_calculationType = calculationType (n);}
    void setCalculation (calculationType n) { m_calculationType = n;}
    calculationType getCalculation() {return m_calculationType;}
    QString getCalculationTxt();

    // Method Type

    void setMethod (int n) {m_methodType = methodType (n);}
    void setMethod (methodType n) {m_methodType = n;}
    methodType getMethod () {return m_methodType;}
    QString getMethodTxt();

    // Basis Set

    void setBasis (int n) {m_basisType = OrcaExtension::basisType (n);}
    void setBasis (OrcaExtension::basisType n) {m_basisType = n;}
    OrcaExtension::basisType getBasis() {return m_basisType;}
    QString getBasisTxt();
    void setEnumBasis(QMetaEnum m) {m_enumBasis = m;}

    // Multiplicity

    void setMultiplicity (int n){ m_multiplicity = n;}
    int getMultiplicity () {return m_multiplicity;}

    // Charge

    void setCharge (int n) {m_charge = n;}
    int getCharge () {return m_charge;}

    // Coordinates Format

    void setFormat (int n) {m_coordsType = coordType (n);}
    void setFormat(coordType n) {m_coordsType = n;}
    coordType getFormat () {return m_coordsType;}
    QString getFormatTxt();

    // reset to default values

    void reset();

private:
    calculationType m_calculationType;
    QString m_comment;
    methodType m_methodType;
    OrcaExtension::basisType m_basisType;
    int m_multiplicity;
    int m_charge;
    coordType m_coordsType;

    QMetaEnum m_enumBasis;
};

class OrcaBasisData {
public:
    OrcaBasisData();
    ~OrcaBasisData () {}

    // Basis Set

    void setBasis (int n){m_basis = OrcaExtension::basisType (n);}
    void setBasis (OrcaExtension::basisType n){m_basis = n;}
    OrcaExtension::basisType getBasis() {return m_basis;}
    QString getBasisTxt();
    void setEnumBasis(QMetaEnum m) {m_enumBasis = m;}

    // Aux Basis Set

    void setAuxBasis (int n) {m_auxBasis = OrcaExtension::basisType (n);}
    void setAuxBasis (OrcaExtension::basisType n) {m_auxBasis = n;}
    OrcaExtension::basisType getAuxBasis() {return m_auxBasis;}
    QString getAuxBasisTxt();


    // Correlation Aux Basis Set (for MP2)

    void setAuxCorrBasis (int n) {m_auxCorrBasis = OrcaExtension::basisType (n);}
    void setAuxCorrBasis (OrcaExtension::basisType n) {m_auxCorrBasis = n;}
    OrcaExtension::basisType getAuxCorrBasis() {return m_auxCorrBasis;}
    QString getAuxCorrBasisTxt();


    // EPC
    void setEPCChecked (bool value) {m_useEPC = value;}
    bool EPCEnabled() {return m_useEPC;}

    // Aux EPC

    void setAuxEPCChecked (bool value) {m_useAuxEPC = value;}
    bool auxEPCEnabled() {return m_useAuxEPC;}


    // Correlation Aux EPC

    void setAuxCorrEPCChecked (bool value) {m_useAuxCorrEPC = value;}
    bool auxCorrEPCEnabled() {return m_useAuxCorrEPC;}

    // Relativistic

    void setRelChecked (bool n) {m_useRel = n;}
    bool relEnabled() {return m_useRel;}

    void setDKHChecked (bool n) {m_useDKH = n;}
    bool dkhEnabled () {return m_useDKH;}

    void setRel(int n) {m_rel = relType (n);}
    void setRel (relType n) {m_rel = n;}
    relType getRel () {return m_rel;}
    QString getRelTxt ();

    void setDKHOrder(int n){m_DKHOrder = n;}
    int getDKHOrder(){return m_DKHOrder;}

    // reset to default values

    void reset();

private:
    QMetaEnum m_enumBasis;

    OrcaExtension::basisType m_basis;
    OrcaExtension::basisType m_auxBasis;
    OrcaExtension::basisType m_auxCorrBasis;

    bool m_useEPC;
    bool m_useAuxEPC;
    bool m_useAuxCorrEPC;

    bool m_useRel;
    bool m_useDKH;
    relType m_rel;
    int m_DKHOrder;
};

class OrcaControlData {
public:
    OrcaControlData();
    ~OrcaControlData() {}

    // Calculation

    void setCalculation (int n) {m_calculationType = calculationType (n);}
    void setCalculation (calculationType n) {m_calculationType = n;}
    calculationType getCalculation() {return m_calculationType;}
    QString getCalculationTxt();

    // Multiplicity

    void setMultiplicity (int n){m_multiplicity = n;}
    int getMultiplicity () {return m_multiplicity;}

    // Charge

    void setCharge (int n) {m_charge = n;}
    int getCharge () {return m_charge;}

    // RijCosX

    void setCosXChecked (bool value) {m_useCosX = value;}
    bool cosXEnabled () {return m_useCosX;}

    // DFT || MP2 || CCSD

    void setDFTChecked (bool value) {m_useDFT = value;}
    bool dftEnabled () {return m_useDFT;}

    void setMP2Checked (bool value) {m_useMP2 = value;}
    bool mp2Enabled () {return m_useMP2;}

    void setCCSDChecked (bool value) {m_useCCSD = value;}
    bool ccsdEnabled () {return m_useCCSD;}

    // reset to default values

    void reset();

private:
    calculationType m_calculationType;
    int m_multiplicity;
    int m_charge;
    bool m_useCosX;
    bool m_useDFT;
    bool m_useMP2;
    bool m_useCCSD;
};
class OrcaSCFData {
public:
    OrcaSCFData();
    ~OrcaSCFData () {}

    void setAccuracy (int n) {m_accuracy = accType (n);}
    void setAccuracy(accType n) {m_accuracy = n;}
    accType getAccuracy () {return m_accuracy;}
    QString getAccuracyTxt();

    void setType (int n) {m_scfType = scfType (n);}
    scfType getType () {return m_scfType;}
    QString getTypeTxt ();

    void setMaxIter (int n) {m_maxIter = n;}
    int getMaxIter () {return m_maxIter;}

    void setConv (int n) {m_conv = convType (n);}
    void setConv(convType n) {m_conv = n;}
    convType getConv () {return m_conv;}

    void setConv2nd (int n) {m_2ndConv = conv2ndType (n);}
    void setConv2nd(conv2ndType n) {m_2ndConv = n;}
    conv2ndType getConv2nd () {return m_2ndConv;}

    void setLevelShift(qreal f) {m_levelShift = f;}
    qreal getLevelShift () {return m_levelShift;}

    void setDampFactor(qreal f) {m_dampFactor = f;}
    qreal getDampFactor () {return m_dampFactor;}


    void setLevelError(qreal f) {m_levelError = f;}
    qreal getLevelError () {return m_levelError;}

    void setDampError(qreal f) {m_dampError = f;}
    qreal getDampError () {return m_dampError;}


    void setLevelShiftChecked (bool value) {m_useLevel = value;}
    bool levelShiftEnabled () {return m_useLevel;}

    void setDampingChecked (bool value) {m_useDamping = value;}
    bool dampingEnabled () {return m_useDamping;}


    // reset to default values

    void reset();

private:

    accType m_accuracy;
    scfType m_scfType;

    convType m_conv;
    conv2ndType m_2ndConv;

    int   m_maxIter;
    qreal m_levelShift;
    qreal m_levelError;
    qreal m_dampFactor;
    qreal m_dampError;

    bool m_useLevel;
    bool m_useDamping;

};

class OrcaDFTData {
public:
    OrcaDFTData();
    ~OrcaDFTData() {}

    void setGrid (int n) {m_grid = OrcaExtension::gridType(n);}
    void setGrid (OrcaExtension::gridType n) {m_grid = n;}
    OrcaExtension::gridType getGrid () {return m_grid;}
    QString getGridTxt();
    void setEnumGrid(QMetaEnum m){ m_enumGrid = m;}

    void setFinalGrid (int n) {m_finalGrid = OrcaExtension::finalgridType (n);}
    void setGrid (OrcaExtension::finalgridType n) {m_finalGrid = n;}
    OrcaExtension::finalgridType getFinalGrid () {return m_finalGrid;}
    QString getFinalGridTxt();
    void setEnumFinalGrid(QMetaEnum m){ m_enumFinalGrid = m;}

    void setDFTFunctional(int n) { m_DFTFuncional = OrcaExtension::DFTFunctionalType (n);}
    void setDFTFunctional(OrcaExtension::DFTFunctionalType n) {m_DFTFuncional = n;}
    OrcaExtension::DFTFunctionalType getDFTFunctional () {return m_DFTFuncional;}
    QString getDFTFunctionalTxt();
    void setEnumDFT(QMetaEnum m) {m_enumDFT = m;}

    // reset to default values

    void reset();

private:

    OrcaExtension::gridType m_grid;
    OrcaExtension::finalgridType m_finalGrid;

    OrcaExtension::DFTFunctionalType m_DFTFuncional;
    QMetaEnum m_enumDFT;
    QMetaEnum m_enumGrid;
    QMetaEnum m_enumFinalGrid;

};
class OrcaCosXData {
public:
    OrcaCosXData ();
    ~OrcaCosXData() {}


    void setGrid (int n) {m_gridX = OrcaExtension::gridType(n);}
    void setGrid (OrcaExtension::gridType n) {m_gridX = n;}
    OrcaExtension::gridType getGrid () {return m_gridX;}
    QString getGridTxt();
    void setEnumGridX(QMetaEnum m){ m_enumGridX = m;}

    void setFinalGrid (int n) {m_finalGridX = OrcaExtension::finalgridType (n);}
    void setGrid (OrcaExtension::finalgridType n) {m_finalGridX = n;}
    OrcaExtension::finalgridType getFinalGrid () {return m_finalGridX;}
    QString getFinalGridTxt();
    void setEnumFinalGridX(QMetaEnum m){ m_enumFinalGridX = m;}

    void setSFittingChecked (bool value) {m_useSFitting = value;}
    bool sFittingEnabled () {return m_useSFitting;}

    // reset to default values

    void reset();

private:
     OrcaExtension::gridType m_gridX;
     OrcaExtension::finalgridType m_finalGridX;

     QMetaEnum m_enumGridX;
     QMetaEnum m_enumFinalGridX;
    bool m_useSFitting;
};

class OrcaDataData {
public:
    OrcaDataData();
    ~OrcaDataData () {}

    // Comment

    void setComment (QString comment) { m_comment = comment;}
    QString getComment() {return m_comment;}

    // Coordinates Format

    void setFormat (int n) {m_coordsType = coordType (n);}
    void setFormat(coordType n) {m_coordsType = n;}
    coordType getFormat () {return m_coordsType;}
    QString getFormatTxt();

    // print options

    void setPrintLvl (int n) {m_printLevel = printType (n);}
    void setPrintLvl(printType n) {m_printLevel = n;}
    printType getPrintLevel () {return m_printLevel;}
    QString getPrintLevelTxt();

    void setMOPrintChecked(bool value) {m_MOPrint = value;}
    bool MOPrintEnabled() { return m_MOPrint;}

    void setBasisPrintChecked(bool value) {m_basisPrint = value;}
    bool basisPrintEnabled() { return m_basisPrint;}

    // reset to default values

    void reset();

private:
    QString m_comment;
    coordType m_coordsType;
    printType m_printLevel;
    bool m_MOPrint;
    bool m_basisPrint;
};

class OrcaData {
public:
    //! Constructors
    OrcaData(Molecule *molecule = 0);
    OrcaData(OrcaData *Copy);

    //! Deconstructors
    ~OrcaData(void);

    Molecule              *m_molecule;
    OrcaBasicData   *Basic;
    OrcaControlData	*Control;
};

}
#endif
