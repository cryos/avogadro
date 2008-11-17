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
   * model holding all data by our various views. 
   */
  class MoleculePrivate;
  class A_EXPORT Molecule : public Primitive
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Molecule(QObject *parent=0);
      Molecule(const Molecule &other);
      virtual ~Molecule();
      void update();

      /**
       * Virtual function inherited from OpenBabel::OBMol.
       * Creates a new Atom object.
       *
       * @return pointer to a newly allocated Atom object
       */
//      Atom *CreateAtom();
//      void createAtom(Atom *) { ; }

      /**
       * Virtual function inherited from OpenBabel::OBMol.
       * Creates a new Bond object.
       *
       * @return pointer to a newly allocated Bond object
       */
//      Bond *CreateBond();

      /**
       * Virtual function inherited from OpenBabel::OBMol.
       * Creates a new Residue object.
       *
       * @return pointer to a newly allocated Residue object
       */
//      Residue *CreateResidue();

      /**
       * Virtual function inherited from OpenBabel::OBMol.
       * Deletes an Atom object.
       *
       * @param atom the atom to delete
       */
//      void DestroyAtom(Atom* atom);

      /**
       * Virtual function inherited from OpenBabel::OBMol.
       * Deletes an Bond object.
       *
       * @param bond the bond to delete
       */
//      void DestroyBond(Bond* bond);

      /**
       * Virtual function inherited from OpenBabel::OBMol.
       * Deletes an Residue object.
       *
       * @param residue the residue to delete
       */
//      void DestroyResidue(Residue* residue);

      /**
       * These are new functions to replace OBMol::NewAtom
       * to use our new unique identifier functionality.
       */
      Atom *newAtom();
      Atom *newAtom(unsigned long id);
      void deleteAtom(Atom *atom);
      void deleteAtom(unsigned long int id);

      /**
       * These are new functions to replace OBMol::NewBond
       * to use our new unique identifier functionality.
       */
      Bond *newBond();
      Bond *newBond(unsigned long id);
      void deleteBond(Bond *bond);
      void deleteBond(unsigned long int id);

      void addHydrogens(Atom *atom = 0);
      void deleteHydrogens(Atom *atom);
      void deleteHydrogens();

      Cube *newCube();
      void deleteCube(Cube *cube);
      void deleteCube(unsigned long int id);

      Fragment *newResidue();
      void deleteResidue(Fragment *residue);
      void deleteResidue(unsigned long int id);

      Fragment *newRing();
      void deleteRing(Fragment *ring);
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
       * @return the atom at the supplied index.
       * @note Replaces GetAtom.
       */
      Atom* atom(int index);

      /**
       * @return the atom at the supplied unqique id.
       */
      Atom *atomById(unsigned long id) const;

      /**
       * @return the bond at the supplied index.
       * @note Replaces GetBond.
       */
      Bond* bond(int index);

      /**
       * @return the bond at the supplied unique id.
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
       * @return a QList of all atoms in the molecule.
       */
      QList<Atom *> atoms() const;

      /**
       * @return a QList of all bonds in the molecule.
       */
      QList<Bond *> bonds() const;

      /**
       * @return a QList of all cubes in the molecule.
       */
      QList<Cube *> cubes() const;

      /**
       * @return a QList of all residues in the molecule.
       */
      QList<Fragment *> residues() const;

      /**
       * @return a QList of all rings in the molecule.
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
