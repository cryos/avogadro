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

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

LabelEngine::LabelEngine(QObject *parent) : Engine(parent), m_glwidget(0),
  m_atomType(1), m_bondType(1), m_settingsWidget(0)
{
  setName(tr("Label"));
  setDescription(tr("Renders primitive labels"));
}

bool LabelEngine::renderOpaque(GLWidget *gl)
{
  m_glwidget = gl;
  gl->painter()->begin(gl);

  QList<Primitive *> list;

  if (m_atomType < 3)
  {
    // Render atom labels
    list = primitives().subList(Primitive::AtomType);
    foreach( Primitive *p, list )
      renderOpaque(static_cast<Atom *>(p));
  }

  if (m_bondType < 1)
  {
    // Now render the bond labels
    list = primitives().subList(Primitive::BondType);
    foreach( Primitive *p, list )
      renderOpaque(static_cast<const Bond*>(p));
  }

  gl->painter()->end();
  return true;
}

bool LabelEngine::renderOpaque(const Atom *a)
{
  // Render atom labels
  const Vector3d pos = a->pos();

  double renderRadius = 0.;
  foreach(Engine *engine, m_glwidget->engines())
  {
    if(engine->isEnabled())
    {
      double engineRadius = engine->radius(a);
      if(engineRadius > renderRadius)
        renderRadius = engineRadius;
    }
  }
  renderRadius += 0.05;

  double zDistance = m_glwidget->camera()->distance(pos);

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
      case 2:
      default:
        str = QString((etab.GetName(a->GetAtomicNum())).c_str());
    }

    Vector3d zAxis = m_glwidget->camera()->backtransformedZAxis();

    Vector3d drawPos = pos + zAxis * renderRadius;

    glColor3f(1.0, 1.0, 1.0);
    m_glwidget->painter()->drawText(drawPos, str);
  }
  return true;
}

bool LabelEngine::renderOpaque(const Bond *b)
{
  // Render bond labels
  const Atom* atom1 = static_cast<const Atom *>(b->GetBeginAtom());
  const Atom* atom2 = static_cast<const Atom *>(b->GetEndAtom());
  Vector3d v1 (atom1->pos());
  Vector3d v2 (atom2->pos());
  Vector3d d = v2 - v1;
  d.normalize();

  // Work out the radii of the atoms and the bond
  double renderRadius = 0.;
  double renderRadiusA1 = 0.;
  double renderRadiusA2 = 0.;
  foreach(Engine *engine, m_glwidget->engines())
  {
    if(engine->isEnabled())
    {
      if (engine->radius(atom1) > renderRadiusA1)
        renderRadiusA1 = engine->radius(atom1);
      if (engine->radius(atom2) > renderRadiusA2)
        renderRadiusA2 = engine->radius(atom2);
      if(engine->radius(b) > renderRadius)
        renderRadius = engine->radius(b);
    }
  }
  // If the render radius is zero then this view does not draw bonds
  if (!renderRadius)
    return false;

  renderRadius += 0.05;

  // Calculate the
  Vector3d pos ( (v1 + v2 + d*(renderRadiusA1-renderRadiusA2)) / 2.0 );

  double zDistance = m_glwidget->camera()->distance(pos);

  if(zDistance < 50.0)
  {
    QString str = QString::number(b->GetIdx());

    Vector3d zAxis = m_glwidget->camera()->backtransformedZAxis();
    Vector3d drawPos = pos + zAxis * renderRadius;

    glColor3f(1.0, 1.0, 1.0);
    m_glwidget->painter()->drawText(drawPos, str);
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

#include "labelengine.moc"

Q_EXPORT_PLUGIN2(labelengine, LabelEngineFactory)
