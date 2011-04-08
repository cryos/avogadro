/**********************************************************************
  CEParameterEditor

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

#ifndef CEPARAMETEREDITOR_H
#define CEPARAMETEREDITOR_H

#include "ceabstracteditor.h"
#include "../crystallographyextension.h"

#include <QtGui/QTextCharFormat>

#include "ui_ceparametereditor.h"

namespace Avogadro
{
  class CEParameterEditor : public CEAbstractEditor
  {
    Q_OBJECT

  public:
    CEParameterEditor(CrystallographyExtension *ext,
                      QMainWindow *w);
    virtual ~CEParameterEditor();

  signals:

  public slots:
    void refreshEditor();
    void lockEditor();
    void unlockEditor();

  protected slots:
    void markAsInvalid();
    void markAsValid();

    // Enable the apply/reset buttons
    void enableButtons();

    // Returns the current parameters
    CEUnitCellParameters validateEditor();

    // Creates and pushes an undo action while setting the current
    // parameters
    void setParameters();

  private:
    Ui::CEParameterEditor ui;

    QTextCharFormat m_charFormat;
  };
}

#endif
