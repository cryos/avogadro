#include <boost/python.hpp>

#include <avogadro/engine.h>
#include <avogadro/color.h>
#include <avogadro/primitive.h>

using namespace boost::python;
using namespace Avogadro;

void export_Engine()
{
  
  enum_<Engine::EngineFlags>("EngineFlags")
    // the Type enum
    .value("NoFlags", Engine::NoFlags)
    .value("Transparent", Engine::Transparent)
    .value("Overlay", Engine::Overlay)
    .value("Bonds", Engine::Bonds)
    .value("Atoms", Engine::Atoms)
    .value("Molecules", Engine::Molecules)
    ;
 
  class_<Avogadro::Engine, bases<Avogadro::Plugin>, boost::noncopyable>("Engine", no_init)
    // read-only poperties 
    .add_property("typeName", &Engine::typeName)
    .add_property("flags", &Engine::flags)
    // read/write properties
    .add_property("alias", &Engine::alias, &Engine::setAlias)
    .add_property("shader", &Engine::shader, &Engine::setShader)
    .add_property("primitives", &Engine::primitives, &Engine::setPrimitives)
    .add_property("enabled", &Engine::isEnabled, &Engine::setEnabled)
    .add_property("colorMap", make_function(&Engine::colorMap, return_value_policy<reference_existing_object>()),
        &Engine::setColorMap)

    // real functions
    .def("clearPrimitives", &Engine::clearPrimitives)
    .def("addPrimitive", &Engine::addPrimitive)
    .def("updatePrimitive", &Engine::updatePrimitive)
    .def("removePrimitive", &Engine::removePrimitive)
    .def("clone", &Engine::clone, return_value_policy<manage_new_object>())
    ;
   
}
