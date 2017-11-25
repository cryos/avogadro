/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2016 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef GUESSPROJECTIONS_H
#define GUESSPROJECTIONS_H

#include <vector>

#include <avogadro/atom.h>
#include <avogadro/molecule.h>

#include <openbabel/generic.h>
#include <openbabel/mol.h>

#include <Eigen/Dense>

#include "numorbitals.h"

inline static bool alreadyUsed(const std::string& symbol,
                               const std::vector<std::string>& alreadyLookedAt)
{
  for (size_t i = 0; i < alreadyLookedAt.size(); ++i) {
    if (symbol == alreadyLookedAt[i])
      return true;
  }
  return false;
}

inline static bool alreadyUsed(int atomicNum,
                               const std::vector<int>& alreadyLookedAt)
{
  for (size_t i = 0; i < alreadyLookedAt.size(); ++i) {
    if (atomicNum == alreadyLookedAt[i])
      return true;
  }
  return false;
}

/* Returns a guess for the yaehmop projections QString of a set of atomic
 * symbols. Does them by types. For example, {O, Ti, O} would return:
 * # O
 * atom 1 1.0, 3 1.0
 * # Ti
 * atom 2 1.0
 *
 * @param mol The molecule of interest.
 * @return The guessed atom projections. We assume summing (i. e., all
 *         weights are 1.0).
 */
static QString guessTypedAtomProjections(const Avogadro::Molecule* mol)
{
  std::vector<std::string> atomicSymbols;
  QList<Avogadro::Atom*> atoms = mol->atoms();
  for (size_t i = 0; i < atoms.size(); ++i)
    atomicSymbols.push_back(OpenBabel::etab.GetSymbol(atoms[i]->atomicNumber()));

  QString ret;
  std::vector<std::string> alreadyLookedAt;
  for (size_t i = 0; i < atomicSymbols.size(); ++i) {
    if (!alreadyUsed(atomicSymbols[i], alreadyLookedAt)) {
      ret += (QString("# ") + atomicSymbols[i].c_str() + "\n");
      ret += (QString("atom ") + QString::number(i + 1) + " 1.0");
      for (size_t j = i + 1; j < atomicSymbols.size(); ++j) {
        if (atomicSymbols[i] == atomicSymbols[j])
          ret += (QString(", ") + QString::number(j + 1) + " 1.0");
      }
      ret += "\n";
      alreadyLookedAt.push_back(atomicSymbols[i]);
    }
  }
  return ret;
}

static QString displayAtomNumbers(const Avogadro::Molecule* mol)
{
  QList<Avogadro::Atom*> atoms = mol->atoms();

  QString ret;
  for (size_t i = 0; i < atoms.size(); ++i) {
    unsigned int atomicNumber = atoms[i]->atomicNumber();
    QString symbol = OpenBabel::etab.GetSymbol(atomicNumber);
    const Eigen::Vector3d& pos = *atoms[i]->pos();
    ret += (QString("# ") + symbol + " (");
    ret += (QString::number(pos[0]) + ", " +
            QString::number(pos[1]) + ", " +
            QString::number(pos[2]) + ") (Cartesian): " + QString::number(i+1));
    ret += "\n";
  }
  return ret;
}

static QString guessOrbitalProjections(const Avogadro::Molecule* mol)
{
  QString ret;
  QList<Avogadro::Atom*> atoms = mol->atoms();
  std::vector<int> alreadyLookedAt;

  for (size_t i = 0; i < atoms.size(); ++i) {
    int atomicNum = atoms[i]->atomicNumber();

    if (alreadyUsed(atomicNum, alreadyLookedAt))
      continue;
    alreadyLookedAt.push_back(atomicNum);

    int numOrbs = getNumYaehmopOrbitals(atomicNum);
    int angMaxInd = 0;
    if (numOrbs == 1)
      angMaxInd = 1;
    else if (numOrbs == 4)
      angMaxInd = 2;
    else if (numOrbs == 9)
      angMaxInd = 3;
    else if (numOrbs == 16)
      angMaxInd = 4;

    for (size_t angInd = 0; angInd < angMaxInd; ++angInd) {
      ret += "# ";
      ret += (QString(OpenBabel::etab.GetSymbol(atomicNum)) + " ");
      if (angInd == 0)
        ret += "s";
      else if (angInd == 1)
        ret += "px, py, pz";
      else if (angInd == 2)
        ret += "dx2y2, dz2, dxy, dxz, dyz";
      else if (angInd == 3)
        ret += "fz3, fxz2, fyz2, fxyz, fz(x2-y2), fx(x2-3y2), fy(3x2-y2)";
      ret += "\norbital ";

      size_t orbInd = 1;
      for (size_t j = 0; j < atoms.size(); ++j) {
        if (atomicNum == atoms[j]->atomicNumber()) {
          if (angInd == 0) {
            ret += (QString::number(orbInd) + " 1.0, ");
          }
          if (angInd == 1) {
            for (size_t k = 1; k <= 3; ++k)
              ret += (QString::number(orbInd + k) + " 1.0, ");
          }
          if (angInd == 2) {
            for (size_t k = 4; k <= 8; ++k)
              ret += (QString::number(orbInd + k) + " 1.0, ");
          }
          if (angInd == 3) {
            for (size_t k = 9; k <= 15; ++k)
              ret += (QString::number(orbInd + k) + " 1.0, ");
          }
        }
        if (j + 1 == atoms.size()) {
          // Remove the final ", ", and create a new line.
          if (ret.size() >= 2)
            ret.chop(2);
          ret += "\n";
        }
        orbInd += getNumYaehmopOrbitals(atoms[j]->atomicNumber());
      }
    }
  }
  return ret;
}

