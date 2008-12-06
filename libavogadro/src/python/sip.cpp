#ifdef ENABLE_PYTHON_SIP

#include <boost/python.hpp>
#include <boost/tuple/tuple.hpp>

#include <sip.h>
#include <iostream>

#include <QList>
#include <QWidget>
#include <QDockWidget>
#include <QAction>
#include <QUndoCommand>

using namespace boost::python;

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

template <typename T>
struct QClass_converters
{

  struct QClass_to_PyQt
  {
    static PyObject* convert(T* object)
    {
      if (!object)
        return incref(Py_None);
      
      sipWrapperType *type = sip_API->api_find_class(object->metaObject()->className());
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
    converter::registry::insert( &QClass_from_PyQt, type_id<T>() );
    to_python_converter<T*, QClass_to_PyQt>();
  }
  
};

// QUndoCommand doesn't have metaObject()
template <>
struct QClass_converters<QUndoCommand>
{

  struct QClass_to_PyQt
  {
    static PyObject* convert(QUndoCommand* object)
    {
      if (!object)
        return incref(Py_None);
      
      sipWrapperType *type = sip_API->api_find_class("QUndoCommand");
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
    converter::registry::insert( &QClass_from_PyQt, type_id<QUndoCommand>() );
    to_python_converter<QUndoCommand*, QClass_to_PyQt>();
  }
  
};



struct QList_QClass_to_array_PyQt
{
  typedef QList<QAction*>::const_iterator iter;

  static PyObject* convert(const QList<QAction*> &qList)
  {
    sipWrapperType *type = sip_API->api_find_class("QAction");
    if (!type)
      return 0;
     
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

#endif


void export_sip()
{
#ifdef ENABLE_PYTHON_SIP
  if (!init_sip_api())
    return;
  
  QClass_converters<QWidget>();
  QClass_converters<QAction>();
  QClass_converters<QDockWidget>();
  QClass_converters<QUndoCommand>();
  
  to_python_converter<QList<QAction*>, QList_QClass_to_array_PyQt>();
#endif
}

