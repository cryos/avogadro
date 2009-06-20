// Last update: timvdm 19 June 2009
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/primitivelist.h>

using namespace boost::python;
using namespace Avogadro;

void export_PrimitiveList()
{
  int (PrimitiveList::*count_ptr)( Primitive::Type ) const = &PrimitiveList::count;

  //class_<Avogadro::PrimitiveList, boost::noncopyable>("PrimitiveList")
  class_<Avogadro::PrimitiveList>("PrimitiveList")
    // constructors
    .def(init<const PrimitiveList&>())
    .def(init<const QList<Primitive*>&>())

    //
    // read-only properties
    //
    .add_property("list", 
        make_function(&PrimitiveList::list, return_value_policy<return_by_value>()),
        "A list of all primitives of all types.")

    .add_property("size", 
        &PrimitiveList::size,
        "The total number of primitives in this queue.")

    .add_property("isEmpty", 
        &PrimitiveList::isEmpty,
        "True if the list is empty.") 

    //
    // real functions
    //
    .def("subList", 
        &PrimitiveList::subList, return_value_policy<return_by_value>(),
        "Returns a list of primitives for a given type.")

    .def("contains", 
        &PrimitiveList::contains,
        "Returns true or false depending on whether p is in this list.")

    .def("append", 
        &PrimitiveList::append,
        "Add a primitive to the queue.")

    .def("removeAll", 
        &PrimitiveList::removeAll,
        "Remove a primitive from the queue.  If the parameter does not "
        "exist in the queue, nothing is removed.")

    .def("count", 
        count_ptr,
        "Returns the number of primitives for the given type")

    .def("clear", 
        &PrimitiveList::clear,
        "Removes every primitive from the queue.")
    ;
 
}
