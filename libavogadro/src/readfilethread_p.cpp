/**********************************************************************
  Copyright (C) 2009-2010 Marcus D. Hanwell, Tim Vandermeersch

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

#include "readfilethread_p.h"

#include "moleculefile.h"

#include <QtCore/QFile>
#include <QtCore/QStringList>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <fstream>

namespace Avogadro {

using OpenBabel::OBConversion;
using std::ifstream;

ReadFileThread::ReadFileThread(MoleculeFile *moleculeFile)
  : m_moleculeFile(moleculeFile)
{
}

void ReadFileThread::addConformer(const OpenBabel::OBMol &conformer)
{
  unsigned int numAtoms = conformer.NumAtoms();
  std::vector<Eigen::Vector3d> *coords = new std::vector<Eigen::Vector3d>;
  coords->reserve(numAtoms); // pre-allocate room for all atoms.

  for (unsigned int i = 0; i < numAtoms; ++i)
    coords->push_back(Eigen::Vector3d(conformer.GetAtom(i+1)->GetVector().AsArray()));

  m_moleculeFile->m_conformers.push_back(coords);
}

void ReadFileThread::detectConformers(unsigned int c,
                                      const OpenBabel::OBMol &first,
                                      const OpenBabel::OBMol &current)
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

void ReadFileThread::run()
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
  if (!m_moleculeFile->m_fileType.isEmpty() &&
      !conv.SetInFormat(m_moleculeFile->m_fileType.toAscii().data())) {
    // Input format not supported
    m_moleculeFile->m_error.append(
          QObject::tr("File type '%1' is not supported for reading.")
          .arg(m_moleculeFile->m_fileType));
    return;
  }
  else {
    inFormat = conv.FormatFromExt(m_moleculeFile->m_fileName.toAscii().data());
    if (!inFormat || !conv.SetInFormat(inFormat)) {
      // Input format not supported
      m_moleculeFile->m_error
          .append(QObject::tr("File type for file '%1' is not supported for reading.")
                  .arg(m_moleculeFile->m_fileName));
      return;
    }
  }

  // set any options
  if (!m_moleculeFile->m_fileOptions.isEmpty()) {
    foreach(const QString &option, m_moleculeFile
            ->m_fileOptions.split('\n', QString::SkipEmptyParts)) {
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

  // single molecule files are not conformer files
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

}

#include "readfilethread_p.moc"
