/**********************************************************************
  CEMatrixEditor

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

    QTextCharFormat m_charFormat;

  };

}

#endif
