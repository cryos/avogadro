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
#include "sortfiltertreeproxymodel.h"

// Defines INSTALL_PREFIX among other things
#include "config.h" // krazy:exclude=includes

#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/moleculefile.h>

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#include <openbabel/builder.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QCloseEvent>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QFileSystemModel>

using namespace OpenBabel;
namespace Avogadro {

  class InsertFragmentPrivate
  {
  public:
    Molecule     fragment;
    OBConversion conv;
    OBBuilder    builder;
    SortFilterTreeProxyModel *proxy;
    QFileSystemModel *model;
    QModelIndex  proxyRoot;

    QString      currentFileName;
    bool         crystalFiles; // are we inserting crystals (i.e., don't center)

    ~InsertFragmentPrivate()
    {
      if (model)
        delete model;
    }

  };

  InsertFragmentDialog::InsertFragmentDialog(QWidget *parent, QString directory, Qt::WindowFlags) : QDialog(parent)
  {
    // Use a small title bar (Qt::Tool) with no minimize or maximize buttons
    // much like the Periodic Table widget
    setWindowFlags(Qt::Dialog | Qt::Tool);
    ui.setupUi(this);

    d = new InsertFragmentPrivate;

    d->currentFileName.clear();

    //@todo: it would be great to allow multiple directories, but that needs our own directory model
    QString m_directory;
#ifdef Q_WS_X11
    m_directory = QString( INSTALL_PREFIX ) + "/share/avogadro/";
#else
    // Mac and Windows use relative path from application location
    m_directory = QCoreApplication::applicationDirPath() + "/../share/avogadro/";
#endif
    m_directory += directory; // fragments or crystals
    if ( directory.contains(QLatin1String("crystals")) )
      d->crystalFiles = true;
    else
      d->crystalFiles = false;

    d->model = new QFileSystemModel(this);
    d->model->setReadOnly(true);
    QModelIndex rootIndex = d->model->setRootPath(m_directory);

    d->proxy = new SortFilterTreeProxyModel(this);
    d->proxy->setSourceModel(d->model);
    d->proxy->setSortLocaleAware(true); // important for files
    d->proxyRoot = d->proxy->mapFromSource(rootIndex); // map from the root path to the proxy index
    d->proxy->setSourceRoot(rootIndex); // FOR our custom filter class, to prevent us from becoming rootless!

    ui.directoryTreeView->setModel(d->proxy);
    ui.directoryTreeView->setRootIndex(d->proxyRoot); // remember to map from the source to the proxy index
    for (int i = 1; i < d->model->columnCount(); ++i)
      ui.directoryTreeView->hideColumn(i);

    ui.directoryTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.directoryTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.directoryTreeView->setUniformRowHeights(true);
    ui.directoryTreeView->expandToDepth(1);

    connect(ui.insertFragmentButton, SIGNAL(clicked(bool)),
            this, SLOT(activated()));

    connect(ui.directoryTreeView, SIGNAL(doubleClicked(const QModelIndex)),
            this, SLOT(activated()));

    connect(ui.directoryTreeView, SIGNAL(activated(const QModelIndex)),
            this, SLOT(activated()));

    connect(ui.filterLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(filterTextChanged(const QString &)));

    connect(ui.clearToolButton, SIGNAL(clicked(bool)),
            this, SLOT(clearFilterText()));
  }

  InsertFragmentDialog::~InsertFragmentDialog()
  {
    delete d;
  }

  const Molecule &InsertFragmentDialog::fragment()
  {
    OBMol obfragment;

    // The selected model index is in the proxy
    QModelIndexList selected = ui.directoryTreeView->selectionModel()->selectedIndexes();
    // So remember to map to the source model
    QString fileName = d->model->filePath(d->proxy->mapToSource(selected.first()));

    if (fileName.isEmpty())
      return d->fragment; // shouldn't happen -- return existing fragment

    if (fileName == d->currentFileName)
      return d->fragment; // don't re-read the file

    d->fragment.clear();
    // Check if it's a directory
    QFileInfo fileInfo(fileName);
    if (fileInfo.isDir())
      return d->fragment; // return an empty fragment and do nothing

    Molecule *mol = MoleculeFile::readMolecule(fileName);
    if (mol) {
      d->fragment = *mol;
    } else {
      QMessageBox::warning( (QWidget*)this, tr( "Avogadro" ),
                            tr( "Cannot read molecular file %1." )
                            .arg( fileName ) );
      // we'll return an empty fragment
    }

    if (!d->crystalFiles)
      d->fragment.center();

    return d->fragment;
  }

  void InsertFragmentDialog::refresh()
  {
    ui.directoryTreeView->update();
  }

  void InsertFragmentDialog::clearFilterText()
  {
    ui.filterLineEdit->setText("");
  }

  void InsertFragmentDialog::filterTextChanged(const QString &newFilter)
  {
    if (!d || !d->proxy)
      return; // no dialog or proxy model to set

    // Allow things like "ti" to match "Ti" etc.
    QRegExp reg(newFilter, Qt::CaseInsensitive, QRegExp::WildcardUnix);
    d->proxy->setFilterRegExp(reg);
  }

  void InsertFragmentDialog::activated()
  {
    emit performInsert();
  }

}
