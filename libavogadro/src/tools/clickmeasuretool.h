/**********************************************************************
  ClickMeasureTool - ClickMeasureTool Tool for Avogadro

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef __CLICKMEASURETOOL_H
#define __CLICKMEASURETOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

#include <openbabel/mol.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QComboBox>
#include <QVBoxLayout>
#include <QVarLengthArray>
#include <QImage>
#include <QAction>

namespace Avogadro {

 class ClickMeasureTool : public Tool
  {
    Q_OBJECT

    public:
      //! Constructor
      ClickMeasureTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~ClickMeasureTool();

      //! \name Description methods
      //@{
      //! Tool Name (ie ClickMeasureTool)
      virtual QString name() const { return(tr("ClickMeasure")); }
      //! Tool Description (ie. ClickMeasureTools atoms and bonds)
      virtual QString description() const { return(tr("Click to Measure Tool")); }
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
        */
      virtual QUndoCommand* mousePress(GLWidget *widget, const QMouseEvent *event);
      virtual QUndoCommand* mouseRelease(GLWidget *widget, const QMouseEvent *event);
      virtual QUndoCommand* mouseMove(GLWidget *widget, const QMouseEvent *event);
      virtual QUndoCommand* wheel(GLWidget *widget, const QWheelEvent *event);

      virtual bool paint(GLWidget *widget);

    private:

      Qt::MouseButtons _buttons;

      bool                _movedSinceButtonPressed;

      QPoint              _initialDraggingPosition;
      QPoint              _lastDraggingPosition;

      QVarLengthArray<Atom *, 4> m_selectedAtoms;
      int m_numSelectedAtoms;
      QList<GLHit> m_hits;

      QComboBox *m_comboElements;
      QComboBox *m_comboBondOrder;
      QVBoxLayout *m_layout;
  };

  class ClickMeasureToolFactory : public QObject, public ToolFactory
    {
      Q_OBJECT;
      Q_INTERFACES(Avogadro::ToolFactory);

      public:
        Tool *createInstance(QObject *parent = 0) { return new ClickMeasureTool(parent); }
    };

} // end namespace Avogadro

#endif
