/**********************************************************************
  PocketDialog - Dialog for Docking

  Copyright (C) 2007 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#ifndef POCKETDIALOG_H
#define POCKETDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QModelIndex>

#include <openbabel/dock.h>
#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>

#include "ui_pocketdialog.h"

namespace Avogadro
{
  class PocketDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit PocketDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Desconstructor
      ~PocketDialog();

      void setDock(OpenBabel::OBDock *dock) { m_dock = dock; }
      void setMolecule(Molecule *molecule) {m_molecule = molecule; }
      void setWidget(GLWidget *widget) {m_widget = widget; }
      int method() { return m_method; }
      double radius() { return m_radius; }
      QString resname() { return m_resname; }
      std::vector<int> pocket() { return m_pocket; }

    public slots:
      void accept();
      void reject();
      void methodToggled(int index);

    private:
      Ui::PocketDialog ui;
      
      int m_method;
      double m_radius;
      QString m_resname;
      OpenBabel::OBDock *m_dock;
      Molecule *m_molecule;
      GLWidget *m_widget;
      std::vector<int> m_pocket;
  };
}

#endif
