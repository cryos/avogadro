 /******************************************************************************

  This source file is part of the OpenQube project.

  Copyright 2008-2010 Marcus D. Hanwell
  Copyright 2008 Albert De Fusco

  Some Portions Copyright (C) Dagmar Lenk
        - calculation of molecular orbitals with higher shells (F-I) for spherical coordinates (general set)
  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/
#include "gaussianset.h"

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <math.h> // needed for M_PI
#endif

#include "cube.h"

#include <cmath>

#include <QtCore/QtConcurrentMap>
#include <QtCore/QFuture>
#include <QtCore/QFutureWatcher>
#include <QtCore/QReadWriteLock>
#include <QtCore/QDebug>

using std::vector;
using Eigen::Vector3d;
using Eigen::Vector3i;
using Eigen::MatrixXd;

namespace OpenQube
{
struct GaussianShell
{
  GaussianSet *set;  // A pointer to the GaussianSet, cannot write to member vars
  Cube *tCube;       // The target cube, used to initialise temp cubes too
  unsigned int pos;  // The index ofposition of the point to calculate the MO for
  unsigned int state;// The MO number to calculate
};

static const double BOHR_TO_ANGSTROM = 0.529177249;
static const double ANGSTROM_TO_BOHR = 1.0 / BOHR_TO_ANGSTROM;

GaussianSet::GaussianSet() : m_numMOs(0), m_numAtoms(0), m_init(false),
  m_cube(0), m_gaussianShells(0), m_useOrcaNorm(false)
{
}

GaussianSet::~GaussianSet()
{
}

unsigned int GaussianSet::addAtom(const Vector3d& pos, int atomicNumber)
{
  m_init = false;
  // Add to the new data structure, delete the old soon
  m_molecule.addAtom(pos, atomicNumber);

  return m_molecule.numAtoms() - 1;
}

unsigned int GaussianSet::addBasis(unsigned int atom, orbital type)
{
  // Count the number of independent basis functions
  switch (type) {
  case S:
    m_numMOs++;
    break;
  case P:
    m_numMOs += 3;
    break;
  case SP:
    m_numMOs += 4;
    break;
  case D:
    m_numMOs += 6;
    break;
  case D5:
    m_numMOs += 5;
    break;
  case F:
    m_numMOs += 10;
    break;
  case F7:
    m_numMOs += 7;
    break;
  case G:
    m_numMOs += 15;
    break;
  case G9:
    m_numMOs += 9;
    break;
  case H:
    m_numMOs += 21;
    break;
  case H11:
    m_numMOs += 11;
    break;
  case I:
    m_numMOs += 28;
    break;
  case I13:
    m_numMOs += 13;
    break;
  default:
    // Should never hit here
    ;
  }
  m_init = false;

  // Add to the new data structure, delete the old soon
  m_symmetry.push_back(type);
  m_atomIndices.push_back(atom);
  return m_symmetry.size() - 1;
}

unsigned int GaussianSet::addGTO(unsigned int, double c, double a)
{
  // Use the new data structure
  if (m_gtoIndices.size() < m_atomIndices.size()) {
    // First GTO added for this basis - add the gto index
    m_gtoIndices.push_back(m_gtoA.size());
  }
  m_gtoA.push_back(a);
  m_gtoC.push_back(c);

  return m_gtoA.size() - 1;
}

void GaussianSet::addMOs(const vector<double>& MOs)
{
  m_init = false;

  // Some programs don't output all MOs, so we take the amount of data
  // and divide by the # of AO functions
  unsigned int columns = MOs.size() / m_numMOs;
  qDebug() << " add MOs: " << m_numMOs << columns;

  m_moMatrix.resize(m_numMOs, m_numMOs);

  for (unsigned int j = 0; j < columns; ++j)
      for (unsigned int i = 0; i < m_numMOs; ++i)
      m_moMatrix.coeffRef(i, j) = MOs[i + j*m_numMOs];
}

void GaussianSet::addMO(double)
{
  m_init = false;
}

bool GaussianSet::setDensityMatrix(const Eigen::MatrixXd &m)
{
  m_density.resize(m.rows(), m.cols());
  m_density = m;
  return true;
}

bool GaussianSet::calculateCubeMO(Cube *cube, unsigned int state)
{
  // Set up the calculation and ideally use the new QtConcurrent code to
  // multithread the calculation...
  if (state < 1 || state > static_cast<unsigned int>(m_moMatrix.rows()))
      return false;

  // Must be called before calculations begin - use different init for Orca written data
  if (!m_useOrcaNorm) {
      initCalculation();
  } else {
      initCalculationForOrca();
  }
  // Set up the points we want to calculate the density at
  m_gaussianShells = new QVector<GaussianShell>(cube->data()->size());

  for (int i = 0; i < m_gaussianShells->size(); ++i) {
      (*m_gaussianShells)[i].set = this;
      (*m_gaussianShells)[i].tCube = cube;
      (*m_gaussianShells)[i].pos = i;
      (*m_gaussianShells)[i].state = state;
  }

  // Lock the cube until we are done.
  cube->lock()->lockForWrite();

  // Watch for the future
  connect(&m_watcher, SIGNAL(finished()), this, SLOT(calculationComplete()));

  // The main part of the mapped reduced function...
  m_future = QtConcurrent::map(*m_gaussianShells, GaussianSet::processPoint);
  // Connect our watcher to our future
  m_watcher.setFuture(m_future);

  return true;
}

bool GaussianSet::calculateCubeDensity(Cube *cube)
{
  if (m_density.size() == 0) {
    qDebug() << "Cannot calculate density -- density matrix not set.";
    return false;
  }

  // FIXME Still not working, committed so others could see current state.
  // - seems to work for Orca data

  // Must be called before calculations begin - use different init for Orca written data
  if (!m_useOrcaNorm) {
      initCalculation();
  } else {
      initCalculationForOrca();
  }

  // Set up the points we want to calculate the density at
  m_gaussianShells = new QVector<GaussianShell>(cube->data()->size());

  for (int i = 0; i < m_gaussianShells->size(); ++i) {
    (*m_gaussianShells)[i].set = this;
    (*m_gaussianShells)[i].tCube = cube;
    (*m_gaussianShells)[i].pos = i;
  }

  // Lock the cube until we are done.
  cube->lock()->lockForWrite();

  // Watch for the future
  connect(&m_watcher, SIGNAL(finished()), this, SLOT(calculationComplete()));

  // The main part of the mapped reduced function...
  m_future = QtConcurrent::map(*m_gaussianShells, GaussianSet::processDensity);
  // Connect our watcher to our future
  m_watcher.setFuture(m_future);

  return true;
}

BasisSet * GaussianSet::clone()
{
  GaussianSet *result = new GaussianSet();

  result->m_symmetry = this->m_symmetry;
  result->m_atomIndices = this->m_atomIndices;
  result->m_moIndices = this->m_moIndices;
  result->m_gtoIndices = this->m_gtoIndices;
  result->m_cIndices = this->m_cIndices;
  result->m_gtoA = this->m_gtoA;
  result->m_gtoC = this->m_gtoC;
  result->m_gtoCN = this->m_gtoCN;
  result->m_moMatrix = this->m_moMatrix;
  result->m_density = this->m_density;

  result->m_numMOs = this->m_numMOs;
  result->m_numAtoms = this->m_numAtoms;
  result->m_init = this->m_init;
  result->m_useOrcaNorm = this->m_useOrcaNorm;


  // Skip tmp vars
  return result;
}

void GaussianSet::calculationComplete()
{
  disconnect(&m_watcher, SIGNAL(finished()), this, SLOT(calculationComplete()));
  (*m_gaussianShells)[0].tCube->lock()->unlock();
  delete m_gaussianShells;
  m_gaussianShells = 0;
  emit finished();
}

inline bool GaussianSet::isSmall(double val)
{
  if (val > -1e-20 && val < 1e-20)
    return true;
  else
    return false;
}

void GaussianSet::initCalculation()
{
  if (m_init)
    return;
  // This currently just involves normalising all contraction coefficients
  m_numAtoms = m_molecule.numAtoms();
  m_gtoCN.clear();

  // Initialise the new data structures that are hopefully more efficient
  unsigned int indexMO = 0;
  unsigned int skip = 0; // for unimplemented shells

  m_moIndices.resize(m_symmetry.size());
  // Add a final entry to the gtoIndices
  m_gtoIndices.push_back(m_gtoA.size());
  for(unsigned int i = 0; i < m_symmetry.size(); ++i) {
    switch (m_symmetry[i]) {
    case S:
      m_moIndices[i] = indexMO++;
      m_cIndices.push_back(m_gtoCN.size());
      // Normalization of the S-type orbitals (normalization used in JMol)
      // (8 * alpha^3 / pi^3)^0.25 * exp(-alpha * r^2)
      for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 0.75) * 0.71270547);
      }
      break;
    case P:
      m_moIndices[i] = indexMO;
      indexMO += 3;
      m_cIndices.push_back(m_gtoCN.size());
      // Normalization of the P-type orbitals (normalization used in JMol)
      // (128 alpha^5 / pi^3)^0.25 * [x|y|z]exp(-alpha * r^2)
      for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 1.25) * 1.425410941);
        m_gtoCN.push_back(m_gtoCN.back());
        m_gtoCN.push_back(m_gtoCN.back());
      }
      break;
    case D:
      // Cartesian - 6 d components
      // Order in xx, yy, zz, xy, xz, yz
      m_moIndices[i] = indexMO;
      indexMO += 6;
      m_cIndices.push_back(m_gtoCN.size());
      // Normalization of the P-type orbitals (normalization used in JMol)
      // xx|yy|zz: (2048 alpha^7/9pi^3)^0.25 [xx|yy|zz]exp(-alpha r^2)
      // xy|xz|yz: (2048 alpha^7/pi^3)^0.25 [xy|xz|yz]exp(-alpha r^2)
      for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 1.75) * 1.645922781);
        m_gtoCN.push_back(m_gtoCN.back());
        m_gtoCN.push_back(m_gtoCN.back());

        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 1.75) * 2.850821881);
        m_gtoCN.push_back(m_gtoCN.back());
        m_gtoCN.push_back(m_gtoCN.back());
      }
      break;
    case D5:
      // Spherical - 5 d components
      // Order in d0, d+1, d-1, d+2, d-2
      // Form d(z^2-r^2), dxz, dyz, d(x^2-y^2), dxy
      m_moIndices[i] = indexMO;
      indexMO += 5;
      m_cIndices.push_back(m_gtoCN.size());
      for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
        m_gtoCN.push_back(m_gtoC[j] * pow(2048 * pow(m_gtoA[j], 7.0)
              / (9.0 * M_PI*M_PI*M_PI), 0.25));
        m_gtoCN.push_back(m_gtoC[j] * pow(2048 * pow(m_gtoA[j], 7.0)
              / (M_PI*M_PI*M_PI), 0.25));
        m_gtoCN.push_back(m_gtoCN.back());
        // I think this is correct but reaally need to check...
        m_gtoCN.push_back(m_gtoC[j] * pow(128 * pow(m_gtoA[j], 7.0)
              / (M_PI*M_PI*M_PI), 0.25));
        m_gtoCN.push_back(m_gtoC[j] * pow(2048 * pow(m_gtoA[j], 7.0)
              / (M_PI*M_PI*M_PI), 0.25));
      }
      break;
    case F:
    /*
     Thanks, Jmol
     Cartesian forms for f (l = 3) basis functions:
     Type         Normalization
     xxx          [(32768 * alpha^9) / (225 * pi^3))]^(1/4)
     xxy          [(32768 * alpha^9) / (9 * pi^3))]^(1/4)
     xxz          [(32768 * alpha^9) / (9 * pi^3))]^(1/4)
     xyy          [(32768 * alpha^9) / (9 * pi^3))]^(1/4)
     xyz          [(32768 * alpha^9) / (1 * pi^3))]^(1/4)
     xzz          [(32768 * alpha^9) / (9 * pi^3))]^(1/4)
     yyy          [(32768 * alpha^9) / (225 * pi^3))]^(1/4)
     yyz          [(32768 * alpha^9) / (9 * pi^3))]^(1/4)
     yzz          [(32768 * alpha^9) / (9 * pi^3))]^(1/4)
     zzz          [(32768 * alpha^9) / (225 * pi^3))]^(1/4)

     Thank you, Python
                                 pi = 3.141592653589793
     (32768./225./(pi**3.))**(0.25) = 1.4721580892990938
     (32768./9./(pi**3.))**(0.25)   = 3.291845561298979
     (32768./(pi**3.))**(0.25)      = 5.701643762839922
     */
      {
      double norm1 = 1.4721580892990938;
      double norm2 = 3.291845561298979;
      double norm3 = 5.701643762839922;
      m_moIndices[i] = indexMO;
      indexMO += 10;
      m_cIndices.push_back(static_cast<unsigned int>(m_gtoCN.size()));
      for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm1); //xxx
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm2);  //xxy
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm2);  //xxz
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm2);  //xyy
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm3);  //xyz
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm2);  //xzz
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm1); //yyy
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm2);  //yyz
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm2);  //yzz
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm1); //zzz
      }}
      break;
    case F7:
      {
      //m-independent normalization factor
      //math.sqrt(2.**(3.+3./2.))/(math.pi**(3./4.))*math.sqrt(2.**3. / 15.)
      //same as norm1 above.
      double norm = 1.4721580892990935;
        m_moIndices[i] = indexMO;
        indexMO += 7;
      m_cIndices.push_back(static_cast<unsigned int>(m_gtoCN.size()));
        for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm); //0
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm); //+1
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm); //-1
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm); //+2
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm); //-2
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm); //+3
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 2.25) * norm); //-3
      }}
        break;
    case G:
      skip = 15;
      m_moIndices[i] = indexMO;
      indexMO += skip;
      m_cIndices.push_back(m_gtoCN.size());
      qDebug() << m_symmetry[i] << " Basis set not handled - results may be incorrect.";
      break;
    case G9:
      skip = 9;
      m_moIndices[i] = indexMO;
      indexMO += skip;
      m_cIndices.push_back(m_gtoCN.size());
      qDebug() << m_symmetry[i] << " Basis set not handled - results may be incorrect.";
      break;
    case H:
      skip = 21;
      m_moIndices[i] = indexMO;
      indexMO += skip;
      m_cIndices.push_back(m_gtoCN.size());
      qDebug() << m_symmetry[i] << " Basis set not handled - results may be incorrect.";
      break;
    case H11:
      skip = 11;
      m_moIndices[i] = indexMO;
      indexMO += skip;
      m_cIndices.push_back(m_gtoCN.size());
      qDebug() << m_symmetry[i] << " Basis set not handled - results may be incorrect.";
      break;
    case I:
      skip = 28;
      m_moIndices[i] = indexMO;
      indexMO += skip;
      m_cIndices.push_back(m_gtoCN.size());
      qDebug() << m_symmetry[i] << " Basis set not handled - results may be incorrect.";
      break;
    case I13:
      skip = 13;
      m_moIndices[i] = indexMO;
      indexMO += skip;
      m_cIndices.push_back(m_gtoCN.size());
      qDebug() << m_symmetry[i] << " Basis set not handled - results may be incorrect.";
      break;
    default:
      qDebug() << "Basis set not handled - results may be incorrect.";
    }
  }
  m_init = true;
