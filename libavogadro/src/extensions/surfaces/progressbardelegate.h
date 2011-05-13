/**********************************************************************
  ProgressBarDelegate - Styled Item Delegate for showing a progress bar

  Copyright (C) 2010 by David C. Lonie

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

#ifndef PROGRESSBARDELEGATE_H
#define PROGRESSBARDELEGATE_H

#include <QStyledItemDelegate>

namespace Avogadro {

// Allow progress bars to be embedded in the table
class ProgressBarDelegate : public QStyledItemDelegate
{
  Q_OBJECT
    public:
  ProgressBarDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {};
  QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const {
    return QSize(60, 30);};

  void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &ind) const {
    QStyleOptionProgressBarV2 opt;
    // Call initFrom() which will set the style based on the parent
    // GRH: This is critical to get things right on Mac
    //   otherwise the status bars always look disabled
    opt.initFrom(qobject_cast<QWidget*>(this->parent()));

    opt.rect = o.rect;
    opt.minimum = 1; // percentage
    opt.maximum = 100;
    opt.textVisible = true;
    int percent = ind.model()->data(ind, Qt::DisplayRole).toInt();
    opt.progress = percent;
    opt.text = QString("%1%").arg(QString::number(percent));
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &opt, p);
  }
};

} // end namespace Avogadro

#endif
