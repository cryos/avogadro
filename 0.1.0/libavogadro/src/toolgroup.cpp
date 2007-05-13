/**********************************************************************
  ToolGroup - GLWidget manager for Tools.

  Copyright (C) 2007 Donald Ephraim Curtis

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

#include "config.h"

#include <avogadro/toolgroup.h>
#include <avogadro/tool.h>

#include <QActionGroup>
#include <QDir>
#include <QDebug>
#include <QPluginLoader>

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

  void ToolGroup::load()
  {
    QString prefixPath = QString(INSTALL_PREFIX) + "/lib/avogadro/tools";
    QStringList pluginPaths;
    pluginPaths << prefixPath;

#ifdef WIN32
	pluginPaths << "./tools";
#endif

    if(getenv("AVOGADRO_TOOLS") != NULL)
    {
      pluginPaths = QString(getenv("AVOGADRO_TOOLS")).split(':');
    }
  
    foreach (QString path, pluginPaths)
    {
      QDir dir(path); 
      foreach (QString fileName, dir.entryList(QDir::Files)) {
        qDebug() << fileName;
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        QObject *instance = loader.instance();
        ToolFactory *factory = qobject_cast<ToolFactory *>(instance);
        if (factory) {
          Tool *tool = factory->createInstance(this);
          qDebug() << "Found Tool: " << tool->name() << " - " << tool->description(); 
          d->tools.append(tool);
          d->activateActions->addAction(tool->activateAction());
          connect(tool->activateAction(), SIGNAL(triggered(bool)),
              this, SLOT(activateTool()));
        }
      }
    }

    qSort(d->tools.begin(), d->tools.end(), toolGreaterThan);
    if(d->tools.count()) {
      setActiveTool(d->tools.at(0));
      d->activeTool->activateAction()->setChecked(true);
    }
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
    Tool *tool = d->tools.at(i);
    if(tool) {
      d->activeTool = tool;
      emit toolActivated(tool);
    }
  }

  void ToolGroup::setActiveTool(Tool *tool)
  {
    if(tool) {
      d->activeTool = tool;
      emit toolActivated(tool);
    }
  }

  const QList<Tool *>& ToolGroup::tools() const
  {
    return d->tools;
  }

  const QActionGroup * ToolGroup::activateActions() const
  {
    return d->activateActions;
  }

} // end namespace Avogadro

#include "toolgroup.moc"
