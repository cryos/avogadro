#include <boost/python.hpp>
#include <boost/tuple/tuple.hpp>

#include <sip.h>
#include <iostream>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/cube.h>
#include <avogadro/engine.h>
#include <avogadro/extension.h>
#include <avogadro/fragment.h>
#include <avogadro/residue.h>
#include <avogadro/painter.h>
#include <avogadro/pluginmanager.h>
#include <avogadro/mesh.h>
#include <avogadro/tool.h>
#include <avogadro/toolgroup.h>
#include <avogadro/molecule.h>
#include <avogadro/glwidget.h>
#include <avogadro/moleculefile.h>

#include <QObject>
#include <QList>
#include <QWidget>
#include <QGLWidget>
#include <QDockWidget>
#include <QAction>
#include <QUndoCommand>
#include <QColor>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QSettings>

using namespace boost::python;

/**
 * Initialize the SIP API
 */
const sipAPIDef *sip_API = 0;

bool init_sip_api()
{
  // import the sip module
  object sip_module = import("sip");
  if (!sip_module.ptr()) {
    std::cout << "Could not import sip python module." << std::endl;
    return false;
  }

  // get the dictionary
  dict sip_dict = extract<dict>(sip_module.attr("__dict__"));
  if (!sip_dict.ptr()) {
    std::cout << "Could not find the __dict__ attribute in the sip python module." << std::endl;
    return false;
  }
  // get the _C_API object from the dictionary
  object sip_capi_obj = sip_dict.get("_C_API");
  if (!sip_capi_obj.ptr()) {
    std::cout << "Could not find the _C_API entry in the sip python module dictionary." << std::endl;
    return false;
  }
  
  if (!PyCObject_Check(sip_capi_obj.ptr())) {
    std::cout << "The _C_API object in the sip python module is invalid." << std::endl;
    return false;
  }

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
template <> struct MetaData<Avogadro::Atom> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::Bond> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::Cube> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::Engine> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::Extension> { static const char* className() { return "QObject";} }; 
template <> struct MetaData<Avogadro::Fragment> { static const char* className() { return "QObject";} }; 
template <> struct MetaData<Avogadro::GLWidget> { static const char* className() { return "QGLWidget";} }; 
template <> struct MetaData<Avogadro::Mesh> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::Molecule> { static const char* className() { return "QObject";} }; 
template <> struct MetaData<Avogadro::Painter> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::PluginManager> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::Primitive> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::Residue> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::Tool> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::ToolGroup> { static const char* className() { return "QObject";} };
template <> struct MetaData<Avogadro::MoleculeFile> { static const char* className() { return "QObject";} };


template <> struct MetaData<QObject> { static const char* className() { return "QObject";} };
template <> struct MetaData<QWidget> { static const char* className() { return "QWidget";} };
template <> struct MetaData<QDockWidget> { static const char* className() { return "QDockWidget";} };
template <> struct MetaData<QAction> { static const char* className() { return "QAction";} };
template <> struct MetaData<QActionGroup> { static const char* className() { return "QActionGroup";} };
template <> struct MetaData<QUndoCommand> { static const char* className() { return "QUndoCommand";} };
template <> struct MetaData<QUndoStack> { static const char* className() { return "QUndoStack";} };
template <> struct MetaData<QPoint> { static const char* className() { return "QPoint";} };
template <> struct MetaData<QColor> { static const char* className() { return "QColor";} };
template <> struct MetaData<QMouseEvent> { static const char* className() { return "QMouseEvent";} };
template <> struct MetaData<QWheelEvent> { static const char* className() { return "QWheelEvent";} };
template <> struct MetaData<QSettings> { static const char* className() { return "QSettings";} };
 

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
#ifdef SIP_4_8
      const sipTypeDef *type = sip_API->api_find_type(MetaData<T>::className());
#else
      sipWrapperType *type = sip_API->api_find_class(MetaData<T>::className());
#endif
      if (!type)
        return incref(Py_None);
      
#ifdef SIP_4_8
      PyObject *sip_obj = sip_API->api_convert_from_type((void*)(&object), type, 0);
#else
      PyObject *sip_obj = sip_API->api_convert_from_instance((void*)(&object), type, 0);
#endif
      if (!sip_obj)
        return incref(Py_None);

      return incref(sip_obj);
    }
 
    static PyObject* convert(T* object)
    {
      if (!object)
        return incref(Py_None);
 
#ifdef SIP_4_8
      const sipTypeDef *type = sip_API->api_find_type(MetaData<T>::className());
#else     
      sipWrapperType *type = sip_API->api_find_class(MetaData<T>::className());
#endif
      if (!type)
        return incref(Py_None);
      
#ifdef SIP_4_8
      PyObject *sip_obj = sip_API->api_convert_from_type(object, type, 0);
#else
      PyObject *sip_obj = sip_API->api_convert_from_instance(object, type, 0);
#endif
      if (!sip_obj)
        return incref(Py_None);

      return incref(sip_obj);
    }

    static PyObject* convert(const T* object)
    {
      return convert((T*)object);
    }

  };

  static void* QClass_from_PyQt(PyObject *obj_ptr)
  {
#ifdef SIP_4_8
    if (PyObject_TypeCheck(obj_ptr, sip_API->api_wrapper_type))
#else
    if (!sip_API->api_wrapper_check(obj_ptr))
#endif
      throw_error_already_set();
    
    // transfer ownership from python to C++
#ifdef SIP_4_8
    sip_API->api_transfer_to(obj_ptr, 0);
#else
    sip_API->api_transfer(obj_ptr, 1);
#endif
    
    // reinterpret to sipWrapper
#ifdef SIP_4_8
    sipSimpleWrapper *wrapper = reinterpret_cast<sipSimpleWrapper*>(obj_ptr);
#else
    sipWrapper *wrapper = reinterpret_cast<sipWrapper*>(obj_ptr);
#endif
    // return the C++ pointer
    return wrapper->u.cppPtr;
  }
    
  QClass_converters()
  {
    // example: PyQt object --> C++ pointer
    converter::registry::insert( &QClass_from_PyQt, type_id<T>() );
    // example: const QColor* -> PyQt object
    to_python_converter<const T*, QClass_to_PyQt>();
    // example: QUndoCommand* Extension::performAction(...) --> PyQt object
    to_python_converter<T*, QClass_to_PyQt>();
    // example: QColor GLWidget::background() --> PyQt object
    to_python_converter<T, QClass_to_PyQt>();
  }
  
};

