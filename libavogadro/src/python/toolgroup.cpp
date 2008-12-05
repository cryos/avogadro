#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/tool.h>
#include <avogadro/toolgroup.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_ToolGroup()
{
  void (ToolGroup::*append_ptr1)(QList<Tool *>) = &ToolGroup::append;
  void (ToolGroup::*append_ptr2)(Tool *) = &ToolGroup::append;
  void (ToolGroup::*setActiveTool_ptr1)(int) = &ToolGroup::setActiveTool;
  void (ToolGroup::*setActiveTool_ptr2)(const QString&) = &ToolGroup::setActiveTool;
  void (ToolGroup::*setActiveTool_ptr3)(Tool *) = &ToolGroup::setActiveTool;


  // FIXME: Add activateActions?? (probably not need)

  class_<Avogadro::ToolGroup, boost::noncopyable>("ToolGroup")
    // read/write properties
    .add_property("activeTool", make_function(&ToolGroup::activeTool, return_value_policy<reference_existing_object>()), 
        setActiveTool_ptr3)
    .add_property("tools", make_function(&ToolGroup::tools, return_value_policy<return_by_value>()))
    // real functions
    .def("append", append_ptr1)
    .def("append", append_ptr2)
    .def("tool", &ToolGroup::tool, return_value_policy<reference_existing_object>())
    .def("setMolecule", &ToolGroup::setMolecule)
    .def("setActiveTool", setActiveTool_ptr1)
    .def("setActiveTool", setActiveTool_ptr2)
    ;
 
}
