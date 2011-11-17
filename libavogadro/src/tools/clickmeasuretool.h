/**********************************************************************
  ClickMeasureTool - ClickMeasureTool Tool for Avogadro

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell

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

#ifndef CLICKMEASURETOOL_H
#define CLICKMEASURETOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

#include <Eigen/Core>

#include <QGLWidget>
#include <QObject>

#include <QVarLengthArray>
#include <QPointer>

class QComboBox;
class QVBoxLayout;

namespace Avogadro {

 class ClickMeasureTool : public Tool
  {
    Q_OBJECT
    AVOGADRO_TOOL("Measure", tr("Measure"),
                  tr("Measure bond lengths, angles, and dihedrals"),
                  tr("Measure Settings"))

    public:
      //! Constructor
      ClickMeasureTool(QObject *parent = 0);
      //! Destructor
      virtual ~ClickMeasureTool();

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
        */
      virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseDoubleClickEvent(GLWidget *widget, QMouseEvent *event) { return 0; }
      virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);

      virtual bool paint(GLWidget *widget);

    private:

      /*
       * Calculates the relevant distances and angles and places them in member
       * variables.
       */
      void calculateParameters();

      Qt::MouseButtons m_buttons;

      bool                m_movedSinceButtonPressed;

      QPoint              m_initialDraggingPosition;
      QPoint              m_lastDraggingPosition;

      QList<QPointer<Atom> > m_selectedAtoms;
      int m_numSelectedAtoms;
      QList<GLHit> m_hits;

      QComboBox *m_comboElements;
      QComboBox *m_comboBondOrder;
      QVBoxLayout *m_layout;

      Eigen::Vector3d m_vector[3];
      double m_angle;
      double m_dihedral;
      // Need to store the previous values of all variables in order to only send
      // an event to the information pane once
      QVector<double> m_lastMeasurement;

  };

  class ClickMeasureToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(ClickMeasureTool)
  };

} // end namespace Avogadro

#endif
