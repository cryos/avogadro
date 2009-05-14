/**********************************************************************
 PolygonEngine - Engine for "polygon" display

 Copyright (C) 2007 by Marcus D. Hanwell

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

#include "polygonengine.h"

#include <avogadro/color.h>
#include <avogadro/painter.h>
#include <avogadro/painterdevice.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <QGLWidget>
#include <QDebug>

using Eigen::Vector3d;

namespace Avogadro{

  PolygonEngine::PolygonEngine(QObject *parent) : Engine(parent)
  {
  }

  Engine *PolygonEngine::clone() const
  {
    PolygonEngine *engine = new PolygonEngine(parent());
    engine->setAlias(alias());
    engine->setEnabled(isEnabled());

    return engine;
  }

  PolygonEngine::~PolygonEngine()
  {
  }

  bool PolygonEngine::renderOpaque(PainterDevice *pd)
  {
    foreach(Atom *a, atoms())
      renderPolygon(pd, a);

    return true;
  }

  bool PolygonEngine::renderPolygon(PainterDevice *pd, Atom *a)
  {
    // Check if the atom is of the right type and has enough neighbours
    switch (a->atomicNumber()) {
      case 1:
      case 6:
      case 7:
      case 8:
      case 16:
        return true;
        break;
      default:
        // we're fine, render this as a possible polygon
        if (a->valence() < 4)
          return true;
    }

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    map->set(a);
    pd->painter()->setColor(map);

    QVector<const Vector3d*> atoms;
    QList<unsigned long> neighbors = a->neighbors();
    foreach (unsigned long neighbor, neighbors) {
      atoms.push_back(pd->molecule()->atomById(neighbor)->pos());
    }

    // Disable face culling for ring structures.
    for (int i = 0; i < atoms.size(); i++) {
      for (int j = 0; j < atoms.size(); j++)
        for (int k = 1; k < atoms.size(); k++)
          pd->painter()->drawTriangle(*atoms[i], *atoms[j], *atoms[k]);
//      pd->painter()->drawTriangle(atoms[i], atoms[0], atoms[atoms.size()-1]);
    }
    // Disable face culling for ring structures.
    return true;
  }

  double PolygonEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return 0.;
  }

  Engine::PrimitiveTypes PolygonEngine::primitiveTypes() const
  {
    return Engine::Atoms;
  }

}

Q_EXPORT_PLUGIN2(polygonengine, Avogadro::PolygonEngineFactory)
