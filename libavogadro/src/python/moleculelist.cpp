#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/cube.h>
#include <avogadro/glwidget.h>

#include <openbabel/mol.h>

#include "moleculelist.h"

namespace Avogadro {
    
  MoleculeList* MoleculeList::instance()
  {
    static MoleculeList *instance = 0;
    if (!instance)
      instance = new MoleculeList;
    return instance;
  }

  Molecule* MoleculeList::addMolecule()
  {
    Molecule *mol;
    mol = new Molecule();
    m_molecules.append(mol);
    connect(mol, SIGNAL(destroyed()), this, SLOT(moleculeDestroyed()));
    return mol;
  }

  void MoleculeList::moleculeDestroyed()
  {
    Molecule* mol = static_cast<Molecule*>(sender());
    int index = m_molecules.indexOf(mol);
    m_molecules.removeAt(index);  
  }

}

using namespace boost::python;
using namespace Avogadro;

Molecule *currentMolecule()
{
  if (!GLWidget::current())
    return 0;
  return GLWidget::current()->molecule();
}

void export_MoleculeList()
{

  class_<MoleculeList, boost::noncopyable>("MoleculeList", no_init)
    .add_property("instance", make_function(&MoleculeList::instance, return_value_policy<reference_existing_object>()))
    .add_property("numMolecules", &MoleculeList::numMolecules)
    .def("addMolecule", &MoleculeList::addMolecule, return_value_policy<reference_existing_object>())
    .def("at", &MoleculeList::at, return_value_policy<reference_existing_object>())
    ;

  // module's DATA: Avogadro.molecules
  MoleculeList *moleculeList = MoleculeList::instance();
  reference_existing_object::apply<MoleculeList*>::type converter;
  PyObject* pyobj = converter( moleculeList );
  object real_obj = object( handle<>( pyobj ) );
  scope().attr("molecules") = real_obj;

}

#include "moleculelist.moc"