//  outputAll();
}
void GaussianSet::initCalculationForOrca()
{
  if (m_init)
    return;
  // This currently just involves normalising all contraction coefficients
  m_numAtoms = m_molecule.numAtoms();
  m_gtoCN.clear();

  // Initialise the new data structures that are hopefully more efficient
  unsigned int indexMO = 0;
  unsigned int skip = 0; // for unimplemented shells

  m_moIndices.resize(m_symmetry.size());
  // Add a final entry to the gtoIndices
  m_gtoIndices.push_back(m_gtoA.size());
  for(unsigned int i = 0; i < m_symmetry.size(); ++i) {
    switch (m_symmetry[i]) {
    case S:
      m_moIndices[i] = indexMO++;
      m_cIndices.push_back(m_gtoCN.size());
      // Normalization of the S-type orbitals (normalization used in JMol)
      // (8 * alpha^3 / pi^3)^0.25 * exp(-alpha * r^2)
      for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 0.75) * 0.71270547);
      }
      break;
    case P:
      m_moIndices[i] = indexMO;
      indexMO += 3;
      m_cIndices.push_back(m_gtoCN.size());
      // Normalization of the P-type orbitals (normalization used in JMol)
      // (128 alpha^5 / pi^3)^0.25 * [x|y|z]exp(-alpha * r^2)
      for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
        m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 1.25) * 1.425410941);
        m_gtoCN.push_back(m_gtoCN.back());
        m_gtoCN.push_back(m_gtoCN.back());
      }
      break;
    case D5:
    {
        // Spherical - 5 d components
        // Order in d0, d+1, d-1, d+2, d-2
        // Form d(3*z^2-r^2), dxz, dyz, d(x^2-y^2), dxy
        m_moIndices[i] = indexMO;
        indexMO += 5;
        m_cIndices.push_back(m_gtoCN.size());
        //
        // define some tmp value to avoid over/underflows during calculations but leave the used formulas visible
        //
        double tmp = (8./(M_PI*M_PI*M_PI));

        double tmp_d0  = 2./(pow(3.,.5));
        double tmp_d1  = 4.;
        double tmp_d2p = 2.;
        double tmp_d2n = tmp_d1;

        for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            double  tmpAlpha = m_gtoA[j];                                                 // alpha
            double  tmp1Alpha = pow(m_gtoA[j], 3.0);                                      // alpha^3

            m_gtoCN.push_back(m_gtoC[j] * tmp_d0*tmpAlpha * pow((tmp*tmp1Alpha),0.25));   // NormD0 -  2*pow((2^3 * alpha^7)/(3^2 * pi^3)),.25)

            m_gtoCN.push_back(m_gtoC[j] * tmp_d1*tmpAlpha * pow((tmp*tmp1Alpha),0.25));   // NormD1p - 2^2*pow((2^3 * alpha^7)/pi^3),.25)
            m_gtoCN.push_back(m_gtoCN.back());                                            // NormD1n - same as NormD1p

            m_gtoCN.push_back(m_gtoC[j] * tmp_d2p*tmpAlpha * pow((tmp*tmp1Alpha),0.25));  // NormD2p - 2*pow((2^3 * alpha^7)/pi^3),.25)
            m_gtoCN.push_back(m_gtoC[j] * tmp_d2n*tmpAlpha * pow((tmp*tmp1Alpha),0.25));  // NormD2n - 2^2*pow((2^3 * alpha^7)/pi^3),.25) same as D1n
        }
        break;
    }
    case F7:
    {
        // Spherical - 7 F components
        // Order in f0, f+1, f-1, f+2, f-2, f+3, f-3
        //
        m_moIndices[i] = indexMO;
        indexMO += 7;
        m_cIndices.push_back(m_gtoCN.size());
        //
        // define some tmp value to avoid over/underflows during calculations but leave the used formulas visible
        //
        double tmp = (8./(M_PI*M_PI*M_PI));
        double tmp1 = (2./(M_PI*M_PI*M_PI));

        double tmp_f0  = 4./(pow(15.,.5));
        double tmp_f1  = 4./(pow(5.,.5));
        double tmp_f2p = 4.;
        double tmp_f2n = 8.;
        double tmp_f3 = 4./(pow(3.,.5));

        for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            double  tmpAlpha = m_gtoA[j];                                       // alpha
            double  tmpAlpha2 = tmpAlpha * tmpAlpha;                            // alpha^2

            m_gtoCN.push_back(m_gtoC[j] * tmp_f0 * tmpAlpha2 * pow((tmp*tmpAlpha),0.25));      // f0 4*pow((2^3 * alpha^9)/(15^2 * pi^3)),.25)

            m_gtoCN.push_back(m_gtoC[j] * tmp_f1 * tmpAlpha2 * pow((tmp1 * tmpAlpha),0.25));   // f+1 4*pow((2 * alpha^9)/(5^2 * pi^3)),.25)
            m_gtoCN.push_back(m_gtoCN.back());                                                  // f-1 (same as f+1)


            m_gtoCN.push_back(m_gtoC[j] * tmp_f2p * tmpAlpha2 * pow((tmp*tmpAlpha),0.25));      // f+2 4*pow((2^3 * alpha^9 / pi^3)),.25)
            m_gtoCN.push_back(m_gtoC[j] * tmp_f2n * tmpAlpha2 * pow((tmp*tmpAlpha),0.25));      // f-2 8*pow((2^3 * alpha^9 / pi^3)),.25)

            m_gtoCN.push_back(m_gtoC[j] * tmp_f3 * tmpAlpha2 * pow((tmp1 * tmpAlpha),0.25));   // f+3 4*pow((2 * alpha^9)/(3^2 * pi^3)),.25)
            m_gtoCN.push_back(m_gtoCN.back());                                                  // f-3 (same as f+3)
        }
