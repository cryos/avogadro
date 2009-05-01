#include <boost/python.hpp>

#include <avogadro/tool.h>
#include <avogadro/molecule.h>
#include <avogadro/glwidget.h>

using namespace boost::python;
using namespace Avogadro;

void export_Tool()
{
  //class_<Avogadro::Tool, bases<QObject, Avogadro::Plugin>, boost::noncopyable>("Tool", no_init)
  class_<Avogadro::Tool, bases<Avogadro::Plugin>, boost::noncopyable>("Tool", no_init)
    // read-only poperties
    .add_property("usefulness", &Tool::usefulness)
    .add_property("typeName", &Tool::typeName)
    .add_property("identifier", &Tool::identifier)
    //.add_property("activateAction", &Tool::activateAction) // QAction
    // real functions
    
    // I don't think we need these functions... only the GLWidget has to be able to call them 
    
    //.def("mousePress", &Tool::mousePress) // QUndoCommand
    //.def("mouseMove", &Tool::mouseMove) // QUndoCommand
    //.def("mouseRelease", &Tool::mouseRelease) // QUndoCommand
    //.def("wheel", &Tool::wheel) // QUndoCommand
    //.def("paint", &Tool::paint)
    //.def("readSettings", &Tool::readSettings) // QSettings
    //.def("writeSettings", &Tool::writeSettings) // QSettings
    .def("setMolecule", &Tool::setMolecule)
    ;
   
}
