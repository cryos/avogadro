/**********************************************************************
  ceabstracteditor.h Base class for crystal builder editor dockwidgets

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

#ifndef CEABSTRACTEDITOR_H
#define CEABSTRACTEDITOR_H

#include <QtGui/QDockWidget>

class QMainWindow;

namespace Avogadro
{
  class CrystallographyExtension;

  class CEAbstractEditor : public QDockWidget
  {
    Q_OBJECT

  public:
    CEAbstractEditor(CrystallographyExtension *ext, QMainWindow *w);
    virtual ~CEAbstractEditor();

    bool isLocked() {return m_isLocked;}

    Qt::DockWidgetArea preferredDockWidgetArea();

  signals:
    void editStarted();
    void editAccepted();
    void editRejected();

    void invalidInput();
    void validInput();
    void visibilityChanged();

  public slots:
    virtual void refreshEditor() = 0;
    virtual void lockEditor() = 0;
    virtual void unlockEditor() = 0;

  protected slots:
    virtual void markAsInvalid() = 0;
    virtual void markAsValid() = 0;

    void storeDockWidgetArea(Qt::DockWidgetArea a);

  protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    CrystallographyExtension *m_ext;
    bool m_isLocked;
  };

}

#endif
