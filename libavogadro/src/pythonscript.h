/**********************************************************************
  Python - Gives us some Python helper stuff

  Copyright (C) 2008 by Donald Ephraim Curtis
  Copyright (C) 2008,2009 by Tim Vandermeersch
 
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
 ***********************************************************************/

#ifndef PYTHONSCRIPT_H
#define PYTHONSCRIPT_H

#include <avogadro/global.h>
#include <boost/python.hpp>
#include <avogadro/pythonerror.h>

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

      QString moduleName() const;
      boost::python::object module() const;

    private:
      QString m_moduleName;
      QString m_fileName;
      mutable QDateTime m_lastModified;
      mutable boost::python::object m_module;
  };

} // end namespace Avogadro

#endif
