
#include <Python.h>
#include <numpy/arrayobject.h> 
#include <boost/python.hpp>
#include <boost/python/tuple.hpp>

#include <avogadro/global.h>
#include <Eigen/Geometry>

#include <iostream>

using namespace boost::python;

template <typename Scalar> struct ScalarTraits;
template <> struct ScalarTraits<int>
{
  enum { isInt = 1, isFloat = 0, isDouble = 0 };
};
template <> struct ScalarTraits<float>
{
  enum { isInt = 0, isFloat = 1, isDouble = 0 };
};
template <> struct ScalarTraits<double>
{
  enum { isInt = 0, isFloat = 0, isDouble = 1 };
};


  /***********************************************************************
   *
   * Vector3x = Vector3d, Vector3f, Vector3i
   *
   ***********************************************************************/
  
  template <class Vector3x>
  struct Vector3x_to_python_array
  {
    typedef typename Vector3x::Scalar Scalar;
    
    struct innerclass
    {
      //
      //  Eigen::Vector3x --> python array
      //
      static PyObject* convert(Vector3x const &vec)
      {
        int dims[1] = { 3 };
        PyObject *result;
        if (ScalarTraits<Scalar>::isInt)
          result = PyArray_FromDims(1, dims, PyArray_INT);
        else if (ScalarTraits<Scalar>::isFloat)
          result = PyArray_FromDims(1, dims, PyArray_FLOAT);
        else
          result = PyArray_FromDims(1, dims, PyArray_DOUBLE);
        
        // copy the data
        Scalar *data = (Scalar*) reinterpret_cast<PyArrayObject*>(result)->data;
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
        PyObject *result;
        if (ScalarTraits<Scalar>::isInt)
          result = PyArray_FromDims(1, dims, PyArray_INT);
        else if (ScalarTraits<Scalar>::isFloat)
          result = PyArray_FromDims(1, dims, PyArray_FLOAT);
        else
          result = PyArray_FromDims(1, dims, PyArray_DOUBLE);
        
        // copy the data
        Scalar *data = (Scalar*) reinterpret_cast<PyArrayObject*>(result)->data;
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
        PyObject *result;
        if (ScalarTraits<Scalar>::isInt)
          result = PyArray_FromDims(1, dims, PyArray_INT);
        else if (ScalarTraits<Scalar>::isFloat)
          result = PyArray_FromDims(1, dims, PyArray_FLOAT);
        else
          result = PyArray_FromDims(1, dims, PyArray_DOUBLE);
        
        // copy the data
        Scalar *data = (Scalar*) reinterpret_cast<PyArrayObject*>(result)->data;
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
      if (!PyArray_Check(obj_ptr))
        throw_error_already_set();

      // only accept int, long, float and double
      switch (PyArray_ObjectType(obj_ptr, 0)) {
        case NPY_INT:
        case NPY_LONG:
        case NPY_FLOAT:
        case NPY_DOUBLE:
          break;
        default:
          return 0;
      }

      // do some type checking
      if ((PyArray_ObjectType(obj_ptr, 0) == NPY_FLOAT) || (PyArray_ObjectType(obj_ptr, 0) == NPY_DOUBLE))
        if (ScalarTraits<Scalar>::isInt)
          return 0;

      if ((PyArray_ObjectType(obj_ptr, 0) == NPY_INT) || (PyArray_ObjectType(obj_ptr, 0) == NPY_LONG))
        if (ScalarTraits<Scalar>::isFloat || ScalarTraits<Scalar>::isDouble)
          return 0;
      
      PyArrayObject *array = reinterpret_cast<PyArrayObject*>(obj_ptr);

      // check the dimensions
      if (array->nd != 1)
        throw_error_already_set(); // the array has at least two dimensions (matrix)
  
      if (array->dimensions[0] != 3)
        throw_error_already_set(); // the 1D array does not have exactly 3 elements

      switch (PyArray_ObjectType(obj_ptr, 0)) {
        case NPY_INT:
          {
            int *values = reinterpret_cast<int*>(array->data);
            return new Vector3x(values[0], values[1], values[2]);
          }
        case NPY_LONG:
          {
            long *values = reinterpret_cast<long*>(array->data);
            return new Vector3x(values[0], values[1], values[2]);      
          }
        case NPY_FLOAT:
          {
            float *values = reinterpret_cast<float*>(array->data);
            return new Vector3x(values[0], values[1], values[2]);
          }
        case NPY_DOUBLE:
          {
            double *values = reinterpret_cast<double*>(array->data);
            return new Vector3x(values[0], values[1], values[2]);
          }
          break;
        default:
          return 0;
      }

    }
 
    static void* convertible(PyObject *obj_ptr)
    {
      if (!PyArray_Check(obj_ptr))
        return 0;

      // only accept int, long, float and double
      switch (PyArray_ObjectType(obj_ptr, 0)) {
        case NPY_INT:
        case NPY_LONG:
        case NPY_FLOAT:
        case NPY_DOUBLE:
          break;
        default:
          return 0;
      }
      
      // do some type checking
      if ((PyArray_ObjectType(obj_ptr, 0) == NPY_FLOAT) || (PyArray_ObjectType(obj_ptr, 0) == NPY_DOUBLE))
        if (ScalarTraits<Scalar>::isInt)
          return 0;

      if ((PyArray_ObjectType(obj_ptr, 0) == NPY_INT) || (PyArray_ObjectType(obj_ptr, 0) == NPY_LONG))
        if (ScalarTraits<Scalar>::isFloat || ScalarTraits<Scalar>::isDouble)
          return 0;
      
      PyArrayObject *array = reinterpret_cast<PyArrayObject*>(obj_ptr);

      // check the dimensions
      if (array->nd != 1)
        return 0; // the array has at least two dimensions (matrix)
  
      if (array->dimensions[0] != 3)
        return 0; // the 1D array does not have exactly 3 elements
 
      return obj_ptr;
    }

    static void construct(PyObject *obj_ptr, converter::rvalue_from_python_stage1_data *data)
    {
      PyArrayObject *array = reinterpret_cast<PyArrayObject*>(obj_ptr);
      void *storage = ((converter::rvalue_from_python_storage<Vector3x>*)data)->storage.bytes;

      switch (PyArray_ObjectType(obj_ptr, 0)) {
        case NPY_INT:
          {
            int *values = reinterpret_cast<int*>(array->data);
            new (storage) Vector3x(values[0], values[1], values[2]);
          }
          break;
        case NPY_LONG:
          {
            long *values = reinterpret_cast<long*>(array->data);
            new (storage) Vector3x(values[0], values[1], values[2]);
          }
          break;
        case NPY_FLOAT:
          {
            float *values = reinterpret_cast<float*>(array->data);
            new (storage) Vector3x(values[0], values[1], values[2]);
          }
          break;
        case NPY_DOUBLE:
          {
            double *values = reinterpret_cast<double*>(array->data);
            new (storage) Vector3x(values[0], values[1], values[2]);
          }
          break;
        default:
          return;
      }

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
        int dims[2] = { 4, 4 };
        PyObject *result = PyArray_FromDims(2, dims, PyArray_DOUBLE);
        
        // copy the data
        double *data = (double*) reinterpret_cast<PyArrayObject*>(result)->data;
        const double *dataPtr = trans.matrix().data();
        for (int i = 0; i < 16; ++i)
          data[i] = dataPtr[i];
 
        return incref(result);
      }
      //
      //  Eigen::Transform3d* --> python array (4x4)
      //
      static PyObject* convert(Eigen::Transform3d *trans)
      {
        int dims[2] = { 4, 4 };
        PyObject *result = PyArray_FromDims(2, dims, PyArray_DOUBLE);
        
        // copy the data
        double *data = (double*) reinterpret_cast<PyArrayObject*>(result)->data;
        double *dataPtr = trans->matrix().data();
        for (int i = 0; i < 16; ++i)
          data[i] = dataPtr[i];
 
        return incref(result);
      }
      //
      //  const Eigen::Transform3d* --> python array (4x4)
      //
      static PyObject* convert(const Eigen::Transform3d *trans)
      {
        int dims[2] = { 4, 4 };
        PyObject *result = PyArray_FromDims(2, dims, PyArray_DOUBLE);
        
        // copy the data
        double *data = (double*) reinterpret_cast<PyArrayObject*>(result)->data;
        const double *dataPtr = trans->matrix().data();
        for (int i = 0; i < 16; ++i)
          data[i] = dataPtr[i];
 
        return incref(result);
      }
 
    };
   
    Transform3d_to_python_array()
    {
      #ifndef WIN32
      to_python_converter<Eigen::Transform3d, innerclass>();
      #endif
      to_python_converter<Eigen::Transform3d*, innerclass>();
      to_python_converter<const Eigen::Transform3d*, innerclass>();
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
      
      converter::registry::insert( &convert, type_id<Eigen::Transform3d>() );
    }

    static void* convert(PyObject *obj_ptr)
    {
      if (!PyArray_Check(obj_ptr))
        throw_error_already_set();

      PyArrayObject *array = reinterpret_cast<PyArrayObject*>(obj_ptr);

      // check the dimensions
      if (array->nd != 2)
        throw_error_already_set(); // the array has at least two dimensions (matrix)
      
      if ((array->dimensions[0] != 4) || (array->dimensions[1] != 4))
        throw_error_already_set(); // the 1D array does not have exactly 3 elements

      double *values = reinterpret_cast<double*>(array->data);
      Eigen::Transform3d *c_obj = new Eigen::Transform3d();
      double *dataPtr = c_obj->data();

      for (int i = 0; i < 16; ++i)
        dataPtr[i] = values[i];

      return c_obj;
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

      // I think this is a better way to get at the double array, where is this
      // deleted though? Does Boost::Python do it?
      double *values = reinterpret_cast<double*>(array->data);
      Eigen::Transform3d *storage = new Eigen::Transform3d();
      double *dataPtr = storage->data();

      for (int i = 0; i < 16; ++i)
        dataPtr[i] = values[i];

      data->convertible = storage;
    }
  };

