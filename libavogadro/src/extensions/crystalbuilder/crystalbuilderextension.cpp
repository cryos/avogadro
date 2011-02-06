/**********************************************************************
  crystalbuilderextension.cpp - Crystal Builder Plugin for Avogadro

  Copyright (C) 2011 by David C. Lonie

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

#include "crystalbuilderextension.h"

namespace Avogadro
{
  CrystalBuilderExtension::CrystalBuilderExtension( QObject *parent ) :
    Extension( parent ),
    m_dialog(0),
    m_molecule(0)
  {
    QAction *action;

    action = new QAction(this);
    action->setText(tr("&Crystal Builder"));
    action->setShortcut( tr("Ctrl+B",
			    "Shortcut key for Crystal Builder") );
    m_actions.append( action );
  }

  CrystalBuilderExtension::~CrystalBuilderExtension()
  {
  }

  QList<QAction *> CrystalBuilderExtension::actions() const
  {
    return m_actions;
  }

  QString CrystalBuilderExtension::menuPath(QAction */*action*/) const
  {
    return tr("&Build");
  }

  void CrystalBuilderExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    if (m_dialog) {
      m_dialog->setMolecule(m_molecule);
    }
  }

  void CrystalBuilderExtension::writeSettings()
  {
    if (m_dialog) {
      m_dialog->writeSettings();
    }
  }

  void CrystalBuilderExtension::readSettings()
  {
    if (m_dialog) {
      m_dialog->readSettings();
    }
  }

  QUndoCommand* CrystalBuilderExtension::performAction( QAction */*action*/, GLWidget */*widget*/)
  {

    QUndoCommand *undo = NULL;

    if (!m_dialog) {
      m_dialog = new CrystalBuilderDialog(static_cast<QWidget*>(parent()));
    }

    m_dialog->setMolecule(m_molecule);
    m_dialog->readSettings();
    m_dialog->show();

    return undo;
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(crystalbuilderextension, Avogadro::CrystalBuilderExtensionFactory)

