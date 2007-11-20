/**********************************************************************
 PolygonEngine - Engine for "polygon" display
 
 Copyright (C) 2007 by Marcus D. Hanwell
 
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

#include "polygonengine.h"
#include <config.h>

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>

#include <QtPlugin>
#include <QMessageBox>
#include <QString>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro{
  
  PolygonEngine::PolygonEngine(QObject *parent) : Engine(parent)
  {
    setName(tr("Polygon"));
    setDescription(tr("Renders structures as polygons"));
  }
  
  PolygonEngine::~PolygonEngine()
  {
  }
  
  bool PolygonEngine::renderOpaque(PainterDevice *pd)
  {
    QList<Primitive *> list;
    list = primitives().subList(Primitive::AtomType);
    foreach(Primitive *p, list)
      renderPolygon(pd, static_cast<const Atom *>(p));

    return true;
  }
  
  bool PolygonEngine::renderTransparent(PainterDevice *)
  {
    return true;
  }

  bool PolygonEngine::renderPolygon(PainterDevice *pd, Atom *a)
  {
    // Check if the atom is of the right type and has enough neighbours
    switch (a->GetAtomicNum()) {
      case 1:
      case 6:
      case 7:
      case 8:
      case 16:
        return true;
        break;
      default:
        // we're fine, render this as a possible polygon
        if (a->GetValence() < 4)
          return true;
    }

    Color map = colorMap();
    map.set(a);
    pd->painter()->setColor(&map);

    QVector<Vector3d> atoms;
    FOR_NBORS_OF_ATOM(neighbor, a)
      atoms.push_back(static_cast<Atom *>(&*neighbor)->pos());

    // Disable face culling for ring structures.
//    glDisable(GL_CULL_FACE);
    for (int i = 0; i < atoms.size(); i++) {
      for (int j = 0; j < atoms.size(); j++)
        for (int k = 1; k < atoms.size(); k++)
          pd->painter()->drawTriangle(atoms[i], atoms[j], atoms[k]);
//      pd->painter()->drawTriangle(atoms[i], atoms[0], atoms[atoms.size()-1]);
    }
    // Disable face culling for ring structures.
    glEnable(GL_CULL_FACE);
  }

  double PolygonEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return 0.;
  }

  Engine::EngineFlags PolygonEngine::flags() const
  {
    return Engine::Atoms;
  }

}

#include "polygonengine.moc"

Q_EXPORT_PLUGIN2(polygonengine, Avogadro::PolygonEngineFactory)
