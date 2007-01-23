/**********************************************************************
  Tool - Qt Plugin Template

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

#ifndef __TOOL_H
#define __TOOL_H

#include <avogadro/primitives.h>

#include <QString>
#include <QAction>
#include <QObject>
#include <QGLWidget>
#include <QVector>
#include <QList>

namespace Avogadro {

  class GLWidget;

  //! Base class for our tools
  class Tool
  {
    public:
      Tool() : m_action(new QAction(0)) { 
        m_action->setText(name());
        m_action->setToolTip(description());
        m_action->setIcon(QIcon(QString::fromUtf8(":/icons/tool.png")));
        m_action->setCheckable(true);
        m_action->setData(qVariantFromValue(this));
      }
      //! Deconstructor
      virtual ~Tool() { delete m_action; }

      //! \name Description methods
      //@{
      //! Tool Name (ie Draw)
      virtual QString name() { return QObject::tr("Unknown"); }
      //! Tool Description (ie. Draws atoms and bonds)
      virtual QString description() { return QObject::tr("Unknown Tool"); };
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      //! Random calls.
      virtual void init() {}
      virtual void cleanup() {}

      virtual QAction* action() const { return m_action; }
      virtual void mousePress(Molecule *molecule, GLWidget *widget, const QMouseEvent *event) = 0;
      virtual void mouseRelease(Molecule *molecule, GLWidget *widget, const QMouseEvent *event) = 0;
      virtual void mouseMove(Molecule *molecule, GLWidget *widget, const QMouseEvent *event) = 0;
      //@}
      
    protected:
      QAction *m_action;

//X:     protected:
//X:       //! \brief Select a region of the widget.
//X:       //! (x,y) top left coordinate of region.
//X:       //! (w,h) width and heigh of region.
//X:       QList<GLHit> selectRegion(GLWidget *widget, int x, int y, int w, int h);

  };

} // end namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Tool*)
Q_DECLARE_INTERFACE(Avogadro::Tool, "net.sourceforge.avogadro.tool/1.0")


#endif
