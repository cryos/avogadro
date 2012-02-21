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

#ifndef MOLECULE_H
#define MOLECULE_H

#include <avogadro/primitive.h>

// Used by the inline functions
#include <QReadWriteLock>

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
  class Cube;
  class Fragment;
  class Mesh;
  class PrimitiveList;
  class Residue;
  class ZMatrix;

  /**
   * @class Molecule molecule.h <avogadro/molecule.h>
   * @brief The molecule contains all of the molecular primitives.
   * @author Marcus D. Hanwell
   *
   * The Molecule class contains the core data of the molecule. It is the central
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
     * @overload
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
    * @overload
     * Create a new Atom object of the specified element at the given
     * position, and return a pointer to it.
     * @param atomicNum Atomic number
     * @param pos Cartesian position
     * @note Do not delete the object, use removeAtom(unsigned long id).
     */
    Atom *addAtom(int atomicNum, const Eigen::Vector3d &pos);

    /**
     * @overload
     * Create a new Atom object and copy the Atom object @a other's data into
     * it. A pointer to the new atom in @a this is returned.
     * @param other The atom to copy into the new atom
     * @note Do not delete the new object, use removeAtom(unsigned long id).
     */
    Atom *addAtom(const Atom &other);

    /**
     * Remove the supplied Atom.
     */
    void removeAtom(Atom *atom);

    /**
     * Delete the Atom with the unique id specified.
     */
    void removeAtom(unsigned long id);

    /**
     * Delete the list of Atom objects specified.
     * @since version 1.1
     */
    void removeAtoms(QList<Atom*> atoms);

    /**
     * @return The Atom at the supplied index.
     * @note Replaces GetAtom.
     */
    Atom * atom(int index) const;

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
     * @overload
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
     * @overload
     * Create a new bond between two atoms of the specified order. A pointer
     * to the new Bond is returned.
     * @param beginAtomId ID of the beginning atom of the bond
     * @param endAtomId ID of the ending atom of the bond
     * @param order Bond order of the new bond (default 1)
     * @note Do not delete the object, use removeBond(Bond*).
     */
    Bond *addBond(unsigned long beginAtomId, unsigned long endAtomId,
                  short order = 1);

    /**
     * @overload
     * Create a new bond between two atoms of the specified order. A pointer
     * to the new Bond is returned.
     * @param beginAtom Pointer to the beginning atom of the bond
     * @param endAtom Pointer to the ending atom of the bond
     * @param order Bond order of the new bond (default 1)
     * @note Do not delete the object, use removeBond(Bond*).
     * @note Both beginAtom and endAtom must be owned by @a this Molecule.
     */
    Bond *addBond(Atom *beginAtom, Atom* endAtom, short order = 1);

    /**
     * Remove the supplied Bond.
     */
    void removeBond(Bond *bond);

    /**
     * Remove the Bond with the unique id specified.
     */
    void removeBond(unsigned long id);

    /**
     * Remove all Bond objects in the list specified.
     * @since version 1.1
     */
    void removeBonds(QList<Bond *>bonds);

    /**
     * @return The Bond at the supplied index.
     * @note Replaces GetBond.
     */
    Bond* bond(int index) const;

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
     * Remove the supplied Residue.
     */
    void removeResidue(Residue *residue);

    /**
     * Remove the Residue with the unique id specified.
     */
    void removeResidue(unsigned long id);

    /**
     * Remove all Residue objects in the list specified.
     */
    void removeResidues(QList<Residue*> residues);

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

    /** @name ZMatrix properties
     * These functions are used to change and retrieve the properties of the
     * ZMatrix objects in the Molecule.
     * @{
     */

    /**
     * Create a new ZMatrix object and return a pointer to it.
     * @note Do not delete the object, use removeZMatrix(unsigned long id).
     */
    ZMatrix * addZMatrix();

    /**
     * Remove the supplied ZMatrix.
     */
    void removeZMatrix(ZMatrix *zmatrix);

    /**
     * @return The ZMatrix at the supplied index.
     */
    ZMatrix * zMatrix(int index) const;

    /**
     * @return QList of all ZMatrix objects in the Molecule.
     */
    QList<ZMatrix *> zMatrices() const;

    /**
     * @return The total number of ZMatrix objects in the molecule.
     */
    unsigned int numZMatrices() const;
    /** @} */

    /**
     * Add hydrogens to the molecule.
     * @param atom If supplied only add hydrogens to the specified atom.
     * @param atomIds Unique Atom IDs when adding hydrogens in undo/redo.
     * @param bondIds Unique Bond IDs when adding hydrogens in undo/redo.
     */
    void addHydrogens(Atom *atom = 0,
                      const QList<unsigned long> &atomIds = QList<unsigned long>(),
                      const QList<unsigned long> &bondIds = QList<unsigned long>());

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
     * Calculate the dipole moment vector for this molecule. If not known,
     * Avogadro will estimate it based on partial charges.
     *
     * @param A boolean to indicate if the dipole is estimated or not
     * @return The dipole moment of the Molecule.
     */
    Eigen::Vector3d dipoleMoment(bool *estimate = 0) const;

    /**
     * Calculate the partial charges on each atom.
     */
    void calculatePartialCharges() const;

    /**
     * Calculate the aromaticity of the bonds.
     */
    void calculateAromaticity() const;

    /**
     * Calculate the indices of atoms in groups of atoms of the same element.
     */
    void calculateGroupIndices() const;

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
     * Get the current conformer size to accommodate all atoms. Since atom
     * positions are indexed by their uniaue id, this is not the same as the
     * number of atoms
     @code
     Molecule *molecule;
     ...
     // create a correctly sized conformer and initialize its elements
     std::vector<Eigen::Vector3d> newConformer(molecule->conformerSize(), Eigen::Vector3d::Zero())
     // change the atom posistions in the conformer
     foreach (Atom *atom, molecule->atoms()) {
       newConformer[atom->id()] = ...;
     }
     // add the conformer to the end of the current list (i.e. index = numConformers())
     molecule->addConformer(newConformer, molecule->numConformers())
     @endcode
     */
    unsigned long conformerSize() { return m_atomPos->size(); }

    /**
     * Add a new conformer to the Molecule. If the index is an already existing
     * conformer, this conformer will be overwritten. If there is a gap between
     * the currently last index and the new index, new conformers will be
     * created to make sure all indexes have a valid conformer. For example, if
     * there is only one conformer and this method is called with @p index 9, a
     * total of nine conformers are added (0 existed, 1-9 are created).
     *
     * @note Conformer atom positions are indexed by their unique id (Atom::id()).
     * Use conformerSize() to check the current size needed to accommodate all
     * atoms.
     *
     * @param conformer A vector of Vector3d with all atom positions.
     * @param index The index of the conformer to add.
     */
    bool addConformer(const std::vector<Eigen::Vector3d> &conformer,
                      unsigned int index);

    /**
     * Add a new conformer and return a pointer to it. If the index is an
     * already existing conformer, no new conformer will be created and a
     * pointer to the already existing conformer is returned. If there is
     * a gap between the currently last index and the new index, new
     * conformers will be created to make sure all indexes have a valid
     * conformer. For example, if there is only one conformer and this
     * method is called with @p index 9, a total of nine conformers are
     * added (0 existed, 1-9 are created) and a pointer to the last one is
     * returned.
     *
     * @note Conformer atom positions are indexed by their unique id (Atom::id()).
     * Use conformerSize() to check the current size needed to accommodate all
     * atoms.
     *
     * @param index The index of the new conformer.
     * @return Pointer to the conformer added.
     */
    std::vector<Eigen::Vector3d> * addConformer(unsigned int index);

    /**
     * @note Conformer atom positions are indexed by their unique id (Atom::id()).
     * Use conformerSize() to check the current size needed to accommodate all
     * atoms.
     * @param index The index of the conformer to retrieve.
     * @return Pointer to an existing conformer, or NULL if the index doesn't exist.
     */
    std::vector<Eigen::Vector3d> * conformer(unsigned int index);

    /**
     * Get const reference to all conformers.
     *
     * @note Conformer atom positions are indexed by their unique id (Atom::id()).
     * Use conformerSize() to check the current size needed to accommodate all
     * atoms.
     */
    const std::vector<std::vector<Eigen::Vector3d> *>& conformers() const;

    /**
     * Change the conformer to the one at the specified index. Conformers are
     * indexed from 0 to numConformers() - 1. When the index is invalid (i.e.
     * >= numConformers()), this method returns false.
     *
     * @return True if the conformer index is valid.
     */
    bool setConformer(unsigned int index);

    /**
     * Replace all conformers in the Molecule. This will first clear all
     * conformers. If the number of specified @p conformers is 0, this method
     * behaves like clearConformers(). In any case, the current conformer is
     * set to the first one (index 0).
     *
     * @note Conformer atom positions are indexed by their unique id (Atom::id()).
     * Use conformerSize() to check the current size needed to accommodate all
     * atoms.
     *
     * @param conformer A vector of conformers (vector of Vector3d)
     * @param deleteExisting Whether to free the memory from the existing conformers
     * @return True if successful (i.e. all conformers have the correct size: conformerSize()).
     */
    bool setAllConformers(const std::vector< std::vector<Eigen::Vector3d>* > conformers, bool deleteExisting = true);

    /**
     * Clear all conformers from the molecule, leaving just conformer zero. This
     * conformer will also be set as current conformer.
     */
    void clearConformers();

    /**
     * @return The number of conformers.
     */
    unsigned int numConformers() const;

    /**
     * @return The current conformer index.
     */
    unsigned int currentConformer() const;

    /**
     * @return The energies for all conformers (in kJ/mol).
     */
    const std::vector<double>& energies() const;

    /**
     * Get the energy of the supplied conformer (in kJ/mol), defaults to returning the
     * energy of the current conformer.
     * @param index The conformer, defaults to the current conformer.
     * @return The energy of the Molecule (or current conformer).
     */
    double energy(int index = -1) const;

    /**
     * Set the energy for the current conformer (in kJ/mol).
     * @param energy The value for this conformer
     */
    void setEnergy(double energy);

    /**
     * Set the energy for the specified conformer (in kJ/mol).
     * @param index the index of the confomer
     * @param energy The energy for this conformer
     */
    void setEnergy(int index, double energy);

    /**
     * Set the energies for all conformers (in kJ/mol).
     * @param energies The vector of all energy values
     */
    void setEnergies(const std::vector<double>& energies);

    /**
     * Remove all elements of the molecule.
     */
    void clear();

    /**
     * Provides locking, should be used before reading/writing to the Molecule.
     */
    QReadWriteLock *lock() const;

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

    /** @name Misc.
     * Functions that don't neatly fit into the above categories.
     * @{
     */

    /**
     * Copy the atoms and bonds in the argument lists into new atoms and bonds
     * in @a this.
     * @param atoms Atoms to copy into @a this.
     * @param bonds Bonds to copy into @a this.
     * @return A PrimitiveList containing new atoms and bonds.
     * @note All atoms and bonds must belong to the same molecule.
     * @note All bonds in @bonds must be between atoms in @atoms.
     */
    PrimitiveList copyAtomsAndBonds(const QList<Atom*> &atoms,
                                    const QList<Bond*> &bonds);

    /**
     * @overload
     * Copy the atoms and bonds in the argument lists into new atoms and bonds
     * in @a this.
     * @param atomsAndBonds Atoms and bonds to copy into @a this.
     * @return A PrimitiveList containing the new atoms and bonds.
     * @note All atoms and bonds must belong to the same molecule.
     * @note All bonds in the list must be between atoms in the list.
     * @note The QList overload of this function is faster.
     */
    PrimitiveList copyAtomsAndBonds(const PrimitiveList &atomsAndBonds);
    /** @} */

  protected:
    MoleculePrivate * const d_ptr;
    QString m_fileName;
    std::vector<Eigen::Vector3d> *m_atomPos; // Atom position vector
    /** Vector containing pointers to various conformers. **/
    std::vector< std::vector<Eigen::Vector3d>* > m_atomConformers;
    mutable unsigned int m_currentConformer;

    mutable bool m_estimatedDipoleMoment;
    mutable Eigen::Vector3d *m_dipoleMoment;
    mutable bool m_invalidPartialCharges;
    mutable bool m_invalidAromaticity;
    Q_DECLARE_PRIVATE(Molecule)

    std::vector<Atom *>   m_atoms;
    std::vector<Bond *>   m_bonds;
    QList<Atom *>         m_atomList;
    QList<Bond *>         m_bondList;

    QReadWriteLock *m_lock;

    /**
     * Compute all the geometry information for the Molecule. This allows
     * several relatively expensive calculations to be cached by the Molecule
     * instead of being recalculated every time the Molecule is drawn.
     */
    void computeGeomInfo() const;

  public Q_SLOTS:
    /**
     * Signal that the molecule has been changed in some large way, emits the
     * moleculeChanged and updated signals. This indicates that anything
     * listening to this signal should tear town its cache/model and rebuild it.
     */
    void updateMolecule();

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
     * Emitted when the Molecule changes in a big way, e.g. thousands of atoms
     * added/removed. Typically functions should respond by building up their
     * interpretation of the Molecule from the beginning.
     */
    void moleculeChanged();

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

  inline Atom * Molecule::atom(int index) const
  {
    if (index >= 0 && index < m_atomList.size())
      return m_atomList[index];
    else
      return 0;
  }

  inline Atom * Molecule::atomById(unsigned long id) const
  {
    if(id < m_atoms.size() && id != FALSE_ID)
      return m_atoms[id];
    else
      return 0;
  }

  inline const Eigen::Vector3d * Molecule::atomPos(unsigned long id) const
  {
    if (id < m_atomPos->size() && id != FALSE_ID)
      return &(*m_atomPos)[id];
    else
      return 0;
  }

  inline Bond * Molecule::bond(int index) const
  {
    if (index >= 0 && index < m_bondList.size())
      return m_bondList[index];
    else
      return 0;
  }

  inline Bond * Molecule::bondById(unsigned long id) const
  {
    if(id < m_bonds.size() && id != FALSE_ID)
      return m_bonds[id];
    else
      return 0;
  }

} // End namespace Avogadro

#endif
