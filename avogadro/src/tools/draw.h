/**********************************************************************
  Draw - Drawing Tool for Avogadro

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

#ifndef __DRAW_H
#define __DRAW_H

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QComboBox>
#include <QVBoxLayout>
#include <QImage>
#include <QAction>

#include <openbabel/mol.h>

#include <avogadro/glwidget.h>
#include "../tool.h"

namespace Avogadro {

 class Draw : public QObject, public Tool
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::Tool)

    public:
      //! Constructor
      Draw();
      //! Deconstructor
      virtual ~Draw();

      //! \name Description methods
      //@{
      //! Tool Name (ie Draw)
      virtual QString name() const { return(tr("Draw")); }
      //! Tool Description (ie. Draws atoms and bonds)
      virtual QString description() const { return(tr("Drawing Tool")); }
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
        */
      virtual void initialize();
      virtual void cleanup();

      virtual void mousePress(Molecule *molecule, GLWidget *widget, const QMouseEvent *event);
      virtual void mouseRelease(Molecule *molecule, GLWidget *widget, const QMouseEvent *event);
      virtual void mouseMove(Molecule *molecule, GLWidget *widget, const QMouseEvent *event);

      void setElement(int i);
      int element() const;

    public slots:
      void elementChanged( int index );

    private:
      Qt::MouseButtons _buttons;

      int m_element;

      bool                _movedSinceButtonPressed;

      QPoint              _initialDraggingPosition;
      QPoint              _lastDraggingPosition;

      Atom *_beginAtom;
      Atom *_endAtom;
      Bond *_bond;
      QList<GLHit> _hits;

      QComboBox *m_comboElements;
      QVBoxLayout *m_layout;

      Atom *newAtom(Molecule *molecule, int x, int y);
      Bond *newBond(Molecule *molecule);
      void moveAtom(Atom *atom, int x, int y);
  };

} // end namespace Avogadro

#endif
