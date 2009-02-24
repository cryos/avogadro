/**********************************************************************
  GAMESS - GAMESS Input Deck Plugin for Avogadro

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

#ifndef GAMESSEXTENSION_H
#define GAMESSEXTENSION_H

#include "gamessinputdialog.h"
#include "gamessefpmatchdialog.h"

#include <openbabel/mol.h>

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include <QObject>
#include <QList>
#include <QString>

class QStandardItemModel;
namespace Avogadro {

  class GamessExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("GAMESS", tr("GAMESS"),
                       tr("Create input files for the GAMESS quantum chemistry package"))

    public:
      //! Constructor
      GamessExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~GamessExtension();

      //! Perform Action
      virtual QList<QAction *> actions() const;

      /**
       * @return the menu path for the specified action
       */
      virtual QString menuPath(QAction *action) const;

      //! GAMESS Dock Widget
      virtual QDockWidget * dockWidget();

      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

      //@}

      void setMolecule(Molecule *molecule);

    private:
      GamessInputDialog *m_inputDialog;
      GamessInputData *m_inputData;
      QList<QAction *> m_actions;
      QDockWidget * m_gamessEfpDock;
      QTreeView *m_efpView;
      QPushButton *m_efpButton;

      QStandardItemModel *m_efpModel;

      Molecule *m_molecule;

      GamessEfpMatchDialog *m_efpDialog;
      GamessEfpMatchDialog *m_qmDialog;

      QMap<GamessEfpMatchDialog *, GLWidget *> m_dialogWidgets;
      QMap<GamessEfpMatchDialog *, Molecule *> m_dialogMolecules;
      QMap<GLWidget *, PrimitiveList > m_widgetSelected;

      QMap<QString, int> m_groupNameID;

      GamessEfpMatchDialog *matchesDialog(Molecule *molecule, GLWidget *widget, GamessEfpMatchDialog::Type type);

    private Q_SLOTS:
      void dockWidgetDestroyed();

      void efpButtonPressed();
      void efpViewSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected );

      void efpWidgetAccepted(const GamessEfpMatchDialog::Type &type, const QString &name, const QList<QVector<Atom *> > &groups);
      void efpWidgetSelected(const QList<QVector<Atom *> > &groups);
      void efpWidgetDone();

      void removePrimitive(Primitive *primitive);

  };

  class GamessExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(GamessExtension)
  };

} // end namespace Avogadro

#endif
