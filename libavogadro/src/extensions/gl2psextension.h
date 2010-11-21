/**********************************************************************
  gl2psextension.h - Vector graphics export

  Copyright (C) 2010 by Konstantin Tokarev
  Based on code from mainwindow.cpp

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef GL2PSEXSTESION_H
#define GL2PSEXSTESION_H

#include <avogadro/extension.h>

#include <QtGui/QUndoCommand>

namespace Avogadro {

//  class QUndoCommand;

  class Gl2psExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Vector graphics", tr("Vector graphics export"),
                         tr("Vector graphics export"))
  public:
    //! Constructor
    Gl2psExtension(QObject *parent=0);
    //! Deconstructor
//    virtual ~Gl2psExtension();

    /** @return a menu path for the extension's actions */
    virtual QString menuPath(QAction *action) const;

    virtual QList<QAction *> actions() const;
    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

  private:
    QList<QAction *> m_actions;
  };

  class Gl2psExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(Gl2psExtension)
  };
}

#endif
