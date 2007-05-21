/**********************************************************************
  Engine - Engine interface and plugin factory.

  Copyright (C) 2006,2007 Donald Ephraim Curtis

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

#include "engine.h"

namespace Avogadro {

  class EnginePrivate
  {
  public:
    EnginePrivate() : enabled(false) {}
    
    PrimitiveList primitiveList;
    Color colorMap;
    bool enabled;

    QString name;
    QString description;
  };

  Engine::Engine(QObject *parent) : QObject(parent), d(new EnginePrivate)
  {
  }

  Engine::~Engine()
  {
    delete d;
  }

  PrimitiveList Engine::primitiveList() const
  {
    return d->primitiveList;
  }

  double Engine::radius(const Primitive*)
  {
    return 0.0;
  }

  void Engine::clearQueue()
  {
    d->primitiveList.clear();
  }

  bool Engine::isEnabled()
  {
    return d->enabled;
  }

  void Engine::setEnabled(bool enabled)
  {
    d->enabled = enabled;
  }

  void Engine::addPrimitive(Primitive *primitive)
  {
    d->primitiveList.append(primitive);
  }

  void Engine::updatePrimitive(Primitive*)
  {
  }

  void Engine::removePrimitive(Primitive *primitive)
  {
    d->primitiveList.removeAll(primitive);
  }

  void Engine::setColorMap(Color &map)
  {
    d->colorMap = map;
  }

  QWidget *Engine::settingsWidget()
  {
    return 0;
  }

  Color &Engine::colorMap()
  {
    return d->colorMap;
  }

  void Engine::setName(const QString &name)
  {
    d->name = name;
  }

  QString Engine::name() const
  {
    return d->name;
  }

  void Engine::setDescription(const QString &description)
  {
    d->description = description;
  }

  QString Engine::description() const
  {
    return d->description;
  }

}

#include "engine.moc"
