/**********************************************************************
  MolecularProp - Standard properties of molecules

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

#include "molecularpropextension.h"

#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>

#include <openbabel/mol.h>

#include <QAction>
#include <QString>

using namespace OpenBabel;

namespace Avogadro {

  MolecularPropertiesExtension::MolecularPropertiesExtension(QObject *parent) : Extension(parent), m_molecule(0), m_dialog(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Molecule Properties..."));
    m_actions.append(action);
  }

  MolecularPropertiesExtension::~MolecularPropertiesExtension()
  {
  }

  QList<QAction *> MolecularPropertiesExtension::actions() const
  {
    return m_actions;
  }

  QString MolecularPropertiesExtension::menuPath(QAction *) const
  {
    return tr("&View") + '>' + tr("&Properties");
  }

  void MolecularPropertiesExtension::setMolecule(Molecule *molecule)
  {
    if (m_molecule)
      disconnect( m_molecule, 0, this, 0 );

    m_molecule = molecule;

    connect(m_molecule, SIGNAL(moleculeChanged()), this, SLOT(update()));
    connect(m_molecule, SIGNAL(primitiveAdded(Primitive *)),
            this, SLOT(updatePrimitives(Primitive*)));
    connect(m_molecule, SIGNAL(primitiveRemoved(Primitive *)),
            this, SLOT(updatePrimitives(Primitive*)));
    connect(m_molecule, SIGNAL(primitiveUpdated(Primitive *)),
            this, SLOT(updatePrimitives(Primitive*)));

    connect(m_molecule, SIGNAL(atomAdded(Atom *)),
            this, SLOT(updateAtoms(Atom*)));
    connect(m_molecule, SIGNAL(atomRemoved(Atom *)),
            this, SLOT(updateAtoms(Atom*)));
    connect(m_molecule, SIGNAL(atomUpdated(Atom *)),
            this, SLOT(updateAtoms(Atom*)));

    connect(m_molecule, SIGNAL(bondAdded(Bond *)),
            this, SLOT(updateBonds(Bond*)));
    connect(m_molecule, SIGNAL(bondRemoved(Bond *)),
            this, SLOT(updateBonds(Bond*)));
    connect(m_molecule, SIGNAL(bondUpdated(Bond *)),
            this, SLOT(updateBonds(Bond*)));
  }

  QUndoCommand* MolecularPropertiesExtension::performAction(QAction *,
                                                            GLWidget *widget)
  {
    if (!m_molecule)
      return 0; // nothing we can do

    // Disconnect in case we're attached to a new widget
    if (m_widget)
      disconnect( m_molecule, 0, this, 0 );

    if (widget) {
      connect(widget, SIGNAL(moleculeChanged(Molecule *)),
              this, SLOT(moleculeChanged(Molecule*)));
      m_widget = widget;
    }

    if (!m_dialog) {
      m_dialog = new MolecularPropertiesDialog(m_widget);
      // Disable the dipole moment for now
      m_dialog->dipoleLabel->setVisible(false);
      m_dialog->dipoleMomentLine->setVisible(false);
    }

    m_dialog->show();
    update();

    return 0;
  }

  void MolecularPropertiesExtension::update()
  {
    if (m_dialog == NULL || m_molecule == NULL)
      return;

    QString format("%L1"); // localized numbers
    OpenBabel::OBMol obmol = m_molecule->OBMol();
    m_dialog->molecularWeightLine->setText(format.arg(obmol.GetMolWt(), 0, 'f', 3));

    // Copied from Kalzium
    QString formula(obmol.GetSpacedFormula(1,"").c_str());
    formula.replace( QRegExp( "(\\d+)" ), "<sub>\\1</sub>" );
    m_dialog->formulaLine->setText(formula);

    m_dialog->energyLine->setText(format.arg(m_molecule->energy(), 0, 'f', 3));
    bool estimate = true; // estimated dipole
    //    m_dialog->dipoleMomentLine->setText(format.arg(m_molecule->dipoleMoment(&estimate)->norm(), 0, 'f', 3));
    if (estimate)
      m_dialog->dipoleLabel->setText(tr("Estimated Dipole Moment (D):"));
    m_dialog->atomsLine->setText(format.arg(m_molecule->numAtoms()));
    m_dialog->bondsLine->setText(format.arg(m_molecule->numBonds()));
    if (m_molecule->numResidues() < 2) {
      m_dialog->residuesLabel->hide();
      m_dialog->residuesLine->hide();
    }
    else {
      m_dialog->residuesLabel->show();
      m_dialog->residuesLine->show();
      m_dialog->residuesLine->setText(format.arg(m_molecule->numResidues()));
    }
  }

  void MolecularPropertiesExtension::updatePrimitives(Primitive*)
  {
    update();
  }

  void MolecularPropertiesExtension::updateAtoms(Atom*)
  {
    update();
  }

  void MolecularPropertiesExtension::updateBonds(Bond*)
  {
    update();
  }

  void MolecularPropertiesExtension::moleculeChanged(Molecule *)
  {
    update();
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(molecularpropertiesextension,
                 Avogadro::MolecularPropertiesExtensionFactory)

