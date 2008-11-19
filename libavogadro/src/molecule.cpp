/**********************************************************************
  Molecule - Molecule class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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
#include "fragment.h"
#include "residue.h"

#include <Eigen/Regression>
#include <Eigen/Geometry>

#include <vector>

#include <openbabel/mol.h>
#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>

#include <QReadWriteLock>
#include <QDebug>

 namespace Avogadro{

  class MoleculePrivate {
    public:
      MoleculePrivate() : farthestAtom(0), invalidGeomInfo(true),
        invalidRings(true), obmol(0) {}
      mutable Eigen::Vector3d       center;
      mutable Eigen::Vector3d       normalVector;
      mutable double                radius;
      mutable Atom *                farthestAtom;
      mutable bool                  invalidGeomInfo;
      mutable bool                  invalidRings;

      QString                       fileName;

      // std::vector used over QVector due to index issues, QVector uses ints
      std::vector<Atom *>           atoms;
      std::vector<Bond *>           bonds;
      std::vector<Cube *>           cubes;
      std::vector<Residue *>        residues;
      std::vector<Fragment *>       rings;

      // Used to store the index based list (not unique ids)
      QList<Atom *>                 atomList;
      QList<Bond *>                 bondList;
      QList<Cube *>                 cubeList;
      QList<Residue *>              residueList;
      QList<Fragment *>             ringList;

      // Our OpenBabel OBMol object
      OpenBabel::OBMol *            obmol;
  };

  Molecule::Molecule(QObject *parent) : Primitive(MoleculeType, parent),
    d_ptr(new MoleculePrivate)
  {
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::Molecule(const Molecule &other) :
    Primitive(MoleculeType, other.parent()), d_ptr(new MoleculePrivate)
  {
    *this = other;
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::~Molecule()
  {
    // Need to iterate through all atoms/bonds and destroy them
    Q_D(Molecule);
    foreach (Atom *atom, d->atomList) {
      atom->deleteLater();
    }
    foreach (Bond *bond, d->bondList) {
      bond->deleteLater();
    }
    foreach (Cube *cube, d->cubeList) {
      cube->deleteLater();
    }
    foreach (Residue *residue, d->residueList) {
      residue->deleteLater();
    }
    foreach (Fragment *ring, d->ringList) {
      ring->deleteLater();
    }
    if (d->obmol) {
      delete d->obmol;
      d->obmol = 0;
    }
    delete d_ptr;
  }

  void Molecule::setFileName(const QString& name)
  {
    Q_D(Molecule);
    d->fileName = name;
  }

  QString Molecule::fileName() const
  {
    Q_D(const Molecule);
    return d->fileName;
  }
/*
  Atom * Molecule::CreateAtom()
  {
    Q_D(Molecule);

    d->lock.lockForWrite();
    Atom *atom = new Atom(this);
    connect(atom, SIGNAL(updated()), this, SLOT(updatePrimitive()));

    if(!d->autoId) {
      d->lock.unlock();
      return(atom);
    }

    atom->setId(d->atoms.size());
    d->atoms.push_back(atom);
    d->lock.unlock();

    emit primitiveAdded(atom);
    return(atom);
  }

  Bond * Molecule::CreateBond()
  {
    Q_D(Molecule);

    d->lock.lockForWrite();
    Bond *bond = new Bond(this);
    connect(bond, SIGNAL(updated()), this, SLOT(updatePrimitive()));

    if(!d->autoId) {
      d->lock.unlock();
      return(bond);
    }

    bond->setId(d->bonds.size());
    d->bonds.push_back(bond);
    d->lock.unlock();

    emit primitiveAdded(bond);
    return(bond);
  }

  Residue * Molecule::CreateResidue()
  {
    Residue *residue = new Residue(this);
    connect(residue, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(residue);
    return(residue);
  }
*/
  Atom *Molecule::newAtom()
  {
    Q_D(Molecule);
    m_lock->lockForWrite();
    d->invalidRings = true;
    Atom *atom = new Atom(this);
    d->atoms.push_back(atom);
    d->atomList.push_back(atom);
    atom->setId(d->atoms.size()-1);
    atom->setIndex(d->atomList.size()-1);
    connect(atom, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(atom);
    m_lock->unlock();
    return atom;
  }

  // do some fancy footwork when we add an atom previously created
  Atom *Molecule::newAtom(unsigned long id)
  {
    Q_D(Molecule);
    m_lock->lockForWrite();
    d->invalidRings = true;
    Atom *atom = new Atom(this);

    if(id >= d->atoms.size())
      d->atoms.resize(id+1,0);
    atom->setId(id);
    d->atoms[id] = atom;

    // Does this still want to have the same index as before somehow?
    d->atomList.push_back(atom);
    atom->setIndex(d->atomList.size()-1);

    // now that the id is correct, emit the signal
    connect(atom, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(atom);
    m_lock->unlock();
    return(atom);
  }

  void Molecule::deleteAtom(Atom *atom)
  {
    Q_D(Molecule);
    if(atom) {
      m_lock->lockForWrite();
      d->invalidRings = true;
      // When deleting an atom this also implicitly deletes any bonds to the atom
      QList<unsigned long int> bonds = atom->bonds();
      foreach (unsigned long int bond, bonds)
        deleteBond(bondById(bond));

      d->atoms[atom->id()] = 0;
      // 1 based arrays stored/shown to user
      int index = atom->index();
      d->atomList.removeAt(index);
      for (int i = index; i < d->atomList.size(); ++i)
        d->atomList[i]->setIndex(i);
      atom->deleteLater();
      disconnect(atom, SIGNAL(updated()), this, SLOT(updatePrimitive()));
      emit primitiveRemoved(atom);
      qDebug() << "Atom" << atom->id() << atom->index() << "deleted";
      m_lock->unlock();
    }
  }

  void Molecule::deleteAtom(unsigned long int id)
  {
    Q_D(Molecule);
    if (id < d->atoms.size())
      deleteAtom(d->atoms[id]);
  }

  Atom *Molecule::atom(int index)
  {
    Q_D(Molecule);
    if (index >= 0 && index < d->atomList.size())
      return d->atomList[index];
    else
      return 0;
  }

  Atom *Molecule::atomById(unsigned long id) const
  {
    Q_D(const Molecule);
    if(id < d->atoms.size())
      return d->atoms[id];
    else
      return 0;
  }

  Bond *Molecule::newBond()
  {
    Q_D(Molecule);
    d->invalidRings = true;
    Bond *bond = new Bond(this);
    d->bonds.push_back(bond);
    d->bondList.push_back(bond);
    bond->setId(d->bonds.size()-1);
    bond->setIndex(d->bondList.size()-1);
    connect(bond, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(bond);
    return bond;
  }

  Bond *Molecule::newBond(unsigned long id)
  {
    Q_D(Molecule);
    d->invalidRings = true;
    Bond *bond = new Bond(this);

    if(id >= d->bonds.size())
      d->bonds.resize(id+1,0);
    bond->setId(id);
    d->bonds[id] = bond;

    d->bondList.push_back(bond);
    bond->setIndex(d->bondList.size()-1);

    // now that the id is correct, emit the signal
    connect(bond, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(bond);
    return(bond);
  }

  void Molecule::deleteBond(Bond *bond)
  {
    Q_D(Molecule);
    if(bond) {
      d->invalidRings = true;
      d->bonds[bond->id()] = 0;
      // 1 based arrays stored/shown to user
      int index = bond->index();
      d->bondList.removeAt(index);
      for (int i = index; i < d->bondList.size(); ++i)
        d->bondList[i]->setIndex(i);
      // Also delete the bond from the attached atoms
      (atomById(bond->beginAtomId()))->deleteBond(bond);
      (atomById(bond->endAtomId()))->deleteBond(bond);

      bond->deleteLater();
      disconnect(bond, SIGNAL(updated()), this, SLOT(updatePrimitive()));
      emit primitiveRemoved(bond);
      qDebug() << "Bond" << bond->id() << bond->index() << "deleted";
    }
  }

  void Molecule::deleteBond(unsigned long int id)
  {
    Q_D(Molecule);
    if (id < d->bonds.size())
      deleteBond(d->bonds[id]);
  }

  Bond *Molecule::bond(int index)
  {
    Q_D(Molecule);
    if (index >= 0 && index < d->bondList.size())
      return d->bondList[index];
    else
      return 0;
  }

  Bond *Molecule::bondById(unsigned long id) const
  {
    Q_D(const Molecule);
    if(id < d->bonds.size())
    {
      return d->bonds[id];
    }
    return 0;
  }

  Cube *Molecule::newCube()
  {
    Q_D(Molecule);

    Cube *cube = new Cube(this);

    d->cubes.push_back(cube);
    cube->setId(d->cubes.size()-1);

    d->cubeList.push_back(cube);
    cube->setIndex(d->cubeList.size()-1);

    // now that the id is correct, emit the signal
    connect(cube, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(cube);
    return(cube);
  }

  void Molecule::deleteCube(Cube *cube)
  {
    Q_D(Molecule);
    if(cube) {
      d->cubes[cube->id()] = 0;
      // 0 based arrays stored/shown to user
      int index = cube->index();
      d->cubeList.removeAt(index);
      for (int i = index; i < d->cubeList.size(); ++i) {
        d->cubeList[i]->setIndex(i);
      }

      cube->deleteLater();
      disconnect(cube, SIGNAL(updated()), this, SLOT(updatePrimitive()));
      emit primitiveRemoved(cube);
      qDebug() << "Cube" << cube->id() << cube->index() << "deleted";
    }
  }

  void Molecule::deleteCube(unsigned long int id)
  {
    Q_D(Molecule);
    if (id < d->cubes.size())
      deleteCube(d->cubes[id]);
  }

  Residue * Molecule::newResidue()
  {
    Q_D(Molecule);

    Residue *residue = new Residue(this);

    d->residues.push_back(residue);
    residue->setId(d->residues.size()-1);

    d->residueList.push_back(residue);
    residue->setIndex(d->residueList.size()-1);

    // now that the id is correct, emit the signal
    connect(residue, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(residue);
    return(residue);
  }

  void Molecule::deleteResidue(Residue *residue)
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
      qDebug() << "Residue" << residue->id() << residue->index() << "deleted";
    }
  }

  void Molecule::deleteResidue(unsigned long int id)
  {
    Q_D(Molecule);
    if (id < d->residues.size())
      deleteResidue(d->residues[id]);
  }

  Fragment * Molecule::newRing()
  {
    Q_D(Molecule);

    Fragment *ring = new Fragment(this);

    d->rings.push_back(ring);
    ring->setId(d->rings.size()-1);

    d->ringList.push_back(ring);
    ring->setIndex(d->ringList.size()-1);

    // now that the id is correct, emit the signal
    connect(ring, SIGNAL(updated()), this, SLOT(updatePrimitive()));
//    emit primitiveAdded(ring);
    return(ring);
  }

  void Molecule::deleteRing(Fragment *ring)
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
      qDebug() << "Ring" << ring->id() << ring->index() << "deleted";
    }
  }

  void Molecule::deleteRing(unsigned long int id)
  {
    Q_D(Molecule);
    if (id < d->rings.size())
      deleteRing(d->rings[id]);
  }

  void Molecule::addHydrogens(Atom *atom)
  {
    // Construct an OBMol, call AddHydrogens and translate the changes
    OpenBabel::OBMol obmol = OBMol();
    if (atom)
      obmol.AddHydrogens(obmol.GetAtom(atom->index()+1));
    else
      obmol.AddHydrogens();
    // All new atoms in the OBMol must be the additional hydrogens
    for (unsigned int i = numAtoms()+1; i <= obmol.NumAtoms(); ++i) {
      if (obmol.GetAtom(i)->IsHydrogen()) {
        OpenBabel::OBAtom *obatom = obmol.GetAtom(i);
        Atom *atom = newAtom();
        atom->setOBAtom(obatom);
        // Get the neighbor atom
        OpenBabel::OBBondIterator iter;
        OpenBabel::OBAtom *next = obatom->BeginNbrAtom(iter);
        Bond *bond = newBond();
        bond->setEnd(Molecule::atom(atom->index()));
        bond->setBegin(Molecule::atom(next->GetIdx()-1));
        Molecule::atom(next->GetIdx()-1)->addBond(bond);
        atom->addBond(bond);
      }
    }
  }

  void Molecule::deleteHydrogens(Atom *atom)
  {
    if (atom) {
      // Delete any connected hydrogen atoms
      QList<unsigned long int> neighbors = atom->neighbors();

      foreach (unsigned long int a, neighbors) {
        Atom *nbrAtom = atomById(a);
        // we need to check if the atom still exists
        if (nbrAtom) {
          if (nbrAtom->isHydrogen()) {
            deleteAtom(a);
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
      Q_D(Molecule);
      foreach (Atom *atom, d->atomList) {
        if (atom->isHydrogen()) {
          deleteAtom(atom);
        }
      }
    }
  }

  unsigned int Molecule::numAtoms() const
  {
    Q_D(const Molecule);
    return d->atomList.size();
  }

  unsigned int Molecule::numBonds() const
  {
    Q_D(const Molecule);
    return d->bondList.size();
  }

  unsigned int Molecule::numCubes() const
  {
    Q_D(const Molecule);
    return d->cubeList.size();
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

  void Molecule::updatePrimitive()
  {
    Q_D(Molecule);
    Primitive *primitive = qobject_cast<Primitive *>(sender());
    d->invalidGeomInfo = true;
    emit primitiveUpdated(primitive);
  }

  void Molecule::update()
  {
    Q_D(Molecule);
    d->invalidGeomInfo = true;
    emit updated();
  }

  Bond* Molecule::bond(unsigned long int id1, unsigned long int id2)
  {
    // Take two atom IDs and see if we have a bond between the two
    Q_D(Molecule);
    foreach (Bond *bond, d->bondList) {
      if (bond->beginAtomId() == id1 && bond->endAtomId() == id2)
        return bond;
      if (bond->beginAtomId() == id2 && bond->endAtomId() == id1)
        return bond;
    }
    return 0;
  }

  Bond* Molecule::bond(const Atom *a1, const Atom *a2)
  {
    if (a1 && a2)
      return bond(a1->id(), a2->id());
    else
      return 0;
  }

  QList<Atom *> Molecule::atoms() const
  {
    Q_D(const Molecule);
    return d->atomList;
  }

  QList<Bond *> Molecule::bonds() const
  {
    Q_D(const Molecule);
    return d->bondList;
  }

  QList<Cube *> Molecule::cubes() const
  {
    Q_D(const Molecule);
    return d->cubeList;
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
        deleteRing(ring);
      }
      OpenBabel::OBMol obmol = OBMol();
      std::vector<OpenBabel::OBRing *> rings;
      rings = obmol.GetSSSR();
      foreach(OpenBabel::OBRing *r, rings) {
        Fragment *ring = newRing();
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
    foreach (Atom *atom, d->atomList) {
      OpenBabel::OBAtom *a = obmol.NewAtom();
      OpenBabel::OBAtom obatom = atom->OBAtom();
      *a = obatom;
    }
    foreach (Bond *bond, d->bondList) {
      obmol.AddBond(atomById(bond->beginAtomId())->index() + 1,
                    atomById(bond->endAtomId())->index() + 1, bond->order());
    }
    obmol.EndModify();

//    qDebug() << "OBMol() run" << obmol.NumAtoms() << obmol.NumBonds();

    return obmol;
  }

  bool Molecule::setOBMol(OpenBabel::OBMol *obmol)
  {
    // Take an OBMol, copy everything we need and store this object
    Q_D(Molecule);
    d->obmol = obmol;
    clear();
    // Copy all the parts of the OBMol to our Molecule

    // Begin by copying all of the atoms
    std::vector<OpenBabel::OBNodeBase*>::iterator i;
    for (OpenBabel::OBAtom *obatom = static_cast<OpenBabel::OBAtom *>(obmol->BeginAtom(i));
          obatom; obatom = static_cast<OpenBabel::OBAtom *>(obmol->NextAtom(i))) {
      Atom *atom = newAtom();
      atom->setOBAtom(obatom);
//      qDebug() << "Old atom:" << obatom->GetIdx() << obatom->GetX() << obatom->GetY() << obatom->GetZ() << obatom->GetAtomicNum();

//      qDebug() << "New atom:" << atom->index() << atom->pos().x() << atom->pos().y() << atom->pos().z() << atom->atomicNumber();
    }

    // Now bonds, we use the indices of the atoms to get the bonding right
    std::vector<OpenBabel::OBEdgeBase*>::iterator j;
    for (OpenBabel::OBBond *obbond = static_cast<OpenBabel::OBBond*>(obmol->BeginBond(j));
         obbond; obbond = static_cast<OpenBabel::OBBond*>(obmol->NextBond(j))) {
      Bond *bond = newBond();
      bond->setOBBond(obbond);
      // Get the begin and end atoms - we use a 0 based index, OB uses 1 based
      bond->setBegin(atom(obbond->GetBeginAtom()->GetIdx()-1));
      bond->setEnd(atom(obbond->GetEndAtom()->GetIdx()-1));
      // Set the bond to the atoms too, remember the 0 based and 1 based arrays
      atom(obbond->GetBeginAtom()->GetIdx()-1)->addBond(bond);
      atom(obbond->GetEndAtom()->GetIdx()-1)->addBond(bond);
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
      Cube *cube = newCube();
      cube->setLimits(min, max, points);
      cube->setData(grid->GetValues());
      cube->setName(name);
//      qDebug() << "Cube" << i << "added.";
    }

    // Copy the residues across...
    std::vector<OpenBabel::OBResidue *> residues;
    OpenBabel::OBResidueIterator iResidue;
    for (OpenBabel::OBResidue *obres = static_cast<OpenBabel::OBResidue *>(obmol->BeginResidue(iResidue));
          obres; obres = static_cast<OpenBabel::OBResidue *>(obmol->NextResidue(iResidue))) {
      /// Copy these residues!
      Residue *residue = newResidue();
      residue->setName(obres->GetName().c_str());
      residue->setNumber(obres->GetNumString().c_str());
      std::vector<OpenBabel::OBAtom*> obatoms = obres->GetAtoms();
      foreach (OpenBabel::OBAtom *obatom, obatoms) {
        unsigned long int atomId = atom(obatom->GetIdx()-1)->id();
        residue->addAtom(atomId);
        residue->setAtomId(atomId, obres->GetAtomID(obatom).c_str());
      }
      std::vector<OpenBabel::OBBond*> obbonds = obres->GetBonds();
      foreach (OpenBabel::OBBond *obbond, obbonds) {
        residue->addBond(bond(obbond->GetIdx())->id());
      }
    }

    // Copy the rings across now
    std::vector<OpenBabel::OBRing *> rings = obmol->GetSSSR();
    foreach(OpenBabel::OBRing *r, rings) {
      Fragment *ring = newRing();
      foreach(int index, r->_path) {
        ring->addAtom(atom(index-1)->id());
      }
    }

    return true;
  }

  const Eigen::Vector3d & Molecule::center() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->center;
  }

  const Eigen::Vector3d & Molecule::normalVector() const
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

  void Molecule::clear()
  {
    Q_D(Molecule);
    d->atoms.resize(0);
    foreach (Atom *atom, d->atomList) {
      atom->deleteLater();
      emit primitiveRemoved(atom);
    }
    d->atomList.clear();

    d->bonds.resize(0);
    foreach (Bond *bond, d->bondList) {
      bond->deleteLater();
      emit primitiveRemoved(bond);
    }
    d->bondList.clear();

    d->cubes.resize(0);
    foreach (Cube *cube, d->cubeList) {
      cube->deleteLater();
      emit primitiveRemoved(cube);
    }
    d->cubeList.clear();

    d->residues.resize(0);
    foreach (Fragment *residue, d->residueList) {
      residue->deleteLater();
      emit primitiveRemoved(residue);
    }
    d->residueList.clear();

    d->rings.resize(0);
    foreach (Fragment *ring, d->ringList) {
      ring->deleteLater();
      emit primitiveRemoved(ring);
    }
    d->ringList.clear();
  }

  Molecule &Molecule::operator=(const Molecule& other)
  {
    Q_D(Molecule);
    clear();
    const MoleculePrivate *e = other.d_func();
    d->atoms.resize(e->atoms.size(), 0);
    d->bonds.resize(e->bonds.size(), 0);

    // Copy the atoms and bonds over
    for (unsigned int i = 0; i < e->atoms.size(); ++i) {
      if (e->atoms.at(i) > 0) {
        Atom *atom = new Atom;
        *atom = *(e->atoms[i]);
        atom->setId(e->atoms[i]->id());
        atom->setIndex(e->atoms[i]->index());
        d->atoms[i] = atom;
        d->atomList.push_back(atom);
        emit primitiveAdded(atom);
      }
    }

    for (unsigned int i = 0; i < e->bonds.size(); ++i) {
      if (e->bonds.at(i)) {
        Bond *bond = new Bond;
        *bond = *(e->bonds[i]);
        bond->setId(e->bonds[i]->id());
        bond->setIndex(e->bonds[i]->index());
        d->bonds[i] = bond;
        d->bondList.push_back(bond);
        emit primitiveAdded(bond);
      }
    }

    return *this;
  }

  Molecule &Molecule::operator+=(const Molecule& other)
  {
    const MoleculePrivate *e = other.d_func();
    // Create a temporary map from the old indices to the new for bonding
    QList<int> map;
    foreach (Atom *a, e->atomList) {
      Atom *atom = newAtom();
      *atom = *a;
      map.push_back(atom->id());
      emit primitiveAdded(atom);
    }
    foreach (Bond *b, e->bondList) {
      Bond *bond = newBond();
      *bond = *b;
      bond->setBegin(atomById(map.at(other.atomById(b->beginAtomId())->index())));
      bond->setEnd(atomById(map.at(other.atomById(b->endAtomId())->index())));
      emit primitiveAdded(bond);
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
    d->radius = 0.0;
    if(numAtoms() != 0)
    {
      // compute center
      foreach (Atom *atom, d->atomList)
        d->center += atom->pos();

      d->center /= numAtoms();

      // compute the normal vector to the molecule's best-fitting plane
      int i = 0;
      Eigen::Vector3d ** atomPositions = new Eigen::Vector3d*[numAtoms()];
      foreach (Atom *atom, d->atomList)
        atomPositions[i++] = const_cast<Eigen::Vector3d*>(&atom->pos());

      Eigen::Hyperplane<double, 3> planeCoeffs;
      Eigen::fitHyperplane(numAtoms(), atomPositions, &planeCoeffs);
      delete[] atomPositions;
      d->normalVector = planeCoeffs.normal();

      // compute radius and the farthest atom
      d->radius = -1.0; // so that ( squaredDistanceToCenter > d->radius ) is true for at least one atom.
      foreach (Atom *atom, d->atomList) {
        double distanceToCenter = (atom->pos() - d->center).norm();
        if(distanceToCenter > d->radius) {
          d->radius = distanceToCenter;
          d->farthestAtom = atom;
        }
      }
    }
    d->invalidGeomInfo = false;
  }

} // End namespace Avogadro

#include "molecule.moc"
