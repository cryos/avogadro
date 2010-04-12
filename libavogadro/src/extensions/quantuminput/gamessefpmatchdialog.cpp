/**********************************************************************
  gamessefpmatchdialog - Class Description

  Copyright (C) 2007  <>

  This file is part of the $PROJECT$.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "gamessefpmatchdialog.h"

#include <openbabel/generic.h>

using namespace OpenBabel;
namespace Avogadro
{

  GamessEfpMatchDialog::GamessEfpMatchDialog( QAbstractItemModel *model,
      GamessEfpMatchDialog::Type type,
      QWidget *parent,
      Qt::WindowFlags f )
      : QDialog( parent, f )
  {

    ui.setupUi( this );
    ui.groupList->setModel( model );
//
    // delete the model when the dialog is destroyed.
    connect( this, SIGNAL( destroyed() ), model, SLOT( deleteLater() ) );
    connect( ui.groupList->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ),
             this, SLOT( efpSelected( QItemSelection, QItemSelection ) ) );

    if ( type == GamessEfpMatchDialog::QMType ) {
      setWindowTitle( tr( "QM Matches" ) );
    }
    m_type = type;

  }


  GamessEfpMatchDialog::~GamessEfpMatchDialog()
  {}

  void GamessEfpMatchDialog::efpSelected( const QItemSelection &, const QItemSelection & )
  {
    QModelIndexList indexes = ui.groupList->selectionModel()->selectedIndexes();

    QList<QVector<Atom *> > selectedGroups;

    foreach( QModelIndex index, indexes ) {
      QVector<Atom *> group = index.data( Qt::UserRole + 1 ).value<QVector<Atom *> >();
      selectedGroups.append( group );
    }

    emit selectionChanged( selectedGroups );

  }

  void GamessEfpMatchDialog::accept()
  {
    QModelIndexList indexes = ui.groupList->selectionModel()->selectedIndexes();

    if ( indexes.size() ) {
      QList<QVector<Atom *> > groups;

      foreach( QModelIndex index, indexes ) {
        QVector<Atom *> group = index.data( Qt::UserRole + 1 ).value<QVector<Atom *> >();
        groups.append( group );
      }

      emit accepted( m_type, ui.groupLine->text(), groups );
    }

    QDialog::accept();
  }

  void GamessEfpMatchDialog::reject()
  {
    QDialog::reject();
  }

  void GamessEfpMatchDialog::select(const QModelIndex & index, QItemSelectionModel::SelectionFlags command)
  {
    ui.groupList->selectionModel()->select(index, command);
  }

}

