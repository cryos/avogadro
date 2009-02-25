/**********************************************************************
  ChargeColor -  Map atom colors based on atom partial charge

  Copyright (C) 2009 Geoffrey R. Hutchison

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

#include "chargecolor.h"

#include <config.h>
#include <avogadro/primitive.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <QtPlugin>

#include <openbabel/mol.h>
#include <openbabel/atom.h>

namespace Avogadro {

  /// Constructor
  ChargeColor::ChargeColor()
  { }

  /// Destructor
  ChargeColor::~ChargeColor()
  { }

  void ChargeColor::set(const Primitive *p)
  {
    if (!p || p->type() != Primitive::AtomType)
      return;

    const Atom *atom = static_cast<const Atom*>(p);
    float charge = atom->partialCharge();
    float scaledCharge = sqrt(fabs(charge));
    if (scaledCharge > 1.0)
      scaledCharge = 1.0;

    if (charge < 0.0f) {
      // white to red (i.e. back down on green and blue)
      // We assume that partial charge could be up to -2.0
      m_channels[0] = 1.0f; // red
      m_channels[1] = 1.0 - scaledCharge; // green
      m_channels[2] = m_channels[1]; // blue = green
    } else {
      // white to blue (i.e., back down on red and green)
      m_channels[0] = 1.0f - scaledCharge; // red
      m_channels[1] = m_channels[0]; // green = red
      m_channels[2] = 1.0f; // blue
    }
    
    m_channels[3] = 1.0;
  }

}

#include "chargecolor.moc"

Q_EXPORT_PLUGIN2(chargecolor, Avogadro::ChargeColorFactory)
