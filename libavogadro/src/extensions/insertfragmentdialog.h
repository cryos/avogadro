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

#ifndef INSERTFRAGMENTDIALOG_H
#define INSERTFRAGMENTDIALOG_H

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
    explicit InsertFragmentDialog(QWidget *parent = 0, QString directory = "fragments", Qt::WindowFlags f = 0);
    //! Destructor
    ~InsertFragmentDialog();

    const Molecule &fragment();

  public Q_SLOTS:
    void refresh();

    void filterTextChanged(const QString &);

    void clearFilterText();
    void activated();

  Q_SIGNALS:
    void performInsert();

  private:
    Ui::InsertFragmentDialog ui;

    InsertFragmentPrivate *d;
  };

} // End of Avogadro namespace

#endif

