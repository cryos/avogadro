/**********************************************************************
  Copyright (C) 2007 Carsten Niehaus <cniehaus@kde.org>

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QtGui/QDialog>

namespace Avogadro {
  class AboutDialog : public QDialog
  {
      Q_OBJECT

    public:
      AboutDialog( QWidget *parent );
      ~AboutDialog();
  };
}
#endif // ABOUTDIALOG_H
