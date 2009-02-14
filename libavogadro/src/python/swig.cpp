#include <boost/python.hpp>

#include <openbabel/mol.h>
#include <avogadro/molecule.h>

using namespace boost::python;

#ifndef SWIGUNUSEDPARM
# ifdef __cplusplus
#   define SWIGUNUSEDPARM(p)
# else
#   define SWIGUNUSEDPARM(p) p SWIGUNUSED 
# endif
#endif

# define SWIG_BUFFER_SIZE 1024
#define SWIG_as_voidptr(a) const_cast< void * >(static_cast< const void * >(a)) 

/*
  Helper for static pointer initialization for both C and C++ code, for example
  static PyObject *SWIG_STATIC_POINTER(MyVar) = NewSomething(...);
*/
#ifdef __cplusplus
#define SWIG_STATIC_POINTER(var)  var
#else
#define SWIG_STATIC_POINTER(var)  var = 0; if (!var) var
#endif

/* Flags for new pointer objects */
#define SWIG_POINTER_OWN           0x1
#define SWIG_POINTER_NOSHADOW       (SWIG_POINTER_OWN      << 1)
#define SWIG_POINTER_NEW            (SWIG_POINTER_NOSHADOW | SWIG_POINTER_OWN)

typedef void *(*swig_converter_func)(void *, int *);
typedef struct swig_type_info *(*swig_dycast_func)(void **);

/* Structure to store information on one type */
typedef struct swig_type_info {
  const char             *name;			/* mangled name of this type */
  const char             *str;			/* human readable name of this type */
  swig_dycast_func        dcast;		/* dynamic cast function down a hierarchy */
  struct swig_cast_info  *cast;			/* linked list of types that can cast into this type */
  void                   *clientdata;		/* language specific type data */
  int                    owndata;		/* flag if the structure owns the clientdata */
} swig_type_info;

/* Structure to store a type and conversion function used for casting */
typedef struct swig_cast_info {
  swig_type_info         *type;			/* pointer to type that is equivalent to this type */
  swig_converter_func     converter;		/* function to cast the void pointers */
  struct swig_cast_info  *next;			/* pointer to next cast in linked list */
  struct swig_cast_info  *prev;			/* pointer to the previous cast */
} swig_cast_info;

/* PySwigObject */
typedef struct {
  PyObject_HEAD
  void *ptr;
  swig_type_info *ty;
  int own;
  PyObject *next;
} PySwigObject;

/* PySwigClientData */
typedef struct {
  PyObject *klass;
  PyObject *newraw;
  PyObject *newargs;
  PyObject *destroy;
  int delargs;
  int implicitconv;
} PySwigClientData;



static swig_type_info _swigt__p_OpenBabel__OBMol = {"_p_OpenBabel__OBMol", "OpenBabel::OBMol *", 0, 0, (void*)0, 0};

/* The python void return value */
PyObject* SWIG_Py_Void(void)
{
  PyObject *none = Py_None;
  Py_INCREF(none);
  return none;
}

PyObject* PySwigObject_long(PySwigObject *v)
{
  return PyLong_FromVoidPtr(v->ptr);
}

PyObject* PySwigObject_format(const char* fmt, PySwigObject *v)
{
  PyObject *res = NULL;
  PyObject *args = PyTuple_New(1);
  if (args) {
    if (PyTuple_SetItem(args, 0, PySwigObject_long(v)) == 0) {
      PyObject *ofmt = PyString_FromString(fmt);
      if (ofmt) {
	res = PyString_Format(ofmt,args);
	Py_DECREF(ofmt);
      }
      Py_DECREF(args);
    }
  }
  return res;
}

PyObject* PySwigObject_oct(PySwigObject *v)
{
  return PySwigObject_format("%o",v);
}

PyObject* PySwigObject_hex(PySwigObject *v)
{
  return PySwigObject_format("%x",v);
}

PyTypeObject* PySwigObject_type(void);

PyObject* PySwigObject_New(void *ptr, swig_type_info *ty, int own)
{
  PySwigObject *sobj = PyObject_NEW(PySwigObject, PySwigObject_type());
  if (sobj) {
    sobj->ptr  = ptr;
    sobj->ty   = ty;
    sobj->own  = own;
    sobj->next = 0;
  }
  return (PyObject *)sobj;
}

