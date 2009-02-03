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

#include <avogadro/protein.h>
#include <avogadro/molecule.h>
#include <avogadro/residue.h>
#include <avogadro/atom.h>
#include <avogadro/neighborlist.h>

#include <QVector>
#include <QDebug>

namespace Avogadro {

  class ProteinPrivate
  {
    public:
      Molecule                        *molecule;
      QVector<QVector<Residue*> >      chains;
      QVector<QVector<Residue*> >      hbondPairs;
      QByteArray                       structure;

      mutable int num3turnHelixes;
      mutable int num4turnHelixes;
      mutable int num5turnHelixes;
  };
      
  Protein::Protein(Molecule *molecule) : d(new ProteinPrivate)
  {
    d->molecule = molecule;
    sortResiduesByChain();
    detectHBonds();
    detectStructure();

    foreach (const QVector<Residue*> &residues, d->chains) { // for each chain
      qDebug() << "chain: " << d->chains.indexOf(residues);
      QByteArray chain;
      foreach (Residue *residue, residues) { // for each residue in the chain
        chain.append( d->structure.at(residue->index()) );
      }
      qDebug() << chain;
    }


    qDebug() << d->structure;
  }
  
  Protein::~Protein()
  {
  }
      
  QByteArray Protein::secondaryStructure() const
  {
    return d->structure;
  }
      
      
  int Protein::numChains() const
  {
    return d->chains.size();
  }
  
  QList<unsigned long int> Protein::chainAtoms(int index) const
  {
    QList<unsigned long int> ids;
    if (index >= d->chains.size())
      return ids;

    foreach (Residue *res, d->chains.at(index))
      foreach (unsigned long int id, res->atoms())
        ids.append(id);
      
    return ids;
  }
      
  QList<unsigned long int> Protein::chainResidues(int index) const
  {
    QList<unsigned long int> ids;
    if (index >= d->chains.size())
      return ids;

    foreach (Residue *res, d->chains.at(index))
      ids.append(res->id());
      
    return ids;
  }

  int Protein::numHelixes(char c) const
  {
    int count = 0;
    foreach (const QVector<Residue*> &residues, d->chains) { // for each chain
      for (int i = 0 ; i < residues.size(); ++i) {
        if (d->structure.at(residues.at(i)->index()) == c) {
          count++;
          while (d->structure.at(residues.at(i)->index()) == c)
            ++i;
        }
      }
    }

    return count;
  }

  int Protein::numHelixes3() const
  {
    if (d->num3turnHelixes >= 0)
      return d->num3turnHelixes;
    d->num3turnHelixes = numHelixes('G');
    return d->num3turnHelixes;
  }

  int Protein::numHelixes4() const
  {
    if (d->num4turnHelixes >= 0)
      return d->num4turnHelixes;
    d->num4turnHelixes = numHelixes('H');
    return d->num4turnHelixes;
  }

  int Protein::numHelixes5() const
  {
    if (d->num5turnHelixes >= 0)
      return d->num5turnHelixes;
    d->num5turnHelixes = numHelixes('I');
    return d->num5turnHelixes;
  }

  QList<unsigned long int> Protein::helix4BackboneAtoms(int index)
  {
    QList <unsigned long int> ids;
    int count = 0;
    for (int i = 0 ; i < d->structure.size(); ++i) {
      if (d->structure.at(i) == 'H') {
        if (count == index) {
          
          while (d->structure.at(i) == 'H') {
            Residue *residue = d->molecule->residue(i);
            unsigned long int O, N, C, CA;
            foreach (unsigned long int id, residue->atoms()) {
              QString atomId = residue->atomId(id).trimmed();
              if (atomId == "N" ) N  = id;
              if (atomId == "CA") CA = id;
              if (atomId == "C" ) C  = id;
              if (atomId == "O" ) O  = id;
            }
            ids.append(N);
            ids.append(CA);
            ids.append(C);
            ids.append(O);
            ++i;
          }

          return ids;
        }

        count++;
        // skip to next non 'H' char
        while (d->structure.at(i) == 'H')
          ++i;
      }
    }

    return ids;
  } 


  int Protein::residueIndex(Residue *residue) const
  {
    return d->chains.at(residue->chainNumber()).indexOf(residue);
  }

