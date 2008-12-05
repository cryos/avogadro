/**********************************************************************
  ToolGroup - GLWidget manager for Tools.

  Copyright (C) 2007,2008 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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
 **********************************************************************/

#include <config.h>

#include <avogadro/toolgroup.h>
#include "pluginmanager.h"

#include <QActionGroup>
#include <QAction>
#include <QPluginLoader>
#include <QDir>

#include <QDebug>
#include <QCoreApplication>
#include <QMessageBox>

#include <QObject>

using namespace std;
namespace Avogadro {

  bool toolGreaterThan(const Tool *first, const Tool *second)
  {
    return first->usefulness() > second->usefulness();
  }

  class ToolGroupPrivate
  {
    public:
      ToolGroupPrivate() : activeTool(0), activateActions(0) {}
      ~ToolGroupPrivate() {}

      Tool *activeTool;
      QList<Tool *> tools;
      QActionGroup *activateActions;
  };

  ToolGroup::ToolGroup(QObject *parent) : QObject(parent), d(new ToolGroupPrivate)
  {
    d->activateActions = new QActionGroup(this);
  }

  ToolGroup::~ToolGroup()
  {
    delete(d);
  }

  void ToolGroup::append(QList<Tool *> tools)
  {
    foreach (Tool *tool, tools) {
      if(tool)
      {
        d->tools.append(tool);

        d->activateActions->addAction(tool->activateAction());
        connect(tool->activateAction(), SIGNAL(triggered(bool)),
            this, SLOT(activateTool()));
      }
    }

    // sort the tools
    qSort(d->tools.begin(), d->tools.end(), toolGreaterThan);

    // activate the first tool
    if(d->tools.count()) {
      setActiveTool(d->tools.at(0));
      d->activeTool->activateAction()->setChecked(true);
    }
  }

  void ToolGroup::append(Tool *tool)
  {
    d->tools.append(tool);

    d->activateActions->addAction(tool->activateAction());
    connect(tool->activateAction(), SIGNAL(triggered(bool)),
        this, SLOT(activateTool()));

    // sort the tools
    qSort(d->tools.begin(), d->tools.end(), toolGreaterThan);
  }

  void ToolGroup::activateTool()
  {
    QAction *action = qobject_cast<QAction *>(sender());
    // FIXME: based on mac osx error
    Tool *tool = dynamic_cast<Tool *>(action->parent());

    if(tool) {
      setActiveTool(tool);
    }
  }

  Tool* ToolGroup::activeTool() const
  {
    return d->activeTool;
  }

  void ToolGroup::setActiveTool(int i)
  {
    if (i < d->tools.size()) {
      Tool *tool = d->tools.at(i);
      if (tool)
        setActiveTool(tool);
    }
  }

  void ToolGroup::setActiveTool(const QString& name)
  {
    foreach (Tool *tool, d->tools) {
      if (tool->name() == name) {
        setActiveTool(tool);
        return;
      }
    }
  }

  void ToolGroup::setActiveTool(Tool *tool)
  {
    if(tool) {
      if (d->activeTool && d->activeTool->activateAction())
        d->activeTool->activateAction()->setChecked(false);
      if (tool->activateAction()) {
        tool->activateAction()->setChecked(true);
      }
      d->activeTool = tool;
      emit toolActivated(tool);
    }
  }

  Tool* ToolGroup::tool(int i) const
  {
    if (i < d->tools.size())
      return d->tools.at(i);
    return 0;
  }

  const QList<Tool *>& ToolGroup::tools() const
  {
    return d->tools;
  }

  const QActionGroup * ToolGroup::activateActions() const
  {
    return d->activateActions;
  }

  void ToolGroup::setMolecule(Molecule *molecule)
  {
    foreach(Tool *tool, d->tools)
    {
      tool->setMolecule(molecule);
    }
  }

  void ToolGroup::writeSettings(QSettings &settings) const
  {
    foreach(Tool *tool, d->tools)
    {
      settings.beginGroup(tool->name());
      tool->writeSettings(settings);
      settings.endGroup();
    }
  }

  void ToolGroup::readSettings(QSettings &settings)
  {
    foreach(Tool *tool, d->tools)
    {
      settings.beginGroup(tool->name());
      tool->readSettings(settings);
      settings.endGroup();
    }
  }

} // end namespace Avogadro

#include "toolgroup.moc"
