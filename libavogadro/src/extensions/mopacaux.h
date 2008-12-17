/**********************************************************************
  MopacAux - parses Mopac Aux files

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#ifndef MOPACAUX_H
#define MOPACAUX_H

#include <QString>
#include <QTextStream>
#include <Eigen/Core>
#include <vector>

namespace Avogadro
{
  class SlaterSet;

  class MopacAux
  {
  public:
    MopacAux(QString filename, SlaterSet *basis);
    ~MopacAux();
    void outputAll();

  private:
    QTextStream m_in;
    void processLine();
    void load(SlaterSet* basis);
    std::vector<int> readArrayI(unsigned int n);
    std::vector<double> readArrayD(unsigned int n);
    std::vector<int> readArraySym(unsigned int n);
    std::vector<Eigen::Vector3d> readArrayVec(unsigned int n);
    bool readOverlapMatrix(unsigned int n);
    bool readEigenVectors(unsigned int n);
    bool readDensityMatrix(unsigned int n);

    int m_electrons;
    std::vector<int> m_aNums;
    std::vector<double> m_aPos;
    std::vector<int> m_shellTypes;
    std::vector<int> m_shellNums;
    std::vector<int> m_shelltoAtom;
    std::vector<double> m_c;
    std::vector<double> m_csp;
    std::vector<double> m_orbitalEnergy;
    std::vector<double> m_MOcoeffs;

    std::vector<int> m_atomIndex;
    std::vector<int> m_atomSym;
    std::vector<double> m_zeta;
    std::vector<int> m_pqn;
    std::vector<Eigen::Vector3d> m_atomPos;

    Eigen::MatrixXd m_overlap;     /// Overlap matrix
    Eigen::MatrixXd m_eigenVectors;
    Eigen::MatrixXd m_density;     /// Total density matrix
  };

} // End namespace Avogadro

#endif
