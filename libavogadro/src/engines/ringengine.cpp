/**********************************************************************
  RingEngine - Engine for "ring" display

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

#include "ringengine.h"
#include <config.h>

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtPlugin>
#include <QMessageBox>
#include <QString>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro{

  RingEngine::RingEngine(QObject *parent) : Engine(parent), m_settingsWidget(0),
    m_alpha(0.5)
  {
    setName(tr("Ring"));
    setDescription(tr("Renders rings"));
  }

  RingEngine::~RingEngine()
  {
  }

  bool RingEngine::renderOpaque(PainterDevice *pd)
  {
    if (m_alpha < 1.0) return true;
    
    // Use the openbabel GetSSSR() function to find all rings.
    // Special case for everything up to 7 membered rings.
    vector<OBRing *> rings;
    rings = const_cast<Molecule *>(pd->molecule())->GetSSSR();

    pd->painter()->setColor(0.7, 0.7, 0.7);

    // Now actually draw the ring structures
    foreach(OBRing *r, rings)
      renderRing(r->_path, pd);

    return true;
  }

  bool RingEngine::renderTransparent(PainterDevice *pd)
  {
    if (m_alpha > 0.999) return true;
    
    // Use the openbabel GetSSSR() function to find all rings.
    // Special case for everything up to 7 membered rings.
    vector<OBRing *> rings;
    rings = const_cast<Molecule *>(pd->molecule())->GetSSSR();

    pd->painter()->setColor(0.7, 0.7, 0.7, m_alpha);

    glEnable(GL_BLEND);
    // Now actually draw the ring structures
    foreach(OBRing *r, rings)
      renderRing(r->_path, pd);
    glEnable(GL_BLEND);

    return true;
  }
  
  bool RingEngine::renderRing(const vector<int> &ring, PainterDevice *pd)
  {
    // We need to get rid of the constness in order to get the atoms
    Molecule *mol = const_cast<Molecule *>(pd->molecule());
	
    // Disable face culling for ring structures.
    glDisable(GL_CULL_FACE);

    // Optimise for smaller ring structures
    switch (ring.size()) {
      case 3:
        // Single triangle - easy
        pd->painter()->drawTriangle(static_cast<Atom *>(mol->GetAtom(ring[0]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[1]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[2]))->pos());
        break;
      case 4:
        // Two triangles
        pd->painter()->drawTriangle(static_cast<Atom *>(mol->GetAtom(ring[0]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[1]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[2]))->pos());
        pd->painter()->drawTriangle(static_cast<Atom *>(mol->GetAtom(ring[0]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[2]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[3]))->pos());
        break;
      case 5:
        // Three triangles
        pd->painter()->drawTriangle(static_cast<Atom *>(mol->GetAtom(ring[0]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[1]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[2]))->pos());
        pd->painter()->drawTriangle(static_cast<Atom *>(mol->GetAtom(ring[0]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[2]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[3]))->pos());
        pd->painter()->drawTriangle(static_cast<Atom *>(mol->GetAtom(ring[0]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[3]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[4]))->pos());
        break;
      case 6:
        // Four triangles
        pd->painter()->drawTriangle(static_cast<Atom *>(mol->GetAtom(ring[0]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[1]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[2]))->pos());
        pd->painter()->drawTriangle(static_cast<Atom *>(mol->GetAtom(ring[2]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[3]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[4]))->pos());
        pd->painter()->drawTriangle(static_cast<Atom *>(mol->GetAtom(ring[4]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[5]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[0]))->pos());
        pd->painter()->drawTriangle(static_cast<Atom *>(mol->GetAtom(ring[0]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[2]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[4]))->pos());
        break;
      default:
        // The generic case - find the centre of the ring and draw a triangle fan
        Vector3d center;
        for (unsigned int i = 0; i < ring.size(); i++)
          center += static_cast<Atom *>(mol->GetAtom(ring[i]))->pos();
        center /= ring.size();
        for (unsigned int i = 0; i < ring.size()-1; i++)
          pd->painter()->drawTriangle(center,
                                      static_cast<Atom *>(mol->GetAtom(ring[i]))->pos(),
                                      static_cast<Atom *>(mol->GetAtom(ring[i+1]))->pos());
        pd->painter()->drawTriangle(center,
                                    static_cast<Atom *>(mol->GetAtom(ring[ring.size()-1]))->pos(),
                                    static_cast<Atom *>(mol->GetAtom(ring[0]))->pos());

    }
    return true;
  }

  double RingEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return 0.;
  }

  double RingEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::EngineFlags RingEngine::flags() const
  {
    return Engine::Transparent;
  }

  void RingEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    emit changed();
  }

  QWidget* RingEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new RingSettingsWidget();
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
    }
    return m_settingsWidget;
  }

  void RingEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

}

#include "ringengine.moc"

Q_EXPORT_PLUGIN2(ringengine, Avogadro::RingEngineFactory)
