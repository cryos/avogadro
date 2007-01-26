/**********************************************************************
  Primitives - Wrapper class around the OpenBabel classes

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

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

#ifndef __PRIMATIVES_H
#define __PRIMATIVES_H

#include <openbabel/mol.h>
#include <QObject>
#include <QAbstractItemModel>

class Engine;

namespace Avogadro {

  /*
   * Primitive 
   * Used to define signals that all our primitives share.
   * 
   */

  class PrimitivePrivate;
  class Primitive : public QObject
  {
    Q_OBJECT
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
    Q_PROPERTY(enum Type type READ type)
    Q_ENUMS(Type)

    public:
      enum Type { 
        MoleculeType, 
        AtomType, 
        BondType, 
        ResidueType, 
        SurfaceType, 
        PlaneType,
        GridType, 
        OtherType, 
        LastType, 
        FirstType=MoleculeType 
      };
      
      Primitive(QObject *parent=0);
      Primitive(enum Type type, QObject *parent=0);
      virtual ~Primitive();

      bool isSelected() const;
      void setSelected(bool s);

      void update();
      enum Type type() const;

    public Q_SLOTS:
      void toggleSelected();

    Q_SIGNALS:
      void updated(Primitive*);

    protected:
      PrimitivePrivate * const d;

  };

  class Atom : public Primitive, public OpenBabel::OBAtom
  {
    Q_OBJECT

    public:
      Atom(QObject *parent=0) : OpenBabel::OBAtom(), Primitive(AtomType, parent) { }
  };

  class Bond : public Primitive, public OpenBabel::OBBond
  {
    Q_OBJECT

    public:
      Bond(QObject *parent=0): OpenBabel::OBBond(), Primitive(BondType, parent) { }
  };

  class Residue : public Primitive, public OpenBabel::OBResidue
  {
    Q_OBJECT

    public:
      Residue(QObject *parent=0): OpenBabel::OBResidue(), Primitive(ResidueType, parent) { }
  };

  class Molecule : public Primitive, public OpenBabel::OBMol
  {
    Q_OBJECT

    public:
      Molecule(QObject *parent=0);

      Atom *CreateAtom(void);
      Bond * CreateBond(void);
      Residue * CreateResidue(void);

      void DestroyAtom(OpenBabel::OBAtom*);
      void DestroyBond(OpenBabel::OBBond*);
      void DestroyResidue(OpenBabel::OBResidue*);

    protected:
      std::vector< Atom * > 	_vatom;
      std::vector< Bond * > 	_vbond;

    public Q_SLOTS:
      void updatePrimitive(Primitive *primitive);

    Q_SIGNALS:
      void primitiveAdded(Primitive *primitive);
      void primitiveUpdated(Primitive *primitive);
      void primitiveRemoved(Primitive *primitive);
  };

  class PrimitiveQueuePrivate;
  class PrimitiveQueue
  {
    public:
      PrimitiveQueue();
      ~PrimitiveQueue();

      const QList<Primitive *>* primitiveList(enum Primitive::Type type) const;

      void addPrimitive(Primitive *p);
      void removePrimitive(Primitive *p);

      void clear();

    private:
      PrimitiveQueuePrivate * const d;

  };

} // namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Primitive*)

#endif
