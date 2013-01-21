/**********************************************************************
  Highlighter - Code highlighting for the packmol extension

  Copyright (C) 2010 by Tim Vandermeersch

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

#include "highlighter.h"

#include <QRegExp>

namespace Avogadro {

  Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
  {
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkGreen);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;

    // Python keywords
    keywordPatterns << "\\bstructure\\b" << "\\bend\\b" << "\\btolerance\\b"
                    << "\\boutput\\b" << "\\bfiletype\\b" << "\\bnumber\\b"
                    << "\\binside\\b" << "\\bcube\\b" << "\\bbox\\b"
                    << "\\bsphere\\b" << "\\boutside\\b" << "\\batoms\\b"
                    << "\\bcenter\\b" << "\\bfixed\\b" << "\\bellipsoid\\b"
                    << "\\bplane\\b" << "\\bover\\b" << "\\bbelow\\b"
                    << "\\bcylinder\\b" << "\\badd_amber_ter\\b"
                    << "\\badd_box_sides\\b" << "\\brandominitialpoint\\b"
                    << "\\bseed\\b" << "\\bmaxit\\b" << "\\bnloop\\b"
                    << "\\bwriteout\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    singleLineCommentFormat.setForeground(Qt::blue);
    singleLineCommentFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
  }

  void Highlighter::highlightBlock(const QString &text)
  {
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = text.indexOf(expression, index + length);
        }
    }
  //  setCurrentBlockState(0);
  }

} // namespace

