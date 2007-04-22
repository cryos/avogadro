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

#include <avogadro/tool.h>
#include <avogadro/glwidget.h>
#include <avogadro/primitives.h>
#include <QAction>
#include <QWidget>

namespace Avogadro {

  class ToolPrivate 
  {
    public:
      ToolPrivate() : m_activateAction(0), m_settingsWidget(0) {}
      ~ToolPrivate() { delete m_settingsWidget; delete m_activateAction; }
      QAction *m_activateAction;
      QWidget *m_settingsWidget;
  };

  Tool::Tool(QObject *parent) : QObject(parent), d(new ToolPrivate)
  { 
    d->m_activateAction = new QAction(this);
    d->m_activateAction->setCheckable(true); 
    d->m_activateAction->setIcon(QIcon(QString::fromUtf8(":/icons/tool.png")));
    d->m_settingsWidget = new QWidget();
  }

  Tool::~Tool() 
  { 
    delete d;
  }

  QString Tool::name() const 
  { 
    return QObject::tr("Unknown"); 
  }

  QString Tool::description() const 
  { 
    return QObject::tr("No Description"); 
  }

  QAction* Tool::activateAction() const {

    if(d->m_activateAction->toolTip() == "")
      d->m_activateAction->setToolTip(description());

    if(d->m_activateAction->text() == "")
      d->m_activateAction->setText(name());

    return d->m_activateAction; 
  }

  QWidget* Tool::settingsWidget() const
  {
    return d->m_settingsWidget;
  }

} // end namespace Avogadro

#include "tool.moc"
