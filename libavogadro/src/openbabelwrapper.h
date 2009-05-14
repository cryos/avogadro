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

namespace Avogadro {

  class Molecule;

  class A_EXPORT OpenbabelWrapper
  {
  public:
    OpenbabelWrapper();

    /**
     * Static function to load a file and return a Molecule pointer. You are
     * responsible for deleting the molecule object.
     * @param fileName The full path to the file to be opened.
     * @param fileType Optional file type parameter - override default file
     * extension parsing.
     * @param fileOptions Options for reading the molecule file, such as bonding.
     * @return The Molecule object loaded, 0 if the file could not be loaded.
     */
    static Molecule * openFile(const QString &fileName,
                               const QString &fileType = QString(),
                               const QString &fileOptions = QString());

    /**
     * Static function to save a file.
     * @param molecule The Molecule object to be saved.
     * @param fileName The full path to the file to be saved.
     * @param fileType Optional file type parameter - override default file
     * extension parsing.
     * @return True on success, false on failure.
     */
    static bool saveFile(const Molecule *molecule,
                         const QString &fileName,
                         const QString &fileType = QString());

  };

} // End namespace Avogadro

#endif // OPENBABELWRAPPER_H
