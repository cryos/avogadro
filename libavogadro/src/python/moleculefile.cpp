// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/moleculefile.h>
#include <avogadro/molecule.h>

#include <QStringList>

using namespace boost::python;
using namespace Avogadro;

// handle default arguments
MoleculeFile* readFile1(const QString &fileName) 
{ return MoleculeFile::readFile(fileName); }
MoleculeFile* readFile2(const QString &fileName, const QString &fileType) 
{ return MoleculeFile::readFile(fileName, fileType); }
MoleculeFile* readFile3(const QString &fileName, const QString &fileType, const QString &fileOptions) 
{ return MoleculeFile::readFile(fileName, fileType, fileOptions); }

// readMolecule
Molecule* readMolecule1(const QString &fileName) 
{ return MoleculeFile::readMolecule(fileName); }
Molecule* readMolecule2(const QString &fileName, const QString &fileType) 
{ return MoleculeFile::readMolecule(fileName, fileType); }
Molecule* readMolecule3(const QString &fileName, const QString &fileType, const QString &fileOptions) 
{ return MoleculeFile::readMolecule(fileName, fileType, fileOptions); }
Molecule* readMolecule4(const QString &fileName, const QString &fileType, const QString &fileOptions, QString *error)
{ return MoleculeFile::readMolecule(fileName, fileType, fileOptions, error); }

// writeMolecule
bool writeMolecule1(Molecule *molecule, const QString &fileName) 
{ return MoleculeFile::writeMolecule(molecule, fileName); }
bool writeMolecule2(Molecule *molecule, const QString &fileName, const QString &fileType) 
{ return MoleculeFile::writeMolecule(molecule, fileName, fileType); }
bool writeMolecule3(Molecule *molecule, const QString &fileName, const QString &fileType, QString *error) 
{ return MoleculeFile::writeMolecule(molecule, fileName, fileType, error); }

// WriteConformers
bool writeConformers1(Molecule *molecule, const QString &fileName) 
{ return MoleculeFile::writeConformers(molecule, fileName); }
bool writeConformers2(Molecule *molecule, const QString &fileName, const QString &fileType) 
{ return MoleculeFile::writeConformers(molecule, fileName, fileType); }
bool writeConformers3(Molecule *molecule, const QString &fileName, const QString &fileType, QString *error) 
{ return MoleculeFile::writeConformers(molecule, fileName, fileType, error); }



