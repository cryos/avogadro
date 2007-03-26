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

#include <avogadro/primitives.h>

#include <QString>
#include <QAction>
#include <QObject>
#include <QGLWidget>
#include <QVector>
#include <QTextEdit>
#include <QList>

namespace Avogadro {

  class Extension
  {
    public:
      virtual ~Extension() {}

      virtual QString name() const { return QObject::tr("Unknown"); }
      virtual QString description() const { return QObject::tr("Unknown Extension"); };

      virtual QList<QAction *> actions() { return m_actions; }
      virtual void performAction(QAction *action, Molecule *molecule, QTextEdit *messages = NULL) {}
      
    protected:
      QList<QAction *> m_actions;

  };

} // end namespace Avogadro

Q_DECLARE_INTERFACE(Avogadro::Extension, "net.sourceforge.avogadro.extension/1.0")


#endif
