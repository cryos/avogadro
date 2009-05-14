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

#include <QFile>
#include <QStringList>
#include <QDebug>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <iostream>

namespace Avogadro {

  using OpenBabel::OBConversion;
  using OpenBabel::OBFormat;
  using OpenBabel::OBMol;

  using std::ifstream;
  using std::ofstream;

  OpenbabelWrapper::OpenbabelWrapper()
  {
  }

  Molecule * OpenbabelWrapper::openFile(const QString &fileName,
                                        const QString &fileType,
                                        const QString &fileOptions)
  {
    // Check that the file can be read from disk
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
      // Cannot read the file
      return 0;
    file.close();

    // Construct the OpenBabel objects, set the file type
    OBConversion conv;
    OBFormat *inFormat;
    if (!fileType.isEmpty() && !conv.SetInFormat(fileType.toAscii().data()))
      // Input format not supported
      return 0;
    else {
      inFormat = conv.FormatFromExt(fileName.toAscii().data());
      if (!conv.SetInFormat(inFormat))
        // Input format not supported
        return 0;
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
    }
    else
      return 0;
  }

  bool OpenbabelWrapper::saveFile(const Molecule *molecule,
                                  const QString &fileName,
                                  const QString &fileType)
  {
    // Check that the file can be written to disk
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
      // Cannot write to the file
      return false;
    file.close();
    QString newFileName(fileName + ".new");
    QFile newFile(newFileName);
    if (!newFile.open(QFile::WriteOnly | QFile::Text))
      // Cannot write to the file
      return false;
    newFile.close();

    // Construct the OpenBabel objects, set the file type
    OBConversion conv;
    OBFormat *outFormat;
    if (!fileType.isEmpty() && !conv.SetOutFormat(fileType.toAscii().data()))
      // Output format not supported
      return false;
    else {
      outFormat = conv.FormatFromExt(fileName.toAscii().data());
      if (!conv.SetOutFormat(outFormat))
        // Output format not supported
        return false;
    }

    // Now attempt to write the molecule in
    ofstream ofs;
    ofs.open(QFile::encodeName(newFileName)); // This handles utf8 file names etc
    if (!ofs) // Should not happen, already checked file could be opened
      return false;
    OBMol obMol = molecule->OBMol();
    if (conv.Write(&obMol, &ofs)) {
      file.remove();
      newFile.rename(fileName);
      return true;
    }
    else {
      newFile.remove();
      return false;
    }
  }

}
