/**********************************************************************
  OpenbabelWrapper - Wrapper around Openbabel calls

  Copyright (C) 2009 Marcus D. Hanwell

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

#include "openbabelwrapper.h"

#include <avogadro/molecule.h>
#include <avogadro/moleculefile.h>

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
  using OpenBabel::OBMol;
  using OpenBabel::OBAtom;
  using OpenBabel::OBAtomIterator;

  using std::ifstream;
  using std::ofstream;

  OpenbabelWrapper::OpenbabelWrapper()
  {
  }
    
  bool OpenbabelWrapper::canOpen(const QString &fileName, QIODevice::OpenMode mode)
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

  Molecule * OpenbabelWrapper::readMolecule(const QString &fileName,
      const QString &fileType, const QString &fileOptions, QString *error)
  {
    // Check that the file can be read from disk
    if (!canOpen(fileName, QFile::ReadOnly | QFile::Text)) {
      if (error)
        error->append(QObject::tr("File %1 can not be opened for reading.").arg(fileName));
      return 0;
    }

    // Construct the OpenBabel objects, set the file type
    OBConversion conv;
    OBFormat *inFormat;
    if (!fileType.isEmpty() && !conv.SetInFormat(fileType.toAscii().data())) {
      // Input format not supported
      if (error)
        error->append(QObject::tr("File type '%1' is not a supported for reading.").arg(fileType));
      return 0;
    } else {
      inFormat = conv.FormatFromExt(fileName.toAscii().data());
      if (!conv.SetInFormat(inFormat)) {
        // Input format not supported
        if (error)
          error->append(QObject::tr("File type for file '%1' is not a supported for reading.").arg(fileName));
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
    ifs.open(QFile::encodeName(fileName)); // This handles utf8 file names etc
    if (!ifs) // Should not happen, already checked file could be opened
      return 0;
    OBMol *obMol = new OBMol;
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

  bool OpenbabelWrapper::writeMolecule(const Molecule *molecule,
      const QString &fileName, const QString &fileType, QString *error)
  {
    // Check that the file can be written to disk
    if (!canOpen(fileName, QFile::WriteOnly | QFile::Text)) {
      // Cannot write to the file
      if (error)
        error->append(QObject::tr("File %1 can not be opened for writing.").arg(fileName));
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
        error->append(QObject::tr("File type '%1' is not a supported for writing.").arg(fileType));
      return false;
    } else {
      outFormat = conv.FormatFromExt(fileName.toAscii());
      if (!conv.SetOutFormat(outFormat)) {
        // Output format not supported
        if (error)
          error->append(QObject::tr("File type for file '%1' is not a supported for writing.").arg(fileName));
        return false;
      }
    }

    // Now attempt to write the molecule in
    ofstream ofs;
    ofs.open(QFile::encodeName(newFileName)); // This handles utf8 file names etc
    if (!ofs) // Should not happen, already checked file could be opened
      return false;
    OBMol obMol = molecule->OBMol();
    if (conv.Write(&obMol, &ofs)) {
      QFile(fileName).remove();
      newFile.rename(fileName);
      return true;
    } else {
      newFile.remove();
      if (error)
        error->append(QObject::tr("Writing a molecule to file '%1' failed.").arg(fileName));
      return false;
    }
  }

  bool OpenbabelWrapper::writeConformers(const Molecule *molecule,
                                         const QString &fileName,
                                         const QString &fileType,
                                         QString *error)
  {
    // Check that the file can be written to disk
    if (!canOpen(fileName, QFile::WriteOnly | QFile::Text)) {
      // Cannot write to the file
      if (error)
        error->append(QObject::tr("File %1 can not be opened for writing.").arg(fileName));
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
      if (!conv.SetOutFormat(outFormat)) {
        // Output format not supported
        if (error)
          error->append(QObject::tr("File type for file '%1' is not supported for writing.").arg(fileName));
        return false;
      }
    }

    // Now attempt to write the molecule in
    ofstream ofs;
    ofs.open(QFile::encodeName(newFileName)); // This handles utf8 file names etc
    if (!ofs) // Should not happen, already checked file could be opened
      return false;
    
    bool success = false;
    const std::vector<std::vector<Eigen::Vector3d>*> &conformers = molecule->conformers();
    OBMol obMol = molecule->OBMol();
    for (unsigned int i = 0; i < conformers.size(); ++i) {
      OBAtomIterator ai;
      for (OBAtom *atom = obMol.BeginAtom(ai); atom; atom = obMol.NextAtom(ai))
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

        // performance: check only certain molecule 1-10,20,50,100
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
          case 100:
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
        if (!OpenbabelWrapper::canOpen(m_moleculeFile->m_fileName, QFile::ReadOnly | QFile::Text)) {
          // Cannot read the file
          m_moleculeFile->m_error.append(QObject::tr("File %1 can not be opened for reading.").arg(
                m_moleculeFile->m_fileName));
          return;
        }
 
        // Construct the OpenBabel objects, set the file type
        OBConversion conv;
        OBFormat *inFormat;
        if (!m_moleculeFile->m_fileType.isEmpty() && !conv.SetInFormat(m_moleculeFile->m_fileType.toAscii().data())) {
          // Input format not supported
          m_moleculeFile->m_error.append(
              QObject::tr("File type '%1' is not supported for reading.").arg(m_moleculeFile->m_fileType));
          return;
        } else {
          inFormat = conv.FormatFromExt(m_moleculeFile->m_fileName.toAscii().data());
          if (!conv.SetInFormat(inFormat)) {
            // Input format not supported
            m_moleculeFile->m_error.append(QObject::tr("File type for file '%1' is not supported for reading.").arg(
                  m_moleculeFile->m_fileName));
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
        ifs.open(QFile::encodeName(m_moleculeFile->m_fileName)); // This handles utf8 file names etc
        if (!ifs) // Should not happen, already checked file could be opened
          return;
      
        // read all molecules
        OpenBabel::OBMol firstOBMol, currentOBMol;
        unsigned int c = 0;
        m_moleculeFile->streampos().push_back(ifs.tellg());
        while (conv.Read(&currentOBMol, &ifs)) {
          if (!c)
            firstOBMol = currentOBMol;
          // detect conformer/trajectory files
          detectConformers(c, firstOBMol, currentOBMol);
          // store information about molecule
          m_moleculeFile->streampos().push_back(ifs.tellg());
          m_moleculeFile->titles().append(currentOBMol.GetTitle());
          // increment count
          ++c;
        }

        // check for empty titles
        for (int i = 0; i < m_moleculeFile->titles().size(); ++i) {
          if (!m_moleculeFile->titles()[i].isEmpty())
            continue;

          QString title;
          if (m_moleculeFile->isConformerFile())
            title = tr("Conformer %1").arg(i+1);
          else
            title = tr("Molecule %1").arg(i+1);
        
          m_moleculeFile->titles()[i] = title;
        }

      
      }

      MoleculeFile *m_moleculeFile; 
  };

  MoleculeFile* OpenbabelWrapper::readFile(const QString &fileName,
      const QString &fileType, const QString &fileOptions, bool wait)
  {
    QPointer<MoleculeFile> moleculeFile = new MoleculeFile(fileName, fileType, fileOptions);

    ReadFileThread *thread = new ReadFileThread(moleculeFile);
    QObject::connect(thread, SIGNAL(finished()), moleculeFile, SLOT(threadFinished()));
    thread->start();

    if (wait) {
      thread->wait();
      moleculeFile->setReady(true);
    }

    return moleculeFile;
  }

}

#include "openbabelwrapper.moc"
