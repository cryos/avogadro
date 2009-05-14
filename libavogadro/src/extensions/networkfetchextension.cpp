/**********************************************************************
  NetworkFetchExtension - Extension for fetching molecules over the network

  Copyright (C) 2009 Marcus D. Hanwell

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

#include "networkfetchextension.h"

#include <avogadro/molecule.h>

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

namespace Avogadro
{
  using OpenBabel::OBConversion;
  using OpenBabel::OBMol;

  NetworkFetchExtension::NetworkFetchExtension(QObject* parent)
    : Extension(parent),
    m_glwidget(0), m_molecule(0), m_network(0), m_moleculeName(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Fetch from PDB..."));
    action->setData("PDB");
    m_actions.append(action);
    action = new QAction(this);
    action->setText(tr("Fetch from URL..."));
    action->setData("URL");
    m_actions.append(action);
  }

  NetworkFetchExtension::~NetworkFetchExtension()
  {
    delete m_moleculeName;
  }

  QList<QAction *> NetworkFetchExtension::actions() const
  {
    return m_actions;
  }

  QString NetworkFetchExtension::menuPath(QAction*) const
  {
    return tr("&File") + '>' + tr("Import");
  }

  QUndoCommand* NetworkFetchExtension::performAction(QAction *action,
                                                     GLWidget *widget)
  {
    m_glwidget = widget;
    if (!m_moleculeName)
      m_moleculeName = new QString;
    if (!m_network) {
      m_network = new QNetworkAccessManager(this);
      connect(m_network, SIGNAL(finished(QNetworkReply*)),
              this, SLOT(replyFinished(QNetworkReply*)));
    }
    if (action->data() == "PDB") {
      // Prompt for a PDB name
      bool ok;
      QString pdbName = QInputDialog::getText(qobject_cast<QWidget*>(parent()),
                                              tr("PDB Entry"),
                                              tr("PDB entry to download."),
                                              QLineEdit::Normal,
                                              "", &ok);
      if (!ok || pdbName.isEmpty())
        return 0;
      // Hard coding the PDB download URL - this could be used for other services
      m_network->get(QNetworkRequest(QUrl("http://www.pdb.org/pdb/download/downloadFile.do?fileFormat=pdb&compression=NO&structureId=" + pdbName)));

      *m_moleculeName = pdbName + ".pdb";
    }
    else if (action->data() == "URL") {
      // Prompt for a URL
      bool ok;
      QString url = QInputDialog::getText(qobject_cast<QWidget*>(parent()),
                                          tr("URL"),
                                          tr("URL of molecule to download."),
                                          QLineEdit::Normal,
                                          "", &ok);
      if (!ok || url.isEmpty())
        return 0;
      // Hard coding the PDB download URL - this could be used for other services
      m_network->get(QNetworkRequest(QUrl(url)));

      *m_moleculeName = url;
    }

    return 0;
  }

  void NetworkFetchExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);

  }

  void NetworkFetchExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
  }

  void NetworkFetchExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  void NetworkFetchExtension::replyFinished(QNetworkReply *reply)
  {
    // Read in all the data
    if (!reply->isReadable()) {
      QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                           tr("Network Download Failed"),
                           tr("Network timeout or other error."));
      delete reply;
      return;
    }

    QByteArray data = reply->readAll();

    // Check if the PDB was successfully downloaded
    if (data.contains("Error report")) {
      QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                           tr("Network Download Failed"),
                           tr("Specified molecule could not be found: ") + *m_moleculeName);
      delete reply;
      return;
    }

    // Now read it in with OpenBabel - we should add a wrapper class to automate
    OBConversion conv;
    conv.SetInFormat("pdb");
    OBMol *obmol = new OBMol;
    if (conv.ReadString(obmol, QString(data).toStdString())) {
      Molecule *mol = new Molecule;
      mol->setOBMol(obmol);
      mol->setFileName(*m_moleculeName);
      emit moleculeChanged(mol);
      m_molecule = mol;
    }
    else {
      QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                           tr("Network Download Failed"),
                           tr("Specified molecule could not be loaded: ") + *m_moleculeName);
    }
    // We are responsible for deleting the reply object
    delete reply;
  }

} // End namespace Avogadro

Q_EXPORT_PLUGIN2(networkfetchextension, Avogadro::NetworkFetchExtensionFactory)

