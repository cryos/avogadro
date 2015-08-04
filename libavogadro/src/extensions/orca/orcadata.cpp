/**********************************************************************
  OrcaData - Data Class Functions

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

#include <QtGui/QDialog>
#include <QtCore/QSettings>
#include <QtDebug>
#include "orcadata.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>


#include <QString>
#include <QTextStream>

using namespace Avogadro;

OrcaVibrations::OrcaVibrations ()
{
    m_dataOK = false;

    m_frequencies.resize(0);
    m_modes.resize(0);
    m_intensities.resize(0);
    m_raman.resize(0);
    m_displacement.resize(0);
}
OrcaVibrations::~OrcaVibrations()
{

}
void OrcaVibrations::setFrequencies(std::vector<double> freq)
{
        m_frequencies = freq;
}
std::vector<double> OrcaVibrations::frequencies()
{
    return m_frequencies;
}

void OrcaVibrations::setIntensities(std::vector<double> intens)
{
        m_intensities = intens;
}
std::vector<double> OrcaVibrations::intensities()
{
    return m_intensities;
}

void OrcaVibrations::setRaman(std::vector<double> raman)
{
        m_raman = raman;
}
std::vector<double> OrcaVibrations::raman()
{
    return m_raman;
}

void OrcaVibrations::setModes(std::vector<int> mode)
{
    m_modes = mode;
}
std::vector<int> OrcaVibrations::modes()
{
    return m_modes;
}

void OrcaVibrations::setDisplacement (std::vector<std::vector<Eigen::Vector3d> *> vec)
{
    m_displacement.resize(0);
    for (unsigned int i = 0; i < vec.size(); ++i) {
        m_displacement.push_back(vec[i]);
    }
}

const std::vector<std::vector<Eigen::Vector3d> *>& OrcaVibrations::displacement() const
{
    return (m_displacement);
}


OrcaBasicData::OrcaBasicData ()
{
    m_calculationType = SP;
    m_methodType = RHF;
    m_basisType = OrcaExtension::SVP;
    m_multiplicity = 1;
    m_charge = 0;
    m_coordsType = CARTESIAN;
}
void OrcaBasicData::reset ()
{
    m_calculationType = SP;
    m_methodType = RHF;
    m_basisType = OrcaExtension::SVP;
    m_multiplicity = 1;
    m_charge = 0;
    m_coordsType = CARTESIAN;
}
void OrcaBasicData::setComment(QString comment)
{
    m_comment = comment;
}
QString OrcaBasicData::getComment()
{
    return m_comment;
}


QString OrcaBasicData::getCalculationTxt()
{
    // Translate the enum calculation Type to normal text

    switch (m_calculationType)
      {
      case SP:
        return "SP";
      case OPT:
        return "OPT";
      case FREQ:
        return "OPT FREQ";
      default:
        return "";
      }
}


QString OrcaBasicData::getMethodTxt()
{

    // Translate the enum method types to normal text
    // enum methodType{RHF, DFT, MP2, CCSD}
    switch (m_methodType)
    {
    case RHF:
        return "RHF";
    case DFT:
        return "BP RI";
    case MP2:
        return "MP2";
    case CCSD:
        return "CCSD";
    default:
        return "";
    }

}

QString OrcaBasicData::getBasisTxt()
{
    // Translate the enum basis set to normal text
    // enum basisType {SVP, TZVP, TZVPP, QZVP }
    QString returnBasis = m_enumBasis.valueToKey(m_basisType);
    returnBasis.prepend("def2-");
    return returnBasis;
//    switch (m_basisType)
//    {
//    case SVP:
//        return "def2-SVP";
//    case TZVP:
//        return "def2-TZVP";
//    case TZVPP:
//        return "def2-TZVPP";
//    case QZVP:
//        return "def2-QZVP";
//    default:
//        return "";
//    }
}

QString OrcaBasicData::getFormatTxt()
{
    switch (m_coordsType)
    {
    case ZMATRIX:
        return "z-Matrix";
    case CARTESIAN:
        return "*xyz";
    case ZMATRIX_COMPACT:
        return "z-Matrix compact";
    default:
        return "";
    }
}

OrcaBasisData::OrcaBasisData()
{
     m_basis= OrcaExtension::SVP;
     m_auxBasis = OrcaExtension::SVP;
     m_auxCorrBasis = OrcaExtension::SVP;

     m_useEPC = false;
     m_useAuxEPC = false;
     m_useAuxCorrEPC = false;
     m_useRel = false;
     m_useDKH = false;
     m_rel = ZORA;
     m_DKHOrder = 0;
}
void OrcaBasisData::reset()
{
     m_basis= OrcaExtension::SVP;
     m_auxBasis = OrcaExtension::SVP;
     m_auxCorrBasis = OrcaExtension::SVP;
     m_useEPC = false;
     m_useAuxEPC = false;
     m_useAuxCorrEPC = false;
     m_useRel = false;
     m_useDKH = false;
     m_rel = ZORA;
     m_DKHOrder = 0;
}
QString OrcaBasisData::getBasisTxt()
{
    // Translate the enum basis set to normal text
    // enum basisType {SVP, TZVP, TZVPP, QZVP }

    QString returnBasis = m_enumBasis.valueToKey(m_basis);
    returnBasis.prepend("def2-");
    return returnBasis;

}


QString OrcaBasisData::getAuxBasisTxt()
{
    // Translate the enum basis set to normal text
    // used as auxilary basis
    // enum basisType {SVP, TZVP, TZVPP, QZVP }

    QString returnBasis = m_enumBasis.valueToKey(m_auxBasis);
    returnBasis.prepend("def2-");
    returnBasis.append("/J");
    return returnBasis;
}

QString OrcaBasisData::getAuxCorrBasisTxt()
{
    // Translate the enum basis set to normal text
    // used as correlation auxilary basis
    // enum basisType {SVP, TZVP, TZVPP, QZVP }

    QString returnBasis = m_enumBasis.valueToKey(m_auxCorrBasis);
    returnBasis.prepend("def2-");
    returnBasis.append("/C");
    return returnBasis;
}

QString OrcaBasisData::getRelTxt ()
{

    switch (m_rel)
    {
    case ZORA:
        return "ZORA";
    case IORA:
        return "IORA";
    case DKH:
        return "DKH";
    default:
        return "";
    }
}
OrcaControlData::OrcaControlData()
{
    m_multiplicity = 1;
    m_charge = 0;
    m_calculationType = SP;
    m_useCosX = false;
    m_useDFT = false;
    m_useMP2 = false;
    m_useCCSD = false;
//    m_useMDCI = false;
}
void OrcaControlData::reset()
{
    m_multiplicity = 1;
    m_charge = 0;
    m_calculationType = SP;
    m_useCosX = false;
    m_useDFT = false;
    m_useMP2 = false;
//    m_useMDCI = false;
}
QString OrcaControlData::getCalculationTxt()
{
    // Translate the enum calculation Type to normal text

    switch (m_calculationType)
      {
      case SP:
        return "SP";
      case OPT:
        return "OPT";
      case FREQ:
        return "OPT FREQ";
      default:
        return "";
      }
}

OrcaDFTData::OrcaDFTData()
{
    m_grid = OrcaExtension::Grid4;
    m_finalGrid = OrcaExtension::fDefault;
    m_DFTFuncional = OrcaExtension::BP;

}
void OrcaDFTData::reset()
{
    m_grid = OrcaExtension::Grid4;
    m_finalGrid = OrcaExtension::fDefault;
    m_DFTFuncional = OrcaExtension::BP;

}
QString OrcaDFTData::getDFTFunctionalTxt()
{
    // Translate the enum DFTFunctional Type to normal text
//     enum OrcaExtension::DFTFunctionalType {LDA, BP, BLYP, PW91, B3LYP, B3PW, PBEO, TPSS, TPSSH, M06L};

    return m_enumDFT.valueToKey(m_DFTFuncional);
}
QString OrcaDFTData::getGridTxt()
{
    QString returnGrid = m_enumGrid.valueToKey(m_grid);
    returnGrid.replace("None", "NoGrid");
    if (returnGrid.contains("Default")) returnGrid = "";
    return returnGrid;
}

QString OrcaDFTData::getFinalGridTxt()
{
    QString returnGrid = m_enumFinalGrid.valueToKey(m_finalGrid);

    returnGrid.replace("fGrid", "FinalGrid");
    returnGrid.replace("fNone", "NoFinalGrid");
    if (returnGrid.contains("fDefault")) returnGrid = "";
    return returnGrid;
}


OrcaCosXData::OrcaCosXData()
{
    m_gridX = OrcaExtension::Grid4;
    m_finalGridX = OrcaExtension::fDefault;

    m_useSFitting = false;

}
void OrcaCosXData::reset()
{
    m_gridX = OrcaExtension::Grid4;
    m_finalGridX = OrcaExtension::fDefault;

    m_useSFitting = false;

}
QString OrcaCosXData::getGridTxt()
{
    QString returnGrid = m_enumGridX.valueToKey(m_gridX);
    returnGrid.replace("Grid", "GridX");
    returnGrid.replace("None", "NoGridX");
    if (returnGrid.contains("Default")) returnGrid = "";
    return returnGrid;
}

QString OrcaCosXData::getFinalGridTxt()
{
    QString returnGrid = m_enumFinalGridX.valueToKey(m_finalGridX);

    returnGrid.replace("fGrid", "FinalGridX");
    returnGrid.replace("fNone", "NoFinalGridX");
    if (returnGrid.contains("fDefault")) returnGrid = "";
    return returnGrid;
}

OrcaSCFData::OrcaSCFData()
{
    m_useLevel = false;
    m_levelShift = 0.25;
    m_levelError = 0.001;
    m_useDamping = false;
    m_dampFactor = 0.7;
    m_dampError = 0.1;
    m_conv = DIIS;
    m_2ndConv = SOSCF;

    m_scfType = RKS;
    m_maxIter = 125;
    m_accuracy = NORMALSCF;
}
void OrcaSCFData::reset()
{
    m_useLevel = false;
    m_levelShift = 0.25;
    m_levelError = 0.001;
    m_useDamping = false;
    m_dampFactor = 0.7;
    m_dampError = 0.1;
    m_conv = DIIS;
    m_2ndConv = SOSCF;
    m_scfType = RKS;
    m_maxIter = 125;
    m_accuracy = NORMALSCF;
}
QString OrcaSCFData::getAccuracyTxt()
{
    switch (m_accuracy)
    {
    case NORMALSCF:
        return "NormalSCF";
    case TIGHTSCF:
        return "TightSCF";
    case VERYTIGHTSCF:
        return "VeryTightSCF";
    case EXTREMESCF:
        return "ExtremSCF";
    default:
        return "";
    }
}

QString OrcaSCFData::getTypeTxt()
{
    switch (m_scfType)
    {
    case RKS:
        return "RHF";
    case UKS:
        return "UHF";
    default:
        return "ROHF";
    }
}
OrcaDataData::OrcaDataData()
{
    m_coordsType = CARTESIAN;
    m_printLevel = NORMAL;
    m_basisPrint = false;
    m_MOPrint = false;
}
void OrcaDataData::reset()
{
    m_coordsType = CARTESIAN;
    m_printLevel = NORMAL;
    m_basisPrint = false;
    m_MOPrint = false;
}
QString OrcaDataData::getPrintLevelTxt()
{
    switch (m_printLevel)
    {
    case NOTHING:
        return " ";
    case MINI:
        return "MiniPrint";
    case SMALL:
        return "SmallPrint";
    case NORMAL:
        return "NormalPrint";
    case LARGE:
        return "LargePrint";
    default:
        return "";
    }
}

QString OrcaDataData::getFormatTxt()
{
    switch (m_coordsType)
    {
    case ZMATRIX:
        return "z-Matrix";
    case CARTESIAN:
        return "Cartesian";
    case ZMATRIX_COMPACT:
        return "z-Matrix compact";
    default:
        return "";
    }
}



















