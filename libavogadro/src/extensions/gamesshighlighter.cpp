/**********************************************************************
  GamessHighlighter - syntax highlighting for Gamess input files

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "gamesshighlighter.h"

namespace Avogadro {

  GamessHighlighter::GamessHighlighter(QTextDocument *parent)
      : QSyntaxHighlighter(parent)
  {
    HighlightingRule rule;

    m_keywordFormat.setForeground(Qt::darkBlue);
    m_keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\s\\$BASIS\\b"
                    << "\\s\\$CONTRL\\b"
                    << "\\s\\$DATA\\b"
                    << "\\s\\$END\\b";
    rule.format = m_keywordFormat;
    foreach (const QString &pattern, keywordPatterns) {
      rule.pattern = QRegExp(pattern);
      m_highlightingRules.append(rule);
    }
    m_singleLineCommentFormat.setForeground(Qt::green);
    rule.pattern = QRegExp("![^\n]*");
    rule.format = m_singleLineCommentFormat;
    m_highlightingRules.append(rule);

    m_numberFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("(\\b[0-9]+\\.([0-9]+\\b)?|\\.[0-9]+\\b)");
    rule.format = m_numberFormat;
    m_highlightingRules.append(rule);

  }

  void GamessHighlighter::highlightBlock(const QString &text)
  {
    foreach (HighlightingRule rule, m_highlightingRules) {
      QRegExp expression(rule.pattern);
      int index = text.indexOf(expression);
      while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, rule.format);
        index = text.indexOf(expression, index + length);
      }
    }
    setCurrentBlockState(0);
  }

} // End namespace Avogadro

#include "gamesshighlighter.moc"
