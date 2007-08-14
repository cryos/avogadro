/**********************************************************************
  gamessefpmatchdialog - Class Description

  Copyright (C) 2007  <>

  This file is part of the $PROJECT$.
  For more information, see <http://avogadro.sourceforge.net/>

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

  GamessEfpMatchDialog::GamessEfpMatchDialog(QAbstractItemModel *model,
                                             Molecule *molecule,
                                             GLWidget *widget,
                                             GamessEfpMatchDialog::Type type,
                                             QWidget *parent,
                                             Qt::WindowFlags f)
      : QDialog(parent, f)
  {

    // delete the dialog if the model is destroyed.
    connect(molecule, SIGNAL(destroyed()), this, SLOT(close()));
    connect(molecule, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(widget, SIGNAL(destroyed()), this, SLOT(close()));
    connect(widget, SIGNAL(destroyed()), this, SLOT(deleteLater()));

    ui.setupUi(this);
    ui.efpList->setModel(model);
//
    // delete the model when the dialog is destroyed.
    connect(this, SIGNAL(destroyed()), model, SLOT(deleteLater()));
    connect(ui.efpList->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(efpSelected(QItemSelection, QItemSelection)));

    if(type == GamessEfpMatchDialog::QMType)
    {
      setWindowTitle(tr("QM Matches"));
    }
    m_type = type;

    m_widget = widget;
    m_molecule = molecule;
  }


  GamessEfpMatchDialog::~GamessEfpMatchDialog()
  {}

  void GamessEfpMatchDialog::efpSelected(const QItemSelection &selected,
                                        const QItemSelection &deselected)
  {
    foreach(QModelIndex index, selected.indexes())
    {
      QList<Primitive *> primitives;
      QVector<int> atoms = index.data(Qt::UserRole + 1).value<QVector<int> >();

      foreach(int idx, atoms)
      {
        Atom *atom  = static_cast<Atom *>(m_molecule->GetAtom(idx));
        primitives.append(atom);
      }
      m_widget->setSelected(primitives, true);
    }

    foreach(QModelIndex index, deselected.indexes())
    {
      QList<Primitive *> primitives;
      QVector<int> atoms = index.data(Qt::UserRole + 1).value<QVector<int> >();

      foreach(int idx, atoms)
      {
        Atom *atom  = static_cast<Atom *>(m_molecule->GetAtom(idx));
        primitives.append(atom);
      }
      m_widget->setSelected(primitives, false);
    }
  }

  void GamessEfpMatchDialog::accept()
  {

    QDialog::accept();
  }

  void GamessEfpMatchDialog::reject()
  {
    QDialog::reject();
  }

  QModelIndexList GamessEfpMatchDialog::selectedIndexes() const
  {
    return ui.efpList->selectionModel()->selectedIndexes();
  }

  QString GamessEfpMatchDialog::groupName() const
  {
    return ui.groupLine->text();
  }

  Molecule *GamessEfpMatchDialog::molecule() const
  {
    return m_molecule;
  }

  GamessEfpMatchDialog::Type GamessEfpMatchDialog::type() const
  {
    return m_type;
  }
}



#include "gamessefpmatchdialog.moc"
