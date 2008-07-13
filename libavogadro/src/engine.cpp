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
    EnginePrivate() : colorMap(0), enabled(false) {}

    PrimitiveList primitives;
    Color *colorMap;
    bool enabled;

    QString alias;
    QString description;
  };

  Engine::Engine(QObject *parent) : QObject(parent), d(new EnginePrivate)
  {
  }

  Engine::~Engine()
  {
    delete d;
  }

  Plugin::Type Engine::type() const
  { 
    return Plugin::EngineType; 
  }
  
  QString Engine::typeName() const
  { 
    return tr("Engines"); 
  }

  PrimitiveList Engine::primitives() const
  {
    return d->primitives;
  }

  void Engine::setPrimitives(const PrimitiveList &primitives)
  {
    d->primitives = primitives;
    emit changed();
  }

  double Engine::radius(const PainterDevice*, const Primitive*) const
  {
    return 0.0;
  }

  void Engine::clearPrimitives()
  {
    d->primitives.clear();
  }

  bool Engine::isEnabled() const
  {
    return d->enabled;
  }

  void Engine::setEnabled(bool enabled)
  {
    d->enabled = enabled;
    emit changed();
  }

  void Engine::addPrimitive(Primitive *primitive)
  {
    d->primitives.append(primitive);
    emit changed();
  }

  void Engine::updatePrimitive(Primitive*)
  {
    emit changed();
  }

  void Engine::removePrimitive(Primitive *primitive)
  {
    d->primitives.removeAll(primitive);
    emit changed();
  }

  void Engine::setColorMap(Color *map)
  {
    d->colorMap = map;
    emit changed();
  }

  QWidget *Engine::settingsWidget()
  {
    return 0;
  }

  Color *Engine::colorMap()
  {
    return d->colorMap;
  }

  void Engine::setAlias(const QString &alias)
  {
    d->alias = alias;
  }

  QString Engine::alias() const
  {
    if(d->alias.isEmpty()) { return name(); }
    return d->alias;
  }

  void Engine::setDescription(const QString &description)
  {
    d->description = description;
  }

  QString Engine::description() const
  {
    return d->description;
  }

  Engine::EngineFlags Engine::flags() const
  {
    return Engine::NoFlags;
  }

  double Engine::transparencyDepth() const
  {
    return 0.0;
  }

  void Engine::writeSettings(QSettings &settings) const
  {
    settings.setValue("enabled", isEnabled());
    settings.setValue("alias", alias());
    settings.setValue("description", description());
  }

  void Engine::readSettings(QSettings &settings)
  {
    setEnabled(settings.value("enabled", false).toBool());
    setAlias(settings.value("alias", name()).toString());
    setDescription(settings.value("description", description()).toString());
  }
}

#include "engine.moc"
