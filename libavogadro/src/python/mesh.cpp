// Last update: timvdm 18 June 2009
#include <boost/python.hpp>

#include <avogadro/mesh.h>
#include <avogadro/color3f.h>
#include <avogadro/primitive.h>
#include <avogadro/molecule.h>

#include <QColor>

using namespace boost::python;
using namespace Avogadro;

bool reserve(Mesh &self, unsigned int size)
{
  return self.reserve(size);
}

void export_Mesh()
{
  
  class_<Avogadro::Mesh, bases<Avogadro::Primitive>, boost::noncopyable>("Mesh", no_init)
    
    //
    // read-only poperties 
    //
    .add_property("valid", 
        &Mesh::valid, 
        "Sanity checking function - is the mesh sane?")

    //
    // read/write properties
    //
    .add_property("name", 
        &Mesh::name, 
        &Mesh::setName, 
        "The name of the Mesh.")

    .add_property("stable", 
        &Mesh::stable, 
        &Mesh::setStable, 
        "Indicate whether the Mesh is complete or currently being modified. In "
        "general using Mesh values from an unstable Mesh is not advisable.")

    .add_property("isoValue", 
        &Mesh::isoValue, 
        &Mesh::setIsoValue, 
        "The iso value used to generate the Mesh.")

    .add_property("otherMesh", 
        &Mesh::otherMesh, 
        &Mesh::setOtherMesh, 
        "The unique id of the other Mesh if this is part of a pair.")

    .add_property("cube", 
        &Mesh::cube, 
        &Mesh::setCube, 
        "The unique id of the Cube the Mesh was generated from.")

    .add_property("vertices", 
        make_function(&Mesh::vertices, return_value_policy<return_by_value>()), 
        &Mesh::setVertices, 
        "List containing all of the vertices in a one dimensional array.")

    .add_property("numVertices", 
        &Mesh::numVertices, 
        "The number of vertices.")

    .add_property("numNormals", 
        &Mesh::numNormals, 
        "The number of normals.")

    .add_property("normals", 
        make_function(&Mesh::normals, return_value_policy<return_by_value>()), 
        &Mesh::setNormals, 
        "List containing all of the normals in a one-dimensional array.")

    .add_property("colors", 
        make_function(&Mesh::colors, return_value_policy<return_by_value>()),
        &Mesh::setColors)
 
    // real functions
    .def("reserve", 
        &Mesh::reserve, 
        "Reserve the expected space for the mesh. This causes all member vector "
        "storage to call the reserve function with the number specified.")
    .def("reserve", 
        reserve, 
        "Reserve the expected space for the mesh. This causes all member vector "
        "storage to call the reserve function with the number specified.")

    .def("vertex", &Mesh::vertex, return_value_policy<return_by_value>()) // FIXME
    .def("normal", &Mesh::normal, return_value_policy<return_by_value>()) // FIXME
    .def("color", &Mesh::color, return_value_policy<return_by_value>()) // FIXME
    
    .def("clear", 
        &Mesh::clear, 
        "Clear all mesh data.")
    
    .def("addVertices", 
        &Mesh::addVertices, 
        "Add one or more vertices, i.e., the vector is expected to be of length "
        "3 x n where n is an integer.")

    .def("addNormals", 
        &Mesh::addNormals,
        "Add one or more normals, i.e., the vector is expected to be of length "
        "3 x n where n is an integer.")

    .def("addColors", 
        &Mesh::addColors,
        "Add one or more colors, i.e., the vector is expected to be of length "
        "3 x n where n is an integer.")
    ;
   
}
