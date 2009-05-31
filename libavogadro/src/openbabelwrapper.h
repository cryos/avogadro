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

#ifndef OPENBABELWRAPPER_H
#define OPENBABELWRAPPER_H

#include <avogadro/global.h>

#include <QString>
#include <QIODevice>

#include <vector>
#include <Eigen/Core>

namespace Avogadro {

  class Molecule;
  class MoleculeFile;

  class A_EXPORT OpenbabelWrapper
  {
  public:
    OpenbabelWrapper();

    /** 
     * Static function to check if a file can be opened in the specified @p mode.
     * 
     * @param fileName The full path to the file to be opened.
     * @param mode QIODevice::OpenMode to check.
     * 
     * @return True if the file can be opened in the specified @p mode.
     */
    static bool canOpen(const QString &fileName, QIODevice::OpenMode mode);

    /**
     * Static function to load a file and return a Molecule pointer. You are
     * responsible for deleting the molecule object.
     * @param fileName The full path to the file to be opened.
     * @param fileType Optional file type parameter - override default file
     * extension parsing.
     * @param fileOptions Options for reading the molecule file, such as bonding.
     * @return The Molecule object loaded, 0 if the file could not be loaded.
     */
    static Molecule * readMolecule(const QString &fileName,
                                   const QString &fileType = QString(),
                                   const QString &fileOptions = QString(),
                                   QString *error = 0);

    /**
     * Static function to save a single molecule to a file. If writing was 
     * unsuccessful, a previously existing file will not be overwritten. 
     * @param molecule The Molecule object to be saved.
     * @param fileName The full path to the file to be saved.
     * @param fileType Optional file type parameter - override default file
     * extension parsing.
     * @return True on success, false on failure.
     */
    static bool writeMolecule(const Molecule *molecule,
                              const QString &fileName,
                              const QString &fileType = QString(),
                              QString *error = 0);

    /**
     * Static function to save a all conformers in a molecule to a file. If 
     * writing was unsuccessful, a previously existing file will not be 
     * overwritten. All formats with support for multiple molecules can be 
     * used.
     * @param molecule The Molecule object to be saved.
     * @param fileName The full path to the file to be saved.
     * @param fileType Optional file type parameter - override default file
     * extension parsing.
     * @return True on success, false on failure.
     */ 
    static bool writeConformers(const Molecule *molecule,
                                const QString &fileName,
                                const QString &fileType = QString(),
                                QString *error = 0);


    /**
     * Read an entire file, possibly containing multiple molecules in a 
     * separate thread and return a MoleculeFile object with the result.
     *
     * By default, the @p wait parameter is set to true and the function
     * waits for the thread to finish before returning. If set to 
     * false, listening to the MoleculeFile::ready() signal will be 
     * emitted when the results are ready.
     * @param fileName The full path to the file to be saved.
     * @param fileType Optional file type parameter - override default file
     * extension parsing.
     * @param fileOptions Options for reading the molecule file, such as bonding.
     * @param wait Wait for the thread to finish before returning.
     * @return MoleculeFile with (future) results.
     */
    static MoleculeFile* readFile(const QString &fileName,
                                  const QString &fileType = QString(),
                                  const QString &fileOptions = QString(),
                                  bool wait = true);

  };

} // End namespace Avogadro

#endif // OPENBABELWRAPPER_H
