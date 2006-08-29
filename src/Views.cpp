/**********************************************************************
  Views - Wrapper class around the Primitive classes

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

#include "Views.h"
#include "MainWindow.h"
#include "GLWidget.h"
#include "GLEngine.h"

using namespace Avogadro;
using namespace std;

View::View( QObject * parent ) : QObject(parent), object(NULL), glEngine(NULL)
{
}

View::View( Primitive *p, QObject * parent ) : QObject(parent), object(p), glEngine(NULL)
{
}

View::View(const View &v) : QObject(v.parent())
{
  object = v.object;
  subViews = v.subViews;
}

Avogadro::GLEngine * View::getDefaultGLEngine()
{
  View *v = dynamic_cast<View *>(parent());
  if(v)
  {
    return v->getDefaultGLEngine();
  }

  GLWidget *p = dynamic_cast<GLWidget *>(parent());
  if(p)
  {
    return p->getDefaultGLEngine();
  }

  return NULL;
}

Avogadro::GLEngine * View::getGLEngine()
{
  if(!glEngine)
  {
    return(getDefaultGLEngine());
  }
  else
  {
    return(glEngine);
  }
}

void View::render()
{
  foreach(View *view, subViews) {
    view->render();
  }
}

void View::addSubView(View *v)
{
  subViews.append(v);
}

void AtomView::render()
{
  View::render();
  GLEngine *engine = getGLEngine();
  if(!engine)
  {
    cout << "No Default Rendering Engine Set\n" << endl;
  }
  else
  {
    glPushName(object->GetIdx());
    engine->render(object);
    glPopName();
  }
}

void BondView::render()
{
  View::render();
  GLEngine *engine = getGLEngine();
  if(!engine)
  {
    cout << "No Default Rendering Engine Set\n" << endl;
  }
  else
  {
    engine->render(object);
  }
}

MoleculeView::MoleculeView(Molecule *m, QObject *parent) : View(m, parent)
{
  vector<OpenBabel::OBNodeBase*>::iterator i;

  for(Atom *atom = (Atom*)m->BeginAtom(i); atom; atom = (Atom*)m->NextAtom(i))
  {
    addAtom(atom);
  }

  vector<OpenBabel::OBEdgeBase*>::iterator j;
  for(Bond *bond = (Bond*)m->BeginBond(j); bond; bond = (Bond*)m->NextBond(j))
  {
    addBond(bond);
  }

  QObject::connect(m,SIGNAL(atomAdded(Atom*)), this, SLOT(addAtom(Atom*)));
  QObject::connect(m,SIGNAL(bondAdded(Bond*)), this, SLOT(addBond(Bond*)));

}

void MoleculeView::addAtom(Atom *a)
{
  addSubView(new AtomView(a, this));
}

void MoleculeView::addBond(Bond *b)
{
  addSubView(new BondView(b, this));
}