//        }
        break;
    }
    case G:
        skip = 15;
        m_moIndices[i] = indexMO;
        indexMO += skip;
        m_cIndices.push_back(m_gtoCN.size());
        qDebug() << m_symmetry[i] << " Basis set not handled - results may be incorrect.";
        break;
    case G9:
    {
        // Spherical - 9 G components
        // Order in g0, g+1, g-1, g+2, g-2, g+3, g-3, g+4, g-4
        //
        m_moIndices[i] = indexMO;
        indexMO += 9;
        m_cIndices.push_back(m_gtoCN.size());

        //
        // define some tmp value to avoid over/underflows during calculations but leave the used formulas visible
        //
        double tmp = (8./(M_PI*M_PI*M_PI));
        double tmp1 = (2./(M_PI*M_PI*M_PI));

        double tmp_g0  = 2./(pow(105.,.5));
        double tmp_g1 = 8./(pow(21.,.5));

        double tmp_g2p = 4./(pow(21.,.5));
        double tmp_g2n = 8./(pow(21.,.5));

        double tmp_g3 = 8./(pow(3.,.5));

        double tmp_g4p = 2./pow(3.,.5);
        double tmp_g4n = 8./pow(3.,.5);


        for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            double  tmpAlpha = pow(m_gtoA[j], 2.0);                                         // alpha^2
            double  tmp1Alpha = tmpAlpha*m_gtoA[j];                                         // alpha^3

            m_gtoCN.push_back(m_gtoC[j] *  tmp_g0*tmpAlpha * pow((tmp*tmp1Alpha),0.25));    // g0  2*pow((2^3 * alpha^11)/(105^2 * pi^3)),.25)

            m_gtoCN.push_back(m_gtoC[j] *  tmp_g1*tmpAlpha * pow((tmp1*tmp1Alpha),0.25));   // g+1  2^3 * pow((2 * alpha^11)/(21^2 * pi^3)),.25)
            m_gtoCN.push_back(m_gtoCN.back());                                              // g-1 (same as g+1)

            m_gtoCN.push_back(m_gtoC[j] *  tmp_g2p*tmpAlpha * pow((tmp*tmp1Alpha),0.25));   // g+2  2^2 * pow((2^3 * alpha^11)/(21^2 * pi^3)),.25)
            m_gtoCN.push_back(m_gtoC[j] *  tmp_g2n*tmpAlpha * pow((tmp*tmp1Alpha),0.25));   // g-2  2^3 * pow((2^3 * alpha^11)/(21^2 * pi^3)),.25)

            m_gtoCN.push_back(m_gtoC[j] *  tmp_g3*tmpAlpha * pow((tmp1*tmp1Alpha),0.25));   // g+3  2^3 *pow((2 * alpha^11)/(3^2 * pi^3)),.25)
            m_gtoCN.push_back(m_gtoCN.back());                                              // g-3 (same as g+3)

            m_gtoCN.push_back(m_gtoC[j] *  tmp_g4p*tmpAlpha * pow((tmp*tmp1Alpha),0.25));    // g+4  2*pow((2^3 * alpha^11)/(3^2 * pi^3)),.25)
            m_gtoCN.push_back(m_gtoC[j] *  tmp_g4n*tmpAlpha * pow((tmp*tmp1Alpha),0.25));    // g-4  2^3 *pow((2^3 * alpha^11)/(3^2 * pi^3)),.25)
        }
        break;
    }
    case H11:
    {
        // Spherical - 11 H components
        // Order in h0, h+1, h-1, h+2, h-2, h+3, h-3, h+4, h-4, h+5, h-5
        //
        m_moIndices[i] = indexMO;
        indexMO += 11;
        m_cIndices.push_back(m_gtoCN.size());

        //
        // define some tmp value to avoid over/underflows during calculations but leave the used formulas visible
        //
        double tmp = (8./(M_PI*M_PI*M_PI));
        double tmp1 = (2./(M_PI*M_PI*M_PI));

        double tmp_h0  = 4./(3. *pow(105.,.5));
        double tmp_h1 = 4./(3. *pow(7.,.5));

        double tmp_h2p = 8./3.;
        double tmp_h2n = 16./3.;

        double tmp_h3 = 4./(3. *pow(3.,.5));

        double tmp_h4p = 4./pow(3.,.5);
        double tmp_h4n = 16./pow(3.,.5);

        double tmp_h5 = 4./pow(15,.5);

        for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            double  tmpAlpha = pow(m_gtoA[j], 3.0);

            m_gtoCN.push_back(m_gtoC[j] *  tmp_h0*tmpAlpha * pow((tmp*m_gtoA[j]),0.25));    // h0  (2^2)/3  * pow((2^3 * alpha^13)/(105^2 * pi^3)),.25)

            m_gtoCN.push_back(m_gtoC[j] * tmp_h1*tmpAlpha *pow((tmp*m_gtoA[j]),0.25));      // h+1  (2^2)/3 * pow((2^3 *alpha^13)/(7^2 * pi^3)),.25)
            m_gtoCN.push_back(m_gtoCN.back());                                              // h-1 (same as h+1)

            m_gtoCN.push_back(m_gtoC[j] * tmp_h2p*tmpAlpha *pow((tmp*m_gtoA[j]),0.25));       // h+2  (2^3)/3 * pow((2^3 *alpha^13)/(pi^3)),.25)
            m_gtoCN.push_back(m_gtoC[j] * tmp_h2n*tmpAlpha *pow((tmp*m_gtoA[j]),0.25));       // h-2  (2^4)/3 * pow((2^3 *alpha^13)/(pi^3)),.25)

            m_gtoCN.push_back(m_gtoC[j] * tmp_h3*tmpAlpha *pow((tmp1*m_gtoA[j]),0.25));       // h+3  (2^2)/3 * pow((2*alpha^13)/(3^2*pi^3)),.25)
            m_gtoCN.push_back(m_gtoCN.back());                                                // h-3 (same as h+3)

            m_gtoCN.push_back(m_gtoC[j] * tmp_h4p*tmpAlpha *pow((tmp*m_gtoA[j]),0.25));       // h+4  (2^2)/sqrt(3) * pow((2^3 *alpha^13)/(pi^3)),.25)
            m_gtoCN.push_back(m_gtoC[j] * tmp_h4n*tmpAlpha *pow((tmp*m_gtoA[j]),0.25));       // h-4  (2^4)/sqrt(3) * pow((2^3 *alpha^13)/(pi^3)),.25)

            m_gtoCN.push_back(m_gtoC[j] * tmp_h5*tmpAlpha *pow((tmp1*m_gtoA[j]),0.25));       // h+5  (2^2)/sqrt(15) * pow((2*alpha^13)/(3^2*pi^3)),.25)
            m_gtoCN.push_back(m_gtoCN.back());                                                // h-5 (same as h+5)
        }

        break;
    }
    case I13:
    {
        // Spherical - 13 I components
        // Order in i0, i+1, i-1, i+2, i-2, i+3, i-3, i+4, i-4, i+5, i-5, i+6, i-6
        //
        m_moIndices[i] = indexMO;
        indexMO += 13;
        m_cIndices.push_back(m_gtoCN.size());

        // define some tmp value to avoid over/underflows during calculations but leave the used formulas visible
        //
        double tmp = (8./(M_PI*M_PI*M_PI));
        double tmp1 = (2./(M_PI*M_PI*M_PI));

        double tmp_i0  = 4./(3. *pow(1155.,.5));
        double tmp_i1 = 8./(3. *pow(55.,.5));

        double tmp_i2p = 4./(3. *pow(11.,.5));
        double tmp_i2n = 8./(3. *pow(11.,.5));

        double tmp_i4p = 4./pow(165.,.5);
        double tmp_i4n = 16./pow(165.,.5);

        double tmp_i5 = 8./pow(15,.5);

        double tmp_i6p = 4./(3. *pow(5.,.5));
        double tmp_i6n = 8./(3. *pow(5.,.5));

        for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            double  tmpAlpha = pow(m_gtoA[j], 3.0);

            m_gtoCN.push_back(m_gtoC[j] *  tmp_i0*tmpAlpha * pow((tmp*tmpAlpha),0.25));    // i0  (2^2)/3  * pow((2^3 * alpha^15)/(1155^2 * pi^3)),.25)

            m_gtoCN.push_back(m_gtoC[j] * tmp_i1*tmpAlpha *pow((tmp*tmpAlpha),0.25));      // i+1  (2^3)/3 * pow((2^3 *alpha^15)/(55^2 * pi^3)),.25)
            m_gtoCN.push_back(m_gtoCN.back());                                             // i-1  (same as i+1)

            m_gtoCN.push_back(m_gtoC[j] * tmp_i2p*tmpAlpha *pow((tmp1*tmpAlpha),0.25));    // i+2  (2^2)/3 * pow((2*alpha^15)/(11^2)*(pi^3)),.25)
            m_gtoCN.push_back(m_gtoC[j] * tmp_i2n*tmpAlpha *pow((tmp1*tmpAlpha),0.25));    // i-2  (2^3)/3 * pow((2*alpha^1i)/(11^2)*(pi^3)),.25)

            m_gtoCN.push_back(m_gtoCN.back());                                             // i+3  (same as i-2)
            m_gtoCN.push_back(m_gtoCN.back());                                             // i-3  (same as i-2)

            m_gtoCN.push_back(m_gtoC[j] * tmp_i4p*tmpAlpha *pow((tmp*tmpAlpha),0.25));     // i+4  (2^2)/sqrt(165) * pow((2^3 *alpha^15)/(pi^3)),.25)
            m_gtoCN.push_back(m_gtoC[j] * tmp_i4n*tmpAlpha *pow((tmp*tmpAlpha),0.25));     // i-4  (2^4)/sqrt(165) * pow((2^3 *alpha^15)/(pi^3)),.25)

            m_gtoCN.push_back(m_gtoC[j] * tmp_i5*tmpAlpha *pow((tmp1*tmpAlpha),0.25));     // i+5  (2^3)/sqrt(15) * pow((2*alpha^15)/(pi^3)),.25)
            m_gtoCN.push_back(m_gtoCN.back());                                             // i-5 (same as i+5)

            m_gtoCN.push_back(m_gtoC[j] * tmp_i6p*tmpAlpha *pow((tmp1*tmpAlpha),0.25));    // i+6  (2^2)/3 * pow((2*alpha^15)/(5^2)*(pi^3)),.25)
            m_gtoCN.push_back(m_gtoC[j] * tmp_i6n*tmpAlpha *pow((tmp1*tmpAlpha),0.25));    // i-6  (2^4)/3 * pow((2*alpha^15)/(5^2)(pi^3)),.25)
        }
        break;
    }
    default:
      qDebug() << "Basis set not handled - results may be incorrect.";
    }
  }
  m_init = true;
//  outputAll();
}
/// This is the stuff we actually use right now - porting to new data structure
void GaussianSet::processPoint(GaussianShell &shell)
{
  GaussianSet *set = shell.set;
  unsigned int atomsSize = set->m_numAtoms;
  unsigned int basisSize = set->m_symmetry.size();
  std::vector<int> &basis = set->m_symmetry;
  vector<Vector3d> deltas;
  vector<double> dr2;
  deltas.reserve(atomsSize);
  dr2.reserve(atomsSize);

  unsigned int indexMO = shell.state-1;

  // Calculate our position
  Vector3d pos = shell.tCube->position(shell.pos) * ANGSTROM_TO_BOHR;

  // Calculate the deltas for the position
  for (unsigned int i = 0; i < atomsSize; ++i) {
    deltas.push_back(pos - set->m_molecule.atomPos(i));
    dr2.push_back(deltas[i].squaredNorm());
  }

  // Now calculate the value at this point in space
  double tmp = 0.0;
  if (!set->m_useOrcaNorm) {
      for (unsigned int i = 0; i < basisSize; ++i) {
          switch(basis[i]) {
          case S:
              tmp += pointS(shell.set, i,
                            dr2[set->m_atomIndices[i]], indexMO);
              break;
          case P:
              tmp += pointP(shell.set, i, deltas[set->m_atomIndices[i]],
                      dr2[set->m_atomIndices[i]], indexMO);
              break;
          case D:
              tmp += pointD(shell.set, i, deltas[set->m_atomIndices[i]],
                      dr2[set->m_atomIndices[i]], indexMO);
              break;
          case D5:
              tmp += pointD5(shell.set, i, deltas[set->m_atomIndices[i]],
                      dr2[set->m_atomIndices[i]], indexMO);
              break;
          case F:
              tmp += pointF(shell.set, i, deltas[set->m_atomIndices[i]],
                      dr2[set->m_atomIndices[i]], indexMO);
              break;
          case F7:
              tmp += pointF7(shell.set, i, deltas[set->m_atomIndices[i]],
                      dr2[set->m_atomIndices[i]], indexMO);
              break;
          default:
              // Not handled - return a zero contribution
              ;
          }
      }
  } else {
      for (unsigned int i = 0; i < basisSize; ++i) {
        switch(basis[i]) {
        case S:
          tmp += pointS(shell.set, i,
                        dr2[set->m_atomIndices[i]], indexMO);
          break;
        case P:
          tmp += pointP(shell.set, i, deltas[set->m_atomIndices[i]],
                        dr2[set->m_atomIndices[i]], indexMO);
          break;
        case D5:
          tmp += pointOrcaD5(shell.set, i, deltas[set->m_atomIndices[i]],
                         dr2[set->m_atomIndices[i]], indexMO);
          break;
        case F7:
          tmp += pointOrcaF7(shell.set, i, deltas[set->m_atomIndices[i]],
                         dr2[set->m_atomIndices[i]], indexMO);
          break;
        case G9:
          tmp += pointOrcaG9(shell.set, i, deltas[set->m_atomIndices[i]],
                         dr2[set->m_atomIndices[i]], indexMO);
          break;
        case H11:
          tmp += pointOrcaH11(shell.set, i, deltas[set->m_atomIndices[i]],
                         dr2[set->m_atomIndices[i]], indexMO);
          break;
        case I13:
          tmp += pointOrcaI13(shell.set, i, deltas[set->m_atomIndices[i]],
                         dr2[set->m_atomIndices[i]], indexMO);
          break;
        default:
          // Not handled - return a zero contribution
          ;
        }
      }
  }

  // Set the value
  shell.tCube->setValue(shell.pos, tmp);
}