/*
  Return the pretty name associated with this type,
  that is an unmangled type name in a form presentable to the user.
*/
const char* SWIG_TypePrettyName(const swig_type_info *type) {
  /* The "str" field contains the equivalent pretty names of the
     type, separated by vertical-bar characters.  We choose
     to print the last name, as it is often (?) the most
     specific. */
  if (!type) return NULL;
  if (type->str != NULL) {
    const char *last_name = type->str;
    const char *s;
    for (s = type->str; *s; s++)
      if (*s == '|') last_name = s+1;
    return last_name;
  }
  else
    return type->name;
}

void PySwigObject_dealloc(PyObject *v)
{
  PySwigObject *sobj = (PySwigObject *) v;
  PyObject *next = sobj->next;
  if (sobj->own == SWIG_POINTER_OWN) {
    if (sobj->ptr)
      delete sobj->ptr;
    sobj->ptr = 0;
    /*
    swig_type_info *ty = sobj->ty;
    PySwigClientData *data = ty ? (PySwigClientData *) ty->clientdata : 0;
    PyObject *destroy = data ? data->destroy : 0;
    if (destroy) {
      // destroy is always a VARARGS method 
      PyObject *res;
      if (data->delargs) {
	// we need to create a temporal object to carry the destroy operation 
	PyObject *tmp = PySwigObject_New(sobj->ptr, ty, 0);
        res = PyObject_CallFunctionObjArgs(destroy, tmp, NULL);
	Py_DECREF(tmp);
      } else {
	PyCFunction meth = PyCFunction_GET_FUNCTION(destroy);
	PyObject *mself = PyCFunction_GET_SELF(destroy);
	res = ((*meth)(mself, v));
      }
      Py_XDECREF(res);
    }
#if !defined(SWIG_PYTHON_SILENT_MEMLEAK)
    else {
      const char *name = SWIG_TypePrettyName(ty);
      printf("swig/python detected a memory leak of type '%s', no destructor found.\n", (name ? name : "unknown"));
    }
#endif
    */
  } 
  Py_XDECREF(next);
  PyObject_DEL(v);
}

PyObject *
#ifdef METH_NOARGS
PySwigObject_repr(PySwigObject *v)
#else
PySwigObject_repr(PySwigObject *v, PyObject *args)
#endif
{
  const char *name = SWIG_TypePrettyName(v->ty);
  PyObject *hex = PySwigObject_hex(v);    
  PyObject *repr = PyString_FromFormat("<Swig Object of type '%s' at 0x%s>", name, PyString_AsString(hex));
  Py_DECREF(hex);
  if (v->next) {
#ifdef METH_NOARGS
    PyObject *nrep = PySwigObject_repr((PySwigObject *)v->next);
#else
    PyObject *nrep = PySwigObject_repr((PySwigObject *)v->next, args);
#endif
    PyString_ConcatAndDel(&repr,nrep);
  }
  return repr;  
}

/* 
   Pack binary data into a string
*/
char* SWIG_PackData(char *c, void *ptr, size_t sz) {
  static const char hex[17] = "0123456789abcdef";
  register const unsigned char *u = (unsigned char *) ptr;
  register const unsigned char *eu =  u + sz;
  for (; u != eu; ++u) {
    register unsigned char uu = *u;
    *(c++) = hex[(uu & 0xf0) >> 4];
    *(c++) = hex[uu & 0xf];
  }
  return c;
}

/* 
   Pack 'void *' into a string buffer.
*/
char* SWIG_PackVoidPtr(char *buff, void *ptr, const char *name, size_t bsz) {
  char *r = buff;
  if ((2*sizeof(void *) + 2) > bsz) return 0;
  *(r++) = '_';
  r = SWIG_PackData(r,&ptr,sizeof(void *));
  if (strlen(name) + 1 > (bsz - (r - buff))) return 0;
  strcpy(r,name);
  return buff;
}

