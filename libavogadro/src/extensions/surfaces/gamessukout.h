/**********************************************************************
  GamessukOut - parses the horror of a GAMESS-UK output file

  Copyright (C) 2010-2011 Jens Thomas

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

#ifndef GAMESSUKOUT_H
#define GAMESSUKOUT_H

#include "gaussianset.h"

#include <QString>
#include <Eigen/Core>

#include <vector>
#include <string>

#define BUFF_SIZE 32768

namespace Avogadro
{
  class GUKBasisSet
  {
    /*
      This class is used to hold the data parsed from the GAMESS-UK output before we create the Avogadro basis set object
     */
  public:
    GUKBasisSet() {};
    ~GUKBasisSet() {};

    void outputCoord();
    void outputBasis();
    /*
      Return true if we have already processed an atom of this label type
    */
    bool labelIndex( std::string label );
    /*
      Return the enum from basis.h for the supplied label as a string
      basisset.h: enum orbital { S, SP, P, D, D5, F, F7, UU };
    */
    orbital shellTypeFromString(std::string label);

    std::vector<std::string> atomLabels; // ordered list of atom labels mapping to coordinates
    std::vector<Eigen::Vector3d> coordinates; // Coordinates in Bohr
    std::vector<std::string> shellLabels; // list of atom labels for the basis functions
    std::vector< std::vector< orbital > > shells; // vector of shell types for each atom (use basisset.h orbital enum here)
    std::vector< std::vector< unsigned int > > gtoIndicies; // Vector of vector of index of where the GTO for a particular shell ends
    std::vector<double> gtoExponents;  // list of exponents
    std::vector<double> gtoCoefficients; // list of contraction coefficients

    // Need to look at what to do about uhf calculations
    std::vector<double> moEnergies; // list of the energies of the MOs
    std::vector< std::vector<double> > moVectors; // list of list of MO vectors in order of moEnergies

    // These are read in after the basis is printed and used by readMOVectors - could use to check the basis 
    // has been parsed correctly too
    int nShell;
    int nBasisFunctions; 
    int nElectrons;
  };

  class GamessukOut
  {
  public:
    GamessukOut(const QString &filename, GaussianSet *basis);
    ~GamessukOut();
    void GamessukOutNoQt(const std::string &filename, GaussianSet *basis);
    void outputParsedData();

  private:
    bool parseFile(std::ifstream &ifs);
    void readInitialCoordinates(std::ifstream &ifs);
    void readBasisSet(std::ifstream &ifs);
    inline void addSpBasis(std::vector<double> s_coeff,
			   std::vector<double> p_coeff,
			   std::vector<double> sp_exponents);
    void readOptimisedCoordinates(std::ifstream &ifs);
    void readMOs(std::ifstream &ifs);
    int readMOVectors(std::ifstream &ifs);

    void load(GaussianSet* basis);

    void addBasisForLabel( unsigned int atomIndex, std::string label, GaussianSet* basis );

    // This holds the basis data parsed in from the file and which is used to create the BasisSet
    GUKBasisSet gukBasis;

    // For parsing the file
    char buffer[BUFF_SIZE];
    std::string line;
    std::vector<std::string> tokens;

  };

} // End namespace Avogadro

#endif
