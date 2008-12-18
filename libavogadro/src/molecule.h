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

#ifndef MOLECULE_H
#define MOLECULE_H

#include <avogadro/primitive.h>

#include <vector>

namespace OpenBabel {
  class OBAtom;
  class OBBond;
  class OBMol;
}

namespace Avogadro {

  // Declare new classes
  class Atom;
  class Bond;
  class Residue;
  class Cube;
  class Fragment;

  /**
   * @class Molecule molecule.h <avogadro/molecule.h>
   * @brief Molecule Class
   * @author Marcus D. Hanwell
   *
   * The Molecule class implements the core molecule data that is the cetral
   * model holding all information displayed by our various views and
   * manipulated by tools and extensions.
   */
  class MoleculePrivate;
  class A_EXPORT Molecule : public Primitive
  {
    Q_OBJECT

    public:
      /**
       * Constructor.
       *
       * @param parent The object parent.
       */
      Molecule(QObject *parent=0);

      /**
       * Copy constructor.
       * @param other Molecule to make a copy of.
       */
      Molecule(const Molecule &other);

      /**
       * Destructor.
       */
      virtual ~Molecule();

      /**
       * Call to trigger an update signal, causing the molecule to be redrawn.
       */
      void update();

      /**
       * Set the filename of the molecule.
       */
      void setFileName(const QString& name);

      /**
       * @return The full path filename of the molecule.
       */
      QString fileName() const;

      /**
       * Create a new atom object and return a pointer to it.
       * @note Do not delete the object, use deleteAtom(Atom*).
       */
      Atom *newAtom();

      /**
       * Create a new atom object with the specified id and return a pointer to
       * it. Used when you need to recreate an atom with the same unique id.
       * @note Do not delete the object, use deleteAtom(unsigned long int id).
       */
      Atom *newAtom(unsigned long id);

      /**
       * Set the atom position.
       * @param id Unique id of the atom to set the position for.
       * @param vec Position vector to set the atom to.
       */
      void setAtomPos(unsigned long int id, const Eigen::Vector3d &vec);

      /**
       * Get the position vector of the supplied atom.
       * @param id Unique id of the atom.
       * @return Position vector of the atom.
       */
      const Eigen::Vector3d * atomPos(unsigned long int id) const;

      /**
       * Delete the supplied atom.
       */
      void deleteAtom(Atom *atom);

      /**
       * Delete the atom with the unique id specified.
       */
      void deleteAtom(unsigned long int id);

      /**
       * Create a new Bond object and return a pointer to it.
       * @note Do not delete the object, use deleteBond(Bond*).
       */
      Bond *newBond();

      /**
       * Create a new bond object with the specified id and return a pointer to
       * it. Used when you need to recreate a bond with the same unique id.
       * @note Do not delete the object, use deleteBond(unsigned long int id).
       */
      Bond *newBond(unsigned long id);

      /**
       * Delete the supplied bond.
       */
      void deleteBond(Bond *bond);

      /**
       * Delete the bond with the unique id specified.
       */
      void deleteBond(unsigned long int id);

      /**
       * Add hydrogens to the molecule.
       * @param atom If supplied only add hydrogens to the specified atom.
       */
      void addHydrogens(Atom *atom = 0);

      /**
       * Delete hydrogens from the molecule.
       * @param atom If supplied only delete hydrogens connected to the
       * specified atom.
       */
      void deleteHydrogens(Atom *atom = 0);

      /**
       * Calculate the partial charges on each atom.
       */
      void calculatePartialCharges() const;

      /**
       * Create a new cube object and return a pointer to it.
       * @note Do not delete the object, use deleteCube(Cube *cube).
       */
      Cube *newCube();

      /**
       * Delete the supplied cube.
       */
      void deleteCube(Cube *cube);

      /**
       * Delete the cube with the unique id specified.
       */
      void deleteCube(unsigned long int id);

      /**
       * Create a new residue object and return a pointer to it.
       * @note Do not delete the object, use deleteResidue(Residue *residue).
       */
      Residue *newResidue();

      /**
       * Delete the supplied residue.
       */
      void deleteResidue(Residue *residue);

      /**
       * Delete the residue with the unique id specified.
       */
      void deleteResidue(unsigned long int id);

      /**
       * Create a new ring object and return a pointer to it.
       * @note Do not delete the object, use deleteRing(Fragment *ring).
       */
      Fragment *newRing();

      /**
       * Delete the supplied ring.
       */
      void deleteRing(Fragment *ring);

