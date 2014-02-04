/**********************************************************************
  IDList - Organized List of Primitives.

  Copyright (C) 2007 Donald Ephraim Curtis

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

#ifndef IDLIST_H
#define IDLIST_H

#include <avogadro/global.h>
#include <avogadro/primitive.h>
#include <avogadro/primitivelist.h>

#include <QVector>
#include <QList>

namespace Avogadro
{

  /**
   * @class IDList
   * @brief List mapping primitives and unique ids
   * @author Donald Ephraim Curtis
   *
   * The IDList class is designed to hold a set of Primitive objects
   * and keep them organized by type allowing groups of them to be
   * retrieved in constant time.
   */
  class IDListPrivate;
  class A_EXPORT IDList
  {
    public:
      /**
       * Constructor(s)
       */
      IDList();
      IDList( const IDList &other );
      IDList( const QList<Primitive *> &other );
      IDList( const PrimitiveList &other );

      IDList &operator=( const IDList &other );
      IDList &operator=(const QList<Primitive *> &other);
      IDList &operator=(const PrimitiveList &other);

      /**
       * Deconstructor
       */
      ~IDList();

      /**
       * Returns a list of primitives for a given type.
       *
       * @param type the type of primitives to retrieve, one of Primitive::Type
       * @return a QList of pointers to Primitive objects
       */
      QList<unsigned long> subList( Primitive::Type type ) const;

      /**
       * @param p the primitive to check if it is in any list
       * @return true or false depending on whether p is in this list
       */
      bool contains( const Primitive *p ) const;

      /**
       * Add a primitive to the queue.
       *
       * @param p primitive to add
       */
      void append( Primitive *p );

      /**
       * Remove a primitive from the queue.  If the parameter does not
       * exist in the queue, nothing is removed.
       *
       * @param p primitive to remove
       */
      void removeAll( Primitive *p );

      /**
       * @return the total number of primitives in this queue
       */
      int size() const;

      /**
       * @return !size()
       */
      bool isEmpty() const;

      /**
       * @param type the type of primitives we're interested in
       * @return the number of primitives for the given type
       */
      int count( Primitive::Type type ) const;

      /**
       * effectively the same as size()
       * \sa size()
       */
      int count() const;

      /**
       * Removes every primitive from the queue.
       */
      void clear();

    private:
      IDListPrivate * const d;
  };

} // namespace Avogadro

#endif // __IDLIST_H
