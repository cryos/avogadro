// Last update: timvdm 18 June 2009
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/cube.h>
#include <avogadro/mesh.h>
#include <avogadro/zmatrix.h>

#include <openbabel/mol.h>

using namespace boost::python;
using namespace Avogadro;

// defined in swig.cpp
PyObject* Molecule_OBMol(Avogadro::Molecule &self);
void Molecule_setOBMol(Molecule &self, PyObject *obj);

void copy(Molecule &self, const Molecule &from)
{
  self = from;
}

// thin wrappers to handle default arguments
void addHydrogens1(Molecule &self)
{
  self.addHydrogens();
}
void addHydrogens2(Molecule &self, Atom *atom)
{
  self.addHydrogens(atom);
}
void removeHydrogens(Molecule &self)
{
  self.removeHydrogens();
}

double energy(Molecule &self)
{
  return self.energy();
}

void export_Molecule()
{

  // define function pointers to handle overloading
  Atom* (Molecule::*addAtom_ptr1)() = &Molecule::addAtom;
  Atom* (Molecule::*addAtom_ptr2)(unsigned long) = &Molecule::addAtom;
  void (Molecule::*removeAtom_ptr1)(Atom*) = &Molecule::removeAtom;
  void (Molecule::*removeAtom_ptr2)(unsigned long) = &Molecule::removeAtom;
  //void (Molecule::*setAtomPos_ptr1)(unsigned long, const Eigen::Vector3d &) = &Molecule::setAtomPos;
  Bond* (Molecule::*addBond_ptr1)() = &Molecule::addBond;
  Bond* (Molecule::*addBond_ptr2)(unsigned long) = &Molecule::addBond;
  void (Molecule::*removeBond_ptr1)(Bond*) = &Molecule::removeBond;
  void (Molecule::*removeBond_ptr2)(unsigned long) = &Molecule::removeBond;
  Cube* (Molecule::*addCube_ptr1)() = &Molecule::addCube;
  Cube* (Molecule::*addCube_ptr2)(unsigned long) = &Molecule::addCube;
  Cube* (Molecule::*cube_ptr)(int) const = &Molecule::cube;
  void (Molecule::*removeCube_ptr1)(Cube*) = &Molecule::removeCube;
  void (Molecule::*removeCube_ptr2)(unsigned long) = &Molecule::removeCube;
  Mesh* (Molecule::*addMesh_ptr1)() = &Molecule::addMesh;
  Mesh* (Molecule::*addMesh_ptr2)(unsigned long) = &Molecule::addMesh;
  Mesh* (Molecule::*mesh_ptr)(int) const = &Molecule::mesh;
  void (Molecule::*removeMesh_ptr1)(Mesh*) = &Molecule::removeMesh;
  void (Molecule::*removeMesh_ptr2)(unsigned long) = &Molecule::removeMesh;
  Residue* (Molecule::*addResidue_ptr1)() = &Molecule::addResidue;
  Residue* (Molecule::*addResidue_ptr2)(unsigned long) = &Molecule::addResidue;
  void (Molecule::*removeResidue_ptr1)(Residue*) = &Molecule::removeResidue;
  void (Molecule::*removeResidue_ptr2)(unsigned long) = &Molecule::removeResidue;
  Fragment* (Molecule::*addRing_ptr1)() = &Molecule::addRing;
  Fragment* (Molecule::*addRing_ptr2)(unsigned long) = &Molecule::addRing;
  void (Molecule::*removeRing_ptr1)(Fragment*) = &Molecule::removeRing;
  void (Molecule::*removeRing_ptr2)(unsigned long) = &Molecule::removeRing;
  Atom* (Molecule::*atom_ptr)(int)const = &Molecule::atom;
  Bond* (Molecule::*bond_ptr1)(int)const = &Molecule::bond;
  Bond* (Molecule::*bond_ptr2)(unsigned long, unsigned long) = &Molecule::bond;
  Bond* (Molecule::*bond_ptr3)(const Atom*, const Atom*) = &Molecule::bond;
  Residue* (Molecule::*residue_ptr)(int) = &Molecule::residue;
  bool (Molecule::*addConformer_ptr1)(const std::vector<Eigen::Vector3d>&, unsigned int) = &Molecule::addConformer;
  std::vector<Eigen::Vector3d>* (Molecule::*addConformer_ptr2)(unsigned int) = &Molecule::addConformer;
  void (Molecule::*setEnergy_ptr1)(double) = &Molecule::setEnergy;
  void (Molecule::*setEnergy_ptr2)(int, double) = &Molecule::setEnergy;

  class_<Avogadro::Molecule, bases<Avogadro::Primitive>, boost::noncopyable,
      std::auto_ptr<Avogadro::Molecule> >("Molecule", no_init)
    // overloaded functions
    .def("copy", &copy)
    .add_property("OBMol", &Molecule_OBMol, &Molecule_setOBMol)

    //
    // read/write properties
    //
    .add_property("fileName",
        &Molecule::fileName,
        &Molecule::setFileName,
        "The full path filename of the molecule.")

    .add_property("dipoleMoment",
        make_function(&Molecule::dipoleMoment, return_value_policy<return_by_value>()),
        &Molecule::setDipoleMoment,
        "The dipole moment of the Molecule.")

    .add_property("energies",
        make_function(&Molecule::energies, return_value_policy<return_by_value>()),
        &Molecule::setEnergies,
        "The energies for all conformers.")

    //
    // read-only poperties
    //
    .add_property("numAtoms",
        &Molecule::numAtoms,
        "The total number of Atom objects in the molecule.")
    .add_property("numBonds",
        &Molecule::numBonds,
        "The total number of Bond objects in the Mmolecule.")
    .add_property("numResidues",
        &Molecule::numResidues,
        "The total number of Residue objects in the Molecule.")
    .add_property("numRings",
        &Molecule::numRings,
        "The total number of ring (Fragment) objects in the Molecule.")
    .add_property("numCubes",
        &Molecule::numCubes,
        "The total number of Cube objects in the Molecule.")
    .add_property("numMeshes",
        &Molecule::numMeshes,
        "The total number of Mesh objects in the Molecule.")
    .add_property("numZMatrices",
        &Molecule::numZMatrices,
        "The total number of ZMatrix objects in the Molecule.")
    .add_property("numConformers",
        &Molecule::numZMatrices,
        "The number of conformers.")

    .add_property("atoms",
        &Molecule::atoms,
        "List of all Atom objects in the Molecule.")
    .add_property("bonds",
        &Molecule::bonds,
        "List of all Bond objects in the Molecule.")
    .add_property("cubes",
        &Molecule::cubes,
        "List of all Cube objects in the Molecule.")
    .add_property("meshes",
        &Molecule::meshes,
        "List of all Mesh objects in the Molecule.")
    .add_property("residues",
        &Molecule::residues,
        "List of all Residue objects in the Molecule.")
    .add_property("rings",
        &Molecule::rings,
        "List of all ring (Fragment) objects in the Molecule.")
    .add_property("zMatrices",
        &Molecule::zMatrices,
        "List of all ZMatrix objects in the Molecule.")

    .add_property("center",
        make_function(&Molecule::center, return_value_policy<return_by_value>()),
        "The position of the center of the Molecule.")

    .add_property("normalVector",
        make_function(&Molecule::normalVector, return_value_policy<return_by_value>()),
        "The normal vector of the Molecule.")

    .add_property("radius",
        &Molecule::radius,
        "The radius of the Molecule.")

    .add_property("farthestAtom",
        make_function(&Molecule::farthestAtom, return_value_policy<reference_existing_object>()),
        "The Atom furthest away from the center of the Molecule.")

    //
    // real functions
    //
    .def("update",
        &Molecule::update,
        "Call to trigger an update signal, causing the molecule to be redrawn.")

    // use Atom::pos
    //.def("setAtomPos", setAtomPos_ptr1, "Set the Atom position.")
    //.def("atomPos", &Molecule::atomPos, return_value_policy<return_by_value>(), "Set the Atom position.")

    // atom functions
    .def("addAtom",
        addAtom_ptr1, return_value_policy<reference_existing_object>(),
        "Create a new Atom object and return a pointer to it.")
    .def("addAtom",
        addAtom_ptr2, return_value_policy<reference_existing_object>(),
        "Create a new Atom object with the specified id and return a pointer to "
        "it. Used when you need to recreate an Atom with the same unique id.")
    .def("atom",
        atom_ptr, return_value_policy<reference_existing_object>(),
        "The Atom at the supplied index.")
    .def("atomById",
        &Molecule::atomById, return_value_policy<reference_existing_object>(),
        "The Atom at the supplied unqique id.")
    .def("removeAtom",
        removeAtom_ptr1,
        "Remove the supplied Atom.")
    .def("removeAtom",
        removeAtom_ptr2,
        "Delete the Atom with the unique id specified.")
    // bond functions
    .def("addBond",
        addBond_ptr1, return_value_policy<reference_existing_object>(),
        "Create a new Bond object and return a pointer to it.")
    .def("addBond",
        addBond_ptr2, return_value_policy<reference_existing_object>(),
        "Create a new Bond object with the specified id and return a pointer to "
        "it. Used when you need to recreate a Bond with the same unique id.")
    .def("bond",
        bond_ptr1, return_value_policy<reference_existing_object>(),
        "Get the Bond at the supplied index.")
    .def("bond",
        bond_ptr2, return_value_policy<reference_existing_object>(),
        "Get the bond between the two supplied atom ids if one exists.")
    .def("bond",
        bond_ptr3, return_value_policy<reference_existing_object>(),
        "Get the bond between the two supplied atoms if one exists.")
    .def("bondById",
        &Molecule::bondById, return_value_policy<reference_existing_object>(),
        "The Bond at the supplied unique id.")
    .def("removeBond",
        removeBond_ptr1,
        "Remove the supplied Bond.")
    .def("removeBond",
        removeBond_ptr2,
        "Remove the Bond with the unique id specified.")
    // cube functions
    .def("addCube",
        addCube_ptr1, return_value_policy<reference_existing_object>(),
        "Create a new Cube object and return a pointer to it.")
    .def("addCube",
        addCube_ptr2, return_value_policy<reference_existing_object>(),
        "Create a new Cube object with the specified id and return a pointer to "
        "it. Used when you need to recreate a Cube with the same unique id.")
    .def("cube",
        cube_ptr, return_value_policy<reference_existing_object>(),
        "Get the Cube at the supplied index.")
    .def("cubeById",
        &Molecule::cubeById, return_value_policy<reference_existing_object>(),
        "Get the Cube at the supplied unique id.")
    .def("removeCube",
        removeCube_ptr1,
        "Remove the supplied Cube.")
    .def("removeCube",
        removeCube_ptr2,
        "Remove the Cube with the unique id specified.")
    // mesh functions
    .def("addMesh",
        addMesh_ptr1, return_value_policy<reference_existing_object>(),
        "Create a new Mesh object and return a pointer to it.")
    .def("addMesh",
        addMesh_ptr2, return_value_policy<reference_existing_object>(),
        "Create a new Mesh object with the specified id and return a pointer to "
        "it. Used when you need to recreate a Mesh with the same unique id.")
    .def("mesh",
        mesh_ptr, return_value_policy<reference_existing_object>(),
        "Get the Mesh at the supplied index.")
    .def("meshById",
        &Molecule::meshById, return_value_policy<reference_existing_object>(),
        "Get the Mesh at the supplied unique id.")
    .def("removeMesh",
        removeMesh_ptr1,
        "Remove the supplied Mesh.")
    .def("removeMesh",
        removeMesh_ptr2,
        "Remove the Mesh with the unique id specified.")
    // residue functions
    .def("addResidue",
        addResidue_ptr1, return_value_policy<reference_existing_object>(),
        "Create a new Residue object and return a pointer to it.")
    .def("addResidue",
        addResidue_ptr2, return_value_policy<reference_existing_object>(),
        "Create a new Residue object with the specified id and return a pointer to "
        "it. Used when you need to recreate a Residue with the same unique id.")
    .def("residue",
        residue_ptr, return_value_policy<reference_existing_object>(),
        "Get the residue at the supplied index.")
    .def("residueById",
        &Molecule::residueById, return_value_policy<reference_existing_object>(),
        "Get the residue at the supplied unique id.")
    .def("removeResidue",
        removeResidue_ptr1,
        "Remove the supplied residue.")
    .def("removeResidue",
        removeResidue_ptr2,
        "Remove the residue with the unique id specified.")
    // ring functions
    .def("addRing",
        addRing_ptr1, return_value_policy<reference_existing_object>(),
        "Create a new ring object and return a pointer to it.")
    .def("addRing",
        addRing_ptr2, return_value_policy<reference_existing_object>(),
        "Create a new Ring object with the specified id and return a pointer to "
        "it. Used when you need to recreate a Ring with the same unique id.")
    .def("removeRing",
        removeRing_ptr1,
        "Remove the supplied ring.")
    .def("removeRing",
        removeRing_ptr2,
        "Remove the ring with the unique id specified.")
    // zmatrix functions
    .def("addZMatrix",
        &Molecule::addZMatrix, return_value_policy<reference_existing_object>(),
        "Create a new ZMatrix object and return a pointer to it.")
    .def("removeZMatrix",
        &Molecule::removeZMatrix,
        "Remove the supplied ZMatrix.")
    .def("zMatrix",
        &Molecule::zMatrix, return_value_policy<reference_existing_object>(),
        "Get the ZMatrix at the supplied index.")
    // conformer functions
    .def("addConformer",
        addConformer_ptr1,
        "Add a new conformer to the Molecule. The conformers are mapped onto the "
        "unique ids of the atoms in the Molecule.")
    .def("addConformer",
        addConformer_ptr2, return_value_policy<return_by_value>(),
        "Add a new conformer and return a pointer to it.")
    .def("conformer",
        &Molecule::conformer, return_value_policy<return_by_value>(),
        "Get the conformer for the supplied index, or None if the index doesn't exist.")
    .def("conformers",
        &Molecule::conformer, return_value_policy<return_by_value>(),
        "Get const reference to all conformers.") // FIXME
    .def("setConformer",
        &Molecule::setConformer,
        "Change the conformer to the one at the specified index.")
    .def("setAllConformers",
        &Molecule::setAllConformers,
        "Replace all conformers in the Molecule. The conformers are "
        "mapped onto the unique ids of the atoms in the Molecule. "
        "This will first clear all conformers.")
    .def("currentConformer",
        &Molecule::currentConformer,
        "The current conformer index.")
    .def("clearConformers",
        &Molecule::clearConformers,
        "Clear all conformers from the molecule, leaving just conformer zero.")
    .def("energy",
        energy,
        "Get the energy of the current conformer.")
    .def("energy",
        &Molecule::energy,
        "Get the energy of the supplied conformer index.")
    .def("setEnergy",
        setEnergy_ptr1,
        "Set the energy for the current conformer.")
    .def("setEnergy",
        setEnergy_ptr2,
        "Set the energy for the specified conformer.")
    // general functions
    .def("addHydrogens",
        &addHydrogens1,
        "Add hydrogens to the molecule.")
    .def("addHydrogens",
        &addHydrogens2,
        "Add hydrogens to the molecule.")

    .def("removeHydrogens",
        &Molecule::removeHydrogens,
        "Remove all hydrogens connected to the supplied atom.")
    .def("removeHydrogens",
        removeHydrogens,
        "Remove all hydrogens from the molecule.")


    .def("calculatePartialCharges",
        &Molecule::calculatePartialCharges,
        "Calculate the partial charges on each atom.")

    .def("calculateAromaticity",
        &Molecule::calculateAromaticity,
        "Calculate the aromaticity of the bonds.")

    .def("clear",
        &Molecule::clear,
        "Remove all elements of the molecule.")

    .def("translate",
        &Molecule::translate,
        "Translate the Molecule using the supplied vector.")
    ;

}

