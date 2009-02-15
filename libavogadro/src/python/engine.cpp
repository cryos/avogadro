#include <boost/python.hpp>

#include <Eigen/Core>

#include <avogadro/engine.h>
#include <avogadro/color.h>
#include <avogadro/primitive.h>

using namespace boost::python;
using namespace Avogadro;

void export_Engine()
{
  
  enum_<Engine::Layers>("Layers")
    // the Type enum
    .value("Opaque", Engine::Opaque)
    .value("Transparent", Engine::Transparent)
    .value("Overlay", Engine::Overlay)
    ;
    
  enum_<Engine::PrimitiveTypes>("PrimitiveTypes")
    .value("NoPrimitives", Engine::NoPrimitives)
    .value("Atoms", Engine::Atoms)
    .value("Bonds", Engine::Bonds)
    .value("Molecules", Engine::Molecules)
    .value("Surfaces", Engine::Surfaces)
    .value("Fragments", Engine::Fragments)
    ;
 
  enum_<Engine::ColorTypes>("ColorTypes")
    .value("NoColors", Engine::NoColors)
    .value("ColorPlugins", Engine::ColorPlugins)
    .value("IndexedColors", Engine::IndexedColors)
    .value("ColorGradients", Engine::ColorGradients)
    ;
    
  class_<Avogadro::Engine, bases<Avogadro::Plugin>, boost::noncopyable>("Engine", no_init)
    // read-only poperties 
    .add_property("typeName", &Engine::typeName)
    .add_property("layers", &Engine::layers)
    .add_property("primitiveTypes", &Engine::primitiveTypes)
    .add_property("colorTypes", &Engine::colorTypes)
    // read/write properties
    .add_property("alias", &Engine::alias, &Engine::setAlias)
    .add_property("shader", &Engine::shader, &Engine::setShader)
    .add_property("primitives", make_function(&Engine::primitives, return_value_policy<reference_existing_object>()), 
        &Engine::setPrimitives)
    .add_property("enabled", &Engine::isEnabled, &Engine::setEnabled)
    .add_property("colorMap", make_function(&Engine::colorMap, return_value_policy<reference_existing_object>()),
        &Engine::setColorMap)

    // real functions
    .def("clearPrimitives", &Engine::clearPrimitives)
    .def("addPrimitive", &Engine::addPrimitive)
    .def("updatePrimitive", &Engine::updatePrimitive)
    .def("removePrimitive", &Engine::removePrimitive)
    .def("clone", &Engine::clone, return_value_policy<manage_new_object>())
    .def("readSettings", &Engine::readSettings)
    .def("writeSettings", &Engine::writeSettings)
    ;
   
}
