/**********************************************************************
  Extension - Avogadro Extension Interface

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

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

#ifndef __EXTENSION_H
#define __EXTENSION_H

#include <avogadro/primitive.h>

#include <QGLWidget>
#include <QList>
#include <QObject>
#include <QString>
#include <QTextEdit>
#include <QVector>

class QAction;
class QUndoCommand;
namespace Avogadro {

  class Extension : public QObject
  {
    Q_OBJECT;

    public:
    Extension(QObject *parent=0);
    virtual ~Extension();

    virtual QString name() const;
    virtual QString description() const;

    virtual QList<QAction *> actions() const = 0;
    virtual QUndoCommand* performAction(QAction *action, Molecule *molecule, QTextEdit *messages = NULL) = 0;

  };

  class A_EXPORT ExtensionFactory
  {
    public:
      /**
       * Extension factory deconstructor.
       */
      virtual ~ExtensionFactory() {}

      /**
       * @return pointer to a new instance of an Engine subclass object
       */
      virtual Extension *createInstance(QObject *parent=0) = 0;
  };

} // end namespace Avogadro

// Q_DECLARE_INTERFACE(Avogadro::Extension, "net.sourceforge.avogadro.extension/1.0")
Q_DECLARE_INTERFACE(Avogadro::ExtensionFactory, "net.sourceforge.avogadro.extensionfactory/1.0");


#endif
