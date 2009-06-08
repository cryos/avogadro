/**********************************************************************
  EngineViewWidget - custom engine table view for engine items

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "engineviewwidget.h"

#include "engineitemmodel.h"
#include "engineitemdelegate.h"

#include <avogadro/engine.h>
#include <avogadro/glwidget.h>

#include <QtGui/QTreeView>
#include <QtGui/QHeaderView>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QVBoxLayout>

namespace Avogadro {

  EngineViewWidget::EngineViewWidget(GLWidget *glWidget, QWidget *parent)
    : QWidget(parent), m_glWidget(glWidget)
  {
    m_view = new QTreeView(this);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_view);
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    setLayout(layout);
    m_model = new EngineItemModel(m_glWidget, m_view);

    // Sort the engines alphabetically
    QSortFilterProxyModel *sortModel = new QSortFilterProxyModel(m_view);
    sortModel->setSourceModel(m_model);
    m_view->setModel(sortModel);
    sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    sortModel->setSortLocaleAware(true);
    sortModel->setDynamicSortFilter(true);
    sortModel->sort(0, Qt::AscendingOrder);
		
    connect(m_view, SIGNAL(clicked(QModelIndex)),
            this, SLOT(selectEngine(QModelIndex)));
        
    // Set up the tree view
    m_view->setAlternatingRowColors(true);
    m_view->header()->hide();
    m_view->setIndentation(0);
    m_view->setTextElideMode(Qt::ElideMiddle);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->header()->setStretchLastSection(false);
    m_view->header()->setResizeMode(0, QHeaderView::Stretch);
    m_view->header()->setResizeMode(1, QHeaderView::Fixed);
    m_view->header()->resizeSection(1, 25);

    // Use the engine item delegate to display
    EngineItemDelegate *eid = new EngineItemDelegate(parent);
    m_view->setItemDelegate(eid);
  }

  EngineViewWidget::~EngineViewWidget()
  {
  }

  GLWidget *EngineViewWidget::glWidget() const
  {
    return m_glWidget;
  }

  void EngineViewWidget::selectEngine(const QModelIndex &index)
  {
    Engine *engine = index.data(EngineItemModel::EngineRole).value<Engine *>();
    if(engine) {
      if(index.column() == 0)
        emit clicked(engine);
      else if(index.column() == 1 && engine->hasSettings())
        emit settings(engine);
    }
  }

  void EngineViewWidget::clear()
  {
    m_model->clear();
  }

} // end namespace Avogadro

#include "engineviewwidget.moc"
