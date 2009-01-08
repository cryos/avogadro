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

#ifndef PYTHONINTERPRETER_H
#define PYTHONINTERPRETER_H

#include <avogadro/global.h>
#include <boost/python.hpp>
#include <avogadro/primitive.h>
#include <QString>

namespace Avogadro {

  class Molecule;

  /**
   * @author Donald Ephraim Curtis
   * @class PythonInterpreter pyhtoninterpreter.h <avogadro/pythoninterpreter.h>
   * @brief Class to embed the Python interpreter in Avogadro.
   *
   * This class is used to help embed the Python interpreter into C++ apps
   * which wish to expose parts of the Avogadro API.  Specifically instances
   * of Avogadro classes (Molecule / GLWidget).
   *
   * Currently all instances of this class share the same namespace.
   *
   */

  class PythonInterpreterPrivate;
  class A_EXPORT PythonInterpreter
  {
    public:
      /**
       * Constructor
       */
      PythonInterpreter();

      /**
       * Deconstructor
       */
      ~PythonInterpreter();

      /**
       * set the molecule object
       */
      void setMolecule(Molecule *molecule);

      void addSearchPath(const QString &path);

      /**
       * @param command string containing the python command to perform on the interpreter
       */
      QString exec(const QString &command);

      QString exec(const QString &command, boost::python::object local);

      QString eval(const QString &string, boost::python::object local);

    private:
      PythonInterpreterPrivate *const d;
      static int m_initCount;

      boost::python::object execWrapper(const QString &command, boost::python::object main, boost::python::object local);
      boost::python::object evalWrapper(const QString &string, boost::python::object main, boost::python::object local);
  };

}

#endif
