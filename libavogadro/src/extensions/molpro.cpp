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
        QStringList list = line.split(" ", QString::SkipEmptyParts);
        while (list.size() == 6) {
          aNum = list.at(2);
          aNum.chop(3); // get rid of the ".00"
          basis->addAtom(Vector3d(list.at(3).toDouble(),
                                  list.at(4).toDouble(),
                                  list.at(5).toDouble()), aNum.toInt()); 
          line = m_in.readLine();
          list = line.split(" ", QString::SkipEmptyParts);
        } 
      }
      else if (line.indexOf( "NUMBER OF ELECTRONS" ) != -1 ) {
        QStringList list = line.split(" ", QString::SkipEmptyParts);
        QString alpha = list.at(3);
        QString beta = list.at(4);
        // get rid of the +/- signs after the values
        alpha.chop(1);
        beta.chop(1);
        basis->setElectrons(alpha.toInt() + beta.toInt());
      }
      else if (line.indexOf( "NUMBER OF CONTRACTIONS" ) != -1 ) {
        QStringList list = line.split(" ", QString::SkipEmptyParts);
        m_numBasisFunctions = list.at(3).toInt();
      } 
      else if (line.indexOf( "Nr Sym  Nuc  Type" ) != -1 ) {
        QString type;
        int currentBasis = 0;
        int ignoreShells = 0;
        line = m_in.readLine(); // blank
        line = m_in.readLine();
        QStringList list = line.split(" ", QString::SkipEmptyParts);
        while (list.size() > 1) {
          if (ignoreShells > 0) {
            ignoreShells--;
            line = m_in.readLine();
            list = line.split(" ", QString::SkipEmptyParts);
            while (list.size() == 2) {
              line = m_in.readLine();
              list = line.split(" ", QString::SkipEmptyParts);
            }
            continue;
          }
          type = list.at(3);
          type.remove(QRegExp("[xyz,0-9]"));
          if (type == "s" ) {
            currentBasis = basis->addBasis(list.at(2).toInt()-1, S);
          } else if (type == "p" ) {
            ignoreShells = 2;
            currentBasis = basis->addBasis(list.at(2).toInt()-1, P);
          } else if (type == "d" ) {
            ignoreShells = 4;
            currentBasis = basis->addBasis(list.at(2).toInt()-1, D);
          } else return;
          basis->addGTO(currentBasis, list.at(5).toDouble(),
                        list.at(4).toDouble());
          line = m_in.readLine();
          list = line.split(" ", QString::SkipEmptyParts);
          while (list.size() == 2) {
            // new contraction
            basis->addGTO(currentBasis, list.at(1).toDouble(),
                          list.at(0).toDouble());
            line = m_in.readLine();
            list = line.split(" ", QString::SkipEmptyParts);
          }
        }
      } 
      else if (line.indexOf( "Orb  Occ    Energy") != -1 ) {
        line = m_in.readLine(); // blank
        line = m_in.readLine(); // column header start
        QStringList list = line.split(" ", QString::SkipEmptyParts);
        while (list.size() > 1) {
          // skip column header
          line = m_in.readLine();
          list = line.split(" ", QString::SkipEmptyParts);
        }
        line = m_in.readLine();
        list = line.split(" ", QString::SkipEmptyParts);
        while (list.size() > 1) {
          // new MO
          m_orbitalEnergy.push_back(list.at(2).toDouble());
          QStringList MOList;
          QString MOLine = line.mid(32);
          MOList = MOLine.split(" ", QString::SkipEmptyParts);
          while (MOList.size() > 1) {
            for (int i = 0; i < MOList.size(); i++) 
              m_MOcoeffs.push_back(MOList.at(i).toDouble()); 
            line = m_in.readLine();
            list = line.split(" ", QString::SkipEmptyParts);
            MOLine = line.mid(32);
            MOList = MOLine.split(" ", QString::SkipEmptyParts);
          }
          line = m_in.readLine();
          list = line.split(" ", QString::SkipEmptyParts);
        }
        basis->addMOs(m_MOcoeffs);
      }                
    } 
  }

  Molpro::~Molpro()
  {
  }
}
