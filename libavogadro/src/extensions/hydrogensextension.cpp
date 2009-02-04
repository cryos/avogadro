/**********************************************************************
  Hydrogens - Hydrogens Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "hydrogensextension.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <QAction>
#include <QInputDialog>
#include <QString>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  HydrogensExtension::HydrogensExtension(QObject *parent) : Extension(parent), m_molecule(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Add Hydrogens"));
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("Add Hydrogens for pH..."));
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("Remove Hydrogens"));
    m_actions.append(action);

    action = new QAction( this );
    action->setSeparator(true);
    m_actions.append( action );
  }

  HydrogensExtension::~HydrogensExtension()
  {
  }

  QList<QAction *> HydrogensExtension::actions() const
  {
    return m_actions;
  }

  QString HydrogensExtension::menuPath(QAction *) const
  {
    return tr("&Build");
  }

  void HydrogensExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* HydrogensExtension::performAction(QAction *action, GLWidget *widget)
  {

    QUndoCommand *undo = 0;
    int i = m_actions.indexOf(action);
    if( 0 <= i && i <= 2) {
      if(i == 1) {
        bool ok;
        double pH = QInputDialog::getDouble(0,
          tr("Add Hydrogens for pH"), tr("pH"), 7.4, 0.0, 14.0, 1, &ok);

        if (!ok)
          return undo;

        undo = new HydrogensCommand(m_molecule, (enum HydrogensCommand::Action) i, widget, pH);
      } else {
        undo = new HydrogensCommand(m_molecule, (enum HydrogensCommand::Action) i, widget);
      }
    }

    return undo;
  }

  HydrogensCommand::HydrogensCommand(Molecule *molecule, enum Action action,
      GLWidget *widget, double pH):
    m_molecule(molecule), m_moleculeCopy(new Molecule(*molecule)),
    m_SelectedList(widget->selectedPrimitives()), m_action(action), m_pH(pH)
  {
    // save the selection from the current view widget
    // (i.e., only modify a few hydrogens)
    //      m_SelectedList = widget->selectedPrimitives;

    switch(action) {
      case AddHydrogens:
        setText(QObject::tr("Add Hydrogens"));
        break;
      case AddHydrogensPH:
        setText(QObject::tr("Add Hydrogens for pH"));
        break;
      case RemoveHydrogens:
        setText(QObject::tr("Remove Hydrogens"));
        break;
    }
  }

  HydrogensCommand::~HydrogensCommand()
  {
    delete m_moleculeCopy;
  }

  void HydrogensCommand::redo()
  {
    if (m_SelectedList.size() == 0) {
      switch(m_action) {
        case AddHydrogens:
          m_molecule->addHydrogens();
          break;
        case AddHydrogensPH:
          /// FIXME - need to add back in pH corrected
          m_molecule->addHydrogens();
/*	  m_molecule->UnsetFlag(OB_PH_CORRECTED_MOL);
	  FOR_ATOMS_OF_MOL (a, m_molecule)
            a->SetFormalCharge(0.0);
	  m_molecule->SetAutomaticFormalCharge(true);
          m_molecule->AddHydrogens(false, true, m_pH); */
          break;
        case RemoveHydrogens:
          m_molecule->removeHydrogens();
          break;
      }
    }
    else { // user selected some atoms, only operate on those

      foreach(unsigned long id, m_SelectedList.subList(Primitive::AtomType))
      {
        Atom *atom = m_molecule->atomById(id);
        if(atom)
        {
          switch(m_action) {
            case AddHydrogens:
              m_molecule->addHydrogens(atom);
              break;
            case RemoveHydrogens:
              m_molecule->removeHydrogens(atom);
              break;
            default:
              break;
          }
        }
      }
    } // end adding to selected atoms
    m_molecule->update();
  }

  void HydrogensCommand::undo()
  {
    *m_molecule = m_moleculeCopy;
    m_molecule->update();
  }

  bool HydrogensCommand::mergeWith ( const QUndoCommand * )
  {
    // we received another call of the same action
    return true;
  }

  int HydrogensCommand::id() const
  {
    return 4709537 + (int) m_action;
  }

} // end namespace Avogadro

#include "hydrogensextension.moc"
Q_EXPORT_PLUGIN2(hydrogensextension, Avogadro::HydrogensExtensionFactory)