PyObject* PySwigObject_str(PySwigObject *v)
{
  char result[SWIG_BUFFER_SIZE];
  return SWIG_PackVoidPtr(result, v->ptr, v->ty->name, sizeof(result)) ?
    PyString_FromString(result) : 0;
}

int PySwigObject_print(PySwigObject *v, FILE *fp, int SWIGUNUSEDPARM(flags))
{
#ifdef METH_NOARGS
  PyObject *repr = PySwigObject_repr(v);
#else
  PyObject *repr = PySwigObject_repr(v, NULL);
#endif
  if (repr) {
    fputs(PyString_AsString(repr), fp);
    Py_DECREF(repr);
    return 0; 
  } else {
    return 1; 
  }
}

int PySwigObject_compare(PySwigObject *v, PySwigObject *w)
{
  void *i = v->ptr;
  void *j = w->ptr;
  return (i < j) ? -1 : ((i > j) ? 1 : 0);
}

PyObject*
#ifdef METH_NOARGS
PySwigObject_disown(PyObject *v)
#else
PySwigObject_disown(PyObject* v, PyObject *SWIGUNUSEDPARM(args))
#endif
{
  PySwigObject *sobj = (PySwigObject *)v;
  sobj->own = 0;
  return SWIG_Py_Void();
}

PyObject*
#ifdef METH_NOARGS
PySwigObject_acquire(PyObject *v)
#else
PySwigObject_acquire(PyObject* v, PyObject *SWIGUNUSEDPARM(args))
#endif
{
  PySwigObject *sobj = (PySwigObject *)v;
  sobj->own = SWIG_POINTER_OWN;
  return SWIG_Py_Void();
}

PyObject* PySwigObject_own(PyObject *v, PyObject *args)
{
  PyObject *val = 0;
  if (!PyArg_UnpackTuple(args, (char *)"own", 0, 1, &val)) 
    {
      return NULL;
    } 
  else
    {
      PySwigObject *sobj = (PySwigObject *)v;
      PyObject *obj = PyBool_FromLong(sobj->own);
      if (val) {
#ifdef METH_NOARGS
	if (PyObject_IsTrue(val)) {
	  PySwigObject_acquire(v);
	} else {
	  PySwigObject_disown(v);
	}
#else
	if (PyObject_IsTrue(val)) {
	  PySwigObject_acquire(v,args);
	} else {
	  PySwigObject_disown(v,args);
	}
#endif
      } 
      return obj;
    }
}

int PySwigObject_Check(PyObject *op) 
{
  return ((op)->ob_type == PySwigObject_type()) || (strcmp((op)->ob_type->tp_name,"PySwigObject") == 0);
}

PyObject* PySwigObject_append(PyObject* v, PyObject* next)
{
  PySwigObject *sobj = (PySwigObject *) v;
#ifndef METH_O
  PyObject *tmp = 0;
  if (!PyArg_ParseTuple(next,(char *)"O:append", &tmp)) return NULL;
  next = tmp;
#endif
  if (!PySwigObject_Check(next)) {
    return NULL;
  }
  sobj->next = next;
  Py_INCREF(next);
  return SWIG_Py_Void();
}

PyObject* 
#ifdef METH_NOARGS
PySwigObject_next(PyObject* v)
#else
PySwigObject_next(PyObject* v, PyObject *SWIGUNUSEDPARM(args))
#endif
{
  PySwigObject *sobj = (PySwigObject *) v;
  if (sobj->next) {    
    Py_INCREF(sobj->next);
    return sobj->next;
  } else {
    return SWIG_Py_Void();
  }
}

