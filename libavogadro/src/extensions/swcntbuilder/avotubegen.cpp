/**********************************************************************
  AvoTubeGen -- interface between Avogadro and TubeGen

  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 **********************************************************************/

#include "avotubegen.h"

#include <avogadro/atom.h>
#include <avogadro/molecule.h>

#include <openbabel/atom.h>
#include <openbabel/obiter.h>
#include <openbabel/generic.h>
#include <openbabel/math/vector3.h>
#include <openbabel/mol.h>

#include <QtCore/qalgorithms.h>
#include <QtCore/QVector>

#include "tubegen/TubuleBasis.h"
#include "tubegen/CrystalCell.h"

// Anonymous namespace for helper functions
namespace {

void adjustLength(Avogadro::Molecule *mol, unsigned int length)
{
  if (length == 0) {
    mol->clear();
    return;
  }

  if (length == 1) {
    return;
  }

  // Fetch the c translation vector
  Q_ASSERT(mol->OBUnitCell() != NULL);
  Eigen::Vector3d v3 (mol->OBUnitCell()->GetCellVectors()[2].AsArray() );

  // Cache current atom list
  QList<Avogadro::Atom*> atoms = mol->atoms();

  // Add atoms
  for (unsigned int i = 1; i < length; ++i) {
    const Eigen::Vector3d trans = v3 * static_cast<double>(i);
    for (QList<Avogadro::Atom*>::const_iterator it = atoms.constBegin(),
         it_end = atoms.constEnd(); it != it_end; ++it) {
      Avogadro::Atom const *oldAtom = *it;
      Avogadro::Atom *newAtom = mol->addAtom();

      *newAtom = *oldAtom;
      newAtom->setPos(*oldAtom->pos() + trans);
    }
  }

  // Done!
  return;
}

void capTube(Avogadro::Molecule *mol)
{
  // Quick bond perception:
  OpenBabel::OBMol obmol = mol->OBMol();
  obmol.ConnectTheDots();

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
    // We know that we are dealing with sp2 hybridized carbons:
    obatom->SetImplicitValence(obatom->GetValence() + 1);
    obmol.SetImplicitValencePerceived();
    obatom->SetHyb(2);
    obmol.SetHybridizationPerceived();

    obmol.AddHydrogens(obatom);
  }

  // Update the molecule
  mol->setOBMol(&obmol);
}

void perceiveBonds(Avogadro::Molecule *mol)
{
  OpenBabel::OBMol obmol = mol->OBMol();
  obmol.ConnectTheDots();
  mol->setOBMol(&obmol);
}

}

namespace SWCNTBuilder
{
namespace AvoTubeGen
{

Avogadro::Molecule * getCarbonNanotube(unsigned int n, unsigned int m,
                                       unsigned int length,
                                       bool cap)
{
  // Create new molecule
  Avogadro::Molecule *mol = new Avogadro::Molecule();

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
  mol->setOBUnitCell(obcell);

  TAtomicCoordinate coord;
  OpenBabel::vector3 fracCoord, cartCoord;

  for (unsigned int i = 0; i < cell->GetBasisCount(); ++i) {
    coord = cell->GetAtomicCoordinate(i);
    fracCoord.Set(
          coord.atomPosition.x, coord.atomPosition.y, coord.atomPosition.z);
    cartCoord = obcell->FractionalToCartesian(fracCoord);

    Avogadro::Atom *atom = mol->addAtom();
    atom->setAtomicNumber(coord.atomicNumber);
    atom->setPos(Eigen::Vector3d(cartCoord.AsArray()));
  }

  // Adjust length
  adjustLength(mol, length);

  // Remove the lattice (no longer needed for non-periodic tube)
  if (length != 1 || cap) {
    mol->setOBUnitCell(NULL);
  }

  // Cap if needed
  if (cap) {
    capTube(mol);
  }

  // Perceive bonds
  perceiveBonds(mol);

  return mol;
}

} // end namespace AvoTubeGen

} // end namespace SWCNTBuilder
