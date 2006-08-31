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

  GLEngine *engine = getGLEngine();
  if(!engine)
  {
    cout << "No Default Rendering Engine Set\n" << endl;
  }
  else
  {
    glPushName(otherType);
    glPushName(0);
    engine->render(object);
    glPopName();
    glPopName();
  }
}

void View::addView(View *v)
{
  subViews.append(v);
}

void AtomView::render()
{
  foreach(View *view, subViews) {
    view->render();
  }

  GLEngine *engine = getGLEngine();
  if(!engine)
  {
    cout << "No Default Rendering Engine Set\n" << endl;
  }
  else
  {
    glPushName(atomType);
    glPushName(object->GetIdx());
    engine->render(object);
    glPopName();
    glPopName();
  }
}

void BondView::render()
{
  foreach(View *view, subViews) {
    view->render();
  }

  GLEngine *engine = getGLEngine();
  if(!engine)
  {
    cout << "No Default Rendering Engine Set\n" << endl;
  }
  else
  {
    glPushName(bondType);
    glPushName(object->GetIdx());
    engine->render(object);
    glPopName();
    glPopName();
  }
}

void ResidueView::render()
{
  foreach(View *view, subViews) {
    view->render();
  }

  GLEngine *engine = getGLEngine();
  if(!engine)
  {
    cout << "No Default Rendering Engine Set\n" << endl;
  }
  else
  {
    glPushName(residueType);
    glPushName(object->GetIdx());
    engine->render(object);
    glPopName();
    glPopName();
  }
}

MoleculeView::MoleculeView(Molecule *m, QObject *parent) : 
  View(parent)
{

  QObject::connect(m,SIGNAL(atomAdded(Atom*)), this, SLOT(addAtom(Atom*)));
  QObject::connect(m,SIGNAL(bondAdded(Bond*)), this, SLOT(addBond(Bond*)));
  QObject::connect(m,SIGNAL(residueAdded(Residue*)), this, SLOT(addResidue(Residue*)));

  setMolecule(m);

}

void MoleculeView::setMolecule(Molecule *m)
{
  object = m;

  subViews.clear();

  vector<OpenBabel::OBNodeBase*>::iterator i;  
  for(Atom *atom = (Atom*)object->BeginAtom(i); atom; atom = (Atom*)object->NextAtom(i))
    {
      addAtom(atom);
    }
  
  vector<OpenBabel::OBEdgeBase*>::iterator j;
  for(Bond *bond = (Bond*)object->BeginBond(j); bond; bond = (Bond*)object->NextBond(j))
    {
      addBond(bond);
    }

  vector<OpenBabel::OBResidue*>::iterator k;
  for(Residue *residue = (Residue*)object->BeginResidue(k); residue;
      residue = (Residue *)object->NextResidue(k)) {
    addResidue(residue);
  }
}

void MoleculeView::addAtom(Atom *a)
{
  addView(new AtomView(a, this));
}

void MoleculeView::addBond(Bond *b)
{
  addView(new BondView(b, this));
}

void MoleculeView::addResidue(Residue *r)
{
  addView(new ResidueView(r, this));
}

void MoleculeView::render()
{
  GLEngine *engine = getGLEngine();
  if(!engine)
  {
    cout << "No Default Rendering Engine Set\n" << endl;
  }
  else
  {
    if(!engine->render(object))
    {
      View::render();
    }
  }

}

