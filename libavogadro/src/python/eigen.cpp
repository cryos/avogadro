#include <Python.h>
#include <Numeric/arrayobject.h> 
#include <boost/python.hpp>
#include <boost/python/numeric.hpp>
#include <boost/python/tuple.hpp>

#include <Eigen/Core>

using namespace boost::python;

  struct Vector3d_to_python_array
  {
    static PyObject* convert(Eigen::Vector3d const &vec)
    {
      return incref(numeric::array(make_tuple(vec.x(), vec.y(), vec.z())).ptr());
    }
    
    static PyObject* convert(Eigen::Vector3d *vec)
    {
      if (!vec)
        throw_error_already_set();
      
      return incref(numeric::array(make_tuple(vec->x(), vec->y(), vec->z())).ptr());
    }

    static PyObject* convert(const Eigen::Vector3d *vec)
    {
      if (!vec)
        throw_error_already_set();

      return incref(numeric::array(make_tuple(vec->x(), vec->y(), vec->z())).ptr());
    }
  };

  struct Vector3d_from_python_array
  {
    Vector3d_from_python_array()
    {
      converter::registry::push_back( &convertible, &construct, type_id<Eigen::Vector3d>() );
    }

    static void* convertible(PyObject *obj_ptr)
    {
      if (!PyArray_Check(obj_ptr))
        return 0;
      return obj_ptr;
    }

    static void construct(PyObject *obj_ptr, converter::rvalue_from_python_stage1_data *data)
    {
      PyArrayObject *array = reinterpret_cast<PyArrayObject*>(obj_ptr);

      // check the dimensions
      if (array->nd != 1)
        throw_error_already_set(); // the array has at least two dimensions (matrix)
  
      if (array->dimensions[0] != 3)
        throw_error_already_set(); // the 1D array does not have exactly 3 elements

      double *values = reinterpret_cast<double*>(array->data);
    
      void *storage = ((converter::rvalue_from_python_storage<Eigen::Vector3d>*)data)->storage.bytes;
      new (storage) Eigen::Vector3d(values[0], values[1], values[2]);
      data->convertible = storage;
    }
  };

void export_Eigen()
{
  import_array(); // needed for NumPy 
  to_python_converter<Eigen::Vector3d, Vector3d_to_python_array>();
  to_python_converter<Eigen::Vector3d*, Vector3d_to_python_array>();
  to_python_converter<const Eigen::Vector3d*, Vector3d_to_python_array>();
  Vector3d_from_python_array();

  /*
  class_<EigenTest>("EigenTest")
    .def("vectorPtr", &EigenTest::vectorPtr, return_value_policy<return_by_value>())
    .def("vectorRef", &EigenTest::vectorRef, return_value_policy<return_by_value>())
    .def("vectorConstPtr", &EigenTest::vectorConstPtr, return_value_policy<return_by_value>())
    .def("vectorConstRef", &EigenTest::vectorConstRef, return_value_policy<return_by_value>())
    .def("vectorConstPtrConst", &EigenTest::vectorConstPtrConst, return_value_policy<return_by_value>())
    ;
  */


}
