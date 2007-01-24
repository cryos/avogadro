/**********************************************************************
  ElementComboBox - Element ComboBox Widget

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.  For more
  information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include <avogadro/elementcombobox.h>

#include <QDebug>
#include <QPainter>

using namespace Avogadro;

ElementComboBox::ElementComboBox(QWidget *parent) : QComboBox(parent)
{
}

/*!
    Displays the list of items in the combobox. If the list is empty then
    the no items will be shown.
*/
void ElementComboBox::showPopup()
{
    if (count() <= 0)
        return;

    // set current item and select it
    view()->selectionModel()->setCurrentIndex(currentIndex(),
                                              QItemSelectionModel::ClearAndSelect);
    QComboBoxPrivateContainer* container = d->viewContainer();
    // use top item as height for complete listView
    int itemHeight = view()->sizeHintForIndex(model()->index(0, d->modelColumn, rootModelIndex())).height()
                     + container->spacing();
    QStyleOptionComboBox opt = d->getStyleOption();
    QRect listRect(style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                           QStyle::SC_ComboBoxListBoxPopup, this));
    QRect screen = d->popupGeometry(QApplication::desktop()->screenNumber(this));
    QPoint below = mapToGlobal(listRect.bottomLeft());
    int belowHeight = screen.bottom() - below.y();
    QPoint above = mapToGlobal(listRect.topLeft());
    int aboveHeight = above.y() - screen.y();

    if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this))
        listRect.setHeight(itemHeight * count());
    else
        listRect.setHeight(itemHeight * qMin(d->maxVisibleItems, count()));
    listRect.setHeight(listRect.height() + 2*container->spacing()
                       + style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, this) * 2);

    // Make sure the popup is wide enough to display its contents.
    if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
        const int diff = d->computeWidthHint() - width();
        if (diff > 0)
            listRect.setWidth(listRect.width() + diff);
    }

    // make sure the widget fits on screen
    if (listRect.width() > screen.width() )
        listRect.setWidth(screen.width());
    if (mapToGlobal(listRect.bottomRight()).x() > screen.right()) {
        below.setX(screen.x() + screen.width() - listRect.width());
        above.setX(screen.x() + screen.width() - listRect.width());
    }
    if (mapToGlobal(listRect.topLeft()).x() < screen.x() ) {
        below.setX(screen.x());
        above.setX(screen.x());
    }

    if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {

        // Position horizontally.
        listRect.moveLeft(above.x());

        // Position vertically so the curently selected item lines up
        // with the combo box.
        const QRect currentItemRect = view()->visualRect(view()->currentIndex());
        const int offset = listRect.top() -  currentItemRect.top();
        listRect.moveTop(above.y() + offset);

        // Clamp the listRect height and vertical position so we don't expand outside the
        // available screen geometry.This may override the vertical position, but it is more
        // important to show as much as possible of the popup.
        const int height = qMin(listRect.height(), screen.height());
        listRect.setHeight(height);
        if (listRect.top() < screen.top())
            listRect.moveTop(screen.top());
        if (listRect.bottom() > screen.bottom())
            listRect.moveBottom(screen.bottom());
    } else if (listRect.height() <= belowHeight) {
        listRect.moveTopLeft(below);
    } else if (listRect.height() <= aboveHeight) {
        listRect.moveBottomLeft(above);
    } else if (belowHeight >= aboveHeight) {
        listRect.setHeight(belowHeight);
        listRect.moveTopLeft(below);
    } else {
        listRect.setHeight(aboveHeight);
        listRect.moveBottomLeft(above);
    }

#ifndef QT_NO_IM
    if (QInputContext *qic = inputContext())
        qic->reset();
#endif
    QScrollBar *sb = view()->horizontalScrollBar();
    Qt::ScrollBarPolicy policy = view()->horizontalScrollBarPolicy();
    bool needHorizontalScrollBar = (policy == Qt::ScrollBarAsNeeded || policy == Qt::ScrollBarAlwaysOn)
        && sb->minimum() < sb->maximum();
    if (needHorizontalScrollBar) {
        listRect.adjust(0, 0, 0, sb->height());
    }
    container->setGeometry(listRect);

    if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this))
        view()->scrollTo(view()->currentIndex(), QAbstractItemView::PositionAtCenter);
    else
        view()->scrollTo(view()->currentIndex(), QAbstractItemView::EnsureVisible);

    container->raise();
    container->show();
    container->updateScrollers();
    view()->setFocus();
}

/*!
    Hides the list of items in the combobox if it is currently visible;
    otherwise this function does nothing.
*/
void ElementComboBox::hidePopup()
{
    Q_D(QComboBox);
    if (d->container && d->container->isVisible())
        d->container->hide();
#ifdef QT_KEYPAD_NAVIGATION
    if (QApplication::keypadNavigationEnabled() && isEditable() && hasFocus())
        setEditFocus(true);
#endif
}
