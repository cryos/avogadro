/**********************************************************************
  DrawTool - Tool for drawing molecules

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007-2009 Marcus D. Hanwell
  Copyright (C) 2008 Tim Vandermeersch

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

#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include <avogadro/tool.h>

class QCheckBox;
class QVBoxLayout;
class QPushButton;
class QComboBox;

namespace OpenBabel {
  class OBForceField;
}

namespace Avogadro {

  class Atom;
  class Bond;
  class GLHit;

  class PeriodicTableView;
  class AddAtomCommand;

  class DrawTool : public Tool
  {
    Q_OBJECT
    AVOGADRO_TOOL("Draw", tr("Draw"),
                  tr("Draw and edit atoms and bonds"),
                  tr("Draw Settings"))

    public:
      //! Constructor
      DrawTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~DrawTool();

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.

      /*! Handle a mouse press (i.e., beginning of drawing)
      */
      virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
      /*! Handle a mouse release (i.e., the end of drawing)
      */
      virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
      /*! Handle a mouse move (perhaps drawing a bond)
      */
      virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
      /*! Handle a scroll wheel (i.e., zooming in and out of the canvas)
      */
      virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);

      virtual QUndoCommand* keyPressEvent(GLWidget *widget, QKeyEvent *event);
      //@}

      /**
       * Write the tool settings so that they can be saved between sessions.
       */
      virtual void writeSettings(QSettings &settings) const;

      /**
       * Read in the settings that have been saved for the tool instance.
       */
      virtual void readSettings(QSettings &settings);

      //! The priority of the tool in the toolbar
      virtual int usefulness() const;

      virtual QWidget *settingsWidget();

      void setElement(int i);
      int element() const;

      int bondOrder() const;
      int addHydrogens() const;

    public Q_SLOTS:
      void setAddHydrogens( int state );

      void elementChanged( int index );
      void customElementChanged( int index );
      void bondOrderChanged( int index );
      void setBondOrder(int i);

      void clearKeyPressBuffer();

    private:
      Qt::MouseButtons    m_buttons;

      bool                m_movedSinceButtonPressed;

      QPoint              m_initialDraggingPosition;
      QPoint              m_lastDraggingPosition;

      QString   m_keyPressBuffer;

      bool m_beginAtomAdded;
      bool m_endAtomAdded;
      Atom *m_beginAtom;
      Atom *m_endAtom;
      int m_element;

      Bond *m_bond;
      int m_bondOrder;

      int m_prevAtomElement;

      Bond *m_prevBond;
      int m_prevBondOrder;

      bool m_addHydrogens;
      QUndoCommand *m_hydrogenCommand;

      QList<GLHit> m_hits;

      QComboBox *m_comboElements;
      QList<int> m_elementsIndex;
      QComboBox *m_comboBondOrder;
      QCheckBox *m_addHydrogensCheck;
      QPushButton *m_tableButton;
      PeriodicTableView *m_periodicTable;
      QPushButton *m_fragmentButton;
      QVBoxLayout *m_layout;

      QWidget *m_settingsWidget;

      Atom *addAtom(GLWidget *widget, const QPoint& p);
      void moveAtom(GLWidget *widget, Atom *atom, const QPoint& p);
      Bond *addBond(Molecule *molecule, Atom *beginAtom, Atom *endAtom);

      OpenBabel::OBForceField *m_forceField;

    private Q_SLOTS:
      void settingsWidgetDestroyed();
  };

  class DrawToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(DrawTool)
  };

} // end namespace Avogadro

#endif
