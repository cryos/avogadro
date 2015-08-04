/**********************************************************************
  SymmetryExtension - Point group perception and symmetrization via libmsym

  Copyright (C) 2015 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#ifndef SYMMETRYEXTENSION_H
#define SYMMETRYEXTENSION_H

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include "ui_symmetrydialog.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>
#include <QCloseEvent>

  namespace msym {
    extern "C" {
#include "libmsym/src/msym.h"
    }
  }

namespace Avogadro {

  class SymmetryDialog : public QDialog, public Ui::SymmetryDialog
    {
      public:
      SymmetryDialog(QWidget *parent=0) : QDialog(parent) {
        setupUi(this);
      }
    };

 class SymmetryExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Symmetry", tr("Symmetry Properties"),
                       tr("Display point group symmetry."))

    public:
      //! Constructor
      SymmetryExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~SymmetryExtension();

      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
      virtual QString menuPath(QAction *action) const;
      //@}

      void setMolecule(Molecule *molecule);

      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);

    public Q_SLOTS:
      // Slots to take signals from Molecules, and GLWidget
      void update();
      void updatePrimitives(Primitive*);
      void updateAtoms(Atom*);
      void updateBonds(Bond*);
      void moleculeChanged(Molecule *previous);

      void disableUpdating();

      void symmetrize();
      void detectSymmetry();

      void toleranceChanged(int);

    private:
      QList<QAction *> m_actions;
      Molecule *m_molecule;
      GLWidget *m_widget;

      SymmetryDialog *m_dialog;

      msym::msym_context ctx;
      int m_tolerance;

      void constructDialog();
      QString pgSymbol(char *);
  };

  class SymmetryExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(SymmetryExtension)
  };


} // end namespace Avogadro

#endif