/* 
 * used for unittest to test all get/set options with Eigen classes
 */
  
#ifndef WIN32
class EigenUnitTestHelper
{
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    EigenUnitTestHelper() : m_vector3d(Eigen::Vector3d(1., 2., 3.))
    {
    }
    //Eigen::Vector3d             vector3d()              { return m_vector3d; }
    //Eigen::Vector3d&            vector3d_ref()          { return m_vector3d; }
    const Eigen::Vector3d&      const_vector3d_ref()    { return m_vector3d; }
    Eigen::Vector3d*            vector3d_ptr()          { return &m_vector3d; }
    const Eigen::Vector3d*      const_vector3d_ptr()    { return &m_vector3d; }

    //void set_vector3d(Eigen::Vector3d vec)                      { m_vector3d = vec; }
    //void set_vector3d_ref(Eigen::Vector3d& vec)                 { m_vector3d = vec; }
    void set_const_vector3d_ref(const Eigen::Vector3d& vec)     { m_vector3d = vec; }
    void set_vector3d_ptr(Eigen::Vector3d* vec)                 { m_vector3d = *vec; }
    void set_const_vector3d_ptr(const Eigen::Vector3d* const vec) { m_vector3d = *vec; }

    //Eigen::Transform3d             transform3d()              { return m_transform3d; }
    //Eigen::Transform3d&            transform3d_ref()          { return m_transform3d; }
    const Eigen::Transform3d&      const_transform3d_ref()    { return m_transform3d; }
    Eigen::Transform3d*            transform3d_ptr()          { return &m_transform3d; }
    const Eigen::Transform3d*      const_transform3d_ptr()    { return &m_transform3d; }

