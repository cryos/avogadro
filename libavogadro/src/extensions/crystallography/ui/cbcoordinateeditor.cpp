/**********************************************************************
  CBCoordinateEditor

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

#include "cbcoordinateeditor.h"

#include "../cbundo.h"
#include "../crystallographyextension.h"

namespace Avogadro
{
  CBCoordinateEditor::CBCoordinateEditor(CrystallographyExtension *ext,
                                         QMainWindow *w)
    : CBAbstractEditor(ext, w)
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

    ui.edit_coords->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));
    m_charFormat = ui.edit_coords->textCursor().charFormat();
  }

  CBCoordinateEditor::~CBCoordinateEditor()
  {
  }

  void CBCoordinateEditor::refreshEditor()
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

    for (int i = 0; i < pos.size(); ++i) {
      text += QString("%1 %2 %3 %4\n")
        .arg(symbols[i], -2)
        .arg(pos[i][0], -9, 'f', 5, '0')
        .arg(pos[i][1], -9, 'f', 5, '0')
        .arg(pos[i][2], -9, 'f', 5, '0');
    }

    ui.edit_coords->blockSignals(true);
    ui.edit_coords->setText(text);
    ui.edit_coords->blockSignals(false);

    ui.edit_coords->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));

    this->setEnabled(true);
    ui.edit_coords->setEnabled(true);
    ui.edit_coords->setReadOnly(false);
    ui.push_coords_apply->setEnabled(false);
    ui.push_coords_reset->setEnabled(false);
    emit validInput();
  }

  void CBCoordinateEditor::lockEditor()
  {
    ui.edit_coords->setEnabled(false);
  }

  void CBCoordinateEditor::unlockEditor()
  {
    ui.edit_coords->setEnabled(true);
  }

  void CBCoordinateEditor::markAsInvalid()
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
    ui.edit_coords->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));
  }

  void CBCoordinateEditor::markAsValid()
  {
    QTextCursor tc (ui.edit_coords->document());
    tc.movePosition(QTextCursor::Start);
    tc.movePosition(QTextCursor::End,
                    QTextCursor::KeepAnchor);
    ui.edit_coords->blockSignals(true);
    tc.setCharFormat(m_charFormat);
    ui.edit_coords->blockSignals(false);
    ui.edit_coords->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));
  }

  void CBCoordinateEditor::enableButtons()
  {
    ui.push_coords_apply->setEnabled(true);
    ui.push_coords_reset->setEnabled(true);
  }

  void CBCoordinateEditor::validateEditor()
  {
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
      row = it->split(QRegExp("\\s+|,|;"),
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

  void CBCoordinateEditor::setCoords()
  {
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
    ids.reserve(lines.size());
    coords.reserve(lines.size());

    for (QStringList::const_iterator
           it = lines.constBegin(),
           it_end = lines.constEnd();
         it != it_end;
         ++it) {
      row = it->split(QRegExp("\\s+|,|;"),
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
      CBUndoState before (m_ext);
      m_ext->setCurrentFractionalCoords(ids, coords);
      CBUndoState after (m_ext);
      m_ext->pushUndo(new CBUndoCommand (before, after,
                                         tr("Set Fractional Coordinates")));
    }
    else {
      CBUndoState before (m_ext);
      m_ext->setCurrentCartesianCoords(ids, coords);
      CBUndoState after (m_ext);
      m_ext->pushUndo(new CBUndoCommand (before, after,
                                         tr("Set Cartesian Coordinates")));
    }

    emit validInput();
  }
}
