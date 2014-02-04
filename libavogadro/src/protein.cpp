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

#include "protein.h"

#include <avogadro/molecule.h>
#include <avogadro/residue.h>
#include <avogadro/atom.h>
#include <avogadro/neighborlist.h>

#include <QVector>
#include <QVariant>
#include <QStringList>
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
    if (!extractFromPDB()) {
      detectHBonds();
      detectStructure();
    }

    /*
    foreach (const QVector<Residue*> &residues, d->chains) { // for each chain
      qDebug() << "chain: " << d->chains.indexOf(residues);
      QByteArray chain;
      foreach (Residue *residue, residues) { // for each residue in the chain
        chain.append( d->structure.at(residue->index()) );
      }
      qDebug() << chain;
    }
    */

  }

  Protein::~Protein()
  {
    delete d;
  }

  QByteArray Protein::secondaryStructure() const
  {
    return d->structure;
  }
      
  const QVector<QVector<Residue*> >& Protein::chains() const
  {
    return d->chains;
  }
  
  bool Protein::isHelix(Residue *residue) const
  {
    char key = d->structure.at(residue->index());
    switch(key) {
      case 'G':
      case 'H':
      case 'I':
        return true;
      default:
        return false;
    }
  }

  bool Protein::isSheet(Residue *residue) const
  {
    char key = d->structure.at(residue->index());
    switch(key) {
      case 'E':
      case 'B':
        return true;
      default:
        return false;
    }
  }




  bool Protein::extractFromPDB()
  {
    bool found = false;

    /*
    COLUMNS        DATA  TYPE     FIELD         DEFINITION
    -----------------------------------------------------------------------------------
     1 -  6        Record name    "HELIX "
     8 - 10        Integer        serNum        Serial number of the helix. This starts
                                                at 1  and increases incrementally.
    12 - 14        LString(3)     helixID       Helix  identifier. In addition to a serial
                                                number, each helix is given an 
                                                alphanumeric character helix identifier.
    16 - 18        Residue name   initResName   Name of the initial residue.
    20             Character      initChainID   Chain identifier for the chain containing
                                                this  helix.
    22 - 25        Integer        initSeqNum    Sequence number of the initial residue.
    26             AChar          initICode     Insertion code of the initial residue.
    28 - 30        Residue  name  endResName    Name of the terminal residue of the helix.
    32             Character      endChainID    Chain identifier for the chain containing
                                                this  helix.
    34 - 37        Integer        endSeqNum     Sequence number of the terminal residue.
    38             AChar          endICode      Insertion code of the terminal residue.
    39 - 40        Integer        helixClass    Helix class (see below).
    41 - 70        String         comment       Comment about this helix.
    72 - 76        Integer        length        Length of this helix.
    */
    QVariant helix = d->molecule->property("HELIX");
    if (helix.isValid()) {
      found = true;

      QStringList lines = helix.toString().split('\n');
      foreach (const QString &line, lines) {
        //qDebug() << "line:" << line;

        bool ok;
        QString helixID = line.mid(5, 3);

        // 
        // initial residue
        //
        QString initResName = line.mid(9, 3);

        QString initChainID = line.mid(13, 1);
        int initChainNum = -1;
        foreach (Residue *residue, d->molecule->residues()) {
          if (QString(residue->chainID()) != initChainID)
            continue;
          initChainNum = residue->chainNumber();
          break;
        }

        if (initChainNum < 0) {
          qDebug() << "Protein: Error, invalid initChainID for helix" << helixID;
          return false;
        }

        int initSeqNum = line.mid(15, 4).toInt(&ok);
        if (!ok) {
          qDebug() << "Protein: Error, can't read interger from initSeqNum for helix" << helixID;
          return false;
        }

        Residue *initResidue = 0;
        foreach (Residue *residue, d->chains.at(initChainNum)) {
          if (residue->number().toInt() == initSeqNum) {
            initResidue = residue;
            break;
          }       
        }

        if (!initResidue) {
          qDebug() << "Protein: Error, could not find initResidue in the chain for helix" << helixID;
          return false;
        }

        if (initResidue->name() != initResName) {
          qDebug() << "Protein: Error, initResName does not match the residue "
              "at the specified position for helix" << helixID;
          qDebug() << initResName << "!=" << initResidue->name();
          return false;
        }
        
        // 
        // end residue
        //
        QString endResName = line.mid(21, 3);

        QString endChainID = line.mid(25, 1);
        int endChainNum = -1;
        foreach (Residue *residue, d->molecule->residues()) {
          if (QString(residue->chainID()) != endChainID)
            continue;
          endChainNum = residue->chainNumber();
        }

        if (endChainNum < 0) {
          qDebug() << "Protein: Error, invalid endChainID for helix" << helixID;
          return false;
        }

        int endSeqNum = line.mid(27, 4).toInt(&ok);
        if (!ok) {
          qDebug() << "Protein: Error, can't read interger from endSeqNum for helix" << helixID;
          return false;
        }

        Residue *endResidue = 0;
        foreach (Residue *residue, d->chains.at(endChainNum)) {
          if (residue->number().toInt() == endSeqNum) {
            endResidue = residue;
            break;
          }       
        }

        if (!endResidue) {
          qDebug() << "Protein: Error, could not find endResidue in the chain for helix" << helixID;
          return false;
        }

        if (endResidue->name() != endResName) {
          qDebug() << "Protein: Error, endResName does not match the residue "
              "at the specified position for helix" << helixID;
          qDebug() << endResName << "!=" << endResidue->name();
          return false;
        }

        int helixClass = line.mid(32, 2).toInt(&ok);
        if (!ok) {
          qDebug() << "Protein: Error, can't read helix class for helix" << helixID;
          return false;
        }

        /*                                   CLASS NUMBER
        TYPE OF  HELIX                     (COLUMNS 39 - 40)
        --------------------------------------------------------------
        Right-handed alpha (default)                1
        Right-handed omega                          2
        Right-handed pi                             3
        Right-handed gamma                          4
        Right-handed 3 - 10                         5
        Left-handed alpha                           6
        Left-handed omega                           7
        Left-handed gamma                           8
        2 - 7 ribbon/helix                          9
        Polyproline                                10
        */

        char key;
        switch (helixClass) {
          case 1:
            key = 'H';
            break;
          case 3:
            key = 'I';
            break;
          case 5:
            key = 'G';
            break;
          default:
            key = '-';
            break;
        }

        int initIndex = d->chains.at(initChainNum).indexOf(initResidue);
        int endIndex = d->chains.at(initChainNum).indexOf(endResidue);
        for (int i = initIndex; i < endIndex; ++i) {
          Residue *residue = d->chains.at(initChainNum).at(i);
          d->structure.data()[residue->index()] = key;
        }

      }
    }
    /*
    COLUMNS        DATA  TYPE     FIELD         DEFINITION
    -------------------------------------------------------------------------------------
     1 -  6        Record name   "SHEET "
     8 - 10        Integer       strand         Strand  number which starts at 1 for each
                                                strand within a sheet and increases by one.
    12 - 14        LString(3)    sheetID        Sheet  identifier.
    15 - 16        Integer       numStrands     Number  of strands in sheet.
    18 - 20        Residue name  initResName    Residue  name of initial residue.
    22             Character     initChainID    Chain identifier of initial residue 
                                                in strand. 
    23 - 26        Integer       initSeqNum     Sequence number of initial residue
                                                in strand.
    27             AChar         initICode      Insertion code of initial residue
                                                in  strand.
    29 - 31        Residue name  endResName     Residue name of terminal residue.
    33             Character     endChainID     Chain identifier of terminal residue.
    34 - 37        Integer       endSeqNum      Sequence number of terminal residue.
    38             AChar         endICode       Insertion code of terminal residue.
    39 - 40        Integer       sense          Sense of strand with respect to previous
                                                strand in the sheet. 0 if first strand,
                                                1 if  parallel,and -1 if anti-parallel.
    42 - 45        Atom          curAtom        Registration.  Atom name in current strand.
    46 - 48        Residue name  curResName     Registration.  Residue name in current strand
    50             Character     curChainId     Registration. Chain identifier in
                                                current strand.
    51 - 54        Integer       curResSeq      Registration.  Residue sequence number
                                                in current strand.
    55             AChar         curICode       Registration. Insertion code in
                                                current strand.
    57 - 60        Atom          prevAtom       Registration.  Atom name in previous strand.
    61 - 63        Residue name  prevResName    Registration.  Residue name in
                                                previous strand.
    65             Character     prevChainId    Registration.  Chain identifier in
                                                previous  strand.
    66 - 69        Integer       prevResSeq     Registration. Residue sequence number
                                                in previous strand.
    70             AChar         prevICode      Registration.  Insertion code in
                                                previous strand.
    */
    QVariant sheet = d->molecule->property("SHEET");
    if (sheet.isValid()) {
      found = true;

      QStringList lines = sheet.toString().split('\n');
      foreach (const QString &line, lines) {
        //qDebug() << "line:" << line;

        bool ok;
        QString sheetID = line.mid(5, 3);
        /*
        int numStrands = line.mid(8, 2).toInt(&ok);
        if (!ok) {
          qDebug() << "Protein: Error, can't read interger from numStrands for sheet" << sheetID;
          return false;
        }
        */

        // 
        // initial residue
        //
        QString initResName = line.mid(11, 3);

        QString initChainID = line.mid(15, 1);
        int initChainNum = -1;
        foreach (Residue *residue, d->molecule->residues()) {
          if (QString(residue->chainID()) != initChainID)
            continue;
          initChainNum = residue->chainNumber();
        }
        
        if (initChainNum < 0) {
          qDebug() << "Protein: Error, invalid initChainID for sheet" << sheetID;
          return false;
        }

        int initSeqNum = line.mid(16, 4).toInt(&ok);
        if (!ok || !initSeqNum) {
          qDebug() << "Protein: Error, can't read interger from initSeqNum for sheet" << sheetID;
          return false;
        }

        Residue *initResidue = 0;
        foreach (Residue *residue, d->chains.at(initChainNum)) {
          if (residue->number().toInt() == initSeqNum) {
            initResidue = residue;
            break;
          }       
        }

        if (!initResidue) {
          qDebug() << "Protein: Error, could not find initResidue in the chain for sheet" << sheetID;
          return false;
        }

        if (initResidue->name() != initResName) {
          qDebug() << "Protein: Error, initResName does not match the residue "
              "at the specified position for sheet" << sheetID;
          qDebug() << initResName << "!=" << initResidue->name();
          return false;
        }

        // 
        // end residue
        //
        QString endResName = line.mid(22, 3);

        QString endChainID = line.mid(26, 1);
        int endChainNum = -1;
        foreach (Residue *residue, d->molecule->residues()) {
          if (QString(residue->chainID()) != endChainID)
            continue;
          endChainNum = residue->chainNumber();
        }
        
        if (endChainNum < 0) {
          qDebug() << "Protein: Error, invalid endChainID for sheet" << sheetID;
          return false;
        }

        int endSeqNum = line.mid(27, 4).toInt(&ok);
        if (!ok || !endSeqNum) {
          qDebug() << "Protein: Error, can't read interger from endSeqNum for sheet" << sheetID;
          return false;
        }

        Residue *endResidue = 0;
        foreach (Residue *residue, d->chains.at(endChainNum)) {
          if (residue->number().toInt() == endSeqNum) {
            endResidue = residue;
            break;
          }       
        }

        if (!endResidue) {
          qDebug() << "Protein: Error, could not find endResidue in the chain for sheet" << sheetID;
          return false;
        }

        if (endResidue->name() != endResName) {
          qDebug() << "Protein: Error, endResName does not match the residue "
              "at the specified position for sheet" << sheetID;
          qDebug() << endResName << "!=" << endResidue->name();
          return false;
        }

        char key = '-';
        int length = endSeqNum - initSeqNum;
        if (length == 1)
          key = 'B';
        if (length > 1)
          key = 'E';

        int initIndex = d->chains.at(initChainNum).indexOf(initResidue);
        int endIndex = d->chains.at(initChainNum).indexOf(endResidue);
        for (int i = initIndex; i < endIndex; ++i) {
          Residue *residue = d->chains.at(initChainNum).at(i);
          d->structure.data()[residue->index()] = key;
        }

      } 
    }

    d->num3turnHelixes = -1;
    d->num4turnHelixes = -1;
    d->num5turnHelixes = -1; 

    return found;
  }


  int Protein::numChains() const
  {
    return d->chains.size();
  }

  QList<unsigned long> Protein::chainAtoms(int index) const
  {
    QList<unsigned long> ids;
    if (index >= d->chains.size())
      return ids;

    foreach (Residue *res, d->chains.at(index))
      foreach (unsigned long id, res->atoms())
        ids.append(id);

    return ids;
  }

  QList<unsigned long> Protein::chainResidues(int index) const
  {
    QList<unsigned long> ids;
    if (index >= d->chains.size())
      return ids;

    foreach (Residue *res, d->chains.at(index))
      ids.append(res->id());

    return ids;
  }

  /*
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

  int Protein::num3turnHelixes() const
  {
    if (d->num3turnHelixes >= 0)
      return d->num3turnHelixes;
    d->num3turnHelixes = numHelixes('G');
    return d->num3turnHelixes;
  }

  int Protein::num4turnHelixes() const
  {
    if (d->num4turnHelixes >= 0)
      return d->num4turnHelixes;
    d->num4turnHelixes = numHelixes('H');
    return d->num4turnHelixes;
  }

  int Protein::num5turnHelixes() const
  {
    if (d->num5turnHelixes >= 0)
      return d->num5turnHelixes;
    d->num5turnHelixes = numHelixes('I');
    return d->num5turnHelixes;
  }
  */

  QList<unsigned long> Protein::helixBackboneAtoms(char c, int index)
  {
    QList <unsigned long> ids;
    int count = 0;
    for (int i = 0 ; i < d->structure.size(); ++i) {
      if (d->structure.at(i) == c) {
        if (count == index) {

          while (d->structure.at(i) == c) {
            Residue *residue = d->molecule->residue(i);
            unsigned long O, N, C, CA;
            foreach (unsigned long id, residue->atoms()) {
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
        while (d->structure.at(i) == c)
          ++i;
      }
    }

    return ids;
  }

  /*
  QList<unsigned long> Protein::helix3BackboneAtoms(int index)
  {
    return helixBackboneAtoms('G', index);
  }

  QList<unsigned long> Protein::helix4BackboneAtoms(int index)
  {
    return helixBackboneAtoms('H', index);
  }

  QList<unsigned long> Protein::helix5BackboneAtoms(int index)
  {
    return helixBackboneAtoms('I', index);
  }
  */

  int Protein::residueIndex(Residue *residue) const
  {
    return d->chains.at(residue->chainNumber()).indexOf(residue);
  }

  bool isAminoAcid(Residue *residue)
  {
    QString resname = residue->name();
    if (resname == "ALA")
      return true;
    if (resname == "ARG")
      return true;
    if (resname == "ASN")
      return true;
    if (resname == "ASP")
      return true;
    if (resname == "CYS")
      return true;
    if (resname == "GLU")
      return true;
    if (resname == "GLN")
      return true;
    if (resname == "GLY")
      return true;
    if (resname == "HIS")
      return true;
    if (resname == "ILE")
      return true;
    if (resname == "LEU")
      return true;
    if (resname == "LYS")
      return true;
    if (resname == "MET")
      return true;
    if (resname == "PHE")
      return true;
    if (resname == "PRO")
      return true;
    if (resname == "SER")
      return true;
    if (resname == "THR")
      return true;
    if (resname == "TRP")
      return true;
    if (resname == "TYR")
      return true;
    if (resname == "VAL")
      return true;
    return false;
  }

  void Protein::iterateBackward(Atom *prevN, Atom *currC, QVector<bool> &visited)
  {
    Residue *residue = currC->residue();
    visited[residue->index()] = true;
    if (!isAminoAcid(residue))
      return;

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
    if (!isAminoAcid(residue))
      return;

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
    d->structure.resize(d->molecule->numResidues());
    for (int i = 0 ; i < d->structure.size(); ++i)
      d->structure[i] = '-';

    // determine the number of chains
    unsigned int numChains = 0;
    foreach (Residue *residue, d->molecule->residues()) {
      if (!isAminoAcid(residue))
        continue;
      if (residue->chainNumber() > numChains)
        numChains = residue->chainNumber();
    }
    d->chains.resize(numChains+1);

    QVector<bool> visited(d->molecule->numResidues());

    foreach (Residue *residue, d->molecule->residues()) {
      if (residue->atoms().size() < 4)
        continue;

      foreach (unsigned long id, residue->atoms()) {
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
    NeighborList neighborList(d->molecule, 4.0);

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

        Eigen::Vector3d H_pos(Eigen::Vector3d::Zero());
        Atom *H = 0, *N = 0, *C = 0, *O = 0;
        // find N in first residue
        foreach (unsigned long id, residue1->atoms()) {
          if (residue1->atomId(id).trimmed() == "N") 
            N = d->molecule->atomById(id);
        }
        if (!N)
          continue;
        
        // find neighboring H, or compute it's position if there are no hydrogens
        foreach (unsigned long nbrId, N->neighbors()) {
          Atom *neighbor = d->molecule->atomById(nbrId);
          if (neighbor->isHydrogen()) {
            H = d->molecule->atomById(nbrId);
            H_pos = *H->pos(); 
            break;
          } else {
            H_pos += *N->pos() - *neighbor->pos();
          }
        }
        if (!H) {
          H_pos = *N->pos() + 1.1 * H_pos.normalized();           
        }

        // find C & O in residue 2
        foreach (unsigned long id, residue2->atoms()) {
          if (residue2->atomId(id).trimmed() == "C") C = d->molecule->atomById(id);
          if (residue2->atomId(id).trimmed() == "O") O = d->molecule->atomById(id);
        }
        if (!C || !O)
          continue;

        //  C=O ~ H-N
        //
        //  C +0.42e   O -0.42e
        //  H +0.20e   N -0.20e
        double rON = (*O->pos() - *N->pos()).norm();
        double rCH = (*C->pos() - H_pos).norm();
        double rOH = (*O->pos() - H_pos).norm();
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

} // End namespace
