/**********************************************************************
  Primitives - Wrapper class around the OpenBabel classes

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

#ifndef __PRIMATIVES_H
#define __PRIMATIVES_H

#include <openbabel/mol.h>
#include <QObject>

class GLEngine;

namespace Avogadro {

  class MainWindow;

  /*
   * Primitive 
   * Used to define signals that all our primatives share.
   * 
   */

  class Primitive : public QObject
  {
    Q_OBJECT

      // XXX We'll need this eventually i'm sure.
  };

  class Atom : public Primitive, public OpenBabel::OBAtom
  {
    Q_OBJECT
  };

  class Bond : public Primitive, public OpenBabel::OBBond
  {
    Q_OBJECT
  };

  class Molecule : public Primitive, public OpenBabel::OBMol
  {
    Q_OBJECT

    public:
      void render();
//X       void setWindow(MainWindow *w) { window = w; }
//X       MainWindow *getWindow() { return window; }

    protected:
      Atom * CreateAtom();
      Bond * CreateBond();

      MainWindow *window;
      std::vector< Atom * > 	_vatom;
      std::vector< Bond * > 	_vbond;

signals:
      void atomAdded(Atom *atom);
      void bondAdded(Bond *bond);
  };

} // namespace Avogadro

#endif
