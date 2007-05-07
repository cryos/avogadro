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

#ifndef __LABELENGINE_H
#define __LABELENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>
#include <avogadro/sphere.h>
#include <avogadro/cylinder.h>

#include <openbabel/mol.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

namespace Avogadro {

  //! Label Engine class.
  class LabelEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      LabelEngine(QObject *parent=0) : Engine(parent) {}
      //! Deconstructor
      ~LabelEngine() {}

      //! \name Description methods
      //@{
      //! @return engine name
      QString name() { return(QString(tr("Label"))); }
      //! @return engine description
      QString description() { return(QString(tr("Renders primitive labels"))); }
      //@}

      //! \name Render Methods
      //@{
      bool render(GLWidget *gl);
      //@}

  };

  //! Generates instances of our LabelEngine class
  class LabelEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance(QObject *parent = 0) { return new LabelEngine(parent); }
  };

} // end namespace Avogadro

#endif
