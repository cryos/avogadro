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

#include "primitivetreeview.h"

#include <avogadro/primitiveitemmodel.h>
#include <avogadro/primitive.h>
#include <avogadro/engine.h>

#include <QHeaderView>
#include <QPainter>
#include <QPen>

namespace Avogadro
{
  class PrimitiveTreeViewPrivate
  {
    public:
      PrimitiveTreeViewPrivate() : delegate(0) {}

      PrimitiveItemDelegate *delegate;
  };

  PrimitiveTreeView::PrimitiveTreeView(QWidget *parent) : QTreeView(parent), d(new PrimitiveTreeViewPrivate)
  {
    initialize();
  }

  PrimitiveTreeView::PrimitiveTreeView(Molecule *molecule, QWidget *parent) : QTreeView(parent), d(new PrimitiveTreeViewPrivate)
  {
    initialize();
    setModel(new PrimitiveItemModel(molecule, this));
  }

  PrimitiveTreeView::PrimitiveTreeView(Engine *engine, QWidget *parent) : QTreeView(parent),
    d(new PrimitiveTreeViewPrivate)
  {
    initialize();
    setModel(new PrimitiveItemModel(engine, this));
  }

  void PrimitiveTreeView::initialize()
  {
    d->delegate = new PrimitiveItemDelegate(this, this);
    setItemDelegate(d->delegate);
    setRootIsDecorated(false);
    header()->hide();
    setUniformRowHeights(true);


    connect(this, SIGNAL(pressed(QModelIndex)), this, SLOT(itemPressed(QModelIndex)));
  }

  PrimitiveTreeView::~PrimitiveTreeView ()
  {
    delete d->delegate;
    delete d;
  }

  void PrimitiveTreeView::itemPressed(const QModelIndex & index)
  {
    QModelIndex parent = model()->parent(index);
    if(!parent.isValid())
    {
      setExpanded(index, !isExpanded(index));
    }
  }

  class PrimitiveItemDelegatePrivate
  {
    public:
      PrimitiveItemDelegatePrivate() : view(0) {}

      QTreeView *view;
  };

  PrimitiveItemDelegate::PrimitiveItemDelegate(QTreeView *view, QWidget *parent)
    : QItemDelegate(parent), d(new PrimitiveItemDelegatePrivate)
  {
    d->view = view;
  }

  void PrimitiveItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    if (!model->parent(index).isValid()) {
      // if we ever want to change up the style a bit.
      //dc:     QStyleOptionHeader headerOption;
      //dc:     headerOption.state = option.state;
      //dc:     headerOption.rect = option.rect;
      //dc:     headerOption.palette = option.palette;
      //dc:     d->view->style()->drawControl(QStyle::CE_HeaderSection, &dockOption, painter, d->view);

      QPen pen = painter->pen();

      // this is a top-level item.
      QStyleOptionButton buttonOption;

      buttonOption.state = option.state;
#ifdef Q_WS_MAC
      buttonOption.state |= QStyle::State_Raised;
#endif
      buttonOption.state &= ~QStyle::State_HasFocus;

      buttonOption.rect = option.rect;
      buttonOption.palette = option.palette;
      buttonOption.features = QStyleOptionButton::None;
      d->view->style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter, d->view);

      QStyleOption branchOption;
      static const int i = 9; // ### hardcoded in qcommonstyle.cpp
      QRect r = option.rect;
      branchOption.rect = QRect(r.left() + i/2, r.top() + (r.height() - i)/2, i, i);
      branchOption.palette = option.palette;
      branchOption.state = QStyle::State_Children;

      if (d->view->isExpanded(index))
      {
        branchOption.state |= QStyle::State_Open;
      }
      else
      {
        painter->setPen(Qt::darkGray);
      }

      d->view->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, d->view);

      // draw text
      QRect textrect = QRect(r.left() + i*2, r.top(), r.width() - ((5*i)/2), r.height());
      QString text = elidedText(option.fontMetrics, textrect.width(), Qt::ElideMiddle,
          model->data(index, Qt::DisplayRole).toString());
      d->view->style()->drawItemText(painter, textrect, Qt::AlignCenter,
          option.palette, d->view->isEnabled(), text);

      painter->setPen(pen);
    } else {
      QItemDelegate::paint(painter, option, index);
    }
  }

  QSize PrimitiveItemDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
  {
    QStyleOptionViewItem option = opt;
    QSize sz = QItemDelegate::sizeHint(opt, index) + QSize(2, 2);
    return sz;
  }

}

#include "primitivetreeview.moc"
