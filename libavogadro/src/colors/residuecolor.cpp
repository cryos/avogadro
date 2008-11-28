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

#include "residuecolor.h"
#include <config.h>
#include <avogadro/primitive.h>
#include <QtPlugin>

#include <avogadro/residue.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>
#include <openbabel/atom.h>
#include <openbabel/residue.h>

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
    QString residueName; // this colors by residue name

    if (p->type() == Primitive::ResidueType) {
      Residue *residue = static_cast<Residue*>(p);
      residueName = residue->name();
    } else if (p->type() == Primitive::AtomType) {
      Atom *atom = static_cast<Atom*>(p);

      // do not perceive new residues
      OpenBabel::OBResidue *residue = atom->OBAtom().GetResidue(false);
      residueName = residue->GetName().c_str();
      if (!residue ||
          residueName.compare("UNK", Qt::CaseInsensitive) == 0) {
	// default is to color by element if no residue is specified
        std::vector<double> rgb = OpenBabel::etab.GetRGB( atom->atomicNumber() );
        m_channels[0] = rgb[0];
        m_channels[1] = rgb[1];
        m_channels[2] = rgb[2];
        m_channels[3] = 1.0;
        return;
      }
    } else
      return; // not something we can color

    // Colors from http://jmol.sourceforge.net/jscolors/
    // "Protein amino"
    int red, blue, green;
    if (residueName.compare("Ala", Qt::CaseInsensitive) == 0) {
      red = 0xC8;
      green = 0xC8;
      blue = 0xC8;
    } else if (residueName.compare("Arg", Qt::CaseInsensitive) == 0) {
      red = 0x14;
      green = 0x5A;
      blue = 0xFF;
    } else if (residueName.compare("Asn", Qt::CaseInsensitive) == 0) {
      red = 0x00;
      green = 0xDC;
      blue = 0xDC;
    } else if (residueName.compare("Asp", Qt::CaseInsensitive) == 0) {
      red = 0xE6;
      green = 0x0A;
      blue = 0x0A;
    } else if (residueName.compare("Cys", Qt::CaseInsensitive) == 0) {
      red = 0xE6;
      green = 0xE6;
      blue = 0x00;
    } else if (residueName.compare("Gln", Qt::CaseInsensitive) == 0) {
      red = 0x00;
      green = 0xDC;
      blue = 0xDC;
    } else if (residueName.compare("Glu", Qt::CaseInsensitive) == 0) {
      red = 0xE6;
      green = 0x0A;
      blue = 0x0A;
    } else if (residueName.compare("Gly", Qt::CaseInsensitive) == 0) {
      red = 0xEB;
      green = 0xEB;
      blue = 0xEB;
    } else if (residueName.compare("His", Qt::CaseInsensitive) == 0) {
      red = 0x82;
      green = 0x82;
      blue = 0xD2;
    } else if (residueName.compare("Ile", Qt::CaseInsensitive) == 0) {
      red = 0x0F;
      green = 0x82;
      blue = 0x0F;
    } else if (residueName.compare("Leu", Qt::CaseInsensitive) == 0) {
      red = 0x0F;
      green = 0x82;
      blue = 0x0F;
    } else if (residueName.compare("Lys", Qt::CaseInsensitive) == 0) {
      red = 0x14;
      green = 0x5A;
      blue = 0xFF;
    } else if (residueName.compare("Met", Qt::CaseInsensitive) == 0) {
      red = 0xE6;
      green = 0xE6;
      blue = 0x00;
    } else if (residueName.compare("Phe", Qt::CaseInsensitive) == 0) {
      red = 0x32;
      green = 0x32;
      blue = 0xAA;
    } else if (residueName.compare("Pro", Qt::CaseInsensitive) == 0) {
      red = 0xDC;
      green = 0x96;
      blue = 0x82;
    } else if (residueName.compare("Ser", Qt::CaseInsensitive) == 0) {
      red = 0xFA;
      green = 0x96;
      blue = 0x00;
    } else if (residueName.compare("Thr", Qt::CaseInsensitive) == 0) {
      red = 0xFA;
      green = 0x96;
      blue = 0x00;
    } else if (residueName.compare("Trp", Qt::CaseInsensitive) == 0) {
      red = 0xB4;
      green = 0x5A;
      blue = 0xB4;
    } else if (residueName.compare("Tyr", Qt::CaseInsensitive) == 0) {
      red = 0x32;
      green = 0x32;
      blue = 0xAA;
    } else if (residueName.compare("Val", Qt::CaseInsensitive) == 0) {
      red = 0x0F;
      green = 0x82;
      blue = 0x0F;
    } else if (residueName.compare("Asx", Qt::CaseInsensitive) == 0) {
      red = 0xFF;
      green = 0x69;
      blue = 0xB4;
    } else if (residueName.compare("Glx", Qt::CaseInsensitive) == 0) {
      red = 0xFF;
      green = 0x69;
      blue = 0xB4;
    } else {
      red = 0xBE;
      green = 0xA0;
      blue = 0x6E;
    }

    m_channels[3] = 1.0;
    m_channels[0] = red / 255.0;
    m_channels[2] = blue / 255.0;
    m_channels[1] = green / 255.0;
  }

}

#include "residuecolor.moc"

Q_EXPORT_PLUGIN2(residuecolor, Avogadro::ResidueColorFactory)
