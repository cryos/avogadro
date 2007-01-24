/**********************************************************************
  Plugin - Qt Plugin Template

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

#ifndef __PLUGIN_H
#define __PLUGIN_H

#include <avogadro/primitives.h>

#include <QString>
#include <QAction>
#include <QObject>
#include <QVector>
#include <QList>

namespace Avogadro {

  class PluginAction;

  //! Base class for our plugins
  class Plugin
  {
    public:
      //! Deconstructor
      virtual ~Plugin() { }

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("Unknown"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Unknown Tool"); };
      //@}

      //! \name Plugin Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      //! Random calls.
      //@}
      virtual QList<QAction *> actions();
      
    protected:
      QList<QAction *> m_actions;

  };

  class PluginAction : QAction
  {
    Q_OBJECT

    signals:
      void triggered(Molecule *molecule);
  };

} // end namespace Avogadro

Q_DECLARE_INTERFACE(Avogadro::Plugin, "net.sourceforge.avogadro.plugin/1.0")


#endif