#ifdef METH_O
static PyMethodDef
swigobject_methods[] = {
  {(char *)"disown",  (PyCFunction)PySwigObject_disown,  METH_NOARGS,  (char *)"releases ownership of the pointer"},
  {(char *)"acquire", (PyCFunction)PySwigObject_acquire, METH_NOARGS,  (char *)"aquires ownership of the pointer"},
  {(char *)"own",     (PyCFunction)PySwigObject_own,     METH_VARARGS, (char *)"returns/sets ownership of the pointer"},
  {(char *)"append",  (PyCFunction)PySwigObject_append,  METH_O,       (char *)"appends another 'this' object"},
  {(char *)"next",    (PyCFunction)PySwigObject_next,    METH_NOARGS,  (char *)"returns the next 'this' object"},
  {(char *)"__repr__",(PyCFunction)PySwigObject_repr,    METH_NOARGS,  (char *)"returns object representation"},
  {0, 0, 0, 0}  
};
#else
static PyMethodDef
swigobject_methods[] = {
  {(char *)"disown",  (PyCFunction)PySwigObject_disown,  METH_VARARGS,  (char *)"releases ownership of the pointer"},
  {(char *)"acquire", (PyCFunction)PySwigObject_acquire, METH_VARARGS,  (char *)"aquires ownership of the pointer"},
  {(char *)"own",     (PyCFunction)PySwigObject_own,     METH_VARARGS,  (char *)"returns/sets ownership of the pointer"},
  {(char *)"append",  (PyCFunction)PySwigObject_append,  METH_VARARGS,  (char *)"appends another 'this' object"},
  {(char *)"next",    (PyCFunction)PySwigObject_next,    METH_VARARGS,  (char *)"returns the next 'this' object"},
  {(char *)"__repr__",(PyCFunction)PySwigObject_repr,   METH_VARARGS,  (char *)"returns object representation"},
  {0, 0, 0, 0}  
};
#endif

PyTypeObject* _PySwigObject_type(void) 
{
  static char swigobject_doc[] = "Swig object carries a C/C++ instance pointer";
  
  static PyNumberMethods PySwigObject_as_number = {
    (binaryfunc)0, /*nb_add*/
    (binaryfunc)0, /*nb_subtract*/
    (binaryfunc)0, /*nb_multiply*/
    (binaryfunc)0, /*nb_divide*/
    (binaryfunc)0, /*nb_remainder*/
    (binaryfunc)0, /*nb_divmod*/
    (ternaryfunc)0,/*nb_power*/
    (unaryfunc)0,  /*nb_negative*/
    (unaryfunc)0,  /*nb_positive*/
    (unaryfunc)0,  /*nb_absolute*/
    (inquiry)0,    /*nb_nonzero*/
    0,		   /*nb_invert*/
    0,		   /*nb_lshift*/
    0,		   /*nb_rshift*/
    0,		   /*nb_and*/
    0,		   /*nb_xor*/
    0,		   /*nb_or*/
    (coercion)0,   /*nb_coerce*/
    (unaryfunc)PySwigObject_long, /*nb_int*/
    (unaryfunc)PySwigObject_long, /*nb_long*/
    (unaryfunc)0,                 /*nb_float*/
    (unaryfunc)PySwigObject_oct,  /*nb_oct*/
    (unaryfunc)PySwigObject_hex,  /*nb_hex*/
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 /* nb_inplace_add -> nb_index */
  };

  static PyTypeObject pyswigobject_type;  
  static int type_init = 0;
  if (!type_init) {
    const PyTypeObject tmp
      = {
	PyObject_HEAD_INIT(NULL)
	0,				    /* ob_size */
	(char *)"PySwigObject",		    /* tp_name */
	sizeof(PySwigObject),		    /* tp_basicsize */
	0,			            /* tp_itemsize */
	(destructor)PySwigObject_dealloc,   /* tp_dealloc */
	(printfunc)PySwigObject_print,	    /* tp_print */
	(getattrfunc)0,			    /* tp_getattr */ 
	(setattrfunc)0,			    /* tp_setattr */ 
	(cmpfunc)PySwigObject_compare,	    /* tp_compare */ 
	(reprfunc)PySwigObject_repr,	    /* tp_repr */    
	&PySwigObject_as_number,	    /* tp_as_number */
	0,				    /* tp_as_sequence */
	0,				    /* tp_as_mapping */
	(hashfunc)0,			    /* tp_hash */
	(ternaryfunc)0,			    /* tp_call */
	(reprfunc)PySwigObject_str,	    /* tp_str */
	PyObject_GenericGetAttr,            /* tp_getattro */
	0,				    /* tp_setattro */
	0,		                    /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,	            /* tp_flags */
	swigobject_doc, 	            /* tp_doc */        
	0,                                  /* tp_traverse */
	0,                                  /* tp_clear */
	0,                                  /* tp_richcompare */
	0,                                  /* tp_weaklistoffset */
	0,                                  /* tp_iter */
	0,                                  /* tp_iternext */
	swigobject_methods,		    /* tp_methods */ 
	0,			            /* tp_members */
	0,				    /* tp_getset */	    	
	0,			            /* tp_base */	        
	0,				    /* tp_dict */	    	
	0,				    /* tp_descr_get */  	
	0,				    /* tp_descr_set */  	
	0,				    /* tp_dictoffset */ 	
	0,				    /* tp_init */	    	
	0,				    /* tp_alloc */	    	
	0,			            /* tp_new */	    	
	0,	                            /* tp_free */	   
        0,                                  /* tp_is_gc */  
	0,				    /* tp_bases */   
	0,				    /* tp_mro */
	0,				    /* tp_cache */   
 	0,				    /* tp_subclasses */
	0,				    /* tp_weaklist */
	0,                                  /* tp_del */
#ifdef COUNT_ALLOCS
	0,0,0,0                             /* tp_alloc -> tp_next */
#endif
      };
    pyswigobject_type = tmp;
    pyswigobject_type.ob_type = &PyType_Type;
    type_init = 1;
  }
  return &pyswigobject_type;
}

