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
#include <QAction>
#include <QObject>
#include <QGLWidget>
#include <QVector>
#include <QTextEdit>
#include <QList>

namespace Avogadro {

  class GLWidget;

  class A_EXPORT Tool
  {
    public:
      Tool() : m_activateAction(new QAction(0)), m_settingsWidget(new QWidget(0)) { 
        m_activateAction->setCheckable(true); 
        m_activateAction->setData(qVariantFromValue(this));
        m_activateAction->setIcon(QIcon(QString::fromUtf8(":/icons/tool.png")));
      }
      virtual ~Tool() { delete m_activateAction; }

      virtual QString name() const { return QObject::tr("Unknown"); }
      virtual QString description() const { return QObject::tr("Unknown Tool"); };

      virtual void init() {}
      virtual void cleanup() {}

      virtual QAction* activateAction() const {
        if(m_activateAction->toolTip() == "")
          m_activateAction->setToolTip(description());
        
        if(m_activateAction->text() == "")
          m_activateAction->setText(name());
        
        
        return m_activateAction; 
      }

      virtual QWidget* settingsWidget() const {
        return m_settingsWidget;
      }
      virtual void mousePress(Molecule *molecule, GLWidget *widget, const QMouseEvent *event) = 0;
      virtual void mouseRelease(Molecule *molecule, GLWidget *widget, const QMouseEvent *event) = 0;
      virtual void mouseMove(Molecule *molecule, GLWidget *widget, const QMouseEvent *event) = 0;
      virtual void wheel(Molecule *molecule, GLWidget *widget, const QWheelEvent *event) = 0;

    protected:
      QAction *m_activateAction;
      QWidget *m_settingsWidget;

  };
} // end namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Tool*)
Q_DECLARE_INTERFACE(Avogadro::Tool, "net.sourceforge.avogadro.tool/1.0")

#endif
