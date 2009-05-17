/**********************************************************************
  MoleculeFile - Class representing molecule file.

  Copyright (C) 2009 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "moleculefile.h"

#include <avogadro/molecule.h>

#include <QFile>
#include <QStringList>
#include <QThread>
#include <QDebug>
#include <QPointer>

#include <openbabel/mol.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <iostream>

namespace Avogadro {

  using OpenBabel::OBConversion;
  using OpenBabel::OBFormat;
  using OpenBabel::OBMol;
  using OpenBabel::OBAtom;
  using OpenBabel::OBAtomIterator;

  using std::ifstream;
  using std::ofstream;

  class MoleculeFilePrivate
  {
    public:
      MoleculeFilePrivate() : isConformerFile(false), ready(false) {}
      QStringList titles;
      std::vector<std::streampos> streampos;
      std::vector<std::vector<Eigen::Vector3d>*> conformers;
      bool isConformerFile;
      bool ready;
  };

  MoleculeFile::MoleculeFile(const QString &fileName, const QString &fileType, 
      const QString &fileOptions) : QObject(), d(new MoleculeFilePrivate), 
      m_fileName(fileName), m_fileType(fileType), m_fileOptions(fileOptions)
  {
  }

  MoleculeFile::~MoleculeFile() 
  {
    delete d;
  }

  bool MoleculeFile::isConformerFile() const
  {
    if (!d->ready) 
      return false;
    return d->isConformerFile;
  }
 
  unsigned int MoleculeFile::numMolecules() const
  {
    if (!d->ready) 
      return 0;
    if (d->isConformerFile)
      return 1;
    return d->titles.size();
  }
      
  QList<QString> MoleculeFile::titles() const
  {
    if (!d->ready) 
      return QStringList();
    return d->titles;
  }

  Molecule* MoleculeFile::molecule(unsigned int i)
  {
    if (!d->ready) 
      return 0;
    if (i >= d->streampos.size())
      return 0;

    // Construct the OpenBabel objects, set the file type
    OBConversion conv;
    OBFormat *inFormat;
    if (!m_fileType.isEmpty() && !conv.SetInFormat(m_fileType.toAscii().data()))
      // Input format not supported
      return 0;
    else {
      inFormat = conv.FormatFromExt(m_fileName.toAscii().data());
      if (!conv.SetInFormat(inFormat))
        // Input format not supported
        return 0;
    }

    // set any options
    if (!m_fileOptions.isEmpty()) {
      foreach(const QString &option,
          m_fileOptions.split('\n', QString::SkipEmptyParts)) {
        conv.AddOption(option.toAscii().data(), OBConversion::INOPTIONS);
      }
    }

    // Now attempt to read the molecule in
    ifstream ifs;
    ifs.open(QFile::encodeName(m_fileName)); // This handles utf8 file names etc
    ifs.seekg(d->streampos.at(i));
    if (!ifs) // Should not happen, already checked file could be opened
      return 0;

    if (i && m_fileName.endsWith("xyz", Qt::CaseInsensitive)) {
      ifs.unget();
    }
 

    OpenBabel::OBMol obmol;
    conv.Read(&obmol, &ifs);
    if (!obmol.NumAtoms())
      return 0;

    QPointer<Molecule> mol = new Molecule;
    mol->setOBMol(&obmol);

    return mol;  
  }
      
  const std::vector<std::vector<Eigen::Vector3d>*>& MoleculeFile::conformers() const
  {
    return d->conformers;
  }

  void MoleculeFile::threadFinished()
  {
    d->ready = true;
    emit ready();
  }
 
  QStringList& MoleculeFile::titles()
  {
    return d->titles;
  }
  
  std::vector<std::streampos>& MoleculeFile::streampos()
  {
    return d->streampos;
  }
      
  std::vector<std::vector<Eigen::Vector3d>*>& MoleculeFile::conformers()
  {
    return d->conformers;
  }

  void MoleculeFile::setConformerFile(bool value)
  {
    d->isConformerFile = value;
  }
  
  void MoleculeFile::setReady(bool value)
  {
    d->ready = value;
  }
 
}

#include "moleculefile.moc"






