/**********************************************************************
  ceviewoptions.h

  Copyright (C) 2011 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#ifndef CEVIEWOPTIONS_H
#define CEVIEWOPTIONS_H

#include "ceabstracteditor.h"
#include "ui_ceviewoptions.h"

class QMainWindow;

namespace Avogadro
{
  class CrystallographyExtension;
  class GLWidget;

  class CEViewOptions : public CEAbstractEditor
  {
    Q_OBJECT

  public:
    CEViewOptions(CrystallographyExtension *ext, QMainWindow *w);
    ~CEViewOptions() {}

    // Override editor methods
    void refreshEditor() {}
    void lockEditor() {}
    void unlockEditor() {}
    void markAsInvalid() {}
    void markAsValid() {}

  protected slots:
    void updateRepeatCells();
    void updateMillerPlane();

    void cellChanged();

  private:
    Ui::CEViewOptions ui;
  };

}

#endif