void GaussianSet::processDensity(GaussianShell &shell)
{
  GaussianSet *set = shell.set;
  unsigned int atomsSize = set->m_numAtoms;
  unsigned int basisSize = set->m_symmetry.size();
  unsigned int matrixSize = set->m_density.rows();
  std::vector<int> &basis = set->m_symmetry;
  vector<Vector3d> deltas;
  vector<double> dr2;
  deltas.reserve(atomsSize);
  dr2.reserve(atomsSize);

  // Calculate our position
  Vector3d pos = shell.tCube->position(shell.pos) * ANGSTROM_TO_BOHR;
  // Calculate the deltas for the position
  for (unsigned int i = 0; i < atomsSize; ++i) {
    deltas.push_back(pos - set->m_molecule.atomPos(i));
    dr2.push_back(deltas[i].squaredNorm());
  }

  // Calculate the basis set values at this point
  MatrixXd values(matrixSize, 1);
  if (!set->m_useOrcaNorm) {
      for (unsigned int i = 0; i < basisSize; ++i) {
          unsigned int cAtom = set->m_atomIndices[i];
          switch(basis[i]) {
          case S:
              pointS(shell.set, dr2[cAtom], i, values);
              break;
          case P:
              pointP(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          case D:
              pointD(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          case D5:
              pointD5(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          case F:
              pointF(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          case F7:
              pointF7(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          default:
              qDebug() << " unhandled function !!!! ";
              // Not handled - return a zero contribution
              ;
          }
      }
  } else {
      for (unsigned int i = 0; i < basisSize; ++i) {
          unsigned int cAtom = set->m_atomIndices[i];
          switch(basis[i]) {
          case S:
              pointS(shell.set, dr2[cAtom], i, values);
              break;
          case P:
              pointP(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          case D5:
              pointOrcaD5(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          case F7:
              pointOrcaF7(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          case G9:
              pointOrcaG9(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          case H11:
              pointOrcaH11(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          case I13:
              pointOrcaI13(shell.set, deltas[cAtom], dr2[cAtom], i, values);
              break;
          default:
              qDebug() << " unhandled function !!!! ";
              // Not handled - return a zero contribution
              ;
          }
      }
  }

  // Now calculate the value of the density at this point in space
  double rho = 0.0;
  for (unsigned int i = 0; i < matrixSize; ++i) {
    // Calculate the off-diagonal parts of the matrix
    for (unsigned int j = 0; j < i; ++j) {
      rho += 2.0 * set->m_density.coeffRef(i, j)
          * (values.coeffRef(i, 0) * values.coeffRef(j, 0));
    }
    // Now calculate the matrix diagonal
    rho += set->m_density.coeffRef(i, i)
        * (values.coeffRef(i, 0) * values.coeffRef(i, 0));
  }

  // Set the value
  shell.tCube->setValue(shell.pos, rho);
}

inline double GaussianSet::pointS(GaussianSet *set, unsigned int moIndex,
                                  double dr2, unsigned int indexMO)
{
  // If the MO coefficient is very small skip it
  if (isSmall(set->m_moMatrix.coeffRef(set->m_moIndices[moIndex], indexMO))) {
    return 0.0;
  }

  // S type orbitals - the simplest of the calculations with one component
  double tmp = 0.0;
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    tmp += set->m_gtoCN[cIndex++] * exp(-set->m_gtoA[i] * dr2);
  }
  // There is one MO coefficient per S shell basis
  return tmp * set->m_moMatrix.coeffRef(set->m_moIndices[moIndex], indexMO);
}

inline double GaussianSet::pointP(GaussianSet *set, unsigned int moIndex,
                                  const Vector3d &delta,
                                  double dr2, unsigned int indexMO)
{
  // P type orbitals have three components and each component has a different
  // independent MO weighting. Many things can be cached to save time though
  unsigned int baseIndex = set->m_moIndices[moIndex];
  double x = 0.0, y = 0.0, z = 0.0;

  // Now iterate through the P type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    x += set->m_gtoCN[cIndex++] * delta.x() * tmpGTO;
    y += set->m_gtoCN[cIndex++] * delta.y() * tmpGTO;
    z += set->m_gtoCN[cIndex++] * delta.z() * tmpGTO;
  }

  // Calculate the prefactors for Px, Py and Pz
  double Px = set->m_moMatrix.coeffRef(baseIndex  , indexMO);
  double Py = set->m_moMatrix.coeffRef(baseIndex+1, indexMO);
  double Pz = set->m_moMatrix.coeffRef(baseIndex+2, indexMO);

  return Px*x + Py*y + Pz*z;
}

inline double GaussianSet::pointD(GaussianSet *set, unsigned int moIndex,
                                  const Vector3d &delta,
                                  double dr2, unsigned int indexMO)
{
  // D type orbitals have six components and each component has a different
  // independent MO weighting. Many things can be cached to save time though
  unsigned int baseIndex = set->m_moIndices[moIndex];
  double xx = 0.0, yy = 0.0, zz = 0.0, xy = 0.0, xz = 0.0, yz = 0.0;

  // Now iterate through the D type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    xx += set->m_gtoCN[cIndex++] * tmpGTO; // Dxx
    yy += set->m_gtoCN[cIndex++] * tmpGTO; // Dyy
    zz += set->m_gtoCN[cIndex++] * tmpGTO; // Dzz
    xy += set->m_gtoCN[cIndex++] * tmpGTO; // Dxy
    xz += set->m_gtoCN[cIndex++] * tmpGTO; // Dxz
    yz += set->m_gtoCN[cIndex++] * tmpGTO; // Dyz
  }

  // Calculate the prefactors
  double Dxx = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * delta.x()
      * delta.x();
  double Dyy = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * delta.y()
      * delta.y();
  double Dzz = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * delta.z()
      * delta.z();
  double Dxy = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) * delta.x()
      * delta.y();
  double Dxz = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) * delta.x()
      * delta.z();
  double Dyz = set->m_moMatrix.coeffRef(baseIndex+5, indexMO) * delta.y()
      * delta.z();
  return Dxx*xx + Dyy*yy + Dzz*zz + Dxy*xy + Dxz*xz + Dyz*yz;
}

inline double GaussianSet::pointF(GaussianSet *set, unsigned int moIndex,
                                  const Vector3d &delta,
                                  double dr2, unsigned int indexMO)
{
  // F type orbitals have 10 components and each component has a different
  // independent MO weighting. Many things can be cached to save time though
  unsigned int baseIndex = set->m_moIndices[moIndex];
  double xxx = 0.0;
  double xxy = 0.0;
  double xxz = 0.0;
  double xyy = 0.0;
  double xyz = 0.0;
  double xzz = 0.0;
  double yyy = 0.0;
  double yyz = 0.0;
  double yzz = 0.0;
  double zzz = 0.0;

  // Now iterate through the D type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    xxx += set->m_gtoCN[cIndex++] * tmpGTO;
    xxy += set->m_gtoCN[cIndex++] * tmpGTO;
    xxz += set->m_gtoCN[cIndex++] * tmpGTO;
    xyy += set->m_gtoCN[cIndex++] * tmpGTO;
    xyz += set->m_gtoCN[cIndex++] * tmpGTO;
    xzz += set->m_gtoCN[cIndex++] * tmpGTO;
    yyy += set->m_gtoCN[cIndex++] * tmpGTO;
    yyz += set->m_gtoCN[cIndex++] * tmpGTO;
    yzz += set->m_gtoCN[cIndex++] * tmpGTO;
    zzz += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors
  double Fxxx = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * \
                delta.x() * delta.x() * delta.x();
  double Fxxy = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * \
                delta.x() * delta.x() * delta.y();
  double Fxxz = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * \
                delta.x() * delta.x() * delta.z();
  double Fxyy = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) * \
                delta.x() * delta.y() * delta.y();
  double Fxyz = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) * \
                delta.x() * delta.y() * delta.z();
  double Fxzz = set->m_moMatrix.coeffRef(baseIndex+5, indexMO) * \
                delta.x() * delta.z() * delta.z();
  double Fyyy = set->m_moMatrix.coeffRef(baseIndex+6, indexMO) * \
                delta.y() * delta.y() * delta.y();
  double Fyyz = set->m_moMatrix.coeffRef(baseIndex+7, indexMO) * \
                delta.y() * delta.y() * delta.z();
  double Fyzz = set->m_moMatrix.coeffRef(baseIndex+8, indexMO) * \
                delta.y() * delta.z() * delta.z();
  double Fzzz = set->m_moMatrix.coeffRef(baseIndex+9, indexMO) * \
                delta.z() * delta.z() * delta.z();

  return Fxxx*xxx + Fxxy*xxy + Fxxz*xxz + Fxyy*xyy + Fxyz*xyz \
        +Fxzz*xzz + Fyyy*yyy + Fyyz*yyz + Fyzz*yzz + Fzzz*zzz;
}

inline double GaussianSet::pointD5(GaussianSet *set, unsigned int moIndex,
                                   const Vector3d &delta,
                                   double dr2, unsigned int indexMO)
{
  // D type orbitals have five components and each component has a different
  // MO weighting. Many things can be cached to save time
  unsigned int baseIndex = set->m_moIndices[moIndex];
  double d0 = 0.0, d1p = 0.0, d1n = 0.0, d2p = 0.0, d2n = 0.0;

  // Now iterate through the D type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    d0  += set->m_gtoCN[cIndex++] * tmpGTO;
    d1p += set->m_gtoCN[cIndex++] * tmpGTO;
    d1n += set->m_gtoCN[cIndex++] * tmpGTO;
    d2p += set->m_gtoCN[cIndex++] * tmpGTO;
    d2n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors
  double xx = delta.x() * delta.x();
  double yy = delta.y() * delta.y();
  double zz = delta.z() * delta.z();
  double xy = delta.x() * delta.y();
  double xz = delta.x() * delta.z();
  double yz = delta.y() * delta.z();

//  double D0  = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * (zz - dr2);    <----- wrong formula - changed to 3*z^2 - r^2   by Dagmar Lenk
  double D0  = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * (3*zz - dr2);

  double D1p = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * xz;
  double D1n = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * yz;
  double D2p = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) * (xx - yy);
  double D2n = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) * xy;

  return D0*d0 + D1p*d1p + D1n*d1n + D2p*d2p + D2n*d2n;
}

inline double GaussianSet::pointF7(GaussianSet *set, unsigned int moIndex,
                                   const Vector3d &delta,
                                   double dr2, unsigned int indexMO)
{
  // Spherical F type orbitals have 7 components and each component has a different
  // MO weighting. Many things can be cached to save time
  unsigned int baseIndex = set->m_moIndices[moIndex];
  double f0 = 0.0, f1p = 0.0, f1n = 0.0, f2p = 0.0, f2n = 0.0, f3p = 0.0, f3n = 0.0;

  // Now iterate through the F type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    f0  += set->m_gtoCN[cIndex++] * tmpGTO;
    f1p += set->m_gtoCN[cIndex++] * tmpGTO;
    f1n += set->m_gtoCN[cIndex++] * tmpGTO;
    f2p += set->m_gtoCN[cIndex++] * tmpGTO;
    f2n += set->m_gtoCN[cIndex++] * tmpGTO;
    f3p += set->m_gtoCN[cIndex++] * tmpGTO;
    f3n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors
  double xxx = delta.x() * delta.x() * delta.x();
  double xxy = delta.x() * delta.x() * delta.y();
  double xxz = delta.x() * delta.x() * delta.z();
  double xyy = delta.x() * delta.y() * delta.y();
  double xyz = delta.x() * delta.y() * delta.z();
  double xzz = delta.x() * delta.z() * delta.z();
  double yyy = delta.y() * delta.y() * delta.y();
  double yyz = delta.y() * delta.y() * delta.z();
  double yzz = delta.y() * delta.z() * delta.z();
  double zzz = delta.z() * delta.z() * delta.z();

  double root6 = 2.449489742783178;
  double root60 = 7.745966692414834;
  double root360 = 18.973665961010276;

  double F0  = set->m_moMatrix.coeffRef(baseIndex  , indexMO) *  \
    (zzz - 3.0/2.0 * (xxz + yyz));
  double F1p = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) *  \
    ((6.0 * xzz - 3.0/2.0 * (xxx + xyy))/root6);
  double F1n = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) *  \
    ((6.0 * yzz - 3.0/2.0 * (xxy + yyy))/root6);
  double F2p = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) *  \
    ((15.0 * (xxz - yyz))/root60);
  double F2n = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) *  \
    ((30.0 * xyz)/root60);
  double F3p = set->m_moMatrix.coeffRef(baseIndex+5, indexMO) *  \
    ((15.0 * xxx - 45.0 * xyy)/root360);
  double F3n = set->m_moMatrix.coeffRef(baseIndex+6, indexMO) *  \
    ((45.0 * xxy - 15.0 * yyy)/root360);

  return  F0*f0 + F1p*f1p + F1n*f1n + F2p*f2p + F2n*f2n + F3p*f3p + F3n*f3n;
}

