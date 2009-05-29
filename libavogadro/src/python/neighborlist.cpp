// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/neighborlist.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

using namespace boost::python;
using namespace Avogadro;

// default arg
QList<Atom*> nbrs_default(NeighborList &self, Atom *atom)
{
  return self.nbrs(atom);
}

void export_NeighborList()
{ 
 
  QList<Atom*> (NeighborList::*nbrs_ptr1)(Atom*,bool) = &NeighborList::nbrs;
  QList<Atom*> (NeighborList::*nbrs_ptr2)(const Eigen::Vector3f*) = &NeighborList::nbrs;

  class_<Avogadro::NeighborList, boost::noncopyable>("NeighborList", no_init)
    .def(init<Molecule*,double>())
    .def(init<Molecule*,double,int>())
    // real functions
    .def("update", &NeighborList::update)
    .def("nbrs", &nbrs_default)
    .def("nbrs", nbrs_ptr1)
    .def("nbrs", nbrs_ptr2)
    .def("r2", &NeighborList::r2)
    ;

}
