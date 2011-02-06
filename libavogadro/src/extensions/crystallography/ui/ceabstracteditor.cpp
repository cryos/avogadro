/**********************************************************************
  ceabstracteditor.h Base class for crystal builder editor dockwidgets

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

#include "ceabstracteditor.h"

#include <QtGui/QMainWindow>

#include <QtCore/QSettings>

#include "../crystallographyextension.h"

namespace Avogadro
{

  CEAbstractEditor::CEAbstractEditor(CrystallographyExtension *ext,
                                     QMainWindow *w)
    : QDockWidget(w),
      m_ext(ext),
      m_isLocked(false)
  {
    connect(this, SIGNAL(invalidInput()),
            this, SLOT(markAsInvalid()));
    connect(this, SIGNAL(validInput()),
            this, SLOT(markAsValid()));

    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(storeDockWidgetArea(Qt::DockWidgetArea)));

    connect(m_ext, SIGNAL(cellChanged()),
            this, SLOT(refreshEditor()));

    connect(this, SIGNAL(visibilityChanged()),
            m_ext, SLOT(refreshActions()));

    connect(this, SIGNAL(editStarted()),
            m_ext, SLOT(lockEditors()));
    connect(this, SIGNAL(editAccepted()),
            m_ext, SLOT(unlockEditors()));
    connect(this, SIGNAL(editRejected()),
            m_ext, SLOT(unlockEditors()));

  }

  CEAbstractEditor::~CEAbstractEditor()
  {

  }

  void CEAbstractEditor::closeEvent(QCloseEvent *event)
  {
    emit visibilityChanged();
    QDockWidget::closeEvent(event);
  }

  void CEAbstractEditor::hideEvent(QHideEvent *event)
  {
    emit visibilityChanged();
    QDockWidget::hideEvent(event);
  }

  void CEAbstractEditor::showEvent(QShowEvent *event)
  {
    emit visibilityChanged();
    QDockWidget::showEvent(event);
  }

  void CEAbstractEditor::storeDockWidgetArea(Qt::DockWidgetArea a)
  {
    QSettings settings;
    settings.setValue("crystallographyextension/editors/"
                      + QString(this->metaObject()->className())
                      + "/area", a);
  }

  Qt::DockWidgetArea CEAbstractEditor::preferredDockWidgetArea()
  {
    QSettings settings;
    return static_cast<Qt::DockWidgetArea>
      (settings.value("crystallographyextension/editors/"
                      + QString(this->metaObject()->className())
                      + "/area", Qt::RightDockWidgetArea).toInt());
  }
}

