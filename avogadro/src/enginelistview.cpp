/**********************************************************************
  EngineListView - View for listing engines

  Copyright (C) 2007 by Geoffrey R. Hutchison

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

#include "enginelistview.h"

#include "engineitemmodel.h"

#include <avogadro/engine.h>
#include <avogadro/glwidget.h>

#include <QStandardItemModel>
#include <QAbstractButton>
#include <QStandardItem>
#include <QVBoxLayout>
#include <QDialog>

namespace Avogadro {

  class EngineListViewPrivate
  {
    public:
      EngineListViewPrivate() : glWidget(0) {};

      GLWidget *glWidget;
  };

  EngineListView::EngineListView( GLWidget *glWidget, QWidget *parent ) : QListView(parent), d(new EngineListViewPrivate)
  {
    d->glWidget = glWidget;

    EngineItemModel *m = new EngineItemModel(d->glWidget, this);

    if(model())
    {
      delete model();
    }

    setModel(m);
    connect(this, SIGNAL(clicked(QModelIndex)), 
        this, SLOT(selectEngine(QModelIndex)));
    connect(m, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
        glWidget, SLOT(update()));
  }

  EngineListView::~EngineListView()
  {
    delete d;
  }

  GLWidget *EngineListView::glWidget() const
  {
    return d->glWidget;
  }

  void EngineListView::selectEngine( const QModelIndex &index )
  {
    Engine *engine = model()->data(index, EngineItemModel::EngineRole).value<Engine *>();
    if(engine) {
      emit clicked(engine);
    }
  }

} // end namespace Avogadro

#include "enginelistview.moc"
