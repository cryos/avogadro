/**********************************************************************
  LabelEngine - Engine for displaying labels.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007 Benoit Jacob

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

LabelEngine::LabelEngine(QObject *parent) : Engine(parent), m_type(0), m_settingsWidget(0)
{
  setName(tr("Label"));
  setDescription(tr("Renders primitive labels"));
}

bool LabelEngine::render(GLWidget *gl)
{
  gl->painter()->begin(gl);
  QList<Primitive *> list;

  list = primitives().subList(Primitive::AtomType);

  foreach( Primitive *p, list ) {
    Atom *atom = static_cast<Atom *>(p);
    const Vector3d pos = atom->pos();

    double renderRadius = 0.;
    foreach(Engine *engine, gl->engines())
    {
      if(engine->isEnabled())
      {
        double engineRadius = engine->radius(atom);
        if(engineRadius > renderRadius) {
          renderRadius = engineRadius;
        }
      }
    }
    renderRadius += 0.05;

    double zDistance = gl->camera()->distance(pos);

    if(zDistance < 50.0)
    {
      QString str;
      switch(m_type)
      {
        case 0:
          str = QString::number(atom->GetIdx());
          break;
        case 1:
          str = QString(etab.GetSymbol(atom->GetAtomicNum()));
          break;
        case 2:
        default:
          str = QString((etab.GetName(atom->GetAtomicNum())).c_str());
      }

      Vector3d zAxis = gl->camera()->backtransformedZAxis();

      Vector3d drawPos = pos + zAxis * renderRadius;

      glColor3f(1.0, 1.0, 1.0);
      gl->painter()->drawText(drawPos, str);
    }
  }
  gl->painter()->end();

  return true;
}

void LabelEngine::setLabelType(int value)
{
  m_type = value;
  emit changed();
}

QWidget *LabelEngine::settingsWidget()
{
  if(!m_settingsWidget)
  {
    m_settingsWidget = new LabelSettingsWidget();
    connect(m_settingsWidget->labelType, SIGNAL(activated(int)), this, SLOT(setLabelType(int)));
  }
  return m_settingsWidget;
}

#include "labelengine.moc"

Q_EXPORT_PLUGIN2(labelengine, LabelEngineFactory)
