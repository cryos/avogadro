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

#include <avogadro/engine.h>
#include <avogadro/glwidget.h>

#include <QStandardItemModel>
#include <QStandardItem>

namespace Avogadro {

  class EngineListViewPrivate
  {
    public:
      EngineListViewPrivate() : widget(0) {};

      GLWidget *widget;
  };

  EngineListView::EngineListView( QWidget *parent ) : d(new EngineListViewPrivate)
  {
  }

  EngineListView::~EngineListView()
  {
    delete d;
  }

  GLWidget *EngineListView::glWidget() const
  {
    return d->widget;
  }

  void EngineListView::setGLWidget(GLWidget *widget)
  {
    d->widget = widget;

    QStandardItemModel *m = new QStandardItemModel(this);

    QList<Engine *> engines = widget->engines();
    int size = engines.size();
    for(int i=0; i<size; i++)
    {
      Engine *e = engines.at(i);
      QStandardItem *item = new QStandardItem(e->name());
      item->setCheckable(true);
      item->setToolTip(e->description());
      if( e->name() == "Dynamic Ball and Stick" ) e->setEnabled(true);
      if(e->isEnabled()) {
        item->setCheckState(Qt::Checked);
      }
      item->setData(qVariantFromValue(e));
      m->appendRow(item);
    }

    if(model())
    {
      delete model();
    }

    setModel(m);
    connect(m, SIGNAL(itemChanged(QStandardItem *)), 
        this, SLOT(updateEngine(QStandardItem *)));
  }

  void EngineListView::updateEngine( QStandardItem *item )
  {
    Engine *engine = item->data().value<Engine *>();
    if(engine) {
      engine->setEnabled(item->checkState());
      d->widget->update();
    }
  }


} // end namespace Avogadro

#include "enginelistview.moc"