    //void set_transform3d(Eigen::Transform3d vec)                      { m_transform3d = vec; }
    //void set_transform3d_ref(Eigen::Transform3d& vec)                 { m_transform3d = vec; }
    void set_const_transform3d_ref(const Eigen::Transform3d& vec)     { m_transform3d = vec; }
    void set_transform3d_ptr(Eigen::Transform3d* vec)                 { m_transform3d = *vec; }
    void set_const_transform3d_ptr(const Eigen::Transform3d* const vec) { m_transform3d = *vec; }
 
  private:
    Eigen::Vector3d m_vector3d;
    Eigen::Transform3d m_transform3d;

};
#endif

void export_Eigen()
{
  import_array(); // needed for NumPy 

#ifndef WIN32
  class_<EigenUnitTestHelper>("EigenUnitTestHelper")
   //.def("vector3d", &EigenUnitTestHelper::vector3d)
   //.def("vector3d_ref", &EigenUnitTestHelper::vector3d_ref, return_value_policy<return_by_value>())
   .def("const_vector3d_ref", &EigenUnitTestHelper::const_vector3d_ref, return_value_policy<return_by_value>())
   .def("vector3d_ptr", &EigenUnitTestHelper::vector3d_ptr, return_value_policy<return_by_value>())
   .def("const_vector3d_ptr", &EigenUnitTestHelper::const_vector3d_ptr, return_value_policy<return_by_value>())

   //.def("set_vector3d", &EigenUnitTestHelper::set_vector3d)
   //.def("set_vector3d_ref", &EigenUnitTestHelper::set_vector3d_ref)
   .def("set_const_vector3d_ref", &EigenUnitTestHelper::set_const_vector3d_ref)
   .def("set_vector3d_ptr", &EigenUnitTestHelper::set_vector3d_ptr)
   .def("set_const_vector3d_ptr", &EigenUnitTestHelper::set_const_vector3d_ptr)
 
   //.def("transform3d", &EigenUnitTestHelper::transform3d)
   //.def("transform3d_ref", &EigenUnitTestHelper::transform3d_ref, return_value_policy<return_by_value>())
   .def("const_transform3d_ref", &EigenUnitTestHelper::const_transform3d_ref, return_value_policy<return_by_value>())
   .def("transform3d_ptr", &EigenUnitTestHelper::transform3d_ptr, return_value_policy<return_by_value>())
   .def("const_transform3d_ptr", &EigenUnitTestHelper::const_transform3d_ptr, return_value_policy<return_by_value>())

   //.def("set_transform3d", &EigenUnitTestHelper::set_transform3d)
   //.def("set_transform3d_ref", &EigenUnitTestHelper::set_transform3d_ref)
   .def("set_const_transform3d_ref", &EigenUnitTestHelper::set_const_transform3d_ref)
   .def("set_transform3d_ptr", &EigenUnitTestHelper::set_transform3d_ptr)
   .def("set_const_transform3d_ptr", &EigenUnitTestHelper::set_const_transform3d_ptr)
 
   ; 
#endif
  
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
