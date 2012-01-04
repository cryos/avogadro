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

#ifndef CEABSTRACTDOCKWIDGET_H
#define CEABSTRACTDOCKWIDGET_H

#include <QtGui/QDockWidget>

class QMainWindow;

namespace Avogadro
{
  class CrystallographyExtension;

  class CEAbstractDockWidget : public QDockWidget
  {
    Q_OBJECT

  public:
    CEAbstractDockWidget(CrystallographyExtension *ext, QMainWindow *w);
    virtual ~CEAbstractDockWidget();

    Qt::DockWidgetArea preferredDockWidgetArea();

  signals:
    void visibilityChanged();

  protected slots:
    virtual void storeDockWidgetArea(Qt::DockWidgetArea a);

  protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    CrystallographyExtension *m_ext;
  };

}

#endif
