/**********************************************************************
  Plugin - Avogadro Plugin Interfaces

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

#ifndef __PLUGIN_H
#define __PLUGIN_H

#include <avogadro/primitives.h>

#include <QString>
#include <QAction>
#include <QObject>
#include <QGLWidget>
#include <QVector>
#include <QList>

namespace Avogadro {

  class ExtensionAction;

  //! Base class for extension plugins
  class Extension
  {
    public:
      //! Deconstructor
      virtual ~Extension() { }

      //! \name Extension Methods
      //@{
      //! \brief Returns set of actions which the 
      //! extension can perform.
      //@}
      virtual QList<QAction *> actions();
      
    protected:
      QList<QAction *> m_actions;

  };

  class ExtensionAction : QAction
  {
    Q_OBJECT
    public:
      ExtensionAction(QObject *parent=0) : QAction(parent) {}

    signals:
      void triggered(Molecule *molecule);
  };

  class GLWidget;

  //! Base class for tool plugins
  class Tool
  {
    public:
      Tool() : m_selectAction(new QAction(0)), m_propertiesWidget(new QWidget(0)) { 
        m_selectAction->setCheckable(true); 
        m_selectAction->setData(qVariantFromValue(this));
        m_selectAction->setIcon(QIcon(QString::fromUtf8(":/icons/tool.png")));
      }
      //! Deconstructor
      virtual ~Tool() { delete m_selectAction; }

      //! \name Description methods
      //@{
      //! Tool Name (ie Draw)
      virtual QString name() const { return QObject::tr("Unknown"); }
      //! Tool Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Unknown Tool"); };
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      //! Random calls.
      virtual void init() {}
      virtual void cleanup() {}

      virtual QAction* selectAction() const {
        // set tooltip first, else text() will set it.
        if(m_selectAction->toolTip() == "")
          m_selectAction->setToolTip(description());
        
        if(m_selectAction->text() == "")
          m_selectAction->setText(name());
        
        
        return m_selectAction; 
      }

      virtual QWidget* propertiesWidget() const {
        return m_propertiesWidget;
      }
      virtual void mousePress(Molecule *molecule, GLWidget *widget, const QMouseEvent *event) = 0;
      virtual void mouseRelease(Molecule *molecule, GLWidget *widget, const QMouseEvent *event) = 0;
      virtual void mouseMove(Molecule *molecule, GLWidget *widget, const QMouseEvent *event) = 0;
      //@}
      
    protected:
      QAction *m_selectAction;
      QWidget *m_propertiesWidget;

//X:     protected:
//X:       //! \brief Select a region of the widget.
//X:       //! (x,y) top left coordinate of region.
//X:       //! (w,h) width and heigh of region.
//X:       QList<GLHit> selectRegion(GLWidget *widget, int x, int y, int w, int h);

  };
} // end namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Tool*)
Q_DECLARE_INTERFACE(Avogadro::Tool, "net.sourceforge.avogadro.tool/1.0")
Q_DECLARE_INTERFACE(Avogadro::Extension, "net.sourceforge.avogadro.extension/1.0")


#endif
