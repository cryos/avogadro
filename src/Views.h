/**********************************************************************
  Views - Wrapper class around the Primative classes

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

#ifndef __VIEWS_H
#define __VIEWS_H

#include "Primatives.h"
#include "GLEngine.h"

#include <QList>
#include <openbabel/mol.h>

namespace Avogadro {

  class View : public QObject
  {
    Q_OBJECT

    public:
      View(QObject *parent);
      View(Primative *p, QObject *parent);
      View(const View &v);
      virtual ~View() {};

      void addSubView(View *v);

      GLEngine * getDefaultGLEngine();
      GLEngine * getGLEngine();

    protected:
      Primative *object;
      QList<View *> subViews;
      GLEngine *glEngine;

      public slots:
      virtual void render();
  };

  class AtomView : public View
  {
    Q_OBJECT

    public:
      AtomView(Atom *atom, QObject *parent) : View(parent), object(atom) {}

    protected:
      Atom *object;

    public slots:
      virtual void render();
  };

  class BondView : public View
  {
    Q_OBJECT

    public:
      BondView(Bond *bond, QObject *parent) : View(parent), object(bond) {}

    protected:
      Bond *object;

    public slots:
      virtual void render();
  };

  class MoleculeView : public View
  {
    Q_OBJECT

    public:
      MoleculeView(Molecule *m, QObject *parent);

    public slots:
      void addAtom(Atom *a);
      void addBond(Bond *b);
  };

} // namespace Avogadro

#endif
