/**********************************************************************
  Residue - Residue class derived from the base Primitive class

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

#ifndef RESIDUE_H
#define RESIDUE_H

#include <avogadro/fragment.h>

namespace Avogadro {

  /**
   * @class Residue residue.h <avogadro/residue.h>
   * @brief Residue Class
   * @author Marcus D. Hanwell
   *
   * The Residue class is a Fragment subclass that provides the unique
   * additional information required for residues.
   */
  class A_EXPORT Residue : public Fragment
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Residue(QObject *parent=0): Fragment(ResidueType, parent) { }

      QString numString() // Replaces GetName
      { return "FIXME"; }
  };

} // End namespace Avoagdro

#endif
