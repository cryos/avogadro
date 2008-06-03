/**********************************************************************
  DipoleEngine - Engine to display a 3D vector such as the dipole moment

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

#include "dipoleengine.h"
#include <config.h>

#include <avogadro/color.h>
#include <avogadro/glwidget.h>

using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  DipoleEngine::DipoleEngine(QObject *parent) : Engine(parent)
  {
    setDescription(tr("Renders dipole moments and other 3D data"));
  }

  DipoleEngine::~DipoleEngine()
  {
  }

  Engine* DipoleEngine::clone() const
  {
    DipoleEngine* engine = new DipoleEngine(parent());

    engine->setName(name());
    engine->setEnabled(isEnabled());
    return engine;
  }

  bool DipoleEngine::renderOpaque(PainterDevice *pd)
  {
    Molecule *mol = const_cast<Molecule *>(pd->molecule());

    pd->painter()->setColor(1.0, 0.0, 0.0);

    // TODO: Get all vectors (e.g., pre-calculated, estimated, custom)
    // pop-up menu to select options:
    //    pre-calculated e.g., from Gaussian
    //    estimated = based on calculated partial charges
    //    custom = enter components
    // TODO: Allow users to pick a custom color for the display
    if (!mol->HasData(OBGenericDataType::VectorData))
      return false;
    
    OBVectorData *vd = (OBVectorData*) mol->GetData(OBGenericDataType::VectorData);
    vector3 moment = vd->GetData();

    Vector3d origin = Vector3d(0.0, 0.0, 0.0); // start at the origin
    Vector3d end = Vector3d(moment.x(), moment.y(), moment.z()); // end of cone
    Vector3d joint = 0.2 * end; // 80% along the length
    pd->painter()->drawLine(end, joint, 2.0);
    pd->painter()->drawCone(joint, origin, 0.1);
    // TODO: add a "cross" line for the <--+ look to the dipole moment)

    return true;
  }

  double DipoleEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return 0.;
  }

  double DipoleEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::EngineFlags DipoleEngine::flags() const
  {
    return Engine::Overlay;
  }

}

#include "dipoleengine.moc"

Q_EXPORT_PLUGIN2(DipoleEngine, Avogadro::DipoleEngineFactory)
