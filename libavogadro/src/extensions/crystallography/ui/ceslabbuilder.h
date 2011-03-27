/**********************************************************************
  ceslabbuilder.h Widget for slab builder

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

#ifndef CESLABBUILDER_H
#define CESLABBUILDER_H

#include "ceabstractdockwidget.h"

#include "ui_ceslabbuilder.h"

class QMainWindow;

namespace Avogadro
{
  class CEUndoState;
  class CrystallographyExtension;
  class GLWidget;

  class CESlabBuilder : public CEAbstractDockWidget
  {
    Q_OBJECT

  public:
    CESlabBuilder(CrystallographyExtension *ext, QMainWindow *w,
                  GLWidget *glwidget);
    virtual ~CESlabBuilder();

  signals:

  protected slots:
    void updateSlabData();
    void buildSlab();

  protected:
    void updateSlabCell(bool build = false);
    void closeEvent(QCloseEvent *);
    void hideEvent(QHideEvent *);

  private:
    Ui::CESlabBuilder ui;

    GLWidget *m_glwidget;
    CEUndoState *m_beforeState;
    bool m_finished;
  };

}

#endif
