// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/openbabelwrapper.h>
#include <avogadro/moleculefile.h>
#include <avogadro/molecule.h>

#include <QStringList>

using namespace boost::python;
using namespace Avogadro;

// handle default arguments
MoleculeFile* readFile1(const QString &fileName) 
{ return OpenbabelWrapper::readFile(fileName); }
MoleculeFile* readFile2(const QString &fileName, const QString &fileType) 
{ return OpenbabelWrapper::readFile(fileName, fileType); }
MoleculeFile* readFile3(const QString &fileName, const QString &fileType, const QString &fileOptions) 
{ return OpenbabelWrapper::readFile(fileName, fileType, fileOptions); }

void export_FileIO()
{
  QList<QString> (MoleculeFile::*titles_ptr)() const = &MoleculeFile::titles;

  class_<Avogadro::MoleculeFile, boost::noncopyable>("MoleculeFile", no_init)
    .add_property("isConformerFile", &MoleculeFile::isConformerFile)
    .add_property("numMolecules", &MoleculeFile::numMolecules)
    .add_property("titles", titles_ptr)
//    .add_property("conformers", &MoleculeFile::conformers)
    .def("molecule", &MoleculeFile::molecule, return_value_policy<manage_new_object>())
    ;
  
  class_<Avogadro::OpenbabelWrapper, boost::noncopyable>("OpenbabelWrapper", no_init)
    .def("openFile", &OpenbabelWrapper::openFile, return_value_policy<manage_new_object>())
    .staticmethod("openFile")
    .def("saveFile", &OpenbabelWrapper::saveFile)
    .staticmethod("saveFile")
    .def("writeConformers", &OpenbabelWrapper::writeConformers)
    .staticmethod("writeConformers")
    .def("readFile", &OpenbabelWrapper::readFile, return_value_policy<manage_new_object>())
    .def("readFile", &readFile1, return_value_policy<manage_new_object>())
    .def("readFile", &readFile2, return_value_policy<manage_new_object>())
    .def("readFile", &readFile3, return_value_policy<manage_new_object>())
    .staticmethod("readFile")
    ;













}
