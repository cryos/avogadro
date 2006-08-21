/**********************************************************************
  Primatives - Wrapper class around the OpenBabel classes

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

class Renderer;

namespace Avogadro {

  class MainWindow;

  /* not necissary yet
  class Primative
  {
    public:
      Primative() : renderer(NULL) {}
      virtual ~Primative() {}

      virtual Renderer *getRenderer();
      virtual void setRenderer(Renderer *r);

    private:
      Renderer *renderer;
  };
  */

  class Atom : public OpenBabel::OBAtom
  {
    public:
      void render();

  };

  class Bond : public OpenBabel::OBBond
  {
    public:
      void render();

  };

  class Molecule : public OpenBabel::OBMol
  {
    public:
      void render();
      void setWindow(MainWindow *w) { window = w; }
      MainWindow *getWindow() { return window; }


    protected:
      MainWindow *window;
      std::vector< Atom * > 	_vatom;
      std::vector< Bond * > 	_vbond;
  };
}

#endif
