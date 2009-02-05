/**********************************************************************
  GaussianFchk - parses Gaussian formatted checkpoint files

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "gaussianfchk.h"

#include <Eigen/Core>

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

using namespace Eigen;
using namespace std;

namespace Avogadro
{
  using std::vector;

  GaussianFchk::GaussianFchk(QString filename, BasisSet* basis)
  {
    // Open the file for reading and process it
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return;

    qDebug() << "File" << filename << "opened.";

    // Process the formatted checkpoint and extract all the information we need
    m_in.setDevice(&file);
    while (!m_in.atEnd())
    {
      processLine();
    }

    // Now it should all be loaded load it into the basis set
    load(basis);
  }

  GaussianFchk::~GaussianFchk()
  {
  }

  void GaussianFchk::processLine()
  {
    // First truncate the line, remove trailing white space and check
    QString line = m_in.readLine();
    QString key = line;
    key.resize(42);
    key = key.trimmed();
    QString tmp = line.mid(43, 37);
    QStringList list = tmp.split(" ", QString::SkipEmptyParts);

    // Big switch statement checking for various things we are interested in
    if (key == "Number of atoms")
      qDebug() << "Number of atoms =" << list.at(1).toInt();
    else if (key == "Number of electrons")
      m_electrons = list.at(1).toInt();
    else if (key == "Number of basis functions")
      qDebug() << "Number of basis functions =" << list.at(1).toInt();
    else if (key == "Atomic numbers")
      m_aNums = readArrayI(list.at(2).toInt());
    // Now we get to the meat of it - coordinates of the atoms
    else if (key == "Current cartesian coordinates")
      m_aPos = readArrayD(list.at(2).toInt());
    // The real meat is here - basis sets etc!
    else if (key == "Shell types")
      m_shellTypes = readArrayI(list.at(2).toInt());
    else if (key == "Number of primitives per shell")
      m_shellNums = readArrayI(list.at(2).toInt());
    else if (key == "Shell to atom map")
      m_shelltoAtom = readArrayI(list.at(2).toInt());
    // Now to get the exponents and coefficients(
    else if (key == "Primitive exponents")
      m_a = readArrayD(list.at(2).toInt());
    else if (key == "Contraction coefficients")
      m_c = readArrayD(list.at(2).toInt());
    else if (key == "P(S=P) Contraction coefficients")
      m_csp = readArrayD(list.at(2).toInt());
    else if (key == "Alpha Orbital Energies")
    {
      m_orbitalEnergy = readArrayD(list.at(2).toInt());
      qDebug() << "MO energies, n =" << m_orbitalEnergy.size();
    }
    else if (key == "Alpha MO coefficients")
    {
      m_MOcoeffs = readArrayD(list.at(2).toInt());
      qDebug() << "MO coefficients, n =" << m_MOcoeffs.size();
    }

  }

  void GaussianFchk::load(BasisSet* basis)
  {
    // Now load up our basis set
    basis->setElectrons(m_electrons);
    int nAtom = 0;
    for (unsigned int i = 0; i < m_aPos.size(); i += 3)
      basis->addAtom(Vector3d(m_aPos.at(i), m_aPos.at(i+1), m_aPos.at(i+2)),
                     m_aNums.at(nAtom++));

    // Set up the GTO primitive counter, go through the shells and add them
    int nGTO = 0;
    for (unsigned int i = 0; i < m_shellTypes.size(); ++i)
    {
      if (m_shellTypes.at(i) == -1)
      {
        // SP orbital type - actually have to add two shells
        int s = basis->addBasis(m_shelltoAtom.at(i) - 1, S);
        int p = basis->addBasis(m_shelltoAtom.at(i) - 1, P);
        for (int j = 0; j < m_shellNums.at(i); ++j)
        {
          basis->addGTO(s, m_c.at(nGTO), m_a.at(nGTO));
          basis->addGTO(p, m_csp.at(nGTO), m_a.at(nGTO));
          nGTO++;
        }
      }
      else
      {
        orbital type;
        switch (m_shellTypes.at(i))
        {
          case 0:
            type = S;
            break;
          case 1:
            type = P;
            break;
          case 2:
            type = D;
            break;
          case -2:
            type = D5;
            break;
          case 3:
            type = F;
            break;
          case -3:
            type = F7;
            break;
          default:
            type = S;
        }
        int b = basis->addBasis(m_shelltoAtom.at(i) - 1, type);
        for (int j = 0; j < m_shellNums.at(i); ++j)
        {
          basis->addGTO(b, m_c.at(nGTO), m_a.at(nGTO));
          nGTO++;
        }
      }
    }
    // Now to load in the MO coefficients
    basis->addMOs(m_MOcoeffs);
  }

  vector<int> GaussianFchk::readArrayI(unsigned int n)
  {
    vector<int> tmp;
    while (tmp.size() < n)
    {
      QString line = m_in.readLine();
      QStringList list = line.split(" ", QString::SkipEmptyParts);
      for (int i = 0; i < list.size(); ++i)
        tmp.push_back(list.at(i).toInt());
    }
    return tmp;
  }

  vector<double> GaussianFchk::readArrayD(unsigned int n)
  {
    vector<double> tmp;
    while (tmp.size() < n)
    {
      QString line = m_in.readLine();
      QStringList list = line.split(" ", QString::SkipEmptyParts);
      for (int i = 0; i < list.size(); ++i)
        tmp.push_back(list.at(i).toDouble());
    }
    return tmp;
  }

  void GaussianFchk::outputAll()
  {
    qDebug() << "Shell mappings.";
    for (unsigned int i = 0; i < m_shellTypes.size(); ++i)
        qDebug() << i << ": type =" << m_shellTypes.at(i)
                      << ", number =" << m_shellNums.at(i)
                      << ", atom =" << m_shelltoAtom.at(i);
    qDebug() << "MO coefficients.";
    for (unsigned int i = 0; i < m_MOcoeffs.size(); ++i)
      qDebug() << m_MOcoeffs.at(i);
  }

}
