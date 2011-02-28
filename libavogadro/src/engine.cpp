/**********************************************************************
  Engine - Engine interface and plugin factory.

  Copyright (C) 2006,2007 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include <avogadro/painterdevice.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/color.h>

#include <QDebug>

namespace Avogadro {

  class EnginePrivate
  {
  public:
    EnginePrivate() {}
  };

  Engine::Engine(QObject *parent) : Plugin(parent), d(new EnginePrivate),
    m_shader(0), m_pd(0), m_molecule(0), m_colorMap(0), m_enabled(false),
    m_customPrims(false)
  {
  }

  Engine::~Engine()
  {
    if (m_molecule)
      disconnect(m_molecule, 0, this, 0);
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
    m_atoms.clear();
    foreach (Primitive *p, m_primitives.subList(Primitive::AtomType))
      m_atoms.append(static_cast<Atom *>(p));
    m_bonds.clear();
    foreach (Primitive *p, m_primitives.subList(Primitive::BondType))
      m_bonds.append(static_cast<Bond *>(p));
    emit changed();
  }

  double Engine::radius(const PainterDevice*, const Primitive*) const
  {
    return 0.0;
  }

  void Engine::clearPrimitives()
  {
    // Set custom primitives to false and clear the lists of primitives
    m_customPrims = false;
    m_primitives.clear();
    m_atoms.clear();
    m_bonds.clear();
    emit changed();
  }

  void Engine::setPainterDevice(const PainterDevice *pd)
  {
    m_pd = pd;
    setMolecule(m_pd->molecule());
  }

  void Engine::setEnabled(bool enabled)
  {
    if (m_enabled != enabled)
      emit enableToggled(enabled);

    m_enabled = enabled;
    emit changed();
  }

  void Engine::addPrimitive(Primitive *p)
  {
    if (m_customPrims) {
      if (p->type() == Primitive::AtomType) {
        if (!m_atoms.contains(static_cast<Atom *>(p)))
          m_atoms.append(static_cast<Atom *>(p));
      }
      else if (p->type() == Primitive::BondType) {
        if (!m_bonds.contains(static_cast<Bond *>(p)))
          m_bonds.append(static_cast<Bond *>(p));
      }
      else {
        if(!m_primitives.contains(p))
          m_primitives.append(p);
      }
    }
    else {
      useCustomPrimitives();
      if (p->type() == Primitive::AtomType) {
        if (!m_atoms.contains(static_cast<Atom *>(p)))
          m_atoms.append(static_cast<Atom *>(p));
      }
      else if (p->type() == Primitive::BondType) {
        if (!m_bonds.contains(static_cast<Bond *>(p)))
          m_bonds.append(static_cast<Bond *>(p));
      }
      else {
        if(!m_primitives.contains(p))
          m_primitives.append(p);
      }
    }

    emit changed();
  }

  void Engine::updatePrimitive(Primitive*)
  {
    emit changed();
  }

  void Engine::removePrimitive(Primitive *p)
  {
    if (m_customPrims) {
      if (p->type() == Primitive::AtomType)
        m_atoms.removeAll(static_cast<Atom *>(p));
      else if (p->type() == Primitive::BondType)
        m_bonds.removeAll(static_cast<Bond *>(p));
      else
        m_primitives.removeAll(p);
    }
    else {
      useCustomPrimitives();
      if (p->type() == Primitive::AtomType)
        m_atoms.removeAll(static_cast<Atom *>(p));
      else if (p->type() == Primitive::BondType)
        m_bonds.removeAll(static_cast<Bond *>(p));
      else
        m_primitives.removeAll(p);
    }
    emit changed();
  }

  void Engine::addAtom(Atom *a)
  {
    if (m_customPrims) {
      if (!m_atoms.contains(a))
        m_atoms.append(a);
    }
    else {
      useCustomPrimitives();
      if (!m_atoms.contains(a))
        m_atoms.append(a);
    }
    emit changed();
  }

  void Engine::removeAtom(Atom *a)
  {
    if (m_customPrims) {
      m_atoms.removeAll(a);
    }
    else {
      useCustomPrimitives();
      m_atoms.removeAll(a);
    }
    emit changed();
  }

  void Engine::addBond(Bond *b)
  {
    if (m_customPrims) {
      if (!m_bonds.contains(b))
        m_bonds.append(b);
    }
    else {
      useCustomPrimitives();
      if (!m_bonds.contains(b))
        m_bonds.append(b);
    }
    emit changed();
  }

  void Engine::removeBond(Bond *b)
  {
    if (m_customPrims) {
      m_bonds.removeAll(b);
    }
    else {
      useCustomPrimitives();
      m_bonds.removeAll(b);
    }
    emit changed();
  }

  void Engine::setColorMap(Color *map)
  {
    m_colorMap->disconnect(this);
    m_colorMap = map;
    connect(m_colorMap, SIGNAL(changed()),
            this, SLOT(colorMapChanged()));
    emit changed();
  }

  void Engine::colorMapChanged()
  {
    emit changed();
  }

  QWidget *Engine::settingsWidget()
  {
    return 0;
  }

  bool Engine::hasSettings()
  {
    return settingsWidget();
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
    settings.setValue("engineID", identifier());
    settings.setValue("enabled", isEnabled());
    if (!m_alias.isEmpty())
      settings.setValue("alias", alias());
    if (!m_description.isEmpty())
      settings.setValue("description", description());
  }

  void Engine::readSettings(QSettings &settings)
  {
    setEnabled(settings.value("enabled", false).toBool());
    QString engineID = settings.value("engineID", QString()).toString();
    if (engineID == identifier()) {
      setAlias(settings.value("alias", QString()).toString());
      setDescription(settings.value("description", QString()).toString());
    }
  }

  void Engine::setMolecule(const Molecule *mol)
  {
    if (m_customPrims) {
      m_primitives.clear();
      m_atoms.clear();
      m_bonds.clear();
      m_customPrims = false;
      if (m_molecule)
        disconnect(m_molecule, 0, this, 0);
    }
    m_molecule = mol;
  }

  void Engine::setMolecule(Molecule *molecule)
  {
    // this was causing an infinite loop before
    //    setMolecule(molecule);
    if (m_customPrims) {
      m_primitives.clear();
      m_atoms.clear();
      m_bonds.clear();
      m_customPrims = false;
      if (m_molecule)
        disconnect(m_molecule, 0, this, 0);
    }
    m_molecule = molecule;
  }

  void Engine::useCustomPrimitives()
  {
    if(!m_molecule)
      return;

    m_customPrims = true;
    m_atoms = m_molecule->atoms();
    m_bonds = m_molecule->bonds();

    // Now listen to the molecule
    connect(m_molecule, SIGNAL(atomAdded(Atom*)),
            this, SLOT(addAtom(Atom*)));
    connect(m_molecule, SIGNAL(atomRemoved(Atom*)),
            this, SLOT(removeAtom(Atom*)));
    connect(m_molecule, SIGNAL(bondAdded(Bond*)),
            this, SLOT(addBond(Bond*)));
    connect(m_molecule, SIGNAL(bondRemoved(Bond*)),
            this, SLOT(removeBond(Bond*)));
  }

  const PrimitiveList Engine::primitives() const
  {
    return m_primitives;
  }

  const QList<Atom *> Engine::atoms() const
  {
    if (m_customPrims)
      return m_atoms;
    else
      return m_molecule->atoms();
  }

  const QList<Bond *> Engine::bonds() const
  {
    if (m_customPrims)
      return m_bonds;
    else
      return m_molecule->bonds();
  }
}
