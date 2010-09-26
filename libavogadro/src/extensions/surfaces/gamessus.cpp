/**********************************************************************
  GAMESS-US - parses GAMESS-US files (for MOs)

  Copyright (C) 2010 Geoffrey R. Hutchison

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

#include "gamessus.h"
#include "qtiocompressor/qtiocompressor.h"

#include <QFile>
#include <QStringList>
#include <QDebug>

using Eigen::Vector3d;
using std::vector;

#ifndef BOHR_TO_ANGSTROM
#define BOHR_TO_ANGSTROM 0.529177249
#endif

namespace Avogadro
{

  GAMESSUSOutput::GAMESSUSOutput(const QString &filename, GaussianSet* basis):
    m_coordFactor(1.0), m_currentMode(NotParsing), m_currentAtom(1)
  {
    // Open the file for reading and process it
    QFile* file = new QFile(filename);
    QtIOCompressor* compressedFile = 0;

    if (filename.endsWith(".gz")) {
      compressedFile = new QtIOCompressor(file);
      compressedFile->setStreamFormat(QtIOCompressor::GzipFormat);
      compressedFile->open(QIODevice::ReadOnly);
      m_in = compressedFile;
    }
    else {
      file->open(QIODevice::ReadOnly | QIODevice::Text);
      m_in = file;
    }

    qDebug() << "File" << filename << "opened.";

    // Process the formatted checkpoint and extract all the information we need
    while (!m_in->atEnd()) {
      processLine();
    }

    // Now it should all be loaded load it into the basis set
    load(basis);

    delete file;
  }

  GAMESSUSOutput::~GAMESSUSOutput()
  {
  }

  void GAMESSUSOutput::processLine()
  {
    // First truncate the line, remove trailing white space and check for blank lines
    QString key = m_in->readLine().trimmed();
    while(key.isEmpty() && !m_in->atEnd()) {
      key = m_in->readLine().trimmed();
    }

    if (m_in->atEnd())
      return;

    QStringList list = key.split(' ', QString::SkipEmptyParts);
    int numGTOs;

    // Big switch statement checking for various things we are interested in
    // Make sure to switch mode:
    //      enum mode { NotParsing, Atoms, GTO, STO, MO, SCF }
    if (key.contains("COORDINATES (BOHR)", Qt::CaseInsensitive)) {
      m_aNums.clear();
      m_aPos.clear();

      m_coordFactor = 1.0; // coordinates are supposed to be in bohr?!
      m_currentMode = Atoms;
      key = m_in->readLine().trimmed(); // skip the column titles
    } else if (key.contains("COORDINATES OF ALL ATOMS ARE (ANGS)", Qt::CaseInsensitive)) {
      m_aNums.clear();
      m_aPos.clear();

      m_coordFactor = 1.0/BOHR_TO_ANGSTROM; // in Angstroms now
      m_currentMode = Atoms;
      key = m_in->readLine(); // skip column titles
      key = m_in->readLine(); // and ----- line
    } else if (key.contains("ATOMIC BASIS SET")) {
      m_currentMode = GTO;
      // ---
      // PRIMITIVE
      // BASIS FUNC
      // blank
      // column header
      // blank
      // element
      for (unsigned int i = 0; i < 7; ++i) {
        key = m_in->readLine();
      }
    } else if (key.contains("TOTAL NUMBER OF BASIS SET")) {
      m_currentMode = NotParsing; // no longer reading GTOs
    } else if (key.contains("NUMBER OF CARTESIAN GAUSSIAN BASIS")) {
      m_currentMode = NotParsing; // no longer reading GTOs
    } else if (key.contains("NUMBER OF ELECTRONS")) {
      m_electrons = list[4].toInt();
    } else if (key.contains("EIGENVECTORS")) { //|| key.contains("MOLECULAR ORBITALS")) {
      m_currentMode = MO;
      key = m_in->readLine(); // ----
      key = m_in->readLine(); // blank line
    } else {
      QString shell;
      orbital shellType;
      vector <vector <double> > columns;
      unsigned int numColumns, numRows;

      // parsing a line -- what mode are we in?
      switch (m_currentMode) {
      case Atoms:
        // element_name atomic_number x y z
        if (list.size() < 5)
          return;
        m_aNums.push_back((int)list[1].toDouble());
        m_aPos.push_back(list[2].toDouble() * m_coordFactor);
        m_aPos.push_back(list[3].toDouble() * m_coordFactor);
        m_aPos.push_back(list[4].toDouble() * m_coordFactor);

        break;
      case GTO:
        // should start at the first line of shell functions
        if (key.isEmpty())
          break;
        list = key.split(' ', QString::SkipEmptyParts);
        numGTOs = 0;
        while (list.size() > 1) {
          numGTOs++;
          shell = list[1].toLower();
          shellType = UU;
          if (shell.contains("s"))
            shellType = S;
          else if (shell.contains("l"))
            shellType = SP;
          else if (shell.contains("p"))
            shellType = P;
          else if (shell.contains("d"))
            shellType = D;
          else if (shell.contains("f"))
            shellType = F;
          else
            return;

          m_a.push_back(list[3].toDouble());
          m_c.push_back(list[4].toDouble());
          if (shellType == SP && list.size() > 4)
            m_csp.push_back(list[5].toDouble());

          // read to the next shell
          key = m_in->readLine().trimmed();
          if (key.isEmpty()) {
            key = m_in->readLine().trimmed();
            m_shellNums.push_back(numGTOs);
            m_shellTypes.push_back(shellType);
            m_shelltoAtom.push_back(m_currentAtom);
            numGTOs = 0;
          }
          list = key.split(' ', QString::SkipEmptyParts);
        } // end "while list > 1) -- i.e., we're on the next atom line

        key = m_in->readLine(); // start reading the next atom
        m_currentAtom++;
        break;
      
    case MO:
      m_MOcoeffs.clear(); // if the orbitals were punched multiple times
      while(!key.contains("END OF") && !key.contains("-----")) {
        // currently reading the MO number
        key = m_in->readLine(); // energies
        key = m_in->readLine(); // symmetries
        key = m_in->readLine(); // now we've got coefficients
        list = key.split(' ', QString::SkipEmptyParts);
        while (list.size() > 5) {
          numColumns = list.size() - 4;
          columns.resize(numColumns);
          for (unsigned int i = 0; i < numColumns; ++i) {
            columns[i].push_back(list[i + 4].toDouble());
          }
          
          key = m_in->readLine();
          if (key.contains(QLatin1String("END OF RHF")))
            break;
          list = key.split(' ', QString::SkipEmptyParts);          
        } // ok, we've finished one batch of MO coeffs
        
        // Now we need to re-order the MO coeffs, so we insert one MO at a time
        for (unsigned int i = 0; i < numColumns; ++i) {
          numRows = columns[i].size();
          for (unsigned int j = 0; j < numRows; ++j) {
            qDebug() << "push back" << columns[i][j];
            m_MOcoeffs.push_back(columns[i][j]);
          }
        }
        columns.clear();

        if (key.trimmed().isEmpty())
          key = m_in->readLine(); // skip the blank line after the MOs
      } // finished parsing MOs
      m_currentMode = NotParsing;
      break;

    default:
      ;
      } // end switch
    } // end if (mode)
  } // end process line

  void GAMESSUSOutput::load(GaussianSet* basis)
  {
    // Now load up our basis set
    basis->setNumElectrons(m_electrons);

    int nAtom = 0;
    for (unsigned int i = 0; i < m_aPos.size(); i += 3)
      basis->addAtom(Vector3d(m_aPos.at(i), m_aPos.at(i+1), m_aPos.at(i+2)),
                     m_aNums.at(nAtom++));

    //    qDebug() << m_shellTypes.size() << m_shellNums.size() << m_shelltoAtom.size() << m_a.size() << m_c.size() << m_csp.size();

    // Set up the GTO primitive counter, go through the shells and add them
    int nGTO = 0;
    int nSP = 0; // number of SP shells
    for (unsigned int i = 0; i < m_shellTypes.size(); ++i) {
      // Handle the SP case separately - this should possibly be a distinct type
      if (m_shellTypes.at(i) == SP)  {
        // SP orbital type - currently have to unroll into two shells
        int tmpGTO = nGTO;
        int s = basis->addBasis(m_shelltoAtom.at(i) - 1, S);
        for (int j = 0; j < m_shellNums.at(i); ++j) {
          basis->addGTO(s, m_c.at(nGTO), m_a.at(nGTO));
          ++nGTO;
        }
        int p = basis->addBasis(m_shelltoAtom.at(i) - 1, P);
        for (int j = 0; j < m_shellNums.at(i); ++j) {
          basis->addGTO(p, m_csp.at(nSP), m_a.at(tmpGTO));
          ++tmpGTO;
          ++nSP;
        }
      }
      else {
        int b = basis->addBasis(m_shelltoAtom.at(i) - 1, m_shellTypes.at(i));
        for (int j = 0; j < m_shellNums.at(i); ++j) {
          basis->addGTO(b, m_c.at(nGTO), m_a.at(nGTO));
          ++nGTO;
        }
      }
    }
    //    qDebug() << " loading MOs " << m_MOcoeffs.size();

    // Now to load in the MO coefficients
    if (m_MOcoeffs.size())
      basis->addMOs(m_MOcoeffs);

    qDebug() << " done loadBasis ";
  }

  void GAMESSUSOutput::outputAll()
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