PyTypeObject* PySwigObject_type(void) 
{
  static PyTypeObject *SWIG_STATIC_POINTER(type) = _PySwigObject_type();
  return type;
}

PyObject* _SWIG_This(void)
{
  return PyString_FromString("this");
}

PyObject* SWIG_This(void)
{
  static PyObject *SWIG_STATIC_POINTER(swig_this) = _SWIG_This();
  return swig_this;
}


/*
  Create a new instance object, whitout calling __init__, and set the
  'this' attribute.
*/
PyObject* SWIG_Python_NewShadowInstance(PySwigClientData *data, PyObject *swig_this)
{
  PyObject *inst = 0;
  PyObject *newraw = data->newraw;
  if (newraw) {
    inst = PyObject_Call(newraw, data->newargs, NULL);
    if (inst) {
      PyObject *key = SWIG_This();
      PyObject_SetAttr(inst, key, swig_this);
    }
  } else {
    PyObject *dict = PyDict_New();
    PyDict_SetItem(dict, SWIG_This(), swig_this);
    inst = PyInstance_NewRaw(data->newargs, dict);
    Py_DECREF(dict);
  }
  return inst;
}


PyObject* SWIG_Python_NewPointerObj(void *ptr, swig_type_info *type, int flags) 
{
  if (!ptr) {
    return SWIG_Py_Void();
  } else {
    int own = (flags & SWIG_POINTER_OWN) ? SWIG_POINTER_OWN : 0;
    PyObject *robj = PySwigObject_New(ptr, type, own);
    PySwigClientData *clientdata = type ? (PySwigClientData *)(type->clientdata) : 0;
    if (clientdata && !(flags & SWIG_POINTER_NOSHADOW)) {
      PyObject *inst = SWIG_Python_NewShadowInstance(clientdata, robj);
      if (inst) {
	Py_DECREF(robj);
	robj = inst;
      }
    }
    return robj;
  }
}

PyObject* Molecule_OBMol(Avogadro::Molecule &self)
{
  PyObject *obj = 0;
  OpenBabel::OBMol *mol = 0 ;
  
  mol = (OpenBabel::OBMol *)new OpenBabel::OBMol(self.OBMol());
  obj = SWIG_Python_NewPointerObj(SWIG_as_voidptr(mol), &_swigt__p_OpenBabel__OBMol, SWIG_POINTER_NEW |  0 );
  return obj;
}

void Molecule_setOBMol(Avogadro::Molecule &self, PyObject *obj)
{
  if (!PyObject_HasAttrString(obj, "this"))
    throw_error_already_set();

  PyObject *thisAttr = PyObject_GetAttrString(obj, "this");
  if (!thisAttr)
    throw_error_already_set();

  OpenBabel::OBMol *mol = ((PySwigObject*)thisAttr)->ptr;
  self.setOBMol(mol);
}
