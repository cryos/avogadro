#include <boost/python.hpp>

#include <avogadro/plugin.h>
#include <avogadro/pluginmanager.h>

#include <avogadro/engine.h>
#include <avogadro/tool.h>
#include <avogadro/extension.h>
#include <avogadro/color.h>

using namespace boost::python;
using namespace Avogadro;

// thin wrappers to handle default arguments
void loadFactories1() { PluginManager::loadFactories(); }
void loadFactories2(const QString &dir) { PluginManager::loadFactories(dir); }

void export_PluginManager()
{
  class_<Avogadro::PluginManager, boost::noncopyable>("PluginManager", no_init)
    // static read-only properties
    .add_static_property("instance", make_function(&PluginManager::instance,
          return_value_policy<reference_existing_object>()))

    // real functions
    .def("loadFactories", &loadFactories1)
    .def("loadFactories", &loadFactories2)
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
    .def("identifiers", &PluginManager::identifiers)
    .def("descriptions", &PluginManager::descriptions)

    .def("extensions", &PluginManager::extensions)
    .def("tools", &PluginManager::tools)
    .def("colors", &PluginManager::colors)
    ;

}
