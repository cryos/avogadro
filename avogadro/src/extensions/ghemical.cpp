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

#include "ghemical.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;

  namespace Avogadro {
    Ghemical::Ghemical() : Extension()
    {
      m_forceField = OBForceField::FindForceField("Ghemical");

      if (m_forceField) { // make sure we can actually find and run it!
        QAction *action = new QAction(this);
        action->setText("Optimize Geometry");
        m_actions.append(action);
      }
    }

    Ghemical::~Ghemical() 
    {
    }

    QUndoCommand* Ghemical::performAction(QAction *action, Molecule *molecule, QTextEdit *textEdit)
    {
      QUndoCommand *undo = new GhemicalCommand(molecule, m_forceField, textEdit);
      undo->setText(QObject::tr("Ghemical Geometric Optimization"));

      return undo;
    }

    GhemicalCommand::GhemicalCommand(Molecule *molecule, OpenBabel::OBForceField* forceField, QTextEdit *textEdit)
    {
      m_cycles = 0;
      m_moleculeCopy = *molecule;
      m_molecule = molecule;
      m_forceField = forceField;
      m_textEdit = textEdit;
    }

    void GhemicalCommand::redo() {

      ostringstream buff;
      m_forceField->SetLogFile(&buff);
      m_forceField->SetLogLevel(OBFF_LOGLVL_LOW);

      if (!m_forceField->Setup(*m_molecule)) {
        qWarning() << "GhemicalCommand: Could not set up force field on " << m_molecule;
        return;
      }

      m_forceField->ConjugateGradientsInitialize(100, 1e-7f); // initialize cg
      while (m_forceField->ConjugateGradientsTakeNSteps(5)) { // take 5 steps until convergence or 100 steps taken
        m_forceField->UpdateCoordinates(*m_molecule);
        m_textEdit->append(QObject::tr(buff.str().c_str()));
        m_molecule->update();
        m_cycles++;
      }
    }

    void GhemicalCommand::undo() {
      *m_molecule = m_moleculeCopy;
      for(int i=0; i<m_cycles; i++) {
        m_textEdit->undo();
      }
      m_cycles = 0;
    }

    bool GhemicalCommand::mergeWith ( const QUndoCommand * command )
    {
      // recieved another of the same call
      return true;
    }

    int GhemicalCommand::id() const
    {
      return 54381241;
    }

  } // end namespace Avogadro

#include "ghemical.moc"
Q_EXPORT_PLUGIN2(ghemical, Avogadro::Ghemical)
