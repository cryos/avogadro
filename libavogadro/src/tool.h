/**********************************************************************
  Tool - Avogadro Tool Interface

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

#ifndef __TOOL_H
#define __TOOL_H

#include <avogadro/global.h>
#include <avogadro/primitives.h>
#include <QString>
#include <QObject>

#include <QAction>
#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>

namespace Avogadro {

  class GLWidget;

  class ToolPrivate;
  class A_EXPORT Tool : public QObject
  {
    Q_OBJECT

    public:
      Tool(QObject *parent = 0);
      virtual ~Tool();

      virtual QString name() const;
      virtual QString description() const;

      virtual QAction* activateAction() const;
      virtual QWidget* settingsWidget() const;

      virtual void mousePress(GLWidget *widget, const QMouseEvent *event) = 0;
      virtual void mouseRelease(GLWidget *widget, const QMouseEvent *event) = 0;
      virtual void mouseMove(GLWidget *widget, const QMouseEvent *event) = 0;
      virtual void wheel(GLWidget *widget, const QWheelEvent *event) = 0;

    protected:
      ToolPrivate *const d;

  };
  class A_EXPORT ToolFactory
  {
    public:
      /**
       * Tool factory deconstructor.
       */
      virtual ~ToolFactory() {}

      /**
       * @return pointer to a new instance of an Engine subclass object
       */
      virtual Tool *createInstance(QObject *parent=0) = 0;
  };

} // end namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Tool*)
Q_DECLARE_INTERFACE(Avogadro::ToolFactory, "net.sourceforge.avogadro.toolfactory/1.0");

#endif
