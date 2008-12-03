#include <boost/python.hpp>

#include <avogadro/mesh.h>
#include <avogadro/primitive.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Mesh()
{
  
  class_<Avogadro::Mesh, bases<Avogadro::Primitive>, boost::noncopyable>("Mesh", no_init)
    // read-only poperties 
    .add_property("valid", &Mesh::valid)

    // read/write properties
    .add_property("name", &Mesh::name, &Mesh::setName)
    .add_property("vertices", make_function(&Mesh::vertices, return_value_policy<reference_existing_object>()), 
        &Mesh::setVertices)
    .add_property("normals", make_function(&Mesh::normals, return_value_policy<reference_existing_object>()), 
        &Mesh::setNormals)
    //.add_property("colors", &Mesh::colors, &Mesh::setColors) // QColor
 
    // real functions
    .def("clear", &Mesh::clear)
    //.def("vertex", &Mesh::vertex, return_value_policy<return_by_value>())
    .def("addVertices", &Mesh::addVertices)
    //.def("normal", &Mesh::normal, return_value_policy<return_by_value>())
    .def("addNormals", &Mesh::addNormals)
    //.def("color", &Mesh::color) // QColor
    //.def("addColors", &Mesh::addColors) // QColor
    ;
   
}
