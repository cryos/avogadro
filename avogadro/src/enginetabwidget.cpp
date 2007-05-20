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

#include <avogadro/engine.h>
#include <avogadro/glwidget.h>

#include <QAbstractButton>
#include <QDialog>
#include <QStackedLayout>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace Avogadro {

  class EngineTabWidgetPrivate
  {
    public:
      EngineTabWidgetPrivate() : glWidget(0) {};

      GLWidget *glWidget;
      QStackedLayout *settingsStacked;
  };

  EngineTabWidget::EngineTabWidget( GLWidget *glWidget, QWidget *parent ) : d(new EngineTabWidgetPrivate)
  {
    d->glWidget = glWidget;

    QWidget *widget = new QWidget(this);
    
    d->settingsStacked = new QStackedLayout(widget);
    d->settingsStacked->addWidget(new QWidget);
    foreach(Engine *engine, glWidget->engines())
    {
      QWidget *settingsWidget = engine->settingsWidget();
      if(settingsWidget)
      {
        d->settingsStacked->addWidget(settingsWidget);
      }
    }

    addTab(widget, tr("&Settings"));
    setTabPosition( QTabWidget::South );
  }

  EngineTabWidget::~EngineTabWidget()
  {
    delete d;
  }

  GLWidget *EngineTabWidget::glWidget() const
  {
    return d->glWidget;
  }

  void EngineTabWidget::setCurrentEngine( Engine *engine )
  {
    QWidget *widget = engine->settingsWidget();
    if(widget)
    {
      d->settingsStacked->setCurrentWidget(engine->settingsWidget());
    } else {
      d->settingsStacked->setCurrentIndex(0);
    }
  }

} // end namespace Avogadro

#include "enginetabwidget.moc"
