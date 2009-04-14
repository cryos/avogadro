/**********************************************************************
  WireEngine - Simple engine for fast wire frame display

  Copyright (C) 2009 Marcus D. Hanwell

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

#include "simplewireengine.h"

#include <config.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/color.h>
#include <avogadro/painterdevice.h>

namespace Avogadro {

  using Eigen::Vector3d;

  SimpleWireEngine::SimpleWireEngine(QObject *parent) : Engine(parent)
  {
  }

  SimpleWireEngine::~SimpleWireEngine()
  {
  }

  Engine* SimpleWireEngine::clone() const
  {
    SimpleWireEngine* engine = new SimpleWireEngine(parent());
    engine->setAlias(alias());
    engine->setEnabled(isEnabled());
    return engine;
  }

  bool SimpleWireEngine::renderOpaque(PainterDevice *pd)
  {
    glDisable(GL_LIGHTING);

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    foreach(Bond *b, bonds()) {
      const Atom* atom1 = b->beginAtom();
      const Vector3d & v1 = *atom1->pos();
      const Atom* atom2 = b->endAtom();
      const Vector3d & v2 = *atom2->pos();
      map->set(atom1);
      pd->painter()->setColor(map);
      if (atom1->atomicNumber() != atom2->atomicNumber()) {
        // compute the mid-point between the two atoms
        Vector3d v3((v1 + v2) / 2.0);
        pd->painter()->drawLine(v1, v3, 1.0);
        map->set(atom2);
        pd->painter()->setColor(map);
        pd->painter()->drawLine(v3, v2, 1.0);
      }
      else {
        // Just draw a line
        pd->painter()->drawLine(v1, v2, 1.0);
      }
    }

    glEnable(GL_LIGHTING);
    return true;
  }
}

#include "simplewireengine.moc"

Q_EXPORT_PLUGIN2(simplewireengine, Avogadro::SimpleWireEngineFactory)
