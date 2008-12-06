#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/plugin.h>

#include <QWidget>

using namespace boost::python;
using namespace Avogadro;

void export_Plugin()
{
  enum_<Plugin::Type>("PluginType")
    // the Type enum
    .value("EngineType", Plugin::EngineType)
    .value("ToolType", Plugin::ToolType)
    .value("ExtensionType", Plugin::ExtensionType)
    .value("ColorType", Plugin::ColorType)
    .value("GradientType", Plugin::GradientType)
    .value("OtherType", Plugin::OtherType)
    .value("TypeCount", Plugin::TypeCount)
    ;

  class_<Avogadro::Plugin, boost::noncopyable>("Plugin", no_init)
    // read-only poperties 
    .add_property("type", &Plugin::type)
    .add_property("name", &Plugin::name)
    .add_property("description", &Plugin::description)
    .add_property("settingsWidget", make_function(&Plugin::settingsWidget, 
          return_value_policy<reference_existing_object>()))
    ;
  
  class_<Avogadro::PluginFactory, boost::noncopyable>("PluginFactory", no_init)
    .add_property("type", &PluginFactory::type)
    .add_property("name", &PluginFactory::name)
    .add_property("description", &PluginFactory::description)
    //.def("createInstance", &PluginFactory::createInstance, return_value_policy<reference_existing_object>())
    .def("createInstance", &PluginFactory::createInstance, return_value_policy<manage_new_object>())
    ;
   
}
