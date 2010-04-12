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
#ifndef AVOGADROGAMESSEFPMATCHDIALOG_H
#define AVOGADROGAMESSEFPMATCHDIALOG_H

#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>
#include <QDialog>

#include "ui_gamessefpmatchdialog.h"
namespace Avogadro
{

  /**
   @author
  */
  class GamessEfpMatchDialog : public QDialog
  {
    Q_OBJECT


    public:
      enum Type {
        EFPType = 0,
        QMType
      };

      explicit GamessEfpMatchDialog(QAbstractItemModel *model, Type = EFPType,
                           QWidget *parent = 0, Qt::WindowFlags f = 0);

      ~GamessEfpMatchDialog();

      QString groupName() const;
      QModelIndexList selectedIndexes() const;
      Molecule *molecule() const;
      Type type() const;

    private:
      Ui::GamessEfpMatchDialog ui;

      Molecule *m_molecule;
      GLWidget *m_widget;

      Type m_type;

    public Q_SLOTS:
      void accept();
      void reject();

      void select(const QModelIndex & index, QItemSelectionModel::SelectionFlags command = QItemSelectionModel::Select);

    private Q_SLOTS:
      void efpSelected( const QItemSelection &selected, const QItemSelection &deselected );

    Q_SIGNALS:
      void selectionChanged(const QList<QVector<Atom *> > &groups);
      void accepted(const GamessEfpMatchDialog::Type &type,
                    const QString &name, const QList<QVector<Atom *> > &groups);

  };

}

Q_DECLARE_METATYPE( QVector<Avogadro::Atom *> )

#endif
