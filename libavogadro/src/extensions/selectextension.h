/**********************************************************************
  Selection - Various selection options for Avogadro

  Copyright (C) 2006-2007 by Donald Ephraim Curtis
  Copyright (C) 2006-2007 by Geoffrey R. Hutchison

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

#ifndef SELECTEXTENSION_H
#define SELECTEXTENSION_H

#include <avogadro/extension.h>
#include <avogadro/periodictableview.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>
#include <QListView>
#include <QTreeView>

namespace Avogadro {

 class SelectExtension : public Extension
  {
    Q_OBJECT
    
    public:
      //! Constructor
      SelectExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~SelectExtension();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("Selections"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Selection Plugin"); };
      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
      virtual QString menuPath(QAction *action) const;
      virtual void setMolecule(Molecule *molecule);
      //@}

    public Q_SLOTS:
      void selectElement(int element);

    private:
      QList<QAction *> m_actions;
      Molecule *m_molecule;
      GLWidget *m_widget;
      PeriodicTableView *m_periodicTable;

      void invertSelection(GLWidget *widget);
      void selectSMARTS(GLWidget *widget);
      void selectResidue(GLWidget *widget);
      void selectSolvent(GLWidget *widget);
      void addNamedSelection(GLWidget *widget);
      void namedSelections(GLWidget *widget);
  };

  class SelectExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)

    AVOGADRO_EXTENSION_FACTORY(SelectExtension,
      tr("Select Extension"),
      tr("Extension with various select actions."))

  };


} // end namespace Avogadro

#endif
