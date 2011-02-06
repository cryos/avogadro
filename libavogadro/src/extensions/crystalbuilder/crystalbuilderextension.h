/**********************************************************************
  crystalbuilderextension.h - Crystal Builder Plugin for Avogadro

  Copyright (C) 2011 by David C. Lonie

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

#ifndef CRYSTALBUILDEREXTENSION_H
#define CRYSTALBUILDEREXTENSION_H

#include "crystalbuilderdialog.h"

#include <avogadro/extension.h>

namespace Avogadro
{
  class CrystalBuilderExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("CrystalBuilder", tr("CrystalBuilder"),
                       tr("Construct and analyze periodic structures."))

    public:
      //! Constructor
      CrystalBuilderExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~CrystalBuilderExtension();

      /** @return a menu path for the extension's actions */
      virtual QString menuPath(QAction *action) const;

      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
      //@}

      void setMolecule(Molecule *molecule);

      void writeSettings();
      void readSettings();

    private:
      QList<QAction *> m_actions;
      CrystalBuilderDialog *m_dialog;

      Molecule *m_molecule;
  };

  class CrystalBuilderExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(CrystalBuilderExtension);
  };

} // end namespace Avogadro

#endif

