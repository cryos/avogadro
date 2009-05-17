#include <boost/python.hpp>
#include <boost/tuple/tuple.hpp>

#include <vector>

#include <Eigen/Core>

using namespace boost::python;

// C = std::vector<T>
template <class C>
struct std_vector_to_python_list
{
  typedef typename C::value_type T;
  typedef typename C::const_iterator iter;
 
  // 
  // example std::vector<double> ???::data()
  //
  static PyObject* convert(C const &list)
  {
    // the python list
    boost::python::list pyList;

    for (iter i = list.begin(); i != list.end(); ++i) {
      pyList.append(*i);
    }

    return incref(pyList.ptr());
  }

};

struct std_vector_double_ptr_to_python_list
{
  //typedef typename std::vector<double>::const_iterator iter;
  typedef std::vector<double>::const_iterator iter;

  //
  // example: std::vector<double>* Cube::data()
  //
  static PyObject* convert(std::vector<double>* list)
  {
    // the python list
    boost::python::list pyList;

    for (iter i = list->begin(); i != list->end(); ++i) {
      pyList.append(*i);
    }

    return incref(pyList.ptr());
  }

};
 

template <class C>
struct std_vector_from_python_list
{
  typedef typename C::value_type T;

  std_vector_from_python_list()
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
        container->push_back( extract<T>(t[i])() );

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
        container->push_back( extract<T>(l[i])() );

      data->convertible = storage;
    }
  }

};


template <class T>
void export_std_vector()
{
  to_python_converter<T, std_vector_to_python_list<T> >();
  std_vector_from_python_list<T>();
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

void export_std_vector()
{
  export_std_vector< std::vector<double> >(); // for Cube
  export_std_vector< std::vector<Eigen::Vector3f> >(); // for Mesh
  export_std_vector< std::vector<Eigen::Vector3d> >(); // for Mesh
  
  to_python_converter<std::vector<double>*, std_vector_double_ptr_to_python_list >();

  /*  
  class_<QListTest>("QListTest")
    .def("constantStringList", &QListTest::constantStringList)
    .def("stringList", &QListTest::stringList, return_value_policy<return_by_value>())
    .def("setStringList", &QListTest::setStringList)
    ;
  */

}
