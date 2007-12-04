/**********************************************************************
  FlatTabBar - Flat Tab Bar Design

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "flattabwidget.h"

#include <QAction>
#include <QPushButton>
#include <QSize>
#include <QRect>

#include <QApplication>

#include <QMenuBar>

#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QEvent>

#include <QDebug>

namespace Avogadro {

  FlatTabWidget::FlatTabWidget(QWidget *parent) : QWidget(parent),
  m_stack(new QStackedWidget(this)),
  m_handle(new FlatHandle(this)),
  m_tabBar(new FlatTabBar(this))
  {
    currentIndex = 0;

    m_tabBar->setShape(QTabBar::RoundedSouth);
    m_stack->hide();

    connect(m_tabBar, SIGNAL(tabPressed(int)),
        this, SLOT(tabPressEvent(int)));
    connect(m_tabBar, SIGNAL(currentChanged(int)),
        m_stack, SLOT(setCurrentIndex(int)));
  }

  int FlatTabWidget::addTab(QWidget *widget, const QString& title)
  {
    m_tabBar->addTab(title);
    return m_stack->addWidget(widget);
  }

  QSize FlatTabWidget::sizeHint () const
  {
    QSize sizeHint;
    int &heightHint = sizeHint.rheight();
    int &widthHint = sizeHint.rwidth();

    QSize hSizeHint = m_handle->sizeHint();
    QSize tbSizeHint = m_tabBar->sizeHint();
    QSize stackSizeHint = m_stack->sizeHint();


    heightHint = tbSizeHint.height() + hSizeHint.height();
    widthHint = tbSizeHint.width() + hSizeHint.width();

    if(!m_stack->isHidden())
    {
      heightHint += stackSizeHint.height();
      widthHint  += stackSizeHint.width();
    }

    return sizeHint;
  }

  QSize FlatTabWidget::minimumSize() const
  {
    QSize minimumSize;

    QSize h = m_handle->sizeHint();
    QSize s = m_stack->minimumSize();
    QSize t = m_tabBar->minimumSize();

    minimumSize.setWidth(qMax(t.width(), s.width()));

    if(!m_stack->isHidden())
    {
      minimumSize.setHeight(h.height() + t.height() + s.height());
    } else {
      minimumSize.setHeight(h.height() + t.height());
    }

    return minimumSize;
  }

  QSize FlatTabWidget::minimumSizeHint() const
  {
    QSize minimumSizeHint;

    QSize s = m_stack->minimumSizeHint();
    QSize t = m_tabBar->minimumSizeHint();

    minimumSizeHint.setWidth(qMax(t.width(), s.width()));

    if(!m_stack->isHidden())
    {
      minimumSizeHint.setHeight(t.height() + s.height());
    } else {
      minimumSizeHint.setHeight(t.height());
    }

    return minimumSizeHint;

  }

  QSize FlatTabWidget::maximumSize() const
  {
    QSize maximumSize;

    QSize s = m_stack->maximumSize();
    QSize t = m_tabBar->sizeHint();

    maximumSize.setWidth(qMax(t.width(), s.width()));

    if(!m_stack->isHidden())
    {
      maximumSize.setHeight(t.height() + s.height());
    } else {
      maximumSize.setHeight(t.height());
    }

    return maximumSize;
  }

  void FlatTabWidget::resizeEvent(QResizeEvent *event)
  {
    QSize s = event->size();


    int handleHeight = m_handle->sizeHint().height();
    int tabHeight = m_tabBar->sizeHint().height();
    int stackHeight = 0;
    if(!m_stack->isHidden())
    {
      stackHeight = s.height() - tabHeight - handleHeight;
    }
    int width = s.width();


    QRect rh(0,0, width, handleHeight);
    m_handle->setGeometry(rh);

    QRect rs(0,handleHeight, s.width(), stackHeight);
    m_stack->setGeometry(rs);

    QRect rt(0,handleHeight + stackHeight, width, tabHeight);
    m_tabBar->setGeometry(rt);

  }

  void FlatTabWidget::tabPressEvent(int index)
  {
    if(index == currentIndex)
    {
      m_stack->setHidden(!m_stack->isHidden());
      emit tabToggled(index);
      updateGeometry();
    } else {
      if(m_stack->isHidden())
      {
        m_stack->show();
        emit tabToggled(index);
        updateGeometry();
      }
      currentIndex = index;
    }
  }

  FlatTabBar::FlatTabBar(QWidget *parent) : QTabBar(parent)
  {
  }

  void FlatTabBar::mousePressEvent(QMouseEvent *event)
  {
    QTabBar::mousePressEvent(event);

    emit tabPressed(currentIndex());

  }

  FlatHandle::FlatHandle(QWidget *parent) : QWidget(parent), hover(false)
  {
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::SplitVCursor);
  }

  QSize FlatHandle::sizeHint() const
  {
    QStyleOption opt(0);

    int hw = parentWidget()->style()->pixelMetric(QStyle::PM_SplitterWidth, 0, parentWidget());
    opt.init(parentWidget());
    opt.state = QStyle::State_None;
    return parentWidget()->style()->sizeFromContents(QStyle::CT_Splitter, &opt, QSize(hw, hw), parentWidget());
    //     .expandedTo(QApplication::globalStrut());
  }

  bool FlatHandle::event(QEvent *event)
  {
    switch(event->type()) {
      case QEvent::HoverEnter:
        hover = true;
        update();
        break;
      case QEvent::HoverLeave:
        hover = false;
        update();
        break;
      default:
        break;
    }
    return QWidget::event(event);
  }

  void FlatHandle::mouseMoveEvent(QMouseEvent *)
  {
    //     if (!(e->buttons() & Qt::LeftButton))
    //         return;
    //     int pos = d->pick(parentWidget()->mapFromGlobal(e->globalPos()))
    //                  - d->mouseOffset;
    //     moveSplitter(pos);
  }

  void FlatHandle::mousePressEvent(QMouseEvent *)
  {
    //     Q_D(QSplitterHandle);
    //     if (e->button() == Qt::LeftButton)
    //         d->mouseOffset = d->pick(e->pos());
  }

  /*!
    \reimp
    */
  void FlatHandle::mouseReleaseEvent(QMouseEvent *)
  {
    //     Q_D(QSplitterHandle);
    //     if (!opaqueResize() && e->button() == Qt::LeftButton) {
    //         int pos = d->pick(parentWidget()->mapFromGlobal(e->globalPos()))
    //                      - d->mouseOffset;
    //         d->s->setRubberBand(-1);
    //         moveSplitter(pos);
    //     }
  }

  /*!
    \reimp
    */
  void FlatHandle::paintEvent(QPaintEvent *)
  {
    QPainter p(this);
    QStyleOption opt(0);
    opt.rect = rect();
    opt.palette = palette();
    opt.state = 0;
    //     opt.state = QStyle::State_Horizontal;
    if (hover)
      opt.state |= QStyle::State_MouseOver;
    if (isEnabled())
      opt.state |= QStyle::State_Enabled;
    parentWidget()->style()->drawControl(QStyle::CE_Splitter, &opt, &p, parentWidget());
  }

  // void FlatTabWidget::paintEvent(QPaintEvent *)
  // {
  //     QStylePainter p(this);
  //     QStyleOptionTabWidgetFrame opt;
  // //     opt = getStyleOption();
  // //     opt.rect = d->panelRect;
  // //     p.drawPrimitive(QStyle::PE_FrameTabWidget, opt);
  // }
  //
  // QStyleOptionTabWidgetFrame FlatTabWidget::getStyleOption() const
  // {
  //     QStyleOptionTabWidgetFrame option;
  //     option.init(this);
  //
  //     int exth = style()->pixelMetric(QStyle::PM_TabBarBaseHeight, 0, this);
  //     QSize t(0, stack->frameWidth());
  //     if (tabs->isVisibleTo(const_cast<QTabWidget *>(this)))
  //         t = tabs->sizeHint();
  //
  //     if (rightCornerWidget)
  //         option.rightCornerWidgetSize
  //             = rightCornerWidget->sizeHint().boundedTo(t - QSize(exth, exth));
  //     else
  //         option.rightCornerWidgetSize = QSize(0, 0);
  //
  //     if (leftCornerWidget)
  //         option.leftCornerWidgetSize
  //             = leftCornerWidget->sizeHint().boundedTo(t - QSize(exth, exth));
  //     else
  //         option.leftCornerWidgetSize = QSize(0, 0);
  //
  //     switch (pos) {
  //     case QTabWidget::North:
  //         option.shape = shape == QTabWidget::Rounded ? QTabBar::RoundedNorth : QTabBar::TriangularNorth;
  //         break;
  //     case QTabWidget::South:
  //         option.shape = shape == QTabWidget::Rounded ? QTabBar::RoundedSouth : QTabBar::TriangularSouth;
  //         break;
  //     case QTabWidget::West:
  //         option.shape = shape == QTabWidget::Rounded ? QTabBar::RoundedWest : QTabBar::TriangularWest;
  //         break;
  //     case QTabWidget::East:
  //         option.shape = shape == QTabWidget::Rounded ? QTabBar::RoundedEast : QTabBar::TriangularEast;
  //         break;
  //     }
  //     option.tabBarSize = t;
  //
  //     return option;
  // }
  // QStyleOptionTabV2 FlatTabBar::getStyleOption(int tab) const
  // {
  //     QStyleOptionTabV2 opt;
  //     opt.init(this);
  //     opt.state &= ~(QStyle::State_HasFocus | QStyle::State_MouseOver);
  //     opt.rect = tabRect(tab);
  //     bool isCurrent = tab == currentIndex();
  //     opt.row = 0;
  // //     if (tab == pressedIndex)
  // //         opt.state |= QStyle::State_Sunken;
  //     if (isCurrent)
  //         opt.state |= QStyle::State_Selected;
  //     if (isCurrent && hasFocus())
  //         opt.state |= QStyle::State_HasFocus;
  //     if (!isTabEnabled(tab))
  //         opt.state &= ~QStyle::State_Enabled;
  //     if (isActiveWindow())
  //         opt.state |= QStyle::State_Active;
  // //     if (opt.rect == hoverRect)
  // //         opt.state |= QStyle::State_MouseOver;
  //     opt.shape = shape();
  //     opt.text = tabText(tab);
  //
  //     if (tabTextColor(tab).isValid())
  //         opt.palette.setColor(foregroundRole(), tabTextColor(tab));
  //
  // //     opt.icon = tabIcon(tab);
  // //     opt.iconSize = iconSize();  // Will get the default value then.
  //
  //     int totalTabs = count();
  //
  //     if (tab > 0 && tab - 1 == currentIndex())
  //         opt.selectedPosition = QStyleOptionTab::PreviousIsSelected;
  //     else if (tab < totalTabs - 1 && tab + 1 == currentIndex())
  //         opt.selectedPosition = QStyleOptionTab::NextIsSelected;
  //     else
  //         opt.selectedPosition = QStyleOptionTab::NotAdjacent;
  //
  //     if (tab == 0) {
  //         if (totalTabs > 1)
  //             opt.position = QStyleOptionTab::Beginning;
  //         else
  //             opt.position = QStyleOptionTab::OnlyOneTab;
  //     } else if (tab == totalTabs - 1) {
  //         opt.position = QStyleOptionTab::End;
  //     } else {
  //         opt.position = QStyleOptionTab::Middle;
  //     }
  //     if (const QTabWidget *tw = qobject_cast<const QTabWidget *>(parentWidget())) {
  //         if (tw->cornerWidget(Qt::TopLeftCorner) || tw->cornerWidget(Qt::BottomLeftCorner))
  //             opt.cornerWidgets |= QStyleOptionTab::LeftCornerWidget;
  //         if (tw->cornerWidget(Qt::TopRightCorner) || tw->cornerWidget(Qt::BottomRightCorner))
  //             opt.cornerWidgets |= QStyleOptionTab::RightCornerWidget;
  //     }
  //     int hframe  = style()->pixelMetric(QStyle::PM_TabBarTabHSpace, &opt, this);
  //
  //     opt.text = fontMetrics().elidedText(opt.text, elideMode(), 1 + (verticalTabs(shape()) ? tabRect(tab).height() : tabRect(tab).width()) - hframe,
  //                                            Qt::TextShowMnemonic);
  //     return opt;
  // }
  //
  // void FlatTabBar::paintEvent(QPaintEvent *event)
  // {
  //     QStyleOptionTab tabOverlap;
  //     tabOverlap.shape = shape();
  //     int overlap = style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabOverlap, this);
  //     QWidget *theParent = parentWidget();
  //     QStyleOptionTabBarBase optTabBase;
  //     optTabBase.init(this);
  //     optTabBase.shape = shape();
  //     if (theParent && overlap > 0) {
  //         QRect rect;
  //         switch (tabOverlap.shape) {
  //         case QTabBar::RoundedNorth:
  //         case QTabBar::TriangularNorth:
  //             rect.setRect(0, height()-overlap, width(), overlap);
  //             break;
  //         case QTabBar::RoundedSouth:
  //         case QTabBar::TriangularSouth:
  //             rect.setRect(0, 0, width(), overlap);
  //             break;
  //         case QTabBar::RoundedEast:
  //         case QTabBar::TriangularEast:
  //             rect.setRect(0, 0, overlap, height());
  //             break;
  //         case QTabBar::RoundedWest:
  //         case QTabBar::TriangularWest:
  //             rect.setRect(width()-overlap, 0, overlap, height());
  //             break;
  //         }
  //         optTabBase.rect = rect;
  //     }
  //     QStylePainter p(this);
  //     QPainter pt(this);
  //     int selected = -1;
  //     int cut = -1;
  //     bool rtl = optTabBase.direction == Qt::RightToLeft;
  //     bool verticalTabs = (shape() == QTabBar::RoundedWest || shape() == QTabBar::RoundedEast
  //                          || shape() == QTabBar::TriangularWest
  //                          || shape() == QTabBar::TriangularEast);
  //     QStyleOptionTab cutTab;
  //     QStyleOptionTab selectedTab;
  //     for (int i = 0; i < count(); ++i) {
  //         QStyleOptionTabV2 tab = getStyleOption(i);
  //         if (!(tab.state & QStyle::State_Enabled)) {
  //             tab.palette.setCurrentColorGroup(QPalette::Disabled);
  //         }
  //         // If this tab is partially obscured, make a note of it so that we can pass the information
  //         // along when we draw the tear.
  //         if ((!verticalTabs && (!rtl && tab.rect.left() < 0) || (rtl && tab.rect.right() > width()))
  //             || (verticalTabs && tab.rect.top() < 0)) {
  //             cut = i;
  //             cutTab = tab;
  //         }
  //         // Don't bother drawing a tab if the entire tab is outside of the visible tab bar.
  //         if ((!verticalTabs && (tab.rect.right() < 0 || tab.rect.left() > width()))
  //             || (verticalTabs && (tab.rect.bottom() < 0 || tab.rect.top() > height())))
  //             continue;
  //
  //         optTabBase.tabBarRect |= tab.rect;
  //         if (i == currentIndex()) {
  //             selected = i;
  //             selectedTab = tab;
  //             optTabBase.selectedTabRect = tab.rect;
  //             continue;
  //         }
  // //         p.drawControl(QStyle::CE_TabBarTab, tab);
  //         pt.drawRect(tab.rect);
  //         pt.drawText(tab.rect, Qt::AlignCenter | Qt::AlignVCenter, tab.text);
  // //         p.drawControl(QStyle::CE_ToolBoxTab, tab);
  // //         p.drawControl(QStyle::CE_TabBarTabLabel, tab);
  // //         p.drawItemText(tab.rect, 0, tab.palette, true, tr("Test"));
  //     }
  //
  //     // Draw the selected tab last to get it "on top"
  //     if (selected >= 0) {
  //         QStyleOptionTabV2 tab = getStyleOption(selected);
  // //         p.drawControl(QStyle::CE_TabBarTab, tab);
  //         pt.drawRect(tab.rect);
  //         pt.drawText(tab.rect, Qt::AlignCenter | Qt::AlignVCenter, tab.text);
  //
  // //         p.drawControl(QStyle::CE_ToolBoxTab, tab);
  // //         p.drawControl(QStyle::CE_TabBarTabLabel, tab);
  // //         p.drawItemText(tab.rect, 0, tab.palette, true, tr("Test"));
  //     }
  // //     if (drawBase())
  // //         p.drawPrimitive(QStyle::PE_FrameTabBarBase, optTabBase);
  //
  //     // Only draw the tear indicator if necessary. Most of the time we don't need too.
  // //     if (d->leftB->isVisible() && cut >= 0) {
  // //         cutTab.rect = rect();
  // //         cutTab.rect = style()->subElementRect(QStyle::SE_TabBarTearIndicator, &cutTab, this);
  // //         p.drawPrimitive(QStyle::PE_IndicatorTabTear, cutTab);
  // //     }
  // }
  //

}

#include "flattabwidget.moc"
