/**********************************************************************
  MopacAux - parses Gaussian formatted checkpoint files

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

#include "mopacaux.h"

#include "slaterset.h"

#include <Eigen/Core>

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

using std::vector;
using Eigen::MatrixXd;
using Eigen::Vector3d;

namespace Avogadro
{
  using std::vector;

  MopacAux::MopacAux(QString filename, SlaterSet* basis)
  {
    // Open the file for reading and process it
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return;

    qDebug() << "File" << filename << "opened.";

    // Process the formatted checkpoint and extract all the information we need
    m_in.setDevice(&file);
    while (!m_in.atEnd()) {
      processLine();
    }

    // Now it should all be loaded load it into the basis set
    load(basis);
  }

  MopacAux::~MopacAux()
  {
  }

  void MopacAux::processLine()
  {
    // First truncate the line, remove trailing white space and check
    QString line = m_in.readLine();
    QString key = line;
    key = key.trimmed();
//    QStringList list = tmp.split("=", QString::SkipEmptyParts);

    // Big switch statement checking for various things we are interested in
    if (key.contains("ATOM_EL")) {
      QString tmp = key.mid(key.indexOf("[")+1, 4);
      qDebug() << "Number of atoms =" << tmp.toInt();
    }
    else if (key.contains("AO_ATOMINDEX")) {
      QString tmp = key.mid(key.indexOf("[")+1, 4);
      qDebug() << "Number of atomic orbitals =" << tmp.toInt();
      m_atomIndex = readArrayI(tmp.toInt());
      for (unsigned int i = 0; i < m_atomIndex.size(); ++i) {
        --m_atomIndex[i];
        qDebug() << i << ":" << m_atomIndex[i];
      }
    }
    else if (key.contains("ATOM_SYMTYPE")) {
      QString tmp = key.mid(key.indexOf("[")+1, 4);
      qDebug() << "Number of atomic orbital types =" << tmp.toInt();
      m_atomSym = readArraySym(tmp.toInt());
      for (unsigned int i = 0; i < m_atomSym.size(); ++i)
        qDebug() << i << ":" << m_atomSym[i];
    }
    else if (key.contains("AO_ZETA")) {
      QString tmp = key.mid(key.indexOf("[")+1, 4);
      qDebug() << "Number of zeta values =" << tmp.toInt();
      m_zeta = readArrayD(tmp.toInt());
      for (unsigned int i = 0; i < m_zeta.size(); ++i)
        qDebug() << i << ":" << m_zeta[i];
    }
    else if (key.contains("ATOM_PQN")) {
      QString tmp = key.mid(key.indexOf("[")+1, 4);
      qDebug() << "Number of PQN values =" << tmp.toInt();
      m_pqn = readArrayD(tmp.toInt());
      for (unsigned int i = 0; i < m_pqn.size(); ++i)
        qDebug() << i << ":" << m_pqn[i];
    }
    else if (key.contains("ATOM_X_OPT:ANGSTROMS")) {
      QString tmp = key.mid(key.indexOf("[")+1, 4);
      qDebug() << "Number of atomic coordinates =" << tmp.toInt();
      m_atomPos = readArrayVec(tmp.toInt());
      for (unsigned int i = 0; i < m_atomPos.size(); ++i)
        qDebug() << i << ":" << m_atomPos[i].x() << m_atomPos[i].y() << m_atomPos[i].z();
    }
    else if (key.contains("OVERLAP_MATRIX")) {
      QString tmp = key.mid(key.indexOf("[")+1, 6);
      qDebug() << "Size of lower half triangle of overlap matrix =" << tmp.toInt();

      readOverlapMatrix(tmp.toInt());
      for (unsigned int i = 0; i < m_zeta.size(); ++i)
        qDebug() << i << ":" << m_overlap(i,0) << m_overlap(i,1) << m_overlap(i,2);
    }
    else if (key.contains("EIGENVECTORS")) {
      QString tmp = key.mid(key.indexOf("[")+1, 6);
      qDebug() << "Size of eigen vectors matrix =" << tmp.toInt();

      readEigenVectors(tmp.toInt());
      for (unsigned int i = 0; i < m_zeta.size(); ++i)
        qDebug() << i << ":" << m_eigenVectors(i,0) << m_eigenVectors(i,1) << m_eigenVectors(i,2);
    }
/*    else if (key == "Number of electrons")
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
    else if (key == "Alpha MO coefficients")
    {
      m_MOcoeffs = readArrayD(list.at(2).toInt());
      qDebug() << "MO coefficients, n =" << m_MOcoeffs.size();
    }
*/
  }

  void MopacAux::load(SlaterSet* basis)
  {
    // Now load up our basis set
    basis->addAtoms(m_atomPos);
    basis->addSlaterIndices(m_atomIndex);
    basis->addSlaterTypes(m_atomSym);
    basis->addZetas(m_zeta);
    basis->addPQNs(m_pqn);
    basis->addOverlapMatrix(m_overlap);
    basis->addEigenVectors(m_eigenVectors);
  }

  vector<int> MopacAux::readArrayI(unsigned int n)
  {
    vector<int> tmp;
    while (tmp.size() < n) {
      QString line = m_in.readLine();
      QStringList list = line.split(" ", QString::SkipEmptyParts);
      for (int i = 0; i < list.size(); ++i)
        tmp.push_back(list.at(i).toInt());
    }
    return tmp;
  }

  vector<double> MopacAux::readArrayD(unsigned int n)
  {
    vector<double> tmp;
    while (tmp.size() < n) {
      QString line = m_in.readLine();
      QStringList list = line.split(" ", QString::SkipEmptyParts);
      for (int i = 0; i < list.size(); ++i)
        tmp.push_back(list.at(i).toDouble());
    }
    return tmp;
  }

  vector<int> MopacAux::readArraySym(unsigned int n)
  {
    int type;
    vector<int> tmp;
    while (tmp.size() < n) {
      QString line = m_in.readLine();
      QStringList list = line.split(" ", QString::SkipEmptyParts);
      for (int i = 0; i < list.size(); ++i) {
        if (list.at(i) == "S") type = SlaterSet::S;
        else if (list.at(i) == "PX") type = SlaterSet::PX;
        else if (list.at(i) == "PY") type = SlaterSet::PY;
        else if (list.at(i) == "PZ") type = SlaterSet::PZ;
        else if (list.at(i) == "X2") type = SlaterSet::X2;
        else if (list.at(i) == "XZ") type = SlaterSet::XZ;
        else if (list.at(i) == "Z2") type = SlaterSet::Z2;
        else if (list.at(i) == "YZ") type = SlaterSet::YZ;
        else if (list.at(i) == "XY") type = SlaterSet::XY;
        else type = SlaterSet::UU;
        tmp.push_back(type);
      }
    }
    return tmp;
  }

  vector<Vector3d> MopacAux::readArrayVec(unsigned int n)
  {
    vector<Vector3d> tmp(n/3);
    double *ptr = tmp[0].data();
    unsigned int cnt = 0;
    while (cnt < n) {
      QString line = m_in.readLine();
      QStringList list = line.split(" ", QString::SkipEmptyParts);
      for (int i = 0; i < list.size(); ++i) {
        ptr[cnt++] = list.at(i).toDouble();
      }
    }
    return tmp;
  }

  bool MopacAux::readOverlapMatrix(unsigned int n)
  {
    m_overlap.resize(m_zeta.size(), m_zeta.size());
    unsigned int cnt = 0;
    unsigned int i = 0, j = 0;
    unsigned int f = 1;
    // Skip the first commment line...
    m_in.readLine();
    while (cnt < n) {
      QString line = m_in.readLine();
      QStringList list = line.split(" ", QString::SkipEmptyParts);
      for (int k = 0; k < list.size(); ++k) {
        //m_overlap.part<Eigen::SelfAdjoint>()(i, j) = list.at(k).toDouble();
        m_overlap(i, j) = m_overlap(j, i) = list.at(k).toDouble();
        ++i; ++cnt;
        if (i == f) {
          // We need to move down to the next row and increment f - lower tri
          i = 0;
          ++f;
          ++j;
        }
      }
    }
    return true;
  }

  bool MopacAux::readEigenVectors(unsigned int n)
  {
    m_eigenVectors.resize(m_zeta.size(), m_zeta.size());
    unsigned int cnt = 0;
    unsigned int i = 0, j = 0;
    while (cnt < n) {
      QString line = m_in.readLine();
      QStringList list = line.split(" ", QString::SkipEmptyParts);
      for (int k = 0; k < list.size(); ++k) {
        m_eigenVectors(i, j) = list.at(k).toDouble();
        ++i; ++cnt;
        if (i == m_zeta.size()) {
          // We need to move down to the next row and increment f - lower tri
          i = 0;
          ++j;
        }
      }
    }
    return true;
  }

  void MopacAux::outputAll()
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
