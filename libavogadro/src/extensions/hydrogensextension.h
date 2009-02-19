/**********************************************************************
  Hydrogens - Hydrogens Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

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

#ifndef HYDROGENSEXTENSION_H
#define HYDROGENSEXTENSION_H

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>
#include <avogadro/idlist.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>

namespace Avogadro {

 class HydrogensExtension : public Extension
  {
    Q_OBJECT
      AVOGADRO_EXTENSION("Hydrogens", tr("Hydrogens"))

    public:
      //! Constructor
      HydrogensExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~HydrogensExtension();

      //! \name Description methods
      //@{
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Hydrogens Plugin"); };
      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
      virtual QString menuPath(QAction *action) const;
      //@}

      void setMolecule(Molecule *molecule);

    private:
      QList<QAction *> m_actions;
      Molecule *m_molecule;
  };

  class HydrogensCommand : public QUndoCommand
  {
    public:
      enum Action {
        AddHydrogens = 0,
        AddHydrogensPH,
        RemoveHydrogens
      };

    public:
      HydrogensCommand(Molecule *molecule, enum Action action, GLWidget *widget, double pH = 7.4);
      ~HydrogensCommand();

      virtual void undo();
      virtual void redo();
      virtual bool mergeWith ( const QUndoCommand * command );
      virtual int id() const;

    private:
      Molecule *m_molecule;
      Molecule *m_moleculeCopy;
      IDList m_SelectedList;
      enum Action m_action;
      double m_pH;
  };

  class HydrogensExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)

      AVOGADRO_EXTENSION_FACTORY(HydrogensExtension,
                                 "Hydrogens Extension",
          tr("Hydrogens Extension"),
          tr("Extension to add or delete hydrogens."))

  };


} // end namespace Avogadro

#endif
