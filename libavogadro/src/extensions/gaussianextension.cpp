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

namespace Avogadro
{

  GaussianExtension::GaussianExtension(QObject* parent) : Extension(parent),
    m_gaussianInputDialog(0), m_qchemInputDialog(0), m_molecule(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Gaussian Input..."));
    action->setData("Gaussian");
    m_actions.append(action);
    action = new QAction(this);
    action->setText(tr("Q-Chem Input..."));
    action->setData("QChem");
    m_actions.append(action);
  }

  GaussianExtension::~GaussianExtension()
  {
    delete m_gaussianInputDialog;
    m_gaussianInputDialog = 0;
    delete m_qchemInputDialog;
    m_qchemInputDialog = 0;
  }

  QList<QAction *> GaussianExtension::actions() const
  {
    return m_actions;
  }

  QString GaussianExtension::menuPath(QAction*) const
  {
    return tr("&Extensions");
  }

  QUndoCommand* GaussianExtension::performAction(QAction *action, GLWidget *)
  {
    if (action->data() == "Gaussian") {
      if (!m_gaussianInputDialog) {
        m_gaussianInputDialog = new GaussianInputDialog();
        m_gaussianInputDialog->setMolecule(m_molecule);
        m_gaussianInputDialog->show();
      }
      else
        m_gaussianInputDialog->show();
    }
    else if (action->data() == "QChem") {
      if (!m_qchemInputDialog) {
        m_qchemInputDialog = new QChemInputDialog();
        m_qchemInputDialog->setMolecule(m_molecule);
        m_qchemInputDialog->show();
      }
      else
        m_qchemInputDialog->show();
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
  }

  void GaussianExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
    if (m_gaussianInputDialog) {
      m_gaussianInputDialog->writeSettings(settings);
    }
  }

  void GaussianExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
    if (m_gaussianInputDialog) {
      m_gaussianInputDialog->readSettings(settings);
    }
    else {
      m_gaussianInputDialog = new GaussianInputDialog();
      m_gaussianInputDialog->readSettings(settings);
      if (m_molecule) {
        m_gaussianInputDialog->setMolecule(m_molecule);
      }
    }
  }

} // End namespace Avogadro

#include "gaussianextension.moc"

Q_EXPORT_PLUGIN2(gaussianextension, Avogadro::GaussianExtensionFactory)
