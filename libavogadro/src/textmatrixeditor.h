/**********************************************************************
  TextMatrixEditor - Text editor that edits 3x3 matrices

  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#ifndef TEXTMATRIXEDITOR_H
#define TEXTMATRIXEDITOR_H

#include "config.h"

#include <avogadro/global.h> // For A_EXPORT

#include <QtGui/QTextEdit>

#include <Eigen/Core>

class QRegExp;

namespace Avogadro
{

/**
 * @class TextMatrixEditor textmatrixeditor.h <avogadro/textmatrixeditor.h>
 * @brief Widget to edit 3x3 matrices in plain text
 * @author David C. Lonie
 *
 * TextMatrixEditor is a QTextEdit that displays and allows editing of a 3x3
 * matrix as plain text. Input is validated with each keystroke, and the text
 * turns red if the input is invalid. The matrix can be set/get with matrix()
 * and setMatrix(). The delimiters used to separate columns can be set and
 * accessed through the pointer returned by delimiters(). The default
 * delimiter regexp is
 * QRegExp("\\s+|,|;|\\||\\[|\\]|\\{|\\}|\\(|\\)|\\&|/|<|>"). resetMatrix()
 * will replace the text with the last known good matrix.
 */
class A_EXPORT TextMatrixEditor : public QTextEdit
{
  Q_OBJECT
public:
  //! Constructor
  explicit TextMatrixEditor(QWidget *parent = 0);
  //! Destructor
  virtual ~TextMatrixEditor();

  //! @return Allowed column separators. See class definition for default.
  QRegExp * delimiters() {return this->m_delimiters;}
  //! @return Allowed column separators. See class definition for default
  const QRegExp * delimiters() const {return this->m_delimiters;}

  //! @return The current matrix
  Eigen::Matrix3d matrix() const;

signals:
  //! Emitted when the text cannot be parsed into a matrix
  void isInvalid();
  //! Emitted when the text has been parsed into a matrix
  void isValid();

public slots:
  //! Set the matrix. This will update the text.
  void setMatrix(const Eigen::Matrix3d &mat);
  //! Reset the text to the last known good matrix.
  void resetMatrix();

protected slots:
  /**
   * Check if input can be parsed. Sets m_matrix, emits isValid(), and returns
   * true if so. Otherwise emits isInvalid() and returns false.
   */
  bool validate();

  //! Turn the text red.
  void markAsInvalid();

  //! Reset the text appearance.
  void markAsValid();

protected:
  //! Stores the text properties while invalid.
  QTextCharFormat *m_charFormat;
  //! Acceptable column separators. See class definition for default.
  QRegExp *m_delimiters;
  //! Stores the current (or last known good) matrix.
  Eigen::Matrix3d m_matrix;
};

} // namespace Avogadro

#endif // TEXTMATRIXEDITOR_H
