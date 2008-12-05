#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/primitivelist.h>

using namespace boost::python;
using namespace Avogadro;

void export_PrimitiveList()
{
  int (PrimitiveList::*count_ptr)( Primitive::Type ) const = &PrimitiveList::count;

  class_<Avogadro::PrimitiveList, boost::noncopyable>("PrimitiveList")
    // constructors
    .def(init<const PrimitiveList&>())
    .def(init<const QList<Primitive*>&>())

    // read-only properties
    .add_property("list", make_function(&PrimitiveList::list, return_value_policy<return_by_value>()))
    .add_property("size", &PrimitiveList::size)
    .add_property("isEmpty", &PrimitiveList::isEmpty) 
    // real functions
    .def("subList", &PrimitiveList::subList, return_value_policy<return_by_value>())
    .def("contains", &PrimitiveList::contains)
    .def("append", &PrimitiveList::append)
    .def("removeAll", &PrimitiveList::removeAll)
    .def("count", count_ptr)
    .def("clear", &PrimitiveList::clear)
    ;
 
}
