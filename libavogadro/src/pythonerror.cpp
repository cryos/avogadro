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

namespace Avogadro
{

  PythonError* pythonError()
  {
    static PythonError *obj = 0;
    if (!obj)
      obj = new PythonError(0);
    return obj;
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
    pythonError()->append( QString(boost::python::extract<const char*>(err.attr("getvalue")())) );
  }

} // namespace

#include "pythonerror.moc"
