/**********************************************************************
  ceabstractdockwidget.h Base class for crystal builder dockwidgets

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#include "ceabstractdockwidget.h"

#include <QtGui/QMainWindow>

#include "../crystallographyextension.h"

namespace Avogadro
{

  CEAbstractDockWidget::CEAbstractDockWidget(CrystallographyExtension *ext,
                                             QMainWindow *w)
    : QDockWidget(w),
      m_ext(ext)
  {
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(storeDockWidgetArea(Qt::DockWidgetArea)));
  }

  CEAbstractDockWidget::~CEAbstractDockWidget()
  {

  }

  void CEAbstractDockWidget::closeEvent(QCloseEvent *event)
  {
    emit visibilityChanged();
    QDockWidget::closeEvent(event);
  }

  void CEAbstractDockWidget::hideEvent(QHideEvent *event)
  {
    emit visibilityChanged();
    QDockWidget::hideEvent(event);
  }

  void CEAbstractDockWidget::showEvent(QShowEvent *event)
  {
    emit visibilityChanged();
    QDockWidget::showEvent(event);
  }

  void CEAbstractDockWidget::storeDockWidgetArea(Qt::DockWidgetArea a)
  {
    QSettings settings;
    settings.setValue("crystallographyextension/dockwidgets/"
                      + QString(this->metaObject()->className())
                      + "/area", a);
  }

  Qt::DockWidgetArea CEAbstractDockWidget::preferredDockWidgetArea()
  {
    QSettings settings;
    return static_cast<Qt::DockWidgetArea>
      (settings.value("crystallographyextension/dockwidgets/"
                      + QString(this->metaObject()->className())
                      + "/area", Qt::RightDockWidgetArea).toInt());
  }
}

