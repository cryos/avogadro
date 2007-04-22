/**********************************************************************
  ClickMeasure - ClickMeasure Tool for Avogadro

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

#ifndef __CLICKMEASURE_H
#define __CLICKMEASURE_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>
#include <avogadro/cylinder.h>

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

 class ClickMeasure : public Tool
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::Tool)

    public:
      //! Constructor
      ClickMeasure();
      //! Deconstructor
      virtual ~ClickMeasure();

      //! \name Description methods
      //@{
      //! Tool Name (ie ClickMeasure)
      virtual QString name() const { return(tr("ClickMeasure")); }
      //! Tool Description (ie. ClickMeasures atoms and bonds)
      virtual QString description() const { return(tr("Click to Measure Tool")); }
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
        */
      virtual void mousePress(GLWidget *widget, const QMouseEvent *event);
      virtual void mouseRelease(GLWidget *widget, const QMouseEvent *event);
      virtual void mouseMove(GLWidget *widget, const QMouseEvent *event);
      virtual void wheel(GLWidget *widget, const QWheelEvent *event);

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
      GLuint m_dl;

      Cylinder *m_line;
  };

} // end namespace Avogadro

#endif
