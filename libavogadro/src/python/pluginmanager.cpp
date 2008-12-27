#include <boost/python.hpp>

#include <avogadro/plugin.h>
#include <avogadro/pluginmanager.h>

#include <avogadro/engine.h>
#include <avogadro/tool.h>
#include <avogadro/extension.h>
#include <avogadro/color.h>

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

    //.def("factories", &PluginManager::factories)
    //.staticmethod("factories")
    //.def("factory", &PluginManager::factory, return_value_policy<reference_existing_object>())
    //.staticmethod("factory")

    .def("extension", &PluginManager::extension, return_value_policy<manage_new_object>())
    .def("tool", &PluginManager::tool, return_value_policy<manage_new_object>())
    .def("color", &PluginManager::color, return_value_policy<manage_new_object>())
    .def("engine", &PluginManager::engine, return_value_policy<manage_new_object>())
    .def("names", &PluginManager::names)
    .def("descriptions", &PluginManager::descriptions)

    .def("extensions", &PluginManager::extensions)
    .def("tools", &PluginManager::tools)
    .def("colors", &PluginManager::colors)
    ;

}
