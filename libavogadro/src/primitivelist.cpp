/**********************************************************************
  PrimitiveList - Organized List of Primitives.

  Copyright (C) 2007 Donald Ephraim Curtis

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

#include "config.h"

#include <avogadro/primitivelist.h>
#include <QDebug>
#include <eigen/regression.h>
#include <openbabel/obiter.h>

using namespace OpenBabel;

namespace Avogadro {

  class PrimitiveListPrivate {
    public:
      PrimitiveListPrivate() : size(0) {};

      int size;

      QVector< QList<Primitive *> > vector;
  };

  PrimitiveList::PrimitiveList() : d(new PrimitiveListPrivate) {
    d->vector.resize(Primitive::LastType);
  }

  PrimitiveList::PrimitiveList(const PrimitiveList &other) : d(new PrimitiveListPrivate)
  {
    PrimitiveListPrivate *e = other.d;
    d->size = e->size;
    d->vector = e->vector;
  }

  PrimitiveList &PrimitiveList::operator=(const PrimitiveList &other)
  {
    PrimitiveListPrivate *e = other.d;
    d->size = e->size;
    d->vector = e->vector;

    return *this;
  }

  PrimitiveList::~PrimitiveList() {
    delete d;
  }

  QList<Primitive *> PrimitiveList::subList(Primitive::Type type) const {

    if(type > Primitive::LastType)
    {
      return QList<Primitive *>();
    }

    return(d->vector[type]);
  }

  QList<Primitive *> PrimitiveList::list() const {
    QList<Primitive*> returnList;

    foreach(QList<Primitive*> typeList, d->vector) {
      returnList += typeList;
    }

    return returnList;
  }

  bool PrimitiveList::contains(const Primitive *p) const {
    // this is really bad, but it's a compiler workaround
    Primitive *cp = const_cast<Primitive *>(p);
    return d->vector[p->type()].contains(cp);
  }

  void PrimitiveList::append(Primitive *p) {
    d->vector[p->type()].append(p);
    d->size++;
  }

  void PrimitiveList::removeAll(Primitive *p) {
    d->vector[p->type()].removeAll(p);
    d->size--;
  }

  int PrimitiveList::size() const {
    return d->size;
  }

  int PrimitiveList::size(Primitive::Type type) const
  {
    if(type > Primitive::LastType)
    {
      return 0;
    }

    return d->vector[type].size();
  }

  void PrimitiveList::clear() {
    for( int i=0; i<d->vector.size(); i++ ) {
      d->vector[i].clear();
    }
    d->size = 0;
  }
}
