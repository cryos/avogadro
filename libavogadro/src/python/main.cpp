#include <Python.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_Eigen();

void export_QString();
void export_QList();
void export_std_vector();

void export_Primitive();
void export_Atom();
void export_Bond();
void export_Fragment();
void export_Residue();
void export_Molecule();
void export_Cube();
void export_Camera();
void export_GLWidget();
void export_Painter();
void export_PainterDevice();
void export_Plugin();
void export_Color();
void export_Tool();
void export_Engine();
void export_Mesh();
void export_PrimitiveList();

BOOST_PYTHON_MODULE(Avogadro) {
  
  // include the Eigen converter(s)
  export_Eigen();

  export_QString();
  export_QList();
  export_std_vector();

  export_Primitive();
  export_Molecule();
  export_Atom();
  export_Bond();
  export_Fragment();
  export_Residue();
  export_Cube();
  export_Camera();
  export_GLWidget();
  export_Painter();
  export_PainterDevice();
  export_Plugin();
  export_Color();
  export_Tool();
  export_Engine();
  export_Mesh();
  export_PrimitiveList();




} 



