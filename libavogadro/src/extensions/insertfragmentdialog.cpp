/**********************************************************************
 InsertFragmentDialog - Inserting fragments using the draw tool

 Copyright (C) 2008 by Geoffrey Hutchison

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

#include "insertfragmentdialog.h"
#include "directorytreemodel.h"
// Defines INSTALL_PREFIX among other things
#include "config.h" // krazy:exclude=includes

#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#include <openbabel/builder.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QCloseEvent>
#include <QDebug>

using namespace OpenBabel;
namespace Avogadro {

  class InsertFragmentPrivate
  {
  public:
    Molecule     fragment;
    OBConversion conv;
    OBBuilder    builder;
    DirectoryTreeModel *model;
    
    QString      currentFileName;

    ~InsertFragmentPrivate()
    {
      if (model)
        delete model;
    }

  };

  QStringList DefaultDirectoryList()
  {
    QStringList directoryList;
#ifdef Q_WS_X11
    directoryList << QString( INSTALL_PREFIX ) + "/share/avogadro/fragments";
    directoryList << QDir::homePath() + "/.avogadro/fragments";
#endif
#ifdef Q_WS_WIN
    directoryList << QCoreApplication::applicationDirPath() + "/../share/avogadro/fragments";
#endif
#ifdef Q_WS_MAC
    directoryList << QCoreApplication::applicationDirPath() + "/../share/avogadro/fragments";
    directoryList << "/Library/Application Support/Avogadro/Fragments";
    directoryList << QDir::homePath() + "/Library/Application Support/Avogadro/Fragments";
#endif

    return directoryList;
  }

  InsertFragmentDialog::InsertFragmentDialog(QWidget *parent, Qt::WindowFlags) : QDialog(parent)
  {
    // Use a small title bar (Qt::Tool) with no minimize or maximize buttons
    // much like the Periodic Table widget
    setWindowFlags(Qt::Dialog | Qt::Tool);

    d = new InsertFragmentPrivate;

    d->currentFileName.clear();

    // There has to be a better way to set this based on the installation prefix
    m_directoryList = DefaultDirectoryList();
    d->model = new DirectoryTreeModel(m_directoryList, this);

    ui.setupUi(this);
    ui.directoryTreeView->setModel(d->model);
    ui.directoryTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.directoryTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.directoryTreeView->setUniformRowHeights(true);
    ui.directoryTreeView->expandToDepth(1);

    connect(ui.insertFragmentButton, SIGNAL(clicked(bool)),
            this, SLOT(insertButtonClicked(bool)));
    connect(ui.addDirectoryButton, SIGNAL(clicked(bool)),
            this, SLOT(addDirectory(bool)));
    connect(ui.clearListButton, SIGNAL(clicked(bool)),
            this, SLOT(clearDirectoryList(bool)));
  }

  InsertFragmentDialog::~InsertFragmentDialog()
  {
    delete d;
  }

  const Molecule &InsertFragmentDialog::fragment()
  {
    OBMol obfragment;

    QModelIndexList selected = ui.directoryTreeView->selectionModel()->selectedIndexes();
    if (selected.count() == 1) {
      QString fileName = d->model->filePath(selected.first());

      if (!fileName.isEmpty()) {
        if (fileName == d->currentFileName)
          return d->fragment; // don't re-read the file

        d->fragment.clear();

        // TODO: Needs porting to MolecularFile
        OBConversion conv;
        OBFormat *inFormat = conv.FormatFromExt(fileName.toAscii());
        if (!inFormat || !conv.SetInFormat(inFormat)) {
          QMessageBox::warning( (QWidget*)this, tr( "Avogadro" ),
                                tr( "Cannot read file format of file %1." )
                                .arg( fileName ) );
          return d->fragment;
        }
        std::ifstream ifs;
        ifs.open(QFile::encodeName(fileName));
        if (!ifs) {
          QMessageBox::warning( (QWidget*)this, tr( "Avogadro" ),
                                tr( "Cannot read file %1." )
                                .arg( fileName ) );
          return d->fragment;
        }

        conv.Read(&obfragment, &ifs);
        d->fragment.setOBMol(&obfragment);
        d->fragment.center();
        ifs.close();
      }
    }

    return d->fragment;
  }

  const QStringList InsertFragmentDialog::directoryList() const
  {
    return m_directoryList;
  }

  void InsertFragmentDialog::setDirectoryList(const QStringList dirList)
  {
    if (dirList.size() != 0)
      m_directoryList = dirList;
    refresh();
  }

  void InsertFragmentDialog::refresh()
  {
    d->model->setDirectoryList(m_directoryList);
    ui.directoryTreeView->update();
  }

  void InsertFragmentDialog::addDirectory(bool)
  {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home");

    // If this is a new directory, add it in
    if (!m_directoryList.contains(dir)) {
      m_directoryList << dir;
      refresh();
    }
  }

  void InsertFragmentDialog::clearDirectoryList(bool)
  {
    m_directoryList.clear();
    m_directoryList = DefaultDirectoryList();
    refresh();
  }

  void InsertFragmentDialog::insertButtonClicked(bool)
  {
    emit insertClicked();
  }
  
}