inline double GaussianSet::pointOrcaD5(GaussianSet *set, unsigned int moIndex,
                                   const Vector3d &delta,
                                   double dr2, unsigned int indexMO)
{
  // D type orbitals have five components and each component has a different
  // MO weighting. Many things can be cached to save time
  unsigned int baseIndex = set->m_moIndices[moIndex];
  double d0 = 0.0, d1p = 0.0, d1n = 0.0, d2p = 0.0, d2n = 0.0;

  // Now iterate through the D type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    d0  += set->m_gtoCN[cIndex++] * tmpGTO;
    d1p += set->m_gtoCN[cIndex++] * tmpGTO;
    d1n += set->m_gtoCN[cIndex++] * tmpGTO;
    d2p += set->m_gtoCN[cIndex++] * tmpGTO;
    d2n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors
  double xx = delta.x() * delta.x();
  double yy = delta.y() * delta.y();
  double zz = delta.z() * delta.z();
  double xy = delta.x() * delta.y();
  double xz = delta.x() * delta.z();
  double yz = delta.y() * delta.z();

  double D0  = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * (3*zz - dr2);
  double D1p = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * xz;
  double D1n = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * yz;
  double D2p = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) * (xx - yy);
  double D2n = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) * xy;

  return D0*d0 + D1p*d1p + D1n*d1n + D2p*d2p + D2n*d2n;
}

inline double GaussianSet::pointOrcaF7(GaussianSet *set, unsigned int moIndex,
                                   const Vector3d &delta,
                                   double dr2, unsigned int indexMO)
{
  // F type orbitals have 7 components and each component has a different
  // MO weighting. Many things can be cached to save time
  unsigned int baseIndex = set->m_moIndices[moIndex];
  double f0 = 0.0, f1p = 0.0, f1n = 0.0, f2p = 0.0, f2n = 0.0, f3p = 0.0, f3n = 0.0;

  // Now iterate through the F type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    f0  += set->m_gtoCN[cIndex++] * tmpGTO;
    f1p += set->m_gtoCN[cIndex++] * tmpGTO;
    f1n += set->m_gtoCN[cIndex++] * tmpGTO;
    f2p += set->m_gtoCN[cIndex++] * tmpGTO;
    f2n += set->m_gtoCN[cIndex++] * tmpGTO;
    f3p += set->m_gtoCN[cIndex++] * tmpGTO;
    f3n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors
  double xyz = delta.x() * delta.y() * delta.z();
  double xxx = delta.x() * delta.x() * delta.x();
  double yyy = delta.y() * delta.y() * delta.y();
  double zzz = delta.z() * delta.z() * delta.z();

  double xxz = delta.x() * delta.x() * delta.z();
  double xxy = delta.x() * delta.x() * delta.y();

  double yyz = delta.y() * delta.y() * delta.z();
  double yyx = delta.y() * delta.y() * delta.x();

  double zzx = delta.z() * delta.z() * delta.x();
  double zzy = delta.z() * delta.z() * delta.y();

  double F0  = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * (-3*xxz - 3*yyz + 2*zzz);
  double F1p = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * (-xxx - yyx + 4*zzx);
  double F1n = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * (-xxy -yyy + 4*zzy);
  double F2p = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) * (-yyz +xxz);
  double F2n = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) * xyz;
  double F3p = set->m_moMatrix.coeffRef(baseIndex+5, indexMO) * (-xxx + 3*yyx);
  double F3n = set->m_moMatrix.coeffRef(baseIndex+6, indexMO) *(-3*xxy + yyy);

  return  F0*f0 + F1p*f1p + F1n*f1n + F2p*f2p + F2n*f2n + F3p*f3p + F3n*f3n;
}

inline double GaussianSet::pointOrcaG9(GaussianSet *set, unsigned int moIndex,
                                   const Vector3d &delta,
                                   double dr2, unsigned int indexMO)
{
  // G type orbitals have 9 components and each component has a different
  // MO weighting. Many things can be cached to save time
  unsigned int baseIndex = set->m_moIndices[moIndex];
  double g0 = 0.0, g1p = 0.0, g1n = 0.0, g2p = 0.0, g2n = 0.0, g3p = 0.0, g3n = 0.0, g4p = 0.0, g4n = 0.0;

  // Now iterate through the G type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    g0  += set->m_gtoCN[cIndex++] * tmpGTO;
    g1p += set->m_gtoCN[cIndex++] * tmpGTO;
    g1n += set->m_gtoCN[cIndex++] * tmpGTO;
    g2p += set->m_gtoCN[cIndex++] * tmpGTO;
    g2n += set->m_gtoCN[cIndex++] * tmpGTO;
    g3p += set->m_gtoCN[cIndex++] * tmpGTO;
    g3n += set->m_gtoCN[cIndex++] * tmpGTO;
    g4p += set->m_gtoCN[cIndex++] * tmpGTO;
    g4n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors

  double xx = delta.x() * delta.x();
  double yy = delta.y() * delta.y();
  double zz = delta.z() * delta.z();

  double xxxx = xx*xx;
  double yyyy = yy*yy;
  double zzzz = zz*zz;

  double xz = delta.x() * delta.z();
  double xy = delta.x() * delta.y();
  double yz = delta.y() * delta.z();

  // NormG0 * (35*z^4 - 30*z^2*r^2 +3*r^4) *exp(-alpha*r^2)
  double G0  = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * (35*zzzz - 30*zz*dr2 +3*dr2*dr2);

  // NormG1p * x*z * (7*z^2 - 3*r^2) *exp(-alpha*r^2)
  // NormG1n * y*z * (7*z^2 - 3*r^2) *exp(-alpha*r^2)
  double G1tmp = (7*zz - 3*dr2);
  double G1p = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * G1tmp * xz;
  double G1n = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * G1tmp * yz;

  // NormG2p * (x^2 - y^2) * (7*z^2 - r^2) *exp(-alpha*r^2)
  // NormG2n * x * y * (7*z^2 - r^2) *exp(-alpha*r^2)
  double G2tmp = (7*zz - dr2);
  double G2p = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) * (xx - yy) * G2tmp;
  double G2n = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) * xy * G2tmp;

  // NormG3p * ... *exp(-alpha*r^2)
  // NormG3n * ... *exp(-alpha*r^2)
  double G3p = set->m_moMatrix.coeffRef(baseIndex+5, indexMO) * (xx - 3*yy) * xz;
  double G3n = set->m_moMatrix.coeffRef(baseIndex+6, indexMO) * (3*xx - yy) * yz;

  // NormG4p * (x^4 - 6*x^2*y^2 + y^4) *exp(-alpha*r^2)
  // NormG4n * x * y * (x^2 - y^2) *exp(-alpha*r^2)
  double G4p = set->m_moMatrix.coeffRef(baseIndex+7, indexMO) * (xxxx - 6*xx*yy + yyyy);
  double G4n = set->m_moMatrix.coeffRef(baseIndex+8, indexMO) * (xx - yy) * xy;

  return  G0*g0 + G1p*g1p + G1n*g1n + G2p*g2p + G2n*g2n + G3p*g3p + G3n*g3n + G4p*g4p + G4n*g4n;
}

inline double GaussianSet::pointOrcaH11(GaussianSet *set, unsigned int moIndex,
                                   const Vector3d &delta,
                                   double dr2, unsigned int indexMO)
{
  // H type orbitals have 11 components and each component has a different
  // MO weighting. Many things can be cached to save time
  unsigned int baseIndex = set->m_moIndices[moIndex];
  double h0 = 0.0, h1p = 0.0, h1n = 0.0, h2p = 0.0, h2n = 0.0, h3p = 0.0, h3n = 0.0, h4p = 0.0, h4n = 0.0, h5p = 0.0, h5n = 0.0;

  // Now iterate through the H type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    h0  += set->m_gtoCN[cIndex++] * tmpGTO;
    h1p += set->m_gtoCN[cIndex++] * tmpGTO;
    h1n += set->m_gtoCN[cIndex++] * tmpGTO;
    h2p += set->m_gtoCN[cIndex++] * tmpGTO;
    h2n += set->m_gtoCN[cIndex++] * tmpGTO;
    h3p += set->m_gtoCN[cIndex++] * tmpGTO;
    h3n += set->m_gtoCN[cIndex++] * tmpGTO;
    h4p += set->m_gtoCN[cIndex++] * tmpGTO;
    h4n += set->m_gtoCN[cIndex++] * tmpGTO;
    h5p += set->m_gtoCN[cIndex++] * tmpGTO;
    h5n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors

  double xx = delta.x() * delta.x();
  double yy = delta.y() * delta.y();
  double zz = delta.z() * delta.z();

  double xxxx = xx*xx;
  double yyyy = yy*yy;
  double zzzz = zz*zz;
  double xxyy = xx*yy;
//  double yyzz = yy*zz;
//  double xxzz = xx*zz;

  double x = delta.x();
  double y = delta.y();
  double z = delta.z();
  double xyz = x*y*z;

  // NormH0 * z * (63*z^4 - 70*z^2*r^2 + 15*r^4) *exp(-alpha*r^2)
  double H0  = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * z * (63*zzzz - 70*zz*dr2 +15*dr2*dr2);

  // NormH1p * x * (21*z^4 -14*z^2*r^2 + r^4) *exp(-alpha*r^2)
  // NormH1n * y * (21*z^4 -14*z^2*r^2 + r^4) *exp(-alpha*r^2)
  double H1tmp = 21*zzzz -14*zz*dr2 + dr2*dr2;
  double H1p = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * x * H1tmp;
  double H1n = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * y * H1tmp;

  // NormH2p * z * (x^2 - y^2) * (3*z^2 - r^2) *exp(-alpha*r^2)
  // NormH2n * x*y*z * (3*z^2 - r^2) *exp(-alpha*r^2)
  double H2tmp = (3*zz - dr2);
  double H2p = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) * z * (xx - yy) * H2tmp;
  double H2n = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) * xyz * H2tmp;

  // NormH3p * x * (x^2 - 3*y^2) * (9*z^2 - r^2) *exp(-alpha*r^2)
  // NormH3n * y * (3*x^2 - y^2) * (9*z^2 - r^2) *exp(-alpha*r^2)
  double H3tmp = (9*zz - dr2);
  double H3p = set->m_moMatrix.coeffRef(baseIndex+5, indexMO) * x * (xx - 3*yy) * H3tmp;
  double H3n = set->m_moMatrix.coeffRef(baseIndex+6, indexMO) * y * (3*xx - yy) * H3tmp;

  // NormH4p * z * (x^4 - 6*x^2*y^2 + y^4) *exp(-alpha*r^2)
  // NormH4n * x*y*z * (x^2 - y^2) *exp(-alpha*r^2)
  double H4p = set->m_moMatrix.coeffRef(baseIndex+7, indexMO) * z*(xxxx - 6*xxyy + yyyy);
  double H4n = set->m_moMatrix.coeffRef(baseIndex+8, indexMO) * xyz*(xx - yy);

  // NormH5p * x * (x^4 - 10*x^2*y^2 + 5*y^4) *exp(-alpha*r^2)
  // NormH5n * y * (5*x^4 - 10*x^2*y^2 + y^4) *exp(-alpha*r^2)
  double H5p = set->m_moMatrix.coeffRef(baseIndex+9, indexMO) * x * (xxxx - 10*xxyy + 5*yyyy);
  double H5n = set->m_moMatrix.coeffRef(baseIndex+10, indexMO) * y * (5*xxxx - 10*xxyy + yyyy);

  return  H0*h0 + H1p*h1p + H1n*h1n + H2p*h2p + H2n*h2n + H3p*h3p + H3n*h3n + H4p*h4p + H4n*h4n + H5p*h5p + H5n*h5n;
}


