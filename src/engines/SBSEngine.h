/**********************************************************************
  SBSEngine - Engine for "balls and sticks" display

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

#ifndef __SBSRENDER_H
#define __SBSRENDER_H

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

#include <openbabel/mol.h>

#include <GLEngine.h>

class SBSEngine : public QObject, public GLEngine
{
  Q_OBJECT

  public:
    SBSEngine() : GLEngine(), atomDL(0), bondDL(0) {}
    ~SBSEngine() {}

    QString name() { return(QString(tr("SBSEngine"))); }
    QString description() { return(QString(tr("Small Ball and Stick Engine"))); }
    void render(Atom &atom);
    void render(Bond &bond);

  private:
    GLuint dlist;

    void initAtomDL();
    GLuint atomDL;
    GLuint bondDL;

};

class SBSEngineFactory : public QObject, public GLEngineFactory
{
  Q_OBJECT
  Q_INTERFACES(GLEngineFactory)

  public:
    GLEngine *createInstance() { return new SBSEngine(); }
};

#endif
