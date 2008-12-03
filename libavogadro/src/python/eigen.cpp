#include <Python.h>
#include <numpy/arrayobject.h> 
#include <boost/python.hpp>
#include <boost/python/tuple.hpp>

#include <Eigen/Geometry>

//#include <iostream>

using namespace boost::python;

  /***********************************************************************
   *
   * Vector3x = Vector3d, Vector3f, Vector3i
   *
   ***********************************************************************/
  
  template <class Vector3x>
  struct Vector3x_to_python_array
  {
    struct innerclass
    {
      //
      //  Eigen::Vector3x --> python array
      //
      static PyObject* convert(Vector3x const &vec)
      {
        int dims[1] = { 3 };
        PyObject *result = PyArray_FromDims(1, dims, PyArray_DOUBLE);
        
        // copy the data
        double *data = (double*) reinterpret_cast<PyArrayObject*>(result)->data;
        data[0] = vec.x();
        data[1] = vec.y();
        data[2] = vec.z();
        
        return incref(result);
      }
 
      //
      //  Eigen::Vector3x * --> python array
      //    
      static PyObject* convert(Vector3x *vec)
      {
        if (!vec)
          throw_error_already_set();
 
        int dims[1] = { 3 };
        PyObject *result = PyArray_FromDims(1, dims, PyArray_DOUBLE);
        
        // copy the data
        double *data = (double*) reinterpret_cast<PyArrayObject*>(result)->data;
        data[0] = vec->x();
        data[1] = vec->y();
        data[2] = vec->z();
        
        return incref(result);
      }

      //
      //  const Eigen::Vector3x * --> python array
      //
      static PyObject* convert(const Vector3x *vec)
      {
        if (!vec)
          throw_error_already_set();

        int dims[1] = { 3 };
        PyObject *result = PyArray_FromDims(1, dims, PyArray_DOUBLE);
        
        // copy the data
        double *data = (double*) reinterpret_cast<PyArrayObject*>(result)->data;
        data[0] = vec->x();
        data[1] = vec->y();
        data[2] = vec->z();
        
        return incref(result);
      }
    };
   
    Vector3x_to_python_array()
    {
      to_python_converter<Vector3x, innerclass>();
      to_python_converter<Vector3x*, innerclass>();
      to_python_converter<const Vector3x*, innerclass>();
    }

  };

  template <class Vector3x>
  struct Vector3x_from_python_array
  {
    typedef typename Vector3x::Scalar Scalar;
    
    Vector3x_from_python_array()
    {
      // Insert an rvalue from_python converter at the tail of the
      // chain. Used for implicit conversions
      //
      //  python array --> Vector3x
      //
      // used for:
      //
      //  void function(Vector3x vec)
      //  void function(Vector3x & vec)
      //  void function(const Vector3x & vec)
      //
      converter::registry::push_back( &convertible, &construct, type_id<Vector3x>() );

      
      // Insert an lvalue from_python converter
      //
      //  python array --> Vector3x*
      //
      // used for:
      //  
      //  void function(const Vector3x * vec)
      converter::registry::insert( &convert, type_id<Vector3x>() );
    }

    static void* convert(PyObject *obj_ptr)
    {
      //if (!PyArray_Check(obj_ptr))
      //  throw_error_already_set();

      PyArrayObject *array = reinterpret_cast<PyArrayObject*>(obj_ptr);

      // check the dimensions
      if (array->nd != 1)
        throw_error_already_set(); // the array has at least two dimensions (matrix)
  
      if (array->dimensions[0] != 3)
        throw_error_already_set(); // the 1D array does not have exactly 3 elements

      //double *values = reinterpret_cast<double*>(array->data);
      Scalar *values = reinterpret_cast<Scalar*>(array->data);
      
      return new Vector3x(values[0], values[1], values[2]);
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

      //double *values = reinterpret_cast<double*>(array->data);
      Scalar *values = reinterpret_cast<Scalar*>(array->data);
    
      void *storage = ((converter::rvalue_from_python_storage<Vector3x>*)data)->storage.bytes;
      new (storage) Vector3x(values[0], values[1], values[2]);
      data->convertible = storage;
    }
  };

  /***********************************************************************
   *
   * Transform3d
   *
   ***********************************************************************/
  
  struct Transform3d_to_python_array
  {
    struct innerclass
    {
      //
      //  Eigen::Transform3d --> python array (4x4)
      //
      static PyObject* convert(Eigen::Transform3d const &trans)
      {
        return incref(numeric::array(
              make_tuple(
                make_tuple(trans(0,0), trans(1,0), trans(2,0), trans(3,0)),
                make_tuple(trans(0,1), trans(1,1), trans(2,1), trans(3,1)),
                make_tuple(trans(0,2), trans(1,2), trans(2,2), trans(3,2)),
                make_tuple(trans(0,3), trans(1,3), trans(2,3), trans(3,3))
              )
            ).ptr());
      }
    };
   
    Transform3d_to_python_array()
    {
      to_python_converter<Eigen::Transform3d, innerclass>();
    }

  };

  struct Transform3d_from_python_array
  {
    Transform3d_from_python_array()
    {
      // Insert an rvalue from_python converter at the tail of the
      // chain. Used for implicit conversions
      //
      //  python array --> Eigen::Transform3d
      //
      // used for:
      //
      //  void function(Eigen::Transform3d vec)
      //  void function(Eigen::Transform3d & vec)
      //  void function(const Eigen::Transform3d & vec)
      //
      converter::registry::push_back( &convertible, &construct, type_id<Eigen::Transform3d>() );
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
      if (array->nd != 2)
        throw_error_already_set(); // the array has at least two dimensions (matrix)
      
      if ((array->dimensions[0] != 4) || (array->dimensions[1] != 4))
        throw_error_already_set(); // the 1D array does not have exactly 3 elements

      double *values = reinterpret_cast<double*>(array->data);
 
      void *storage = ((converter::rvalue_from_python_storage<Eigen::Transform3d>*)data)->storage.bytes;
      new (storage) Eigen::Transform3d();
      
      double *dataPtr = reinterpret_cast<double*>(storage);

      for (int i = 0; i < 16; ++i)
        dataPtr[i] = values[i];

      data->convertible = storage;
    }
  };


void export_Eigen()
{
  import_array(); // needed for NumPy 
  
  // Eigen::Vector3d
  Vector3x_to_python_array<Eigen::Vector3d>();
  Vector3x_from_python_array<Eigen::Vector3d>();
  // Eigen::Vector3f
  Vector3x_to_python_array<Eigen::Vector3f>();
  Vector3x_from_python_array<Eigen::Vector3f>();
  // Eigen::Vector3i
  Vector3x_to_python_array<Eigen::Vector3i>();
  Vector3x_from_python_array<Eigen::Vector3i>();

  // Eigen::Transform3d
  Transform3d_to_python_array();
  Transform3d_from_python_array();




}
