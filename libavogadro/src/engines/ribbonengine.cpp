/**********************************************************************
  RibbonEngine - Engine for "ribbon" display

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

#include <config.h>
#include "ribbonengine.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QMessageBox>
#include <QString>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  RibbonEngine::RibbonEngine(QObject *parent) : Engine(parent), m_settingsWidget(0),
  m_type(0), m_radius(1.0)
  {
    setDescription(tr("Renders residues as ribbons"));

    // Initialise variables
    m_update = true;
    // Pretty colours for the chains - we can add more. Need a colour picker...
    m_chainColors.push_back(Color(1., 0., 0.));
    m_chainColors.push_back(Color(0., 1., 0.));
    m_chainColors.push_back(Color(0., 0., 1.));
    m_chainColors.push_back(Color(1., 0., 1.));
    m_chainColors.push_back(Color(1., 1., 0.));
    m_chainColors.push_back(Color(0., 1., 1.));
  }

  RibbonEngine::~RibbonEngine()
  {
    // Delete the settings widget if it exists
    if(m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  bool RibbonEngine::renderOpaque(PainterDevice *pd)
  {
    // Check if the chains need updating before drawing them
    if (m_update) updateChains();

    if (m_type == 0) {
      for (int i = 0; i < m_chains.size(); i++) {
        if (m_chains[i].size() <= 1)
          continue;
        pd->painter()->setColor(&m_chainColors[i % m_chainColors.size()]);
        pd->painter()->drawSpline(m_chains[i], m_radius);
      }
    }
    else {
      // Render cylinders between the points and spheres at each point
      for (int i = 0; i < m_chains.size(); i++) {
        if (m_chains[i].size() <= 1)
          continue;
        pd->painter()->setColor(&m_chainColors[i % m_chainColors.size()]);
        pd->painter()->drawSphere(m_chains[i][0], m_radius);
        for (int j = 1; j < m_chains[i].size(); j++) {
          pd->painter()->drawSphere(m_chains[i][j], m_radius);
          pd->painter()->drawCylinder(m_chains[i][j-1], m_chains[i][j], m_radius);
        }
      }
    }

    return true;
  }

  bool RibbonEngine::renderTransparent(PainterDevice *)
  {
    return true;
  }

  double RibbonEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return m_radius;
  }

  void RibbonEngine::setPrimitives(const PrimitiveList &primitives)
  {
    Engine::setPrimitives(primitives);
    m_update = true;
  }

  void RibbonEngine::addPrimitive(Primitive *primitive)
  {
    Engine::addPrimitive(primitive);
    m_update = true;
  }

  void RibbonEngine::updatePrimitive(Primitive *)
  {
    m_update = true;
  }

  void RibbonEngine::removePrimitive(Primitive *primitive)
  {
    Engine::removePrimitive(primitive);
    m_update = true;
  }

  void RibbonEngine::updateChains()
  {
    if (!isEnabled()) return;
    qDebug() << "Update chains called.";
    // Get a list of residues for the molecule
    m_chains.clear();
    QList<Primitive *> list;
    list = primitives().subList(Primitive::ResidueType);
    unsigned int currentChain = 0;
    QVector<Vector3d> pts;

    foreach(Primitive *p, list) {
      Residue *r = static_cast<Residue *>(p);
      if(r->GetName().find("HOH") != string::npos)
        continue;

      if(r->GetChainNum() != currentChain) {
        // this residue is on a new chain
        if(pts.size() > 0)
          m_chains.push_back(pts);
        qDebug() << "Chain " << m_chains.size() << " added.";
        currentChain = r->GetChainNum();
        pts.clear();
      }

      FOR_ATOMS_OF_RESIDUE(a, r) {
        // should be CA
        QString atomID = QString(r->GetAtomID(&*a).c_str());
        atomID.trimmed();
        if (atomID == "CA") {
          pts.push_back(static_cast<Atom *>(&*a)->pos());
        }
        else if (atomID == "N") { } // Possibly use nitrogens too
      } // end atoms in residue

    } // end primitive list (i.e., all residues)
    m_chains.push_back(pts); // Add the last chain (possibly the only chain)
    m_update = false;
  }

  double RibbonEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::EngineFlags RibbonEngine::flags() const
  {
    return Engine::Transparent | Engine::Atoms;
  }

  void RibbonEngine::setType(int value)
  {
    m_type = value;
    emit changed();
  }

  void RibbonEngine::setRadius(int value)
  {
    m_radius = 0.1 * value;
    emit changed();
  }

  QWidget* RibbonEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new RibbonSettingsWidget();
      connect(m_settingsWidget->renderType, SIGNAL(activated(int)), this, SLOT(setType(int)));
      connect(m_settingsWidget->radiusSlider, SIGNAL(valueChanged(int)), this, SLOT(setRadius(int)));
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
    }
    return m_settingsWidget;
  }

  void RibbonEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }
}

#include "ribbonengine.moc"

Q_EXPORT_PLUGIN2(ribbonengine, Avogadro::RibbonEngineFactory)