void export_FileIO()
{

  class_<Avogadro::MoleculeFile, boost::noncopyable>("MoleculeFile", no_init)
 
    .add_property("isReady", 
        &MoleculeFile::isReady,
        "True if the file contains multiple frames/conformers of the same molecule.")
   
    .add_property("isConformerFile", 
        &MoleculeFile::isConformerFile,
        "True if the file contains multiple frames/conformers of the same molecule.")

    .add_property("numMolecules", 
        &MoleculeFile::numMolecules,
        "The number of molecules in the file.")

    .add_property("titles", 
        &MoleculeFile::titles,
        "Te titles for the molecules.")

    /*
    .add_property("conformers", 
        &MoleculeFile::conformers,
        "Get all the conformers from the file. This methods returns an empty"
        "vector if the opened file isn't a conformer file (see isConformerFile()).")
        */

    .add_property("errors",
        make_function(&MoleculeFile::errors, return_value_policy<return_by_value>()),
        "Errors from reading/writing to the file.")

    .add_property("fileName",
        make_function(&MoleculeFile::fileName, return_value_policy<return_by_value>()),
        "The filename for this file.")
     .add_property("fileType",
        make_function(&MoleculeFile::fileType, return_value_policy<return_by_value>()),
        "The file type of this file. (e.g., extension or Open Babel code)")
     .add_property("fileOptions",
        make_function(&MoleculeFile::fileOptions, return_value_policy<return_by_value>()),
        "Any options set for reading this file.")
    
     //
    // real functions
    //
    .def("molecule", 
        &MoleculeFile::molecule, return_value_policy<manage_new_object>(),
        "The ith molecule or 0 when i > numMolecule(). In FileIO::Output "
        "mode, this method always returns 0.")

     .def("replaceMolecule", 
        &MoleculeFile::replaceMolecule,
        "Replace the i-th molecule with the supplied molecule. When a molecule "
        "returned by molecule() has changed, this function can be used to write "
        " it back to the file at the same position.")

     .def("insertMolecule", 
        &MoleculeFile::insertMolecule,
        "Insert a molecule at index i.")

     .def("appendMolecule", 
        &MoleculeFile::appendMolecule,
        "Append @p molecule to the end of the file.")
 
     .def("clearErrors", 
        &MoleculeFile::appendMolecule,
        "Clear the errors. Errors are always appended to error(), so unless you "
        "clear them explicitly, consecutive calls to errors() returns all errors "
        "from before plus the new ones (if any).")
 
    //
    // static methods
    //    
    .def("readMolecule", 
        &readMolecule1, return_value_policy<manage_new_object>(),
        "Static function to load a file and return a Molecule pointer. You are "
        "responsible for deleting the molecule object.")
    .def("readMolecule", 
        &readMolecule2, return_value_policy<manage_new_object>(),
        "Static function to load a file and return a Molecule pointer. You are "
        "responsible for deleting the molecule object.")
    .def("readMolecule", 
        &readMolecule3, return_value_policy<manage_new_object>(),
        "Static function to load a file and return a Molecule pointer. You are "
        "responsible for deleting the molecule object.")
    .def("readMolecule", 
        &readMolecule4, return_value_policy<manage_new_object>(),
        "Static function to load a file and return a Molecule pointer. You are "
        "responsible for deleting the molecule object.")
    .staticmethod("readMolecule")

    .def("writeMolecule", 
        &writeMolecule1,
        "Static function to save a single molecule to a file. If writing was "
        "unsuccessful, a previously existing file will not be overwritten.")
    .def("writeMolecule", 
        &writeMolecule2,
        "Static function to save a single molecule to a file. If writing was "
        "unsuccessful, a previously existing file will not be overwritten.")
    .def("writeMolecule", 
        &writeMolecule3,
        "Static function to save a single molecule to a file. If writing was "
        "unsuccessful, a previously existing file will not be overwritten.")
    .staticmethod("writeMolecule")
    
    .def("writeConformers", 
        &writeConformers1,
        "Static function to save a all conformers in a molecule to a file. If "
        "writing was unsuccessful, a previously existing file will not be "
        "overwritten. All formats with support for multiple molecules can be "
        "used.")
    .def("writeConformers", 
        &writeConformers2,
        "Static function to save a all conformers in a molecule to a file. If "
        "writing was unsuccessful, a previously existing file will not be "
        "overwritten. All formats with support for multiple molecules can be "
        "used.")
    .def("writeConformers", 
        &writeConformers3,
        "Static function to save a all conformers in a molecule to a file. If "
        "writing was unsuccessful, a previously existing file will not be "
        "overwritten. All formats with support for multiple molecules can be "
        "used.")
    .staticmethod("writeConformers")
    
    .def("readFile", 
        &MoleculeFile::readFile, return_value_policy<manage_new_object>(),
        "Read an entire file, possibly containing multiple molecules in a "
        "separate thread and return a MoleculeFile object with the result. "
        "By default, the @p wait parameter is set to true and the function "
        "waits for the thread to finish before returning. If set to "
        "false, listening to the MoleculeFile::ready() signal will be "
        "emitted when the results are ready.")
    .def("readFile", 
        &readFile1, return_value_policy<manage_new_object>(),
        "Read an entire file, possibly containing multiple molecules in a "
        "separate thread and return a MoleculeFile object with the result. "
        "By default, the @p wait parameter is set to true and the function "
        "waits for the thread to finish before returning. If set to "
        "false, listening to the MoleculeFile::ready() signal will be "
        "emitted when the results are ready.")   
    .def("readFile", 
        &readFile2, return_value_policy<manage_new_object>(),
        "Read an entire file, possibly containing multiple molecules in a "
        "separate thread and return a MoleculeFile object with the result. "
        "By default, the @p wait parameter is set to true and the function "
        "waits for the thread to finish before returning. If set to "
        "false, listening to the MoleculeFile::ready() signal will be "
        "emitted when the results are ready.")  
    .def("readFile", 
        &readFile3, return_value_policy<manage_new_object>(),
        "Read an entire file, possibly containing multiple molecules in a "
        "separate thread and return a MoleculeFile object with the result. "
        "By default, the @p wait parameter is set to true and the function "
        "waits for the thread to finish before returning. If set to "
        "false, listening to the MoleculeFile::ready() signal will be "
        "emitted when the results are ready.")
    .staticmethod("readFile")
    ;




}
