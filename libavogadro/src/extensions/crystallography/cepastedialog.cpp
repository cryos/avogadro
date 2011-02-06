/**********************************************************************
  CEPasteDialog - Dialog Crystal import

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

/*
 *  @todo add GULP format
 *  @todo add PWscf format
 *  @todo add CASTEP format
 *  @todo add FINDSYM format
 */

#include "cepastedialog.h"

#include "obeigenconv.h"

#include <avogadro/atom.h>

#include <QtGui/QMessageBox>

namespace Avogadro {

  CEPasteDialog::CEPasteDialog(QWidget *parent,
                               const QString &text,
                               Molecule *mol)
    : QDialog(parent),
      m_molecule(mol),
      m_text(text),
      m_format(InvalidFormat),
      m_numAtomTypes(0)
  {
    ui.setupUi(this);
    refresh();
  }

  void CEPasteDialog::refresh()
  {
    switch (guessFormat(m_text)) {
    case InvalidFormat:
    default:
      refreshInvalidFormat();
      break;
    case VaspFormat:
      refreshVaspFormat();
      break;
    }
  }

  void CEPasteDialog::accept()
  {
    switch (guessFormat(m_text)) {
    case InvalidFormat:
    default:
      break;
    case VaspFormat:
      if (!acceptVaspFormat()) {
        return;
      }
      break;
    }

    QDialog::accept();
  }

  CEPasteDialog::CEPFormat
  CEPasteDialog::guessFormat(const QString &text)
  {
    if (m_format) {
      return m_format;
    }

    if (isVaspFormat(text)) {
      m_format = VaspFormat;
      return m_format;
    }
    else {
      m_format = InvalidFormat;
      return m_format;
    }
  }

  bool CEPasteDialog::isVaspFormat(const QString &text)
  {
    QStringList lines = text.split("\n");

    if (lines.size() < 7) {
      return false;
    }
    if (lines.at(7).at(0).toLower() == QChar('s') &&
        lines.size() < 8) {
      return false;
    }

    unsigned int lineIndex = 0;
    const QString *line;
    QStringList lineList;
    bool ok;
    unsigned int numAtoms = 0;

    // First line is comment
    line = &lines[lineIndex++];

    // Next is a scale factor, single float
    line = &lines[lineIndex++];
    line->toDouble(&ok);
    if (!ok) {
      return false;
    }

    // Next three lines are cell vectors, three floats
    for (int vecInd = 0; vecInd < 3; ++vecInd) {
      line = &lines[lineIndex++];
      lineList = line->simplified().split(QRegExp("\\s+|,|;"));
      if (lineList.size() != 3) {
        return false;
      }
      for (int i = 0; i < 3; ++i) {
        lineList.at(i).toDouble(&ok);
        if (!ok) {
          return false;
        }
      }
    }

    // Next line is a list of unsigned integers (composition)
    line = &lines[lineIndex++];
    lineList = line->simplified().split(QRegExp("\\s+|,|;"));
    for (QStringList::const_iterator
           it = lineList.constBegin(),
           it_end = lineList.constEnd();
         it != it_end; ++it) {
      numAtoms += it->toUInt(&ok);
      if (!ok) {
        return false;
      }
    }

    // Next line is a string. First character is important:
    line = &lines[lineIndex++];
    if (line->size() && line->at(0).toLower() == QChar('s')){
      // If first letter is s (selective dynamics), skip one more line
      line = &lines[lineIndex++];
    }
    // Current line is a string, contents not important here

    // Next [numAtoms] lines are vectors. So long as they're ok, we
    // have a POSCAR or CONTCAR!
    for (unsigned int i = 0; i < numAtoms; ++i) {
      line = &lines[lineIndex++];
      lineList = line->simplified().split(QRegExp("\\s+|,|;"));
      if (lineList.size() != 3) {
        return false;
      }
      for (int i = 0; i < 3; ++i) {
        lineList.at(i).toDouble(&ok);
        if (!ok) {
          return false;
        }
      }
    }
    return true;
  }

