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

class Engine;

namespace Avogadro {

  class MainWindow;

  //! 
  enum primitiveType { moleculeType, atomType, bondType, 
    residueType, surfaceType, planeType,
    gridType, otherType, numTypes };

  /*
   * Primitive 
   * Used to define signals that all our primitives share.
   * 
   */

  class Primitive : public QObject
  {
    Q_OBJECT

    public:
      Primitive() : _selected(false), _type(otherType) {}
      Primitive(enum primitiveType type) : _selected(false), _type(type) {}

      bool isSelected() { return _selected;}
      void setSelected(bool s) { _selected = s;}
      void toggleSelected() { _selected = !_selected;}

      void setType(enum primitiveType type) { _type = type; }
      enum primitiveType getType() { return _type; }

    protected:
      bool _selected;
      enum primitiveType _type;
  };

  class Atom : public Primitive, public OpenBabel::OBAtom
  {
    Q_OBJECT

    public:
      Atom() : OpenBabel::OBAtom(), Primitive() { setType(atomType); }
  };

  class Bond : public Primitive, public OpenBabel::OBBond
  {
    Q_OBJECT

    public:
      Bond(): OpenBabel::OBBond(), Primitive() { setType(bondType); }
  };

  class Residue : public Primitive, public OpenBabel::OBResidue
  {
    Q_OBJECT

    public:
      Residue(): OpenBabel::OBResidue(), Primitive() { setType(residueType); }
  };

  class Molecule : public Primitive, public OpenBabel::OBMol
  {
    Q_OBJECT

    public:
      Molecule() : OpenBabel::OBMol(), Primitive(moleculeType) { }

      Atom *CreateAtom(void);
      Bond * CreateBond(void);
      Residue * CreateResidue(void);

//dc:       // coming soon
      Atom * NewAtom();
      Bond * NewBond();
      Residue * NewResidue();


    protected:
      MainWindow *window;
      std::vector< Atom * > 	_vatom;
      std::vector< Bond * > 	_vbond;

    signals:
      void atomAdded(Atom *atom);
      void bondAdded(Bond *bond);
      void residueAdded(Residue *residue);
  };

  class PrimitiveQueue
  {
    public:
      PrimitiveQueue() { for( int i=0; i<numTypes; i++ ) { _queue.append(new QList<Primitive *>()); } }

      QList<Primitive *>* getTypeQueue(int t) { 
        return(_queue[t]); 
      }

      void add(Primitive *p) { 
        _queue[p->getType()]->append(p); 
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

#endif