/**
 *  Special case QList<QAction*> --> PyQt object
 */
struct QList_QAction_to_python_list_PyQt
{
  typedef QList<QAction*>::const_iterator iter;

  static PyObject* convert(const QList<QAction*> &qList)
  {
#ifdef SIP_4_8
    const sipTypeDef *type = sip_API->api_find_type("QAction");
#else
    sipWrapperType *type = sip_API->api_find_class("QAction");
#endif
    if (!type)
      return incref(Py_None);
     
    boost::python::list pyList;

    foreach (QAction *action, qList) {
#ifdef SIP_4_8
      PyObject *sip_obj = sip_API->api_convert_from_type(action, type, 0);
#else
      PyObject *sip_obj = sip_API->api_convert_from_instance(action, type, 0);
#endif
      if (!sip_obj)
        continue;
      boost::python::object real_obj = object(handle<>(sip_obj));
      pyList.append(real_obj);
    }

    return boost::python::incref(pyList.ptr());
  }
};

struct QList_QAction_from_python_list_PyQt
{
  QList_QAction_from_python_list_PyQt()
  {
    converter::registry::push_back( &convertible, &construct, type_id< QList<QAction*> >() );
  }

  static void* convertible(PyObject *obj_ptr)
  {

    //is this a tuple type?
    if (PyTuple_Check(obj_ptr)) {
      //check the tuple elements... - convert to a boost::tuple object
      boost::python::tuple t( handle<>(borrowed(obj_ptr)) );
      
      //how many elements are there?
      int n = PyTuple_Size(obj_ptr);
      
      //can they all be converted to type 'T'?
      for (int i=0; i<n; ++i) {
        if (!boost::python::extract<QAction*>(t[i]).check())
          return 0;
      }
                            
      //the tuple is ok!
      return obj_ptr;                         
    }
    //is this a list type?
    else if (PyList_Check(obj_ptr)) {             
      //check that all of the list elements can be converted to the right type                    
      boost::python::list l( handle<>(borrowed(obj_ptr)) );
                                
      //how many elements are there?                
      int n = PyList_Size(obj_ptr);

      //return obj_ptr;
                  
      //can all of the elements be converted to type 'T'?
      for (int i=0; i<n; ++i) {
        if (!boost::python::extract<QAction*>(l[i]).check())
          return 0;
      }

      //the list is ok!
      return obj_ptr;
    }    
               
    //could not recognise the type...
    return 0;
  }

