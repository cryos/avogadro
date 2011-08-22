/**********************************************************************
  SelectRotateTool - Selection and Rotation Tool for Avogadro

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007,2008 by Marcus D. Hanwell
  Copyright (C) 2010 Konstantin Tokarev
  Copyright (C) 2011 Geoffrey R. Hutchison

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

#ifndef SELECTROTATETOOL_H
#define SELECTROTATETOOL_H

#include <avogadro/tool.h>
#include <avogadro/glwidget.h>

#include <Eigen/Core>

#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtGui/QMenu>

class QMouseEvent;
class QWheelEvent;
class QComboBox;
class QVBoxLayout;

namespace Avogadro {

  class Molecule;


  class SelectRotateTool : public Tool
  {
    Q_OBJECT
    AVOGADRO_TOOL("Selection", tr("Selection"),
                  tr("Select atoms, residues, and molecules"),
                  tr("Selection Settings"))

    public:
      //! Constructor
      SelectRotateTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~SelectRotateTool();

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
      */
      virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseDoubleClickEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);

      virtual int usefulness() const;

      virtual QWidget *settingsWidget();
      virtual bool paint(GLWidget *widget);

      void setSelectionMode(int i);
      int selectionMode() const;

    public Q_SLOTS:
      void selectionModeChanged( int index );
      void defineCentroid(bool);
      void defineCenterOfMass(bool);
      void changeAtomColor();
      void resetAtomColor();
      void changeAtomLabel();
      void resetAtomLabel();
      void changeAtomRadius();
      void resetAtomRadius();

    protected:
      void selectionBox(float sx, float sy, float ex, float ey);

      bool                m_leftButtonPressed;  // rotation
      bool                m_rightButtonPressed;
      bool                m_movedSinceButtonPressed;
      bool                m_doubleClick;

      //! Temporary var for adding selection box
      bool                m_selectionBox;

      QPoint              m_initialDraggingPosition;
      QPoint              m_lastDraggingPosition;

      Eigen::Vector3d     m_selectedPrimitivesCenter;    // centroid of selected atoms
      GLWidget           *m_widget; // for defining centroids

      int                 m_selectionMode;      // atom, residue, molecule

      QList<GLHit>        m_hits;

      QComboBox          *m_comboSelectionMode;
      QVBoxLayout        *m_layout;

      QWidget            *m_settingsWidget;
      QMenu              *m_atomMenu;
      QMenu              *m_bondMenu;
      Primitive          *m_currentPrimitive;

    private Q_SLOTS:
      void settingsWidgetDestroyed();
  };

  class SelectRotateToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(SelectRotateTool)
  };

} // end namespace Avogadro

#endif
