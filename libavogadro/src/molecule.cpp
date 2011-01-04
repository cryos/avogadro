/**********************************************************************
  Molecule - Molecule class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008-2009 Marcus D. Hanwell

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

#include "molecule.h"

#include "atom.h"
#include "bond.h"
#include "cube.h"
#include "mesh.h"
#include "fragment.h"
#include "residue.h"
#include "zmatrix.h"

#include <Eigen/Geometry>
#include <Eigen/LeastSquares>

#include <vector>

#include <openbabel/mol.h>
#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>
#include <openbabel/generic.h>
#include <openbabel/forcefield.h>
#include <openbabel/obiter.h>

#include <QDir>
#include <QDebug>
#include <QVariant>

namespace Avogadro{

  using std::vector;
  using Eigen::Vector3d;

  class MoleculePrivate {
    public:
      MoleculePrivate() : farthestAtom(0), invalidGeomInfo(true),
                          invalidRings(true), obmol(0), obunitcell(0),
                          obvibdata(0)
#if (OB_VERSION >= OB_VERSION_CHECK(2, 2, 99))
                        , obdosdata(0), obelectronictransitiondata(0)
#endif
    {}
    // These are logically cached variables and thus are marked as mutable.
    // Const objects should be logically constant (and not mutable)
    // http://www.highprogrammer.com/alan/rants/mutable.html
      mutable Eigen::Vector3d       center;
      mutable Eigen::Vector3d       normalVector;
      mutable double                radius;
      mutable Atom *                farthestAtom;
      mutable bool                  invalidGeomInfo;
      mutable bool                  invalidRings;
      mutable std::vector<double>   energies;

      // std::vector used over QVector due to index issues, QVector uses ints
      std::vector<Cube *>           cubes;
      std::vector<Mesh *>           meshes;
      std::vector<Residue *>        residues;
      std::vector<Fragment *>       rings;
      std::vector<ZMatrix *>        zMatrix;

      // Used to store the index based list (not unique ids)
      QList<Cube *>                 cubeList;
      QList<Mesh *>                 meshList;
      QList<Residue *>              residueList;
      QList<Fragment *>             ringList;
      QList<ZMatrix *>              zMatrixList;

      // Our OpenBabel OBMol object
      OpenBabel::OBMol *            obmol;
      // Our OpenBabel OBUnitCell object (if any)
      OpenBabel::OBUnitCell *       obunitcell;
      // Our OpenBabel OBVibrationData object (if any)
      // TODO: Cache an OBMol, in which case the vib. data (and others)
      //       won't be necessary
      OpenBabel::OBVibrationData *  obvibdata;
#if (OB_VERSION >= OB_VERSION_CHECK(2, 2, 99))
      OpenBabel::OBDOSData *        obdosdata;
      OpenBabel::OBElectronicTransitionData *
                                    obelectronictransitiondata;
#endif
  };

  Molecule::Molecule(QObject *parent) : Primitive(MoleculeType, parent),
                                        d_ptr(new MoleculePrivate),
                                        m_atomPos(0),
                                        m_currentConformer(0),
                                        m_estimatedDipoleMoment(true),
                                        m_dipoleMoment(0),
                                        m_invalidPartialCharges(true),
                                        m_invalidAromaticity(true),
                                        m_lock(new QReadWriteLock)
  {
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    // Assign a default path and file name to new molecules.
    m_fileName = QDir::homePath() + '/' +
                 tr("untitled", "Name of a new, untitled molecule file") +
                 ".cml";
  }

  Molecule::Molecule(const Molecule &other) :
    Primitive(MoleculeType, other.parent()), d_ptr(new MoleculePrivate),
    m_atomPos(0), m_dipoleMoment(0), m_invalidPartialCharges(true),
    m_invalidAromaticity(true), m_lock(new QReadWriteLock)
  {
    *this = other;
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::~Molecule()
  {
    // Need to iterate through all atoms/bonds and destroy them
    disconnect(this, 0);
    blockSignals(true);
    clear();
    delete m_lock;
    delete d_ptr;
  }

  void Molecule::setFileName(const QString& name)
  {
    m_fileName = name;
  }

  QString Molecule::fileName() const
  {
    return m_fileName;
  }

  Atom *Molecule::addAtom()
  {
    // Add an atom with the next unique id
    return addAtom(m_atoms.size());
  }

    // do some fancy footwork when we add an atom previously created
  Atom *Molecule::addAtom(unsigned long id)
  {
    Atom *atom = new Atom(this);

    if (!m_atomPos) {
      m_atomConformers.resize(1);
      m_atomConformers[0] = new vector<Vector3d>;
      m_atomPos = m_atomConformers[0];
      m_atomPos->reserve(100);
    }

    if(id >= m_atoms.size()) {
      m_atoms.resize(id+1,0);
      m_atomPos->resize(id+1, Vector3d::Zero());
    }
    m_atoms[id] = atom;
    // Does this still want to have the same index as before somehow?
    m_atomList.push_back(atom);

    atom->setId(id);
    atom->setIndex(m_atomList.size()-1);
    // now that the id is correct, emit the signal
    connect(atom, SIGNAL(updated()), this, SLOT(updateAtom()));
    emit atomAdded(atom);
    return atom;
  }
  void Molecule::setAtomPos(unsigned long id, const Eigen::Vector3d& vec)
  {
    if (id < m_atomPos->size())
      (*m_atomPos)[id] = vec;
  }

  void Molecule::setAtomPos(unsigned long id, const Eigen::Vector3d *vec)
  {
    if (vec)
      setAtomPos(id, *vec);
  }

void Molecule::removeAtom(Atom *atom)
  {
    if(atom) {
      // When deleting an atom this also implicitly deletes any bonds to the atom
      foreach (unsigned long bond, atom->bonds()) {
        removeBond(bond);
      }

      m_atoms[atom->id()] = 0;
      // 1 based arrays stored/shown to user
      int index = atom->index();
      m_atomList.removeAt(index);
      for (int i = index; i < m_atomList.size(); ++i)
        m_atomList[i]->setIndex(i);
      atom->deleteLater();

      disconnect(atom, SIGNAL(updated()), this, SLOT(updateAtom()));
      emit atomRemoved(atom);
    }
  }

  void Molecule::removeAtom(unsigned long id)
  {
    removeAtom(atomById(id));
  }

  Bond *Molecule::addBond()
  {
    return addBond(m_bonds.size());
  }

  Bond *Molecule::addBond(unsigned long id)
  {
    Q_D(Molecule);
    Bond *bond = new Bond(this);

    d->invalidRings = true;
    m_invalidPartialCharges = true;
    m_invalidAromaticity = true;
    if(id >= m_bonds.size())
      m_bonds.resize(id+1,0);
    m_bonds[id] = bond;
    m_bondList.push_back(bond);

    bond->setId(id);
    bond->setIndex(m_bondList.size()-1);
    // now that the id is correct, emit the signal
    connect(bond, SIGNAL(updated()), this, SLOT(updateBond()));
    emit bondAdded(bond);
    return(bond);
  }

  void Molecule::removeBond(Bond *bond)
  {
    if(bond) {
      removeBond(bond->id());
    }
  }

  void Molecule::removeBond(unsigned long id)
  {
    if (id < m_bonds.size()) {
      Q_D(Molecule);
      if (m_bonds[id] == 0)
        return;

      d->invalidRings = true;
      m_invalidPartialCharges = true;
      m_invalidAromaticity = true;
      Bond *bond = m_bonds[id];
      m_bonds[id] = 0;
      // Delete the bond from the list and reorder the remaining bonds
      int index = bond->index();
      m_bondList.removeAt(index);
      for (int i = index; i < m_bondList.size(); ++i) {
        m_bondList[i]->setIndex(i);
      }

      // Also delete the bond from the attached atoms
      if (m_atoms.size() > bond->beginAtomId()) {
        if (m_atoms[bond->beginAtomId()])
          m_atoms[bond->beginAtomId()]->removeBond(id);
      }
      if (m_atoms.size() > bond->endAtomId()) {
        if (m_atoms[bond->endAtomId()])
          m_atoms[bond->endAtomId()]->removeBond(id);
      }

      disconnect(bond, SIGNAL(updated()), this, SLOT(updateBond()));
      emit bondRemoved(bond);
      bond->deleteLater();
    }
  }

  Residue *Molecule::residue(int index)
  {
    Q_D(Molecule);

    if (index >= 0 && index < d->residueList.size())
      return d->residueList[index];
    else
      return 0;
  }

  const Residue *Molecule::residue(int index) const
  {
    Q_D(const Molecule);
    if (index >= 0 && index < d->residueList.size())
      return d->residueList[index];
    else
      return 0;
  }

  Residue *Molecule::residueById(unsigned long id) const
  {
    Q_D(const Molecule);
    if(id < d->residues.size())
      return d->residues[id];
    else
      return 0;
  }

  Cube *Molecule::cube(int index) const
  {
    Q_D(const Molecule);
    if (index >= 0 && index < d->cubeList.size())
      return d->cubeList[index];
    else
      return 0;
  }

  Cube *Molecule::cubeById(unsigned long id) const
  {
    Q_D(const Molecule);
    if(id < d->cubes.size())
      return d->cubes[id];
    else
      return 0;
  }

  Cube *Molecule::addCube()
  {
    Q_D(const Molecule);
    return addCube(d->cubes.size());
  }

  Cube *Molecule::addCube(unsigned long id)
  {
    Q_D(Molecule);

    Cube *cube = new Cube(this);

    if(id >= d->cubes.size())
      d->cubes.resize(id+1,0);
    d->cubes[id] = cube;
    // Does this still want to have the same index as before somehow?
    d->cubeList.push_back(cube);

    cube->setId(id);
    cube->setIndex(d->cubeList.size()-1);

    // now that the id is correct, emit the signal
    connect(cube, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(cube);
    return(cube);
  }




  void Molecule::removeCube(Cube *cube)
  {
    Q_D(Molecule);
    if(cube) {
      d->cubes[cube->id()] = 0;
      // 0 based arrays stored/shown to user
      int index = cube->index();
      d->cubeList.removeAt(index);
      for (int i = index; i < d->cubeList.size(); ++i)
        d->cubeList[i]->setIndex(i);

      cube->deleteLater();
      disconnect(cube, SIGNAL(updated()), this, SLOT(updatePrimitive()));
      emit primitiveRemoved(cube);
    }
  }

  void Molecule::removeCube(unsigned long id)
  {
    Q_D(Molecule);
    if (id < d->cubes.size())
      removeCube(d->cubes[id]);
  }

  Mesh * Molecule::addMesh()
  {
    Q_D(const Molecule);
    return addMesh(d->meshes.size());
  }

  Mesh * Molecule::addMesh(unsigned long id)
  {
    Q_D(Molecule);

    Mesh *mesh = new Mesh(this);

    if (id >= d->meshes.size())
      d->meshes.resize(id+1,0);
    d->meshes[id] = mesh;
    d->meshList.push_back(mesh);

    mesh->setId(id);
    mesh->setIndex(d->meshList.size()-1);

    // now that the id is correct, emit the signal
    connect(mesh, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(mesh);
    return(mesh);
  }

  void Molecule::removeMesh(Mesh *mesh)
  {
    Q_D(Molecule);
    if(mesh) {
      d->meshes[mesh->id()] = 0;
      // 0 based arrays stored/shown to user
      int index = mesh->index();
      d->meshList.removeAt(index);
      for (int i = index; i < d->meshList.size(); ++i)
        d->meshList[i]->setIndex(i);

      mesh->deleteLater();
      disconnect(mesh, SIGNAL(updated()), this, SLOT(updatePrimitive()));
      emit primitiveRemoved(mesh);
    }
  }

  void Molecule::removeMesh(unsigned long id)
  {
    Q_D(Molecule);
    if (id < d->meshes.size())
      removeMesh(d->meshes[id]);
  }

  Mesh * Molecule::mesh(int index) const
  {
    Q_D(const Molecule);
    if (index >= 0 && index < d->meshList.size())
      return d->meshList[index];
    else
      return 0;
  }

  Mesh * Molecule::meshById(unsigned long id) const
  {
    Q_D(const Molecule);
    if(id < d->meshes.size())
      return d->meshes[id];
    else
      return 0;
  }

  Residue * Molecule::addResidue()
  {
    Q_D(const Molecule);
    return addResidue(d->residues.size());
  }

  Residue * Molecule::addResidue(unsigned long id)
  {
    Q_D(Molecule);

    Residue *residue = new Residue(this);

    if (id >= d->residues.size())
      d->residues.resize(id+1,0);
    d->residues[id] = residue;
    d->residueList.push_back(residue);

    residue->setId(id);
    residue->setIndex(d->residueList.size()-1);

    // now that the id is correct, emit the signal
    connect(residue, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(residue);
    return(residue);
  }

  void Molecule::removeResidue(Residue *residue)
  {
    Q_D(Molecule);
    if(residue) {
      d->residues[residue->id()] = 0;
      // 0 based arrays stored/shown to user
      int index = residue->index();
      d->residueList.removeAt(index);
      for (int i = index; i < d->residueList.size(); ++i) {
        d->residueList[i]->setIndex(i);
      }

      residue->deleteLater();
      disconnect(residue, SIGNAL(updated()), this, SLOT(updatePrimitive()));
      emit primitiveRemoved(residue);
    }
  }

  void Molecule::removeResidue(unsigned long id)
  {
    Q_D(Molecule);
    if (id < d->residues.size())
      removeResidue(d->residues[id]);
  }

  Fragment * Molecule::addRing()
  {
    Q_D(const Molecule);
    return addRing(d->rings.size());
  }

  Fragment * Molecule::addRing(unsigned long id)
  {
    Q_D(Molecule);

    Fragment *ring = new Fragment(this);

    if (id >= d->rings.size())
      d->rings.resize(id+1,0);
    d->rings[id] = ring;
    d->ringList.push_back(ring);

    ring->setId(id);
    ring->setIndex(d->ringList.size()-1);

    // now that the id is correct, emit the signal
    connect(ring, SIGNAL(updated()), this, SLOT(updatePrimitive()));
//    emit primitiveAdded(ring);
    return(ring);
  }

  void Molecule::removeRing(Fragment *ring)
  {
    Q_D(Molecule);
    if(ring) {
      d->rings[ring->id()] = 0;
      // 0 based arrays stored/shown to user
      int index = ring->index();
      d->ringList.removeAt(index);
      for (int i = index; i < d->ringList.size(); ++i) {
        d->ringList[i]->setIndex(i);
      }

      ring->deleteLater();
      disconnect(ring, SIGNAL(updated()), this, SLOT(updatePrimitive()));
//      emit primitiveRemoved(ring);
    }
  }

  void Molecule::removeRing(unsigned long id)
  {
    Q_D(Molecule);
    if (id < d->rings.size())
      removeRing(d->rings[id]);
  }

  ZMatrix * Molecule::addZMatrix()
  {
    Q_D(Molecule);
    ZMatrix *zmatrix = new ZMatrix(this);
    d->zMatrixList.push_back(zmatrix);

    return zmatrix;
  }

  void Molecule::removeZMatrix(ZMatrix *zmatrix)
  {
    Q_D(Molecule);
    if (zmatrix) {
      d->zMatrixList.removeAll(zmatrix);
      delete zmatrix;
    }
  }

  ZMatrix * Molecule::zMatrix(int index) const
  {
    Q_D(const Molecule);
    if (index < d->zMatrixList.size())
      return d->zMatrixList.at(index);
    else
      return 0;
  }

  QList<ZMatrix *> Molecule::zMatrices() const
  {
    Q_D(const Molecule);
    return d->zMatrixList;
  }

  unsigned int Molecule::numZMatrices() const
  {
    Q_D(const Molecule);
    return d->zMatrixList.size();
  }

  void Molecule::addHydrogens(Atom *a,
                              const QList<unsigned long> &atomIds,
                              const QList<unsigned long> &bondIds)
  {
    if (atomIds.size() != bondIds.size()) {
      qDebug() << "Error, addHydrogens called with atom & bond id lists of different size!";
    }

    // Construct an OBMol, call AddHydrogens and translate the changes
    OpenBabel::OBMol obmol = OBMol();
    if (a) {
      OpenBabel::OBAtom *obatom = obmol.GetAtom(a->index()+1);
      // Set implicit valence for unusual elements not handled by OpenBabel
      // PR#2803076
      switch (obatom->GetAtomicNum()) {
      case 3:
      case 11:
      case 19:
      case 37:
      case 55:
      case 85:
      case 87:
        obatom->SetImplicitValence(1);
        obatom->SetHyb(1);
        obmol.SetImplicitValencePerceived();
        break;

      case 4:
      case 12:
      case 20:
      case 38:
      case 56:
      case 88:
        obatom->SetImplicitValence(2);
        obatom->SetHyb(2);
        obmol.SetImplicitValencePerceived();
        break;

      case 84: // Po
        obatom->SetImplicitValence(2);
        obatom->SetHyb(3);
        obmol.SetImplicitValencePerceived();
        break;

      default: // do nothing
        break;
      }
      obmol.AddHydrogens(obatom);
    }
    else
      obmol.AddHydrogens();
    // All new atoms in the OBMol must be the additional hydrogens
    unsigned int numberAtoms = numAtoms();
    int j = 0;
    for (unsigned int i = numberAtoms+1; i <= obmol.NumAtoms(); ++i, ++j) {
      if (obmol.GetAtom(i)->IsHydrogen()) {
        OpenBabel::OBAtom *obatom = obmol.GetAtom(i);
        Atom *atom;
        if (atomIds.isEmpty())
          atom = addAtom();
        else if (j < atomIds.size())
          atom = addAtom(atomIds.at(j));
        else {
          qDebug() << "Error - not enough unique ids in addHydrogens.";
          break;
        }
        atom->setOBAtom(obatom);
        // Get the neighbor atom
        OpenBabel::OBBondIterator iter;
        OpenBabel::OBAtom *next = obatom->BeginNbrAtom(iter);
        Bond *bond;
        if (bondIds.isEmpty())
          bond = addBond();
        else // Already confirmed by atom ids
          bond = addBond(bondIds.at(j));
        bond->setEnd(Molecule::atom(atom->index()));
        bond->setBegin(Molecule::atom(next->GetIdx()-1));
      }
    }
    for (unsigned int i = 1; i <= numberAtoms; ++i) {
      // Warning -- OB atom index off-by-one here
      atom(i-1)->setPartialCharge(obmol.GetAtom(i)->GetPartialCharge());
    }
  }

  void Molecule::removeHydrogens(Atom *atom)
  {
    if (atom) {
      // Delete any connected hydrogen atoms
      QList<unsigned long> neighbors = atom->neighbors();

      foreach (unsigned long a, neighbors) {
        Atom *nbrAtom = atomById(a);
        // we need to check if the atom still exists
        if (nbrAtom) {
          if (nbrAtom->isHydrogen()) {
            removeAtom(a);
          }
        }
        else {
          qDebug() << "Error, atom advertising deleted neighbor" << atom->id()
                   << a;
        }
      }
    }
    // Delete all of the hydrogens
    else {
      foreach (Atom *atom, m_atomList) {
        if (atom->isHydrogen()) {
          removeAtom(atom);
        }
      }
    }
  }

  void Molecule::setDipoleMoment(const Eigen::Vector3d &moment)
  {
    *m_dipoleMoment = moment;
    m_estimatedDipoleMoment = false;
  }

  Eigen::Vector3d Molecule::dipoleMoment(bool *estimate) const
  {
    if (m_dipoleMoment && !m_estimatedDipoleMoment) {
      if (estimate != NULL) // passed this as an argument
        *estimate = false; // genuine calculated dipole moment
      return *m_dipoleMoment;
    }
    else {
      // Calculate a new estimate (e.g., the geometry changed
      Vector3d dipoleMoment(0.0, 0.0, 0.0);
      // Use MMFF94 charges -- good estimate of dipole moment
      OpenBabel::OBForceField *ff = OpenBabel::OBForceField::FindForceField("MMFF94");
      OpenBabel::OBMol obmol = OBMol();
      if (ff->Setup(obmol)) {
        ff->GetPartialCharges(obmol);
        for( OpenBabel::OBMolAtomIter atom(obmol); atom; ++atom ) {
          OpenBabel::OBPairData *chg = (OpenBabel::OBPairData*) atom->GetData("FFPartialCharge");
          if (chg)
            dipoleMoment += Vector3d(atom->GetVector().AsArray()) * atof(chg->GetValue().c_str());
        }
        dipoleMoment *= 3.60; // fit from regression, R^2 = 0.769
      }
      else {
        foreach (Atom *a, atoms())
          dipoleMoment += *a->pos() * a->partialCharge();
      }

      if (estimate)
        *estimate = true;
      m_estimatedDipoleMoment = true;
      return dipoleMoment;
    }
  }

  void Molecule::calculatePartialCharges() const
  {
    if (numAtoms() < 1 || !m_invalidPartialCharges) {
      return;
    }
    OpenBabel::OBMol obmol = OBMol();
    for (unsigned int i = 0; i < numAtoms(); ++i) {
      // Warning: OB off-by-one index
      atom(i)->setPartialCharge(obmol.GetAtom(i+1)->GetPartialCharge());
    }
    m_invalidPartialCharges = false;
  }

  void Molecule::calculateAromaticity() const
  {
    if (numBonds() < 1 || !m_invalidAromaticity)
      return;

    OpenBabel::OBMol obmol = OBMol();
    for (unsigned int i = 0; i < obmol.NumBonds(); ++i) {
      bond(i)->setAromaticity(obmol.GetBond(i)->IsAromatic());
    }
    m_invalidAromaticity = false;
  }

  void Molecule::calculateGroupIndices() const
  {
/*    OpenBabel::OBMol obmol = OBMol();
#if (OB_VERSION >= OB_VERSION_CHECK(2, 2, 99))
    for (unsigned int i = 0; i < obmol.NumAtoms(); ++i) {
      atom(i)->setGroupIndex(obmol.GetAtomGroupNumbers().at(i));
    }
#endif*/
    QVector<unsigned int> group_number;   // numbers of atoms in each group
    QVector<int> group_ele;    // elements of each group
    QVector<unsigned int> atomGroupNumber;
    atomGroupNumber.resize(numAtoms());

    for (unsigned int i = 0;
         i < numAtoms() && static_cast<int>(i) < atomGroupNumber.size();
         ++i) {
      bool match = false;
      for (int j=0; j < group_number.size(); ++j) {
          if ((atom(i)->atomicNumber()) == group_ele.at(j)) {
            group_number[j] += 1;
            atomGroupNumber[i] = group_number[j];
            match = true;
          }
      }
      if (!match) {
        group_ele.push_back(atom(i)->atomicNumber());
        group_number.push_back(1);
        atomGroupNumber[i] = 1;
      }
    }

    for (unsigned int i = 0;
         i < numAtoms() && static_cast<int>(i) < atomGroupNumber.size();
         ++i) {
      bool match = false;
      for (int j=0; j<group_number.size(); ++j) {
        if ((atom(i)->atomicNumber()) == group_ele.at(j) &&
            (group_number.at(j) == 1)) {
          match = true;
        }
      }
      if (match) {
        atom(i)->setGroupIndex(0);
      } else {
        atom(i)->setGroupIndex(atomGroupNumber.at(i));
      }
    }
  }

  unsigned int Molecule::numAtoms() const
  {
    return m_atomList.size();
  }

  unsigned int Molecule::numBonds() const
  {
    return m_bondList.size();
  }

  unsigned int Molecule::numCubes() const
  {
    Q_D(const Molecule);
    return d->cubeList.size();
  }

  unsigned int Molecule::numMeshes() const
  {
    Q_D(const Molecule);
    return d->meshList.size();
  }

  unsigned int Molecule::numResidues() const
  {
    Q_D(const Molecule);
    return d->residueList.size();
  }

  unsigned int Molecule::numRings() const
  {
    Q_D(const Molecule);
    return d->ringList.size();
  }

  void Molecule::updateMolecule()
  {
    Q_D(Molecule);
    d->invalidGeomInfo = true;
    emit moleculeChanged();
    emit updated();
  }

  void Molecule::updatePrimitive()
  {
    Q_D(Molecule);
    Primitive *primitive = qobject_cast<Primitive *>(sender());
    d->invalidGeomInfo = true;
    emit primitiveUpdated(primitive);
  }

  void Molecule::updateAtom()
  {
    Q_D(Molecule);
    Atom *atom = qobject_cast<Atom *>(sender());
    d->invalidGeomInfo = true;
    calculateGroupIndices();
    emit atomUpdated(atom);
  }

  void Molecule::updateBond()
  {
    Bond *bond = qobject_cast<Bond *>(sender());
    emit bondUpdated(bond);
  }

  void Molecule::update()
  {
    emit updated();
  }

  Bond* Molecule::bond(unsigned long id1, unsigned long id2)
  {
    // Take two atom IDs and see if we have a bond between the two
    if (atomById(id1)) {
      QList<unsigned long> bonds = atomById(id1)->bonds();
      foreach (unsigned long id, bonds) {
        Bond *bond = bondById(id);
        if (bond) {
          if (bond->otherAtom(id1) == id2) {
            return bond;
          }
        }
      }
    }
    return 0;
  }

  Bond* Molecule::bond(const Atom *a1, const Atom *a2)
  {
    if (a1 && a2) {
      return bond(a1->id(), a2->id());
    }
    else {
      return 0;
    }
  }

  bool Molecule::addConformer(const std::vector<Eigen::Vector3d> &conformer,
                              unsigned int index)
  {
    if (conformer.size() != m_atomPos->size())
      return false;

    if (m_atomConformers.size() < index+1) {
      unsigned int size = m_atomConformers.size();
      // If there is a gap between the current last conformer and the new index, pad it
      for (unsigned int i = size; i <= index; ++i)
        m_atomConformers.push_back( new vector<Vector3d>(m_atomPos->size()) );
    }
    *m_atomConformers[index] = conformer;
    return true;
  }

  vector<Vector3d> * Molecule::addConformer(unsigned int index)
  {
    if (index < m_atomConformers.size())
      return m_atomConformers[index];
    else {
      unsigned int size = m_atomConformers.size();
      m_atomConformers.resize(index+1);
      for (unsigned int i = size; i <= index; ++i)
        m_atomConformers[i] = new vector<Vector3d>(m_atomPos->size());
      return m_atomConformers[index];
    }
  }

  vector<Vector3d> * Molecule::conformer(unsigned int index)
  {
    if (index && index < m_atomConformers.size())
      return m_atomConformers[index];
    else if (index == 0)
      return m_atomPos;
    else
      return NULL;
  }

  const std::vector<std::vector<Eigen::Vector3d> *>& Molecule::conformers() const
  {
    return m_atomConformers;
  }

  bool Molecule::setConformer(unsigned int index)
  {
    // If the index is higher than the size
    if (m_atomConformers.size() < index + 1)
      return false;
    else {
      // m_atomPos is resized if atoms are added/deleted, we store it's size here
      unsigned int size = m_atomPos->size();
      // Set the current m_atomPos
      m_atomPos = m_atomConformers[index];
      // It is possible new atoms got added since the conformers were set.
      // Here, we pad m_atomPos to match the current number of atoms.
      while (m_atomPos->size() < size)
        m_atomPos->push_back(Eigen::Vector3d::Zero());
      // set the current conformer index
      m_currentConformer = index;
      return true;
    }
  }

  bool Molecule::setAllConformers(const std::vector< std::vector<Eigen::Vector3d>* > conformers, bool deleteExisting)
  {
    if (!conformers.size()) {
      clearConformers();
      return true;
    }
    unsigned long size = m_atomPos->size();

    // delete any previous conformers
    // TODO: Combine this code with clearConformers()
    if (deleteExisting) {
      for (unsigned int i = 0; i < m_atomConformers.size(); ++i)
        delete m_atomConformers[i];
    }
    m_atomConformers.clear();

    // add the new conformers
    for (unsigned int i = 0; i < conformers.size(); ++i) {
      if (conformers[i]->size() != size)
        return false;
      m_atomConformers.push_back(conformers[i]);
    }

    m_atomPos = m_atomConformers[0];
    m_currentConformer = 0;
    return true;
  }

  void Molecule::clearConformers()
  {
    if (m_atomConformers.size() > 1) {
      for (unsigned int i = 1; i < m_atomConformers.size(); ++i)
        delete m_atomConformers[i];
      m_atomConformers.resize(1);
      m_atomPos = m_atomConformers[0];
    }
    m_currentConformer = 0;
  }

  unsigned int Molecule::numConformers() const
  {
    return m_atomConformers.size();
  }

  unsigned int Molecule::currentConformer() const
  {
    return m_currentConformer;
  }

  const std::vector<double>& Molecule::energies() const
  {
    Q_D(const Molecule);
    while (d->energies.size() < numConformers())
      d->energies.push_back(0.0);
    if (d->energies.size() > numConformers())
      d->energies.resize(numConformers());
    return d->energies;
  }

  double Molecule::energy(int index) const
  {
    Q_D(const Molecule);
    if (index == -1 && d->energies.size()) // if there are any...
      return d->energies[m_currentConformer];
    else if (index < static_cast<int>(d->energies.size()))
      return d->energies[index];
    else
      return 0.0;
  }

  void Molecule::setEnergy(double energy)
  {
    Q_D(const Molecule);
    while (d->energies.size() < numConformers())
      d->energies.push_back(0.0);
    if (m_currentConformer < d->energies.size())
      d->energies[m_currentConformer] = energy;
  }

  void Molecule::setEnergy(int index, double energy)
  {
    Q_D(const Molecule);
    if (index > static_cast<int>(numConformers() - 1) || index < 0)
      return;
    while (d->energies.size() != numConformers())
      d->energies.push_back(0.0);
    d->energies[index] = energy;
  }

  void Molecule::setEnergies(const std::vector<double>& energies)
  {
    Q_D(const Molecule);
    d->energies = energies;
  }


  QList<Atom *> Molecule::atoms() const
  {
    return m_atomList;
  }

  QList<Bond *> Molecule::bonds() const
  {
    return m_bondList;
  }

  QList<Cube *> Molecule::cubes() const
  {
    Q_D(const Molecule);
    return d->cubeList;
  }

  QList<Mesh *> Molecule::meshes() const
  {
    Q_D(const Molecule);
    return d->meshList;
  }

  QList<Residue *> Molecule::residues() const
  {
    Q_D(const Molecule);
    return d->residueList;
  }

  QList<Fragment *> Molecule::rings()
  {
    Q_D(Molecule);
    // Check is the rings need updating before returning the list
    if(d->invalidRings) {
      // Now update the rings
      foreach(Fragment *ring, d->ringList) {
        removeRing(ring);
      }
      OpenBabel::OBMol obmol = OBMol();
      std::vector<OpenBabel::OBRing *> rings;
      rings = obmol.GetSSSR();
      foreach(OpenBabel::OBRing *r, rings) {
        Fragment *ring = addRing();
        foreach(int index, r->_path) {
          ring->addAtom(atom(index-1)->id());
        }
      }
      d->invalidRings = false;
    }
    return d->ringList;
  }

  OpenBabel::OBMol Molecule::OBMol() const
  {
    Q_D(const Molecule);
    // Right now we make an OBMol each time
    OpenBabel::OBMol obmol;
    obmol.BeginModify();

    foreach(Atom *atom, m_atomList) {
      OpenBabel::OBAtom *a = obmol.NewAtom();
      OpenBabel::OBAtom obatom = atom->OBAtom();
      *a = obatom;
    }
    // we are copying partial charges above
    obmol.SetPartialChargesPerceived();
    foreach(Bond *bond, m_bondList) {
      Atom *beginAtom = atomById(bond->beginAtomId());
      if (!beginAtom)
        continue;

      Atom *endAtom = atomById(bond->endAtomId());
      if (!endAtom)
        continue;

      obmol.AddBond(beginAtom->index() + 1,
                    endAtom->index() + 1, bond->order());

      QString label = bond->customLabel();
      if(!label.isEmpty()) {
        OpenBabel::OBPairData *dp = new OpenBabel::OBPairData();
        dp->SetAttribute("label");
        dp->SetValue(label.toLatin1());
        obmol.GetBond(obmol.NumBonds()-1)->SetData(dp);
      }
    }
    // We're doing this after copying all atoms, so we can grab them ourselves
    foreach(Residue *residue, d->residueList) {
      OpenBabel::OBResidue *r = obmol.NewResidue();
      // Copy per-residue information
      r->SetNum(residue->number().toStdString());
      r->SetChain(residue->chainID());
      r->SetName(residue->name().toUpper().toStdString());

      OpenBabel::OBAtom *a;
      foreach(unsigned long atomId, residue->atoms()){
        // Avogadro indexes from 0, but OB from 1. Watch out!
        Atom *avoAtom = this->atomById(atomId);
        if (!avoAtom)
          continue;
        a = obmol.GetAtom(avoAtom->index() + 1);
        r->AddAtom(a);
        r->SetSerialNum(a, a->GetIdx());
        QString atomLabel = residue->atomId(atomId);
        if (!atomLabel.isEmpty())
          r->SetAtomID(a, atomLabel.toStdString());
        else {
          r->SetAtomID(a, OpenBabel::etab.GetSymbol(avoAtom->atomicNumber()));
          r->SetHetAtom(a, true);
        }
      }
    }
    foreach(Cube *cube, d->cubeList) {
      OpenBabel::OBGridData *obgrid = new OpenBabel::OBGridData;
      obgrid->SetOrigin(OpenBabel::fileformatInput);
      obgrid->SetAttribute(cube->name().toLatin1().data());
      obgrid->SetUnit(OpenBabel::OBGridData::ANGSTROM);
      obgrid->SetNumberOfPoints(cube->dimensions().x(),
                                cube->dimensions().y(),
                                cube->dimensions().z());
      OpenBabel::vector3 origin(cube->min().x(), cube->min().y(), cube->min().z());
      OpenBabel::vector3 x(cube->spacing().x(), 0.0, 0.0);
      OpenBabel::vector3 y(0.0, cube->spacing().y(), 0.0);
      OpenBabel::vector3 z(0.0, 0.0, cube->spacing().z());
      obgrid->SetLimits(origin, x, y, z);
      obgrid->SetValues(cube->m_data);
      obmol.SetData(obgrid);
    }

    obmol.EndModify();

    // Copy unit cells
    if (d->obunitcell != NULL) {
      OpenBabel::OBUnitCell *obunitcell = new OpenBabel::OBUnitCell;
      *obunitcell = *d->obunitcell;
      obmol.SetData(obunitcell);
    }

    // Copy OBPairData, if needed
    OpenBabel::OBPairData *obproperty;
    foreach(const QByteArray &propertyName, dynamicPropertyNames()) {
      obproperty = new OpenBabel::OBPairData;
      obproperty->SetAttribute(propertyName.data());
      obproperty->SetValue(property(propertyName).toByteArray().data());
      obmol.SetData(obproperty);
    }

    // Copy vibrations, if needed
    if (d->obvibdata != NULL) {
      obmol.SetData(d->obvibdata->Clone(&obmol));
    }

#if (OB_VERSION >= OB_VERSION_CHECK(2, 2, 99))
    // Copy dos, if needed
    if (d->obdosdata != NULL) {
      obmol.SetData(d->obdosdata->Clone(&obmol));
    }

    // Copy excited states data, if needed
    if (d->obelectronictransitiondata != NULL) {
      obmol.SetData(d->obelectronictransitiondata->Clone(&obmol));
    }
#endif
    return obmol;
  }

  bool Molecule::setOBMol(OpenBabel::OBMol *obmol)
  {
    // Take an OBMol, copy everything we need and store this object
    Q_D(Molecule);
    qDebug() << "setOBMol called.";
    clear();
    // Copy all the parts of the OBMol to our Molecule
    blockSignals(true);

    // Begin by copying all of the atoms
    std::vector<OpenBabel::OBAtom*>::iterator i;

    for (OpenBabel::OBAtom *obatom = obmol->BeginAtom(i); obatom; obatom = obmol->NextAtom(i)) {
      Atom *atom = addAtom();
      atom->setOBAtom(obatom);
    }

    // Now bonds, we use the indices of the atoms to get the bonding right
    std::vector<OpenBabel::OBBond*>::iterator j;
    for (OpenBabel::OBBond *obbond = obmol->BeginBond(j); obbond; obbond = obmol->NextBond(j)) {
      Bond *bond = addBond();
      // Get the begin and end atoms - we use a 0 based index, OB uses 1 based
      bond->setAtoms(obbond->GetBeginAtom()->GetIdx()-1,
                     obbond->GetEndAtom()->GetIdx()-1,
                     obbond->GetBondOrder());
      if (obbond->HasData("label"))
        bond->setCustomLabel(obbond->GetData("label")->GetValue().c_str());
    }

    // Now for the volumetric data
    std::vector<OpenBabel::OBGenericData*> data = obmol->GetAllData(OpenBabel::OBGenericDataType::GridData);
    for (unsigned int i = 0; i < data.size(); ++i) {
      QString name = QString(data[i]->GetAttribute().c_str());
      OpenBabel::OBGridData *grid = static_cast<OpenBabel::OBGridData *>(data[i]);
      OpenBabel::vector3 obmin = grid->GetOriginVector();
      Eigen::Vector3d min(obmin.x(), obmin.y(), obmin.z());
      OpenBabel::vector3 obmax = grid->GetMaxVector();
      Eigen::Vector3d max(obmax.x(), obmax.y(), obmax.z());
      int x, y, z;
      grid->GetNumberOfPoints(x, y, z);
      Eigen::Vector3i points(x, y, z);
      Cube *cube = addCube();
      cube->setLimits(min, max, points);
      cube->setData(grid->GetValues());
      cube->setName(name);
      cube->setCubeType(Cube::FromFile);
    }

    // Copy the residues across...
    std::vector<OpenBabel::OBResidue *> residues;
    OpenBabel::OBResidueIterator iResidue;
    short chainNumber = 0;
    QHash<char, short> chains;
    for (OpenBabel::OBResidue *obres = static_cast<OpenBabel::OBResidue *>(obmol->BeginResidue(iResidue));
          obres; obres = static_cast<OpenBabel::OBResidue *>(obmol->NextResidue(iResidue))) {
      /// Copy these residues!
      Residue *residue = addResidue();
      residue->setName(obres->GetName().c_str());
      residue->setNumber(obres->GetNumString().c_str());
      if (!chains.contains(obres->GetChain())) {
        chains[obres->GetChain()] = chainNumber;
        chainNumber++;
      }
      residue->setChainID(obres->GetChain());
      residue->setChainNumber(chains.value(obres->GetChain()));
      std::vector<OpenBabel::OBAtom*> obatoms = obres->GetAtoms();
      foreach (OpenBabel::OBAtom *obatom, obatoms) {
        unsigned long atomId = atom(obatom->GetIdx()-1)->id();
        residue->addAtom(atomId);
        residue->setAtomId(atomId, obres->GetAtomID(obatom).c_str());
      }
      std::vector<OpenBabel::OBBond*> obbonds = obres->GetBonds();
      foreach (OpenBabel::OBBond *obbond, obbonds) {
        residue->addBond(bond(obbond->GetIdx())->id());
      }
    }

    // If available, copy the unit cell
    OpenBabel::OBUnitCell *obunitcell = static_cast<OpenBabel::OBUnitCell *>(obmol->GetData(OpenBabel::OBGenericDataType::UnitCell));
    if (obunitcell) {
      d->obunitcell = new OpenBabel::OBUnitCell;
      *d->obunitcell = *obunitcell;
    }
    // (that could return NULL, but other methods know they could get NULL)

    // Copy conformers, if present
    if (obmol->NumConformers() > 1) {
      for (int i = 0; i < obmol->NumConformers(); ++i) {
        obmol->SetConformer(i);
        // copy the coordinates
        double *coordPtr = obmol->GetCoordinates();
        std::vector<Eigen::Vector3d> conformer;
        foreach (Atom *atom, atoms()) {
          while (conformer.size() < atom->id())
            conformer.push_back(Eigen::Vector3d(0.0, 0.0, 0.0));
          conformer.push_back(Eigen::Vector3d(coordPtr));
          coordPtr += 3;
        } // end foreach atom
        addConformer(conformer, i);
      } // end for(conformers)
      setConformer(obmol->NumConformers() - 1);
      // energies and forces will be set below
    } else {
      // one conformer, so use setEnergy
      setEnergy(obmol->GetEnergy() * KCAL_TO_KJ);
    }

    // Copy conformer data (e.g., energies, forces) if present and valid
    if (obmol->HasData(OpenBabel::OBGenericDataType::ConformerData)) {
      OpenBabel::OBConformerData *cd = static_cast<OpenBabel::OBConformerData*>(obmol->GetData(OpenBabel::OBGenericDataType::ConformerData));
      if (cd) {
        // copy energies -- should be one for each conformer
        std::vector<double> energies = cd->GetEnergies();
        for (unsigned int i = 0; i < energies.size(); ++i)
          energies[i] *= KCAL_TO_KJ;
        setEnergies(energies);

        // check for validity (i.e., we have some forces, one for each atom
        std::vector< std::vector<OpenBabel::vector3> > allForces = cd->GetForces();
        if (allForces.size() && allForces[0].size() == numAtoms()) {
          OpenBabel::vector3 force;
          foreach (Atom *atom, m_atomList) { // loop through each atom
            force = allForces[0][atom->index()];
            atom->setForceVector(Eigen::Vector3d(force.x(), force.y(), force.z()));
          } // end setting forces on each atom
        } // forces
      } // end if (cd)
    }  // end HasData(ConformerData)

    // Copy any vibration data if possible
    if (obmol->HasData(OpenBabel::OBGenericDataType::VibrationData)) {
      OpenBabel::OBVibrationData *vibData = static_cast<OpenBabel::OBVibrationData*>(obmol->GetData(OpenBabel::OBGenericDataType::VibrationData));
      d->obvibdata = vibData;
    }

#if (OB_VERSION >= OB_VERSION_CHECK(2, 2, 99))
    // Copy DOS data
    if (obmol->HasData(OpenBabel::OBGenericDataType::DOSData)) {
      OpenBabel::OBDOSData *dosData = static_cast<OpenBabel::OBDOSData*>(obmol->GetData(OpenBabel::OBGenericDataType::DOSData));
      d->obdosdata = dosData;
    }

    // Copy electronic transition data
    if (obmol->HasData(OpenBabel::OBGenericDataType::ElectronicTransitionData)) {
      OpenBabel::OBElectronicTransitionData *etd =
        static_cast<OpenBabel::OBElectronicTransitionData*>
        (obmol->GetData(OpenBabel::OBGenericDataType::ElectronicTransitionData));
      d->obelectronictransitiondata = etd;
    }
#endif

    // Finally, sync OBPairData to dynamic properties
    OpenBabel::OBDataIterator dIter;
    OpenBabel::OBPairData *property;
    data = obmol->GetAllData(OpenBabel::OBGenericDataType::PairData);
    for (dIter = data.begin(); dIter != data.end(); ++dIter) {
      property = static_cast<OpenBabel::OBPairData *>(*dIter);
      setProperty(property->GetAttribute().c_str(), property->GetValue().c_str());
    }

    computeGeomInfo();

    // Copy the dipole moment of the molecule - do this after calling the
    // initial computeGeomInfo call
    OpenBabel::OBVectorData *vd = (OpenBabel::OBVectorData*)obmol->GetData("Dipole Moment");
    if (vd) {
      OpenBabel::vector3 moment = vd->GetData();
      m_dipoleMoment = new Vector3d(moment.x(), moment.y(), moment.z());
      m_estimatedDipoleMoment = false;
    }

    // we set the partial charges above
    m_invalidPartialCharges = false;

    blockSignals(false);
    emit update();
    return true;
  }

  OpenBabel::OBUnitCell *Molecule::OBUnitCell() const
  {
    Q_D(const Molecule);
    return d->obunitcell;
  }

  bool Molecule::setOBUnitCell(OpenBabel::OBUnitCell *obunitcell)
  {
    Q_D(Molecule);
    d->obunitcell = obunitcell;
    if (obunitcell == NULL) {
      // delete it from our private obmol
      if (d->obmol)
        d->obmol->DeleteData(OpenBabel::OBGenericDataType::UnitCell);
    }
    return true;
  }

  const Eigen::Vector3d Molecule::center() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->center;
  }

  const Eigen::Vector3d Molecule::normalVector() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->normalVector;
  }

  double Molecule::radius() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->radius;
  }

  const Atom * Molecule::farthestAtom() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->farthestAtom;
  }

  void Molecule::translate(const Eigen::Vector3d& offset)
  {
    if (!m_atomPos)
      return; // nothing to do

    Q_D(const Molecule);
    d->invalidGeomInfo = true;
    foreach (Atom *atom, m_atomList) {
      (*m_atomPos)[atom->id()] += offset;
      emit atomUpdated(atom);
    }
  }

  void Molecule::clear()
  {
    Q_D(Molecule);
    m_atoms.clear();
    foreach (Atom *atom, m_atomList) {
      atom->deleteLater();
      emit primitiveRemoved(atom);
    }
    m_atomList.clear();
    clearConformers();
    delete m_atomPos;
    m_atomPos = 0;
    delete m_dipoleMoment;
    m_dipoleMoment = 0;
    delete d->obunitcell;
    d->obunitcell = 0;

    m_bonds.clear();
    foreach (Bond *bond, m_bondList) {
      bond->deleteLater();
      emit primitiveRemoved(bond);
    }
    m_bondList.clear();

    d->cubes.clear();
    foreach (Cube *cube, d->cubeList) {
      cube->deleteLater();
      emit primitiveRemoved(cube);
    }
    d->cubeList.clear();

    d->meshes.clear();
    foreach (Mesh *mesh, d->meshList) {
      mesh->deleteLater();
      emit primitiveRemoved(mesh);
    }
    d->meshList.clear();

    d->residues.clear();
    foreach (Residue *residue, d->residueList) {
      residue->deleteLater();
      emit primitiveRemoved(residue);
    }
    d->residueList.clear();

    d->rings.clear();
    foreach (Fragment *ring, d->ringList) {
      ring->deleteLater();
      emit primitiveRemoved(ring);
    }
    d->ringList.clear();
  }

  QReadWriteLock * Molecule::lock() const
  {
    return m_lock;
  }

  Molecule &Molecule::operator=(const Molecule& other)
  {
    // FIXME: Copy all the other stuff in the molecule!
    //Q_D(Molecule);
    clear();
    //const MoleculePrivate *e = other.d_func();
    m_atoms.resize(other.m_atoms.size(), 0);
    if (other.m_atomPos) {
      m_atomConformers.resize(1);
      m_atomConformers[0] = new vector<Vector3d>;
      m_atomPos = m_atomConformers[0];
      m_atomPos->reserve(100);

      m_atomPos->clear();
      m_atomPos->resize(other.m_atomPos->size());
    }

    m_bonds.resize(other.m_bonds.size(), 0);

    // Copy the atoms and bonds over
    unsigned int size = other.m_atoms.size();
    for (unsigned int i = 0; i < size; ++i) {
      if (other.m_atoms.at(i) > 0) {
        Atom *atom = new Atom(this);
        atom->setId(other.m_atoms[i]->id());
        atom->setIndex(other.m_atoms[i]->index());
        m_atoms[i] = atom;
        m_atomList.push_back(atom);
        *atom = *(other.m_atoms[i]);
        emit primitiveAdded(atom);
      }
    }

    size = other.m_bonds.size();
    for (unsigned int i = 0; i < size; ++i) {
      if (other.m_bonds.at(i)) {
        Bond *bond = new Bond(this);
        *bond = *(other.m_bonds[i]);
        bond->setId(other.m_bonds[i]->id());
        bond->setIndex(other.m_bonds[i]->index());
        m_bonds[i] = bond;
        m_bondList.push_back(bond);
        // Add the bond to it's atoms
        bond->beginAtom()->addBond(bond);
        bond->endAtom()->addBond(bond);
        emit primitiveAdded(bond);
      }
    }

    foreach (Residue *r, other.residues()) {
      Residue *residue = addResidue();
      residue->setChainNumber(r->chainNumber());
      residue->setChainID(r->chainID());
      residue->setNumber(r->number());
      residue->setName(r->name());

      foreach(unsigned int atomId, r->atoms()) {
        residue->addAtom(atomId);
      }
      residue->setAtomIds(r->atomIds());
    }

    return *this;
  }

  Molecule &Molecule::operator+=(const Molecule& other)
  {
    // FIXME: Copy all the other stuff in the molecule!
    //const MoleculePrivate *e = other.d_func();
    // Create a temporary map from the old indices to the new for bonding
    QList<int> map;
    foreach (Atom *a, other.m_atomList) {
      Atom *atom = addAtom();
      *atom = *a;
      map.push_back(atom->id());
      emit primitiveAdded(atom);
    }
    foreach (Bond *b, other.m_bondList) {
      Bond *bond = addBond();
      *bond = *b;
      bond->setBegin(atomById(map.at(other.atomById(b->beginAtomId())->index())));
      bond->setEnd(atomById(map.at(other.atomById(b->endAtomId())->index())));
      emit primitiveAdded(bond);
    }
    foreach (Residue *r, other.residues()) {
      Residue *residue = addResidue();
      residue->setChainNumber(r->chainNumber());
      residue->setChainID(r->chainID());
      residue->setNumber(r->number());
      residue->setName(r->name());

      foreach(unsigned int atomId, r->atoms()) {
        residue->addAtom(map.at(atomId));
      }
      residue->setAtomIds(r->atomIds());
    }

    return *this;
  }

  void Molecule::computeGeomInfo() const
  {
    Q_D(const Molecule);
    d->invalidGeomInfo = true;
    d->farthestAtom = 0;
    d->center.setZero();
    d->normalVector.setZero();
    d->radius = 1.0;

    /// FIXME This leads to the dipole moment always getting invalidated
    /// as the geometry information must be computed on load
    // invalidate the previous dipole moment
    if (m_dipoleMoment) {
      delete m_dipoleMoment; // don't leak -- this is the previous estimate
      m_dipoleMoment = 0;
      m_estimatedDipoleMoment = true;
    }

    unsigned int nAtoms = numAtoms();
    // In order to calculate many parameters we need at least two atoms
    if(nAtoms > 1) {
      // Compute the normal vector to the molecule's best-fitting plane
      int i = 0;
      Vector3d ** atomPositions = new Vector3d*[nAtoms];
      // Calculate the center of the molecule too
      foreach (Atom *atom, m_atomList) {
        Vector3d *pos = &(*m_atomPos)[atom->id()];
        d->center += *pos;
        atomPositions[i++] = pos;
      }
      d->center /= static_cast<double>(nAtoms);
      Eigen::Hyperplane<double, 3> planeCoeffs;
      Eigen::fitHyperplane(numAtoms(), atomPositions, &planeCoeffs);
      delete[] atomPositions;
      d->normalVector = planeCoeffs.normal();

      // compute radius and the farthest atom
      d->radius = -1.0; // so that ( squaredDistanceToCenter > d->radius ) is true for at least one atom.
      foreach (Atom *atom, m_atomList) {
        double distanceToCenter = (*atom->pos() - d->center).norm();
        if(distanceToCenter > d->radius) {
          d->radius = distanceToCenter;
          d->farthestAtom = atom;
        }
      }
    }
    d->invalidGeomInfo = false;
  }

} // End namespace
