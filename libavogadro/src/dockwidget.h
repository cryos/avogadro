/**********************************************************************
  DockWidget -- Dock widget for avogadro

  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 **********************************************************************/

#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QtGui/QDockWidget>

#include <avogadro/global.h> // For A_EXPORT

namespace Avogadro
{

class A_EXPORT DockWidget : public QDockWidget
{
  Q_OBJECT
public:
  explicit DockWidget(const QString &title,
                      QWidget *parent = 0, Qt::WindowFlags flags = 0);
  explicit DockWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  virtual ~DockWidget();

  virtual Qt::DockWidgetArea preferredWidgetDockArea();

public slots:
  void setPreferredDockWidgetArea(Qt::DockWidgetArea);

protected:
  Qt::DockWidgetArea m_preferredDockWidgetArea;

};

}
#endif // DOCKWIDGET_H
