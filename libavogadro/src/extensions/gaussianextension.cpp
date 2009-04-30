/**********************************************************************
  GaussianExtension - Extension for generating Gaussian input decks

  Copyright (C) 2008-2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "gaussianextension.h"

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <avogadro/molecule.h>

#include <QMessageBox>

using namespace OpenBabel;
using namespace std;

namespace Avogadro
{

  GaussianExtension::GaussianExtension(QObject* parent) : Extension(parent),
    m_gaussianInputDialog(0), m_qchemInputDialog(0), m_mopacInputDialog(0),
    m_nwchemInputDialog(0), m_molproInputDialog(0), m_molecule(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Gaussian Input..."));
    action->setData("Gaussian");
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("Q-Chem Input..."));
    action->setData("QChem");
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("MOLPRO Input..."));
    action->setData("Molpro");
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("MOPAC Input..."));
    action->setData("MOPAC");
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("&NWChem Input..."));
    action->setData("NWChem");
    m_actions.append(action);

    action = new QAction(this);
    action->setSeparator(true);
    m_actions.append(action);
  }

  GaussianExtension::~GaussianExtension()
  {
  }

  QList<QAction *> GaussianExtension::actions() const
  {
    return m_actions;
  }

  QString GaussianExtension::menuPath(QAction*) const
  {
    return tr("E&xtensions");
  }

  QUndoCommand* GaussianExtension::performAction(QAction *action, GLWidget *widget)
  {
    m_widget = widget;

    if (action->data() == "Gaussian") {
      if (!m_gaussianInputDialog) {
        m_gaussianInputDialog = new GaussianInputDialog(static_cast<QWidget*>(parent()));
        m_gaussianInputDialog->setMolecule(m_molecule);
        m_gaussianInputDialog->show();
      }
      else
        m_gaussianInputDialog->show();
    }
    else if (action->data() == "QChem") {
      if (!m_qchemInputDialog) {
        m_qchemInputDialog = new QChemInputDialog(static_cast<QWidget*>(parent()));
        m_qchemInputDialog->setMolecule(m_molecule);
        m_qchemInputDialog->show();
      }
      else
        m_qchemInputDialog->show();
    }
    else if (action->data() == "NWChem") {
      if (!m_nwchemInputDialog) {
        m_nwchemInputDialog = new NWChemInputDialog(static_cast<QWidget*>(parent()));
        m_nwchemInputDialog->setMolecule(m_molecule);
        m_nwchemInputDialog->show();
      }
      else
        m_nwchemInputDialog->show();
    }
    else if (action->data() == "Molpro") {
      if (!m_molproInputDialog) {
        m_molproInputDialog = new MolproInputDialog(static_cast<QWidget*>(parent()));
        m_molproInputDialog->setMolecule(m_molecule);
        m_molproInputDialog->show();
      }
      else
        m_molproInputDialog->show();
    }
    else if (action->data() == "MOPAC") {
      if (!m_mopacInputDialog) {
        m_mopacInputDialog = new MOPACInputDialog(static_cast<QWidget*>(parent()));
        connect(m_mopacInputDialog, SIGNAL(readOutput(QString)),
          this, SLOT(readOutputFile(QString)));
        m_mopacInputDialog->setMolecule(m_molecule);
        m_mopacInputDialog->show();
      }
      else {
        m_mopacInputDialog->show();
      }
    }
    return 0;
  }

  void GaussianExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    if (m_gaussianInputDialog)
      m_gaussianInputDialog->setMolecule(m_molecule);
    if (m_qchemInputDialog)
      m_qchemInputDialog->setMolecule(m_molecule);
    if (m_nwchemInputDialog)
      m_nwchemInputDialog->setMolecule(m_molecule);
    if (m_molproInputDialog)
      m_molproInputDialog->setMolecule(m_molecule);
    if (m_mopacInputDialog)
      m_mopacInputDialog->setMolecule(m_molecule);
  }

  void GaussianExtension::readOutputFile(const QString filename)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    OBConversion conv;
    OBFormat     *inFormat = conv.FormatFromExt(( filename.toAscii() ).data() );
    if ( !inFormat || !conv.SetInFormat( inFormat ) ) {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(m_widget, tr("Avogadro"),
        tr("Cannot read file format of file %1.").arg(filename));
      return;
    }

    ifstream ifs;
    ifs.open((filename.toAscii()).data());
    if (!ifs) { // shouldn't happen, already checked file above
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(m_widget, tr("Avogadro"),
        tr("Cannot read file %1.").arg( filename ) );
      return;
    }

    OBMol *obmol = new OBMol;
    if (conv.Read(obmol, &ifs)) {
      Molecule *mol = new Molecule;
      mol->setOBMol(obmol);
      mol->setFileName(filename);
      emit moleculeChanged(mol);
      m_molecule = mol;
    }

    QApplication::restoreOverrideCursor();
  }

  void GaussianExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
    if (m_gaussianInputDialog) {
      m_gaussianInputDialog->writeSettings(settings);
    }
    if (m_mopacInputDialog) {
      m_mopacInputDialog->writeSettings(settings);
    }
  }

  void GaussianExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
    if (m_gaussianInputDialog) {
      m_gaussianInputDialog->readSettings(settings);
    }
    else {
      m_gaussianInputDialog = new GaussianInputDialog(static_cast<QWidget*>(parent()));
      m_gaussianInputDialog->readSettings(settings);
      if (m_molecule) {
        m_gaussianInputDialog->setMolecule(m_molecule);
      }
    }

    if (m_mopacInputDialog) {
      m_mopacInputDialog->readSettings(settings);
    }
    else {
      m_mopacInputDialog = new MOPACInputDialog(static_cast<QWidget*>(parent()));
      connect(m_mopacInputDialog, SIGNAL(readOutput(QString)),
        this, SLOT(readOutputFile(QString)));
      m_mopacInputDialog->readSettings(settings);
      if (m_molecule) {
        m_mopacInputDialog->setMolecule(m_molecule);
      }
    }
  }

} // End namespace Avogadro

#include "gaussianextension.moc"

Q_EXPORT_PLUGIN2(gaussianextension, Avogadro::GaussianExtensionFactory)
