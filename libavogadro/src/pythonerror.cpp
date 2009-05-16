/**********************************************************************
  PythonError - Handle python errors

  Copyright (C) 2008,2009 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 ***********************************************************************/

#include <avogadro/pythonerror.h>

#include <boost/python.hpp>
#include <QStringList>

namespace Avogadro
{

  PythonError::PythonError() : QObject(), m_listening(false)
  {
  }

  PythonError* PythonError::instance()
  {
    static PythonError *obj = 0;
    if (!obj)
      obj = new PythonError;
    return obj;
  }

  void PythonError::append(const QString &str)
  {
    if (m_listening)
      emit message(str); // emit signal when other class is listening
    else
      m_str += str; // else, store the error        
  }
  
  QString& PythonError::string()
  {
    return m_str;
  }

  void PythonError::setListening(bool listening)
  {
    m_listening = listening;
  }

  class PythonThreadPrivate
  {
    public:
      PythonThreadPrivate() : gstate(PyGILState_Ensure())
      {
      }
      PyGILState_STATE gstate;
  };

  PythonThread::PythonThread() : d(new PythonThreadPrivate)
  {
  }

  PythonThread::~PythonThread()
  {
    PyGILState_Release(d->gstate);
    delete d;
  }
 
  bool initializePython(const QString &addToSearchPath)
  {
    Py_Initialize();

    static QStringList addedPaths = QStringList();

    if (Py_IsInitialized()) {
      using namespace boost::python;
      try {
        prepareToCatchError();
        object main_module = object(( handle<>(borrowed(PyImport_AddModule("__main__")))));
        object main_namespace = main_module.attr("__dict__");
        exec("import sys", main_namespace, main_namespace);
        foreach (const QString &path, addToSearchPath.split(';')) {
          if (addedPaths.contains(path))
            continue;
          addedPaths.append(path);
          QString exp("sys.path.insert(0,\"");
          exp.append(path);
          exp.append("\")");
          exec(exp.toAscii().data(), main_namespace, main_namespace);
        }
      } catch (const error_already_set &) {
        catchError();
      }
     
      return true;
    }

    return false;
  }

  void prepareToCatchError()
  {
    // make sure output is available to extract it in catch if the import fails
    PyRun_SimpleString("import cStringIO");
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.stderr = cStringIO.StringIO()");
  }

  void catchError()
  {
    PyErr_Print();
    // extract the error to a string
    boost::python::object sys = boost::python::import("sys");
    boost::python::object err = sys.attr("stderr");
    PythonError::instance()->append( QString(boost::python::extract<const char*>(err.attr("getvalue")())) );
  }

} // namespace

#include "pythonerror.moc"
