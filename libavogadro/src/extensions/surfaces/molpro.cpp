/**********************************************************************
  Molpro - parses MOLPRO files

  Copyright (C) 2009 Michael Banck

  based on gaussianfchk.cpp,
  Copyright (C) 2008-2009 Marcus D. Hanwell

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

#include "molpro.h"
#include "basisset.h"

#include <QFile>
#include <QStringList>
#include <QString>
#include <QDebug>

using Eigen::Vector3d;
using std::vector;

namespace Avogadro
{

  Molpro::Molpro(const QString &filename, BasisSet* basis)
  {
    // Open the file for reading and process it
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return;

    qDebug() << "File" << filename << "opened.";

    // Process the file and extract all the information we need
    m_in.setDevice(&file);
    while (!m_in.atEnd()) {
      QString line = m_in.readLine();
      if (line.indexOf( "NR  ATOM" ) != -1 ) {
        QString aNum;
        line = m_in.readLine(); // blank
        line = m_in.readLine();
        QStringList list = line.split(' ', QString::SkipEmptyParts);
        while (list.size() == 6) {
          aNum = list.at(2);
          aNum.chop(3); // get rid of the ".00"
          basis->addAtom(Vector3d(list.at(3).toDouble(),
                                  list.at(4).toDouble(),
                                  list.at(5).toDouble()), aNum.toInt()); 	
	  m_aNums.push_back(aNum.toInt());
	  m_aPos.push_back(list.at(3).toDouble());
	  m_aPos.push_back(list.at(4).toDouble());
	  m_aPos.push_back(list.at(5).toDouble());
	  m_aPos.push_back(list.at(3).toDouble());

          line = m_in.readLine();
          list = line.split(' ', QString::SkipEmptyParts);
        } 
      }
      else if (line.indexOf( "NUMBER OF ELECTRONS" ) != -1 ) {
        QStringList list = line.split(' ', QString::SkipEmptyParts);
        QString alpha = list.at(3);
        QString beta = list.at(4);
        // get rid of the +/- signs after the values
        alpha.chop(1);
        beta.chop(1);
        basis->setElectrons(alpha.toInt() + beta.toInt());
	m_electrons = alpha.toInt() + beta.toInt();
      }
      else if (line.indexOf( "NUMBER OF CONTRACTIONS" ) != -1 ) {
        QStringList list = line.split(' ', QString::SkipEmptyParts);
        m_numBasisFunctions = list.at(3).toInt();
      } 
      /* Atomic basis definitions.  They look like this in the output:
 BASIS DATA

   Nr Sym  Nuc  Type         Exponents   Contraction coefficients

   1.1 A     1  1s         6665.000000     0.000692   -0.000146    0.000000
   2.1 A                   1000.000000     0.005329   -0.001154    0.000000
   3.1 A                    228.000000     0.027077   -0.005725    0.000000
                             64.710000     0.101718   -0.023312    0.000000
                             21.060000     0.274740   -0.063955    0.000000
                              7.495000     0.448564   -0.149981    0.000000
                              2.797000     0.285074   -0.127262    0.000000
                              0.521500     0.015204    0.544529    0.000000
                              0.159600    -0.003191    0.580496    1.000000
   4.1 A     1  2px           9.439000     0.038109
                              2.002000     0.209480
                              0.545600     0.508557
                              0.151700     0.468842
   5.1 A     1  2py           9.439000     0.038109
[...]
      */
      else if (line.indexOf( "Nr Sym  Nuc  Type" ) != -1 ) {
        QString type;
	int shellType = -1;
        int currentBasis;
	int currentAtom;
	int zetaType;
        int ignoreShells = 0;
	int fchkShellType;
	bool newShell = true;
        line = m_in.readLine(); // blank
        line = m_in.readLine();
        QStringList list = line.split(' ', QString::SkipEmptyParts);
        QString frontline = line.mid(1, 25);
        QStringList frontlist = frontline.split(' ', QString::SkipEmptyParts);
        QString backline = line.mid(25, 55);
        QStringList backlist = backline.split(' ', QString::SkipEmptyParts);
        while (list.size() > 1) {
	  vector< vector<double> > contrCoeffs;
	  vector<double> exponents;
	  /* New shell. */
	  if (ignoreShells <= 0)
	    newShell = true;
	  else
	    newShell = false;
	  currentAtom=list.at(2).toInt();
	  zetaType = list.size()-5;
	  /* regular basis sets have one contraction coefficient per shell.
 	   * Double-zeta basis sets have three, tripe-zeta have four and so on. */
          type = list.at(3);
          type.remove(QRegExp("[xyz,0-9]"));
          type.remove(QRegExp("[+-]"));
	  /* FIXME: handle cartesian d/f basis sets */
	  if (newShell) {
            if (type == "s" ) {
	      shellType = S;
	      ignoreShells = 0;
	      fchkShellType = 0;
            } else if (type == "p" ) {
              shellType = P;
	      ignoreShells = 2;
	      fchkShellType = 1;
            } else if (type == "d" ) {
	      shellType = D5;
	      ignoreShells = 4;
	      fchkShellType = -2;
            } else if (type == "f" ) {
	      shellType = F7;
	      ignoreShells = 6;
	      fchkShellType = -3;
	    } else {
	      qDebug() << "cannot parse!" << type; 
	      qDebug() << line;
              return; 
            }
	  }
	  exponents.push_back(list.at(4).toDouble());
	  vector<double> contractions;
          for (int i = 0; i < zetaType; i++) {
	    if (newShell)
	      m_shellTypes.push_back(fchkShellType);
	    contractions.push_back(list.at(5+i).toDouble());
	  }
	  contrCoeffs.push_back(contractions);

          line = m_in.readLine();
          list = line.split(' ', QString::SkipEmptyParts);
          frontline = line.mid(1, 25);
          frontlist = frontline.split(' ', QString::SkipEmptyParts);
          backline = line.mid(25, 55);
          backlist = backline.split(' ', QString::SkipEmptyParts);

          while (list.size() > 0 && frontlist.size() < 4) {
	    /* new contraction.  This can either have a AO header without type
             * in front, like
    2.1 A                   1000.000000     0.005329   -0.001154    0.000000
               or the second case,
                              64.710000     0.101718   -0.023312    0.000000
             */
	    QString backline = line.mid(25, 55);
            backlist = backline.split(' ', QString::SkipEmptyParts);
            exponents.push_back(backlist.at(0).toDouble());
	    vector<double> contractions;
	    for (int i = 0; i < zetaType; i++) {
	      contractions.push_back(backlist.at(1+i).toDouble());
	    }
            contrCoeffs.push_back(contractions);

            line = m_in.readLine();
            list = line.split(' ', QString::SkipEmptyParts);
            frontline = line.mid(1, 25);
            frontlist = frontline.split(' ', QString::SkipEmptyParts);
            backline = line.mid(25, 55);
            backlist = backline.split(' ', QString::SkipEmptyParts);
          }

	  /* We're at the end of this shell.  Now add the data we gathered to
	   * the basis unless this is for a redundant p/d/f shell to be
	   * skipped. */
	  if (newShell == true) {
	    for (int i=0; i < zetaType; i++) {
	      m_shelltoAtom.push_back(currentAtom);
	      int numExponents = 0;
	      currentBasis = basis->addBasis(currentAtom-1, (Avogadro::orbital)shellType); 
	      for (unsigned int j=0; j < exponents.size(); j++) {
	        if (fabs(contrCoeffs[j][i]) > 0.0) {
	          if (zetaType == 1 
		      || (i < zetaType - 1 
		      && contrCoeffs[j][zetaType-1] == 0.0)
		      || (i == zetaType - 1)) {
		    /* for multiple zeta shells, do not add GTOs for the
		     * columns if the last column is non-zero. */
		    numExponents++;
	            basis->addGTO(currentBasis, contrCoeffs[j][i], exponents[j]);
		  }
		}
	      }
	      m_shellNums.push_back(numExponents); /* FIXME */
	    }
	    newShell = false;
	  } else {
	    if (ignoreShells > 0) {
	      ignoreShells--;
	    }
	  }
	}
      } 
      else if (line.indexOf( "Orb  Occ    Energy") != -1 ) {
/* Molecular orbitals are printed out by MOLPRO as follows:
   Orb  Occ    Energy  Couls-En    Coefficients

                                   1 1s      1 1s      1 1s      1 2px     1 2py     1 2pz     1 2px     1 2py     1 2pz     1 3d0
                                   1 3d2-    1 3d1+    1 3d2+    1 3d1-    2 1s      2 1s      2 2px     2 2py     2 2pz     3 1s
                                   3 1s      3 2px     3 2py     3 2pz     4 1s      4 1s      4 2px     4 2py     4 2pz     5 1s
                                   5 1s      5 2px     5 2py     5 2pz

   1.1   2   -11.2389  -31.0278  1.001397  0.007923 -0.008912  0.000006  0.000000  0.000017 -0.000011  0.000000 -0.000036 -0.000017
                                 0.000000 -0.000002  0.000008  0.000000  0.000075  0.001063 -0.000219 -0.000330 -0.000069  0.000075
                                 0.001063 -0.000219  0.000330 -0.000070  0.000075  0.001063  0.000322  0.000000 -0.000242  0.000105
                                 0.001059  0.000115  0.000000  0.000380

   2.1   2    -0.9219   -6.2932 -0.022923  0.749443 -0.163575  0.000692  0.000002  0.002152 -0.000431 -0.000001 -0.001340  0.000006
[...]
*/
        line = m_in.readLine(); // blank
        line = m_in.readLine(); // column header start
        QStringList list = line.split(' ', QString::SkipEmptyParts);
        while (list.size() > 1) {
          // skip column header
          line = m_in.readLine();
          list = line.split(' ', QString::SkipEmptyParts);
        }
        line = m_in.readLine();
        list = line.split(' ', QString::SkipEmptyParts);
        while (list.size() > 1) {
          // new MO
          m_orbitalEnergy.push_back(list.at(2).toDouble());
          QStringList MOList;
          QString MOLine = line.mid(32);
          MOList = MOLine.split(' ', QString::SkipEmptyParts);
          while (MOList.size() > 1) {
            for (int i = 0; i < MOList.size(); i++) 
              m_MOcoeffs.push_back(MOList.at(i).toDouble()); 
            line = m_in.readLine();
            list = line.split(' ', QString::SkipEmptyParts);
            MOLine = line.mid(32);
            MOList = MOLine.split(' ', QString::SkipEmptyParts);
          }
          line = m_in.readLine();
          list = line.split(' ', QString::SkipEmptyParts);
        }
        reorderD5Orbitals();
        basis->addMOs(m_MOcoeffs);
      }                
    } 
  }

  Molpro::~Molpro()
  {
  }

