/**********************************************************************
  SpectraExtension - Visualize spectral data from QM calculations

  Copyright (C) 2009 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "spectraextension.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/generic.h>
#include <openbabel/mol.h>

#include <QAction>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  SpectraExtension::SpectraExtension(QObject *parent) : Extension(parent),
                                                            m_dialog(0),
                                                            m_molecule(NULL)
  {
    QAction *action = new QAction( this );
    action->setSeparator(true);
    m_actions.append( action );

    action = new QAction(this);
    action->setText(tr("&Spectra..."));
    m_actions.append(action);
  }

  SpectraExtension::~SpectraExtension()
  {
    //TODO: Anything to delete?
  }

  QList<QAction *> SpectraExtension::actions() const
  {
    return m_actions;
  }

  void SpectraExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
    if (m_dialog) {
      m_dialog->writeSettings();
    }
  }

  void SpectraExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
  }

  QString SpectraExtension::menuPath(QAction *) const
  {
    return tr("E&xtensions");
  }

  void SpectraExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    if (m_dialog)
      m_dialog->setMolecule(molecule);
  }

  QUndoCommand* SpectraExtension::performAction( QAction *, GLWidget *widget )
  {
    Q_UNUSED(widget);
    if (m_molecule == NULL) {
      return NULL;
    }

    if (!m_dialog) {
      m_dialog = new SpectraDialog(qobject_cast<QWidget*>(parent()));
      m_dialog->setMolecule(m_molecule);
    }
    m_dialog->show();
    return NULL;
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(spectraextension, Avogadro::SpectraExtensionFactory)

