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

#ifndef MOLECULEFILE_H
#define MOLECULEFILE_H

#include <avogadro/global.h>

#include <QString>
#include <QIODevice>

#include <vector>
#include <Eigen/Core>

namespace Avogadro {

  class Molecule;

  class MoleculeFilePrivate;
  class A_EXPORT MoleculeFile : public QObject
  {
    Q_OBJECT

    friend class OpenbabelWrapper;
    friend class ReadFileThread;

    public:
      virtual ~MoleculeFile();
      /**
       * @return True if the file contains multiple frames/conformers of the same molecule.
       */
      bool isConformerFile() const;
      /**
       * Get the number of molecules in the file. When 
       */
      unsigned int numMolecules() const;
      /**
       * Get the titles for the molecules.
       */
      QList<QString> titles() const;
      /**
       * Get the @p {i}th molecule.
       *
       * @param i The index for the molecule to get from the file (indexed from 0).
       * 
       * @return The ith molecule or 0 when i > numMolecule(). In FileIO::Output 
       * mode, this method always returns 0.
       */
      Molecule* molecule(unsigned int i = 0);
      /**
       * Get all the conformers from the file. This methods returns an empty 
       * vector if the opened file isn't a conformer file (see isConformerFile()).
       */
      const std::vector<std::vector<Eigen::Vector3d>*>& conformers() const;
      /**
       * @return Errors from reading/writing to the file.
       */
      const QString& errors() const;
      /**
       * Clear the errors. Errors are always appended to error(), so unless you 
       * clear them explicitly, consecutive calls to errors() returns all errors
       * from before plus the new ones (if any).
       */
      void clearErrors();
    Q_SIGNALS:
      void ready();
    protected Q_SLOTS:
      void threadFinished();
    protected:
      MoleculeFile(const QString &fileName, const QString &fileType,
                   const QString &fileOptions);
      QStringList& titles();
      QString& errors();
      std::vector<std::streampos>& streampos();
      std::vector<std::vector<Eigen::Vector3d>*>& conformers();
      void setConformerFile(bool value);
      void setReady(bool value);
 
      MoleculeFilePrivate * const d; 
      QString m_fileName, m_fileType, m_fileOptions;
      QString m_error;
  };

} // End namespace Avogadro

#endif // MOLECULEFILE_H
