/**********************************************************************
  SmartsColor -  Map atom colors based on atom partial charge

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

#include "smartscolor.h"

#include <avogadro/primitive.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <QtPlugin>

#include <openbabel/mol.h>
#include <openbabel/atom.h>
#include <openbabel/parsmart.h>

using namespace OpenBabel;

namespace Avogadro {

  /// Constructor
  SmartsColor::SmartsColor()
  { 
    _pattern = new OBSmartsPattern;
    _pattern->Init("[#7]");
  }

  /// Destructor
  SmartsColor::~SmartsColor()
  { 
    if (_pattern) {
      delete _pattern;
      _pattern = NULL;
    }
  }

  void SmartsColor::setFromPrimitive(const Primitive *p)
  {
    if (!p || p->type() != Primitive::AtomType)
      return;

    const Atom *atom = static_cast<const Atom*>(p);

    // Start with the default "element color"
    QColor newcolor;
    if (atom->atomicNumber()) {
      std::vector<double> rgb = OpenBabel::etab.GetRGB(atom->atomicNumber());
      newcolor.setRgbF(rgb[0], rgb[1], rgb[2]);
    } else {
      newcolor.setRgbF(0.2f, 0.2f, 0.2f);
    }

    Molecule *molecule = qobject_cast<Molecule *>(atom->parent());
    if (!molecule || !_pattern)
      return;

    OBMol obmol = molecule->OBMol();
    _pattern->Match(obmol);
    std::vector<std::vector<int> > mlist = _pattern->GetUMapList();
    std::vector<std::vector<int> >::iterator match;
    bool matched = false;
    for (match = mlist.begin(); match != mlist.end(); ++match) { // iterate through matches
      for (unsigned idx = 0; idx < (*match).size(); ++idx) { // iterate through atoms in match
        if (atom->index() == ((*match)[idx] - 1)) { // TODO: OB uses index from 1
          matched = true;
          break;
        }
      } // atoms in match
      if (matched)
        break; // no need to check other matches
    } // matches

    // OK, now highlight aromatic nitrogens
    if (matched) {
      newcolor.setRgb(255, 0, 128);
      setFromQColor(newcolor);
    }
    else
      setFromQColor(newcolor.darker());
    
    m_channels[3] = 1.0;
  }

}

Q_EXPORT_PLUGIN2(smartscolor, Avogadro::SmartsColorFactory)

