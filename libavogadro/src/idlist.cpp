/**********************************************************************
  IDList - Organized List of Primitives.

  Copyright (C) 2007 Donald Ephraim Curtis

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

#include "idlist.h"
#include "primitive.h"

namespace Avogadro {

  class IDListPrivate {
    public:
      IDListPrivate() : size(0) {};

      int size;

      QVector< QList<unsigned long> > vector;
  };

  IDList::IDList() : d(new IDListPrivate) {
    d->vector.resize(Primitive::LastType);
  }

  IDList::IDList(const IDList &other) : d(new IDListPrivate)
  {
    IDListPrivate *e = other.d;
    d->size = e->size;
    d->vector = e->vector;
  }

  IDList::IDList(const QList<Primitive *> &other) : d(new IDListPrivate)
  {
    d->vector.resize(Primitive::LastType);
    foreach(Primitive *primitive, other)
    {
      append(primitive);
    }
  }

  IDList::IDList(const PrimitiveList &other) : d(new IDListPrivate)
  {
    d->vector.resize(Primitive::LastType);
    foreach(Primitive *primitive, other)
    {
      append(primitive);
    }
  }

  IDList &IDList::operator=(const IDList &other)
  {
    IDListPrivate *e = other.d;
    d->size = e->size;
    d->vector = e->vector;

    return *this;
  }

  IDList &IDList::operator=(const QList<Primitive *> &other)
  {
    clear();

    foreach(Primitive *primitive, other)
    {
      append(primitive);
    }

    return *this;
  }

  IDList &IDList::operator=(const PrimitiveList &other)
  {
    clear();

    foreach(Primitive *primitive, other)
    {
      append(primitive);
    }

    return *this;
  }

  IDList::~IDList() {
    delete d;
  }

  QList<unsigned long> IDList::subList(Primitive::Type type) const {

    if(type > Primitive::LastType)
    {
      return QList<unsigned long>();
    }

    return(d->vector[type]);
  }

  bool IDList::contains(const Primitive *p) const {
    return d->vector[p->type()].contains(p->id());
  }

  void IDList::append(Primitive *p) {
    d->vector[p->type()].append(p->id());
    d->size++;
  }

  void IDList::removeAll(Primitive *p) {
    d->vector[p->type()].removeAll(p->id());
    d->size--;
  }

  int IDList::size() const {
    return d->size;
  }

  bool IDList::isEmpty() const
  {
    return !size();
  }

  int IDList::count() const {
    return size();
  }

  int IDList::count(Primitive::Type type) const
  {
    if(type > Primitive::LastType)
    {
      return 0;
    }

    return d->vector[type].size();
  }

  void IDList::clear() {
    for( int i=0; i<d->vector.size(); i++ ) {
      d->vector[i].clear();
    }
    d->size = 0;
  }

}
