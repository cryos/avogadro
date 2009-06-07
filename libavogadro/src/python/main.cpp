#include <Python.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_sip();

void export_Eigen();

void export_QString();
void export_QList();
void export_std_vector();

void export_Animation();
void export_Atom();
void export_Bond();
void export_Camera();
void export_Color();
void export_Cube();
void export_ElementTranslator();
void export_Engine();
void export_Extension();
void export_FileIO();
void export_Fragment();
void export_GLWidget();
void export_Mesh();
void export_MeshGenerator();
void export_Molecule();
void export_MoleculeList();
void export_Navigate();
void export_NeighborList();
void export_Painter();
void export_PainterDevice();
void export_PeriodicTableView();
void export_Plugin();
void export_PluginManager();
void export_Primitive();
void export_PrimitiveList();
void export_Residue();
void export_Tool();
void export_ToolGroup();

BOOST_PYTHON_MODULE(_Avogadro) {

  try { import("PyQt4.QtCore"); } catch(...) {} // needed for toPyQt(...)
  try { import("PyQt4.QtGui"); } catch(...) {} // needed for toPyQt(...)
  try { import("PyQt4.QtOpenGL"); } catch(...) {} // needed for toPyQt(...)
  
  // include the Eigen converter(s)
  export_Eigen();

  export_sip();

  export_QString();
  export_QList();
  export_std_vector();

  // create base classes first
  export_Primitive();
  export_Plugin();
  
  export_Animation();
  export_Atom();
  export_Bond();
  export_Camera();
  export_Color();
  export_Cube();
  export_ElementTranslator();
  export_Engine();
  export_Extension();
  export_FileIO();
  export_Fragment();
  export_GLWidget();
  export_Mesh();
  export_MeshGenerator();
  export_Molecule();
  export_MoleculeList();
  export_Navigate();
  export_NeighborList();
  export_Painter();
  export_PainterDevice();
  export_PeriodicTableView();
  export_PluginManager();
  export_PrimitiveList();
  export_Residue();
  export_Tool();
  export_ToolGroup();




} 



