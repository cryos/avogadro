/**********************************************************************
  DockWidget -- Dock widget for avogadro

  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 **********************************************************************/

#include "dockwidget.h"

namespace Avogadro
{

DockWidget::DockWidget(const QString &title,
                       QWidget *parent, Qt::WindowFlags flags)
  : QDockWidget(title, parent, flags),
    m_preferredDockWidgetArea(Qt::RightDockWidgetArea)
{
}

DockWidget::DockWidget(QWidget *parent, Qt::WindowFlags flags)
  : QDockWidget(parent, flags),
    m_preferredDockWidgetArea(Qt::RightDockWidgetArea)
{
}

DockWidget::~DockWidget()
{
}

Qt::DockWidgetArea DockWidget::preferredWidgetDockArea()
{
  return this->m_preferredDockWidgetArea;
}

void DockWidget::setPreferredDockWidgetArea(Qt::DockWidgetArea pref)
{
  this->m_preferredDockWidgetArea = pref;
}

}
