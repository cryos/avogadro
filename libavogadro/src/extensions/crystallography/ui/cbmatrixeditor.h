/**********************************************************************
  CBMatrixEditor

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

#ifndef CBMATRIXEDITOR_H
#define CBMATRIXEDITOR_H

#include "cbabstracteditor.h"

#include <Eigen/Core>

#include <QtGui/QTextCharFormat>

#include "ui_cbmatrixeditor.h"

namespace Avogadro
{
  class CBMatrixEditor : public CBAbstractEditor
  {
    Q_OBJECT

  public:
    CBMatrixEditor(CrystallographyExtension *ext,
                   QMainWindow *w);
    virtual ~CBMatrixEditor();

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
    Ui::CBMatrixEditor ui;

    QTextCharFormat m_charFormat;

  };

}

#endif
