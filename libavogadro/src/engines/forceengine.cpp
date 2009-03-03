/**********************************************************************
  ForceEngine - Display forces

  Copyright (C) 2008 by Tim Vandermeersch

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

#include "forceengine.h"

#include <config.h>
#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/glwidget.h>
#include <avogadro/painterdevice.h>

#include <QMessageBox>
#include <QString>
#include <QDebug>

using namespace std;
using namespace Eigen;

namespace Avogadro {

  ForceEngine::ForceEngine(QObject *parent) : Engine(parent)
  {  }

  Engine *ForceEngine::clone() const
  {
    ForceEngine *engine = new ForceEngine(parent());
    engine->setAlias(alias());

    return engine;
  }

  ForceEngine::~ForceEngine()
  {
  }

  bool ForceEngine::renderOpaque(PainterDevice *pd)
  {
    pd->painter()->setColor(0.0, 1.0, 0.0); // TODO: make configurable

    Vector3d v2, v3;
    foreach(Atom *atom, atoms()) {
      renderOpaque(pd, atom);
    }

    return true;
  }

  bool ForceEngine::renderOpaque(PainterDevice *pd, const Atom *atom)
  {
    const Vector3d forceVector = atom->forceVector();
    if (forceVector.norm() < 0.01) // too small to really show
      return true;

    const Vector3d &v1 = *atom->pos();

    // Use the camera and painter device to "float" the arrows
    // in front of the atom. This is similar to the label engine code
    double renderRadius = pd->radius(atom) + 0.05;
    Vector3d zAxis = pd->camera()->backTransformedZAxis();
    Vector3d drawPos = v1 + zAxis * renderRadius;

    // now based on our "drawing" position, we calculate a displacement
    Vector3d v2 = drawPos + atom->forceVector();
    Vector3d v3 = drawPos + 0.8 * atom->forceVector();

    pd->painter()->drawLine(drawPos, v2, 2);
    pd->painter()->drawCone(v3, v2, 0.1);

    return true;
  }

}

#include "forceengine.moc"

Q_EXPORT_PLUGIN2(forceengine, Avogadro::ForceEngineFactory)
