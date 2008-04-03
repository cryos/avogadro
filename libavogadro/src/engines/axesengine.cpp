/**********************************************************************
  AxesEngine - Engine to display the x, y, z axes

  Copyright (C) 2008      Marcus D. Hanwell

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

#include <config.h>
#include "axesengine.h"

#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <eigen/regression.h>

#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace Eigen;

namespace Avogadro {

  AxesEngine::AxesEngine(QObject *parent) : Engine(parent)
  {
    setDescription(tr("Renders the x, y and z axes"));
  }

  AxesEngine::~AxesEngine()
  {
  }

  Engine* AxesEngine::clone() const
  {
    AxesEngine* engine = new AxesEngine(parent());

    engine->setName(name());
    engine->setEnabled(isEnabled());
    return engine;
  }

  bool AxesEngine::renderOpaque(PainterDevice *pd)
  {
    // Right now just draw x, y, z axes one unit long. Will add more options.
    Vector3d origin = Vector3d(0.0, 0.0, 0.0);
    Vector3d aXa = Vector3d(0.8, 0.0, 0.0);
    Vector3d aX = Vector3d(1.0, 0.0, 0.0);
    Vector3d aYa = Vector3d(0.0, 0.8, 0.0);
    Vector3d aY = Vector3d(0.0, 1.0, 0.0);
    Vector3d aZa = Vector3d(0.0, 0.0, 0.8);
    Vector3d aZ = Vector3d(0.0, 0.0, 1.0);
    // x axis
    pd->painter()->setColor(1.0, 0.0, 0.0);
    pd->painter()->drawCylinder(origin, aXa, 0.05);
    pd->painter()->drawCone(aXa, aX, 0.1);
    // y axis
    pd->painter()->setColor(0.0, 1.0, 0.0);
    pd->painter()->drawCylinder(origin, aYa, 0.05);
    pd->painter()->drawCone(aYa, aY, 0.1);
    // y axis
    pd->painter()->setColor(0.0, 0.0, 1.0);
    pd->painter()->drawCylinder(origin, aZa, 0.05);
    pd->painter()->drawCone(aZa, aZ, 0.1);

    return true;
  }

  double AxesEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return 0.;
  }

  double AxesEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::EngineFlags AxesEngine::flags() const
  {
    return Engine::Overlay;
  }

}

#include "axesengine.moc"

Q_EXPORT_PLUGIN2(axesengine, Avogadro::AxesEngineFactory)
