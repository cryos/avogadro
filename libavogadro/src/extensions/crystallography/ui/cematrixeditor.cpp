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

#include "cematrixeditor.h"

#include "../ceundo.h"
#include "../crystallographyextension.h"

namespace Avogadro
{
  CEMatrixEditor::CEMatrixEditor(CrystallographyExtension *ext, QMainWindow *w)
    : CEAbstractEditor(ext, w)
  {
    ui.setupUi(this);

    // Emit editStarted
    connect(ui.edit_matrix, SIGNAL(textChanged()),
            this, SIGNAL(editStarted()));

    // Apply button connections
    connect(ui.push_matrix_apply, SIGNAL(clicked()),
            this, SIGNAL(editAccepted()));
    connect(ui.push_matrix_apply, SIGNAL(clicked()),
            this, SLOT(setMatrix()));

    // Reset button connections
    connect(ui.push_matrix_reset, SIGNAL(clicked()),
            this, SIGNAL(editRejected()));
    connect(ui.push_matrix_reset, SIGNAL(clicked()),
            this, SLOT(refreshEditor()));

    // Validation
    connect(ui.edit_matrix, SIGNAL(textChanged()),
            this, SLOT(validateEditor()));

    // Apply/reset enable
    connect(ui.edit_matrix, SIGNAL(textChanged()),
            this, SLOT(enableButtons()));

    ui.edit_matrix->setCurrentFont(QFont("Monospace",
                                         CE_FONTSIZE));
    m_charFormat = ui.edit_matrix->textCursor().charFormat();
  }

  CEMatrixEditor::~CEMatrixEditor()
  {
  }

  void CEMatrixEditor::refreshEditor()
  {
    Eigen::Matrix3d mat;

    switch (m_ext->matrixCartFrac()) {
    case Cartesian:
      switch (m_ext->matrixVectorStyle()) {
      case RowVectors:
        mat = m_ext->currentCellMatrix();
        break;
      case ColumnVectors:
        mat = m_ext->currentCellMatrix().transpose();
        break;
      }
      this->setWindowTitle(tr("Cell &Matrix", "Unit cell matrix"));
      break;
    case Fractional:
      switch (m_ext->matrixVectorStyle()) {
      case RowVectors:
        mat = m_ext->currentFractionalMatrix();
        break;
      case ColumnVectors:
        // TODO check math here:
        mat = m_ext->currentFractionalMatrix().transpose();
        break;
      }
      setWindowTitle(tr("Fractional Cell &Matrix",
                        "Fractional unit cell matrix"));
      break;
    }

    // Clean up matrix
    for (unsigned short row = 0; row < 3; ++row) {
      for (unsigned short col = 0; col < 3; ++col) {
        double &current = mat(row,col);
        // Remove negative zeros:
        if (fabs(current) < 1e-10) {
          current = 0.0;
        }
      }
    }

    QString text =
      QString("%1 %2 %3\n"
              "%4 %5 %6\n"
              "%7 %8 %9")
      .arg(mat(0, 0), -9, 'f', 5, '0')
      .arg(mat(0, 1), -9, 'f', 5, '0')
      .arg(mat(0, 2), -9, 'f', 5, '0')
      .arg(mat(1, 0), -9, 'f', 5, '0')
      .arg(mat(1, 1), -9, 'f', 5, '0')
      .arg(mat(1, 2), -9, 'f', 5, '0')
      .arg(mat(2, 0), -9, 'f', 5, '0')
      .arg(mat(2, 1), -9, 'f', 5, '0')
      .arg(mat(2, 2), -9, 'f', 5, '0');

    ui.edit_matrix->blockSignals(true);
    ui.edit_matrix->setText(text);
    ui.edit_matrix->blockSignals(false);

    ui.edit_matrix->setCurrentFont(QFont("Monospace",
                                         CE_FONTSIZE));

    ui.edit_matrix->setEnabled(true);

    if (m_ext->matrixCartFrac() == Fractional) {
      ui.edit_matrix->setReadOnly(true);
    }
    else {
      ui.edit_matrix->setReadOnly(false);
    }
    ui.push_matrix_apply->setEnabled(false);
    ui.push_matrix_reset->setEnabled(false);
    emit validInput();
  }

  void CEMatrixEditor::lockEditor()
  {
    ui.edit_matrix->setEnabled(false);
  }

  void CEMatrixEditor::unlockEditor()
  {
    ui.edit_matrix->setEnabled(true);
  }

  void CEMatrixEditor::markAsInvalid()
  {
    QTextCursor tc (ui.edit_matrix->document());
    QTextCharFormat redFormat;
    redFormat.setForeground(QBrush(Qt::red));
    tc.movePosition(QTextCursor::Start);
    tc.movePosition(QTextCursor::End,
                    QTextCursor::KeepAnchor);
    ui.edit_matrix->blockSignals(true);
    tc.mergeCharFormat(redFormat);
    ui.edit_matrix->blockSignals(false);
    ui.edit_matrix->setCurrentFont(QFont("Monospace",
                                         CE_FONTSIZE));
  }

  void CEMatrixEditor::markAsValid()
  {
    QTextCursor tc (ui.edit_matrix->document());
    tc.movePosition(QTextCursor::Start);
    tc.movePosition(QTextCursor::End,
                    QTextCursor::KeepAnchor);
    ui.edit_matrix->blockSignals(true);
    tc.setCharFormat(m_charFormat);
    ui.edit_matrix->blockSignals(false);
    ui.edit_matrix->setCurrentFont(QFont("Monospace",
                                         CE_FONTSIZE));
  }

  void CEMatrixEditor::enableButtons()
  {
    ui.push_matrix_apply->setEnabled(true);
    ui.push_matrix_reset->setEnabled(true);
  }

  Eigen::Matrix3d CEMatrixEditor::validateEditor()
  {
    // Editing fractional matrix is not allowed. The widget is
    // disabled to ensure that this assertion passes.
    Q_ASSERT(m_ext->matrixCartFrac() != Fractional);

    QString text = ui.edit_matrix->document()->toPlainText();
    QStringList lines = text.split("\n",
                                   QString::SkipEmptyParts);
    if (lines.size() != 3) {
      emit invalidInput();
      return Eigen::Matrix3d::Zero();
    }

    QList<QStringList> stringVecs;
    Eigen::Matrix3d mat;
    for (int row = 0; row < 3; ++row) {
      stringVecs.append(lines.at(row).simplified()
                        .split(QRegExp("\\s+|,|;")));
      QStringList &stringVec = stringVecs[row];
      if (stringVec.size() != 3) {
        emit invalidInput();
        return Eigen::Matrix3d::Zero();
      }
      for (int col = 0; col < 3; ++col) {
        bool ok;
        double val = stringVec[col].toDouble(&ok);
        if (!ok) {
          emit invalidInput();
          return Eigen::Matrix3d::Zero();
        }
        mat(row,col) = val;
      }
    }

    if (m_ext->matrixVectorStyle() == ColumnVectors) {
      mat = mat.transpose();
    }

    emit validInput();
    return mat;
  }

  void CEMatrixEditor::setMatrix()
  {
    Eigen::Matrix3d mat = validateEditor();
    if (mat.isZero()) {
      return;
    }
    CEUndoState before (m_ext);
    m_ext->setCurrentCellMatrix(mat);
    CEUndoState after (m_ext);
    m_ext->pushUndo(new CEUndoCommand (before, after,
                                tr("Set Unit Cell Matrix")));
  }
}
