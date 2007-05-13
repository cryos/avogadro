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

#ifndef __FLATTABWIDGET_H
#define __FLATTABWIDGET_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QResizeEvent>
#include <QTabBar>

namespace Avogadro {

  class FlatTabBar;
  class FlatHandle;
  
  class FlatTabWidget : public QWidget
  {
    Q_OBJECT

    public:
      FlatTabWidget(QWidget *parent = 0);
      
      int addTab(QWidget *widget, QString title);
      QSize sizeHint() const;
      QSize minimumSize() const;
      QSize minimumSizeHint() const;
      QSize maximumSize() const;

    protected:
      void resizeEvent(QResizeEvent *event);

    private:
      QVBoxLayout *m_layout;
      QStackedWidget *m_stack;

      FlatHandle *m_handle;
      FlatTabBar *m_tabBar;

      int currentIndex;

    Q_SIGNALS:
      void tabToggled(int index);

    public Q_SLOTS:
      void tabPressEvent(int index);
  };

  class FlatTabBar : public QTabBar
  {
    Q_OBJECT
    public:
      FlatTabBar(QWidget *parent = 0);

    protected:
      void mousePressEvent(QMouseEvent *event);

    Q_SIGNALS:
      void tabPressed(int);
  };

  class FlatHandle : public QWidget
  {
    Q_OBJECT

    public:
      FlatHandle(QWidget *parent = 0);

      QSize sizeHint() const;

    protected:
      void paintEvent(QPaintEvent *event);
      bool event(QEvent *event);
      void mouseMoveEvent(QMouseEvent *e);
      void mousePressEvent(QMouseEvent *e);
      void mouseReleaseEvent(QMouseEvent *e);

    private:
      bool hover;

  };

      

}

#endif