  void Protein::iterateBackward(Atom *prevN, Atom *currC, QVector<bool> &visited)
  {
    Residue *residue = currC->residue();
    visited[residue->index()] = true;

    d->chains[residue->chainNumber()].prepend(residue);

    foreach (unsigned long id1, currC->neighbors()) {
      Atom *nbr1 = d->molecule->atomById(id1);
      if (nbr1 == prevN)
        continue;
      
      QString nbr1Id = nbr1->residue()->atomId(nbr1->id()).trimmed();
      if (nbr1Id == "CA") {
        foreach (unsigned long id2, nbr1->neighbors()) {
          Atom *nbr2 = d->molecule->atomById(id2);
          if (nbr2 == currC)
            continue;
      
          QString nbr2Id = nbr2->residue()->atomId(nbr2->id()).trimmed();
          if (nbr2Id == "N") {
            foreach (unsigned long id3, nbr2->neighbors()) {
              Atom *nbr3 = d->molecule->atomById(id3);
              if (nbr3 == nbr1)
                continue;
              
              QString nbr3Id = nbr3->residue()->atomId(nbr3->id()).trimmed();
              if (nbr3Id == "C") {
                if (!visited.at(nbr3->residue()->index()))
                  iterateBackward(nbr2, nbr3, visited);
              }
            }
          }
        }
      } else if (nbr1Id == "N") {
        if (!visited.at(nbr1->residue()->index()))
          iterateForward(currC, nbr1, visited);
      }
    }
  
  }
  
  void Protein::iterateForward(Atom *prevC, Atom *currN, QVector<bool> &visited)
  {
    Residue *residue = currN->residue();
    visited[residue->index()] = true;

    d->chains[residue->chainNumber()].append(residue);

    foreach (unsigned long id1, currN->neighbors()) {
      Atom *nbr1 = d->molecule->atomById(id1);
      if (nbr1 == prevC)
        continue;
      
      QString nbr1Id = nbr1->residue()->atomId(nbr1->id()).trimmed();
      if (nbr1Id == "CA") {
        foreach (unsigned long id2, nbr1->neighbors()) {
          Atom *nbr2 = d->molecule->atomById(id2);
          if (nbr2 == currN)
            continue;
      
          QString nbr2Id = nbr2->residue()->atomId(nbr2->id()).trimmed();
          if (nbr2Id == "C") {
            foreach (unsigned long id3, nbr2->neighbors()) {
              Atom *nbr3 = d->molecule->atomById(id3);
              if (nbr3 == nbr1)
                continue;
              
              QString nbr3Id = nbr3->residue()->atomId(nbr3->id()).trimmed();
              if (nbr3Id == "N") {
                if (!visited.at(nbr3->residue()->index()))
                  iterateForward(nbr2, nbr3, visited);
              }
            }
          }
        }
      } else if (nbr1Id == "C") {
        if (!visited.at(nbr1->residue()->index()))
          iterateBackward(currN, nbr1, visited);
      }
    }
  }

  void Protein::sortResiduesByChain()
  {
    // determine the number of chains
    unsigned int numChains = 0;
    foreach (Residue *residue, d->molecule->residues()) {
      if (residue->chainNumber() > numChains)
        numChains = residue->chainNumber();
    }
    d->chains.resize(numChains+1);

    QVector<bool> visited(d->molecule->numResidues());

    foreach (Residue *residue, d->molecule->residues()) {
      if (residue->atoms().size() < 4)
        continue;

      foreach (unsigned long int id, residue->atoms()) {
        Atom *atom = d->molecule->atomById(id);
        QString atomId = residue->atomId(id).trimmed();
        
        if (visited.at(atom->residue()->index()))
          continue;
        

        if (atomId == "N")
          iterateForward(0, atom, visited);
        else if (atomId == "CA")
          iterateBackward(0, atom, visited);

      } // end atoms in residue
    
    }
  
  }
      
  void Protein::detectHBonds()
  {
    d->hbondPairs.resize(d->molecule->numResidues());
    NeighborList neighborList(d->molecule, 4, 1);
    
    for (unsigned int i = 0; i < d->molecule->numAtoms(); ++i) {
      Atom *atom = d->molecule->atom(i);
      QList<Atom*> nbrs = neighborList.nbrs(atom);

      foreach(Atom *nbr, nbrs) {
        Residue *residue1 = atom->residue();
        if (!residue1)
          continue;

        Residue *residue2 = nbr->residue();
        if (!residue2)
          continue;

        if (residue1 == residue2)
          continue;
        
        if (d->hbondPairs.at(residue1->index()).contains(residue2))
          continue;

        int res1 = residueIndex(residue1);
        int res2 = residueIndex(residue2);
        int delta = abs(res1 - res2);
        
        if (delta <= 2)
          continue;
 
        // residue 1 has the N-H
        // residue 2 has the C=O 
        if (residue1->atomId(atom->id()).trimmed() != "O") {
          if (residue2->atomId(nbr->id()).trimmed() != "O")
            continue;
        } else {
          Residue *swap = residue1;
          residue1 = residue2;
          residue2 = swap;        
        }

        Atom *H = 0, *N = 0, *C = 0, *O = 0;
        foreach (unsigned long int id, residue1->atoms()) {
          if (residue1->atomId(id).trimmed() == "N") N = d->molecule->atomById(id);
          if (residue1->atomId(id).trimmed() == "H") H = d->molecule->atomById(id);
        }
        foreach (unsigned long int id, residue2->atoms()) {
          if (residue2->atomId(id).trimmed() == "C") C = d->molecule->atomById(id);
          if (residue2->atomId(id).trimmed() == "O") O = d->molecule->atomById(id);
        }

        if (!C || !O || !N || !H)
          continue;

        //  C=O ~ H-N
        //  
        //  C +0.42e   O -0.42e
        //  H +0.20e   N -0.20e
        double rON = (*O->pos() - *N->pos()).norm();
        double rCH = (*C->pos() - *H->pos()).norm();
        double rOH = (*O->pos() - *H->pos()).norm();
        double rCN = (*C->pos() - *N->pos()).norm();

        double eON = 332 * (-0.42 * -0.20) / rON;
        double eCH = 332 * ( 0.42 *  0.20) / rCH;
        double eOH = 332 * (-0.42 *  0.20) / rOH;
        double eCN = 332 * ( 0.42 * -0.20) / rCN;
        double E = eON + eCH + eOH + eCN;

        if (E >= -0.5)
          continue;

        d->hbondPairs[residue1->index()].append(residue2);
        d->hbondPairs[residue2->index()].append(residue1);
        
        //qDebug() << atom->residue()->index() << "-" << nbr->residue()->index() << "=" << delta;
      }
    }
  
  }
      
