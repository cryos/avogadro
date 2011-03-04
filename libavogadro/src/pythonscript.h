/**********************************************************************
  Python - Gives us some Python helper stuff

  Copyright (C) 2008 by Donald Ephraim Curtis
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

#ifndef PYTHONSCRIPT_H
#define PYTHONSCRIPT_H

#include <avogadro/global.h>
#include <boost/python.hpp>

#include "pythonerror.h"

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
      PythonScript(const QString &fileName);
      ~PythonScript();

      /**
       * Get the module name.
       */
      QString moduleName() const;
      /**
       * Get the python module object.
       */
      boost::python::object module() const;
      /**
       * Get a QFile object for the script file.
       */
      const QString& fileName() const;
      /**
       * Get the identifier for this script. This is the Sha1 hash of the 
       * script's contents.
       */
      QString identifier() const;

      /**
       * Returns true is script module has giveen attribute string
       */
      bool hasAttrString(const char *attrString) const;


    private:
      QString m_moduleName;
      QString m_fileName;
      mutable QDateTime m_lastModified;
      mutable boost::python::object m_module;
  };

} // end namespace Avogadro

#endif