/* MOLPRO prints out spherical (D5) d-orbitals MO coefficients in the order d0,
 * d2-, d1+, d2+, d1-.  However, Avogadro expects them in the order d0, d1+,
 * d1-, d2+, d2- (as they are printed out e.g. by Gaussian03).  Thus, we need
 * to reorder the MO coefficients for D5 orbitals.
 */
  void Molpro::reorderD5Orbitals()
  {
   
    int GTOOffset = 0;
    /* FIXME: This should be the number of MOs, not basis functions */
    int numMOs = m_numBasisFunctions;

    for (unsigned int i=0; i < m_shellTypes.size(); ++i) {
      if (m_shellTypes.at(i) == -2) {
	/* D5 orbital. */
	/* FIXME: This should go over the number of MOs, not basis functions */
	for (unsigned int j=1; j < m_numBasisFunctions; ++j) {
	  /* set temp to MOLPRO's d2- */
	  double temp = m_MOcoeffs[j*numMOs+GTOOffset+1];
	  /* set second coefficient to MOLPRO's d1+ */
	  m_MOcoeffs[j*numMOs+GTOOffset+1] = m_MOcoeffs[j*numMOs+GTOOffset+2];
	  /* set third coefficient to MOLPRO's d1- */
	  m_MOcoeffs[j*numMOs+GTOOffset+2] = m_MOcoeffs[j*numMOs+GTOOffset+4];
	  /* set fifth coefficient to temp */
	  m_MOcoeffs[j*numMOs+GTOOffset+4] = temp;
	}
      }
      switch (m_shellTypes.at(i)) {
        case 0:
          GTOOffset += 1;   /* S */
          break;
        case 1:
          GTOOffset += 3;   /* P */
          break;
        case 2:
          GTOOffset += 6;   /* D */
          break;
        case -2:
          GTOOffset += 5;   /* D5 */
          break;
      }
    }
  }

  void Molpro::outputAll()
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
