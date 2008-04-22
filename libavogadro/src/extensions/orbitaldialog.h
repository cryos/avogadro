/**********************************************************************
  OrbitalExtension - Extension for generating orbital cubes

  Copyright (C) 2008 Marcus D. Hanwell

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

#ifndef ORBITALDIALOG_H
#define ORBITALDIALOG_H

#include <QDialog>

#include "ui_orbitaldialog.h"

namespace Avogadro
{
  class OrbitalDialog : public QDialog
  {
  Q_OBJECT

  public:
    OrbitalDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~OrbitalDialog();
    void setMOs(int num);
    void setHOMO(int num);
    void setLUMO(int num);

  private:
    Ui::OrbitalDialog ui;

  public Q_SLOTS:
    void loadFile();
    void calculate();

  Q_SIGNALS:
    void fileName(QString filename);
    void calculateMO(int MO);

  };

} // End namespace Avogadro

#endif
