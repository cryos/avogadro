/**********************************************************************
  GLEngine - QT Plugin Template

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

#ifndef __GLENGINE_H
#define __GLENGINE_H

#include "Primatives.h"

#include <QString>

using namespace Avogadro;

class GLEngine
{
  public:
    virtual ~GLEngine() {}

    virtual QString name() = 0;
    virtual QString description() = 0;
    virtual void render(Primative *) {}
    virtual void render(Atom *) {}
    virtual void render(Bond *) {}
};

class GLEngineFactory
{
  public:
    virtual GLEngine *createInstance() = 0;
};

Q_DECLARE_INTERFACE(GLEngineFactory, "net.sourceforge.avogadro/1.0")

#endif
