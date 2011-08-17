/**********************************************************************
  InsertFragment - Insert molecular fragments or SMILES

  Copyright (C) 2009-2001 by Geoffrey R. Hutchison

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

#include "insertfragmentextension.h"
#include "insertcommand.h"

#include <avogadro/atom.h>
#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/primitivelist.h>

#include <openbabel/mol.h>
#include <openbabel/builder.h>
#include <openbabel/forcefield.h>
#include <openbabel/obconversion.h>

#include <QInputDialog>
#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  enum FragmentIndex
  {
    CrystalFromFileIndex = 0,
    FragmentFromFileIndex,
    SMILESIndex
  };

  InsertFragmentExtension::InsertFragmentExtension(QObject *parent) :
    Extension(parent),
    m_fragmentDialog(0),
    m_crystalDialog(0),
    m_molecule(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Crystal..."));
    action->setData(CrystalFromFileIndex);
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("Fragment..."));
    action->setData(FragmentFromFileIndex);
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("SMILES..."));
    action->setData(SMILESIndex);
    m_actions.append(action);

    // Dialog is created later, if needed
  }

  InsertFragmentExtension::~InsertFragmentExtension()
  {
    if (m_fragmentDialog) {
      delete m_fragmentDialog;
      m_fragmentDialog = 0;
    }
    if (m_crystalDialog) {
      delete m_crystalDialog;
      m_crystalDialog = 0;
    }
  }

  QList<QAction *> InsertFragmentExtension::actions() const
  {
    return m_actions;
  }

  QString InsertFragmentExtension::menuPath(QAction *) const
  {
    return tr("&Build") + '>' + tr("&Insert");
  }

  void InsertFragmentExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* InsertFragmentExtension::performAction(QAction *action,
                                                       GLWidget *widget)
  {
    if (m_molecule == NULL || widget == NULL)
      return NULL; // nothing we can do

    if (action->data() == SMILESIndex) {
      // Read a SMILES and use the OBBuilder class to build it and insert the new fragment
      OBBuilder builder;
      Molecule fragment;
      OBMol obfragment;
      OBConversion conv;

      bool ok, noConnection;
      QList<int> selectedIds;
      QString smiles = QInputDialog::getText((widget),
                                             tr("Insert SMILES"),
                                             tr("Insert SMILES fragment:"),
                                             QLineEdit::Normal,
                                             m_smilesString, &ok);
      if (ok && !smiles.isEmpty()) {
        m_smilesString = smiles; // save for settings
        std::string SmilesString(smiles.toAscii());

        QList<Primitive *> selectedAtoms = widget->selectedPrimitives().subList(Primitive::AtomType);
        if (!selectedAtoms.empty()) {
          // Loop through the selection and add the ids
          // But if it's a hydrogen, we need to find the attached parent
          // (making sure it's not also selected)
          selectedIds.append(findSelectedForInsert(selectedAtoms));
          noConnection = false;
        } else {
          selectedIds.append(-1);
          noConnection = true;
        }

        if(conv.SetInFormat("smi")
           && conv.ReadString(&obfragment, SmilesString))
          {
            builder.Build(obfragment);

            // Let's do a quick cleanup
            OBForceField* pFF =  OBForceField::FindForceField("MMFF94");
            if (pFF && pFF->Setup(obfragment)) {
              pFF->ConjugateGradients(250, 1.0e-4);
              pFF->UpdateCoordinates(obfragment);
            } // Note tricky assignment used as logic below
            else if ((pFF = OBForceField::FindForceField("UFF")) && pFF->Setup(obfragment)) {
              pFF->ConjugateGradients(250, 1.0e-4);
              pFF->UpdateCoordinates(obfragment);
            }

            fragment.setOBMol(&obfragment);
            if (noConnection) { // if we're not connecting to a specific atom, add Hs, center
              fragment.addHydrogens(); // hydrogen addition is done by InsertCommand when bonding
              fragment.center();
            }
          }
      }

      foreach(int id, selectedIds) {
        emit performCommand(new InsertFragmentCommand(m_molecule, fragment, widget, tr("Insert SMILES"), id));
      }
    } else if (action->data() == FragmentFromFileIndex) { // molecular fragments
        if (m_fragmentDialog == NULL) {
          m_fragmentDialog = new InsertFragmentDialog(widget, "fragments");
          m_fragmentDialog->setWindowTitle(tr("Insert Fragment"));
          connect(m_fragmentDialog, SIGNAL(performInsert()), this, SLOT(insertFragment()));
        }
        m_fragmentDialog->show();

    } else { // crystals
      if (m_crystalDialog == NULL) {
        m_crystalDialog = new InsertFragmentDialog(widget, "crystals");
        m_crystalDialog->setWindowTitle(tr("Insert Crystal"));
        connect(m_crystalDialog, SIGNAL(performInsert()), this, SLOT(insertCrystal()));
      }
      m_crystalDialog->show();
    }

    m_widget = widget; // save for delayed response

    return NULL; // delayed action on user clicking the Insert button
  }

  void InsertFragmentExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
    settings.setValue("smiles", m_smilesString);
    /* @todo bring back multiple directory paths
    if (m_dialog) {
      settings.setValue("fragmentPath", m_dialog->directoryList().join("\n"));
    }
    */
  }

  void InsertFragmentExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);

    m_smilesString = settings.value("smiles").toString();
    /*
    if(m_dialog) {
      if (settings.contains("fragmentPath")) {
        QString directoryList = settings.value("fragmentPath").toString();
        m_dialog->setDirectoryList(directoryList.split('\n'));
      }
    }
    */
  }

  QList<int> InsertFragmentExtension::findSelectedForInsert(QList<Primitive*> selectedAtomList) const
  {
    QList<int> selectedIds;

    foreach(const Primitive *primitive, selectedAtomList) {
      const Atom *atom = static_cast<const Atom*>(primitive); // we know it's an atom, since AtomType was requested
      if (!atom->isHydrogen()) {
        // Only append if it doesn't have a selected hydrogen attached
        bool noSelectedHatoms = true;
        foreach (unsigned long int neighborId, atom->neighbors())
          {
            Atom *neighbor = m_molecule->atomById(neighborId);
            if (neighbor->isHydrogen()) { // check if it's selected
              if (selectedAtomList.contains(neighbor)) {
                noSelectedHatoms = false;
                break;
              }
            }
          }
        if (noSelectedHatoms) // add the heavy atom
          selectedIds.append(atom->id());

      } else {
        const Atom *hydrogen = atom;
        if (!hydrogen->neighbors().empty()) {
          atom = m_molecule->atomById(hydrogen->neighbors()[0]); // the first bonded atom to this "H"
        }
        selectedIds.append(atom->id());
      }
    }

    return selectedIds;
  }

  void InsertFragmentExtension::insertCrystal()
  {
    InsertFragmentDialog *dialog = qobject_cast<InsertFragmentDialog *>(this->sender());
    if (!dialog)
      return;

    const Molecule fragment = dialog->fragment();
    if (fragment.numAtoms() == 0)
      return;

    *m_molecule = fragment;
    m_molecule->update();
    emit moleculeChanged(m_molecule, Extension::NewWindow);
  }

  // only called by the fragment dialog (not SMILES)
  void InsertFragmentExtension::insertFragment()
  {
    InsertFragmentDialog *dialog = qobject_cast<InsertFragmentDialog *>(this->sender());
    if (!dialog)
      return;

    // Get the fragment and make sure it exists (e.g., we didn't try to insert a directory
    const Molecule fragment = dialog->fragment();
    if (fragment.numAtoms() == 0)
      return;

    // Check to see if we're going to connect to an existing atom using OBBuilder::Connect()
    QList<Primitive *> selectedAtoms = m_widget->selectedPrimitives().subList(Primitive::AtomType);
    QList<int> selectedIds;
    if (!selectedAtoms.empty()) {
      // Loop through the selection and add the ids
      // But if it's a hydrogen, we need to find the attached parent
      // (making sure it's not also selected)
      selectedIds.append(findSelectedForInsert(selectedAtoms));
    } else {
      selectedIds.append(-1);
    }

    foreach(int id, selectedIds) {
      emit performCommand(new InsertFragmentCommand(m_molecule, fragment, m_widget, tr("Insert Fragment"), id));
    }
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(insertfragmentextension, Avogadro::InsertFragmentExtensionFactory)

