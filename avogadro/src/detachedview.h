/**********************************************************************
  DetachedView - a widget to display a GLWidget detached view

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
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

#ifndef DETACHEDVIEW_H
#define DETACHEDVIEW_H

#include <QtGui/QWidget>

namespace Avogadro {

  class MainWindow;
  class GLWidget;

  /**
   * @class DetachedView
   * @brief QWidget derived class to manage a detached GLWidget view.
   * @author Marcus D. Hanwell
   *
   * This class is derived from QWidget and manages a detached GLWidget view.
   * Right now it just displays a GLWidget and listens for the MainWindow parent
   * getting closed.
   */

  class DetachedView : public QWidget
  {
    Q_OBJECT

  public:
    DetachedView(GLWidget *glWidget, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~DetachedView();

  public slots:
    void mainWindowClosed();

  private:
    GLWidget *m_glWidget;
  };

} // End namespace Avogadro

#endif // DETACHEDVIEW_H
