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
    m_keywords << "\\s\\$BASIS\\b"
               << "\\s\\$CONTRL\\b"
               << "\\s\\$DATA\\b";
    rule.format = m_keywordFormat;
    foreach (const QString &pattern, m_keywords) {
      rule.pattern = QRegExp(pattern);
      m_highlightingRules.append(rule);
    }
    rule.pattern = QRegExp("\\s\\$END\\b");
    m_highlightingRules.append(rule);

    m_singleLineCommentFormat.setForeground(Qt::green);
    rule.pattern = QRegExp("![^\n]*");
    rule.format = m_singleLineCommentFormat;
    m_highlightingRules.append(rule);

    m_numberFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("(\\b|[\\s-])[0-9]+\\.([0-9]+\\b)?|\\.[0-9]+\\b");
    rule.format = m_numberFormat;
    m_highlightingRules.append(rule);

    m_numberFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("(\\b|[\\s-])[0-9]+\\.([0-9]+\\b)?|\\.[0-9]+\\b");
    rule.format = m_numberFormat;
    m_highlightingRules.append(rule);
    rule.pattern = QRegExp("(\\b|[\\s-])[0-9]+([0-9]+\\b)?|\\.[0-9]+\\b");
    rule.format = m_numberFormat;
    m_highlightingRules.append(rule);

    m_inDataBlockFormat.setForeground(Qt::gray);

    m_errorFormat.setForeground(Qt::red);
    m_errorFormat.setBackground(Qt::yellow);
  }

  void GamessHighlighter::highlightBlock(const QString &text)
  {
    // Single line comments
    QRegExp pattern("![^\n]*");
    int commentIndex = pattern.indexIn(text);
    if (commentIndex >= 0)
      setFormat(commentIndex, pattern.matchedLength(), m_singleLineCommentFormat);

    setCurrentBlockState(0);

    int startIndex = 0;
    int keywordLength = 0;
    if (previousBlockState() != 1) {
      foreach(const QString &pattern, m_keywords) {
        QRegExp expression(pattern);
        expression.setCaseSensitivity(Qt::CaseInsensitive);
        startIndex = expression.indexIn(text);
        keywordLength = expression.matchedLength();
        if (startIndex >= 0) {
          setFormat(startIndex, keywordLength, m_keywordFormat);
          break;
        }
      }
    }

    while (startIndex >= 0) {
      QRegExp endExpression("\\s\\$END\\b");
      endExpression.setCaseSensitivity(Qt::CaseInsensitive);
      int endIndex = endExpression.indexIn(text, startIndex);
      int blockLength;
      if (endIndex == -1) {
        setCurrentBlockState(1);
        blockLength = text.length() - startIndex - keywordLength;
      }
      else {
        setFormat(endIndex, endExpression.matchedLength(), m_keywordFormat);
        blockLength = endIndex - startIndex - keywordLength;
      }
      setFormat(startIndex + keywordLength, blockLength, m_inDataBlockFormat);
      bool found = false;
      foreach(const QString &pattern, m_keywords) {
        QRegExp expression(pattern);
        int index = expression.indexIn(text, startIndex + blockLength);
        if (index > startIndex) {
          found = true;
          startIndex = index;
          keywordLength = expression.matchedLength();
          setFormat(startIndex, keywordLength, m_keywordFormat);
          break;
        }
      }
      if (!found) break;
    }

    if (previousBlockState() == 1) { // Anything outside of data blocks is a comment
      foreach (const HighlightingRule &rule, m_highlightingRules) {
        QRegExp expression(rule.pattern);
        expression.setCaseSensitivity(Qt::CaseInsensitive);
        int index = text.indexOf(expression);
        while (index >= 0) {
          int length = expression.matchedLength();
          setFormat(index, length, rule.format);
          index = text.indexOf(expression, index + length);
        }
      }
    }

    // Anything over 80 columns will not be read
    if (text.length() > 80)
      setFormat(80, text.length(), m_errorFormat);
  }

} // End namespace Avogadro

