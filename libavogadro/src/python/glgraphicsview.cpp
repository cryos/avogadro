// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/glgraphicsview.h>
#include <avogadro/glwidget.h>

using namespace boost::python;
using namespace Avogadro;

void export_GLGraphicsView()
{ 
 
  class_<Avogadro::GLGraphicsView, boost::noncopyable>("GLGraphicsView")
    .def(init<QWidget*>())
    .def(init<GLWidget*>())
    .def(init<GLWidget*, QWidget*>())
    .def(init<QGraphicsScene*, QWidget*>())
    // read-only properties
    .add_property("glWidget", make_function(&GLGraphicsView::glWidget, return_value_policy<reference_existing_object>()))
    ;

}
