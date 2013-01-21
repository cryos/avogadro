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

#include "packmolextension.h"
#include "packmoldialog.h"

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

  PackmolExtension::PackmolExtension(QObject *parent) : Extension(parent),
      m_molecule(0), m_dialog(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Packmol"));
    m_actions.append(action);

    createDialog();
  }

  PackmolExtension::~PackmolExtension()
  {
  }

  QList<QAction *> PackmolExtension::actions() const
  {
    return m_actions;
  }

  QString PackmolExtension::menuPath(QAction *) const
  {
    return tr("&Build");
  }

  void PackmolExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* PackmolExtension::performAction(QAction *action, GLWidget *widget)
  {
    m_dialog->show();
    return 0;
  }

  void PackmolExtension::createDialog()
  {
    if (!m_dialog) {
      m_dialog = new PackmolDialog(qobject_cast<QWidget *>(parent()));
      connect(m_dialog, SIGNAL(resultReady(Molecule*)), this, SLOT(resultsReady(Molecule*)));
    }
  }

  void PackmolExtension::resultsReady(Molecule *molecule)
  {
    emit moleculeChanged(molecule, NewWindow);
  }

  void PackmolExtension::writeSettings(QSettings &settings) const
  {
    m_dialog->writeSettings(settings);
  }

  void PackmolExtension::readSettings(QSettings &settings)
  {
    m_dialog->readSettings(settings);
  }


} // end namespace Avogadro

Q_EXPORT_PLUGIN2(packmolextension, Avogadro::PackmolExtensionFactory)

