/**********************************************************************
  LabelEngine - Engine for displaying labels.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007 Benoit Jacob
  Copyright (C) 2007 Marcus D. Hanwell

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
#include "labelengine.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtPlugin>
#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  LabelEngine::LabelEngine(QObject *parent) : Engine(parent),
  m_atomType(1), m_bondType(2), m_settingsWidget(0)
  {
    setName(tr("Label"));
    setDescription(tr("Renders primitive labels"));
  }

  bool LabelEngine::renderOpaque(PainterDevice *pd)
  {
    QList<Primitive *> list;

    if (m_atomType < 6)
    {
      // Render atom labels
      list = primitives().subList(Primitive::AtomType);
      foreach( Primitive *p, list )
        renderOpaque(pd, static_cast<Atom *>(p));
    }

    if (m_bondType < 2)
    {
      // Now render the bond labels
      list = primitives().subList(Primitive::BondType);
      foreach( Primitive *p, list )
        renderOpaque(pd, static_cast<const Bond*>(p));
    }

    return true;
  }

  bool LabelEngine::renderOpaque(PainterDevice *pd, const Atom *a)
  {
    // Render atom labels
    const Vector3d pos = a->pos();

    double renderRadius = pd->radius(a);
    renderRadius += 0.05;

    double zDistance = pd->camera()->distance(pos);

    if(zDistance < 50.0)
    {
      QString str;
      switch(m_atomType)
      {
        case 0:
          str = QString::number(a->GetIdx());
          break;
        case 1:
          str = QString(etab.GetSymbol(a->GetAtomicNum()));
          break;
        case 3:
          str = QString(((const_cast<Atom *>(a)->GetResidue())->GetName()).c_str());
          break;
        case 4:
          str = QString::number((const_cast<Atom *>(a)->GetResidue())->GetNum());
          break;
        case 5:
          str = QString::number(const_cast<Atom *>(a)->GetPartialCharge(), 10, 2);
          break;
        case 2:
        default:
          str = QString((etab.GetName(a->GetAtomicNum())).c_str());
      }

      Vector3d zAxis = pd->camera()->backTransformedZAxis();

      Vector3d drawPos = pos + zAxis * renderRadius;

      glColor3f(1.0, 1.0, 1.0);
      pd->painter()->drawText(drawPos, str);
    }
    return true;
  }

  bool LabelEngine::renderOpaque(PainterDevice *pd, const Bond *b)
  {
    // Render bond labels
    const Atom* atom1 = static_cast<const Atom *>(b->GetBeginAtom());
    const Atom* atom2 = static_cast<const Atom *>(b->GetEndAtom());
    Vector3d v1 (atom1->pos());
    Vector3d v2 (atom2->pos());
    Vector3d d = v2 - v1;
    d.normalize();

    // Work out the radii of the atoms and the bond
    double renderRadius = pd->radius(b);
    double renderRadiusA1 = pd->radius(atom1);
    double renderRadiusA2 = pd->radius(atom2);
    // If the render radius is zero then this view does not draw bonds
    if (!renderRadius)
      return false;

    renderRadius += 0.05;

    // Calculate the
    Vector3d pos ( (v1 + v2 + d*(renderRadiusA1-renderRadiusA2)) / 2.0 );

    double zDistance = pd->camera()->distance(pos);

    if(zDistance < 50.0)
    {
      QString str;
      switch(m_bondType)
      {
        case 0:
          str = QString::number(b->GetIdx());
          break;
        case 1:
        default:
          str = QString::number(b->GetBondOrder());
      }

      Vector3d zAxis = pd->camera()->backTransformedZAxis();
      Vector3d drawPos = pos + zAxis * renderRadius;

      glColor3f(1.0, 1.0, 1.0);
      pd->painter()->drawText(drawPos, str);
    }
    return true;
  }

  void LabelEngine::setAtomType(int value)
  {
    m_atomType = value;
    emit changed();
  }

  void LabelEngine::setBondType(int value)
  {
    m_bondType = value;
    emit changed();
  }

  QWidget *LabelEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new LabelSettingsWidget();
      connect(m_settingsWidget->atomType, SIGNAL(activated(int)), this, SLOT(setAtomType(int)));
      connect(m_settingsWidget->bondType, SIGNAL(activated(int)), this, SLOT(setBondType(int)));
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
    }
    return m_settingsWidget;
  }

  void LabelEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  Engine::EngineFlags LabelEngine::flags() const
  {
    return Engine::Overlay;
  }
}

#include "labelengine.moc"

Q_EXPORT_PLUGIN2(labelengine, Avogadro::LabelEngineFactory)
