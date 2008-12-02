#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/plugin.h>

using namespace boost::python;
using namespace Avogadro;

void export_Plugin()
{
  class_<Avogadro::Plugin, boost::noncopyable>("Plugin", no_init)
    // read-only poperties 
    .add_property("type", &Plugin::type)
    .add_property("name", &Plugin::name)
    .add_property("description", &Plugin::description)
    //.add_property("settingsWidget", &Plugin::settingsWidget) // QWidget
    ;
   
}
