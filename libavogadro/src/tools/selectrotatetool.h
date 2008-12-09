/**********************************************************************
  SelectRotateTool - Selection and Rotation Tool for Avogadro

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007,2008 by Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#ifndef SELECTROTATETOOL_H
#define SELECTROTATETOOL_H

#include <avogadro/tool.h>
#include <avogadro/glwidget.h>

#include <Eigen/Core>

#include <QObject>
#include <QString>
#include <QPoint>

class QMouseEvent;
class QWheelEvent;
class QComboBox;
class QVBoxLayout;

namespace Avogadro {

  class Molecule;


  class SelectRotateTool : public Tool
  {
    Q_OBJECT

    public:
      //! Constructor
      SelectRotateTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~SelectRotateTool();

      //! \name Description methods
      //@{
      //! Tool Name (ie Draw)
      virtual QString name() const { return(tr("Selection")); }
      //! Tool Description (ie. Draws atoms and bonds)
      virtual QString description() const { return(tr("Selection Tool")); }
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
      */
      virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);

      virtual int usefulness() const;

      virtual QWidget *settingsWidget();
      virtual bool paint(GLWidget *widget);

      void setSelectionMode(int i);
      int selectionMode() const;

    public Q_SLOTS:
      void selectionModeChanged( int index );

    protected:
      void selectionBox(float sx, float sy, float ex, float ey);

      bool                m_leftButtonPressed;  // rotation
      bool                m_rightButtonPressed; // translation
      bool                m_midButtonPressed;   // scale / zoom
      bool                m_movedSinceButtonPressed;

      //! Temporary var for adding selection box
      bool                m_selectionBox;

      QPoint              m_initialDraggingPosition;
      QPoint              m_lastDraggingPosition;

      Eigen::Vector3d     m_selectedPrimitivesCenter;    // centroid of selected atoms

      int                 m_selectionMode;      // atom, residue, molecule

      QList<GLHit>        m_hits;

      QComboBox          *m_comboSelectionMode;
      QVBoxLayout        *m_layout;

      QWidget            *m_settingsWidget;

    private Q_SLOTS:
      void settingsWidgetDestroyed();
  };

  class SelectRotateToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(SelectRotateTool, tr("Select-Rotate Tool"),
        tr("Tool for selecting atoms, residues and molecules."))
  };

} // end namespace Avogadro

#endif
