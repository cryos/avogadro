/**********************************************************************
  CEMatrixEditor

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

#include "cematrixeditor.h"

#include "../ceundo.h"
#include "../crystallographyextension.h"

namespace Avogadro
{
  CEMatrixEditor::CEMatrixEditor(CrystallographyExtension *ext,
                                 QMainWindow *w)
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

    // Forward signals from the TextMatrixEditor
    this->connect(this->ui.edit_matrix, SIGNAL(isInvalid()),
                  SIGNAL(invalidInput()));
    this->connect(this->ui.edit_matrix, SIGNAL(isValid()),
                  SIGNAL(validInput()));

    ui.edit_matrix->setCurrentFont(QFont(CE_FONT,
                                         CE_FONTSIZE));

    *(this->ui.edit_matrix->delimiters()) = CE_PARSE_IGNORE_REGEXP;
  }

  CEMatrixEditor::~CEMatrixEditor()
  {
  }

  void CEMatrixEditor::refreshEditor()
  {
    Eigen::Matrix3d mat;

    QString vectorStyleStr = "";

    switch (m_ext->matrixCartFrac()) {
    case Cartesian:
      switch (m_ext->matrixVectorStyle()) {
      case RowVectors:
        vectorStyleStr = "Row Vectors";
        mat = m_ext->currentCellMatrix();
        break;
      case ColumnVectors:
        vectorStyleStr = "Column Vectors";
        mat = m_ext->currentCellMatrix().transpose();
        break;
      }
      this->setWindowTitle(tr("Cell &Matrix (%1)",
                              "Unit cell matrix, arg is vector style "
                              "(row or column")
                           .arg(vectorStyleStr));
      break;
    case Fractional:
      switch (m_ext->matrixVectorStyle()) {
      case RowVectors:
        vectorStyleStr = "Row Vectors";
        mat = m_ext->currentFractionalMatrix();
        break;
      case ColumnVectors:
        vectorStyleStr = "Column Vectors";
        // TODO check math here:
        mat = m_ext->currentFractionalMatrix().transpose();
        break;
      }
      this->setWindowTitle(tr("Fractional Cell &Matrix (%1)",
                              "Unit cell matrix, arg is vector style "
                              "(row or column")
                           .arg(vectorStyleStr));
      break;
    }

    ui.edit_matrix->blockSignals(true);
    ui.edit_matrix->setMatrix(mat);
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
    // Handled internally by TextMatrixEditor
  }

  void CEMatrixEditor::markAsValid()
  {
    // Handled internally by TextMatrixEditor
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

    Eigen::Matrix3d mat (this->ui.edit_matrix->matrix());

    // Transpose if needed
    if (m_ext->matrixVectorStyle() == ColumnVectors) {
      // Warning: mat = mat.transpose() will *not* work correctly with
      // Eigen matrices. Use transposeInPlace() instead.
      mat.transposeInPlace();
    }

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
