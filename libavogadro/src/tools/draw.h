/**********************************************************************
  DrawTool - DrawTooling Tool for Avogadro

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

#ifndef __DRAWTOOL_H
#define __DRAWTOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

#include <openbabel/mol.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QComboBox>
#include <QVBoxLayout>
#include <QImage>
#include <QAction>

namespace Avogadro {

  class DrawTool : public Tool
  {
    Q_OBJECT

    public:
      //! Constructor
      DrawTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~DrawTool();

      //! \name Description methods
      //@{
      //! Tool Name (ie DrawTool)
      virtual QString name() const { return(tr("Draw")); }
      //! Tool Description (ie. DrawTools atoms and bonds)
      virtual QString description() const { return(tr("Draws Things")); }
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

      void setElement(int i);
      int element() const;

      void setBondOrder(int i);
      int bondOrder() const;

      public slots:
        void elementChanged( int index );
      void bondOrderChanged( int index );

    private:
      Qt::MouseButtons _buttons;

      int m_element;
      int m_bondOrder;
      int m_prevAtomElement;
      int m_prevBondOrder;
      Bond *m_prevBond;

      bool                m_movedSinceButtonPressed;

      QPoint              m_initialDragginggPosition;
      QPoint              m_lastDraggingPosition;

      Atom *m_beginAtom;
      Atom *m_endAtom;
      Bond *m_bond;
      QList<GLHit> m_hits;

      QComboBox *m_comboElements;
      QComboBox *m_comboBondOrder;
      QVBoxLayout *m_layout;

      Atom *newAtom(GLWidget *widget, int x, int y);
      Bond *newBond(Molecule *molecule, Atom *beginAtom, Atom *endAtom);
      //       void moveAtom(Atom *atom, const MolGeomInfo &molGeomInfo, int x, int y);

      /** @return the 3D coords of the point P obtained by unprojective the pixel (x,y) with
       * the Z-index of the center of the molecule being viewed in the given
       * GLWidget.
       */
      Eigen::Vector3d unProject(GLWidget *widget, int x, int y);
  };

  class DrawToolFactory : public QObject, public ToolFactory
  {
      Q_OBJECT;
      Q_INTERFACES(Avogadro::ToolFactory);

      public:
      Tool *createInstance(QObject *parent = 0) { return new DrawTool(parent); }
    };

} // end namespace Avogadro

#endif
