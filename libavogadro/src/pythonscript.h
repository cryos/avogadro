/**********************************************************************
  Python - Gives us some Python helper stuff

  Copyright (C) 2008 by Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef PYTHONSCRIPT_H
#define PYTHONSCRIPT_H

#include <avogadro/global.h>
#include <boost/python.hpp>

#include <QList>
#include <QDir>
#include <QString>
#include <QDateTime>
#include <QFileInfo>
#include <QTextEdit>

namespace Avogadro {

  class A_EXPORT PythonScript
  {

    public:
      PythonScript(QDir dir, QString fileName);
      ~PythonScript();

      QString moduleName() const;
      boost::python::object module() const;

    private:
      QString m_moduleName;
      QString m_fileName;
      QDir m_dir;
      mutable QDateTime m_lastModified;
      mutable boost::python::object m_module;
  };

  
  inline void prepareToCatchError()
  {
    // make sure output is available to extract it in catch if the import fails
    PyRun_SimpleString("import cStringIO");
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.stderr = cStringIO.StringIO()");
  }

  inline const char* catchError()
  {
    PyErr_Print();
    // extract the error to a string
    boost::python::object sys = boost::python::import("sys");
    boost::python::object err = sys.attr("stderr");
    return boost::python::extract<const char*>(err.attr("getvalue")());
  }

  

} // end namespace Avogadro

#endif
