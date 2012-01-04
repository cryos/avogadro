/**********************************************************************
  cetranslatewidget.h

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

#ifndef CETRANSLATEWIDGET_H
#define CETRANSLATEWIDGET_H

#include "ceabstractdockwidget.h"
#include "ui_cetranslatewidget.h"

#include <QtCore/QTimer>
#include <QtGui/QDockWidget>

#include <Eigen/Core>

class QMainWindow;

namespace Avogadro
{
  class CrystallographyExtension;
  class GLWidget;

  class CETranslateWidget : public CEAbstractDockWidget
  {
    Q_OBJECT

  public:
    CETranslateWidget(CrystallographyExtension *ext, QMainWindow *w,
                      GLWidget *gl);
    virtual ~CETranslateWidget();

    enum TranslateMode {
      TM_VECTOR = 0,
      TM_ATOM
    };

  signals:
    // Too many/few atoms selected
    void invalidSelection();
    // One atom selected
    void validSelection();

  protected slots:
    void updateGui();
    void updateVector();
    void translate();
    // GLWidget doesn't have a selectionChanged signal, so we will
    // poll the selections using this slot and m_selectionTimer
    void checkSelection();
    void disableVectorEditor();
    void enableVectorEditor();
    void setError(const QString &err);
    void clearError();

  protected:
    void readSettings();
    void writeSettings();

    Eigen::Vector3d m_vector;
    QTimer m_selectionTimer;

    GLWidget *m_gl;

  private:
    Ui::CETranslateWidget ui;
  };

}

#endif
