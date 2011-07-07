/**********************************************************************
  CEMatrixEditor

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

#ifndef CEMATRIXEDITOR_H
#define CEMATRIXEDITOR_H

#include "ceabstracteditor.h"

#include <Eigen/Core>

#include <QtGui/QTextCharFormat>

#include "ui_cematrixeditor.h"

namespace Avogadro
{
  class CEMatrixEditor : public CEAbstractEditor
  {
    Q_OBJECT

  public:
    CEMatrixEditor(CrystallographyExtension *ext,
                   QMainWindow *w);
    virtual ~CEMatrixEditor();

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

    // Return the input matrix if valid, Eigen::Matrix3d::Zero
    // otherwise
    Eigen::Matrix3d validateEditor();

    // Creates and pushes an undo action while setting the current
    // matrix
    void setMatrix();

  private:
    Ui::CEMatrixEditor ui;
  };

}

#endif
