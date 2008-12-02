#include <Python.h>
#include <boost/python.hpp>

#include <QString>

using namespace boost::python;

struct QString_to_python_str
{

  // FIXME: handle this using boost::python::str
  static PyObject* convert(QString const &str)
  {
#if defined ( Py_UNICODE_WIDE )
    PyObject *obj = PyUnicode_FromUnicode( 0, str.length() );
    if (!obj)
      throw_error_already_set();

    Py_UNICODE *pyu = PyUnicode_AS_UNICODE( obj );

    for (int i = 0; i < str.length(); ++i)
      *pyu++ = (str.at(i)).unicode();

    return obj;
#else
   return PyUnicode_FromWideChar( (const wchar_t*) str.unicode(), str.length() );
#endif
  }
};

struct QString_from_python_str
{
  QString_from_python_str()
  {
    converter::registry::push_back( &convertible, &construct, type_id<QString>() );
  }

  static void* convertible(PyObject *obj_ptr)
  {
    if (PyUnicode_Check(obj_ptr))
      return obj_ptr;
    if (PyString_Check(obj_ptr))
      return obj_ptr;
      
    return 0;
  }

  static void construct(PyObject *obj_ptr, converter::rvalue_from_python_stage1_data *data)
  {
    if (PyUnicode_Check(obj_ptr)) {
      void *storage = ((converter::rvalue_from_python_storage<QString> *) data)->storage.bytes;
      PY_UNICODE_TYPE *ucode = PyUnicode_AS_UNICODE(obj_ptr);
      int len = PyUnicode_GET_SIZE(obj_ptr);
#if defined(Py_UNICODE_WIDE)
      QString* out = new(storage) QString;
      for ( int i = 0; i < len; ++i )
        out->append( (uint)ucode[i] );
#else
      new(storage) QString( (const QChar *)ucode, len );
#endif
      data->convertible = storage;
    } else if ( PyString_Check( obj_ptr ) ) {
      const char *value = PyString_AsString(obj_ptr);
      if(!value)
        throw_error_already_set();

      void *storage = ((converter::rvalue_from_python_storage<QString> *) data)->storage.bytes;
      new(storage) QString(QByteArray(value, PyString_Size(obj_ptr)));
      data->convertible = storage;
    }
  }

};

/*
class QStringTest 
{
  public:
    QString getString() const
    {
      return m_str;
    }
    void setString(const QString &str)
    {
      m_str = str;
    }
  private:
    QString m_str;
};
*/

void export_QString()
{
  to_python_converter<QString, QString_to_python_str>();
  QString_from_python_str();

  /*
  class_<QStringTest>("QStringTest")
    .def("getString", &QStringTest::getString)
    .def("setString", &QStringTest::setString)
    ;
  */
}
