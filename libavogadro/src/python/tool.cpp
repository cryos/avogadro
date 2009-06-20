// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/tool.h>
#include <avogadro/molecule.h>
#include <avogadro/glwidget.h>

using namespace boost::python;
using namespace Avogadro;

void export_Tool()
{
  
  class_<Avogadro::Tool, bases<Avogadro::Plugin>, boost::noncopyable>("Tool", no_init)
    //
    // read-only poperties
    //
    .add_property("usefulness", 
        &Tool::usefulness,
        "Determines the ordering of the tools.  More useful "
        "tools are placed first.  It is up to the tool designer "
        "to be humble about their usefulness value.")

    .add_property("typeName", 
        &Tool::typeName,
        "Plugin Type Name (Tools).")

    .add_property("identifier", 
        &Tool::identifier,
        "The untranslated identifier for the tool.")
    ;
   
}
