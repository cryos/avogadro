/**********************************************************************
  EngineTabWidget - View for listing engines

  Copyright (C) 2007 Donald Ephraim Curtis

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

#include "enginetabwidget.h"

#include "primitivetreeview.h"

#include <avogadro/engine.h>
#include <avogadro/glwidget.h>

#include <avogadro/primitiveitemmodel.h>

#include <QAbstractButton>
#include <QDialog>
#include <QStackedLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QVBoxLayout>

namespace Avogadro {

  class EngineTabWidgetPrivate
  {
    public:
      EngineTabWidgetPrivate() : glWidget(0), engine(0) {};

      GLWidget *glWidget;
      Engine *engine;
      QStackedLayout *settingsStacked;
      PrimitiveTreeView *primitiveTree;
  };

  EngineTabWidget::EngineTabWidget( GLWidget *glWidget, QWidget *parent ) : QWidget(parent), d(new EngineTabWidgetPrivate)
  {
    ui.setupUi(this);

    d->glWidget = glWidget;

    d->settingsStacked = new QStackedLayout(ui.engineSettingsWidget);
    d->settingsStacked->addWidget(new QWidget);
    foreach(Engine *engine, glWidget->engines())
    {
      QWidget *settingsWidget = engine->settingsWidget();
      if(settingsWidget)
      {
        d->settingsStacked->addWidget(settingsWidget);
      }
    }

    ui.enginePrimitivesTree->header()->hide();

    ui.addSelectionButton->setEnabled(false);
    ui.removeSelectionButton->setEnabled(false);

    connect(ui.addSelectionButton, SIGNAL(clicked()), 
        this, SLOT(addSelection()));
    connect(ui.removeSelectionButton, SIGNAL(clicked()), 
        this, SLOT(removeSelection()));
  }

  EngineTabWidget::~EngineTabWidget()
  {
    delete d;
  }
  
  void EngineTabWidget::addSelection()
  {
    QList<Primitive *> selection = d->glWidget->selection();
    PrimitiveList list = d->engine->primitiveList();
    foreach(Primitive *p, selection)
    {
      if(!list.contains(p)) {
        d->engine->addPrimitive(p);
      }
    }
  }

  void EngineTabWidget::removeSelection()
  {
    QList<Primitive *> selection = d->glWidget->selection();
    foreach(Primitive *p, selection)
    {
      d->engine->removePrimitive(p);
    }
  }

  GLWidget *EngineTabWidget::glWidget() const
  {
    return d->glWidget;
  }

  void EngineTabWidget::setCurrentEngine( Engine *engine )
  {
    d->engine = engine;
    QWidget *widget = engine->settingsWidget();
    if(widget)
    {
      d->settingsStacked->setCurrentWidget(engine->settingsWidget());
    } else {
      d->settingsStacked->setCurrentIndex(0);
    }

    QAbstractItemModel *model = ui.enginePrimitivesTree->model();

    if(model)
    {
      delete model;
    }

    model = new PrimitiveItemModel(engine, this);
    ui.enginePrimitivesTree->setModel(model);
    ui.enginePrimitivesTree->expandAll();

    ui.addSelectionButton->setEnabled(true);
    ui.removeSelectionButton->setEnabled(true);
  }

} // end namespace Avogadro

#include "enginetabwidget.moc"
