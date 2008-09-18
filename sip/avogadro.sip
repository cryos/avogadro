%Module AvoSIP 0

%ModuleHeaderCode
#include <python2.5/Python.h>

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
%End



%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtOpenGL/QtOpenGLmod.sip


%ModuleCode


struct PySwigObject {
  PyObject_HEAD
  void * ptr;
  const char * desc;
};

PyObject* swig2sip(PyObject *swigObject)
{
  if (!PyObject_HasAttrString(swigObject, "this"))
    return Py_None;

  PyObject *thisAttr = PyObject_GetAttrString(swigObject, "this");
  if (!thisAttr)
    return Py_None;

  const char *repr = PyString_AsString(PyObject_Repr(thisAttr));
  std::string s(repr);
  unsigned int begin = s.find_first_of("'");
  unsigned int  end = s.find_last_of("'");
  std::string typeStr = s.substr(begin+1, end-begin-3);
  printf("thisAttr = %s\n", typeStr.c_str());

  void *ptr = reinterpret_cast<PySwigObject*>(thisAttr)->ptr;
  if (!ptr)
    return Py_None;

  sipWrapperType *type = sipFindClass(typeStr.c_str());
  if (!type)
    return Py_None;

  PyObject *obj = sipConvertFromInstance(ptr, type, 0);
  if (!obj)
    return Py_None;

  return obj;
}

bool readFile(const char *filename, PyObject *molecule)
{
  printf("readFile\n");
  sipWrapperType *type = sipFindClass("OpenBabel::OBMol");
  
  if (!sipCanConvertToInstance(molecule, type, SIP_NOT_NONE)) {
    printf("Error in readFile: sipCanConvertToInstance() failed.\n");
    return false;
  }

  int iserr = 0;
  OpenBabel::OBMol *mol = static_cast<OpenBabel::OBMol*>(
      sipConvertToInstance(molecule, type, 0, SIP_NOT_NONE, 0, &iserr));
  if (iserr) {
    printf("Error in readFile: sipConvertToInstance() failed.\n");
    return false;
  }  

  OpenBabel::OBConversion conv;
  OpenBabel::OBFormat *format = conv.FormatFromExt(filename);
  if (!format || !conv.SetInFormat(format)) {
    printf("Error in readFile: cannot find file format.\n");
    return false;
  }

  return conv.ReadFile(mol, filename);
}

%End

SIP_PYOBJECT swig2sip(SIP_PYOBJECT swigObject);
bool readFile(const char *filename, SIP_PYOBJECT molecule);



%Include eigen.sip
%Include openbabel.sip

%If (Qt_4_2_2 -)

%Include primitive.sip
%Include primitivelist.sip
%Include plugin.sip
%Include pluginmanager.sip
%Include tool.sip
%Include toolgroup.sip
%Include color.sip
%Include camera.sip
%Include painter.sip
%Include painterdevice.sip
%Include engine.sip
%Include glwidget.sip


%End
