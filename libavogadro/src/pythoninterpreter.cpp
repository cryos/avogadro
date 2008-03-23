/**********************************************************************
  PythonInterpreter - Python Internal Interactive Interpreter

  Copyright (C) 2008 Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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
 **********************************************************************/

#include "pythoninterpreter.h"

#include <avogadro/boost.h>
#include <QString>
#include <QDebug>

namespace Avogadro {

  class PythonInterpreterPrivate {

    public:
      PythonInterpreterPrivate() : molecule(0)
    {
//      static bool initialized = false;

//      if(!initialized)
//      {
        std::cout << "initializing python interpreter\n";
        try {
          PyImport_AppendInittab( const_cast<char *>("Avogadro"), &initAvogadro );

          Py_Initialize();

//          object sys_module( handle<>(PyImport_ImportModule("sys")) );
//          object cStringIO_module( handle<>(PyImport_ImportModule("cStringIO")) );
//          main_module = handle<>(borrowed(PyImport_AddModule("__main__")));

          //        m_main_namespace = main_module.attr("__dict__");

          //        scope(avogadro_module).attr("widget") = ptr(widget);
        } catch( error_already_set ) {
          PyErr_Print();
        }
//        initialized = true;
//      }
    }

      dict local_namespace;
      Molecule *molecule;
  };

  PythonInterpreter::PythonInterpreter() : d(new PythonInterpreterPrivate)
  {
  }

  PythonInterpreter::~PythonInterpreter()
  {
    Py_Finalize();
  }

  void PythonInterpreter::setMolecule(Molecule *molecule)
  {
    d->molecule = molecule;

  }

  QString PythonInterpreter::run(const QString &command)
  {
    object main_module = object(( handle<>(borrowed(PyImport_AddModule("__main__")))));
    object main_namespace = main_module.attr("__dict__");

    object avogadro_module(handle<>(PyImport_ImportModule("Avogadro")) );
    if(d->molecule)
    {
//      d->local_namespace["Avogadro"] = ptr(d->molecule);
//      object avogadro_module( handle<>(PyImport_ImportModule("Avogadro")) );
//      scope(avogadro_module).attr("molecule") = ptr(molecule);
      avogadro_module.attr("molecule") = ptr(d->molecule);
    }
    d->local_namespace["Avogadro"] = avogadro_module;
    d->local_namespace["sys"] = object(handle<>(PyImport_ImportModule("sys")));
    d->local_namespace["cStringIO"] = object(handle<>(PyImport_ImportModule("cStringIO")));

    try
    {
      exec("CIO = cStringIO.StringIO()", main_namespace, d->local_namespace);
      exec("sys.stdout=CIO", main_namespace, d->local_namespace);
      exec("sys.stderr=CIO", main_namespace, d->local_namespace);
    }
    catch(error_already_set const &)
    {
      PyErr_Print();
    }

//    handle<> ignored(( PyRun_String( command.toAscii().constData(), Py_file_input, main_namespace.ptr(), main_namespace.ptr() ) ));
    try
    {
      object ignored = exec(command.toAscii().constData(), main_namespace, d->local_namespace);
    }
    catch(error_already_set const &)
    {
      PyErr_Print();
    }

    object result = eval("str(CIO.getvalue())", main_namespace, d->local_namespace);

    QString s;
    try
    {
      s = extract<const char *>(result);
    }
    catch(error_already_set const &)
    {
      return QString();
    }
//    return extract<>(result);
    return s;
  }
}

