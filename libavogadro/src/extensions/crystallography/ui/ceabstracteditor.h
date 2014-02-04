/**********************************************************************
  ceabstracteditor.h Base class for crystal builder editor dockwidgets

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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
#include "ceabstractdockwidget.h"

class QMainWindow;

namespace Avogadro
{
  class CrystallographyExtension;

  class CEAbstractEditor : public CEAbstractDockWidget
  {
    Q_OBJECT

  public:
    CEAbstractEditor(CrystallographyExtension *ext);
    virtual ~CEAbstractEditor();

    bool isLocked() {return m_isLocked;}

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

  protected:
    bool m_isLocked;
  };

}

#endif
