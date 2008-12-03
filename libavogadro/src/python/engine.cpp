#include <boost/python.hpp>

#include <avogadro/engine.h>
#include <avogadro/color.h>
#include <avogadro/primitive.h>

using namespace boost::python;
using namespace Avogadro;

void export_Engine()
{
  
  class_<Avogadro::Engine, bases<Avogadro::Plugin>, boost::noncopyable>("Engine", no_init)
    // read-only poperties 
    .add_property("typeName", &Engine::typeName)
    // read/write properties
    .add_property("alias", &Engine::alias, &Engine::setAlias)
    .add_property("primitives", &Engine::primitives, &Engine::setPrimitives)
    .add_property("enabled", &Engine::isEnabled, &Engine::setEnabled)
    .add_property("colorMap", make_function(&Engine::colorMap, return_value_policy<reference_existing_object>()),
        &Engine::setColorMap)
    // settingsWidget

    // real functions
    .def("clearPrimitives", &Engine::clearPrimitives)
    .def("clone", &Engine::clone, return_value_policy<reference_existing_object>())
    ;
   
}
