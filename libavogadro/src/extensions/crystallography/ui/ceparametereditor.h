/**********************************************************************
  CEParameterEditor

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
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
