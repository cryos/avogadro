/**********************************************************************
  PythonError - Handle python errors

  Copyright (C) 2008 by Tim Vandermeersch

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

#ifndef PYTHONERROR_H
#define PYTHONERROR_H

#include <avogadro/global.h>

#include <QObject>
#include <QString>

namespace Avogadro {

  class A_EXPORT PythonError : public QObject
  {
    Q_OBJECT

    Q_SIGNALS:
      void message(const QString&);

    public:
      PythonError(QObject *parent = 0) : QObject(parent), m_listening(false)
      {}
      void append(const QString &str)
      {
        if (m_listening)
          emit message(str); // emit signal when other class is listening
        else
          m_str += str; // else, store the error        
      }
      QString& string()
      {
        return m_str;
      }
      void setListening(bool listening)
      {
        m_listening = listening;
      }

    private:
      QString m_str;
      bool m_listening;
  };

  A_EXPORT PythonError* pythonError();
  
  A_EXPORT void prepareToCatchError();

  A_EXPORT void catchError();

} // namespace

#endif
