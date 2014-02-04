/**********************************************************************
  AvoTubeGen -- interface between Avogadro and TubeGen

  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 **********************************************************************/

#include "avotubegen.h"

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

#include <openbabel/atom.h>
#include <openbabel/obiter.h>
#include <openbabel/generic.h>
#include <openbabel/math/vector3.h>
#include <openbabel/mol.h>

#include <QtCore/qalgorithms.h>
#include <QtCore/QHash>
#include <QtCore/QVector>

#include "tubegen/TubuleBasis.h"
#include "tubegen/CrystalCell.h"

// Needed for OB macros
using OpenBabel::OBMolAtomIter;

namespace SWCNTBuilder
{

AvoTubeGen::AvoTubeGen(QObject *parent)
  : QObject(parent),
    m_molecule(NULL)
{
}

AvoTubeGen::~AvoTubeGen()
{
}

void AvoTubeGen::buildCarbonNanotube(unsigned int n, unsigned int m,
                                     bool periodicCell, double length,
                                     bool cap, bool dbonds)
{
  // Create new molecule
  m_molecule = new Avogadro::Molecule();

  TubuleBasis tube (n, m);
  CrystalCell *cell = tube.GenerateCrystalCell();

  TVector3D tv1 = cell->GetRealBasisVector1();
  TVector3D tv2 = cell->GetRealBasisVector2();
  TVector3D tv3 = cell->GetRealBasisVector3();

  OpenBabel::vector3 v1 (tv1.x, tv1.y, tv1.z);
  OpenBabel::vector3 v2 (tv2.x, tv2.y, tv2.z);
  OpenBabel::vector3 v3 (tv3.x, tv3.y, tv3.z);

  OpenBabel::OBUnitCell *obcell = new OpenBabel::OBUnitCell();
  obcell->SetData(v1, v2, v3);
  m_molecule->setOBUnitCell(obcell);

  TAtomicCoordinate coord;
  OpenBabel::vector3 fracCoord, cartCoord;

  for (unsigned int i = 0; i < cell->GetBasisCount(); ++i) {
    coord = cell->GetAtomicCoordinate(i);
    fracCoord.Set(
          coord.atomPosition.x, coord.atomPosition.y, coord.atomPosition.z);
    cartCoord = obcell->FractionalToCartesian(fracCoord);

    Avogadro::Atom *atom = m_molecule->addAtom();
    atom->setAtomicNumber(coord.atomicNumber);
    atom->setPos(Eigen::Vector3d(cartCoord.AsArray()));
  }

  // Determine how many periodic images are needed:
  bool integerPeriodicity = (periodicCell)
      ? (fabs(length - floor((length * 100.0) + 0.5)/ 100.0) < 1e-2)
      : false;
  const unsigned int numTranslations = (periodicCell)
      ? ((integerPeriodicity) ? static_cast<unsigned int>(length + 0.5)
                              : ceil(length))
      : ceil(length / v3.z());

  // Determine the maximum z value
  const double maxz = (periodicCell)
      ? ((integerPeriodicity) ? 0.0 /* no trimming */ : length * v3.z())
      : length;

  // Grow tube as needed
  if (numTranslations > 1) {
    this->addTranslationalUnits(numTranslations, maxz);
  }

  // If requested length is smaller than a single translational unit,
  // trim tube to size
  if (!integerPeriodicity && maxz < v3.z()) {
    this->trimTube(maxz);
  }

  // Remove the lattice (no longer needed for non-periodic tube)
  if (length != 1 || cap) {
    m_molecule->setOBUnitCell(NULL);
  }

  // Perceive single bonds
  this->perceiveSingleBonds();

  // Cap if needed
  if (cap) {
    this->capTube();
  }

  // Perceive double bonds
  if (dbonds) {
    this->perceiveDoubleBonds();
  }

  emit buildFinished();
}

void AvoTubeGen::addTranslationalUnits(unsigned int num, double maxz)
{
  if (num == 0) {
    m_molecule->clear();
    return;
  }

  if (num == 1) {
    return;
  }

  // Do we need to cut off the cell at a custom length?
  bool usingCutoff = true;
  if (fabs(maxz) < 1e-2) {
    usingCutoff = false;
  }

  // Fetch the c translation vector
  Q_ASSERT(m_molecule->OBUnitCell() != NULL);
  Eigen::Vector3d v3 (m_molecule->OBUnitCell()->GetCellVectors()[2].AsArray());

  // Cache current atom list
  QList<Avogadro::Atom*> atoms = m_molecule->atoms();

  // Add atoms
  Eigen::Vector3d newPos;
  for (unsigned int i = 1; i < num; ++i) {
    const Eigen::Vector3d trans = v3 * static_cast<double>(i);
    for (QList<Avogadro::Atom*>::const_iterator it = atoms.constBegin(),
         it_end = atoms.constEnd(); it != it_end; ++it) {
      Avogadro::Atom const *oldAtom = *it;

      // calculate and check new coordinate
      newPos = *oldAtom->pos() + trans;
      if (usingCutoff && newPos.z() > maxz) {
        continue;
      }

      Avogadro::Atom *newAtom = m_molecule->addAtom();

      *newAtom = *oldAtom;
      newAtom->setPos(newPos);
    }
  }

  // Done!
  return;
}

void AvoTubeGen::trimTube(double maxz)
{
  QList<Avogadro::Atom*> atoms = m_molecule->atoms();

  for (QList<Avogadro::Atom*>::const_iterator it = atoms.constBegin(),
       it_end = atoms.constEnd(); it != it_end; ++it) {
    if ((*it)->pos()->z() > maxz) {
      m_molecule->removeAtom(*it);
    }
  }
}

void AvoTubeGen::capTube()
{
  // Quick bond perception:
  OpenBabel::OBMol obmol = m_molecule->OBMol();

  // Can't modify obmol in the FOR_ATOMS... loop, so cache the unsaturated
  // atoms here
  QVector<OpenBabel::OBAtom*> needSaturation;

  // Add hydrogens to all carbons with < 3 bonds
  FOR_ATOMS_OF_MOL(obatom, &obmol) {
    // Only single bonds result from ConnectTheDots
    int numBonds = obatom->CountBondsOfOrder(1);
    if (numBonds < 3) {
      needSaturation.append(&*obatom);
    }
  }
  // Add hydrogens to all in needSaturation
  for (QVector<OpenBabel::OBAtom*>::const_iterator
       it = needSaturation.constBegin(), it_end = needSaturation.constEnd();
       it != it_end; ++it) {
    OpenBabel::OBAtom *obatom = *it;
    // Only single bonds result from ConnectTheDots
    int numBonds = obatom->CountBondsOfOrder(1);
    // We know that we are dealing with sp2 hybridized carbons:
    if (numBonds == 2) {
      obatom->SetImplicitValence(obatom->GetValence() + 1);
      obmol.SetImplicitValencePerceived();
      obatom->SetHyb(2);
      obmol.SetHybridizationPerceived();
    }
    else if (numBonds == 1) {
      obatom->SetImplicitValence(obatom->GetValence() + 2);
      obmol.SetImplicitValencePerceived();
      obatom->SetHyb(2);
      obmol.SetHybridizationPerceived();
    }

    obmol.AddHydrogens(obatom);
  }

  // Update the molecule
  m_molecule->setOBMol(&obmol);
}

void AvoTubeGen::perceiveSingleBonds()
{
  OpenBabel::OBMol obmol = m_molecule->OBMol();
  obmol.ConnectTheDots();
  m_molecule->setOBMol(&obmol);
}

void AvoTubeGen::perceiveDoubleBonds()
{
  OpenBabel::OBMol obmol = m_molecule->OBMol();
  obmol.PerceiveBondOrders();
  m_molecule->setOBMol(&obmol);
}

} // end namespace SWCNTBuilder