inline double GaussianSet::pointOrcaI13(GaussianSet *set, unsigned int moIndex,
                                   const Vector3d &delta,
                                   double dr2, unsigned int indexMO)
{
  // I type orbitals have 13 components and each component has a different
  // MO weighting. Many things can be cached to save time
  unsigned int baseIndex = set->m_moIndices[moIndex];
  double i0 = 0.0, i1p = 0.0, i1n = 0.0, i2p = 0.0, i2n = 0.0, i3p = 0.0, i3n = 0.0, i4p = 0.0, i4n = 0.0, i5p = 0.0, i5n = 0.0, i6p = 0.0, i6n = 0.0;

  // Now iterate through the I type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[moIndex];
  for (unsigned int i = set->m_gtoIndices[moIndex];
       i < set->m_gtoIndices[moIndex+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    i0  += set->m_gtoCN[cIndex++] * tmpGTO;
    i1p += set->m_gtoCN[cIndex++] * tmpGTO;
    i1n += set->m_gtoCN[cIndex++] * tmpGTO;
    i2p += set->m_gtoCN[cIndex++] * tmpGTO;
    i2n += set->m_gtoCN[cIndex++] * tmpGTO;
    i3p += set->m_gtoCN[cIndex++] * tmpGTO;
    i3n += set->m_gtoCN[cIndex++] * tmpGTO;
    i4p += set->m_gtoCN[cIndex++] * tmpGTO;
    i4n += set->m_gtoCN[cIndex++] * tmpGTO;
    i5p += set->m_gtoCN[cIndex++] * tmpGTO;
    i5n += set->m_gtoCN[cIndex++] * tmpGTO;
    i6p += set->m_gtoCN[cIndex++] * tmpGTO;
    i6n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors

  double xx = delta.x() * delta.x();
  double yy = delta.y() * delta.y();
  double zz = delta.z() * delta.z();

  double xy = delta.x() * delta.y();
  double xz = delta.x() * delta.z();
  double yz = delta.y() * delta.z();

  double x4 = xx*xx;
  double x6 = x4*xx;
  double y4 = yy*yy;
  double y6 = y4*yy;

  double z4 = zz*zz;
  double z6 = z4 * zz;
  double xxyy = xx*yy;


  // NormI0 * (231*z^6 - 315*z^4*r^2 + 105*z^2*r^4 -5*r^6) *exp(-alpha*r^2)
  double I0  = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * (231*z6 - 315*z4*dr2 + 105*zz*dr2*dr2 - 5*dr2*dr2*dr2);

  // NormI1p * x*z*(33z^4 - 30z^2*r^2 + 5*r^4) *exp(-alpha*r^2)
  // NormI1n * y*z*(33z^4 - 30z^2*r^2 + 5*r^4) *exp(-alpha*r^2)
  double I1tmp = 33.*z4 - 30.*zz*dr2 + 5.*dr2*dr2;
  double I1p = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * xz * I1tmp;
  double I1n = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * yz * I1tmp;

  // NormI2p * ((x^2-y^2)(33*z4 -18*zz*r^2 +r^4)) *exp(-alpha*r^2)
  // NormI2n * x*y*(33*z^4 - 18z^r^2 +r^4) *exp(-alpha*r^2)

  double I2tmp = 33*z4 - 18*zz*dr2 + dr2*dr2;
  double I2p = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) * (xx - yy) * I2tmp;
  double I2n = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) * xy * I2tmp;

  // NormI3p * x*z*(x^2 - 3y^2)*(11z^2 - 3r^2) *exp(-alpha*r^2)
  // NormI3n * y*z*(3x^2 - y^2)*(11z^2 - 3r^2) *exp(-alpha*r^2)
  double I3tmp = 11*zz - 3*dr2;
  double I3p = set->m_moMatrix.coeffRef(baseIndex+5, indexMO) * xz * (3*xx - yy) * I3tmp;
  double I3n = set->m_moMatrix.coeffRef(baseIndex+6, indexMO) * yz * (xx - 3*yy) * I3tmp;

  // NormI4p * (x^4 - 6*x^2*y^2 +y^4)*(11z^2 - r^2)*exp(-alpha*r^2)
  // NormI4n * x*y*(x^2 - y^2)(11z^2 - r^2) *exp(-alpha*r^2)
  double I4tmp = 11*zz - dr2;
  double I4p = set->m_moMatrix.coeffRef(baseIndex+7, indexMO) * (x4 - 6*xxyy +y4) * I4tmp;
  double I4n = set->m_moMatrix.coeffRef(baseIndex+8, indexMO) * xy*(xx - yy) * I4tmp;

  // NormI5p * xz(5x^4 - 10x^2*y^2 +y^4) *exp(-alpha*r^2)
  // NormI5n * y*z*(5x^4 - 10x^2*y^2 +y^4) *exp(-alpha*r^2)
  double I5tmp = 5*x4 - 10*xx*yy + y4;
  double I5p = set->m_moMatrix.coeffRef(baseIndex+9, indexMO) * xz * I5tmp;
  double I5n = set->m_moMatrix.coeffRef(baseIndex+10, indexMO) * yz * I5tmp;

  // NormI6p * (x^6 - 15x^4*y^2 + 15x^2y^4 - y^6) *exp(-alpha*r^2)
  // NormI6n * xy(3x^4 - 10x^2y^2 + 3y^4) *exp(-alpha*r^2)
  double I6p = set->m_moMatrix.coeffRef(baseIndex+11, indexMO) * (x6 - 15*x4*yy + 15*xx*y4 - y6);
  double I6n = set->m_moMatrix.coeffRef(baseIndex+12, indexMO) * xy * (3*x4 - 10*xx*yy + 3*y4);

  return  I0*i0 + I1p*i1p + I1n*i1n + I2p*i2p + I2n*i2n + I3p*i3p + I3n*i3n + I4p*i4p + I4n*i4n + I5p*i5p + I5n*i5n + I6p*i6p + I6n* i6n;
}




inline void GaussianSet::pointS(GaussianSet *set, double dr2, int basis,
                                Eigen::MatrixXd &out)
{
  // S type orbitals - the simplest of the calculations with one component
  double tmp = 0.0;
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    tmp += set->m_gtoCN[cIndex++] * exp(-set->m_gtoA[i] * dr2);
  }
  out.coeffRef(set->m_moIndices[basis], 0) = tmp;
}

inline void GaussianSet::pointP(GaussianSet *set, const Vector3d &delta,
                                double dr2, int basis,
                                Eigen::MatrixXd &out)
{
  double x = 0.0, y = 0.0, z = 0.0;

  // Now iterate through the P type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    x += set->m_gtoCN[cIndex++] * tmpGTO;
    y += set->m_gtoCN[cIndex++] * tmpGTO;
    z += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Save values to the matrix
  int baseIndex = set->m_moIndices[basis];
  out.coeffRef(baseIndex  , 0) = x * delta.x();
  out.coeffRef(baseIndex+1, 0) = y * delta.y();
  out.coeffRef(baseIndex+2, 0) = z * delta.z();
}

inline void GaussianSet::pointD(GaussianSet *set, const Eigen::Vector3d &delta,
                                double dr2, int basis,
                                Eigen::MatrixXd &out)
{
  // D type orbitals have six components and each component has a different
  // independent MO weighting. Many things can be cached to save time though
  double xx = 0.0, yy = 0.0, zz = 0.0, xy = 0.0, xz = 0.0, yz = 0.0;

  // Now iterate through the D type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    xx += set->m_gtoCN[cIndex++] * tmpGTO; // Dxx
    yy += set->m_gtoCN[cIndex++] * tmpGTO; // Dyy
    zz += set->m_gtoCN[cIndex++] * tmpGTO; // Dzz
    xy += set->m_gtoCN[cIndex++] * tmpGTO; // Dxy
    xz += set->m_gtoCN[cIndex++] * tmpGTO; // Dxz
    yz += set->m_gtoCN[cIndex++] * tmpGTO; // Dyz
  }

  // Save values to the matrix
  int baseIndex = set->m_moIndices[basis];
  out.coeffRef(baseIndex  , 0) = delta.x() * delta.x() * xx;
  out.coeffRef(baseIndex+1, 0) = delta.y() * delta.y() * yy;
  out.coeffRef(baseIndex+2, 0) = delta.z() * delta.z() * zz;
  out.coeffRef(baseIndex+3, 0) = delta.x() * delta.y() * xy;
  out.coeffRef(baseIndex+4, 0) = delta.x() * delta.z() * xz;
  out.coeffRef(baseIndex+5, 0) = delta.y() * delta.z() * yz;
}

inline void GaussianSet::pointF(GaussianSet *set, const Eigen::Vector3d &delta,
                                double dr2, int basis,
                                Eigen::MatrixXd &out)
{
  // F type orbitals have 10 components and each component has a different
  // independent MO weighting. Many things can be cached to save time though
  double xxx = 0.0;
  double xxy = 0.0;
  double xxz = 0.0;
  double xyy = 0.0;
  double xyz = 0.0;
  double xzz = 0.0;
  double yyy = 0.0;
  double yyz = 0.0;
  double yzz = 0.0;
  double zzz = 0.0;

  // Now iterate through the D type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    xxx += set->m_gtoCN[cIndex++] * tmpGTO;
    xxy += set->m_gtoCN[cIndex++] * tmpGTO;
    xxz += set->m_gtoCN[cIndex++] * tmpGTO;
    xyy += set->m_gtoCN[cIndex++] * tmpGTO;
    xyz += set->m_gtoCN[cIndex++] * tmpGTO;
    xzz += set->m_gtoCN[cIndex++] * tmpGTO;
    yyy += set->m_gtoCN[cIndex++] * tmpGTO;
    yyz += set->m_gtoCN[cIndex++] * tmpGTO;
    yzz += set->m_gtoCN[cIndex++] * tmpGTO;
    zzz += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Save values to the matrix
  int baseIndex = set->m_moIndices[basis];
  out.coeffRef(baseIndex  , 0) = delta.x() * delta.x() * delta.x() * xxx;
  out.coeffRef(baseIndex+1, 0) = delta.x() * delta.x() * delta.y() * xxy;
  out.coeffRef(baseIndex+2, 0) = delta.x() * delta.x() * delta.z() * xxz;
  out.coeffRef(baseIndex+3, 0) = delta.x() * delta.y() * delta.y() * xyy;
  out.coeffRef(baseIndex+4, 0) = delta.x() * delta.y() * delta.z() * xyz;
  out.coeffRef(baseIndex+5, 0) = delta.x() * delta.z() * delta.z() * xzz;
  out.coeffRef(baseIndex+6, 0) = delta.y() * delta.y() * delta.y() * yyy;
  out.coeffRef(baseIndex+7, 0) = delta.y() * delta.y() * delta.z() * yyz;
  out.coeffRef(baseIndex+8, 0) = delta.y() * delta.z() * delta.z() * yzz;
  out.coeffRef(baseIndex+9, 0) = delta.z() * delta.z() * delta.z() * zzz;
}

inline void GaussianSet::pointD5(GaussianSet *set, const Eigen::Vector3d &delta,
                                 double dr2, int basis,
                                 Eigen::MatrixXd &out)
{
  // spherical D type orbitals have 5 components and each component has a different
  // independent MO weighting. Many things can be cached to save time though
  double d0 = 0.0, d1p = 0.0, d1n = 0.0, d2p = 0.0, d2n = 0.0;

  // Now iterate through the D type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    d0  += set->m_gtoCN[cIndex++] * tmpGTO;
    d1p += set->m_gtoCN[cIndex++] * tmpGTO;
    d1n += set->m_gtoCN[cIndex++] * tmpGTO;
    d2p += set->m_gtoCN[cIndex++] * tmpGTO;
    d2n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors
  double xx = delta.x() * delta.x();
  double yy = delta.y() * delta.y();
  double zz = delta.z() * delta.z();
  double xy = delta.x() * delta.y();
  double xz = delta.x() * delta.z();
  double yz = delta.y() * delta.z();

  // Save values to the matrix
  int baseIndex = set->m_moIndices[basis];
  //  out.coeffRef(baseIndex  , 0) = (zz - dr2) * d0; <-- wrong formula - changed to 3*z^2 - r^2   by Dagmar Lenk
  out.coeffRef(baseIndex  , 0) = (3*zz - dr2) * d0;
  out.coeffRef(baseIndex+1, 0) = xz * d1p;
  out.coeffRef(baseIndex+2, 0) = yz * d1n;
  out.coeffRef(baseIndex+3, 0) = (xx - yy) * d2p;
  out.coeffRef(baseIndex+4, 0) = xy * d2n;
}

