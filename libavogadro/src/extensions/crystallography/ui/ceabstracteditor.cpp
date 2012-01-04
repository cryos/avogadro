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

#include "ceabstracteditor.h"

#include <QtGui/QMainWindow>

#include <QtCore/QSettings>

#include "../crystallographyextension.h"

namespace Avogadro
{

  CEAbstractEditor::CEAbstractEditor(CrystallographyExtension *ext,
                                     QMainWindow *w)
    : CEAbstractDockWidget(ext, w),
      m_isLocked(false)
  {
    connect(this, SIGNAL(invalidInput()),
            this, SLOT(markAsInvalid()));
    connect(this, SIGNAL(validInput()),
            this, SLOT(markAsValid()));

    connect(m_ext, SIGNAL(cellChanged()),
            this, SLOT(refreshEditor()));

    connect(this, SIGNAL(visibilityChanged()),
            m_ext, SLOT(refreshActions()));

    connect(this, SIGNAL(editStarted()),
            m_ext, SLOT(lockEditors()));
    connect(this, SIGNAL(editAccepted()),
            m_ext, SLOT(unlockEditors()));
    connect(this, SIGNAL(editRejected()),
            m_ext, SLOT(unlockEditors()));

  }

  CEAbstractEditor::~CEAbstractEditor()
  {

  }

}

