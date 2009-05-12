// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/meshgenerator.h>
#include <avogadro/mesh.h>
#include <avogadro/cube.h>

using namespace boost::python;
using namespace Avogadro;

// handle default argument
bool initialize(MeshGenerator &self, const Cube *cube, Mesh *mesh, float iso)
{
  return self.initialize(cube, mesh, iso);
}

void export_MeshGenerator()
{
  
  class_<Avogadro::MeshGenerator, boost::noncopyable>("MeshGenerator")
    .def(init<QObject*>())
    .def(init<const Cube*, Mesh*, float>())
    .def(init<const Cube*, Mesh*, float, bool>())
    .def(init<const Cube*, Mesh*, float, QObject*>())
    // read-only poperties 
    .add_property("cube", make_function(&MeshGenerator::cube, return_value_policy<reference_existing_object>()))
    .add_property("mesh", make_function(&MeshGenerator::mesh, return_value_policy<reference_existing_object>()))
    // real functions
    .def("initialize", &MeshGenerator::initialize)
    .def("initialize", &initialize)
    .def("run", &MeshGenerator::run)
    .def("clear", &MeshGenerator::clear)
    ;
   
}
