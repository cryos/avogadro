/**********************************************************************
  PackmolExtension - Hydrogens Plugin for Avogadro

  Copyright (C) 2010 by Tim Vandermeersch

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

#ifndef PACKMOLEXTENSION_H
#define PACKMOLEXTENSION_H

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>
#include <avogadro/idlist.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>

namespace Avogadro {

  class PackmolDialog;

  class PackmolExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Packmol", tr("Packmol"),
                       tr("Create complex input systems for MD"))

    public:
      //! Constructor
      PackmolExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~PackmolExtension();

      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
      virtual QString menuPath(QAction *action) const;
      //@}

      void setMolecule(Molecule *molecule);
    
      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);

    public slots:
      void resultsReady(Molecule*);

    private:
      void createDialog();

      QList<QAction *> m_actions;
      Molecule *m_molecule;
      PackmolDialog *m_dialog;
  };

  class PackmolExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(PackmolExtension)
  };


} // end namespace Avogadro

#endif
