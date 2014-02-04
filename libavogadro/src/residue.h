/**********************************************************************
  Residue - Residue class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#ifndef RESIDUE_H
#define RESIDUE_H

#include <avogadro/fragment.h>

namespace Avogadro {

  /**
   * @class Residue residue.h <avogadro/residue.h>
   * @brief A biological residue that contains atoms and bonds.
   * @author Marcus D. Hanwell
   *
   * The Residue class is a Fragment subclass that provides the unique
   * additional information required for residues.
   */
  class A_EXPORT Residue : public Fragment
  {
    Q_OBJECT

      public:
    /**
     * Constructor.
     */
    Residue(QObject *parent=0);

    /**
     * Destructor.
     */
    ~Residue();

    /**
     * Add an Atom to the Fragment.
     */
    void addAtom(unsigned long id);

    /**
     * Remove the Atom from the Fragment.
     */
    void removeAtom(unsigned long id);

    /**
     * Set the number of the Residue, as in the file, e.g. 5A, 69, etc.
     */
    void setNumber(const QString& number);

    /**
     * @return The "number" of the residue, e.g. 5A, 69, etc.
     */
    QString number();

    /**
     * Set the chain number that this residue belongs to.
     */
    void setChainNumber(unsigned int number);

    /**
     * @return The chain number that the residue belongs to.
     */
    unsigned int chainNumber();

    /**
     * Set the chain ID (' ', 'A', 'B', ...) that this residue belongs to.
     */
    void setChainID(char id);
    /**
     * @return The chain ID (' ', 'A', 'B', ...) that the residue belongs to.
     */
    char chainID();


    /**
     * Set the text id of the Atom.
     * @param id The unique id of the Atom.
     * @param atomId The text id of the Atom in the Residue.
     * @return False if the atom id could not be set, true otherwise.
     */
    bool setAtomId(unsigned long id, QString atomId);

    /**
     * Set the text id of all the Atom objects.
     * @param atomIds QList containing the text ids of all the atoms.
     * @return False if the atom ids could not be set, true otherwise.
     */
    bool setAtomIds(const QList<QString> &atomIds);

    /**
     * Returns the atom text id, as in the Residue.
     * @param id The unique id of the Atom.
     * @return The text id of the supplied atom in the Residue.
     */
    QString atomId(unsigned long id);

    /**
     * @return QList of all atom text ids in the Residue.
     */
    const QList<QString> & atomIds() const;

    private Q_SLOTS:
    /**
     * Slot that handles when an atom has been updated (i.e., the atomID has changed)
     */
    void updateAtom();

  protected:
    QString m_number; /** Residue number as in the file, e.g. 5A, 69, etc. **/
    QList<QString> m_atomId; /** Atom text ids. **/
    unsigned int m_chainNumber; /** The chain number that the residue belongs to. **/
    char m_chainID;

  };

} // End namespace Avoagdro

#endif
