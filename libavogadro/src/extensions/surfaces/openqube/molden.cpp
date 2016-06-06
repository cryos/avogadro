/******************************************************************************

  This source file is part of the OpenQube project.

  Copyright 2010 Geoffrey R. Hutchison

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "molden.h"

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#ifdef WIN32
#define _USE_MATH_DEFINES
#include <math.h> // needed for M_PI
#endif

using Eigen::Vector3d;
using std::vector;

#ifndef BOHR_TO_ANGSTROM
#define BOHR_TO_ANGSTROM 0.529177249
#endif

namespace OpenQube
{

MoldenFile::MoldenFile(const QString &filename, GaussianSet* basis):
    m_coordFactor(1.0), m_currentMode(NotParsing), m_electrons(0), m_sphericalD(false),
    m_sphericalG(false)
{
  // Open the file for reading and process it
  QFile* file = new QFile(filename);

  file->open(QIODevice::ReadOnly | QIODevice::Text);
  m_in = file;

  qDebug() << "File" << filename << "opened.";

  // Process the formatted checkpoint and extract all the information we need
  while (!m_in->atEnd()) {
    processLine(basis);
  }

  // check for spherical components of D, F and G shells

  if (m_sphericalD) {
      for (uint i = 0; i < m_shellTypes.size(); ++i) {
          if (m_shellTypes.at(i) == D){
              m_shellTypes.at(i) = D5;
          } else if (m_shellTypes.at(i) == F) {
              m_shellTypes.at(i) = F7;
          }
      }
  }
  if (m_sphericalG) {
      for (uint i = 0; i < m_shellTypes.size(); ++i) {
          if (m_shellTypes.at(i) == G)
              m_shellTypes.at(i) = G9;
      }
  }

  // Now it should all be loaded load it into the basis set

  if (basis->getUseOrcaNormalization()) unnormalizeBasis();        // Molden files written by ORCA_2mkl have always normalized basissets
  load(basis);

  delete file;
}

MoldenFile::~MoldenFile()
{
}

void MoldenFile::processLine(GaussianSet* basis)
{
  // First truncate the line, remove trailing white space and check for blank lines
  QString key = m_in->readLine().trimmed();
  while(key.isEmpty() && !m_in->atEnd()) {
    key = m_in->readLine().trimmed();
  }

  if (m_in->atEnd())
    return;

  QStringList list = key.split(' ', QString::SkipEmptyParts);

  // Big switch statement checking for various things we are interested in
  // Make sure to switch mode:
  //      enum mode { NotParsing, Atoms, GTO, STO, MO, SCF }
  if (key.contains("[Title]", Qt::CaseInsensitive)) {
        key = m_in->readLine().trimmed();
        if (key.contains("created by orca_2mkl", Qt::CaseInsensitive)) {
              basis->setUseOrcaNormalization(true);
        }
  } else if (key.contains("[atoms]", Qt::CaseInsensitive)) {
      m_coordFactor = 1./BOHR_TO_ANGSTROM;      // convert to Bohr
      if (list.size() > 1 && list[1].contains("au", Qt::CaseInsensitive))
          m_coordFactor = 1.;       //already in Bohr;
      m_currentMode = Atoms;
  } else if (key.contains("[gto]", Qt::CaseInsensitive)) {
    m_currentMode = GTO;
  } else if (key.contains("[mo]", Qt::CaseInsensitive)) {
    m_currentMode = MO;
  } else if (key.contains("[5D]", Qt::CaseInsensitive)) {
    m_sphericalD = true;
  } else if (key.contains("[9G]", Qt::CaseInsensitive)) {
    m_sphericalG = true;
  } else if (key.contains("[")) { // unknown section
    m_currentMode = NotParsing;
  } else {
    QString shell;
    orbital shellType;

    // parsing a line -- what mode are we in?
    switch (m_currentMode) {
    case Atoms:
      // element_name number atomic_number x y z
      if (list.size() < 6)
        return;
      m_aNums.push_back(list[2].toInt());
      m_aPos.push_back(list[3].toDouble() * m_coordFactor);
      m_aPos.push_back(list[4].toDouble() * m_coordFactor);
      m_aPos.push_back(list[5].toDouble() * m_coordFactor);

      break;
    case GTO:
    {
      // TODO: detect dead files and make bullet-proof
      int atom = list[0].toInt();

      key = m_in->readLine().trimmed();
      while (!key.isEmpty()) { // read the shell types in this GTO
        list = key.split(' ', QString::SkipEmptyParts);
        shell = list[0].toLower();
        shellType = UU;
        if (shell.contains("sp"))
          shellType = SP;
        else if (shell.contains("s"))
          shellType = S;
        else if (shell.contains("p"))
          shellType = P;
        else if (shell.contains("d"))
          shellType = D;
        else if (shell.contains("f"))
          shellType = F;
        else if (shell.contains("g"))
          shellType = G;
        if (shellType != UU) {
          m_shellTypes.push_back(shellType);
          m_shelltoAtom.push_back(atom);
        }
        else
          return;

        int numGTOs = list[1].toInt();
        m_shellNums.push_back(numGTOs);

        // now read all the exponents and contraction coefficients
        for (int gto = 0; gto < numGTOs; ++gto) {
          key = m_in->readLine().trimmed();
          list = key.split(' ', QString::SkipEmptyParts);
          m_a.push_back(list[0].replace('D','E').toDouble());
          m_c.push_back(list[1].replace('D','E').toDouble());
          if (shellType == SP && list.size() > 2)
            m_csp.push_back(list[2].replace('D','E').toDouble());
        } // finished parsing a new GTO
        key = m_in->readLine().trimmed(); // start reading the next shell
      }
    }
    break;

    case MO:
      // parse occ, spin, energy, etc.
      while (!key.isEmpty() && key.contains('=')) {
        key = m_in->readLine().trimmed();
        list = key.split(' ', QString::SkipEmptyParts);
        if (key.contains("occup", Qt::CaseInsensitive))
          m_electrons += (int)list[1].toDouble();
      }

      // parse MO coefficients
      while (!key.isEmpty() && !key.contains('=')) {
        list = key.split(' ', QString::SkipEmptyParts);
        if (list.size() < 2)
          break;

        m_MOcoeffs.push_back(list[1].replace('D','E').toDouble());
        key = m_in->readLine().trimmed();
      } // finished parsing a new MO

      break;
    case STO:
    case SCF:
    case NotParsing:
      break;
    }
  }
}

void MoldenFile::load(GaussianSet* basis)
{
  // Now load up our basis set
  basis->setNumElectrons(m_electrons);
  int nAtom = 0;
  for (unsigned int i = 0; i < m_aPos.size(); i += 3)
    basis->addAtom(Vector3d(m_aPos.at(i), m_aPos.at(i+1), m_aPos.at(i+2)),
                   m_aNums.at(nAtom++));

  // Set up the GTO primitive counter, go through the shells and add them
  int nGTO = 0;
  int nSP = 0; // number of SP shells
  for (unsigned int i = 0; i < m_shellTypes.size(); ++i) {

    // Handle the SP case separately - this should possibly be a distinct type
    if (m_shellTypes.at(i) == SP)  {
      // SP orbital type - currently have to unroll into two shells
      int s = basis->addBasis(m_shelltoAtom.at(i) - 1, S);
      int p = basis->addBasis(m_shelltoAtom.at(i) - 1, P);
      for (int j = 0; j < m_shellNums.at(i); ++j) {
        basis->addGTO(s, m_c.at(nGTO), m_a.at(nGTO));
        basis->addGTO(p, m_csp.at(nSP), m_a.at(nGTO));
        ++nSP;
        ++nGTO;
      }
    }
    else {
      int b = basis->addBasis(m_shelltoAtom.at(i) - 1, m_shellTypes.at(i));
      for (int j = 0; j < m_shellNums.at(i); ++j) {
        basis->addGTO(b, m_c.at(nGTO), m_a.at(nGTO));
        ++nGTO;
      }
    }
  }
  // Now to load in the MO coefficients
  if (m_MOcoeffs.size())
    basis->addMOs(m_MOcoeffs);


    qDebug() << " Molden loadBasis done";

}
void MoldenFile::unnormalizeBasis()
{
    // Unnormalize Gauss Basisset when Moldenfile was written by the ORCA_2mkl Program
    // because ORCA writes normalized basissets up to now (1-Sep-2014)
    // but ONLY for MOLDEN format output
    int iGTO=0;
    double alpha, NP, N;
    for (int i=0;i<m_shellNums.size(); i++) {

        switch (m_shellTypes.at(i)) {
        case S:
            for (int j = 0; j < m_shellNums.at(i); ++j) {
                alpha = m_a.at(iGTO);
                NP = pow(2.0*alpha/M_PI, 0.75);
                m_c.at(iGTO) /= NP;
                iGTO ++;
            }
            break;
        case P:
            for (int j = 0; j < m_shellNums.at(i); ++j) {
                alpha = m_a.at(iGTO);
                NP = pow(2.0*alpha/M_PI, 0.75);
                N= sqrt(alpha)*2.0;
                m_c.at(iGTO) /= NP*N;
                iGTO ++;
            }
            break;
        case D5:
            for (int j = 0; j < m_shellNums.at(i); ++j) {
                alpha = m_a.at(iGTO);
                NP = pow(2.0*alpha/M_PI, 0.75);
                N= 4.0*alpha;
                m_c.at(iGTO) /= NP*N;
                iGTO ++;
            }
            break;
        case F7:
            for (int j = 0; j < m_shellNums.at(i); ++j) {
                alpha = m_a.at(iGTO);
                NP = pow(2.0*alpha/M_PI, 0.75);
                N= 8.0*pow(alpha,1.5);
                m_c.at(iGTO) /= NP*N;
                iGTO ++;
            }
            break;
        case G9:
            for (int j = 0; j < m_shellNums.at(i); ++j) {
                alpha = m_a.at(iGTO);
                NP = pow(2.0*alpha/M_PI, 0.75);
                N= 16.0/sqrt(3.0)*alpha*alpha;
                m_c.at(iGTO) /= NP*N;
                iGTO ++;
            }
            break;
        default:
            // Should never hit here
            ;
        }
    }
}

//int GAUSS_UnNormalize(BFNGauss &BG)
//{ int i;
//  double alpha,N,NP;
//  for (i=0;i<BG.ng;i++){
//    alpha=BG.a[i];
//    NP   =pow(2.0*alpha/M_PI, 0.75);
//    switch(BG.l){
//    case 0:
//      N=1.0;
//      break;
//    case 1:
//      N= sqrt(alpha)*2.0;
//      break;
//    case 2:
//      N= 4.0*alpha;
//      break;
//    case 3:
//      N= 8.0*pow(alpha,1.5);
//      break;
//    case 4:
//      N= 16.0/sqrt(3.0)*alpha*alpha;
//      break;
//    case 5:
//      N= 32.0*sqrt(105.0)/315.0*pow(alpha,2.5);
//      break;
//    case 6:
//      N= 64.0*sqrt(1155.0)/3465.0*pow(alpha,3.0);
//      break;
//    case 7:
//      N= 128.0*sqrt(15015.0)/45045.0*pow(alpha,3.5);
//      break;
//    case 8:
//      N= 256.0*sqrt(1001.0)/45045.0*pow(alpha,4.0);
//      break;
//    default:
//      printf("Invalid angular momentum in GAUSS_UnNormalize\n");
//      exit(1);
//      break;
//    };
//    BG.d[i]/=N*NP;
//  };
//  return 0;
//};
void MoldenFile::outputAll()
{
  qDebug() << "Shell mappings.";
  for (unsigned int i = 0; i < m_shellTypes.size(); ++i)
    qDebug() << i << ": type =" << m_shellTypes.at(i)
             << ", number =" << m_shellNums.at(i)
             << ", atom =" << m_shelltoAtom.at(i);
  qDebug() << "MO coefficients.";
  for (unsigned int i = 0; i < m_MOcoeffs.size(); ++i)
    qDebug() << m_MOcoeffs.at(i);
}

}
