/**********************************************************************
  Ghemical - Ghemical Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

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

#ifndef __GHEMICALEXTENSION_H
#define __GHEMICALEXTENSION_H


#include "forcefielddialog.h"

#include <openbabel/mol.h>
#include <openbabel/forcefield.h>

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>
#include <QThread>

#ifndef BUFF_SIZE
#define BUFF_SIZE 256
#endif

class QProgressDialog;
namespace Avogadro {

 class GhemicalExtension : public QObject, public Extension
  {
    Q_OBJECT

    public:
      //! Constructor
      GhemicalExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~GhemicalExtension();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("Ghemical"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Ghemical Plugin"); };

      /** @return a menu path for the extension's actions */
      virtual QString menuPath() const
      { return QObject::tr("&Tools>&Molecular Mechanics"); }

      //! Perform Action
      virtual QList<Action *> actions() const;
      virtual QUndoCommand* performAction(Action *action, Molecule *molecule,
                                          GLWidget *widget, QTextEdit *textEdit);
      //@}

    private:
      OpenBabel::OBForceField* m_forceField;
      QList<Action *> m_actions;
      ForceFieldDialog *m_Dialog;
  };

  class GhemicalExtensionFactory : public QObject, public ExtensionFactory
  {
    Q_OBJECT;
    Q_INTERFACES(Avogadro::ExtensionFactory);

    public:
    Extension *createInstance(QObject *parent = 0) { return new GhemicalExtension(parent); }
  };

  class GhemicalThread : public QThread
  {
    Q_OBJECT;

    public:
    GhemicalThread(Molecule *molecule, OpenBabel::OBForceField* forceField,
        QTextEdit *textEdit, int forceFieldID, int nSteps, int algorithm,
        int gradients, int convergence, int task, QObject *parent=0);

      void run();
      int cycles() const;

    Q_SIGNALS:
      void stepsTaken(int steps);

    public Q_SLOTS:
      void stop();

    private:
      Molecule *m_molecule;
      QTextEdit *m_textEdit;

      int m_cycles;
      int m_forceFieldID;
      int m_nSteps;
      int m_algorithm;
      int m_gradients;
      int m_convergence;
      int m_task;

      OpenBabel::OBForceField* m_forceField;
      ForceFieldDialog *m_Dialog;

      bool m_stop;
  };

 class GhemicalCommand : public QUndoCommand
  {
    public:
      GhemicalCommand(Molecule *molecule, OpenBabel::OBForceField *forcefield, QTextEdit *messages,
                      int forceFieldID, int nSteps, int algorithm, int gradients, int convergence, int task);

      ~GhemicalCommand();

      virtual void redo();
      virtual void undo();
      virtual bool mergeWith ( const QUndoCommand * command );
      virtual int id() const;

      void detach() const;
      void cleanup();

      GhemicalThread *thread() const;
      QProgressDialog *progressDialog() const;

    private:
      Molecule m_moleculeCopy;

      int m_nSteps;
      Molecule *m_molecule;
      QTextEdit *m_textEdit;

      GhemicalThread *m_thread;
      QProgressDialog *m_dialog;

      mutable bool m_detached;
  };

} // end namespace Avogadro

#endif