  void Protein::detectStructure()
  {
    d->structure.resize(d->molecule->numResidues());
    for (int i = 0 ; i < d->structure.size(); ++i)
      d->structure[i] = '-';


    foreach (const QVector<Residue*> &residues, d->chains) { // for each chain

      foreach (Residue *residue, residues) { // for each residue in the chain

        //qDebug() << "extending 3-trun helix...";
        extendHelix('G', 3, residue, residues);
        //qDebug() << "3 turn helix:" << d->structure;
        clearShortPatterns('G', 3);
        //qDebug() << "     cleaned:" << d->structure;

        //qDebug() << "extending 4-trun helix...";
        extendHelix('H', 4, residue, residues);
        //qDebug() << "4 trun helix:" << d->structure;
        clearShortPatterns('H', 4);
        //qDebug() << "     cleaned:" << d->structure;

        //qDebug() << "extending 5-trun helix";
        extendHelix('I', 5, residue, residues);
        //qDebug() << "5 trun helix:" << d->structure;
        clearShortPatterns('I', 5);
        //qDebug() << "     cleaned:" << d->structure;

        if (d->structure.at(residue->index()) != '-')
          continue;

        //extendSheet(0, residue, residues);
      }
    }
  
    d->num3turnHelixes = -1;
    d->num4turnHelixes = -1;
    d->num5turnHelixes = -1;
  }

  void Protein::extendHelix(char c, int turn, Residue *residue, const QVector<Residue*> &residues)
  {
     if (d->structure.at(residue->index()) != '-')
       return;

    // 4-turn helix
    foreach (Residue *partner, d->hbondPairs.at(residue->index())) { // for each H-bond partner
      if (residue->chainNumber() != partner->chainNumber())
        continue;

      int res1 = residues.indexOf(residue);
      int res2 = residues.indexOf(partner);
      int delta = abs(res1 - res2);

      if (delta == turn) {
        d->structure.data()[residue->index()] = c;

        int next = res1 + 1;
        if (next >= residues.size())
          return;

        Residue *nextResidue = residues.at(next);
        extendHelix(c, turn, nextResidue, residues);
      }

    }
  }

  void Protein::extendSheet(int delta, Residue *residue, const QVector<Residue*> &residues)
  {
    // 4-turn helix
    foreach (Residue *partner, d->hbondPairs.at(residue->index())) { // for each H-bond partner
      int res1 = residues.indexOf(residue);
      int res2 = residues.indexOf(partner);
      int del = abs(res1 - res2);

      if ((del == delta) || !delta) {
        int next = res1 + 1;
        if (next == residues.size())
          continue;

        Residue *nextResidue = residues.at(next);
        d->structure.data()[residue->index()] = 'B';
        extendSheet(del, nextResidue, residues);
      }

    }
  }

  void Protein::clearShortPatterns()
  {
    clearShortPatterns('G', 3);
    clearShortPatterns('H', 4);
  }
  
  void Protein::clearShortPatterns(char c, int min)
  {
    for (int i = 0 ; i < d->structure.size(); ++i) {
      if (d->structure.at(i) == c) {
        QByteArray array;
        for (int j = i ; j < d->structure.size(); ++j) {
          if (d->structure.at(j) == c)
            array.append('-');
          else
            break;
        }

        if (array.size() < min)
          d->structure.replace(i, array.size(), array);

        i += array.size();
      }
    }
  }
 
} // End namespace Avogadro

#include "protein.moc"
