/**********************************************************************
  cbabstracteditor.h Base class for crystal builder editor dockwidgets

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

#ifndef CBABSTRACTEDITOR_H
#define CBABSTRACTEDITOR_H

#include <QtGui/QDockWidget>

class QMainWindow;

namespace Avogadro
{
  class CrystallographyExtension;

  class CBAbstractEditor : public QDockWidget
  {
    Q_OBJECT

  public:
    CBAbstractEditor(CrystallographyExtension *ext, QMainWindow *w);
    virtual ~CBAbstractEditor();

    bool isLocked() {return m_isLocked;}

    Qt::DockWidgetArea preferredDockWidgetArea();

  signals:
    void editStarted();
    void editAccepted();
    void editRejected();

    void invalidInput();
    void validInput();
    void visibilityChanged();

  public slots:
    virtual void refreshEditor() = 0;
    virtual void lockEditor() = 0;
    virtual void unlockEditor() = 0;

  protected slots:
    virtual void markAsInvalid() = 0;
    virtual void markAsValid() = 0;

    void storeDockWidgetArea(Qt::DockWidgetArea a);

  protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    CrystallographyExtension *m_ext;
    bool m_isLocked;
  };

}

#endif
