/**********************************************************************
  OrcaSpectra - plot IR spectra

  Copyright (C) 2014 Dagmar Lenk

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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
#ifndef ORCASPECTRA_H
#define ORCASPECTRA_H
#include "ui_orcaspectra.h"

#include "orcadata.h"

#include <QtGui>

namespace Avogadro {

class OrcaSpectra : public QDialog
{
  // The Q_OBJECT macro must be included if a class is to have
  // signals/slots
  Q_OBJECT

public:
    OrcaSpectra(QWidget *parent = 0, Qt::WindowFlags f = 0 );

public slots:

    void plotSpectra(OrcaVibrations *vibData);
private slots:
    void freqChangedIdx(double x, double y);

Q_SIGNALS:
    void selectNewVibration(int n);

private:

    OrcaVibrations *m_vibration;
    // This member provides access to all ui elements
    Ui::OrcaSpectra ui;

};
} // end namespace avogadro
#endif // ORCASPECTRA_H
