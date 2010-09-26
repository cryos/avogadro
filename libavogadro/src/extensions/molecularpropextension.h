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

#ifndef MOLECULARPROPEXTENSION_H
#define MOLECULARPROPEXTENSION_H

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include "ui_molecularpropdialog.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>
#include <QCloseEvent>

// Forward declarations
class QNetworkAccessManager;
class QNetworkReply;

namespace Avogadro {

  class MolecularPropertiesDialog : public QDialog, public Ui::MolecularPropertiesDialog
    {
      public:
      MolecularPropertiesDialog(QWidget *parent=0) : QDialog(parent) {
        setupUi(this);
      }
    };

 class MolecularPropertiesExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("MolecularProp", tr("Molecular Properties"),
                       tr("Display standard molecular properties."))

    public:
      //! Constructor
      MolecularPropertiesExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~MolecularPropertiesExtension();

      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
      virtual QString menuPath(QAction *action) const;
      //@}

      void setMolecule(Molecule *molecule);

    public Q_SLOTS:
      // Slots to take signals from Molecules, and GLWidget
      void update();
      void updatePrimitives(Primitive*);
      void updateAtoms(Atom*);
      void updateBonds(Bond*);
      void moleculeChanged(Molecule *previous);

      void disableUpdating();

    private:
      QList<QAction *> m_actions;
      Molecule *m_molecule;
      GLWidget *m_widget;

      MolecularPropertiesDialog *m_dialog;
      // to query the NIH chemical resolver for an IUPAC name
      QString                m_inchi;
      QNetworkAccessManager *m_network;
      bool m_nameRequestPending;

      void clearName();

      private Q_SLOTS:
      void requestIUPACName();
      void replyFinished(QNetworkReply*);

  };

  class MolecularPropertiesExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(MolecularPropertiesExtension)
  };


} // end namespace Avogadro

#endif