inline void GaussianSet::pointF7(GaussianSet *set, const Eigen::Vector3d &delta,
                                 double dr2, int basis,
                                 Eigen::MatrixXd &out)
{
  // spherical F type orbitals have 7 components and each component has a different
  // independent MO weighting. Many things can be cached to save time though
  double f0 = 0.0, f1p = 0.0, f1n = 0.0, f2p = 0.0, f2n = 0.0, f3p = 0.0, f3n = 0.0;

  // Now iterate through the D type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    f0  += set->m_gtoCN[cIndex++] * tmpGTO;
    f1p += set->m_gtoCN[cIndex++] * tmpGTO;
    f1n += set->m_gtoCN[cIndex++] * tmpGTO;
    f2p += set->m_gtoCN[cIndex++] * tmpGTO;
    f2n += set->m_gtoCN[cIndex++] * tmpGTO;
    f3p += set->m_gtoCN[cIndex++] * tmpGTO;
    f3n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors
  double xxx = delta.x() * delta.x() * delta.x();
  double xxy = delta.x() * delta.x() * delta.y();
  double xxz = delta.x() * delta.x() * delta.z();
  double xyy = delta.x() * delta.y() * delta.y();
  double xyz = delta.x() * delta.y() * delta.z();
  double xzz = delta.x() * delta.z() * delta.z();
  double yyy = delta.y() * delta.y() * delta.y();
  double yyz = delta.y() * delta.y() * delta.z();
  double yzz = delta.y() * delta.z() * delta.z();
  double zzz = delta.z() * delta.z() * delta.z();

  double root6 = 2.449489742783178;
  double root60 = 7.745966692414834;
  double root360 = 18.973665961010276;

  // Save values to the matrix
  int baseIndex = set->m_moIndices[basis];
  out.coeffRef(baseIndex  , 0) = f0*(zzz - 3.0/2.0 * (xxz + yyz));
  out.coeffRef(baseIndex+1, 0) = f1p*((6.0 * xzz - 3.0/2.0 * (xxx + xyy))/root6);
  out.coeffRef(baseIndex+2, 0) = f1n*((6.0 * yzz - 3.0/2.0 * (xxy + yyy))/root6);
  out.coeffRef(baseIndex+3, 0) = f2p*((15.0 * (xxz - yyz))/root60);
  out.coeffRef(baseIndex+4, 0) = f2n*((30.0 * xyz)/root60);
  out.coeffRef(baseIndex+5, 0) = f3p*((15.0 * xxx - 45.0 * xyy)/root360);
  out.coeffRef(baseIndex+6, 0) = f3n*((45.0 * xxy - 15.0 * yyy)/root360);
}

inline void GaussianSet::pointOrcaD5(GaussianSet *set, const Eigen::Vector3d &delta,
                                 double dr2, int basis,
                                 Eigen::MatrixXd &out)
{
  // D type orbitals have 5 components and each component has a different
  // independent MO weighting. Many things can be cached to save time though
  double d0 = 0.0, d1p = 0.0, d1n = 0.0, d2p = 0.0, d2n = 0.0;

  // Now iterate through the D type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    d0  += set->m_gtoCN[cIndex++] * tmpGTO;
    d1p += set->m_gtoCN[cIndex++] * tmpGTO;
    d1n += set->m_gtoCN[cIndex++] * tmpGTO;
    d2p += set->m_gtoCN[cIndex++] * tmpGTO;
    d2n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors
  double xx = delta.x() * delta.x();
  double yy = delta.y() * delta.y();
  double zz = delta.z() * delta.z();
  double xy = delta.x() * delta.y();
  double xz = delta.x() * delta.z();
  double yz = delta.y() * delta.z();

  // Save values to the matrix
  int baseIndex = set->m_moIndices[basis];

  out.coeffRef(baseIndex  , 0) = (3*zz - dr2) * d0;
  out.coeffRef(baseIndex+1, 0) = xz * d1p;
  out.coeffRef(baseIndex+2, 0) = yz * d1n;
  out.coeffRef(baseIndex+3, 0) = (xx - yy) * d2p;
  out.coeffRef(baseIndex+4, 0) = xy * d2n;
}
inline void GaussianSet::pointOrcaF7(GaussianSet *set, const Eigen::Vector3d &delta,
                    double dr2, int basis, Eigen::MatrixXd &out)
{
  // F type orbitals have 7 components and each component has a different
  // MO weighting. Many things can be cached to save time

  double f0 = 0.0, f1p = 0.0, f1n = 0.0, f2p = 0.0, f2n = 0.0, f3p = 0.0, f3n = 0.0;

  // Now iterate through the F type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    f0  += set->m_gtoCN[cIndex++] * tmpGTO;
    f1p += set->m_gtoCN[cIndex++] * tmpGTO;
    f1n += set->m_gtoCN[cIndex++] * tmpGTO;
    f2p += set->m_gtoCN[cIndex++] * tmpGTO;
    f2n += set->m_gtoCN[cIndex++] * tmpGTO;
    f3p += set->m_gtoCN[cIndex++] * tmpGTO;
    f3n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors
  double xyz = delta.x() * delta.y() * delta.z();
  double xxx = delta.x() * delta.x() * delta.x();
  double yyy = delta.y() * delta.y() * delta.y();
  double zzz = delta.z() * delta.z() * delta.z();

  double xxz = delta.x() * delta.x() * delta.z();
  double xxy = delta.x() * delta.x() * delta.y();

  double yyz = delta.y() * delta.y() * delta.z();
  double yyx = delta.y() * delta.y() * delta.x();

  double zzx = delta.z() * delta.z() * delta.x();
  double zzy = delta.z() * delta.z() * delta.y();

  // Save values to the matrix
  unsigned int baseIndex = set->m_moIndices[basis];
  out.coeffRef(baseIndex  , 0) = (-3*xxz - 3*yyz + 2*zzz) * f0;
  out.coeffRef(baseIndex+1, 0) = (-xxx - yyx + 4*zzx) *f1p;
  out.coeffRef(baseIndex+2, 0) = (-xxy -yyy + 4*zzy) *f1n;
  out.coeffRef(baseIndex+3, 0) = (-yyz +xxz) * f2p;
  out.coeffRef(baseIndex+4, 0) = xyz * f2n;
  out.coeffRef(baseIndex+5, 0) = (-xxx + 3*yyx) * f3p;
  out.coeffRef(baseIndex+6, 0) = (-3*xxy + yyy) *f3n;

}

inline void GaussianSet::pointOrcaG9(GaussianSet *set, const Eigen::Vector3d &delta,
                    double dr2, int basis, Eigen::MatrixXd &out)
{
  // G type orbitals have 9 components and each component has a different
  // MO weighting. Many things can be cached to save time
  unsigned int baseIndex = set->m_moIndices[basis];
  double g0 = 0.0, g1p = 0.0, g1n = 0.0, g2p = 0.0, g2n = 0.0, g3p = 0.0, g3n = 0.0, g4p = 0.0, g4n = 0.0;

  // Now iterate through the G type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    g0  += set->m_gtoCN[cIndex++] * tmpGTO;
    g1p += set->m_gtoCN[cIndex++] * tmpGTO;
    g1n += set->m_gtoCN[cIndex++] * tmpGTO;
    g2p += set->m_gtoCN[cIndex++] * tmpGTO;
    g2n += set->m_gtoCN[cIndex++] * tmpGTO;
    g3p += set->m_gtoCN[cIndex++] * tmpGTO;
    g3n += set->m_gtoCN[cIndex++] * tmpGTO;
    g4p += set->m_gtoCN[cIndex++] * tmpGTO;
    g4n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors

  double xx = delta.x() * delta.x();
  double yy = delta.y() * delta.y();
  double zz = delta.z() * delta.z();

  double xxxx = xx*xx;
  double yyyy = yy*yy;
  double zzzz = zz*zz;

  double xz = delta.x() * delta.z();
  double xy = delta.x() * delta.y();
  double yz = delta.y() * delta.z();

  // NormG0 * (35*z^4 - 30*z^2*r^2 +3*r^4) *exp(-alpha*r^2)
  out.coeffRef(baseIndex  , 0) = (35*zzzz - 30*zz*dr2 +3*dr2*dr2) *g0;

  // NormG1p * x*z * (7*z^2 - 3*r^2) *exp(-alpha*r^2)
  // NormG1n * y*z * (7*z^2 - 3*r^2) *exp(-alpha*r^2)
  double G1tmp = (7*zz - 3*dr2);
  out.coeffRef(baseIndex+1, 0) = G1tmp * xz * g1p;
  out.coeffRef(baseIndex+2, 0) = G1tmp * yz * g1n;

  // NormG2p * (x^2 - y^2) * (7*z^2 - r^2) *exp(-alpha*r^2)
  // NormG2n * x * y * (7*z^2 - r^2) *exp(-alpha*r^2)
  double G2tmp = (7*zz - dr2);
  out.coeffRef(baseIndex+3, 0) = (xx - yy) * G2tmp * g2p;
  out.coeffRef(baseIndex+4, 0) = xy * G2tmp * g2n;

  // NormG3p * ... *exp(-alpha*r^2)
  // NormG3n * ... *exp(-alpha*r^2)
  out.coeffRef(baseIndex+5, 0) = (xx - 3*yy) * xz * g3p;
  out.coeffRef(baseIndex+6, 0) = (3*xx - yy) * yz * g3n;

  // NormG4p * (x^4 - 6*x^2*y^2 + y^4) *exp(-alpha*r^2)
  // NormG4n * x * y * (x^2 - y^2) *exp(-alpha*r^2)
  out.coeffRef(baseIndex+7, 0) = (xxxx - 6*xx*yy + yyyy) * g4p;
  out.coeffRef(baseIndex+8, 0) = (xx - yy) * xy * g4n;
}

inline void GaussianSet::pointOrcaH11(GaussianSet *set, const Eigen::Vector3d &delta,
                                   double dr2, int basis, Eigen::MatrixXd &out)
{
  // H type orbitals have 11 components and each component has a different
  // MO weighting. Many things can be cached to save time
  unsigned int baseIndex = set->m_moIndices[basis];
  double h0 = 0.0, h1p = 0.0, h1n = 0.0, h2p = 0.0, h2n = 0.0, h3p = 0.0, h3n = 0.0, h4p = 0.0, h4n = 0.0, h5p = 0.0, h5n = 0.0;

  // Now iterate through the H type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    h0  += set->m_gtoCN[cIndex++] * tmpGTO;
    h1p += set->m_gtoCN[cIndex++] * tmpGTO;
    h1n += set->m_gtoCN[cIndex++] * tmpGTO;
    h2p += set->m_gtoCN[cIndex++] * tmpGTO;
    h2n += set->m_gtoCN[cIndex++] * tmpGTO;
    h3p += set->m_gtoCN[cIndex++] * tmpGTO;
    h3n += set->m_gtoCN[cIndex++] * tmpGTO;
    h4p += set->m_gtoCN[cIndex++] * tmpGTO;
    h4n += set->m_gtoCN[cIndex++] * tmpGTO;
    h5p += set->m_gtoCN[cIndex++] * tmpGTO;
    h5n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors

  double xx = delta.x() * delta.x();
  double yy = delta.y() * delta.y();
  double zz = delta.z() * delta.z();

  double xxxx = xx*xx;
  double yyyy = yy*yy;
  double zzzz = zz*zz;
  double xxyy = xx*yy;
//  double yyzz = yy*zz;
//  double xxzz = xx*zz;

  double x = delta.x();
  double y = delta.y();
  double z = delta.z();
  double xyz = x*y*z;

  // NormH0 * z * (63*z^4 - 70*z^2*r^2 + 15*r^4) *exp(-alpha*r^2)
  out.coeffRef(baseIndex  , 0) = z * (63*zzzz - 70*zz*dr2 +15*dr2*dr2) * h0;

  // NormH1p * x * (21*z^4 -14*z^2*r^2 + r^4) *exp(-alpha*r^2)
  // NormH1n * y * (21*z^4 -14*z^2*r^2 + r^4) *exp(-alpha*r^2)
  double H1tmp = 21*zzzz -14*zz*dr2 + dr2*dr2;
  out.coeffRef(baseIndex+1, 0) = x * H1tmp * h1p;
  out.coeffRef(baseIndex+2, 0) = y * H1tmp * h1n;

  // NormH2p * z * (x^2 - y^2) * (3*z^2 - r^2) *exp(-alpha*r^2)
  // NormH2n * x*y*z * (3*z^2 - r^2) *exp(-alpha*r^2)
  double H2tmp = (3*zz - dr2);
  out.coeffRef(baseIndex+3, 0) = z * (xx - yy) * H2tmp * h2p;
  out.coeffRef(baseIndex+4, 0) = xyz * H2tmp * h2n;

  // NormH3p * x * (x^2 - 3*y^2) * (9*z^2 - r^2) *exp(-alpha*r^2)
  // NormH3n * y * (3*x^2 - y^2) * (9*z^2 - r^2) *exp(-alpha*r^2)
  double H3tmp = (9*zz - dr2);
  out.coeffRef(baseIndex+5, 0) = x * (xx - 3*yy) * H3tmp * h3p;
  out.coeffRef(baseIndex+6, 0) = y * (3*xx - yy) * H3tmp * h3n;

  // NormH4p * z * (x^4 - 6*x^2*y^2 + y^4) *exp(-alpha*r^2)
  // NormH4n * x*y*z * (x^2 - y^2) *exp(-alpha*r^2)
  out.coeffRef(baseIndex+7, 0) = z*(xxxx - 6*xxyy + yyyy) * h4p;
  out.coeffRef(baseIndex+8, 0) = xyz*(xx - yy) * h4n;

  // NormH5p * x * (x^4 - 10*x^2*y^2 + 5*y^4) *exp(-alpha*r^2)
  // NormH5n * y * (5*x^4 - 10*x^2*y^2 + y^4) *exp(-alpha*r^2)
  out.coeffRef(baseIndex+9, 0) = x * (xxxx - 10*xxyy + 5*yyyy) * h5p;
  out.coeffRef(baseIndex+10, 0) = y * (5*xxxx - 10*xxyy + yyyy) * h5n;

 }

