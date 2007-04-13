/**********************************************************************
  Engine - Engine interface and plugin factory.

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Donald Ephraim Curtis

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

#include "engine.h"

namespace Avogadro {

  class EnginePrivate
  {
    public:
      EnginePrivate() : enabled(true) {}

      PrimitiveQueue queue;
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
}

#include "engine.moc"
