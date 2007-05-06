/**********************************************************************
  Primitive - Wrapper class around the OpenBabel classes

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef __PRIMITIVE_H
#define __PRIMITIVE_H

#include <avogadro/global.h>

#include <openbabel/mol.h>

#include <QObject>
#include <QAbstractItemModel>

#include <eigen/vector.h>

class Engine;

namespace Avogadro {

  /**
   * @class Primitive 
   * Base class for all primitives (Molecule, Atom, Bond, Residue, ...).
   */

  class PrimitivePrivate;
  class A_EXPORT Primitive : public QObject
  {
    Q_OBJECT
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
    Q_PROPERTY(enum Type type READ type)
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
        /// Surface Primitive
        SurfaceType, 
        /// Plane Primitive
        PlaneType,
        /// Grid Primitive
        GridType, 
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
      Primitive(enum Type type, QObject *parent=0);
      /**
       * Deconstructor
       */
      virtual ~Primitive();

      /**
       * @property selected
       * Holds whether the primitive object has been selected
       * by the user.
       *
       * Defaults to false.
       *
       * @sa isSelected()
       * @sa setSelected()
       */

      /**
       * @return @c true if primitive is selected, @c false otherwise
       */
      bool isSelected() const;

      /**
       * @param selected the selected value.
       */
      void setSelected(bool selected);

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
      enum Type type() const;

    public Q_SLOTS:
      /**
       * Toggle the @ref type property
       */
      void toggleSelected();

    Q_SIGNALS:
      /**
       * Emitted when the primitive has been updated.
       */
      void updated();

    protected:
      PrimitivePrivate * const d_ptr;
      Primitive(PrimitivePrivate &dd, QObject *parent = 0);
      Primitive(PrimitivePrivate &dd, enum Type type, QObject *parent=0);

    private:
      Q_DECLARE_PRIVATE(Primitive);

  };

  /**
   * @class Atom
   * @brief Atom Class
   * @author Donald Ephraim Curtis
   *
   * The Atom class is a Primitive subclass that provides a wrapper around
   * OpenBabel::OBAtom.  This class is provided to give more control of
   * the OpenBabel::OBAtom class through slots/signals provided by the 
   * Primitive superclass.
   */
  class A_EXPORT Atom : public Primitive, public OpenBabel::OBAtom
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Atom(QObject *parent=0) : OpenBabel::OBAtom(), Primitive(AtomType, parent) { }

      /** Returns the position of the atom, as a Eigen::Vector3d. This is similar to
        * the OBAtom::GetVector() method, which returns the position as a OpenBabel::vector3.
        *
        * Rationale for inlining: this method only does a cast on the return value of OBAtom::GetVector().
        * The memory layouts of the types between which it casts are not likely to change: both
        * types represent 3D vectors of doubles, and there's only one sane way to represent them:
        * struct{ double x,y,z; }.
        *
        * @return OBAtom::GetVector() but reinterpret_casted as a const Eigen::Vector3d &
        */
      inline const Eigen::Vector3d &pos () const
      {
        return *reinterpret_cast<const Eigen::Vector3d*>(&GetVector());
      }

      /** Sets the position of the atom, from a Eigen::Vector3d. This is similar to
        * the OBAtom::SetVector() method, which sets the position from a OpenBabel::vector3.
        *
        * Rationale for inlining: this method only does a cast on the argument of OBAtom::SetVector().
        * The memory layouts of the types between which it casts are not likely to change: both
        * types represent 3D vectors of doubles, and there's only one sane way to represent them:
        * struct{ double x,y,z; }.
        */
      inline void setPos(const Eigen::Vector3d &vec)
      {
        SetVector( *reinterpret_cast<const OpenBabel::vector3*>(&vec) );
      }
  };

  /**
   * @class Bond
   * @brief Bond Class
   * @author Donald Ephraim Curtis
   *
   * The Bond class is a Primitive subclass that provides a wrapper around
   * OpenBabel::OBBond.  This class is provided to give more control of
   * the OpenBabel::OBBond class through slots/signals provided by the 
   * Primitive superclass.
   */
  class A_EXPORT Bond : public Primitive, public OpenBabel::OBBond
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Bond(QObject *parent=0): OpenBabel::OBBond(), Primitive(BondType, parent) { }
  };

  /**
   * @class Residue
   * @brief Residue Class
   * @author Donald Ephraim Curtis
   *
   * The Residue class is a Primitive subclass that provides a wrapper around
   * OpenBabel::OBResidue.  This class is provided to give more control of
   * the OpenBabel::OBResidue class through slots/signals provided by the 
   * Primitive superclass.
   */
  class A_EXPORT Residue : public Primitive, public OpenBabel::OBResidue
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Residue(QObject *parent=0): OpenBabel::OBResidue(), Primitive(ResidueType, parent) { }
  };

  /**
   * @class Molecule
   * @brief Molecule Class
   * @author Donald Ephraim Curtis
   *
   * The Molecule class implements the OpenBabel::OBMol virtual functions
   * in order to not only use our primitive objects but also to provide signals
   * based on internal OpenBabel actions.  In terms of a Model-View architecture, 
   * this is our model class and is used by our various views to hold
   * all required data.
   */
  class MoleculePrivate;
  class A_EXPORT Molecule : public Primitive, public OpenBabel::OBMol
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Molecule(QObject *parent=0);
      Molecule(const Molecule &other);
      virtual ~Molecule();
      void update();

      /**
       * Virtual function inherited from OpenBabel::OBMol.  
       * Creates a new Atom object.
       *
       * @return pointer to a newly allocated Atom object
       */
      Atom *CreateAtom(void);

      /**
       * Virtual function inherited from OpenBabel::OBMol.  
       * Creates a new Bond object.
       *
       * @return pointer to a newly allocated Bond object
       */
      Bond * CreateBond(void);

      /**
       * Virtual function inherited from OpenBabel::OBMol.  
       * Creates a new Residue object.
       *
       * @return pointer to a newly allocated Residue object
       */
      Residue * CreateResidue(void);

      /**
       * Virtual function inherited from OpenBabel::OBMol.
       * Deletes an Atom object.
       *
       * @param atom the atom to delete
       */
      void DestroyAtom(OpenBabel::OBAtom* atom);

      /**
       * Virtual function inherited from OpenBabel::OBMol.
       * Deletes an Bond object.
       *
       * @param atom the bond to delete
       */
      void DestroyBond(OpenBabel::OBBond* bond);

      /**
       * Virtual function inherited from OpenBabel::OBMol.
       * Deletes an Residue object.
       *
       * @param atom the residue to delete
       */
      void DestroyResidue(OpenBabel::OBResidue* residue);

      const Eigen::Vector3d & center() const;
      const Eigen::Vector3d & normalVector() const;
      const double & radius() const;
      const Atom *farthestAtom() const;

      Molecule& operator=(const Molecule& other);

      Molecule& operator+=(const Molecule& other);

    private:
      /* shared d_ptr with Primitive */
      Q_DECLARE_PRIVATE(Molecule);

      void computeGeomInfo() const;

    private Q_SLOTS:
      /**
       * Function which handles when a child primitive has been
       * updated.  The response is to find the sender object
       * and then emit a signal passing the sender as a parameter.
       * 
       * @sa primitiveAdded
       * @sa primitiveUpdated
       * @sa primitiveRemoved
       */
      void updatePrimitive();

    Q_SIGNALS:
      /**
       * Emitted when a child primitive is added.
       *
       * @param primitive pointer to the primitive that was added
       */
      void primitiveAdded(Primitive *primitive);
      /**
       * Emitted when a child primitive is updated.
       *
       * @param primitive pointer to the primitive that was updated
       */
      void primitiveUpdated(Primitive *primitive);
      /**
       * Emitted when a child primitive is deleted.
       *
       * @param primitive pointer to the primitive that was updated before it is free'd
       */
      void primitiveRemoved(Primitive *primitive);
  };

  /**
   * @class PrimitiveQueue
   * @brief Class which set of Primitives
   * @author Donald Ephraim Curtis
   *
   * The PrimitiveQueue class is designed to hold a set of Primitive objects
   * and keep them organized by type allowing groups of them to be 
   * retrieved in constant time.
   */
  class PrimitiveQueuePrivate;
  class A_EXPORT PrimitiveQueue
  {
    public:
      /**
       * Constructor
       */
      PrimitiveQueue();

      /**
       * Deconstructor
       */
      ~PrimitiveQueue();

      /**
       * Returns a list of primitives for a given type.
       *
       * @param type the type of primitives to retrieve, one of Primitive::Type
       * @return a QList of pointers to Primitive objects
       */
      const QList<Primitive *>& primitiveList(enum Primitive::Type type) const;

      /**
       * Add a primitive to the queue.
       *
       * @param p primitive to add
       */
      void addPrimitive(Primitive *p);

      /**
       * Remove a primitive from the queue.  If the parameter does not
       * exist in the queue, nothing is removed.
       *
       * @param p primitive to remove
       */
      void removePrimitive(Primitive *p);

      /**
       * @return the total number of primitives in this queue
       */
      int size() const;
      /**
       * Removes every primitive from the queue.
       */
      void clear();

    private:
      PrimitiveQueuePrivate * const d;

  };

} // namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Primitive*);

#endif // __PRIMITIVES_H
