/**********************************************************************
  FileImportExtension - Extension for generating POV-Ray rendered images

  Copyright (C) 2008 Marcus D. Hanwell

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

#include "fileimportextension.h"

#include <avogadro/molecule.h>
#include <avogadro/bond.h>
#include <avogadro/toolgroup.h>
#include <avogadro/openbabelwrapper.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QTextStream>
#include <QFile>
#include <QDebug>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

using namespace OpenBabel;
using namespace std;

namespace Avogadro
{

  FileImportExtension::FileImportExtension(QObject* parent) : Extension(parent),
    m_glwidget(0), m_molecule(0), m_fileImportDialog(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Trajectory..."));
    m_actions.append(action);
  }

  FileImportExtension::~FileImportExtension()
  {
    if (m_fileImportDialog) {
      delete m_fileImportDialog;
      m_fileImportDialog = 0;
    }
  }

  QList<QAction *> FileImportExtension::actions() const
  {
    return m_actions;
  }

  QString FileImportExtension::menuPath(QAction*) const
  {
    return tr("&File") + ">" + tr("Import");
  }

  QUndoCommand* FileImportExtension::performAction(QAction *, GLWidget *widget)
  {
    m_glwidget = widget;
    if (!m_fileImportDialog) {
      m_fileImportDialog = new FileImportDialog();
      connect(m_fileImportDialog->selectFileName, SIGNAL(clicked()),
              this, SLOT(selectFileName()));
      connect(m_fileImportDialog->selectFileName2, SIGNAL(clicked()),
              this, SLOT(selectFileName2()));
      connect(m_fileImportDialog->buttonBox, SIGNAL(accepted()),
              this, SLOT(import()));
      connect(m_fileImportDialog->buttonBox, SIGNAL(rejected()),
              m_fileImportDialog, SLOT(hide()));
      m_fileImportDialog->show();
    }
    else {
      m_fileImportDialog->show();
    }
    return 0;
  }

  void FileImportExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
  }

  void FileImportExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
  }

  void FileImportExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  void FileImportExtension::readParmFile(Molecule *mol)
  {
    // Now the task of reading the parm file. Right now just reading in bonds
    QFile file(m_fileImportDialog->fileName2->text());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
      qDebug() << "Unable to read file" << m_fileImportDialog->fileName2->text();
      return;
    }
    QTextStream in(&file);

    while (!in.atEnd()) {
      processLine(&in, mol);
    }
  }

  void FileImportExtension::processLine(QTextStream *in, Molecule *mol)
  {
    // First truncate the line, remove trailing white space and check
    QString line = in->readLine();
    QString key = line;
    key = key.trimmed();

    if (key == "%FLAG BONDS_WITHOUT_HYDROGEN") {
      qDebug() << "Reading in bonds...";
      line = in->readLine(); // Throw away this line
      line = "";

      QStringList list;
      while (line[0] != '%') {
        line = in->readLine();
        list += line.split(" ", QString::SkipEmptyParts);
        if (list.size() == 30) {
          for (int i = 0; i <= 27; i += 3) {
            Bond *bond = mol->addBond();
            bond->setAtoms(list.at(i).toInt()/3, list.at(i+1).toInt()/3);
            //bond->setOrder(list.at(i+2).toInt());
          }
          list.clear();
        }
      }
    }

  }

  void FileImportExtension::import()
  {
    QString fileName = m_fileImportDialog->fileName->text();
    QApplication::setOverrideCursor(Qt::WaitCursor);

/*    OBConversion conv;
    OBFormat     *inFormat = conv.FormatFromExt(( fileName.toAscii() ).data() );
    if ( !inFormat || !conv.SetInFormat( inFormat ) ) {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(m_fileImportDialog, tr("Avogadro"),
          tr("Cannot read file format of file %1.").arg(fileName));
      return;
    }

    ifstream ifs;
    ifs.open((fileName.toAscii()).data());
    if (!ifs) { // shouldn't happen, already checked file above
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(m_fileImportDialog, tr("Avogadro"),
          tr("Cannot read file %1.").arg( fileName ) );
      return;
    }

    OBMol *obmol = new OBMol; */
    // Turn off bond perception
    Molecule *mol = OpenbabelWrapper::openFile(fileName, "", "b");
    if (mol) {
      Molecule *oldMol = m_molecule;
      qDebug() << "Attempting to read parm file...";
      readParmFile(mol);
      qDebug() << "Parm file read...";
      emit moleculeChanged(mol);
      oldMol->deleteLater();
    }
    else
      qDebug() << "Reading molecule file failed...";

    QApplication::restoreOverrideCursor();
  }

  void FileImportExtension::selectFileName()
  {
    QString fileName = QFileDialog::getOpenFileName(m_fileImportDialog,
      tr("Open chemical file format"), m_fileImportDialog->fileName->text(),
      tr("Chemical files (*.xyz)"));
    m_fileImportDialog->fileName->setText(fileName);
  }

  void FileImportExtension::selectFileName2()
  {
    QString fileName = QFileDialog::getOpenFileName(m_fileImportDialog,
      tr("Open parameter file"), m_fileImportDialog->fileName->text(),
      tr("Chemical files (*.parm7)"));
    m_fileImportDialog->fileName2->setText(fileName);
  }

} // End namespace Avogadro

Q_EXPORT_PLUGIN2(fileimportextension, Avogadro::FileImportExtensionFactory)

