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

#ifndef PYTHONERROR_H
#define PYTHONERROR_H

#include <avogadro/global.h>

#include <QObject>
#include <QString>

namespace Avogadro {

  /**
   * @brief Singleton object to store all python errors
   * 
   * To be able to display all python errors in the GUI later, it is needed to
   * store python errors somewhere until the GUI objects are created. This 
   * class stores the appended errors and when the GUI is created, the preceding
   * errors can be retrieved for display. The PythonError class has a message 
   * signal which is emitted if the object is in listening mode (see setListening()).
   *
   * All calls to python functions (directly to the Python C API or through 
   * boost::python), should use the following form:
   *
   * @code
   * try {
   *   prepareToCatchError();
   *   // Do python stuff here...
   * } catch(error_already_set const &) {
   *   catchError(); 
   *   // additional error handling here... 
   * }
   * @endcode
   * 
   *
   */
  class A_EXPORT PythonError : public QObject
  {
    Q_OBJECT

    Q_SIGNALS:
      /**
       * In listening mode, this signal is emitted when a new python error is 
       * caught by catchError().
       */
      void message(const QString&);

    public:
      /**
       * Get a pointer to the singleton instance.
       */
      static PythonError* instance();
      /**
       * This method has 2 behaviours depening on the listening mode.
       *
       * 1) When a class is listening, calling this method will emit the message signal.
       * 
       * 2) When there is no class listening, calling this method will 
       * append str to the internal error log.
       */
      void append(const QString &str);
      /**
       * Get the internal error log containing all errors when listening 
       * was disabled.
       */
      QString& string();
      /**
       * Set listening mode.
       */
      void setListening(bool listening);
    private:
      PythonError();
      QString m_str;
      bool m_listening;
  };

  /** 
   * Initialize python. This is a no-operation when called for a second time.
   *
   * @note Py_Finalize is never called. While there may be situations where a 
   * user disables all python features after using them, calling Py_Finalize()
   * may result in unexpected behaviour. 
   * (http://docs.python.org/c-api/init.html#Py_Finalize)
   *
   * @param addToSearchPath Optional parameter containing path(s) to add 
   * (prepend) to sys.path. Multiple paths can be separated by ';'. 
   * 
   * @return True if python was successfully initialized.
   */
  A_EXPORT bool initializePython(const QString &addToSearchPath = QString());

  /**
   * Prepare to catch errors (exceptions) from executing python code. See 
   * PythonError class documentation for more information on how to use this
   * function.
   */
  A_EXPORT void prepareToCatchError();

  /**
   * Get the last error resulting from executing python code. See PythonError 
   * class documentation for more information on how to use this function.
   */
  A_EXPORT void catchError();

} // namespace

#endif
