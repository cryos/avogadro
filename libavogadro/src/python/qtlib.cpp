#include <Python.h>
#include <boost/python.hpp>

#include <QObject>
#include <QWidget>

using namespace boost::python;

void export_QtLib()
{

  class_<QObject, boost::noncopyable>("QObject", no_init)
    .add_property("parent", make_function(&QObject::parent, return_value_policy<reference_existing_object>()))
    ;
 
  // QWidget
  void (QWidget::*resize_ptr)(int, int) = &QWidget::resize;

  class_<QWidget, bases<QObject>, boost::noncopyable>("QWidget", no_init)
    .def("show", &QWidget::show)
    .def("resize", resize_ptr)

    ;
   
}
