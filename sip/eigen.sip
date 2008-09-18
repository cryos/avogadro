////////////////////////////////////////////////////////////
//
// Eigen::Vector3d
//
////////////////////////////////////////////////////////////

%MappedType Eigen::Vector3d
{
%TypeHeaderCode
#include <Numeric/arrayobject.h>
#include <Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN
%End

%ConvertFromTypeCode 
  import_array(); // needed for Numeric/Array  
  
  int dims[1] = { 3 };
  PyObject *result = PyArray_FromDims(1, dims, PyArray_DOUBLE);

  // copy the data
  double *data = (double*) reinterpret_cast<PyArrayObject*>(result)->data;
  data[0] = sipCpp->x();
  data[1] = sipCpp->y();
  data[2] = sipCpp->z();

  return result;
%End

%ConvertToTypeCode
  import_array(); // needed for Numeric/Array

  // check the type if that is all that is required
  if (sipIsErr == NULL)
    return PyArray_Check(sipPy);

  PyArrayObject *array = reinterpret_cast<PyArrayObject*>(sipPy);

  // check the dimensions
  if (array->nd != 1) 
  {
    *sipIsErr = 1;
    return 0; // the array has at least two dimensions (matrix)
  }
  if (array->dimensions[0] != 3)
  {
    *sipIsErr = 1;
    return 0; // the 1D array does not have exactly 3 elements
  }

  double *data = reinterpret_cast<double*>(array->data);
  Vector3d *vec = new Vector3d(data[0], data[1], data[2]);

  // return the instance on the heap
  *sipCppPtr = vec;

  // apply normal transfer
  return sipGetState(sipTransferObj);
%End
};

////////////////////////////////////////////////////////////
//
// Eigen::Transform3d
//
////////////////////////////////////////////////////////////

%MappedType Eigen::Transform3d
{
%TypeHeaderCode
#include <Numeric/arrayobject.h>
#include <Eigen/Geometry>
USING_PART_OF_NAMESPACE_EIGEN
%End

%ConvertFromTypeCode 
  import_array(); // needed for Numeric/Array  
  
  int dims[2] = { 4, 4 };
  PyObject *result = PyArray_FromDims(2, dims, PyArray_DOUBLE);

  // copy the data
  double *data_to = (double*) reinterpret_cast<PyArrayObject*>(result)->data;
  double *data_from = sipCpp->data();
  for (int i = 0; i < 16; ++i)
    data_to[i] = data_from[i];

  return result;
%End

%ConvertToTypeCode
  import_array(); // needed for Numeric/Array

  // check the type if that is all that is required
  if (sipIsErr == NULL)
    return PyArray_Check(sipPy);

  PyArrayObject *array = reinterpret_cast<PyArrayObject*>(sipPy);

  // check the dimensions
  if (array->nd != 2) 
  {
    *sipIsErr = 1;
    return 0; // the array does nothave two dimensions
  }
  if (array->dimensions[0] != 4 || array->dimensions[1] != 4)
  {
    *sipIsErr = 1;
    return 0; // the array isn't a 4x4 matrix
  }

  Eigen::Transform3d *mat = new Eigen::Transform3d();
  double *data_to = mat->data();
  double *data_from = reinterpret_cast<double*>(array->data);
  for (int i = 0; i < 16; ++i)
    data_to[i] = data_from[i];

  // return the instance on the heap
  *sipCppPtr = mat;

  // apply normal transfer
  return sipGetState(sipTransferObj);
%End
};


////////////////////////////////////////////////////////////
//
// QVector<Eigen::Vector3d>
//
////////////////////////////////////////////////////////////

%MappedType QVector<Eigen::Vector3d>
{
%TypeHeaderCode
#include <Numeric/arrayobject.h>
#include <qvector.h>
#include <Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN
%End

%ConvertFromTypeCode 
  import_array();  
  // create the list
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;
  
  int dims[1] = { 3 };

  // set the list elements
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    PyObject *pObj;

    if ((pObj = PyArray_FromDims(1, dims, PyArray_DOUBLE)) == NULL)
    {
      Py_DECREF(l);
      return NULL;
    }

    // copy the data
    double *data = (double*) reinterpret_cast<PyArrayObject*>(pObj)->data;
    data[0] = sipCpp->at(i).x();
    data[1] = sipCpp->at(i).y();
    data[2] = sipCpp->at(i).z();

    PyList_SET_ITEM(l, i, pObj);
  }

  return l;
%End

%ConvertToTypeCode
  import_array();  
  // check the type if that is all that is required
  if (sipIsErr == NULL)
    return PyList_Check(sipPy);

  QVector<Eigen::Vector3d> *qv = new QVector<Eigen::Vector3d>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i) 
  {
    PyArrayObject *array = reinterpret_cast<PyArrayObject*>(PyList_GET_ITEM(sipPy, i));
    double *data = reinterpret_cast<double*>(array->data);
    Vector3d vec(data[0], data[1], data[2]);
    qv->append(vec);    
  }

  *sipCppPtr = qv;

  return sipGetState(sipTransferObj);
%End
};





