/**********************************************************************
  Molden - parses Molden files

  Copyright (C) 2010 Geoffrey R. Hutchison

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

#ifndef MOLDEN_H
#define MOLDEN_H

#include <Eigen/Core>
#include <vector>

#include "gaussianset.h"

namespace Avogadro
{

  class MoldenFile
  {
  // Parsing mode: section of the file currently being parsed
  enum mode { NotParsing, Atoms, GTO, STO, MO, SCF };
  public:
    MoldenFile(const QString &filename, GaussianSet *basis);
    ~MoldenFile();
    void outputAll();
  private:
    QIODevice *m_in;
    void processLine();
    void load(GaussianSet* basis);

    double m_coordFactor;
    mode m_currentMode;
    int m_electrons;
    unsigned int m_numBasisFunctions;
    std::vector<int> m_aNums;
    std::vector<double> m_aPos;
    std::vector<orbital> m_shellTypes;
    std::vector<int> m_shellNums;
    std::vector<int> m_shelltoAtom;
    std::vector<double> m_a;
    std::vector<double> m_c;
    std::vector<double> m_csp;
    std::vector<double> m_orbitalEnergy;
    std::vector<double> m_MOcoeffs;
  };

} // End namespace Avogadro

#endif
