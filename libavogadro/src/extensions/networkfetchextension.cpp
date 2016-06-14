/**********************************************************************
  NetworkFetchExtension - Extension for fetching molecules over the network

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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
#include <avogadro/glwidget.h>
#include <avogadro/toolgroup.h>

#include <QtGui/QAction>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslSocket>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/format.h>
//#include <openbabel/builder.h>
//#include <openbabel/forcefield.h>

namespace Avogadro
{
  using OpenBabel::OBConversion;
  using OpenBabel::OBMol;
  using OpenBabel::OBFormat;
  /*using OpenBabel::OBBuilder;
  using OpenBabel::OBForceField;*/

  NetworkFetchExtension::NetworkFetchExtension(QObject* parent)
    : Extension(parent),
      m_glwidget(0), m_molecule(0), m_network(0), m_moleculeName(0), m_redirects(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Fetch from PDB..."));
    action->setData("PDB");
    m_actions.append(action);
    action = new QAction(this);
    action->setText(tr("Fetch by chemical name..."));
    action->setData("NIH");
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
      connect(m_network, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
              this, SLOT(printSslErrors(QNetworkReply*, const QList<QSslError>&)));
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
      m_network->get(QNetworkRequest(QUrl("http://www.rcsb.org/pdb/files/" + pdbName + ".pdb")));

      *m_moleculeName = pdbName + ".pdb";
    }
    else if (action->data() == "NIH") {
      // Prompt for a chemical structure name
      bool ok;
      QString structureName = QInputDialog::getText(qobject_cast<QWidget*>(parent()),
                                                    tr("Chemical Name"),
                                                    tr("Chemical structure to download."),
                                                    QLineEdit::Normal,
                                                    "", &ok);
      if (!ok || structureName.isEmpty())
        return 0;
      // Hard coding the NIH resolver download URL - this could be used for other services
      m_network->get(QNetworkRequest(
          QUrl("https://cactus.nci.nih.gov/chemical/structure/" + structureName + "/sdf?get3d=true"
               + "&resolver=name_by_opsin,name_by_cir,name_by_chemspider"
               + "&requester=Avogadro")));

      *m_moleculeName = structureName + ".sdf";
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
      // Arbitrary URL
      m_redirects = 0;
      m_urlRequest = QUrl(url);
      m_network->get(QNetworkRequest( m_urlRequest ));

      *m_moleculeName = url;
    }

    widget->toolGroup()->setActiveTool("Navigate");

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

  QUrl NetworkFetchExtension::checkRedirect(const QUrl& possibleRedirectUrl, const QUrl& oldUrl)
  {
    // From Trolltech/Nokia:
    // http://developer.nokia.com/community/wiki/Handling_an_HTTP_redirect_with_QNetworkAccessManager
    QUrl redirectUrl;
    /*
     * Check if the URL is empty and that we aren't being fooled into a infinite redirect loop.
     */
    if(!possibleRedirectUrl.isEmpty() &&
       possibleRedirectUrl != oldUrl &&
       m_redirects < 10 // More than enough
       ) {
      redirectUrl = possibleRedirectUrl;
      m_redirects++;
    }
    return redirectUrl;
  }

  void NetworkFetchExtension::printSslErrors(QNetworkReply*,
                                             const QList<QSslError> &errors)
  {
    foreach(const QSslError &error, errors) {
      qDebug() << tr("SSL Error: %1").arg(error.errorString());
    }
  }

  void NetworkFetchExtension::replyFinished(QNetworkReply *reply)
  {
    // Print error messages
    if (reply->error() != QNetworkReply::NoError) {
      qDebug() << tr("Network Error: %1").arg(reply->errorString());
      return;
    }

    // Read in all the data
    if (!reply->isReadable()) {
      QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                           tr("Network Download Failed"),
                           tr("Network timeout or other error."));
      reply->deleteLater();
      return;
    }

    // Check for a redirect
    QVariant possibleRedirectUrl =
      reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    // Is this a valid HTTP redirect?
    m_urlRequest = this->checkRedirect(possibleRedirectUrl.toUrl(),
                                       m_urlRequest);

    // If we have a non-empty request, we need to try again
    if(!m_urlRequest.isEmpty()) {
      /* We'll do another request to the redirection url. */
      m_network->get(QNetworkRequest( m_urlRequest ));
      qDebug() << " handling redirect " << m_urlRequest;
      reply->deleteLater();
      return; // don't try to fetch, because it's not real data
    }

    // OK, we have our real data
    QByteArray data = reply->readAll();

    // Check if the file was successfully downloaded
    if (data.contains("Error report")) {
      QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                           tr("Network Download Failed"),
                           tr("Specified molecule could not be found: %1").arg(*m_moleculeName));
      reply->deleteLater();
      return;
    }

    QString strData(data);
    QFileInfo info(m_urlRequest.path());
    QString contentType(reply->header(QNetworkRequest::ContentTypeHeader).toString());
    OBConversion conv;

    qDebug() << " parsing " << strData.length() << " format "<< info.suffix() << " mime: " << contentType;

    if ( info.suffix().isEmpty() || !conv.SetInFormat(info.suffix().toAscii()) ) {
      // we might not get an extension, so
      // try to guess from the content type
      OBFormat *format = OBConversion::FormatFromMIME(contentType.toAscii());
      if (!format || !conv.SetInFormat(format)) {
        // last try, use the m_moleculeName
        info.setFile(*m_moleculeName);
        if ( info.suffix().isEmpty() || !conv.SetInFormat(info.suffix().toAscii()) ) {
          // nothing is working!
          return;
        }
      }
    }

    // Now read it in with OpenBabel - we should add a wrapper class to automate
    OBMol *obmol = new OBMol;
    if (conv.ReadString(obmol, QString(data).toStdString())) {
      Molecule *mol = new Molecule;
      mol->setOBMol(obmol);
      mol->setFileName(*m_moleculeName);
      emit moleculeChanged(mol, Extension::DeleteOld | Extension::NewWindow);
      m_molecule = mol;
    }
    else {
      QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                           tr("Network Download Failed"),
                           tr("Specified molecule could not be loaded: %1").arg(*m_moleculeName));
    }
    // We are responsible for deleting the reply object
    reply->deleteLater();
  }

} // End namespace Avogadro

Q_EXPORT_PLUGIN2(networkfetchextension, Avogadro::NetworkFetchExtensionFactory)
