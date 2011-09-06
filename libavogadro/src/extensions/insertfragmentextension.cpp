/**********************************************************************
  InsertFragment - Insert molecular fragments or SMILES

  Copyright (C) 2009 by Geoffrey R. Hutchison

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
    FragmentFromFileIndex = 0,
    SMILESIndex
  };

  InsertFragmentExtension::InsertFragmentExtension(QObject *parent) :
    Extension(parent),
    m_dialog(0),
    m_molecule(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Fragment..."));
    action->setData(FragmentFromFileIndex);
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("SMILES..."));
    action->setData(SMILESIndex);
    m_actions.append(action);
  }

  InsertFragmentExtension::~InsertFragmentExtension()
  {
    if (m_dialog) {
      delete m_dialog;
      m_dialog = 0;
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

      OBBuilder builder;
      Molecule fragment;
      OBMol obfragment;
      OBConversion conv;

      int selectedAtom = -1;
      bool ok;
      QString smiles = QInputDialog::getText((widget),
                                             tr("Insert SMILES"),
                                             tr("Insert SMILES fragment:"),
                                             QLineEdit::Normal,
                                             m_smilesString, &ok);
      if (ok && !smiles.isEmpty()) {
        m_smilesString = smiles; // save for settings
        std::string SmilesString(smiles.toAscii());

        QList<Primitive *> selectedAtoms = widget->selectedPrimitives().subList(Primitive::AtomType);
        if (selectedAtoms.size() == 1) { // TODO: Expand to handle multiple addition points
          selectedAtom = selectedAtoms[0]->id();
        }

        if(conv.SetInFormat("smi")
           && conv.ReadString(&obfragment, SmilesString))
          {
            builder.Build(obfragment);

            OBForceField* pFF =  OBForceField::FindForceField("UFF");
            if (pFF && pFF->Setup(obfragment)) {
              pFF->ConjugateGradients(250, 1.0e-4);
              pFF->UpdateCoordinates(obfragment);
            }

            fragment.setOBMol(&obfragment);
            if (selectedAtom == -1) { // if we're not connecting to a specific atom, add Hs, center
              fragment.addHydrogens(); // hydrogen addition is done by InsertCommand when connecting
              fragment.center();
            }
          }
      }

      return new InsertFragmentCommand(m_molecule, fragment, widget, tr("Insert SMILES"), selectedAtom);
    }
    else if (action->data() == FragmentFromFileIndex) {
      m_widget = widget; // save for delayed response

      if (m_dialog == NULL) {
        m_dialog = new InsertFragmentDialog(widget);
        connect(m_dialog, SIGNAL(insertClicked()), this, SLOT(performInsert()));
      }
      m_dialog->show();

      return NULL; // delayed action on user clicking the Insert button
    }
    return NULL; // some other action
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

  // only called by the fragment dialog (not SMILES)
  void InsertFragmentExtension::performInsert()
  {
    if (m_dialog) {
      // check to see if we're going to connect to an existing atom using OBBuilder::Connect()
      int selectedAtom = -1;
      QList<Primitive *> selectedAtoms = m_widget->selectedPrimitives().subList(Primitive::AtomType);
      if (selectedAtoms.size() == 1) { // TODO: Expand to handle multiple addition points
        selectedAtom = selectedAtoms[0]->id();
      }

      emit performCommand(new InsertFragmentCommand(m_molecule, m_dialog->fragment(), m_widget, tr("Insert Fragment"), selectedAtom));
    }
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(insertfragmentextension, Avogadro::InsertFragmentExtensionFactory)

