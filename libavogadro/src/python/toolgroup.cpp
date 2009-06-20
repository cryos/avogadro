#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/tool.h>
#include <avogadro/toolgroup.h>
#include <avogadro/molecule.h>

#include <QActionGroup>

using namespace boost::python;
using namespace Avogadro;

void export_ToolGroup()
{
  void (ToolGroup::*append_ptr1)(QList<Tool *>) = &ToolGroup::append;
  void (ToolGroup::*append_ptr2)(Tool *) = &ToolGroup::append;
  
  Tool* (ToolGroup::*activeTool_ptr)() const = &ToolGroup::activeTool;
  void (ToolGroup::*setActiveTool_ptr1)(int) = &ToolGroup::setActiveTool;
  void (ToolGroup::*setActiveTool_ptr2)(const QString&) = &ToolGroup::setActiveTool;
  void (ToolGroup::*setActiveTool_ptr3)(Tool *) = &ToolGroup::setActiveTool;


  class_<Avogadro::ToolGroup, boost::noncopyable>("ToolGroup")
    //
    // read/write properties
    //
    .add_property("activeTool", 
        make_function(activeTool_ptr, return_value_policy<reference_existing_object>()), 
        setActiveTool_ptr3,
        "The active tool.")

    .add_property("tools", 
        make_function(&ToolGroup::tools, return_value_policy<return_by_value>()),
        "List of the tools.")

    // FIXME: QActionGroup
    .add_property("activateActions", 
        make_function(&ToolGroup::activateActions, return_value_policy<return_by_value>()),
        "QActionGroup of all the tool select actions.")

    //
    // real functions
    //
    .def("append", 
        append_ptr1,
        "Append the tools to the ToolGroup.")
    .def("append", 
        append_ptr2,
        "Append the tool to the ToolGroup")

    .def("tool", 
        &ToolGroup::tool, return_value_policy<reference_existing_object>(),
        "Get the tool at index i.")

    .def("setMolecule", 
        &ToolGroup::setMolecule,
        "Set the molecule.")

    .def("setActiveTool", 
        setActiveTool_ptr1,
        "Set the active tool.")
    .def("setActiveTool", 
        setActiveTool_ptr2,
        "Set the active tool.")

    .def("removeAllTools", 
        &ToolGroup::removeAllTools,
        "Reset the toolgroup to it's original state.")

    .def("writeSettings", 
        &ToolGroup::writeSettings,
        "Write the settings of the GLWidget in order to save them to disk.")

    .def("readSettings", 
        &ToolGroup::readSettings,
        "Read the settings of the GLWidget and restore them.")
    ;
 
}
