// Last update: timvdm 18 June 2009
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
    //
    // read-only poperties
    //
    .add_property("actions", 
        &Extension::actions, 
        "A list of actions which this widget can perform")

    .add_property("typeName", 
        &Extension::typeName, 
        "Plugin Type Name (Extensions)")

    .add_property("dockWidget", 
        make_function(&Extension::dockWidget, return_value_policy<return_by_value>()),
        "A list of dock widgets associated with this extensions") // use QClass_converter to return PyQt object

    // real functions
    .def("menuPath", 
        &Extension::menuPath, 
        "The menu path for the specified action")

    .def("setMolecule", 
        &Extension::setMolecule, 
        "Slot to set the Molecule for the Extension - should be called whenever the active Molecule changes.")

    .def("performAction", 
        make_function(&Extension::performAction, 
          return_value_policy<return_by_value>())) // use QClass_converter to return PyQt object
    ;
   
}
