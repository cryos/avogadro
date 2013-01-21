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

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>

class QTextDocument;

namespace Avogadro {

  class Highlighter : public QSyntaxHighlighter
  {
      Q_OBJECT

    public:
      Highlighter(QTextDocument *parent = 0);

    protected:
      void highlightBlock(const QString &text);

    private:
      struct HighlightingRule
      {
        QRegExp pattern;
        QTextCharFormat format;
      };
      
      QVector<HighlightingRule> highlightingRules;
      QTextCharFormat keywordFormat;
      QTextCharFormat singleLineCommentFormat;
  };

} // namespace

#endif
