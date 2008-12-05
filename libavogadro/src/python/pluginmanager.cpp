#include <boost/python.hpp>

#include <avogadro/plugin.h>
#include <avogadro/pluginmanager.h>

using namespace boost::python;
using namespace Avogadro;

void export_PluginManager()
{
  class_<Avogadro::PluginManager, boost::noncopyable>("PluginManager", no_init)
    // static read-only properties
    .add_static_property("instance", make_function(&PluginManager::instance, 
          return_value_policy<reference_existing_object>()))
   
    // real functions
    .def("loadFactories", &PluginManager::loadFactories)
    .staticmethod("loadFactories")

    .def("factories", &PluginManager::factories)
    .staticmethod("factories")
 
    .def("factory", &PluginManager::factory, return_value_policy<reference_existing_object>())
    .staticmethod("factory")
 
    .def("extensions", &PluginManager::extensions)
    .def("tools", &PluginManager::tools)
    .def("colors", &PluginManager::colors)
    ;


   
}
