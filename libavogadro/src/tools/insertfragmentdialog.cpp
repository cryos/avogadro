/**********************************************************************
 InsertFragmentDialog - Inserting fragments using the draw tool

 Copyright (C) 2008 by Geoffrey Hutchison

 This file is part of the Avogadro molecular editor project.
 For more information, see <http://avogadro.sourceforge.net/>

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

#include <avogadro/primitive.h>

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

    bool         insertMode;
		bool         smilesMode;

    InsertFragmentPrivate() : insertMode(false)
    { }

    ~InsertFragmentPrivate()
    {
      if (model)
        delete model;
    }

  };

  InsertFragmentDialog::InsertFragmentDialog(QWidget *parent, Qt::WindowFlags) : QDialog(parent)
  {
    // Use a small title bar (Qt::Tool) with no minimize or maximize buttons
    // much like the Periodic Table widget
    setWindowFlags(Qt::Dialog | Qt::Tool);

    d = new InsertFragmentPrivate;

    QString dir;
#ifdef Q_WS_MAC
    dir = QDir::homePath() + "/Library/Application Support/Avogadro/";
#endif
    if (!dir.isEmpty())
      _directoryList << dir;
    d->model = new DirectoryTreeModel(_directoryList, this);

    ui.setupUi(this);
    ui.directoryTreeView->setModel(d->model);
    ui.directoryTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.directoryTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.directoryTreeView->setUniformRowHeights(true);

    connect(ui.insertFragmentButton, SIGNAL(clicked(bool)),
            this, SLOT(setupInsertMode(bool)));
    connect(ui.addDirectoryButton, SIGNAL(clicked(bool)),
            this, SLOT(addDirectory(bool)));
    connect(ui.clearListButton, SIGNAL(clicked(bool)),
      this, SLOT(clearDirectoryList(bool)));
  }

  InsertFragmentDialog::~InsertFragmentDialog()
  {
    delete d;
  }

  const Molecule *InsertFragmentDialog::fragment()
  {
    d->fragment.Clear();

    // SMILES insert
    if (d->smilesMode) { 
	
  		// We should use the method because it will grab updates to the line edit
      std::string SmilesString(smilesString().toAscii());
      if(d->conv.SetInFormat("smi")
         && d->conv.ReadString(&d->fragment, SmilesString))
        {
          d->builder.Build(d->fragment);
          d->fragment.Center();
          d->fragment.AddHydrogens();
        }
    } else {
      QModelIndexList selected = ui.directoryTreeView->selectionModel()->selectedIndexes();
      if (selected.count() != 0) {
        QString file = d->model->filePath(selected.first());
        if (!file.isEmpty()) {
          std::string fileName(file.toAscii());
          OBFormat *inFormat = d->conv.FormatFromExt(fileName.c_str());
          if (!inFormat || !d->conv.SetInFormat(inFormat)) {
            QMessageBox::warning( (QWidget*)this, tr( "Avogadro" ),
                                  tr( "Cannot read file format of file %1." )
                                  .arg( QString(fileName.c_str()) ) );
            return &d->fragment;
          }
          std::ifstream ifs;
          ifs.open(fileName.c_str());
          if (!ifs) {
            QMessageBox::warning( (QWidget*)this, tr( "Avogadro" ),
                                  tr( "Cannot read file %1." )
                                  .arg( QString(fileName.c_str()) ) );
            return &d->fragment;
          }

          d->conv.Read(&d->fragment, &ifs);
          d->fragment.Center();
        }
      }
    }

    return &d->fragment;
  }

  const QString InsertFragmentDialog::smilesString()
  {
	  if (!ui.smilesLineEdit->text().isEmpty()) {
			_smilesString = ui.smilesLineEdit->text();
		}
    return _smilesString;
  }

  void InsertFragmentDialog::setSmilesString(const QString smiles)
  {
    _smilesString = smiles;
		ui.smilesLineEdit->setText(_smilesString);
  }

  const QStringList InsertFragmentDialog::directoryList() const
  {
    return _directoryList;
  }
  
  void InsertFragmentDialog::setDirectoryList(const QStringList dirList)
  {
    _directoryList = dirList;
    refresh();
  }

  void InsertFragmentDialog::refresh()
  {
    d->model->setDirectoryList(_directoryList);
		ui.directoryTreeView->update();
  }

  void InsertFragmentDialog::setupInsertMode(bool)
  {
    bool inserting = (ui.insertFragmentButton->text() == tr("Stop Inserting"));

		if (ui.smilesLineEdit->hasFocus()) {
    	ui.smilesLineEdit->clearFocus();
			d->smilesMode = true;
		}
		else {
			d->smilesMode = false;
		}

    if(!inserting) {
      ui.insertFragmentButton->setText(tr("Stop Inserting"));
    } else {
      ui.insertFragmentButton->setText(tr("Insert Fragment"));
    }
    emit setInsertMode(!inserting);
  }

  void InsertFragmentDialog::closeEvent(QCloseEvent *event)
  {
    // stop inserting
    if (ui.insertFragmentButton->text() == tr("Stop Inserting"))
      setupInsertMode(false);
		event->accept();
  }

  void InsertFragmentDialog::addDirectory(bool)
  {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home");
    
    // If this is a new directory, add it in
    if (!_directoryList.contains(dir)) {
      _directoryList << dir;
      refresh();
    }
  }
  
  void InsertFragmentDialog::clearDirectoryList(bool)
  {
    _directoryList.clear();
    refresh();
  }

}

#include "insertfragmentdialog.moc"
