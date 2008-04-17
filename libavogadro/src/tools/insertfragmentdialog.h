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

#ifndef __INSERTFRAGMENTDIALOG_H
#define __INSERTFRAGMENTDIALOG_H

#include <avogadro/global.h>

#include <QDialog>

#include "ui_insertfragmentdialog.h"

class QCloseEvent;

namespace Avogadro {

  class Molecule;
  class InsertFragmentPrivate;

  class InsertFragmentDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit InsertFragmentDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
      //! Desconstructor
      ~InsertFragmentDialog();

      const Molecule *fragment();

			const QString smilesString();
      void setSmilesString(const QString);

      const QStringList directoryList() const;
      void setDirectoryList(const QStringList);

			void closeEvent( QCloseEvent *event );

    public slots:
      void refresh();
      void setupInsertMode(bool);
      void addDirectory(bool);

    signals:
      void setInsertMode(bool);

    private:
      Ui::InsertFragmentDialog ui;

      QStringList _directoryList;
      QString     _smilesString;

      InsertFragmentPrivate *d;
  };

} // End of Avogadro namespace

#endif

