/**********************************************************************
  PrimitiveTreeView - Specialize QTreeView for PrimitiveItemModels

  Copyright (C) 2007 Donald Ephraim Curtis <dcurtis3@sourceforge.net>

  This file is part of the Avogadro molecular editor project.
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

#include <QTreeView>
#include <QObject>
#include <QWidget>
#include <QItemDelegate>

#ifndef __PRIMITIVETREEVIEW_H
#define __PRIMITIVETREEVIEW_H

namespace Avogadro
{
  class Engine;
  class Molecule;

  /**
   * @class PrimitiveTreeView
   * @author
   * @brief
   *
   */
  class PrimitiveTreeViewPrivate;
  class PrimitiveTreeView : public QTreeView
  {
    Q_OBJECT

    public:

      /**
       * Constructor(s)
       */
      PrimitiveTreeView(QWidget *parent = 0);
      explicit PrimitiveTreeView(Molecule *molecule, QWidget *parent = 0);
      explicit PrimitiveTreeView(Engine *engine, QWidget *parent = 0);

      /**
       * Deconstructor
       */
      ~PrimitiveTreeView ();

    private:
      PrimitiveTreeViewPrivate * const d;

      /**
       * Common constructor operations
       */
      void initialize();

    private Q_SLOTS:
      void itemPressed(const QModelIndex & index);
  };

  /**
   * @class MoleculeItemDelegate
   * @author Donald Ephraim Curtis
   *
   * Special delegate provided to jazz up a TreeView
   * when looking at PrimitiveItemModel's.
   */
  class PrimitiveItemDelegatePrivate;
  class PrimitiveItemDelegate : public QItemDelegate
  {
    Q_OBJECT
    public:
      PrimitiveItemDelegate(QTreeView *view, QWidget *parent);

      void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
      QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;

    private:
      PrimitiveItemDelegatePrivate * const d;

  };


}

#endif // __PRIMITIVETREEVIEW_H