static QString guessDetailedOrbitalProjections(const Avogadro::Molecule* mol)
{
  QString ret;
  QList<Avogadro::Atom*> atoms = mol->atoms();
  size_t ind = 1;

  for (size_t i = 0; i < atoms.size(); ++i) {
    int atomicNum = atoms[i]->atomicNumber();
    const Eigen::Vector3d& pos = *atoms[i]->pos();

    ret += "# ";
    ret += OpenBabel::etab.GetSymbol(atoms[i]->atomicNumber());
    ret += (QString(" (") + QString::number(pos[0]) + ", " +
                             QString::number(pos[1]) + ", " +
                             QString::number(pos[2]) +
             ") (Cartesian)\n");
    size_t numOrbs = getNumYaehmopOrbitals(atomicNum);
    if (numOrbs == 0)
      ret += "# **No orbitals for this atomic number!**";
    if (numOrbs >= 1) {
      ret += "# s\n";
      ret += ("orbital " + QString::number(ind) + " 1.0\n");
      ++ind;
    }
    if (numOrbs >= 4) {
      ret += "# px, py, pz\n";
      ret += "orbital ";
      for (size_t i = 0; i < 3; ++i) {
        ret += (QString::number(ind) + " 1.0");
        if (i != 2)
          ret += ", ";
        ++ind;
      }
      ret += "\n";
    }
    if (numOrbs >= 9) {
      ret += "# dx2y2, dz2, dxy, dxz, dyz\n";
      ret += "orbital ";
      for (size_t i = 0; i < 5; ++i) {
        ret += (QString::number(ind) + " 1.0");
        if (i != 4)
          ret += ", ";
        ++ind;
      }
      ret += "\n";
    }
    if (numOrbs == 16) {
      ret += "# fz3, fxz2, fyz2, fxyz, fz(x2-y2), fx(x2-3y2), fy(3x2-y2)\n";
      ret += "orbital ";
      for (size_t i = 0; i < 7; ++i) {
        ret += (QString::number(ind) + " 1.0");
        if (i != 6)
          ret += ", ";
        ++ind;
      }
      ret += "\n";
    }
    ret += "\n";
  }
  return ret;
}

static QString displayOrbitalNumbers(const Avogadro::Molecule* mol)
{
  QString ret;
  QList<Avogadro::Atom*> atoms = mol->atoms();
  size_t ind = 1;

  for (size_t i = 0; i < atoms.size(); ++i) {
    int atomicNum = atoms[i]->atomicNumber();
    const Eigen::Vector3d& pos = *atoms[i]->pos();

    ret += "# ";
    ret += OpenBabel::etab.GetSymbol(atoms[i]->atomicNumber());
    ret += (QString(" (") + QString::number(pos[0]) + ", " +
                             QString::number(pos[1]) + ", " +
                             QString::number(pos[2]) +
             ") (Cartesian)\n");
    size_t numOrbs = getNumYaehmopOrbitals(atomicNum);
    if (numOrbs == 0)
      ret += "  # **No orbitals for this atomic number!**";
    if (numOrbs >= 1) {
      ret += (QString("  # s: ") + QString::number(ind) + "\n");
      ++ind;
    }
    if (numOrbs >= 4) {
      ret += "  # px, py, pz: ";
      for (size_t i = 0; i < 3; ++i) {
        ret += (QString::number(ind));
        if (i != 2)
          ret += ", ";
        ++ind;
      }
      ret += "\n";
    }
    if (numOrbs >= 9) {
      ret += "  # dx2y2, dz2, dxy, dxz, dyz: ";
      for (size_t i = 0; i < 5; ++i) {
        ret += (QString::number(ind));
        if (i != 4)
          ret += ", ";
        ++ind;
      }
      ret += "\n";
    }
    if (numOrbs == 16) {
      ret += "  # fz3, fxz2, fyz2, fxyz, fz(x2-y2), fx(x2-3y2), fy(3x2-y2): ";
      for (size_t i = 0; i < 7; ++i) {
        ret += (QString::number(ind));
        if (i != 6)
          ret += ", ";
        ++ind;
      }
      ret += "\n";
    }
    ret += "\n";
  }
  return ret;
}

#endif
