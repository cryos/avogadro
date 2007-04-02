/**********************************************************************
  LabelEngine - Engine for displaying labels.

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "config.h"
#include "labelengine.h"

#include <avogadro/primitives.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

bool LabelEngine::render()
{
  GLWidget *gl = qobject_cast<GLWidget *>(parent());
  if(!gl) {
    return false;
  }

  QList<Primitive *> list;

  list = queue().primitiveList(Primitive::AtomType);
  foreach( Primitive *p, list ) {
    Atom *atom = qobject_cast<Atom *>(p);
    const Vector3d pos = atom->pos();
    float radius = 0.18 + etab.GetVdwRad(atom->GetAtomicNum()) * 0.3;
    
    gl->renderText(pos.x(), pos.y(), pos.z() + (radius), 
        QString::number(atom->GetIdx()));
  }
}

bool LabelEngine::render(const Atom *a)
{
}

bool LabelEngine::render(const Bond *b)
{
}

bool LabelEngine::render(const Molecule *m)
{
}

#include "labelengine.moc"

Q_EXPORT_PLUGIN2(labelengine, LabelEngineFactory)
