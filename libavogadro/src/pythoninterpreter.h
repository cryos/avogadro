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


#ifndef __PYTHON_H
#define __PYTHON_H

#include <avogadro/boost.h>
#include <avogadro/primitive.h>
#include <QString>

namespace Avogadro {

  /**
   * @author Donald Ephraim Curtis
   * @class PythonInterpreter
   *
   * This class is used to help embed the Python interpreter into C++ apps
   * which wish to expose parts of the Avogadro API.  Specifically instances
   * of Avogadro classes (Molecule / GLWidget).
   *
   * Currently all instances of this class share the same namespace.
   *
   */

  class PythonInterpreterPrivate;
  class PythonInterpreter
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

      QString exec(const QString &command, object local);

      QString eval(const QString &string, object local);

    private:
      PythonInterpreterPrivate *const d;

      object execWrapper(const QString &command, object main, object local);
      object evalWrapper(const QString &string, object main, object local);
  };

}

#endif
