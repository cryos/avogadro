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
#include <openbabel/obconversion.h>

#include <QtGui/QAction>
#include <QtGui/QMessageBox>
#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

using namespace OpenBabel;

namespace Avogadro {

  MolecularPropertiesExtension::MolecularPropertiesExtension(QObject *parent) : Extension(parent),
                                                                                m_molecule(0), m_dialog(0),
                                                                                m_inchi(),
                                                                                m_network(0),
                                                                                m_nameRequestPending(false)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Molecule Properties..."));
    m_actions.append(action);
  }

  MolecularPropertiesExtension::~MolecularPropertiesExtension()
  {  }

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
      connect(m_dialog, SIGNAL(accepted()), this, SLOT(disableUpdating()));
      connect(m_dialog, SIGNAL(rejected()), this, SLOT(disableUpdating()));
    }
    if (!m_network) {
      m_network = new QNetworkAccessManager(this);
      connect(m_network, SIGNAL(finished(QNetworkReply*)),
              this, SLOT(replyFinished(QNetworkReply*)));
    }

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

    m_dialog->nameLine->setText(tr("unknown", "Unknown molecule name"));

    update();
    m_dialog->show();

    return 0;
  }

  void MolecularPropertiesExtension::update()
  {
    if (m_dialog == NULL || m_molecule == NULL)
      return;

    // used multiple times below
    OpenBabel::OBMol obmol = m_molecule->OBMol();

    if (!m_nameRequestPending) {
      m_nameRequestPending = true;
      // Wait 250 msec before requesting to limit number of requests
      // (Additional throttling is done by checking the InChI for each request)
      qDebug() << "Requesting IUPAC name...";
      QTimer::singleShot(250, this, SLOT(requestIUPACName()));
    }

    QString format("%L1"); // localized numbers
    m_dialog->molecularWeightLine->setText(format.arg(obmol.GetMolWt(), 0, 'f', 3));

    // Copied from Kalzium
    QString formula(obmol.GetSpacedFormula(1,"").c_str());
    formula.replace( QRegExp( "(\\d+)" ), "<sub>\\1</sub>" );
    m_dialog->formulaLine->setText(formula);
    // we should actually handle charges with superscripts too (e.g., [SO4]-2)

    m_dialog->energyLine->setText(format.arg(m_molecule->energy(), 0, 'f', 3));
    bool estimate = true; // estimated dipole
    m_dialog->dipoleMomentLine->setText(format.arg(m_molecule->dipoleMoment(&estimate).norm(), 0, 'f', 3));
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

  void MolecularPropertiesExtension::disableUpdating()
  {
    // don't ask for more updates
    disconnect( m_molecule, 0, this, 0 );
  }

  void MolecularPropertiesExtension::clearName()
  {
    if (m_dialog)
      m_dialog->nameLine->setText(tr("unknown", "Unknown molecule name"));
    if (m_molecule)
      m_molecule->setProperty("name", QVariant()); // set an invalid name, since we don't have one
  }

  void MolecularPropertiesExtension::replyFinished(QNetworkReply *reply)
  {
    // Read in all the data
    if (!reply->isReadable()) {
      QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                           tr("Network Download Failed"),
                           tr("Network timeout or other error."));
      reply->deleteLater();
      clearName();
      return;
    }

    // check if the data came through
    QByteArray data = reply->readAll();
    if (data.contains("Error report") || data.contains("<h1>")) {
      reply->deleteLater();
      clearName();
      return;
    }

    QString name = QString(data).trimmed().toLower();
    if (!name.isEmpty()) {
      m_dialog->nameLine->setText(name);
      if (m_molecule) {
        m_molecule->setProperty("name", QVariant(name)); // set the name for future use by other code
      }
    } else {
      clearName();
    }

    reply->deleteLater();
  }

  void MolecularPropertiesExtension::requestIUPACName()
  {
    if (m_dialog == NULL || m_molecule == NULL)
      return;

    m_nameRequestPending = false;
    OpenBabel::OBMol obmol = m_molecule->OBMol();

    // Check if the molecule has changed,
    // so we need to ask for a new name from the resolver
    OBConversion conv;
    conv.SetOutFormat("inchi"); // use a standard InChI key (which avoids issues with URL escaping)
    QString inchi = QString::fromStdString(conv.WriteString(&obmol, true)); // skip whitespace
    if (m_inchi == inchi)
      return; // no need to send a new query, since it's the same request

    m_inchi = inchi; // cache for next use

    QString requestURL = QLatin1String("http://cactus.nci.nih.gov/chemical/structure/") + m_inchi + QLatin1String("/iupac_name");
    qDebug() << " requesting URL: " << requestURL;

    m_network->get(QNetworkRequest(QUrl(requestURL)));

    m_dialog->nameLine->setText(tr("(pending)", "asking server for molecule name"));
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(molecularpropextension,
                 Avogadro::MolecularPropertiesExtensionFactory)

