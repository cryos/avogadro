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

#include "pythonscript.h"

#include <QDebug>
#include <QTextEdit>
#include <QCryptographicHash>

using namespace std;
using namespace boost::python;

namespace Avogadro
{

  PythonScript::PythonScript(const QString &fileName)
  {
    m_fileName = fileName;
    QFileInfo info(fileName);
    m_moduleName = info.baseName();
    m_lastModified = info.lastModified();
     
    try
    {
      prepareToCatchError();
      // try to import the module
      m_module = import(m_moduleName.toAscii().data());
      // import doesn't really reload the module if it was already loaded
      // to be save, we always reload it
      m_module = object(handle<>(PyImport_ReloadModule(m_module.ptr())));
    }
    catch(error_already_set const &)
    {
      catchError();
    }
  }

  PythonScript::~PythonScript()
  {
  }

  QString PythonScript::moduleName() const
  {
    return m_moduleName;
  }

  object PythonScript::module() const
  {
    QFileInfo fileInfo(m_fileName);

    if(fileInfo.lastModified() > m_lastModified)
    {
      try
      {
        prepareToCatchError();
        m_module = object(handle<>(PyImport_ReloadModule(m_module.ptr())));
      }
      catch(error_already_set const &)
      {
        catchError();
      }

      m_lastModified = fileInfo.lastModified();
    }

    return m_module;
  }

  const QString& PythonScript::fileName() const
  {
    return m_fileName;
  }
      
  QString PythonScript::identifier() const
  {
    QFile file(m_fileName);
    file.open(QIODevice::ReadOnly);
    QString identity = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha1).toHex();
    return identity;
  }

  bool PythonScript::hasAttrString(const char *attrString) const
  {
    return PyObject_HasAttrString(module().ptr(), attrString);
  }
}
