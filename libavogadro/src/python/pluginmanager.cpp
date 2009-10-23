// Last update: timvdm 12 May 2009

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
void loadFactories1(PluginManager &self) { self.loadFactories(); }
void loadFactories2(PluginManager &self, const QString &dir) { self.loadFactories(dir); }

Tool* tool1(PluginManager &self, const QString &id) { return self.tool(id); }
Extension* extension1(PluginManager &self, const QString &id) { return self.extension(id); }
Engine* engine1(PluginManager &self, const QString &id) { return self.engine(id); }
Color* color1(PluginManager &self, const QString &id) { return self.color(id); }

QList<Tool*> tools1(PluginManager &self) { return self.tools(); }
QList<Extension*> extensions1(PluginManager &self) { return self.extensions(); }
QList<Color*> colors1(PluginManager &self) { return self.colors(); }


void export_PluginManager()
{
  class_<Avogadro::PluginManager, boost::noncopyable>("PluginManager", no_init)
    // static read-only properties
    .add_static_property("instance", make_function(&PluginManager::instance,
          return_value_policy<reference_existing_object>()))

    // real functions
    .def("loadFactories",
        &loadFactories1,
        "Find all plugins by looking through the search paths:\n"
        "   <bin_location>/../<lib_dir>/avogadro/engines\n"
        "   <bin_location>/../<lib_dir>/avogadro/tools\n"
        "   <bin_location>/../<lib_dir>/avogadro/extensions\n"
        "   <bin_location>/../<lib_dir>/avogadro/colors\n"
        "   <bin_location>/../<lib_dir>/avogadro/contrib (external plugins)\n\n"
        "You can set AVOGADRO_PLUGINS to designate a path at runtime to search.")
    .def("loadFactories", 
        &loadFactories2,
        "Find all plugins by looking through the search paths:\n"
        "   <bin_location>/../<lib_dir>/avogadro/engines\n"
        "   <bin_location>/../<lib_dir>/avogadro/tools\n"
        "   <bin_location>/../<lib_dir>/avogadro/extensions\n"
        "   <bin_location>/../<lib_dir>/avogadro/colors\n"
        "   <bin_location>/../<lib_dir>/avogadro/contrib (external plugins)\n\n"
        "You can set AVOGADRO_PLUGINS to designate a path at runtime to search.")

    .def("factories", &PluginManager::factories)
    //.staticmethod("factories")
    .def("factory", &PluginManager::factory, return_value_policy<reference_existing_object>())
    //.staticmethod("factory")

    .def("extension", 
        &PluginManager::extension, return_value_policy<manage_new_object>(),
        "Get a new instance of the extension with identifier @p id.")
    .def("extension", 
        &extension1, return_value_policy<manage_new_object>(),
        "Get a new instance of the extension with identifier @p id.")

    .def("tool", 
        &PluginManager::tool, return_value_policy<manage_new_object>(),
        "Get a new instance of the tool with identifier @p id.")
    .def("tool", 
        &tool1, return_value_policy<manage_new_object>(),
        "Get a new instance of the tool with identifier @p id.")

    .def("color", 
        &PluginManager::color, return_value_policy<manage_new_object>(),
        "Get a new instance of the color with identifier @p id.")
    .def("color", 
        &color1, return_value_policy<manage_new_object>(),
        "Get a new instance of the color with identifier @p id.")

    .def("engine", 
        &PluginManager::engine, return_value_policy<manage_new_object>(),
        "Get a new instance of the color with identifier @p id.")
    .def("engine", 
        &engine1, return_value_policy<manage_new_object>(),
        "Get a new instance of the color with identifier @p id.")

    .def("names", 
        &PluginManager::names,
        "Get a list with the plugin names of type @p type.")

    .def("identifiers", 
        &PluginManager::identifiers,
        "Get a list with the plugin identifiers of type @p type.")

    .def("descriptions", 
        &PluginManager::descriptions,
        "Get a list with the plugin descriptions of type @p type.")

    .def("extensions", 
        &PluginManager::extensions,
        "Get a list of all extension (new instances).")
    .def("extensions", 
        &extensions1,
        "Get a list of all extension (new instances).")

    .def("tools", 
        &PluginManager::tools,
        "Get a list of all tool (new instances).")
    .def("tools", 
        &tools1,
        "Get a list of all tool (new instances).")

    .def("colors", 
        &PluginManager::colors,
        "Get a list of all color (new instances).")
    .def("colors", 
        &colors1,
        "Get a list of all color (new instances).")

    .def("writeSettings", 
        &PluginManager::writeSettings,
        "Write the settings of the PluginManager in order to save them to disk.")
    ;

}
