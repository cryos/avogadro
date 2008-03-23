#ifndef DOCKINGEXTENSION_H
#define DOCKINGEXTENSION_H
/**********************************************************************
  dockingextension.h - Docking plugin for Avogadro

  Copyright (C) 2007 by Tim Vandermeersch

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


#include "liganddialog.h"
#include "pocketdialog.h"
#include <openbabel/dock.h>

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>
#include <QThread>
#include <QMutex>

#ifndef BUFF_SIZE
#define BUFF_SIZE 256
#endif

class QProgressDialog;
namespace Avogadro {

 class DockingExtension : public Extension
  {
    Q_OBJECT

    public slots:
      void primitiveRemoved(Primitive *primitive);

    public:
      //! Constructor
      DockingExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~DockingExtension();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("Docking"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Docking Plugin"); };

      /** @return a menu path for the extension's actions */
      virtual QString menuPath(QAction *action) const;

      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, Molecule *molecule,
                                          GLWidget *widget, QTextEdit *textEdit);
      //@}

      //! Select the atoms from m_ligand
      void selectLigand();
      //! Select the atoms from m_pocket
      void selectPocket();
    private:
      OpenBabel::OBDock* m_dock;
      QList<QAction *> m_actions;
      LigandDialog *m_ligandDialog;
      PocketDialog *m_pocketDialog;
      std::vector<int> m_ligand;
      std::vector<int> m_pocket;
      QString m_fileName;
  };

  class DockingExtensionFactory : public QObject, public ExtensionFactory
  {
    Q_OBJECT;
    Q_INTERFACES(Avogadro::ExtensionFactory);

    public:
    Extension *createInstance(QObject *parent = 0) { return new DockingExtension(parent); }
  };
/*
  class DockingThread : public QThread
  {
    Q_OBJECT;

    public:
    DockingThread(Molecule *molecule, OpenBabel::OBDock* dock,
        QTextEdit *textEdit,
	int forceFieldID, int nSteps, int algorithm, int gradients,
	int convergence, int task, QObject *parent=0);

      void run();
      int cycles() const;
      void setTask(int task);
      void setNumConformers(int numConformers);

    Q_SIGNALS:
      void stepsTaken(int steps);

    public Q_SLOTS:
      void stop();

    private:
      Molecule *m_molecule;
      OpenBabel::OBDock* m_dock;
      QTextEdit *m_textEdit;

      QMutex m_mutex;

      int m_cycles;
      int m_forceFieldID;
      int m_nSteps;
      int m_algorithm;
      int m_gradients;
      int m_convergence;
      int m_task;
      int m_numConformers;
      bool m_stop;
  };

 class DockingCommand : public QUndoCommand
 {
   public:
     DockingCommand(Molecule *molecule, OpenBabel::OBDock *dock,
         QTextEdit *messages, int forceFieldID,
	 int nSteps, int algorithm, int gradients, int convergence, int task);

     ~DockingCommand();

     virtual void redo();
     virtual void undo();
     virtual bool mergeWith ( const QUndoCommand * command );
     virtual int id() const;

     void detach() const;
     void cleanup();
     void setTask(int task);
     void setNumConformers(int numConformers);

     DockingThread *thread() const;
     QProgressDialog *progressDialog() const;

   private:
     Molecule m_moleculeCopy;

     int m_nSteps;
     int m_task;
     int m_numConformers;
     Molecule *m_molecule;
     OpenBabel::OBDock* m_dock;
     QTextEdit *m_textEdit;

     DockingThread *m_thread;
     QProgressDialog *m_dialog;

     mutable bool m_detached;

 };
*/
} // end namespace Avogadro

#endif // DOCKINGEXTENSION_H
