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
    
    PrimitiveQueue queue;
    Color colorMap;
    bool enabled;
  };

  Engine::Engine(QObject *parent) : QObject(parent), d(new EnginePrivate)
  {
  }

  Engine::~Engine()
  {
    delete d;
  }

  const PrimitiveQueue& Engine::queue() const
  {
    return d->queue;
  }

  double Engine::radius(const Primitive *primitive)
  {
    return 0.0;
  }

  void Engine::clearQueue()
  {
    d->queue.clear();
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
    d->queue.addPrimitive(primitive);
  }

  void Engine::updatePrimitive(Primitive *primitive)
  {
  }

  void Engine::removePrimitive(Primitive *primitive)
  {
    d->queue.removePrimitive(primitive);
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

}

#include "engine.moc"