  void CEPasteDialog::refreshInvalidFormat()
  {
    QMessageBox::critical
      (this,
       tr("Cannot Parse Text"),
       tr("The input is not formatted as one of the "
          "following supported formats:\n") +
       tr("VASP Format\n"));
    reject();
    close();
  }

  void CEPasteDialog::refreshVaspFormat()
  {
    QStringList lines = m_text.split("\n");

    Q_ASSERT(lines.size() >= 7);
    Q_ASSERT_X(lines.at(7).at(0).toLower() != QChar('s') ||
               lines.size() >= 8, Q_FUNC_INFO,
               "'lines' is too short to be a proper POSCAR.");

    // Determine number of atom types
    const QString *typesStr = &lines[5];
    m_numAtomTypes = typesStr->simplified().split(QRegExp("\\s+|,|;")).size();

    // Try to determine the atom types from the comment line
    const QString *comment = &lines[0];

    /// @todo Support more style formats
    // Assume alternating numbers / letters for composition, e.g. Sr1Ti3O4, etc.
    // Parse sets of numbers between letters
    QStringList symbolList = comment->split(QRegExp("[0-9|\\s|,|;]+"),
                                            QString::SkipEmptyParts);

    // Look for the first list of m_numAtomTypes consecutive strings
    // that converts cleanly into atomic numbers:
    QList<unsigned int> atomicNums;
    for (int i = 0; i < symbolList.size(); ++i) {
      atomicNums.append(OpenBabel::etab.GetAtomicNum
                        (symbolList.at(i).toStdString().c_str()));
    }
    int startInd = -1;
    for (int i = 0; i < atomicNums.size(); ++i) {
      bool found = true;
      for (unsigned int j = 0; j < m_numAtomTypes; ++j) {
        if (atomicNums.at(i + j) == 0) {
          found = false;
          break;
        }
      }
      if (found) {
        startInd = i;
        break;
      }
    }

    QString idents = "";
    if (startInd >= 0) {
      // Found list of consecutive identifiers.
      Q_ASSERT(startInd + m_numAtomTypes <=
               static_cast<unsigned int>((symbolList.size())));
      for (unsigned int i = 0; i < m_numAtomTypes; ++i) {
        idents += symbolList.at(startInd + i) + " ";
      }
    }
    else {
      // Did not find. Fill with consecutive integers.
      for (unsigned int i = 1; i <= m_numAtomTypes; ++i) {
        idents += QString::number(i) + " ";
      }
    }

    ui.edit_identities->setText(idents);
    ui.edit_text->setText(m_text);
  }

