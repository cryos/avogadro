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
#include <avogadro/primitive.h>

#include <QAction>
#include <QUndoCommand>
#include <QWidget>

namespace Avogadro {

  class ToolPrivate 
  {
    public:
      ToolPrivate() : activateAction(0), settingsWidget(0) {}
      ~ToolPrivate() { 
//         delete settingsWidget; settingsWidget = 0; 
        delete activateAction; activateAction = 0;
      }
      QAction *activateAction;
      QWidget *settingsWidget;
  };

  Tool::Tool(QObject *parent) : QObject(parent), d(new ToolPrivate)
  { 
    d->activateAction = new QAction(this);
    d->activateAction->setCheckable(true); 
    d->activateAction->setIcon(QIcon(QString::fromUtf8(":/icons/tool.png")));
    d->settingsWidget = new QWidget();
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

    if(d->activateAction->toolTip() == "")
      d->activateAction->setToolTip(description());

    if(d->activateAction->text() == "")
      d->activateAction->setText(name());

    return d->activateAction; 
  }

  QWidget* Tool::settingsWidget() const
  {
    return d->settingsWidget;
  }

  int Tool::usefulness() const
  {
    return 0;
  }

  bool Tool::operator<(const Tool &other) const {
    return usefulness() < other.usefulness();
  }

} // end namespace Avogadro

#include "tool.moc"
