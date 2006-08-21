/**********************************************************************
  BSRenderer - Renderer for "balls and sticks" display

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

#ifndef __BSRENDER_H
#define __BSRENDER_H

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

#include <openbabel/mol.h>

#include <Renderer.h>

class BSRenderer : public QObject, public Renderer
{
  Q_OBJECT

  public:
    BSRenderer() : Renderer(), atomDL(0), bondDL(0) {}
    ~BSRenderer() {}

    QString name() { return(QString(tr("BSRenderer"))); }
    QString description() { return(QString(tr("Ball and Stick Renderer"))); }
    void renderAtom(Atom &atom);
    void renderBond(Bond &bond);

  private:
    GLuint dlist;

    void initAtomDL();
    GLuint atomDL;
    GLuint bondDL;

};

class BSRendererFactory : public QObject, public RendererFactory
{
  Q_OBJECT
  Q_INTERFACES(RendererFactory)

  public:
    Renderer *createInstance() { return new BSRenderer(); }
};

#endif
