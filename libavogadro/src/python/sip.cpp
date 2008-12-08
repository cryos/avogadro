#ifdef ENABLE_PYTHON_SIP

#include <boost/python.hpp>
#include <boost/tuple/tuple.hpp>

#include <sip.h>
#include <iostream>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/cube.h>
#include <avogadro/molecule.h>
#include <avogadro/glwidget.h>

#include <QObject>
#include <QList>
#include <QWidget>
#include <QGLWidget>
#include <QDockWidget>
#include <QAction>
#include <QUndoCommand>

using namespace boost::python;

/**
 * Initialize the SIP API
 */
const sipAPIDef *sip_API = 0;

bool init_sip_api()
{
  // import the sip module
  object sip_module = import("sip");
  if (!sip_module.ptr())
    return false;

  // get the dictionary
  dict sip_dict = extract<dict>(sip_module.attr("__dict__"));
  // get the _C_API object from the dictionary
  object sip_capi_obj = sip_dict.get("_C_API");
  if (!sip_capi_obj.ptr())
    return false;
  
  if (!sip_capi_obj.ptr() || !PyCObject_Check(sip_capi_obj.ptr()))
    return false;

  sip_API = reinterpret_cast<const sipAPIDef*>(PyCObject_AsVoidPtr(sip_capi_obj.ptr()));

  return true;
}

/**
 * MetaData helper templates. While this information is available 
 * (QObject::metaObject) for objects derived from QObject, not 
 * all classes are derived from it. To make all classes work with 
 * the same templates we do it this way.
 * 
 */

// these are used by the toPyQt function,
template <typename T> struct MetaData;

// used by toPyQt(...)
// Atom* -> QObject*
template <> struct MetaData<Avogadro::Atom> { static const char* className() { return "QObject";} }; 
// Bond* -> QObject*
template <> struct MetaData<Avogadro::Bond> { static const char* className() { return "QObject";} }; 
// Cube* -> QObject*
template <> struct MetaData<Avogadro::Cube> { static const char* className() { return "Qobject";} };
// Molecule* -> QObject*
template <> struct MetaData<Avogadro::Molecule> { static const char* className() { return "QObject";} };
// GLWidget* -> QGLWidget*
template <> struct MetaData<Avogadro::GLWidget> { static const char* className() { return "QGLWidget";} };

template <> struct MetaData<QObject> { static const char* className() { return "QObject";} };
template <> struct MetaData<QWidget> { static const char* className() { return "QWidget";} };
template <> struct MetaData<QDockWidget> { static const char* className() { return "QDockWidget";} };
template <> struct MetaData<QAction> { static const char* className() { return "QAction";} };
template <> struct MetaData<QUndoCommand> { static const char* className() { return "QUndoCommand";} };
template <> struct MetaData<QUndoStack> { static const char* className() { return "QUndoStack";} };
template <> struct MetaData<QPoint> { static const char* className() { return "QPoint";} };
template <> struct MetaData<QColor> { static const char* className() { return "QColor";} };
 

/**
 *  QWidget* <--> PyQt object
 *  QAction* <--> PyQt object
 *  ...
 */
template <typename T>
struct QClass_converters
{

  struct QClass_to_PyQt
  {
    static PyObject* convert(const T& object)
    {
      sipWrapperType *type = sip_API->api_find_class(MetaData<T>::className());
      if (!type)
        return incref(Py_None);
      
      PyObject *sip_obj = sip_API->api_convert_from_instance((void*)(&object), type, 0);
      if (!sip_obj)
        return incref(Py_None);

      return incref(sip_obj);
    }
 
    static PyObject* convert(T* object)
    {
      if (!object)
        return incref(Py_None);
      
      sipWrapperType *type = sip_API->api_find_class(MetaData<T>::className());
      if (!type)
        return incref(Py_None);
      
      PyObject *sip_obj = sip_API->api_convert_from_instance(object, type, 0);
      if (!sip_obj)
        return incref(Py_None);

      return incref(sip_obj);
    }
  };

  static void* QClass_from_PyQt(PyObject *obj_ptr)
  {
    if (!sip_API->api_wrapper_check(obj_ptr))
      throw_error_already_set();
    sipWrapper *wrapper = reinterpret_cast<sipWrapper*>(obj_ptr);
    return wrapper->u.cppPtr;
  }
    
  QClass_converters()
  {
    // example: PyQt object --> C++ pointer
    converter::registry::insert( &QClass_from_PyQt, type_id<T>() );
    // example: QUndoCommand* Extension::performAction(...) --> PyQt object
    to_python_converter<T*, QClass_to_PyQt>();
    // example: QColor GLWidget::background() --> PyQt object
    to_python_converter<T, QClass_to_PyQt>();
  }
  
};

/**
 *  Special case QList<QAction*> --> PyQt object
 */
struct QList_QClass_to_array_PyQt
{
  typedef QList<QAction*>::const_iterator iter;

  static PyObject* convert(const QList<QAction*> &qList)
  {
    sipWrapperType *type = sip_API->api_find_class("QAction");
    if (!type)
      return incref(Py_None);
     
    boost::python::list pyList;

    foreach (QAction *action, qList) {
      PyObject *sip_obj = sip_API->api_convert_from_instance(action, type, 0);
      if (!sip_obj)
        continue;
      boost::python::object real_obj = object(handle<>(sip_obj));
      pyList.append(real_obj);
    }

    return boost::python::incref(pyList.ptr());
  }
};


/**
 *  The toPyQt implementation
 */
template <typename T>
PyObject* toPyQt(T *obj)
{
  if (!obj)
    return incref(Py_None);
      
  sipWrapperType *type = sip_API->api_find_class(MetaData<T>::className());
  if (!type)
    return incref(Py_None);
      
  PyObject *sip_obj = sip_API->api_convert_from_instance(obj, type, 0);
  if (!sip_obj)
    return incref(Py_None);

  return incref(sip_obj);
}

#endif


void export_sip()
{
#ifdef ENABLE_PYTHON_SIP
  if (!init_sip_api())
    return;
 
  // toPyQt functions
  def("toPyQt", &toPyQt<Avogadro::Atom>);
  def("toPyQt", &toPyQt<Avogadro::Bond>);
  def("toPyQt", &toPyQt<Avogadro::Cube>);
  def("toPyQt", &toPyQt<Avogadro::Molecule>);
  def("toPyQt", &toPyQt<Avogadro::GLWidget>);
 
  // QClass* <--> PyQt objects 
  QClass_converters<QObject>();
  QClass_converters<QWidget>();
  QClass_converters<QAction>();
  QClass_converters<QDockWidget>();
  QClass_converters<QUndoCommand>();
  QClass_converters<QUndoStack>();
  QClass_converters<QPoint>();
  QClass_converters<QColor>();
 
  // special case 
  to_python_converter<QList<QAction*>, QList_QClass_to_array_PyQt>();
#endif
}

