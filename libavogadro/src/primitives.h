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
#include <QDebug>
#include <QAbstractItemModel>

class Engine;

namespace Avogadro {

  class PrimitivePrivate;

  /*
   * Primitive 
   * Used to define signals that all our primitives share.
   * 
   */

  class Primitive : public QObject
  {
    Q_OBJECT

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
      enum Primitive::Type type() const;

    public slots:
      void toggleSelected();

    signals:
      void updated(Primitive*);

    protected:
      PrimitivePrivate *d;

  };

  class Atom : public Primitive, public OpenBabel::OBAtom
  {
    Q_OBJECT

    public:
      Atom() : OpenBabel::OBAtom(), Primitive(AtomType) { }
  };

  class Bond : public Primitive, public OpenBabel::OBBond
  {
    Q_OBJECT

    public:
      Bond(): OpenBabel::OBBond(), Primitive(BondType) { }
  };

  class Residue : public Primitive, public OpenBabel::OBResidue
  {
    Q_OBJECT

    public:
      Residue(): OpenBabel::OBResidue(), Primitive(ResidueType) { }
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

    public slots:
      void updatePrimitive(Primitive *primitive);

    signals:
      void primitiveAdded(Primitive *primitive);
      void primitiveUpdated(Primitive *primitive);
      void primitiveRemoved(Primitive *primitive);
  };

  class PrimitiveQueue
  {
    public:
      PrimitiveQueue() { 
        for( int i=Primitive::FirstType; i<Primitive::LastType; i++ ) { 
          _queue.append(new QList<Primitive *>()); 
        } 
      }

      const QList<Primitive *>* getTypeQueue(int t) const { 
        return(_queue[t]); 
      }

      void add(Primitive *p) { 
        _queue[p->type()]->append(p); 
      }

      void remove(Primitive *p) {
        _queue[p->type()]->removeAll(p);
      }

      void clear() {
        for( int i=0; i<_queue.size(); i++ ) {
          _queue[i]->clear();
        }
      }

    private:
      QList< QList<Primitive *>* > _queue;
  };

} // namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Primitive*)

#endif
