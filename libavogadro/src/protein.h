/**********************************************************************
  Protein - Protein class

  Copyright (C) 2009 Tim Vandermeersch

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

#ifndef PROTEIN_H
#define PROTEIN_H

#include <avogadro/global.h>

#include <QObject>
#include <QVector>

namespace Avogadro {

  class Atom;
  class Residue;
  class Molecule;

  class ProteinPrivate;
  /**
   * @class Protein protein.h <avogadro/protein.h>
   * @brief Representation of a protein, including secondary structure
   *
   * The Protein class helps other parts of the library or plugins to work
   * with proteins. If the molecule was read from a pdb file, an attempt
   * will be made to get the secondary structure information from the HELIX
   * and SHEET lines. If this fails, a simplified version of the DSSP 
   * algorithm is used.
   *
   * http://en.wikipedia.org/wiki/Secondary_structure#The_DSSP_code
   *
   * The secondary structure is assigned based on hydrogen bonding
   * patterns. There are eight types of secondary structure that
   * DSSP defines:
   *
   * * G = 3-turn helix (310 helix). Min length 3 residues.
   * * H = 4-turn helix (α helix). Min length 4 residues.
   * * I = 5-turn helix (π helix). Min length 5 residues.
   * * T = hydrogen bonded turn (3, 4 or 5 turn)
   * * E = extended strand in parallel and/or anti-parallel β-sheet conformation. Min length 2 residues.
   * * B = residue in isolated β-bridge (single pair β-sheet hydrogen bond formation)
   * * S = bend (the only non-hydrogen-bond based assignment)
   *
   * Amino acid residues which are not in any of the above
   * conformations are assigned as the eighth type '-' = Coil.
   * The helices (G,H and I) and sheet conformations are all required
   * to have a reasonable length. This means that 2 adjacent residues
   * in the primary structure must form the same hydrogen bonding
   * pattern. If the helix or sheet hydrogen bonding pattern is too
   * short they are designated as T or B, respectively.
   */
  class A_EXPORT Protein : public QObject
  {
    Q_OBJECT

    public:
      /**
       * Constructor.
       * @param molecule The object parent.
       */
      Protein(Molecule *molecule);
      /**
       * Destructor.
       */
      virtual ~Protein();

      //! @name Chains
      //@{
      /**
       * @return All the residues in the protein ordered by chain.
       */
      const QVector<QVector<Residue*> >& chains() const;
      /**
       * @return the number of chains in this protein
       */
      int numChains() const;
      /**
       * @return All atom ids for chain with @p index.
       */
      QList<unsigned long> chainAtoms(int index) const;
      /**
       * @return All residue ids for chain with @p index.
       */
      QList<unsigned long> chainResidues(int index) const;
      //@}

      //! @name Secondary structure
      //@{
      /**
       * @return QByteArray with codes for the protein's secondary structure.
       */
      QByteArray secondaryStructure() const;
      /**
       * @return True if the residue is part of a sheet.
       */ 
      bool isHelix(Residue *residue) const;
      /**
       * @return True if the residue is part of a sheet.
       */
      bool isSheet(Residue *residue) const;
      //@}

    private:
      bool extractFromPDB();
      
      void sortResiduesByChain();
      void iterateForward(Atom *prevCA, Atom *currN, QVector<bool> &visited);
      void iterateBackward(Atom *prevN, Atom *currCA, QVector<bool> &visited);

      void detectHBonds();

      void detectStructure();
      void extendHelix(char c, int turn, Residue *residue, const QVector<Residue*> &residues);
      void extendSheet(int delta, Residue *residue, const QVector<Residue*> &residues);
      void clearShortPatterns();
      void clearShortPatterns(char c, int min);

      int residueIndex(Residue *residue) const;

      int numHelixes(char c) const;
      QList<unsigned long> helixBackboneAtoms(char c, int index);

      ProteinPrivate * const d;
  };

} // End namespace Avogadro

#endif
