/**********************************************************************
  Polymer Tool - Make easy polymers, including custom monomer units

  Copyright (C) 2012 Geoffrey R. Hutchison

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

#ifndef POLYMERTOOL_H
#define POLYMERTOOL_H

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

  class PolymerSettingsWidget;
  class PolymerTool : public Tool
  {
    Q_OBJECT
    AVOGADRO_TOOL("Polymer", tr("Polymer"),
                  tr("Build polymers and co-polymers"),
                  tr("Polymer Settings"))

    public:
      //! Constructor
      PolymerTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~PolymerTool();

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

      // Generate the monomer list from disk
      QStringList monomerList();
      QString getSmilesFromFile(QString fileName);

    public Q_SLOTS:

      void monomerBChanged(int index);
      QUndoCommand* build();

    protected:
      bool                m_leftButtonPressed;  // rotation
      bool                m_rightButtonPressed;
      bool                m_movedSinceButtonPressed;
      bool                m_doubleClick;

      QString             m_prefix; // directory prefix for monomer files
      QStringList         m_monomerList;

      GLWidget           *m_widget;
      Molecule           *m_molecule;

      QList<GLHit>        m_hits;
      QList<QPointer<Atom> > m_selectedAtoms;
      int m_numSelectedAtoms;

      PolymerSettingsWidget *m_settingsWidget;
  };

  class PolymerToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(PolymerTool)
  };

} // end namespace Avogadro

#endif
