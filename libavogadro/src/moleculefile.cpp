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
#include <openbabel/obconversion.h>

#include <iostream>

namespace Avogadro {

  using OpenBabel::OBConversion;
  using OpenBabel::OBFormat;
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
    OpenBabel::OBMol *obmol = OBMol(i);
    if (!obmol)
      return 0;

    Molecule *mol = new Molecule;
    mol->setOBMol(obmol);
    
    delete obmol;
    return mol;  
  }

  OpenBabel::OBMol* MoleculeFile::OBMol(unsigned int i)
  {
    if (!d->ready) 
      return 0;
    if (i >= d->streampos.size()) {
      m_error.append(tr("OBMol: index %1 out of reach.").arg(i));
      return 0;
    }

    // Construct the OpenBabel objects, set the file type
    OBConversion conv;
    OBFormat *inFormat;
    if (!m_fileType.isEmpty() && !conv.SetInFormat(m_fileType.toAscii())) {
      // Input format not supported
      m_error.append(tr("File type '%1' is not supported for reading.").arg(m_fileType));
      return 0;
    } else {
      inFormat = conv.FormatFromExt(m_fileName.toAscii());
      if (!conv.SetInFormat(inFormat)) {
        // Input format not supported
        m_error.append(tr("File type for file '%1' is not supported for reading.").arg(m_fileName));
        return 0;
      }
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

    if (i && m_fileName.endsWith(QLatin1String("xyz"), Qt::CaseInsensitive)) {
      ifs.unget();
    }

    OpenBabel::OBMol *obmol = new OpenBabel::OBMol;
    if (!conv.Read(obmol, &ifs) || !obmol->NumAtoms()) {
      m_error.append(tr("Reading molecule with index %1 from file '%2' failed.").arg(i).arg(m_fileName));
      return 0;
    }

    return obmol;
  }

  bool MoleculeFile::replaceMolecule(unsigned int i, Molecule *molecule)
  {
    if (!d->ready) 
      return false;
    if (i >= d->streampos.size()) {
      m_error.append(tr("replaceMolecule: index %1 out of reach.").arg(i));
      return false;
    }

    // Construct the OpenBabel objects, set the file type
    OBConversion conv;
    OBFormat *outFormat;
    if (!m_fileType.isEmpty() && !conv.SetOutFormat(m_fileType.toAscii())) {
      // Output format not supported
      m_error.append(tr("File type '%1' is not supported for writing.").arg(m_fileType));
      return false;
    } else {
      outFormat = conv.FormatFromExt(m_fileName.toAscii());
      if (!conv.SetOutFormat(outFormat)) {
        // Output format not supported
        m_error.append(tr("File type for file '%1' is not supported for writing.").arg(m_fileName));
        return false;
      }
    }

    // Now attempt to open the file.new for writing
    ofstream ofs;
    QString newFilename(QFile::encodeName(m_fileName) + QLatin1String(".new"));
    ofs.open(newFilename.toAscii().data()); // This handles utf8 file names etc
    if (!ofs) {
      m_error.append(tr("Could not open file '%1' for writing.").arg(m_fileName));
      return false;
    }
    // Copy molecules 0 to i-1 to .new file
    ifstream ifs;
    ifs.open(QFile::encodeName(m_fileName)); // This handles utf8 file names etc
    if (!ifs) {
      m_error.append(tr("Could not open file '%1' for reading.").arg(m_fileName));
      return false;
    }
    for (std::streampos pos = 0; pos < d->streampos.at(i); pos+=1)
      ofs.put(ifs.get()); // FIXME using istream_iterator or something

    // write the molecule
    OpenBabel::OBMol obmol = molecule->OBMol();
    if (!conv.Write(&obmol, &ofs)) {
      m_error.append(tr("Replacing molecule with index %1 in file '%2' failed.").arg(i).arg(m_fileName));
      return false;
    }

    // size of the molecule to be replaced (in chars)
    std::streampos oldSize = d->streampos[i+1] - d->streampos[i];
    std::streampos newSize = ofs.tellp() - d->streampos[i];
    std::streampos delta = newSize - oldSize; 

    // copy remaining molecules 
    ifs.seekg(0, std::ios::end);
    std::streampos endpos = ifs.tellg();
    ifs.seekg(d->streampos.at(i+1));
    while (ifs.tellg() < endpos)
      ofs.put(ifs.get()); // FIXME using istream_iterator or something
    /*
    std::copy(std::istream_iterator<char>(ifs),
              std::istream_iterator<char>(),
              std::ostream_iterator<char>(ofs));
    */
    ifs.close();
    ofs.close();

    QFile newFile(newFilename);
    QFile(m_fileName).remove();
    newFile.rename(m_fileName);


    // adjust the cached variables
    if (i+1 < d->streampos.size()) {
      for (unsigned int j = i+1; j < d->streampos.size(); ++j) {
        d->streampos[j] += delta;
      }
    }

    return true;
  }
  
  bool MoleculeFile::insertMolecule(unsigned int i, Molecule *molecule)
  {
    return false;
  }
  
  bool MoleculeFile::appendMolecule(Molecule *molecule)
  {
    return false;
  }
      
  const std::vector<std::vector<Eigen::Vector3d>*>& MoleculeFile::conformers() const
  {
    return m_conformers;
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
      
  void MoleculeFile::setConformerFile(bool value)
  {
    d->isConformerFile = value;
  }
  
  void MoleculeFile::setReady(bool value)
  {
    d->ready = value;
  }
 
  const QString& MoleculeFile::errors() const
  {
    return m_error;
  }

  void MoleculeFile::clearErrors()
  {
    m_error.clear();
  }

}

#include "moleculefile.moc"

