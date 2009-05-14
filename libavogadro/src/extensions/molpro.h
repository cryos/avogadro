/**********************************************************************
  Molpro - parses MOLPRO files

  Copyright (C) 2009 Michael Banck

  based on gaussianfchk.cpp,
  Copyright (C) 2008-2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
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

#ifndef MOLPRO_H
#define MOLPRO_H 

#include <QString>
#include <QTextStream>
#include <vector>

namespace Avogadro
{
  class BasisSet;

  class Molpro
  {
  public:
    Molpro(const QString &filename, BasisSet *basis);
    ~Molpro();
    void outputAll();
    void reorderD5Orbitals();
  private:
    QTextStream m_in;

    int m_electrons;
    unsigned int m_numBasisFunctions;
    std::vector<int> m_aNums;
    std::vector<double> m_aPos;
    std::vector<int> m_shellTypes;
    std::vector<int> m_shellNums;
    std::vector<int> m_shelltoAtom;
    std::vector<double> m_orbitalEnergy;
    std::vector<double> m_MOcoeffs;
  };

} // End namespace Avogadro

#endif
