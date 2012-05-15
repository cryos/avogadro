/**********************************************************************
  cesupercellbuilder.h Widget for supercell / nanoparticle builder

  Copyright (C) 2012 by Geoffrey R. Hutchison

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

#ifndef CESUPERCELLBUILDER_H
#define CESUPERCELLBUILDER_H

#include "ceabstractdockwidget.h"

#include "ui_cesupercellbuilder.h"

class QMainWindow;

namespace Avogadro
{
  class CEUndoState;
  class CrystallographyExtension;
  class GLWidget;

  class CESuperCellBuilder : public CEAbstractDockWidget
  {
    Q_OBJECT

  public:
    CESuperCellBuilder(CrystallographyExtension *ext);
    virtual ~CESuperCellBuilder();

    GLWidget *glWidget() const {return m_glwidget;}

    // These match the indices in the UI file
    enum GeometryType {
      GT_BOX = 0,
      /*
      GT_CYLINDER,
      GT_ELLIPSOID,
      */
      GT_SPHERE
    };

  signals:
    void finished();

  public slots:
    void setGLWidget(GLWidget *gl) {m_glwidget = gl;}

  protected slots:
    void updateGeometryType(int type);
    // Do the work!
    void buildSuperCell();
    // Called by the extension if the user changes the length setting
    void updateLengthUnit();

  protected:
    void closeEvent(QCloseEvent *);
    void hideEvent(QHideEvent *);

    void readSettings();
    void writeSettings();

  private:
    Ui::CESuperCellBuilder ui;

    GLWidget *m_glwidget;
    CEUndoState *m_beforeState;
    bool m_finished;
  };

}

#endif
