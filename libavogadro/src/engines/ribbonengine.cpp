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

#include "ribbonengine.h"
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
using namespace Avogadro;
using namespace Eigen;


RibbonEngine::RibbonEngine(QObject *parent) : Engine(parent), m_settingsWidget(0),
  m_alpha(1.)
{
  setName(tr("Ribbon"));
  setDescription(tr("Renders residues as ribbons"));
}

RibbonEngine::~RibbonEngine()
{
  // Delete the settings widget if it exists
  if(m_settingsWidget)
    m_settingsWidget->deleteLater();
}

bool RibbonEngine::renderOpaque(PainterDevice *pd)
{
  QList<Primitive *> list;

  // List of chains that make up the backbone
  QList< QVector<Vector3d> > chains;

  // Get a list of residues for the molecule
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
        chains.push_back(pts);
      qDebug() << "Chain " << chains.size() << " added.";
      currentChain = r->GetChainNum();
      pts.clear();
    }
	
    FOR_ATOMS_OF_RESIDUE(a, r) {
      // should be CA
//      cerr << r->GetAtomID(&*a) << "\n";
      QString atomID = QString(r->GetAtomID(&*a).c_str());
      atomID.trimmed();
      if (atomID == "CA") {
        qDebug() << " CA detected!";
        pts.push_back(static_cast<Atom *>(&*a)->pos());
      }
      else if (atomID == "N") { } // Possibly use nitrogens too
    } // end atoms in residue

  } // end primitive list (i.e., all residues)
  chains.push_back(pts); // Add the last chain (possibly the only chain)  

  foreach(QVector<Vector3d> pts, chains) {
    if (pts.size() > 1) {
	  qDebug() << "Drawing residue with " << pts.size() << "atoms.";
      pd->painter()->drawSpline(pts, 0.35);
    }
  }

  return true;
}

bool RibbonEngine::renderTransparent(PainterDevice *)
{
  return true;
}

bool RibbonEngine::render(PainterDevice *pd, const Atom *a)
{
  // Render the atoms as Van der Waals spheres
  Color map = colorMap();
  map.set(a);
  map.setAlpha(m_alpha);
  pd->painter()->setColor(&map);
  pd->painter()->setName(a);
  pd->painter()->drawSphere( a->pos(), radius(a) );

  return true;
}

inline double RibbonEngine::radius(const Atom *a) const
{
  return etab.GetVdwRad(a->GetAtomicNum());
}

double RibbonEngine::radius(const PainterDevice *pd, const Primitive *p) const
{
  // Atom radius
  if (p->type() == Primitive::AtomType)
  {
    if(primitives().contains(p))
    {
      if (pd && pd->isSelected(p))
      {
        return radius(static_cast<const Atom *>(p)) + SEL_ATOM_EXTRA_RADIUS;
      }
      return radius(static_cast<const Atom *>(p));
    }
  }
  // Something else
  return 0.;
}

double RibbonEngine::transparencyDepth() const
{
  return 1.0;
}

Engine::EngineFlags RibbonEngine::flags() const
{
  return Engine::Transparent | Engine::Atoms;
}

void RibbonEngine::setOpacity(int value)
{
  m_alpha = 0.05 * value;
  emit changed();
}

QWidget* RibbonEngine::settingsWidget()
{
  if(!m_settingsWidget)
  {
    m_settingsWidget = new RibbonSettingsWidget();
    connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
    connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
  }
  return m_settingsWidget;
}

void RibbonEngine::settingsWidgetDestroyed()
{
  qDebug() << "Destroyed Settings Widget";
  m_settingsWidget = 0;
}


#include "ribbonengine.moc"

Q_EXPORT_PLUGIN2(ribbonengine, RibbonEngineFactory)
