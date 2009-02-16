/**********************************************************************
  Molecule - Molecule class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell

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

#ifndef MOLECULE_H
#define MOLECULE_H

#include <avogadro/primitive.h>

#include <vector>

namespace OpenBabel {
  class OBAtom;
  class OBBond;
  class OBMol;
  class OBUnitCell;
}

namespace Avogadro {

  // Declare new classes
  class Atom;
  class Bond;
  class Residue;
  class Cube;
  class Mesh;
  class Fragment;

  /**
   * @class Molecule molecule.h <avogadro/molecule.h>
   * @brief The molecule contains all of the molecular primitives.
   * @author Marcus D. Hanwell
   *
   * The Molecule class contains the core data of the molecule. It is the cetral
   * model, holding all information displayed by our various views and
   * manipulated by tools and extensions. It typically contains Atom and Bond
   * objects, but may also contain Cube, Mesh, Fragment, Residue and other
   * objects.
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


    /** @name Molecule parameters
     * These methods set and get Molecule parameters.
     * @{
     */

    /**
     * Set the filename of the molecule.
     */
    void setFileName(const QString& name);

    /**
     * @return The full path filename of the molecule.
     */
    QString fileName() const;
    /** @} */

    /** @name Atom properties
     * These functions are used to change and retrieve the properties of the
     * Atom objects in the Molecule.
     * @{
     */

    /**
     * Create a new Atom object and return a pointer to it.
     * @note Do not delete the object, use removeAtom(Atom*).
     */
    Atom *addAtom();

    /**
     * Create a new Atom object with the specified id and return a pointer to
     * it. Used when you need to recreate an Atom with the same unique id.
     * @note Do not delete the object, use removeAtom(unsigned long id).
     */
    Atom *addAtom(unsigned long id);

    /**
     * Remove the supplied Atom.
     */
    void removeAtom(Atom *atom);

    /**
     * Delete the Atom with the unique id specified.
     */
    void removeAtom(unsigned long id);

    /**
     * @return The Atom at the supplied index.
     * @note Replaces GetAtom.
     */
    Atom * atom(int index);
    const Atom * atom(int index) const;

    /**
     * @return The Atom at the supplied unqique id.
     */
    Atom *atomById(unsigned long id) const;

    /**
     * @return QList of all Atom objects in the Molecule.
     */
    QList<Atom *> atoms() const;

    /**
     * Set the Atom position.
     * @param id Unique id of the Atom to set the position for.
     * @param vec Position vector to set the Atom to.
     */
    void setAtomPos(unsigned long id, const Eigen::Vector3d &vec);

    /**
     * Set the Atom position.
     * @param id Unique id of the Atom to set the position for.
     * @param vec Position vector to set the Atom to.
     */
    void setAtomPos(unsigned long id, const Eigen::Vector3d *vec);

    /**
     * Get the position vector of the supplied Atom.
     * @param id Unique id of the Atom.
     * @return Position vector of the Atom.
     */
    const Eigen::Vector3d * atomPos(unsigned long id) const;

    /**
     * @return The total number of Atom objects in the molecule.
     */
    unsigned int numAtoms() const;
    /** @} */


    /** @name Bond properties
     * These functions are used to change and retrieve the properties of the
     * Bond objects in the Molecule.
     * @{
     */

    /**
     * Create a new Bond object and return a pointer to it.
     * @note Do not delete the object, use removeBond(Bond*).
     */
    Bond *addBond();

    /**
     * Create a new Bond object with the specified id and return a pointer to
     * it. Used when you need to recreate a Bond with the same unique id.
     * @note Do not delete the object, use removeBond(unsigned long id).
     */
    Bond *addBond(unsigned long id);

    /**
     * Remove the supplied Bond.
     */
    void removeBond(Bond *bond);

    /**
     * Remove the Bond with the unique id specified.
     */
    void removeBond(unsigned long id);

    /**
     * @return The Bond at the supplied index.
     * @note Replaces GetBond.
     */
    Bond* bond(int index);
    const Bond* bond(int index) const;

    /**
     * @return The Bond at the supplied unique id.
     */
    Bond *bondById(unsigned long id) const;

    /**
     * @return QList of all Bond objects in the Molecule.
     */
    QList<Bond *> bonds() const;

    /**
     * @return The total number of Bond objects in the Mmolecule.
     */
    unsigned int numBonds() const;
    /** @} */

    /** @name Residue properties
     * These functions are used to change and retrieve the properties of the
     * Residue objects in the Molecule.
     * @{
     */

    /**
     * Create a new Residue object and return a pointer to it.
     * @note Do not delete the object, use removeResidue(Residue *residue).
     */
    Residue *addResidue();

    /**
     * Create a new Residue object with the specified id and return a pointer to
     * it. Used when you need to recreate a Residue with the same unique id.
     * @note Do not delete the object, use removeResidue(unsigned long id).
     */
    Residue *addResidue(unsigned long id);

    /**
     * Remove the supplied residue.
     */
    void removeResidue(Residue *residue);

    /**
     * Remove the residue with the unique id specified.
     */
    void removeResidue(unsigned long id);

    /**
     * @return The residue at the supplied index.
     * @note Replaces GetResidue.
     */
    Residue* residue(int index);
    const Residue* residue(int index) const;

    /**
     * @return The residue at the supplied unique id.
     */
    Residue *residueById(unsigned long id) const;

    /**
     * @return QList of all Residue objects in the Molecule.
     */
    QList<Residue *> residues() const;

    /**
     * @return The total number of Residue objects in the Molecule.
     */
    unsigned int numResidues() const;
    /** @} */

    /** @name Ring properties
     * These functions are used to change and retrieve the properties of the
     * Ring objects in the Molecule.
     * @note These should probably be generalized to Fragments, but then a
     * convenience function to return a list of just rings would be needed.
     * @{
     */

    /**
     * Create a new ring object and return a pointer to it.
     * @note Do not delete the object, use removeRing(Fragment *ring).
     */
    Fragment *addRing();

    /**
     * Create a new Ring object with the specified id and return a pointer to
     * it. Used when you need to recreate a Ring with the same unique id.
     * @note Do not delete the object, use removeRing(unsigned long id).
     */
    Fragment *addRing(unsigned long id);

    /**
     * Remove the supplied ring.
     */
    void removeRing(Fragment *ring);

    /**
     * Remove the ring with the unique id specified.
     */
    void removeRing(unsigned long id);

    /**
     * @return QList of all rings in the Molecule.
     */
    QList<Fragment *> rings();

    /**
     * @return The total number of rings in the molecule.
     */
    unsigned int numRings() const;
    /** @} */

    /** @name Cube properties
     * These functions are used to change and retrieve the properties of the
     * Cube objects in the Molecule.
     * @{
     */

    /**
     * Create a new Cube object and return a pointer to it.
     * @note Do not delete the object, use removeCube(unsigned long id).
     */
    Cube *addCube();

    /**
     * Create a new Cube object with the specified id and return a pointer to
     * it. Used when you need to recreate a Cube with the same unique id.
     * @note Do not delete the object, use removeCube(unsigned long id).
     */
    Cube *addCube(unsigned long id);

    /**
     * Remove the supplied Cube.
     */
    void removeCube(Cube *cube);

    /**
     * Remove the Cube with the unique id specified.
     */
    void removeCube(unsigned long id);

    /**
     * @return The Cube at the supplied index.
     */
    Cube* cube(int index) const;

    /**
     * @return The Cube at the supplied unique id.
     */
    Cube *cubeById(unsigned long id) const;

    /**
     * @return QList of all cubes in the molecule.
     */
    QList<Cube *> cubes() const;

    /**
     * @return The total number of Cube objects in the Molecule.
     */
    unsigned int numCubes() const;
    /** @} */

    /** @name Mesh properties
     * These functions are used to change and retrieve the properties of the
     * Mesh objects in the Molecule.
     * @{
     */

    /**
     * Create a new Mesh object and return a pointer to it.
     * @note Do not delete the object, use removeMesh(unsigned long id).
     */
    Mesh * addMesh();

    /**
     * Create a new Mesh object with the specified id and return a pointer to
     * it. Used when you need to recreate a Mesh with the same unique id.
     * @note Do not delete the object, use removeMesh(unsigned long id).
     */
    Mesh *addMesh(unsigned long id);

    /**
     * Remove the supplied Mesh.
     */
    void removeMesh(Mesh *mesh);

    /**
     * Remove the Mesh with the unique id specified.
     */
    void removeMesh(unsigned long id);

    /**
     * @return The Mesh at the supplied index.
     */
    Mesh* mesh(int index) const;

    /**
     * @return The Mesh at the supplied unique id.
     */
    Mesh *meshById(unsigned long id) const;

    /**
     * @return QList of all Mesh objects in the Molecule.
     */
    QList<Mesh *> meshes() const;

    /**
     * @return The total number of meshes in the molecule.
     */
    unsigned int numMeshes() const;
    /** @} */

    /**
     * Add hydrogens to the molecule.
     * @param atom If supplied only add hydrogens to the specified atom.
     */
    void addHydrogens(Atom *atom = 0);

    /**
     * Remove all hydrogens from the molecule.
     * @param atom If supplied only remove hydrogens connected to the
     * specified atom.
     */
    void removeHydrogens(Atom *atom = 0);

    /**
     * Set the dipole moment of the Molecule.
     * @param moment The dipole moment of the Molecule.
     */
    void setDipoleMoment(const Eigen::Vector3d &moment);

    /**
     * @return The dipole moment of the Molecule.
     */
    const Eigen::Vector3d * dipoleMoment() const;

    /**
     * Calculate the partial charges on each atom.
     */
    void calculatePartialCharges() const;

    /**
     * Calculate the aromaticity of the bonds.
     */
    void calculateAromaticity() const;

    /**
     * @return The bond between the two supplied atom ids if one exists,
     * otherwise 0 is returned.
     */
    Bond* bond(unsigned long id1, unsigned long id2);

    /**
     * @return The bond between the two supplied atom pointers if one exists,
     * otherwise 0 is returned.
     */
    Bond* bond(const Atom*, const Atom*);

    /**
     * Add a new conformer to the Molecule. The conformers are mapped onto the
     * unique ids of the atoms in the Molecule.
     * @param conformer A vector of Vector3d with all atom positions.
     * @param index The index of the conformer to add.
     */
    bool addConformer(const std::vector<Eigen::Vector3d> &conformer, unsigned int index);

    /**
     * Add a new conformer and return a pointer to it.
     * @param index The index of the new conformer.
     * @return Pointer to the conformer added.
     */
    std::vector<Eigen::Vector3d> * addConformer(unsigned int index);

    /**
     * Change the conformer to the one at the specified index.
     */
    bool setConformer(unsigned int index);

    /**
     * Clear all conformers from the molecule, leaving just conformer zero.
     */
    void clearConformers();

    /**
     * @return The number of conformers.
     */
    unsigned int numConformers() const;

    /**
     * @return The energies for all conformers.
     */
    const std::vector<double>& energies() const;

    /**
     * Get the energy of the supplied conformer, defaults to returning the
     * energy of the current conformer.
     * @param index The conformer, defaults to the current conformer.
     * @return The energy of the Molecule (or current conformer).
     */
    double energy(unsigned int index = -1) const;

    /**
     * Set the energies for all conformers.
     */
    void setEnergies(const std::vector<double>& energies);

    /**
     * Remove all elements of the molecule.
     */
    void clear();

    /** @name OpenBabel translation functions
     * These functions are used to exchange information with OpenBabel.
     * @{
     */

    /**
     * Get access to an OpenBabel::OBMol, this is a copy of the internal data
     * structure in OpenBabel form, you must call setOBMol in order to save
     * any changes you make to this object.
     */
    OpenBabel::OBMol OBMol() const;

    /**
     * Copy as much data as possible from the supplied OpenBabel::OBMol to the
     * Avogadro Molecule object.
     */
    bool setOBMol(OpenBabel::OBMol *obmol);

    /**
     * Get access to the OpenBabel unit cell, if any
     * @return the OBUnitCell or NULL if none exists
     */
    OpenBabel::OBUnitCell *OBUnitCell() const;

    /**
     * Copy as much data as possible from the supplied OpenBabel::OBUnitCell
     * to this Avogadro Molecule object.
     * @return True if successful
     */
    bool setOBUnitCell(OpenBabel::OBUnitCell *obunitcell);
    /** @} */

    /** @name Molecule geometry information and manipulation
     * These functions can be used to retrieve several aspects of Molecule
     * geometry and to manipulate some aspects.
     * @{
     */

    /**
     * @return The position of the center of the Molecule.
     */
    const Eigen::Vector3d center() const;

    /**
     * @return The normal vector of the Molecule.
     */
    const Eigen::Vector3d normalVector() const;

    /**
     * @return The radius of the Molecule.
     */
    double radius() const;

    /**
     * @return The Atom furthest away from the center of the Molecule.
     */
    const Atom *farthestAtom() const;

    /**
     * Translate the Molecule using the supplied vector.
     */
    void translate(const Eigen::Vector3d&);
    /** @} */

    /** @name Operators
     * Overloaded operators.
     * @{
     */

    /**
     * Assignment operator used to set this Molecule equal to other.
     */
    Molecule& operator=(const Molecule& other);

    /**
     * Addition operator used to add elements from the other Molecule to this
     * one.
     */
    Molecule& operator+=(const Molecule& other);
    /** @} */

  protected:
    MoleculePrivate * const d_ptr;
    QString m_fileName;
    std::vector<Eigen::Vector3d> *m_atomPos; // Atom position vector
    /** Vector containing pointers to various conformers. **/
    std::vector< std::vector<Eigen::Vector3d>* > m_atomConformers;
    mutable Eigen::Vector3d *m_dipoleMoment;
    mutable bool m_invalidPartialCharges;
    mutable bool m_invalidAromaticity;
    Q_DECLARE_PRIVATE(Molecule)

    /**
     * Compute all the geometry information for the Molecule. This allows
     * several relatively expensive calculations to be cached by the Molecule
     * instead of being recalculated every time the Molecule is drawn.
     */
    void computeGeomInfo() const;

  private Q_SLOTS:
    /**
     * Function which handles when a child primitive has been
     * updated.  The response is to find the sender object
     * and then emit a signal passing the sender as a parameter.
     * @sa primitiveAdded
     * @sa primitiveUpdated
     * @sa primitiveRemoved
     */
    void updatePrimitive();

    /**
     * Slot that handles when an atom has been updated.
     * @sa atomAdded
     * @sa atomUpdated
     * @sa atomRemoved
     */
    void updateAtom();

    /**
     * Slot that handles when a bond has been updated.
     * @sa bondAdded
     * @sa bondUpdated
     * @sa bondRemoved
     */
    void updateBond();

  Q_SIGNALS:
    /**
     * Emitted when a child primitive is added.
     * @param primitive pointer to the primitive that was added
     */
    void primitiveAdded(Primitive *primitive);

    /**
     * Emitted when a child primitive is updated.
     * @param primitive pointer to the primitive that was updated
     */
    void primitiveUpdated(Primitive *primitive);

    /**
     * Emitted when a child primitive is removed.
     * @param primitive pointer to the primitive that was removed.
     */
    void primitiveRemoved(Primitive *primitive);

    /**
     * Emitted when an Atom is added.
     * @param Atom pointer to the Atom that was added.
     */
    void atomAdded(Atom *atom);

    /**
     * Emitted when an Atom is updated.
     * @param Atom pointer to the Atom that was updated.
     */
    void atomUpdated(Atom *atom);

    /**
     * Emitted when an Atom is removed.
     * @param Atom pointer to the Atom that was removed.
     */
    void atomRemoved(Atom *atom);

    /**
     * Emitted when a Bond is added.
     * @param Bond pointer to the bond that was added.
     */
    void bondAdded(Bond *bond);

    /**
     * Emitted when a Bond is updated.
     * @param Bond pointer to the bond that was updated.
     */
    void bondUpdated(Bond *bond);

    /**
     * Emitted when a Bond is removed.
     * @param Bond pointer to the Bond that was removed.
     */
    void bondRemoved(Bond *bond);
  };

} // End namespace Avogadro

#endif