  bool CEPasteDialog::acceptVaspFormat()
  {
    // Validate identities field
    QStringList idents = ui.edit_identities->text().simplified()
      .split(QRegExp("\\s+|,|;"));

    if (static_cast<unsigned int>(idents.size()) != m_numAtomTypes) {
      QMessageBox::critical
        (this,
         tr("Bad Compostion"),
         tr("The identities field must contain the same number of "
            "space delimited entries as line 6 of the POSCAR."));
      return false;
    }

    bool ok;
    QList<unsigned int> compAtomicNums;
    QList<unsigned int> compCounts;

    for (QStringList::const_iterator
           it = idents.constBegin(),
           it_end = idents.constEnd();
         it != it_end; ++it) {
      // Attempt to use the string as a uint
      unsigned int atomicNum = it->toUInt(&ok);
      // If this doesn't work, try passing the string to OB's translator
      if (!ok) {
        atomicNum = OpenBabel::etab.GetAtomicNum(it->toStdString().c_str());
      }
      compAtomicNums.append(atomicNum);
    }

    QStringList lines = m_text.split("\n");

    unsigned int lineIndex = 0;
    const QString *line;
    QStringList lineList;
    unsigned int numAtoms = 0;
    QList<Eigen::Vector3d> positions;
    QList<unsigned int> atomicNums;
    Eigen::Matrix3d cellMatrix;

    // First line is comment
    line = &lines[lineIndex++];

    // Next is a scale factor, single float
    line = &lines[lineIndex++];
    double scale = line->toDouble(&ok);
    if (!ok) {
      return false;
    }

    // Next three lines are cell vectors, three floats
    for (int vecInd = 0; vecInd < 3; ++vecInd) {
      line = &lines[lineIndex++];
      lineList = line->simplified().split(QRegExp("\\s+|,|;"));
      if (lineList.size() != 3) {
        return false;
      }
      for (int i = 0; i < 3; ++i) {
        double v = lineList.at(i).toDouble(&ok);
        if (!ok) {
          return false;
        }
        cellMatrix(vecInd, i) = v * scale;
      }
    }

    // Next line is a list of unsigned integers (composition)
    line = &lines[lineIndex++];
    lineList = line->simplified().split(QRegExp("\\s+|,|;"));
    for (QStringList::const_iterator
           it = lineList.constBegin(),
           it_end = lineList.constEnd();
         it != it_end; ++it) {
      unsigned int v = it->toUInt(&ok);
      if (!ok) {
        return false;
      }
      numAtoms += v;
      compCounts.append(v);
    }

    // Next line is a string. First character is important:
    line = &lines[lineIndex++];
    if (line->size() && line->at(0).toLower() == QChar('s')){
      // If first letter is s (selective dynamics), skip one more line
      line = &lines[lineIndex++];
    }
    // Current line is a string. If it starts with K, k, C, or c,
    // positions are in cartesian units. Otherwise, reciprocal units.
    bool isCartesian = false;
    if (line->size() &&
        (line->at(0).toLower() == QChar('k') ||
         line->at(0).toLower() == QChar('c') ) ) {
      isCartesian = true;
    }

    // Next [numAtoms] lines are vectors. So long as they're ok, we
    // have a POSCAR or CONTCAR!
    for (unsigned int i = 0; i < numAtoms; ++i) {
      line = &lines[lineIndex++];
      lineList = line->simplified().split(QRegExp("\\s+|,|;"));
      if (lineList.size() != 3) {
        return false;
      }
      Eigen::Vector3d vec;
      for (int i = 0; i < 3; ++i) {
        double v = lineList.at(i).toDouble(&ok);
        if (!ok) {
          return false;
        }
        vec(i) = v;
      }
      positions.append(vec);
    }

    // Build list of atomic numbers
    Q_ASSERT(compAtomicNums.size() == compCounts.size());
    for (int i = 0; i < compCounts.size(); ++i) {
      for (unsigned int j = 0; j < compCounts.at(i); ++j) {
        atomicNums.append(compAtomicNums.at(i));
      }
    }
    Q_ASSERT(atomicNums.size() == positions.size());

    // Set unit cell
    QWriteLocker locker (m_molecule->lock());
    OpenBabel::OBUnitCell *cell =  m_molecule->OBUnitCell();
    cell->SetData(Eigen2OB(cellMatrix));

    // Convert to cartesian coords if necessary:
    if (!isCartesian) {
      for (QList<Eigen::Vector3d>::iterator
             it = positions.begin(),
             it_end = positions.end();
           it != it_end; ++it) {
        *it = OB2Eigen(cell->FractionalToCartesian
                       (Eigen2OB(*it)));
      }
    }

    // Remove old atoms
    QList<Avogadro::Atom*> oldAtoms = m_molecule->atoms();
    for (QList<Avogadro::Atom*>::iterator
           it = oldAtoms.begin(),
           it_end = oldAtoms.end();
         it != it_end;
         ++it) {
      m_molecule->removeAtom(*it);
    }

    // Add new atoms
    for (int i = 0; i < positions.size(); ++i) {
      Atom *atom = m_molecule->addAtom();
      atom->setAtomicNumber(atomicNums.at(i));
      atom->setPos(positions.at(i));
    }

    return true;
  }

}