inline void GaussianSet::pointOrcaI13(GaussianSet *set, const Eigen::Vector3d &delta,
                                  double dr2, int basis, Eigen::MatrixXd &out)
{
  // I type orbitals have 13 components and each component has a different
  // MO weighting. Many things can be cached to save time
  unsigned int baseIndex = set->m_moIndices[basis];
  double i0 = 0.0, i1p = 0.0, i1n = 0.0, i2p = 0.0, i2n = 0.0, i3p = 0.0, i3n = 0.0, i4p = 0.0, i4n = 0.0, i5p = 0.0, i5n = 0.0, i6p = 0.0, i6n = 0.0;

  // Now iterate through the I type GTOs and sum their contributions
  unsigned int cIndex = set->m_cIndices[basis];
  for (unsigned int i = set->m_gtoIndices[basis];
       i < set->m_gtoIndices[basis+1]; ++i) {
    // Calculate the common factor
    double tmpGTO = exp(-set->m_gtoA[i] * dr2);
    i0  += set->m_gtoCN[cIndex++] * tmpGTO;
    i1p += set->m_gtoCN[cIndex++] * tmpGTO;
    i1n += set->m_gtoCN[cIndex++] * tmpGTO;
    i2p += set->m_gtoCN[cIndex++] * tmpGTO;
    i2n += set->m_gtoCN[cIndex++] * tmpGTO;
    i3p += set->m_gtoCN[cIndex++] * tmpGTO;
    i3n += set->m_gtoCN[cIndex++] * tmpGTO;
    i4p += set->m_gtoCN[cIndex++] * tmpGTO;
    i4n += set->m_gtoCN[cIndex++] * tmpGTO;
    i5p += set->m_gtoCN[cIndex++] * tmpGTO;
    i5n += set->m_gtoCN[cIndex++] * tmpGTO;
    i6p += set->m_gtoCN[cIndex++] * tmpGTO;
    i6n += set->m_gtoCN[cIndex++] * tmpGTO;
  }

  // Calculate the prefactors

  double xx = delta.x() * delta.x();
  double yy = delta.y() * delta.y();
  double zz = delta.z() * delta.z();

  double xy = delta.x() * delta.y();
  double xz = delta.x() * delta.z();
  double yz = delta.y() * delta.z();

  double x4 = xx*xx;
  double x6 = x4*xx;
  double y4 = yy*yy;
  double y6 = y4*yy;

  double z4 = zz*zz;
  double z6 = z4 * zz;
  double xxyy = xx*yy;


  // NormI0 * (231*z^6 - 315*z^4*r^2 + 105*z^2*r^4 -5*r^6) *exp(-alpha*r^2)
  out.coeffRef(baseIndex  , 0) = (231*z6 - 315*z4*dr2 + 105*zz*dr2*dr2 - 5*dr2*dr2*dr2) * i0;

  // NormI1p * x*z*(33z^4 - 30z^2*r^2 + 5*r^4) *exp(-alpha*r^2)
  // NormI1n * y*z*(33z^4 - 30z^2*r^2 + 5*r^4) *exp(-alpha*r^2)
  double I1tmp = 33.*z4 - 30.*zz*dr2 + 5.*dr2*dr2;
  out.coeffRef(baseIndex+1, 0) = xz * I1tmp * i1p;
  out.coeffRef(baseIndex+2, 0) = yz * I1tmp * i1n;

  // NormI2p * ((x^2-y^2)(33*z4 -18*zz*r^2 +r^4)) *exp(-alpha*r^2)
  // NormI2n * x*y*(33*z^4 - 18z^r^2 +r^4) *exp(-alpha*r^2)

  double I2tmp = 33*z4 - 18*zz*dr2 + dr2*dr2;
  out.coeffRef(baseIndex+3, 0) = (xx - yy) * I2tmp * i2p;
  out.coeffRef(baseIndex+4, 0) = xy * I2tmp * i2n;

  // NormI3p * x*z*(x^2 - 3y^2)*(11z^2 - 3r^2) *exp(-alpha*r^2)
  // NormI3n * y*z*(3x^2 - y^2)*(11z^2 - 3r^2) *exp(-alpha*r^2)
  double I3tmp = 11*zz - 3*dr2;
  out.coeffRef(baseIndex+5, 0) = xz * (3*xx - yy) * I3tmp * i3p;
  out.coeffRef(baseIndex+6, 0) = yz * (xx - 3*yy) * I3tmp * i3n;

  // NormI4p * (x^4 - 6*x^2*y^2 +y^4)*(11z^2 - r^2)*exp(-alpha*r^2)
  // NormI4n * x*y*(x^2 - y^2)(11z^2 - r^2) *exp(-alpha*r^2)
  double I4tmp = 11*zz - dr2;
  out.coeffRef(baseIndex+7, 0) = (x4 - 6*xxyy +y4) * I4tmp * i4p;
  out.coeffRef(baseIndex+8, 0) = xy*(xx - yy) * I4tmp * i4n;

  // NormI5p * xz(5x^4 - 10x^2*y^2 +y^4) *exp(-alpha*r^2)
  // NormI5n * y*z*(5x^4 - 10x^2*y^2 +y^4) *exp(-alpha*r^2)
  double I5tmp = 5*x4 - 10*xx*yy + y4;
  out.coeffRef(baseIndex+9, 0) = xz * I5tmp * i5p;
  out.coeffRef(baseIndex+10, 0) = yz * I5tmp * i5n;

  // NormI6p * (x^6 - 15x^4*y^2 + 15x^2y^4 - y^6) *exp(-alpha*r^2)
  // NormI6n * xy(3x^4 - 10x^2y^2 + 3y^4) *exp(-alpha*r^2)
  out.coeffRef(baseIndex+11, 0) = (x6 - 15*x4*yy + 15*xx*y4 - y6) * i6p;
  out.coeffRef(baseIndex+12, 0) = xy * (3*x4 - 10*xx*yy + 3*y4) * i6n;

}



unsigned int GaussianSet::numMOs()
{
  // Return the total number of MOs
  return m_moMatrix.rows();
}

void GaussianSet::outputAll()
{
  // Can be called to print out a summary of the basis set as read in
  qDebug() << "\nGaussian Basis Set\nNumber of atoms:" << m_numAtoms;

  initCalculation();

  int tmpOrbital = 0;
  if (!isValid()) {
    qDebug() << "Basis set is marked as invalid.";
    return;
  }

  for (uint i = 0; i < m_symmetry.size(); ++i) {
    qDebug() << i
             << "\tAtom Index:" << m_atomIndices[i]
             << "\tSymmetry:" << m_symmetry[i]
             << "\tMO Index:" << m_moIndices[i]
             << "\tGTO Index:" << m_gtoIndices[i];
  }
  qDebug() << "Symmetry:" << m_symmetry.size()
           << "\tgtoIndices:" << m_gtoIndices.size()
           << "\tLast gtoIndex:" << m_gtoIndices[m_symmetry.size()]
           << "\ngto size:" << m_gtoA.size() << m_gtoC.size() << m_gtoCN.size();
  for (uint i = 0; i < m_symmetry.size(); ++i) {
    switch(m_symmetry[i]) {
    case S:
      qDebug() << "Shell" << i << "\tS\n  MO 1\t"
               << m_moMatrix(  m_moIndices[i], tmpOrbital);
      break;
    case P:
      qDebug() << "Shell" << i << "\tP\n  MO 1\t"
               << m_moMatrix( m_moIndices[i], tmpOrbital)
               << "\t" << m_moMatrix(  m_moIndices[i] + 1,tmpOrbital)
               << "\t" << m_moMatrix( m_moIndices[i] + 2, tmpOrbital);
      break;
    case D:
      qDebug() << "Shell" << i << "\tD\n  MO 1\t"
               << m_moMatrix(  m_gtoIndices[i],tmpOrbital)
               << "\t" << m_moMatrix( m_moIndices[i] + 1, tmpOrbital)
               << "\t" << m_moMatrix(  m_moIndices[i] + 2, tmpOrbital)
               << "\t" << m_moMatrix(  m_moIndices[i] + 3, tmpOrbital)
               << "\t" << m_moMatrix(  m_moIndices[i] + 4, tmpOrbital)
               << "\t" << m_moMatrix(  m_moIndices[i] + 5, tmpOrbital);
        break;
    case D5:
        qDebug() << "Shell" << i << "\tD5\n  MO 1\t"
                 << m_moMatrix( m_moIndices[i], tmpOrbital)
                 << "\t" << m_moMatrix(  m_moIndices[i]+ 1, tmpOrbital)
                 << "\t" << m_moMatrix(  m_moIndices[i] + 2, tmpOrbital)
                 << "\t" << m_moMatrix(m_moIndices[i] + 3, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 4, tmpOrbital);
        break;
    case F:
        qDebug() << "Shell" << i << "\tF\n  MO 1\t"
                 << m_moMatrix( m_moIndices[i], tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 1, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 2, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 3, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 4, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 5, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 6, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 7, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 8, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 9, tmpOrbital);
        break;
    case F7:
        qDebug() << "Shell" << i << "\tF7\n  MO 1\t"
                 << m_moMatrix( m_moIndices[i], tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 1, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 2, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 3, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 4, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 5, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 6, tmpOrbital);
      break;
    case G9:
        qDebug() << "Shell" << i << "\tG9\n  MO 1\t"
                 << m_moMatrix( m_moIndices[i],tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 1, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 2, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 3, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 4, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 5, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 6, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 7, tmpOrbital)
                 << "\t" << m_moMatrix( m_moIndices[i] + 8, tmpOrbital);
      break;
    default:
      qDebug() << "Error: unhandled type...";
    }
    unsigned int cIndex = m_gtoIndices[i];
    for (uint j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
      if (j >= m_gtoA.size()) {
        qDebug() << "Error, j is too large!" << j << m_gtoA.size();
        continue;
      }
      qDebug() << cIndex
               << "\tc:" << m_gtoC[cIndex]
               << "\ta:" << m_gtoA[cIndex];
      ++cIndex;
    }
  }
  qDebug() << "\nEnd of orbital data...\n";
}

}
