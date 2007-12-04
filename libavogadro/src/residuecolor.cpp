/**********************************************************************
  ResidueColor - Class for coloring based on residues (if available)

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

#include <config.h>
#include "residuecolor.h"

#include <avogadro/primitive.h>

using namespace OpenBabel;

namespace Avogadro {

  ResidueColor::ResidueColor()
  { }

  ResidueColor::~ResidueColor()
  {   }

  void ResidueColor::set(const Primitive *primitive)
  {
    if (!primitive)
      return;

    Primitive *p = const_cast<Primitive *>(primitive);
    Residue *residue;
    Atom *atom;
    if (p->type() == Primitive::ResidueType) {
      residue = static_cast<Residue*>(p);
    } else if (p->type() == Primitive::AtomType) {
      atom = static_cast<Atom*>(p);

      // do not perceive new residues
      residue = static_cast<Residue*>(atom->GetResidue(false));
      if (!residue ||
          strncasecmp(residue->GetName().c_str(), "UNK",3) == 0) {
        std::vector<double> rgb = etab.GetRGB( atom->GetAtomicNum() );
        m_red = rgb[0];
        m_green = rgb[1];
        m_blue = rgb[2];
        m_alpha = 1.0;
        return; // default if no residue is specified
      }
    }

    // Colors from http://jmol.sourceforge.net/jscolors/
    // "Protein amino"
    int red, blue, green;
    if (strncasecmp(residue->GetName().c_str(), "Ala",3) == 0) {
      red = 0xC8;
      green = 0xC8;
      blue = 0xC8;
    } else if (strncasecmp(residue->GetName().c_str(), "Arg",3) == 0) {
      red = 0x14;
      green = 0x5A;
      blue = 0xFF;
    } else if (strncasecmp(residue->GetName().c_str(), "Asn",3) == 0) {
      red = 0x00;
      green = 0xDC;
      blue = 0xDC;
    } else if (strncasecmp(residue->GetName().c_str(), "Asp",3) == 0) {
      red = 0xE6;
      green = 0x0A;
      blue = 0x0A;
    } else if (strncasecmp(residue->GetName().c_str(), "Cys",3) == 0) {
      red = 0xE6;
      green = 0xE6;
      blue = 0x00;
    } else if (strncasecmp(residue->GetName().c_str(), "Gln",3) == 0) {
      red = 0x00;
      green = 0xDC;
      blue = 0xDC;
    } else if (strncasecmp(residue->GetName().c_str(), "Glu",3) == 0) {
      red = 0xE6;
      green = 0x0A;
      blue = 0x0A;
    } else if (strncasecmp(residue->GetName().c_str(), "Gly",3) == 0) {
      red = 0xEB;
      green = 0xEB;
      blue = 0xEB;
    } else if (strncasecmp(residue->GetName().c_str(), "His",3) == 0) {
      red = 0x82;
      green = 0x82;
      blue = 0xD2;
    } else if (strncasecmp(residue->GetName().c_str(), "Ile",3) == 0) {
      red = 0x0F;
      green = 0x82;
      blue = 0x0F;
    } else if (strncasecmp(residue->GetName().c_str(), "Leu",3) == 0) {
      red = 0x0F;
      green = 0x82;
      blue = 0x0F;
    } else if (strncasecmp(residue->GetName().c_str(), "Lys",3) == 0) {
      red = 0x14;
      green = 0x5A;
      blue = 0xFF;
    } else if (strncasecmp(residue->GetName().c_str(), "Met",3) == 0) {
      red = 0xE6;
      green = 0xE6;
      blue = 0x00;
    } else if (strncasecmp(residue->GetName().c_str(), "Phe",3) == 0) {
      red = 0x32;
      green = 0x32;
      blue = 0xAA;
    } else if (strncasecmp(residue->GetName().c_str(), "Pro",3) == 0) {
      red = 0xDC;
      green = 0x96;
      blue = 0x82;
    } else if (strncasecmp(residue->GetName().c_str(), "Ser",3) == 0) {
      red = 0xFA;
      green = 0x96;
      blue = 0x00;
    } else if (strncasecmp(residue->GetName().c_str(), "Thr",3) == 0) {
      red = 0xFA;
      green = 0x96;
      blue = 0x00;
    } else if (strncasecmp(residue->GetName().c_str(), "Trp",3) == 0) {
      red = 0xB4;
      green = 0x5A;
      blue = 0xB4;
    } else if (strncasecmp(residue->GetName().c_str(), "Tyr",3) == 0) {
      red = 0x32;
      green = 0x32;
      blue = 0xAA;
    } else if (strncasecmp(residue->GetName().c_str(), "Val",3) == 0) {
      red = 0x0F;
      green = 0x82;
      blue = 0x0F;
    } else if (strncasecmp(residue->GetName().c_str(), "Asx",3) == 0) {
      red = 0xFF;
      green = 0x69;
      blue = 0xB4;
    } else if (strncasecmp(residue->GetName().c_str(), "Glx",3) == 0) {
      red = 0xFF;
      green = 0x69;
      blue = 0xB4;
    } else {
      red = 0xBE;
      green = 0xA0;
      blue = 0x6E;
    }

    m_alpha = 1.0;
    m_red = red / 255.0;
    m_blue = blue / 255.0;
    m_green = green / 255.0;
  }

}
