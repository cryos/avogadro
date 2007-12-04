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

#ifndef __PRIMITIVELIST_H
#define __PRIMITIVELIST_H

#include <avogadro/global.h>
#include <avogadro/primitive.h>


namespace Avogadro
{

  /**
   * @class PrimitiveList
   * @brief Class which set of Primitives
   * @author Donald Ephraim Curtis
   *
   * The PrimitiveList class is designed to hold a set of Primitive objects
   * and keep them organized by type allowing groups of them to be
   * retrieved in constant time.
   */
  class PrimitiveListPrivate;
  class A_EXPORT PrimitiveList
  {
    public:
      /**
       * Constructor(s)
       */
      PrimitiveList();
      PrimitiveList( const PrimitiveList &other );
      PrimitiveList( const QList<Primitive *> &other );

      PrimitiveList &operator=( const PrimitiveList &other );
      PrimitiveList &operator=(const QList<Primitive *> &other);

      /**
       * Deconstructor
       */
      ~PrimitiveList();

      /**
       * Returns a list of primitives for a given type.
       *
       * @param type the type of primitives to retrieve, one of Primitive::Type
       * @return a QList of pointers to Primitive objects
       */
      QList<Primitive *> subList( Primitive::Type type ) const;

      /**
       * Returns a list of all primitives of all types.
       *
       * @return a QList of pointers to Primitive objects
       */
      QList<Primitive *> list() const;

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
      PrimitiveListPrivate * const d;


  };

} // namespace Avogadro

#endif // __PRIMITIVELIST_H
