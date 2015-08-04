/**********************************************************************
  OrcaExtension

  Copyright (C) 2010 David C. Lonie
  Some portions Copyright (C) 2014 Dagmar Lenk

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

#include "orcaextension.h"
#include "orcainputdialog.h"
#include "orcaanalysedialog.h"


#include <QtGui/QAction>
#include <QMessageBox>
#include <QDebug>
#include <avogadro/periodictableview.h>

#include <openbabel/babelconfig.h>
#include <openbabel/obconversion.h>
#include <openbabel/format.h>

using namespace Avogadro;
using namespace OpenBabel;

namespace Avogadro {

    enum OrcaDialogIndex { GENERATE, ANALYSE, VIBRATION, PLOT};

  OrcaExtension::OrcaExtension(QObject *parent)
    : Extension(parent),
      m_dialog(NULL), m_analyseDialog(NULL) // Initialize the dialog pointer to 0, it will be
                  // created only when requested
  {

      // Set up list of codes and descriptions
      OBConversion conv;
      OBFormat *pFormat;
      Formatpos pos;
      const char* str=NULL;

      QString qtest=NULL;
      m_NOAnalyse = false;
 //
 // Look if the installed OpenBabel version supports orca file format
 //
      while(OBConversion::GetNextFormat(pos,str,pFormat))
      {
          qtest.append(pFormat->GetID());
          if (qtest.contains("orca")) {
              m_NOAnalyse = true;
              break;
          }

      }


    // This block sets the text for menu entry
      QAction *action = new QAction(this);
      // Wrap all user visible strings in tr() so they can be translated
      action->setText(tr("Generate Orca Input..."));
      m_actions.append(action);
      action->setData(GENERATE);

      if (!m_NOAnalyse){            // this dialog box ist only needed  if openbabel doesn't supports orca file format
          action = new QAction(this);
          action->setText(tr("Analyse Orca Output..."));
          m_actions.append(action);
          action->setData(ANALYSE);
      }
  }

  QList<QAction *> OrcaExtension::actions() const
  {
    return m_actions;
  }

  QString OrcaExtension::menuPath(QAction *) const
  {

    return tr("E&xtensions") + '>' + tr("&Orca");
  }

  QUndoCommand* OrcaExtension::performAction( QAction *action, GLWidget *widget )
  {

      int i = action->data().toInt();

      switch (i) {
      case GENERATE:
          // Create the dialog if needed
          if (!m_dialog) {
              m_dialog = new OrcaInputDialog(qobject_cast<QWidget*>(parent()));
          }
          if (m_molecule) {
              m_dialog->setMolecule(m_molecule);
          }
          m_dialog->setWindowTitle("Orca Input Parameters");
          m_dialog->show();

          break;
      case ANALYSE:
          // Create the dialog if needed
          if (!m_analyseDialog) {
              m_analyseDialog = new OrcaAnalyseDialog(qobject_cast<QWidget*>(parent()));
              connect (m_analyseDialog, SIGNAL(moleculeChangedHere(Molecule* , int)), this, SIGNAL(moleculeChanged(Molecule*,int)));
          }
          if (m_molecule) {
              m_analyseDialog->setMolecule(m_molecule);
          }
          m_widget = widget;
          if (m_widget){
              m_analyseDialog->setWidget(m_widget);
          }
          m_analyseDialog->setWindowTitle("Analyse Orca Output");
          m_analyseDialog->show();
      }

      return NULL;
  }

    void OrcaExtension::setMolecule(Molecule *molecule)
    {
        m_molecule = molecule;
    }
}
//// Include Qt moc'd headers
//#include "orcaextension.moc"

// Set up for the plugin to work correctly
Q_EXPORT_PLUGIN2(orcaextension, Avogadro::OrcaExtensionFactory)
