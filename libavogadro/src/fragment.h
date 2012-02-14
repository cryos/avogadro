/**********************************************************************
  Fragment - Fragment class derived from the base Primitive class

  Copyright (C) 2008 Marcus D. Hanwell

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

#ifndef FRAGMENT_H
#define FRAGMENT_H

#include <avogadro/primitive.h>

#include <QList>

namespace Avogadro {

  class Molecule;

  /**
   * @class Fragment fragment.h <avogadro/fragment.h>
   * @brief A fragment containing atoms and bonds.
   * @author Marcus D. Hanwell
   *
   * The Fragment class is a Primitive subclass that provides a generic way
   * of addressing fragments. This is intended to be suitable for rings,
   * residues, molecule fragments etc. That is anything that needs to address
   * a subset of atoms/bonds in a Molecule.
   */
  class FragmentPrivate;
  class A_EXPORT Fragment : public Primitive
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Fragment(QObject *parent=0);

      /**
       * Consttructor. This class is further inherited by Residue.
       */
      explicit Fragment(Type type, QObject *parent=0);

      /**
       * Destructor.
       */
      ~Fragment();

      /**
       * @return the name of the fragment.
       * @note Replaces GetName().
       */
      inline QString name() const { return m_name; }

      /**
       * Set the name of the fragment.
       */
      inline void setName(QString name) { m_name = name; }

      /**
       * Add an Atom to the Fragment.
       */
      void addAtom(unsigned long id);

      /**
       * Remove the Atom from the Fragment.
       */
      void removeAtom(unsigned long id);

      /**
       * @return QList of the unique ids of the atoms in this Fragment.
       */
      QList<unsigned long> atoms() const;

      /**
       * Add a Bond to the Fragment.
       */
      void addBond(unsigned long id);

      /**
       * Remove the Bond from the Fragment.
       */
      void removeBond(unsigned long id);

      /**
       * @return QList of the unique ids of the bonds in this Fragment.
       */
      QList<unsigned long> bonds() const;

      friend class Molecule;

    protected:
      Molecule *m_molecule;
      QString m_name; /** The name of the Fragment. **/
      QList<unsigned long> m_atoms; /** QList of the atom ids. **/
      QList<unsigned long> m_bonds; /** QList of the bond ids. **/

    private:
      Q_DECLARE_PRIVATE(Fragment)
  };

} // End namespace Avoagdro

#endif
