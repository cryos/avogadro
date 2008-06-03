/**********************************************************************
 ElementColor - Default class for coloring atoms based on element

  Copyright (C) 2007 Geoffrey R. Hutchison

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

#include "elementcolor.h"

#include <config.h>
#include <avogadro/primitive.h>

using namespace OpenBabel;

namespace Avogadro {

  /// Constructor
  ElementColor::ElementColor()
  { }

  // Destructor
  ElementColor::~ElementColor()
  {   }

  void ElementColor::set(const Primitive *p)
  {
    if (!p || p->type() != Primitive::AtomType)
      return;

    const Atom *atom = static_cast<const Atom*>(p);
    std::vector<double> rgb = etab.GetRGB( atom->GetAtomicNum() );
    m_red = rgb[0];
    m_green = rgb[1];
    m_blue = rgb[2];
    m_alpha = 1.0;
  }

}
