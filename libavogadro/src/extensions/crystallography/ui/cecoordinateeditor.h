/**********************************************************************
  CECoordinateEditor

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

#ifndef CECOORDINATEEDITOR_H
#define CECOORDINATEEDITOR_H

#include "ceabstracteditor.h"

#include <QtGui/QTextCharFormat>

#include "ui_cecoordinateeditor.h"

namespace Avogadro
{
  class CECoordinateEditor : public CEAbstractEditor
  {
    Q_OBJECT

  public:
    CECoordinateEditor(CrystallographyExtension *ext,
                       QMainWindow *w);
    virtual ~CECoordinateEditor();

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

    void validateEditor();

    // Creates and pushes an undo action while setting the current
    // coordinates
    void setCoords();

  private:
    Ui::CECoordinateEditor ui;

    QTextCharFormat m_charFormat;

  };

}

#endif
