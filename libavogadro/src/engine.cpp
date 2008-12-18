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

#include "molecule.h"

namespace Avogadro {

  class EnginePrivate
  {
  public:
    EnginePrivate() {}
  };

  Engine::Engine(QObject *parent) : QObject(parent), d(new EnginePrivate),
    m_shader(0), m_pd(0), m_colorMap(0), m_enabled(false), m_customPrims(false)
  {
  }

  Engine::~Engine()
  {
    if (m_pd)
      if (m_pd->molecule())
        disconnect(m_pd->molecule(), 0, this, 0);
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

  void Engine::setPrimitives(const PrimitiveList &primitives)
  {
    m_customPrims = true;
    m_primitives = primitives;
    emit changed();
  }

  double Engine::radius(const PainterDevice*, const Primitive*) const
  {
    return 0.0;
  }

  void Engine::clearPrimitives()
  {
    m_customPrims = false;
    m_primitives.clear();
  }

  void Engine::setEnabled(bool enabled)
  {
    m_enabled = enabled;
    emit changed();
  }

  void Engine::addPrimitive(Primitive *primitive)
  {
    if (m_customPrims) {
      if(!m_primitives.contains(primitive))
        m_primitives.append(primitive);
    }
    else {
      m_customPrims = true;
      m_primitives = *m_pd->primitives();
      if(!m_primitives.contains(primitive))
        m_primitives.append(primitive);

      // Now listen to the molecule
      connect(m_pd->molecule(), SIGNAL(primitiveAdded(Primitive*)),
              this, SLOT(addPrimitive(Primitive*)));
      connect(m_pd->molecule(), SIGNAL(primitiveRemoved(Primitive*)),
              this, SLOT(removePrimitive(Primitive*)));
    }

    emit changed();
  }

  void Engine::updatePrimitive(Primitive*)
  {
    emit changed();
  }

  void Engine::removePrimitive(Primitive *primitive)
  {
    if (m_customPrims)
      m_primitives.removeAll(primitive);
    else {
      m_customPrims = true;
      m_primitives = *m_pd->primitives();
      m_primitives.removeAll(primitive);
    }
    m_primitives.removeAll(primitive);
    emit changed();
  }

  void Engine::setColorMap(Color *map)
  {
    m_colorMap = map;
    emit changed();
  }

  QWidget *Engine::settingsWidget()
  {
    return 0;
  }

  void Engine::setAlias(const QString &alias)
  {
    m_alias = alias;
  }

  QString Engine::alias() const
  {
    if(m_alias.isEmpty()) { return name(); }
    return m_alias;
  }

  void Engine::setDescription(const QString &description)
  {
    m_description = description;
  }

  QString Engine::description() const
  {
    return m_description;
  }

  Engine::Layers Engine::layers() const
  {
    return Engine::Opaque;
  }

  Engine::PrimitiveTypes Engine::primitiveTypes() const
  {
    return (Engine::Atoms | Engine::Bonds);
  }

  Engine::ColorTypes Engine::colorTypes() const
  {
    return Engine::ColorPlugins;
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

  void Engine::changeMolecule(Molecule *previous, Molecule *)
  {
    if (m_customPrims) {
      m_primitives.clear();
      m_customPrims = false;
      disconnect(previous, 0, this, 0);
    }
  }
}

#include "engine.moc"
