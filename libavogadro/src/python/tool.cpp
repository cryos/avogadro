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
    // read-only poperties
    .add_property("usefulness", &Tool::usefulness)
    .add_property("typeName", &Tool::typeName)
    .add_property("identifier", &Tool::identifier)
    // real functions
    .def("setMolecule", &Tool::setMolecule)
    ;
   
}
