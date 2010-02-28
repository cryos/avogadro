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
#include <QFileInfo>
#include <QStringList>
#include <QThread>
#include <QDebug>
#include <QPointer>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

// Included in obconversion.h
//#include <iostream>

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
      MoleculeFilePrivate() : isConformerFile(false), ready(false), specialCaseOBMol(0) {}
      QStringList titles;
      std::vector<std::streampos> streampos;
      bool isConformerFile;
      bool ready;

      // special cases call OBConversion::ReadFile and save the the resulting
      // OBMol in specialCaseOBMol. MoleculeFile::molecule will return this 
      // OBMol object (if non 0) regardless of the index.
      OBMol *specialCaseOBMol;
  };

  MoleculeFile::MoleculeFile(const QString &fileName, const QString &fileType, 
      const QString &fileOptions) : QObject(), d(new MoleculeFilePrivate), 
      m_fileName(fileName), m_fileType(fileType), m_fileOptions(fileOptions)
  {
  }

  MoleculeFile::~MoleculeFile() 
  {
    if (d->specialCaseOBMol)
      delete d->specialCaseOBMol;
    delete d;
  }
    
  bool MoleculeFile::isReady() const
  {
    return d->ready;
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
      
  QStringList MoleculeFile::titles() const
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

    // return a copy of the specialCaseOBMol if it is set
    if (d->specialCaseOBMol)
      return (new OpenBabel::OBMol(*d->specialCaseOBMol));

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
      if (!inFormat || !conv.SetInFormat(inFormat)) {
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

  bool MoleculeFile::replaceMolecule(unsigned int i, Molecule *molecule, QString fileName)
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
      if (!outFormat || !conv.SetOutFormat(outFormat)) {
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

    std::streampos newSize = ofs.tellp() - d->streampos[i];
    if (i+1 < d->streampos.size()) {
      // copy remaining molecules 
      ifs.seekg(0, std::ios::end);
      std::streampos endpos = ifs.tellg();
      ifs.seekg(d->streampos.at(i+1));
      while (ifs.tellg() < endpos)
        ofs.put(ifs.get()); // FIXME using istream_iterator or something
    }
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
      // size of the molecule to be replaced (in chars)
      std::streampos oldSize = d->streampos[i+1] - d->streampos[i];
      std::streampos delta = newSize - oldSize; 

      for (unsigned int j = i+1; j < d->streampos.size(); ++j) {
        d->streampos[j] += delta;
      }
    }

    return true;
  }
  
  bool MoleculeFile::insertMolecule(unsigned int, Molecule *, QString)
  {
    return false;
  }
  
  bool MoleculeFile::appendMolecule(Molecule *, QString)
  {
    return false;
  }

  void MoleculeFile::threadFinished()
  {
    d->ready = true;
    emit ready();
  }

  std::vector<std::streampos>& MoleculeFile::streamposRef()
  {
    return d->streampos;
  }

  QStringList& MoleculeFile::titlesRef()
  {
    return d->titles;
  }

  const std::vector<std::vector<Eigen::Vector3d>*>& MoleculeFile::conformers() const
  {
    return m_conformers;
  }

  std::vector<std::vector<Eigen::Vector3d>*>& MoleculeFile::conformersRef()
  {
    return m_conformers;
  }

  void MoleculeFile::setConformerFile(bool value)
  {
    d->isConformerFile = value;
  }
  
  void MoleculeFile::setReady(bool value)
  {
    d->ready = value;
  }
 
  void MoleculeFile::setFirstReady(bool value)
  {
    if (value && !d->ready)
      emit firstMolReady();
  }

  const QString& MoleculeFile::errors() const
  {
    return m_error;
  }

  void MoleculeFile::clearErrors()
  {
    m_error.clear();
  }
    
  bool MoleculeFile::canOpen(const QString &fileName, QIODevice::OpenMode mode)
  {
    // Check that the file can be opened in mode
    QFile file(fileName);
    if (!file.open(mode))
      // Cannot open the file in mode
      return false;
    file.close();

    // WriteOnly, ReadWrite: also check to make sure we can open fileName.new 
    if (mode & QIODevice::WriteOnly) {
      QString newFileName(fileName + ".new");
      QFile newFile(newFileName);
      if (!newFile.open(QFile::WriteOnly | QFile::Text))
        // Cannot write to the file
        return false;
      newFile.close();
    }
    
    return true;
  }

  Molecule * MoleculeFile::readMolecule(const QString &fileName,
      const QString &fileType, const QString &fileOptions, QString *error)
  {
    // Check that the file can be read from disk
    if (!canOpen(fileName, QFile::ReadOnly | QFile::Text)) {
      if (error)
        error->append(QObject::tr("File %1 cannot be opened for reading.").arg(fileName));
      return 0;
    }

    // Construct the OpenBabel objects, set the file type
    OBConversion conv;
    OBFormat *inFormat;
    if (!fileType.isEmpty() && !conv.SetInFormat(fileType.toAscii().data())) {
      // Input format not supported
      if (error)
        error->append(QObject::tr("File type '%1' is not supported for reading.").arg(fileType));
      return 0;
    } else {
      inFormat = conv.FormatFromExt(fileName.toAscii().data());
      if (!inFormat || !conv.SetInFormat(inFormat)) {
        // Input format not supported
        if (error)
          error->append(QObject::tr("File type for file '%1' is not supported for reading.").arg(fileName));
        return 0;
      }
    }

    // set any options
    if (!fileOptions.isEmpty()) {
      foreach(const QString &option,
              fileOptions.split('\n', QString::SkipEmptyParts)) {
        conv.AddOption(option.toAscii().data(), OBConversion::INOPTIONS);
      }
    }

    // Now attempt to read the molecule in
    ifstream ifs;
    ifs.open(fileName.toLocal8Bit()); // This handles utf8 file names etc
    if (!ifs) // Should not happen, already checked file could be opened
      return 0;
    OpenBabel::OBMol *obMol = new OpenBabel::OBMol;
    if (conv.Read(obMol, &ifs)) {
      Molecule *mol = new Molecule;
      mol->setOBMol(obMol);
      mol->setFileName(fileName);
      return mol;
    } else {
      if (error)
        error->append(QObject::tr("Reading a molecule from file '%1' failed.").arg(fileName));
      return 0;
    }
  }

  bool MoleculeFile::writeMolecule(const Molecule *molecule,
      const QString &fileName, const QString &fileType, QString *error)
  {
    // Check is we are replacing an existing file
    QFile file(fileName);
    bool replaceExistingFile = file.exists();
    // Check that the file can be written to disk
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
      // Cannot write to the file
      if (error) {
        error->append(QObject::tr("File %1 can not be opened for writing.")
                      .arg(fileName));
      }
      return false;
    }
    file.close();

    QString newFileName = fileName;
    if (replaceExistingFile) {
      newFileName += ".new";
      QFile newFile(newFileName);
      if (!newFile.open(QFile::WriteOnly | QFile::Text)) {
        // Cannot write to the temporary file location
        if (error) {
          error->append(QObject::tr("File %1 can not be opened for writing.")
                        .arg(newFileName));
        }
        return false;
      }
      newFile.close();
    }

    // Construct the OpenBabel objects, set the file type
    OBConversion conv;
    OBFormat *outFormat;
    if (!fileType.isEmpty() && !conv.SetOutFormat(fileType.toAscii())) {
      // Output format not supported
      if (error)
        error->append(QObject::tr("File type '%1' is not supported for writing.")
                      .arg(fileType));
      return false;
    }
    else {
      outFormat = conv.FormatFromExt(fileName.toAscii());
      if (!outFormat || !conv.SetOutFormat(outFormat)) {
        // Output format not supported
        if (error)
          error->append(QObject::tr("File type for file '%1' is not supported for writing.").arg(fileName));
        return false;
      }
    }

    // Now attempt to write the molecule in
    ofstream ofs;
    ofs.open(newFileName.toLocal8Bit()); // This handles utf8 file names etc
    if (!ofs) {// Should not happen, already checked file could be opened
      qDebug() << "ofs is bad";
      return false;
    }
    OpenBabel::OBMol obmol = molecule->OBMol();
      
    OpenBabel::OBChainsParser chainparser;
    obmol.UnsetFlag(OB_CHAINS_MOL);
    chainparser.PerceiveChains(obmol);
 
    if (conv.Write(&obmol, &ofs)) {
      ofs.close();
      if (replaceExistingFile) {
        QFile newFile(newFileName);
        bool success;
        success = file.rename(fileName + ".old");
        if (success) {
          // Leave to ensure we work around a bug in Qt < 4.5.1
          file.setFileName(fileName + ".old");
          success = newFile.rename(fileName);
        }
        else {
          if (error)
              error->append(QObject::tr("Saving molecular file failed - could not rename original file."));
          return false;
        }
        if (success) // renaming worked
          success = file.remove(); // remove the old file: WARNING -- would much prefer to just rename, but Qt won't let you
        else {
          if (error)
            error->append(QObject::tr("Saving molecular file failed - could not rename new file."));
          return false;
        }

        if (success) {
          return true;
        }
        else {
          if (error)
            error->append(QObject::tr("Saving molecular file failed - could not remove old file."));
          return false;
        }
      }
      else // No need for all that - this is a new file in an empty location
        return true;
    }
    else {
      if (error)
        error->append(QObject::tr("Writing a molecule to file '%1' failed. OpenBabel function failed.").arg(fileName));
      return false;
    }
    // Assume something went wrong if we did not return true earlier
    qDebug() << "OBWrapper should never get here...";
    return false;
  }

  bool MoleculeFile::writeConformers(const Molecule *molecule,
                                         const QString &fileName,
                                         const QString &fileType,
                                         QString *error)
  {
    // Check that the file can be written to disk
    if (!canOpen(fileName, QFile::WriteOnly | QFile::Text)) {
      // Cannot write to the file
      if (error)
        error->append(QObject::tr("File %1 cannot be opened for writing.").arg(fileName));
      return false;
    }
    
    QString newFileName(fileName + ".new");
    QFile newFile(newFileName);

    // Construct the OpenBabel objects, set the file type
    OBConversion conv;
    OBFormat *outFormat;
    if (!fileType.isEmpty() && !conv.SetOutFormat(fileType.toAscii())) {
      // Output format not supported
      if (error)
        error->append(QObject::tr("File type '%1' is not supported for writing.").arg(fileType));
      return false;
    } else {
      outFormat = conv.FormatFromExt(fileName.toAscii());
      if (!outFormat || !conv.SetOutFormat(outFormat)) {
        // Output format not supported
        if (error)
          error->append(QObject::tr("File type for file '%1' is not supported for writing.").arg(fileName));
        return false;
      }
    }

    // Now attempt to write the molecule in
    ofstream ofs;
    ofs.open(newFileName.toLocal8Bit()); // This handles utf8 file names etc
    if (!ofs) // Should not happen, already checked file could be opened
      return false;
    
    bool success = false;
    const std::vector<std::vector<Eigen::Vector3d>*> &conformers = molecule->conformers();
    OpenBabel::OBMol obMol = molecule->OBMol();
    for (unsigned int i = 0; i < conformers.size(); ++i) {
      OpenBabel::OBAtomIterator ai;
      for (OpenBabel::OBAtom *atom = obMol.BeginAtom(ai); atom; atom = obMol.NextAtom(ai))
        atom->SetVector(conformers.at(i)->at(atom->GetIdx()-1).data());
      success = conv.Write(&obMol, &ofs);
      if (!success)
        break;
      if (fileName.endsWith(QLatin1String("xyz"), Qt::CaseInsensitive))
        ofs << std::endl;
    }

    if (success) {
      QFile(fileName).remove();
      newFile.rename(fileName);
      return true;
    } 
 
    if (error)
      error->append(QObject::tr("Writing conformers to file '%1' failed.").arg(fileName));
      
    newFile.remove();
    return false;
  }

  class ReadFileThread : public QThread
  {
    //    Q_OBJECT 

    public:
      ReadFileThread(MoleculeFile *moleculeFile) : m_moleculeFile(moleculeFile)
      {
      }
      
      void addConformer(const OpenBabel::OBMol &conformer)
      {
        unsigned int numAtoms = conformer.NumAtoms();
        std::vector<Eigen::Vector3d> *coords = new std::vector<Eigen::Vector3d>(numAtoms);
        for (unsigned int i = 0; i < numAtoms; ++i)
          coords->push_back(Eigen::Vector3d(conformer.GetAtom(i+1)->GetVector().AsArray()));
        m_moleculeFile->m_conformers.push_back(coords);
      }

      void detectConformers(unsigned int c, const OpenBabel::OBMol &first, const OpenBabel::OBMol &current)
      {
        if (!c) {
          // this is the first molecule read
          m_moleculeFile->setConformerFile(true);
          addConformer(current);
          return;
        }

        if (!m_moleculeFile->isConformerFile())
          return;

        // as long as we are not sure if this really is a 
        // conformer/trajectory file, add the conformers
        addConformer(current);

        // performance: check only certain molecule 1-10,20,50
        switch (c) {
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
          case 8:
          case 9:
          case 10:
          case 20:
          case 50:
            break;
          default:
            return;
        }

        if (first.NumAtoms() != current.NumAtoms()) {
          m_moleculeFile->setConformerFile(false);
          m_moleculeFile->m_conformers.clear();
          return;
        }

        for (unsigned int i = 0; i < first.NumAtoms(); ++i) {
          OpenBabel::OBAtom *firstAtom = first.GetAtom(i+1);
          OpenBabel::OBAtom *currentAtom = current.GetAtom(i+1);
          if (firstAtom->GetAtomicNum() != currentAtom->GetAtomicNum()) {
            m_moleculeFile->setConformerFile(false);
            m_moleculeFile->m_conformers.clear();
            return;
          }    
        }
      }

      void run()
      {
        // Check that the file can be read from disk
        if (!MoleculeFile::canOpen(m_moleculeFile->m_fileName, QFile::ReadOnly | QFile::Text)) {
          // Cannot read the file
          m_moleculeFile->m_error.append(QObject::tr("File %1 cannot be opened for reading.")
                                         .arg(m_moleculeFile->m_fileName));
          return;
        }
 
        // Construct the OpenBabel objects, set the file type
        OpenBabel::OBConversion conv;
        OpenBabel::OBFormat *inFormat;
        if (!m_moleculeFile->m_fileType.isEmpty() && !conv.SetInFormat(m_moleculeFile->m_fileType.toAscii().data())) {
          // Input format not supported
          m_moleculeFile->m_error.append(
              QObject::tr("File type '%1' is not supported for reading.").arg(m_moleculeFile->m_fileType));
          return;
        } else {
          inFormat = conv.FormatFromExt(m_moleculeFile->m_fileName.toAscii().data());
          if (!inFormat || !conv.SetInFormat(inFormat)) {
            // Input format not supported
            m_moleculeFile->m_error.append(QObject::tr("File type for file '%1' is not supported for reading.")
                                           .arg(m_moleculeFile->m_fileName));
            return;
          }
        }

        // set any options
        if (!m_moleculeFile->m_fileOptions.isEmpty()) {
          foreach(const QString &option,
              m_moleculeFile->m_fileOptions.split('\n', QString::SkipEmptyParts)) {
            conv.AddOption(option.toAscii().data(), OBConversion::INOPTIONS);
          }
        }

        // Now attempt to read the molecule in
        ifstream ifs;
        ifs.open(m_moleculeFile->m_fileName.toLocal8Bit()); // This handles utf8 file names etc
        if (!ifs) // Should not happen, already checked file could be opened
          return;
      
        // read all molecules
        OpenBabel::OBMol firstOBMol, currentOBMol;
        unsigned int c = 0;
        conv.SetInStream(&ifs);
        m_moleculeFile->streamposRef().push_back(ifs.tellg());
        while (ifs.good() && conv.Read(&currentOBMol)) {
          if (!c)
            firstOBMol = currentOBMol;

          if (c > 20 && !m_moleculeFile->isConformerFile())
            m_moleculeFile->setFirstReady(true);

          // detect conformer/trajectory files
          detectConformers(c, firstOBMol, currentOBMol);
          // store information about molecule
          m_moleculeFile->streamposRef().push_back(ifs.tellg());
          m_moleculeFile->titlesRef().append(currentOBMol.GetTitle());
          // increment count
          ++c;
        }
        m_moleculeFile->streamposRef().pop_back();

        // signle molecule files are not conformer files
        if (c == 1) {
          m_moleculeFile->setConformerFile(false);
          m_moleculeFile->m_conformers.clear();
        }

        // check for empty titles
        for (int i = 0; i < m_moleculeFile->titlesRef().size(); ++i) {
          if (!m_moleculeFile->titlesRef()[i].isEmpty())
            continue;

          QString title;
          if (m_moleculeFile->isConformerFile())
            title = tr("Conformer %1").arg(i+1);
          else
            title = tr("Molecule %1").arg(i+1);
        
          m_moleculeFile->titlesRef()[i] = title;
        }
      }

      MoleculeFile *m_moleculeFile;
  }; // end ReadFileThread class

  MoleculeFile* MoleculeFile::readFile(const QString &fileName,
      const QString &fileType, const QString &fileOptions, bool wait)
  {
    QPointer<MoleculeFile> moleculeFile = new MoleculeFile(fileName, fileType, fileOptions);

    QFileInfo qfile(fileName);
    // handle VASP files by calling OBConversion::ReadFile
    if (qfile.baseName() == "POSCAR" || qfile.baseName() == "CONTCAR") {
      // Check that the file can be read from disk
      if (!MoleculeFile::canOpen(fileName, QFile::ReadOnly | QFile::Text)) {
        // Cannot read the file
        moleculeFile->m_error.append(
            QObject::tr("File %1 cannot be opened for reading.").arg(fileName));
        moleculeFile->setReady(true);
        moleculeFile->threadFinished(); // set & emit ready
      }

      // Construct the OpenBabel objects, set the file type
      OpenBabel::OBConversion conv;
      // OBFormats are "POSCAR" and "CONTCAR"
      if (!conv.SetInFormat(qfile.baseName().toAscii().data())) {
        // Input format not supported
        moleculeFile->m_error.append(
            QObject::tr("File type '%1' is not supported for reading.").arg(qfile.baseName()));
        moleculeFile->setReady(true);
        moleculeFile->threadFinished(); // set & emit ready
      } 
      
      moleculeFile->setConformerFile(false);
      // Now attempt to read the molecule in
      moleculeFile->d->specialCaseOBMol = new OpenBabel::OBMol;
      if (conv.ReadFile(moleculeFile->d->specialCaseOBMol, fileName.toLocal8Bit().data())) {
        moleculeFile->titlesRef().push_back(tr("Molecule %1").arg(1));
      } else {
        delete moleculeFile->d->specialCaseOBMol;
        moleculeFile->d->specialCaseOBMol = 0;

        moleculeFile->m_error.append(
            QObject::tr("Reading a molecule from file '%1' failed.").arg(fileName));
      }
      
      moleculeFile->threadFinished(); // set & emit ready
      return moleculeFile;
    } // handle VASP files

    ReadFileThread *thread = new ReadFileThread(moleculeFile);
    QObject::connect(thread, SIGNAL(finished()), moleculeFile, SLOT(threadFinished()));
    thread->start();

    if (wait) {
      thread->wait();
      moleculeFile->setReady(true);
    }

    return moleculeFile;
  }

} // end namespace Avogadro

#include "moleculefile.moc"

