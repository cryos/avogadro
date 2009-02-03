/**********************************************************************
  Protein - Protein class 

  Copyright (C) 2009 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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
      virtual ~Protein();

      /**
       * @return QByteArray with codes for the protein's secondary structure.
       */
      QByteArray secondaryStructure() const;

      /**
       * @return the number of chains in this protein
       */
      int numChains() const;

      QList<unsigned long int> chainAtoms(int index) const;
      QList<unsigned long int> chainResidues(int index) const;

      int num3turnHelixes() const;
      int num4turnHelixes() const;
      int num5turnHelixes() const;
  
      QList<unsigned long int> helix3BackboneAtoms(int index);
      QList<unsigned long int> helix4BackboneAtoms(int index);
      QList<unsigned long int> helix5BackboneAtoms(int index);
  
    private:
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
      QList<unsigned long int> helixBackboneAtoms(char c, int index);

      ProteinPrivate * const d;
  };

} // End namespace Avogadro

#endif
