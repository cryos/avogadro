/**********************************************************************
  TextMatrixEditor - Text editor that edits 3x3 matrices

  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "textmatrixeditor.h"

#include <QtCore/QRegExp>

#include <stdio.h> // for snprintf

namespace Avogadro {

TextMatrixEditor::TextMatrixEditor(QWidget *parent) :
  QTextEdit(parent), m_charFormat(0),
  m_delimiters(new QRegExp("\\s+|,|;|\\||\\[|\\]|\\{|\\}|\\(|\\)|\\&|/|<|>"))
{
  this->m_matrix.fill(0.0);

  // Validate on every keystroke
  this->connect(this, SIGNAL(textChanged()), SLOT(validate()));

  // Provide validation feedback
  this->connect(this, SIGNAL(isInvalid()), SLOT(markAsInvalid()));
  this->connect(this, SIGNAL(isValid()), SLOT(markAsValid()));
}

TextMatrixEditor::~TextMatrixEditor()
{}

Eigen::Matrix3d TextMatrixEditor::matrix() const
{
  return this->m_matrix;
}

void TextMatrixEditor::setMatrix(const Eigen::Matrix3d &mat)
{
  this->m_matrix = mat;

  // Clean up matrix
  for (unsigned short row = 0; row < 3; ++row) {
    for (unsigned short col = 0; col < 3; ++col) {
      double &current = this->m_matrix(row,col);
      // Remove negative zeros:
      if (fabs(current) < 1e-10) {
        current = 0.0;
      }
    }
  }

  char text[128]; // actual size 91 with null
  snprintf(text, 128,
           "%9.5f %9.5f %9.5f\n"
           "%9.5f %9.5f %9.5f\n"
           "%9.5f %9.5f %9.5f\n",
           this->m_matrix(0, 0), this->m_matrix(0, 1), this->m_matrix(0, 2),
           this->m_matrix(1, 0), this->m_matrix(1, 1), this->m_matrix(1, 2),
           this->m_matrix(2, 0), this->m_matrix(2, 1), this->m_matrix(2, 2));

  this->setText(text);
  emit this->isValid();
}

void TextMatrixEditor::resetMatrix()
{
  this->setMatrix(this->m_matrix);
}

bool TextMatrixEditor::validate()
{
  // Clear selection, otherwise there is a crash on Qt 4.7.2.
  QTextCursor tc = this->textCursor();
  tc.clearSelection();
  this->setTextCursor(tc);

  QString text = this->document()->toPlainText();
  QStringList lines = text.split("\n", QString::SkipEmptyParts);
  if (lines.size() != 3) {
    emit this->isInvalid();
    return false;
  }

  QList<QStringList> stringVecs;
  Eigen::Matrix3d mat;
  for (int row = 0; row < 3; ++row) {
    stringVecs.append(lines.at(row).simplified()
                      .split(*(this->m_delimiters), QString::SkipEmptyParts));
    QStringList &stringVec = stringVecs[row];
    if (stringVec.size() != 3) {
      emit this->isInvalid();
      return false;
    }
    for (int col = 0; col < 3; ++col) {
      bool ok;
      double val = stringVec[col].toDouble(&ok);
      if (!ok) {
        emit this->isInvalid();
        return false;
      }
      mat(row,col) = val;
    }
  }

  emit this->isValid();
  m_matrix = mat;
  return true;
}

void TextMatrixEditor::markAsInvalid()
{
  if (this->m_charFormat)
    return;

  this->m_charFormat = new QTextCharFormat(this->textCursor().charFormat());

  QTextCursor tc (this->document());
  QTextCharFormat redFormat;
  redFormat.setForeground(QBrush(Qt::red));
  tc.movePosition(QTextCursor::Start);
  tc.movePosition(QTextCursor::End,
                  QTextCursor::KeepAnchor);

  this->blockSignals(true);
  tc.mergeCharFormat(redFormat);
  this->blockSignals(false);
}

void TextMatrixEditor::markAsValid()
{
  if (this->m_charFormat == NULL)
    return;

  QTextCursor tc (this->document());
  tc.movePosition(QTextCursor::Start);
  tc.movePosition(QTextCursor::End,
                  QTextCursor::KeepAnchor);

  this->blockSignals(true);
  tc.setCharFormat(*(this->m_charFormat));
  this->blockSignals(false);

  this->m_charFormat = NULL;
}

} // namespace Avogadro
