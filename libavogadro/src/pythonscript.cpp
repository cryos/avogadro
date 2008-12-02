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

#include <avogadro/pythonscript.h>

#include <QDebug>

using namespace std;
using namespace boost::python;

namespace Avogadro
{

  PythonScript::PythonScript(QDir dir, QString fileName)
  {
    m_fileName = fileName;
    m_dir = dir;
    m_lastModified = QFileInfo(dir, fileName).lastModified();

    QString moduleName = fileName.left(fileName.size()-3);

    m_moduleName = moduleName;
//    object script_module(handle<>(PyImport_ImportModule(moduleName.toAscii().data())));
    try
    {
      // these do the same thing one is just a boost helper function
      // the other just wraps in the same way
//      m_module = object(handle<>(PyImport_ImportModule(moduleName.toAscii().data())));
      m_module = import(moduleName.toAscii().data());
    }
    catch(error_already_set const &)
    {
    }
  }

  QString PythonScript::moduleName() const
  {
    return m_moduleName;
  }

  object PythonScript::module() const
  {
    QFileInfo fileInfo(m_dir, m_fileName);
    if(fileInfo.lastModified() > m_lastModified)
    {
      try
      {
        m_module = object(handle<>(PyImport_ReloadModule(m_module.ptr())));
      }
      catch(error_already_set const &)
      { }
      m_lastModified = fileInfo.lastModified();
    }
    return m_module;
  }

}

