/**********************************************************************
  ZMatrixTool - Z Matrix Tool

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef ZMATRIXTOOL_H
#define ZMATRIXTOOL_H

#include <avogadro/tool.h>

#include <QPointer>

namespace Avogadro {

  class ZMatrixDialog;

  class ZMatrixTool : public Tool
  {
    Q_OBJECT
    AVOGADRO_TOOL("Z-Matrix", tr("Z-Matrix"),
                  tr("Create/edit a z-matrix"),
                  tr("Z-Matrix Settings"))

  public:
    //! Constructor
    explicit ZMatrixTool(QObject *parent = 0);
    //! Destructor
    virtual ~ZMatrixTool();

    virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);

    virtual bool paint(GLWidget *widget);

    virtual QWidget *settingsWidget();

  private slots:
    void showZMatrixDialog();

  private:
    // Guarded pointers, for storing pointers to things that might go poof...
    QPointer<Molecule>       m_molecule;
    QPointer<GLWidget>       m_glwidget;
    QPointer<QWidget>        m_settingsWidget;
    QPointer<ZMatrixDialog>  m_zMatrixDialog;

  };

  class ZMatrixToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
        AVOGADRO_TOOL_FACTORY(ZMatrixTool);
  };

} // end namespace Avogadro

#endif // ZMATRIXTOOL_H
