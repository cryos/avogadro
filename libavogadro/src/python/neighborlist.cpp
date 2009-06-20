// Last update: timvdm 19 June 2009
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
    // constructors
    .def(init<Molecule*,double>())
    .def(init<Molecule*,double,bool>())
    .def(init<Molecule*,double,bool,int>())
    .def(init<const QList<Atom*>&,double>())
    .def(init<const QList<Atom*>&,double,bool>())
    .def(init<const QList<Atom*>&,double,bool,int>())
    
    //
    // real functions
    //
    .def("update", 
        &NeighborList::update,
        "Update the cells. While minimizing or running MD simulations, "
        "atoms move and can go from on cell into the next. This function "
        "should be called every 10-20 iterations to make sure the cells "
        "stay accurate.")

    .def("nbrs", 
        &nbrs_default,
        "Get the near-neighbor atoms for @p atom. The squared distance is "
        "checked and is cached for later use (see r2() function). "
        "Atoms in relative 1-2 and 1-3 positions are not returned. "
        "The @p atom itself isn't added to the list.")
    .def("nbrs", 
        nbrs_ptr1,
        "Get the near-neighbor atoms for @p atom. The squared distance is "
        "checked and is cached for later use (see r2() function). "
        "Atoms in relative 1-2 and 1-3 positions are not returned. "
        "The @p atom itself isn't added to the list.")
    .def("nbrs", 
        nbrs_ptr2,
        "Get the near-neighbor atoms around @p pos. The squared distance is "
        "checked and is cached for later use (see r2() function).")

    .def("r2", 
        &NeighborList::r2,
        "Get the cached squared distance from the atom last used to call "
        "nbrs to the atom with @p index in the returned vector.")
    ;

}
