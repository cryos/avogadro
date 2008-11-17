/**********************************************************************
  Primitive - Wrapper class around the OpenBabel classes

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <avogadro/global.h>

#include <QAbstractItemModel>

#include <Eigen/Core>

// Forward declarations
class QReadWriteLock;

namespace Avogadro {

  /**
   * @class Primitive primitive.h <avogadro/primitive.h>
   * @brief Base class for all primitives (Molecule, Atom, Bond, Residue, ...).
   */

  class PrimitivePrivate;
  class A_EXPORT Primitive : public QObject
  {
    Q_OBJECT
    Q_PROPERTY(Type type READ type)
    Q_ENUMS(Type)

    public:
      /**
       * This enum allows us to iterate through the various types
       * of primitives.
       */
      enum Type {
        /// Untyped Primitive
        OtherType=0,
        /// Molecule Primitive
        MoleculeType,
        /// Atom Primitive
        AtomType,
        /// Bond Primitive
        BondType,
        /// Residue Primitive
        ResidueType,
        /// Chain Primitive (i.e., a set of residues)
        ChainType,
        /// Fragment Primitve
        FragmentType,
        /// Surface Primitive
        SurfaceType,
        /// Surface Mesh Primitive
        MeshType,
        /// Cube Primitive
        CubeType,
        /// Plane Primitive
        PlaneType,
        /// Grid Primitive
        GridType,
        /// Points (i.e., non-atoms)
        PointType,
        /// Lines
        LineType,
        /// Vectors (i.e., arrows, dipole moments)
        VectorType,
        /// Non-bonded interactions (i.e., non-bond connections)
        NonbondedType,
        /// Text annoations
        TextType,
        /// End Placeholder
        LastType,
        /// First Placeholder
        FirstType=OtherType
      };

      /**
       * Default constructor.
       * @param parent the object parent
       */
      Primitive(QObject *parent = 0);
      /**
       * Constructor
       * @param type the primitive type
       * @param parent the object parent
       */
      explicit Primitive(Type type, QObject *parent=0);
      /**
       * Destructor
       */
      virtual ~Primitive();

      /**
       * Function used to push changes to a primitive to
       * the rest of the system.  At this time there is no
       * way (other than this) to generate a signal when
       * properties of a primitive change.
       *
       * In the case of the Atom primitive, this should be called
       * when changes to coordinates have been made.
       */
      void update();

      /**
       * @property Type
       * Holds the primitive type
       */

      /**
       * @return the primitive type (one of Primitive::Type)
       */
      Type type() const;

      QReadWriteLock *lock();

      /**
       * Set the id of the primitive, used to uniquely indentify the primitive.
       */
      void setId(unsigned long m_id);

      /**
       * @return the unique id of the primitive.
       */
      unsigned long id() const;

      /**
       * Set the index of the primitive, starting at zero.
       */
      void setIndex(unsigned long m_index);

      /**
       * @return the index of the primitive.
       * @note Replaces GetIdx().
       */
      unsigned long index() const;

    Q_SIGNALS:
      /**
       * Emitted when the primitive has been updated.
       */
      void updated();

    protected:
      PrimitivePrivate * const d_ptr;
      Primitive(PrimitivePrivate &dd, QObject *parent = 0);
      Primitive(PrimitivePrivate &dd, Type type, QObject *parent=0);

      enum Primitive::Type m_type;
      unsigned long m_id;
      unsigned long m_index;
      QReadWriteLock *m_lock;

    private:
      Q_DECLARE_PRIVATE(Primitive)

  };

} // namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Primitive*)

#endif // PRIMITIVE_H
