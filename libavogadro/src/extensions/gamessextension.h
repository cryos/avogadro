/**********************************************************************
  GAMESS - GAMESS Input Deck Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

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

#ifndef __GAMESSEXTENSION_H
#define __GAMESSEXTENSION_H

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

    public:
      //! Constructor
      GamessExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~GamessExtension();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("GAMESS"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("GAMESS Input Deck Generator"); };

      //! Perform Action
      virtual QList<QAction *> actions() const;

      /**
       * @return the menu path for the specified action
       */
      virtual QString menuPath(QAction *action) const;


      //! GAMESS Dock Widget
      virtual QDockWidget * dockWidget();

      virtual QUndoCommand* performAction(QAction *action, Molecule *molecule,
          GLWidget *widget, QTextEdit *messages=NULL);
      //@}

    private:
      GamessInputDialog *m_inputDialog;
      GamessInputData *m_inputData;
      QList<QAction *> m_actions;
      QDockWidget * m_gamessEfpDock;
      QTreeView *m_efpView;
      QPushButton *m_efpButton;

      QStandardItemModel *m_efpModel;

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

  class GamessExtensionFactory : public QObject, public ExtensionFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::ExtensionFactory);

    public:
      Extension *createInstance(QObject *parent = 0) { return new GamessExtension(parent); }
  };

} // end namespace Avogadro

#endif
