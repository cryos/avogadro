/**********************************************************************
 AtomIndexColor - Color atoms by numbering in the file

  Copyright (C) 2007 Geoffrey R. Hutchison

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

#include "atomindexcolor.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <QtPlugin>

#include <openbabel/mol.h>
#include <openbabel/atom.h>

namespace Avogadro {

  /// Constructor
  AtomIndexColor::AtomIndexColor()
  { }

  /// Destructor
  AtomIndexColor::~AtomIndexColor()
  { }

  void AtomIndexColor::setFromPrimitive(const Primitive *p)
  {
    if (!p || p->type() != Primitive::AtomType)
      return;

    const Atom *atom = static_cast<const Atom*>(p);
    Molecule *molecule = qobject_cast<Molecule *>(atom->parent());
    if (!molecule)
      return;

    unsigned int numAtoms = molecule->numAtoms();
    unsigned int index = atom->index();
    float indexFraction = float(index) / float(numAtoms);

    if (indexFraction < 0.4f) {
      // red to orange (i.e., R = 1.0  and G goes from 0 -> 0.5
      // also orange to yellow R = 1.0 and G goes from 0.5 -> 1.0
      m_channels[0] = 1.0f; // red
      m_channels[1] = indexFraction * 2.5f; // green
      m_channels[2] = 0.0f; // blue
    } else if (indexFraction > 0.4f && indexFraction < 0.6f) {
      // yellow to green: R 1.0 -> 0.0 and G stays 1.0
      m_channels[0] = 1.0f - 5.0f * (indexFraction - 0.4f); // red
      m_channels[1] = 1.0f; // green
      m_channels[2] = 0.0f; // blue
    } else if (indexFraction > 0.6f && indexFraction < 0.8f) {
      // green to blue: G -> 0.0 and B -> 1.0
      m_channels[0] = 0.0f; // red
      m_channels[1] = 1.0f - 5.0f * (indexFraction - 0.6f); // green
      m_channels[2] = 5.0f * (indexFraction - 0.6f); // blue
    } else if (indexFraction > 0.8f) {
    // blue to purple: B -> 0.5 and R -> 0.5
      m_channels[0] = 2.5f * (indexFraction - 0.8f);
      m_channels[1] = 0.0;
      m_channels[2] = 1.0f - 2.5f * (indexFraction - 0.8f);
    }

    m_channels[3] = 1.0;
  }

}

Q_EXPORT_PLUGIN2(atomindexcolor, Avogadro::AtomIndexColorFactory)

