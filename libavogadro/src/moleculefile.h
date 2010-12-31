/**********************************************************************
  MoleculeFile - Class representing molecule file.

  Copyright (C) 2009 Marcus D. Hanwell, Tim Vandermeersch

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

namespace OpenBabel {
  class OBMol;
}

namespace Avogadro {

  class Molecule;

  class MoleculeFilePrivate;
  class A_EXPORT MoleculeFile : public QObject
  {
    Q_OBJECT

      friend class ReadFileThread;

  public:
    virtual ~MoleculeFile();
    /**
     * @return True if the MoleculeFile is ready (i.e. completed reading the file)
     */
    bool isReady() const;
    /**
     * @return True if the file contains multiple frames/conformers of the same molecule.
     */
    bool isConformerFile() const;
    /**
     * Get the number of molecules in the file.  
     */
    unsigned int numMolecules() const;
    /**
     * Get the titles for the molecules.
     */
    QStringList titles() const;

    //! @name Input (reading molecules)
    //@{
    /**
     * Get the @p {i}th molecule. This function returns a new pointer, you are 
     * responsible for deleting it.
     *
     * @param i The index for the molecule to get from the file (indexed from 0).
     * 
     * @return The ith molecule or 0 when i > numMolecule(). In FileIO::Output 
     * mode, this method always returns 0.
     */
    Molecule* molecule(unsigned int i = 0);
    /**
     * Get the original OBMol object for the @p {i}th molecule. This function 
     * returns a new pointer, you are responsible for deleting it.
     *
     * @param i The index for the molecule to get from the file (indexed from 0 to numMolecule()-1).
     * @return The original OBMol object read by OpenBabel.
     */
    OpenBabel::OBMol* OBMol(unsigned int i = 0);
    /**
     * Get all the conformers from the file. This methods returns an empty 
     * vector if the opened file isn't a conformer file (see isConformerFile()).
     */
    const std::vector<std::vector<Eigen::Vector3d>*>& conformers() const;
    //@}

    //! @name Output (writing molecules)
    //@{
    /**
     * Replace the @p {i}th molecule with @p molecule. When a molecule 
     * returned by molecule() has changed, this function can be used to write
     * it back to the file at the same position.
     * 
     * @param i The index for the molecule to replace.
     * @param molecule The changed (or a totally different) molecule
     * @param fileName The name of the file for saving.
     */
    bool replaceMolecule(unsigned int i, Molecule *molecule, QString fileName);
    /**
     * Insert a molecule at index @p i.
     * @param i The index for inserting the molecule
     * @param molecule The molecule to insert
     * @param fileName The name of the file for saving.
     * @todo implement this method
     */
    bool insertMolecule(unsigned int i, Molecule *molecule, QString fileName);
    /**
     * Append @p molecule to the end of the file.
     * @param molecule The molecule to append.
     * @param fileName The name of the file for saving.
     * @todo implement this method
     */
    bool appendMolecule(Molecule *molecule, QString fileName);
    //@}

    //! @name Error handling
    //@{
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
    //@}

    //! @name Convenience functions
    //@{
    /**
     * @return the filename for this file
     */
    const QString &fileName() const
    { return m_fileName; }

    /**
     * @return the file type of this file (e.g., extension or Open Babel code)
     */
    const QString &fileType() const
    { return m_fileType; }

    /**
     * @return any options set for this file
     */
    const QString &fileOptions() const
    { return m_fileOptions; }
    //@}

    //! @name Static methods
    //@{
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
    //@}

  Q_SIGNALS:
    /**
     * This signal is emitted when the results are read (i.e. the file is read).
     */
    void ready();

    /**
     * This signal is emitted when the first molecule is read
     */
    void firstMolReady();

    protected Q_SLOTS:
    void threadFinished();
  protected:
    MoleculeFile(const QString &fileName, const QString &fileType,
                 const QString &fileOptions);

    QStringList& titlesRef();
    std::vector<std::streampos>& streamposRef();
    std::vector<std::vector<Eigen::Vector3d>*>& conformersRef();
    void setConformerFile(bool value);
    void setReady(bool value);
    void setFirstReady(bool value); // used by ReadFileThread

    MoleculeFilePrivate * const d; 
    QString m_fileName, m_fileType, m_fileOptions;
    QString m_error;
    std::vector<std::vector<Eigen::Vector3d>*> m_conformers;
  };

} // End namespace Avogadro

#endif // MOLECULEFILE_H