  static void construct(PyObject *obj_ptr, converter::rvalue_from_python_stage1_data *data)
  {
    if (PyTuple_Check(obj_ptr)) {
      //convert the PyObject to a boost::python::object
      boost::python::tuple t( handle<>(borrowed(obj_ptr)) );

      //locate the storage space for the result
      void* storage = ((converter::rvalue_from_python_storage< QList<QAction*> >*)data)->storage.bytes;

      //create the T container
      new (storage) QList<QAction*>();

      QList<QAction*> *container = static_cast< QList<QAction*>* >(storage);

      //add all of the elements from the tuple - get the number of elements in the tuple
      int n = PyTuple_Size(obj_ptr);

      for (int i=0; i<n; ++i) {
        QAction *action = boost::python::extract<QAction*>(t[i]);
        container->append( action );
      }

      data->convertible = storage;
    }
    else if (PyList_Check(obj_ptr)) {
      //convert the PyObject to a boost::python::object
      boost::python::list l( handle<>(borrowed(obj_ptr)) );

      //locate the storage space for the result
      void* storage = ((converter::rvalue_from_python_storage< QList<QAction*> >*)data)->storage.bytes;

      //create the T container
      new (storage) QList<QAction*>();

      QList<QAction*> *container = static_cast< QList<QAction*>* >(storage);

      //add all of the elements from the tuple - get the number of elements in the tuple
      int n = PyList_Size(obj_ptr);

      for (int i=0; i<n; ++i) {
        QAction *action = boost::python::extract<QAction*>(l[i]);
        container->append( action );
      }

      data->convertible = storage;
    }
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
      
#ifdef SIP_4_8
  const sipTypeDef *type = sip_API->api_find_type(MetaData<T>::className());
#else
  sipWrapperType *type = sip_API->api_find_class(MetaData<T>::className());
#endif
  if (!type)
    return incref(Py_None);
      
#ifdef SIP_4_8
  PyObject *sip_obj = sip_API->api_convert_from_type(obj, type, 0);
#else
  PyObject *sip_obj = sip_API->api_convert_from_instance(obj, type, 0);
#endif
  if (!sip_obj)
    return incref(Py_None);

  return incref(sip_obj);
}

void export_sip()
{
  if (!init_sip_api()) {
    std::cout << "Could not initialize SIP API !" << std::endl;
    return;
  } 

  // toPyQt functions
  def("toPyQt", &toPyQt<Avogadro::Atom>);
  def("toPyQt", &toPyQt<Avogadro::Bond>);
  def("toPyQt", &toPyQt<Avogadro::Cube>);
  def("toPyQt", &toPyQt<Avogadro::Engine>);
  def("toPyQt", &toPyQt<Avogadro::Extension>);
  def("toPyQt", &toPyQt<Avogadro::Fragment>);
  def("toPyQt", &toPyQt<Avogadro::GLWidget>);
  def("toPyQt", &toPyQt<Avogadro::Mesh>);
  def("toPyQt", &toPyQt<Avogadro::Molecule>);
  def("toPyQt", &toPyQt<Avogadro::Painter>);
  def("toPyQt", &toPyQt<Avogadro::PluginManager>);
  def("toPyQt", &toPyQt<Avogadro::Primitive>);
  def("toPyQt", &toPyQt<Avogadro::Residue>);
  def("toPyQt", &toPyQt<Avogadro::Tool>);
  def("toPyQt", &toPyQt<Avogadro::ToolGroup>);
  def("toPyQt", &toPyQt<Avogadro::MoleculeFile>);
  
  // QClass* <--> PyQt objects 
  QClass_converters<QObject>();
  QClass_converters<QWidget>(); // from python
  QClass_converters<QAction>(); // from/to python
  QClass_converters<QDockWidget>();
  QClass_converters<QUndoCommand>(); // from python
  QClass_converters<QUndoStack>();
  QClass_converters<QPoint>();
  QClass_converters<QColor>();
  QClass_converters<QMouseEvent>(); // to python
  QClass_converters<QWheelEvent>(); // to python
  QClass_converters<QSettings>(); // to python
  QClass_converters<QActionGroup>(); // to python


  // special case 
  to_python_converter<QList<QAction*>, QList_QAction_to_python_list_PyQt>();
  QList_QAction_from_python_list_PyQt();
}

