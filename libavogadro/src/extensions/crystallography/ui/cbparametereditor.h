/**********************************************************************
  CBParameterEditor

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

#ifndef CBPARAMETEREDITOR_H
#define CBPARAMETEREDITOR_H

#include "cbabstracteditor.h"
#include "../crystallographyextension.h"

#include <QtGui/QTextCharFormat>

#include "ui_cbparametereditor.h"

namespace Avogadro
{
  class CBParameterEditor : public CBAbstractEditor
  {
    Q_OBJECT

  public:
    CBParameterEditor(CrystallographyExtension *ext,
                      QMainWindow *w);
    virtual ~CBParameterEditor();

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
    CBUnitCellParameters validateEditor();

    // Creates and pushes an undo action while setting the current
    // parameters
    void setParameters();

  private:
    Ui::CBParameterEditor ui;

    QTextCharFormat m_charFormat;
  };
}

#endif
