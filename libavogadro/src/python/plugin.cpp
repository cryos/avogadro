// Last update: timvdm 19 June 2009
#include <boost/python.hpp>

#include <avogadro/plugin.h>

#include <QWidget>

using namespace boost::python;
using namespace Avogadro;
      
Plugin* createInstance(PluginFactory &self)
{
  return self.createInstance();
}

void export_Plugin()
{
  enum_<Plugin::Type>("PluginType")
    // the Type enum
    .value("EngineType", Plugin::EngineType)
    .value("ToolType", Plugin::ToolType)
    .value("ExtensionType", Plugin::ExtensionType)
    .value("ColorType", Plugin::ColorType)
    .value("OtherType", Plugin::OtherType)
    .value("TypeCount", Plugin::TypeCount)
    ;

  class_<Avogadro::Plugin, boost::noncopyable>("Plugin", no_init)
    //
    // read-only poperties 
    //
    .add_property("type", 
        &Plugin::type,
        "The type of the plugin.")

    .add_property("identifier", 
        &Plugin::identifier,
        "The unique, untranslated identifier for the plugin.")

    .add_property("name", 
        &Plugin::name,
        "The translated name of the plugin.")

    .add_property("description", 
        &Plugin::description,
        "A description of the plugin.")

    .add_property("settingsWidget", 
        make_function(&Plugin::settingsWidget, 
        // use the QClass_converters to give a real PyQt object
        return_value_policy<return_by_value>()),
        "a QWidget containing the engine settings or None if no settings widget is available.") 

    //
    // functions
    //
    .def("readSettings", 
        &Plugin::readSettings,
        "Read in the settings that have been saved for the plugin instance.")

    .def("writeSettings", 
        &Plugin::writeSettings,
        "Write the plugin settings so that they can be saved between sessions.")
    ;
  
  class_<Avogadro::PluginFactory, boost::noncopyable>("PluginFactory", no_init)
    .add_property("type",
        &PluginFactory::type, 
        "The type of the plugin.")
    .add_property("identifier", 
        &PluginFactory::identifier,
        "The untranslated identifier of the plugin.")
    .add_property("name", 
        &PluginFactory::name,
        "The translated name of the plugin.")
    .add_property("description", 
        &PluginFactory::description,
        "A description of the plugin.")
    //.def("createInstance", &PluginFactory::createInstance, return_value_policy<reference_existing_object>())
    .def("createInstance", 
        &PluginFactory::createInstance, return_value_policy<manage_new_object>(),
        "A new instance of an Engine subclass object.")
    .def("createInstance", 
        createInstance, return_value_policy<manage_new_object>(),
        "A new instance of an Engine subclass object.")
    ;
   
}
