/**********************************************************************
  AlignTool - AlignTool Tool for Avogadro

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

#ifndef ALIGNTOOL_H
#define ALIGNTOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QVarLengthArray>
#include <QImage>
#include <QAction>
#include <QPointer>

namespace Avogadro {

 class AlignTool : public Tool
  {
    Q_OBJECT

    public:
      //! Constructor
      AlignTool(QObject *parent = 0);
      //! Destructor
      virtual ~AlignTool();

      //! \name Description methods
      //@{
      //! Tool Name (i.e. Align)
      virtual QString name() const { return(tr("Align")); }
      //! Tool Description (i.e. Align molecules)
      virtual QString description() const { return(tr("Align molecules")); }
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

      virtual bool paint(GLWidget *widget);

      virtual QWidget *settingsWidget();

    public Q_SLOTS:
      void axisChanged(int axis);
      void alignChanged(int align);
      void align();

    private:
      // Guarded pointers, for storing pointers to things that might go poof...
      QPointer<Molecule> m_molecule;
      QVarLengthArray<QPointer<Atom>, 2> m_selectedAtoms;
      int m_numSelectedAtoms;
      int m_axis;
      int m_alignType;

      QWidget *m_settingsWidget;

    private Q_SLOTS:
      void settingsWidgetDestroyed();

  };

  class AlignToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(AlignTool, tr("Align Tool"), tr("Align  molecules with XYZ axes."))
  };

} // end namespace Avogadro

#endif
