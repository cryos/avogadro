/**********************************************************************
  EnginePrimitivesWidget - View for listing engines

  Copyright (C) 2007 Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
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

#include "engineprimitiveswidget.h"

#include "primitivetreeview.h"
#include "primitiveitemmodel.h"

#include <avogadro/engine.h>
#include <avogadro/glwidget.h>
#include <avogadro/primitivelist.h>

#include <QAbstractButton>
#include <QDialog>
#include <QStackedLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QVBoxLayout>

namespace Avogadro {

  class EnginePrimitivesWidgetPrivate
  {
    public:
      EnginePrimitivesWidgetPrivate() : glWidget(0), engine(0) {};

      GLWidget *glWidget;
      Engine *engine;
      QStackedLayout *settingsStacked;
      PrimitiveTreeView *primitiveTree;
      QWidget *blankWidget;
  };

  EnginePrimitivesWidget::EnginePrimitivesWidget( GLWidget *glWidget, QWidget *parent ) : QWidget(parent), d(new EnginePrimitivesWidgetPrivate)
  {
    ui.setupUi(this);

    d->glWidget = glWidget;

    d->blankWidget = new QWidget();

    ui.enginePrimitivesTree->header()->hide();

    ui.addSelectionButton->setEnabled(false);
    ui.removeSelectionButton->setEnabled(false);
    ui.resetButton->setEnabled(false);
    ui.fromSelectionButton->setEnabled(false);

    connect(ui.addSelectionButton, SIGNAL(clicked()),
        this, SLOT(addSelection()));
    connect(ui.removeSelectionButton, SIGNAL(clicked()),
        this, SLOT(removeSelection()));
    connect(ui.resetButton, SIGNAL(clicked()),
           this, SLOT(addAll()));
    connect(ui.fromSelectionButton, SIGNAL(clicked()),
           this, SLOT(fromSelection()));
  }

  EnginePrimitivesWidget::~EnginePrimitivesWidget()
  {
    delete d;
  }

  void EnginePrimitivesWidget::addSelection()
  {
    PrimitiveList selectedPrimitives = d->glWidget->selectedPrimitives();
    PrimitiveList list = d->engine->primitives();
    foreach(Primitive *p, selectedPrimitives)
    {
      if(!list.contains(p)) {
        d->engine->addPrimitive(p);
      }
    }
  }

  void EnginePrimitivesWidget::removeSelection()
  {
    PrimitiveList selectedPrimitives = d->glWidget->selectedPrimitives();
    foreach(Primitive *p, selectedPrimitives)
    {
      d->engine->removePrimitive(p);
    }
  }

  void EnginePrimitivesWidget::addAll()
  {
    d->engine->setPrimitives(d->glWidget->primitives());
  }

  void EnginePrimitivesWidget::toSelection()
  {
    PrimitiveList pl = d->glWidget->selectedPrimitives();
    if(pl.size())
    {
      d->engine->setPrimitives(pl);
    }
    else
    {
      d->engine->setPrimitives(d->glWidget->primitives());
    }
  }

  void EnginePrimitivesWidget::fromSelection()
  {
    PrimitiveList pl = d->glWidget->selectedPrimitives();
    if(pl.size())
    {
      d->engine->setPrimitives(pl);
    }
    else
    {
      d->engine->setPrimitives(d->glWidget->primitives());
    }
  }

  GLWidget *EnginePrimitivesWidget::glWidget() const
  {
    return d->glWidget;
  }

  void EnginePrimitivesWidget::setEngine( Engine *engine )
  {
    d->engine = engine;

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
    ui.resetButton->setEnabled(true);
    ui.fromSelectionButton->setEnabled(true);
  }

} // end namespace Avogadro

#include "engineprimitiveswidget.moc"
