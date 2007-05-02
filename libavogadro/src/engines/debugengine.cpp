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
#include "debugengine.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

bool DebugEngine::render(GLWidget *gl)
{
  QList<Primitive *> list;

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_LIGHTING);

  gl->renderText(5,20,"---- Debug Information ----");
  gl->renderText(5,35,"FPS: " + QString::number(gl->framesPerSecond()));

  list = queue().primitiveList(Primitive::AtomType);
  gl->renderText(5,50,"Atoms: " + QString::number(list.size()));

  list = queue().primitiveList(Primitive::BondType);
  gl->renderText(5,65,"Bonds: " + QString::number(list.size()));

  glPopAttrib();
}

bool DebugEngine::render(const Atom *a)
{
}

bool DebugEngine::render(const Bond *b)
{
}

bool DebugEngine::render(const Molecule *m)
{
}

#include "debugengine.moc"

Q_EXPORT_PLUGIN2(debugengine, DebugEngineFactory)
