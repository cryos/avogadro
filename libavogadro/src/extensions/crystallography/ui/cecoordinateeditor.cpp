/**********************************************************************
  CECoordinateEditor

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

#include "cecoordinateeditor.h"

#include "../ceundo.h"
#include "../crystallographyextension.h"

namespace Avogadro
{
  CECoordinateEditor::CECoordinateEditor(CrystallographyExtension *ext,
                                         QMainWindow *w)
    : CEAbstractEditor(ext, w)
  {
    ui.setupUi(this);

    // Emit editStarted
    connect(ui.edit_coords, SIGNAL(textChanged()),
            this, SIGNAL(editStarted()));

    // Apply button connections
    connect(ui.push_coords_apply, SIGNAL(clicked()),
            this, SIGNAL(editAccepted()));
    connect(ui.push_coords_apply, SIGNAL(clicked()),
            this, SLOT(setCoords()));

    // Reset button connections
    connect(ui.push_coords_reset, SIGNAL(clicked()),
            this, SIGNAL(editRejected()));
    connect(ui.push_coords_reset, SIGNAL(clicked()),
            this, SLOT(refreshEditor()));

    // Validation
    connect(ui.edit_coords, SIGNAL(textChanged()),
            this, SLOT(validateEditor()));

    // Apply/reset enable
    connect(ui.edit_coords, SIGNAL(textChanged()),
            this, SLOT(enableButtons()));

    ui.edit_coords->setCurrentFont(QFont(CE_FONT,
                                         CE_FONTSIZE));
    m_charFormat = ui.edit_coords->textCursor().charFormat();
  }

  CECoordinateEditor::~CECoordinateEditor()
  {
  }

  void CECoordinateEditor::refreshEditor()
  {
    QList<QString> symbols = m_ext->currentAtomicSymbols();

    QList<Eigen::Vector3d> pos;
    switch (m_ext->coordsCartFrac()) {
    case Cartesian:
      pos = m_ext->currentCartesianCoords();
      setWindowTitle(tr("Cartesian &Coordinates"));
      break;
    case Fractional:
      pos = m_ext->currentFractionalCoords();
      setWindowTitle(tr("Fractional &Coordinates"));
      break;
    }

    // Non-fatal assert. If the number of atoms changed
    // in between the two calls, tail-recurse this function.
    if (pos.size() != symbols.size()) {
      return refreshEditor();
    }

    // Clean up number display
    for (QList<Eigen::Vector3d>::iterator
           it = pos.begin(),
           it_end = pos.end();
         it != it_end; ++it) {
      // Remove negative zeros
      if (fabs(it->x()) < 1e-10) {
        it->x() = 0.0;
      }
      if (fabs(it->y()) < 1e-10) {
        it->y() = 0.0;
      }
      if (fabs(it->z()) < 1e-10) {
        it->z() = 0.0;
      }
    }
    QString text;

    char line[256];
    for (int i = 0; i < pos.size(); ++i) {
      snprintf(line, 256, "%2s %9.5f %9.5f %9.5f\n",
               symbols[i].toStdString().c_str(),
               pos[i][0], pos[i][1], pos[i][2]);
      text += line;
    }

    ui.edit_coords->blockSignals(true);
    ui.edit_coords->setText(text);
    ui.edit_coords->blockSignals(false);

    ui.edit_coords->setCurrentFont(QFont(CE_FONT,
                                         CE_FONTSIZE));

    this->setEnabled(true);
    ui.edit_coords->setEnabled(true);
    ui.edit_coords->setReadOnly(false);
    ui.push_coords_apply->setEnabled(false);
    ui.push_coords_reset->setEnabled(false);
    emit validInput();
  }

  void CECoordinateEditor::lockEditor()
  {
    ui.edit_coords->setEnabled(false);
  }

  void CECoordinateEditor::unlockEditor()
  {
    ui.edit_coords->setEnabled(true);
  }

  void CECoordinateEditor::markAsInvalid()
  {
    QTextCursor tc (ui.edit_coords->document());
    QTextCharFormat redFormat;
    redFormat.setForeground(QBrush(Qt::red));
    tc.movePosition(QTextCursor::Start);
    tc.movePosition(QTextCursor::End,
                    QTextCursor::KeepAnchor);
    ui.edit_coords->blockSignals(true);
    tc.mergeCharFormat(redFormat);
    ui.edit_coords->blockSignals(false);
    ui.edit_coords->setCurrentFont(QFont(CE_FONT,
                                         CE_FONTSIZE));
  }

  void CECoordinateEditor::markAsValid()
  {
    QTextCursor tc (ui.edit_coords->document());
    tc.movePosition(QTextCursor::Start);
    tc.movePosition(QTextCursor::End,
                    QTextCursor::KeepAnchor);
    ui.edit_coords->blockSignals(true);
    tc.setCharFormat(m_charFormat);
    ui.edit_coords->blockSignals(false);
    ui.edit_coords->setCurrentFont(QFont(CE_FONT,
                                         CE_FONTSIZE));
  }

  void CECoordinateEditor::enableButtons()
  {
    ui.push_coords_apply->setEnabled(true);
    ui.push_coords_reset->setEnabled(true);
  }

  void CECoordinateEditor::validateEditor()
  {
    // Clear selection, otherwise there is a crash on Qt 4.7.2.
    QTextCursor tc = ui.edit_coords->textCursor();
    tc.clearSelection();
    ui.edit_coords->setTextCursor(tc);

    QStringList list;
    QString text = ui.edit_coords->document()->toPlainText();
    QStringList lines = text.split("\n",
                                   QString::SkipEmptyParts);
    // Clean up lines
    for (QStringList::iterator
           it = lines.begin(),
           it_end = lines.end();
         it != it_end; ++it) {
      *it = it->trimmed();
    }
    lines.removeAll("");

    QStringList row;

    for (QStringList::const_iterator
           it = lines.constBegin(),
           it_end = lines.constEnd();
         it != it_end;
         ++it) {
      row = it->split(CE_PARSE_IGNORE_REGEXP,
                      QString::SkipEmptyParts);
      if (row.size() != 4) {
        emit invalidInput();
        return;
      }
      bool ok;
      row.at(1).toDouble(&ok);
      if (ok) {
        row.at(2).toDouble(&ok);
      }
      if (ok) {
        row.at(3).toDouble(&ok);
      }
      if (!ok) {
        emit invalidInput();
        return;
      }
    }

    emit validInput();
  }

  void CECoordinateEditor::setCoords()
  {
    // Clear selection, otherwise there is a crash on Qt 4.7.2.
    QTextCursor tc = ui.edit_coords->textCursor();
    tc.clearSelection();
    ui.edit_coords->setTextCursor(tc);

    QStringList list;
    QString text = ui.edit_coords->document()->toPlainText();
    QStringList lines = text.split("\n",
                                   QString::SkipEmptyParts);

    // Clean up lines
    for (QStringList::iterator
           it = lines.begin(),
           it_end = lines.end();
         it != it_end; ++it) {
      *it = it->trimmed();
    }
    lines.removeAll("");

    double x,y,z;
    QStringList ids;
    QStringList row;
    QList<Eigen::Vector3d> coords;
#if QT_VERSION >= 0x040700
    ids.reserve(lines.size());
    coords.reserve(lines.size());
#endif

    for (QStringList::const_iterator
           it = lines.constBegin(),
           it_end = lines.constEnd();
         it != it_end;
         ++it) {
      row = it->split(CE_PARSE_IGNORE_REGEXP,
                      QString::SkipEmptyParts);
      if (row.size() != 4) {
        emit invalidInput();
        return;
      }
      ids.append(row.at(0));
      bool ok;
      x = row.at(1).toDouble(&ok);
      if (ok) {
        y = row.at(2).toDouble(&ok);
      }
      if (ok) {
        z = row.at(3).toDouble(&ok);
      }
      if (!ok) {
        emit invalidInput();
        return;
      }
      coords.append(Eigen::Vector3d(x,y,z));
    }

    if (m_ext->coordsCartFrac() == Fractional) {
      CEUndoState before (m_ext);
      m_ext->setCurrentFractionalCoords(ids, coords);
      CEUndoState after (m_ext);
      m_ext->pushUndo(new CEUndoCommand (before, after,
                                         tr("Set Fractional Coordinates")));
    }
    else {
      CEUndoState before (m_ext);
      m_ext->setCurrentCartesianCoords(ids, coords);
      CEUndoState after (m_ext);
      m_ext->pushUndo(new CEUndoCommand (before, after,
                                         tr("Set Cartesian Coordinates")));
    }

    emit validInput();
  }
}
