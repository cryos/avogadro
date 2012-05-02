/**********************************************************************
  ceslabbuilder.h Widget for slab builder

  Copyright (C) 2011 by David C. Lonie
  Copyright (C) 2011 by Geoffrey R. Hutchison

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
    CESlabBuilder(CrystallographyExtension *ext);
    virtual ~CESlabBuilder();

    GLWidget *glWidget() const {return m_glwidget;}

  signals:
    void finished();

  public slots:
    void setGLWidget(GLWidget *gl) {m_glwidget = gl;}

  protected slots:
    // Miller indices changed
    void updateMillerIndices();
    // Do the work!
    void buildSlab();
    // Called by the extension if the user changes the length setting
    // (unlikely)
    void updateLengthUnit();

  protected:
    void updateSlabCell(bool build = false);
    void closeEvent(QCloseEvent *);
    void hideEvent(QHideEvent *);

    void readSettings();
    void writeSettings();

  private:
    Ui::CESlabBuilder ui;

    GLWidget *m_glwidget;
    CEUndoState *m_beforeState;
    bool m_finished;
  };

}

#endif
