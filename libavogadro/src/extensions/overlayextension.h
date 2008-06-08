/**********************************************************************
  Overlay - 2D Overlay Extension

  Copyright (C) 2008 by Tim Vandermeersch

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

#ifndef OVERLAYEXTENSION_H
#define OVERLAYEXTENSION_H

#include <avogadro/extension.h>
#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>

#include <QGraphicsView>

namespace Avogadro {

  class OverlayExtension : public Extension
  {
    Q_OBJECT

    public:
      //! Constructor
      OverlayExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~OverlayExtension();

      virtual QString name() const { return QObject::tr("Overlay"); }
      virtual QString description() const { return QObject::tr("2D Overlay Extension"); };

      virtual QList<QAction *> actions() const;
      virtual QString menuPath(QAction *action) const;

      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

      void espOverlay(GLWidget *widget);
      void moleculeOverlay(Molecule *molecule, GLWidget *widget);

    private:
      QList<QAction *> m_actions;
      QGraphicsView *m_espView;
      QGraphicsView *m_moleculeView;

    private Q_SLOTS:
      void espResize();
      void moleculeResize();
  };

  class OverlayExtensionFactory : public QObject, public ExtensionFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::ExtensionFactory)

    public:
      Extension *createInstance(QObject *parent = 0) { return new OverlayExtension(parent); }
  };

} // end namespace Avogadro

#endif
