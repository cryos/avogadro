/**********************************************************************
  GaussianChk - parses Gaussian formatted checkpoint files

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef __GAUSSIANFCHK_H
#define __GAUSSIANFCHK_H

#include "basisset.h"
#include <QString>
#include <QTextStream>
#include <QList>

namespace Avogadro
{
  class GaussianFchk
  {
  public:
    GaussianFchk(QString filename, BasisSet *basis);
    ~GaussianFchk();
    void outputAll();
  private:
    QTextStream m_in;
    void processLine();
    void load(BasisSet* basis);
    QList<int> readArrayI(int n);
    QList<double> readArrayD(int n);

    int m_electrons;
    QList<int> m_aNums;
    QList<double> m_aPos;
    QList<int> m_shellTypes;
    QList<int> m_shellNums;
    QList<int> m_shelltoAtom;
    QList<double> m_a;
    QList<double> m_c;
    QList<double> m_csp;
    QList<double> m_orbitalEnergy;
    QList<double> m_MOcoeffs;
  };

} // End namespace Avogadro

#endif
