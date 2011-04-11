/**********************************************************************
  InputFileExtension - Extension for generating input files

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

#include "inputfileextension.h"

#include "daltoninputdialog.h"
#include "gamessukinputdialog.h"
#include "gaussianinputdialog.h"
#include "molproinputdialog.h"
#include "mopacinputdialog.h"
#include "nwcheminputdialog.h"
#include "qcheminputdialog.h"

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <avogadro/molecule.h>

#include <QMessageBox>
#include <QFile>
#include <QDebug>

using namespace OpenBabel;
using namespace std;

namespace Avogadro
{

  InputFileExtension::InputFileExtension(QObject* parent) : Extension(parent),
    m_molecule(0)
  {
    QAction* action;

    action = new QAction(this);
    action->setText(tr("&Dalton..."));
    action->setData("Dalton");
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("&GAMESS-UK..."));
    action->setData("GAMESS-UK");
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("&Gaussian..."));
    action->setData("Gaussian");
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("&MOLPRO..."));
    action->setData("Molpro");
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("M&OPAC..."));
    action->setData("MOPAC");
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("&NWChem..."));
    action->setData("NWChem");
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("&Q-Chem..."));
    action->setData("QChem");
    m_actions.append(action);

    action = new QAction(this);
    action->setSeparator(true);
    m_actions.append(action);

    m_hasDialog["Dalton"] = false;
    m_hasDialog["GAMESS-UK"] = false;
    m_hasDialog["Gaussian"] = false;
    m_hasDialog["Molpro"] = false;
    m_hasDialog["MOPAC"] = false;
    m_hasDialog["NWChem"] = false;
    m_hasDialog["QChem"] = false;

    //connect(m_dialog["MOPAC"], SIGNAL(readOutput(QString)),
      //  this, SLOT(readOutputFile(QString)));
  }

  InputFileExtension::~InputFileExtension()
  {
  }

  QList<QAction *> InputFileExtension::actions() const
  {
    return m_actions;
  }

  QUndoCommand* InputFileExtension::performAction(QAction *action, GLWidget *widget)
  {
    QSettings settings;
    m_widget = widget;
    QString data(action->data().toString());
    InputDialog *dialog = m_dialog[data];

    // If dialog was not called earlier, create it now
    if (!m_hasDialog[data]) {
      dialog = createInputDialog(data);
      if (!dialog) {
        // We don't know anything about this menu action.
        // If you're adding new generator, modify createInputDialog function
        qDebug() << "No dialog for " + data + "! Something went wrong!";
        return 0;
      }
      if(m_molecule)
       dialog->setMolecule(m_molecule);
      m_dialog[data] = dialog;
      m_hasDialog[data] = true;
      connect(m_dialog[data], SIGNAL(readOutput(QString)),
          this, SLOT(readOutputFile(QString)));
    }
    dialog->show();
    return 0;
  }

  void InputFileExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    foreach (InputDialog *dialog, m_dialog) {
      if (dialog)
        dialog->setMolecule(m_molecule);
    }
  }

  void InputFileExtension::readOutputFile(const QString filename)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    OBConversion conv;
    OBFormat     *inFormat = conv.FormatFromExt( filename.toAscii() );
    if ( !inFormat || !conv.SetInFormat( inFormat ) ) {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(m_widget, tr("Avogadro"),
        tr("Cannot read file format of file %1.").arg(filename));
      return;
    }

    // TODO: Switch to MoleculeFile
    ifstream ifs;
#ifdef Q_CC_MSVC
    ifs.open(filename.toStdWString().c_str());
#else
    ifs.open(QFile::encodeName(filename));
#endif
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
      emit moleculeChanged(mol, Extension::DeleteOld);
      m_molecule = mol;
    }

    QApplication::restoreOverrideCursor();
  }

  void InputFileExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
  }

  void InputFileExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
  }

  InputDialog* InputFileExtension::createInputDialog(QString name)
  {
    // Returns 0 if dialog name is invalid
    if (name == "Dalton")
      return new DaltonInputDialog(static_cast<QWidget*>(parent()));;
    if (name == "GAMESS-UK")
      return new GAMESSUKInputDialog(static_cast<QWidget*>(parent()));;
    if (name == "Gaussian")
      return new GaussianInputDialog(static_cast<QWidget*>(parent()));;
    if (name == "Molpro")
      return new MolproInputDialog(static_cast<QWidget*>(parent()));
    if (name == "MOPAC")
      return new MOPACInputDialog(static_cast<QWidget*>(parent()));
    if (name == "NWChem")
      return new NWChemInputDialog(static_cast<QWidget*>(parent()));
    if (name == "QChem")
      return new QChemInputDialog(static_cast<QWidget*>(parent()));
    return 0;
  }

} // End namespace Avogadro

Q_EXPORT_PLUGIN2(inputfileextension, Avogadro::InputFileExtensionFactory)

