// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/extension.h>
#include <avogadro/molecule.h>
#include <avogadro/glwidget.h>

#include <QAction>
#include <QDockWidget>
#include <QUndoCommand>

using namespace boost::python;
using namespace Avogadro;

void export_Extension()
{
  
  class_<Avogadro::Extension, bases<Avogadro::Plugin>, boost::noncopyable>("Extension", no_init)
    // read-only poperties
    .add_property("actions", &Extension::actions)
    .add_property("typeName", &Extension::typeName)
    .add_property("dockWidget", make_function(&Extension::dockWidget, 
          return_value_policy<return_by_value>())) // use QClass_converter to return PyQt object
    // real functions
    
    .def("menuPath", &Extension::menuPath)
    .def("setMolecule", &Extension::setMolecule)
    .def("performAction", make_function(&Extension::performAction, 
          return_value_policy<return_by_value>())) // use QClass_converter to return PyQt object
 
    ;
   
}
