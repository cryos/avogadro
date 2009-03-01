#include <boost/python.hpp>
#include <boost/tuple/tuple.hpp>

#include <QList>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/fragment.h>
#include <avogadro/cube.h>
#include <avogadro/mesh.h>
#include <avogadro/glwidget.h>
#include <avogadro/engine.h>
#include <avogadro/tool.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

using namespace boost::python;

// C = QList<T>
template <class C>
struct QList_to_python_list
{
  typedef typename C::value_type T;
  typedef typename C::const_iterator iter;
  
  static PyObject* convert(C const &list)
  {
    // the python list
    boost::python::list pyList;

    foreach (const T& item, list) {
      pyList.append(item);
    }

    return incref(pyList.ptr());
  }
};

// C = QList<T*>
template <class C>
struct QList_ptr_to_python_list
{
  typedef typename C::value_type T;
  typedef typename C::const_iterator iter;
  
  static PyObject* convert(C const &list)
  {
    // the python list
    boost::python::list pyList;
    // we need to wrap the pointers into PyObjects
    typename boost::python::reference_existing_object::apply<T*>::type converter;

    for (iter i = list.begin(); i != list.end(); ++i) {
      PyObject *obj = converter(*i);
      object real_obj = object(handle<>(obj));
      // append the PyObject
      pyList.append(real_obj);
    }

    return incref(pyList.ptr());
  }
};


template <class C>
struct QList_from_python_list
{
  typedef typename C::value_type T;

  QList_from_python_list()
  {
    converter::registry::push_back( &convertible, &construct, type_id<C>() );
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
        if (!boost::python::extract<T>(t[i]).check())
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
                  
      //can all of the elements be converted to type 'T'?
      for (int i=0; i<n; ++i) {
        if (!boost::python::extract<T>(l[i]).check())
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
      void* storage = ((converter::rvalue_from_python_storage<C>*)data)->storage.bytes;

      //create the T container
      new (storage) C();

      C *container = static_cast<C*>(storage);

      //add all of the elements from the tuple - get the number of elements in the tuple
      int n = PyTuple_Size(obj_ptr);

      for (int i=0; i<n; ++i)
        container->append( extract<T>(t[i])() );

      data->convertible = storage;
    }
    else if (PyList_Check(obj_ptr)) {
      //convert the PyObject to a boost::python::object
      boost::python::list l( handle<>(borrowed(obj_ptr)) );

      //locate the storage space for the result
      void* storage = ((converter::rvalue_from_python_storage<C>*)data)->storage.bytes;

      //create the T container
      new (storage) C();

      C *container = static_cast<C*>(storage);

      //add all of the elements from the tuple - get the number of elements in the tuple
      int n = PyList_Size(obj_ptr);

      for (int i=0; i<n; ++i)
        container->append( extract<T>(l[i])() );

      data->convertible = storage;
    }
  }

};

template <class C>
struct QList_ptr_from_python_list
{
  typedef typename C::value_type T;

  QList_ptr_from_python_list()
  {
    converter::registry::push_back( &convertible, &construct, type_id<C>() );
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
        if (!boost::python::extract<T>(t[i]).check())
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
                  
      //can all of the elements be converted to type 'T'?
      for (int i=0; i<n; ++i) {
        if (!boost::python::extract<T>(l[i]).check())
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
      void* storage = ((converter::rvalue_from_python_storage<C>*)data)->storage.bytes;

      //create the T container
      new (storage) C();

      C *container = static_cast<C*>(storage);

      //add all of the elements from the tuple - get the number of elements in the tuple
      int n = PyTuple_Size(obj_ptr);

      for (int i=0; i<n; ++i)
        container->append( extract<T>(t[i])() );

      data->convertible = storage;
    }
    else if (PyList_Check(obj_ptr)) {
      //convert the PyObject to a boost::python::object
      boost::python::list l( handle<>(borrowed(obj_ptr)) );

      //locate the storage space for the result
      void* storage = ((converter::rvalue_from_python_storage<C>*)data)->storage.bytes;

      //create the T container
      new (storage) C();

      C *container = static_cast<C*>(storage);

      //add all of the elements from the tuple - get the number of elements in the tuple
      int n = PyList_Size(obj_ptr);

      for (int i=0; i<n; ++i)
        container->append( extract<T>(l[i])() );

      data->convertible = storage;
    }
  }

};

template <class T>
void export_list()
{
  to_python_converter<T, QList_to_python_list<T> >();
  QList_from_python_list<T>();
}

template <class T>
void export_ptr_list()
{
  to_python_converter<T, QList_ptr_to_python_list<T> >();
  QList_ptr_from_python_list<T>();
}

/*
class QListTest
{
  public:
    QList<QString> constantStringList()
    {
      QList<QString> list;

      list.append(QString("test 1"));
      list.append(QString("test 2"));
      list.append(QString("test 3"));

      return list;
    }
    const QList<QString>& stringList() const
    {
      return m_list;
    }
    void setStringList(QList<QString> list)
    {
      m_list = list;
    }
  private:
    QList<QString> m_list;
};
*/

using namespace Avogadro;

void export_QList()
{
  export_list< QList<QString> >();
  export_list< QList<GLHit> >();
  export_list< QList<unsigned long> >();

  /*  
  class_<QListTest>("QListTest")
    .def("constantStringList", &QListTest::constantStringList)
    .def("stringList", &QListTest::stringList, return_value_policy<return_by_value>())
    .def("setStringList", &QListTest::setStringList)
    ;
  */

  export_ptr_list< QList<Primitive*> >();
  export_ptr_list< QList<Atom*> >();
  export_ptr_list< QList<Bond*> >();
  export_ptr_list< QList<Residue*> >();
  export_ptr_list< QList<Cube*> >();
  export_ptr_list< QList<Mesh*> >();
  export_ptr_list< QList<Fragment*> >();
  
  export_ptr_list< QList<PluginFactory*> >();
  export_ptr_list< QList<Engine*> >();
  export_ptr_list< QList<Tool*> >();
  export_ptr_list< QList<Color*> >();
  export_ptr_list< QList<Extension*> >();
}