      /**
       * Delete the ring with the unique id specified.
       */
      void deleteRing(unsigned long int id);

      /**
       * @return The total number of atoms in the molecule.
       */
      unsigned int numAtoms() const;

      /**
       * @return The total number of bonds in the molecule.
       */
      unsigned int numBonds() const;

      /**
       * @return The total number of cubes in the molecule.
       */
      unsigned int numCubes() const;

      /**
       * @return The total number of residues in the molecule.
       */
      unsigned int numResidues() const;

      /**
       * @return The total number of rings in the molecule.
       */
      unsigned int numRings() const;

      /**
       * @return The atom at the supplied index.
       * @note Replaces GetAtom.
       */
      Atom* atom(int index);

      const Atom * atom(int index) const;

      /**
       * @return The atom at the supplied unqique id.
       */
      Atom *atomById(unsigned long id) const;

      /**
       * @return The bond at the supplied index.
       * @note Replaces GetBond.
       */
      Bond* bond(int index);

      /**
       * @return The bond at the supplied unique id.
       */
      Bond *bondById(unsigned long id) const;

      /**
       * @return The bond between the two supplied atom ids if one exists,
       * otherwise 0 is returned.
       */
      Bond* bond(unsigned long int id1, unsigned long int id2);

      /**
       * @return The bond between the two supplied atom pointers if one exists,
       * otherwise 0 is returned.
       */
      Bond* bond(const Atom*, const Atom*);

      /**
       * @return The residue at the supplied index.
       * @note Replaces GetResidue.
       */
      Residue* residue(int index) const;

      /**
       * @return The residue at the supplied unique id.
       */
      Residue *residueById(unsigned long id) const;

      /**
       * @return The Cube at the supplied index.
       * @note Replaces GetResidue.
       */
      Cube* cube(int index) const;

      /**
       * @return The Cube at the supplied unique id.
       */
      Cube *cubeById(unsigned long id) const;

      /**
       * @return QList of all atoms in the molecule.
       */
      QList<Atom *> atoms() const;

      /**
       * @return QList of all bonds in the molecule.
       */
      QList<Bond *> bonds() const;

      /**
       * @return QList of all cubes in the molecule.
       */
      QList<Cube *> cubes() const;

      /**
       * @return QList of all residues in the molecule.
       */
      QList<Residue *> residues() const;

      /**
       * @return QList of all rings in the molecule.
       */
      QList<Fragment *> rings();

      /**
       * Delete all elements of the molecule.
       */
      void clear();

      /**
       * Get access to an OpenBabel atom, this is a copy of the internal data
       * structure in OpenBabel form, you must call setOBAtom in order to save
       * any changes you make to this object.
       */
      OpenBabel::OBMol OBMol() const;

      /**
       * Copy as much data as possible from the supplied OpenBabel OBMol to the
       * Avogadro Molecule object.
       */
      bool setOBMol(OpenBabel::OBMol *obmol);

      const Eigen::Vector3d & center() const;
      const Eigen::Vector3d & normalVector() const;
      double radius() const;
      const Atom *farthestAtom() const;
      void translate(const Eigen::Vector3d&) { ; }

      Molecule& operator=(const Molecule& other);

      Molecule& operator+=(const Molecule& other);

    protected:
      MoleculePrivate * const d_ptr;

    private:
      /* shared d_ptr with Primitive */
      QString m_fileName;
      std::vector<Eigen::Vector3d> *m_atomPos;
      mutable bool m_invalidPartialCharges;
      Q_DECLARE_PRIVATE(Molecule)

      void computeGeomInfo() const;

    private Q_SLOTS:
      /**
       * Function which handles when a child primitive has been
       * updated.  The response is to find the sender object
       * and then emit a signal passing the sender as a parameter.
       *
       * @sa primitiveAdded
       * @sa primitiveUpdated
       * @sa primitiveRemoved
       */
      void updatePrimitive();

    Q_SIGNALS:
      /**
       * Emitted when a child primitive is added.
       *
       * @param primitive pointer to the primitive that was added
       */
      void primitiveAdded(Primitive *primitive);
      /**
       * Emitted when a child primitive is updated.
       *
       * @param primitive pointer to the primitive that was updated
       */
      void primitiveUpdated(Primitive *primitive);
      /**
       * Emitted when a child primitive is deleted.
       *
       * @param primitive pointer to the primitive that was updated before it is free'd
       */
      void primitiveRemoved(Primitive *primitive);
  };

} // End namespace Avogadro

#endif
