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

  // color tables
#define RESNUM  23
  // Colors from http://jmol.sourceforge.net/jscolors/
  // "Protein amino"
  int jMolAmino[RESNUM][3] = {
    { 0xC8, 0xC8, 0xC8 }, /*  0: "Ala" */
    { 0x14, 0x5A, 0xFF }, /*  1: "Arg" */
    { 0x00, 0xDC, 0xDC }, /*  2: "Asn" */
    { 0xE6, 0x0A, 0x0A }, /*  3: "Asp" */
    { 0xE6, 0xE6, 0x00 }, /*  4: "Cys" */
    { 0x00, 0xDC, 0xDC }, /*  5: "Gln" */
    { 0xE6, 0x0A, 0x0A }, /*  6: "Glu" */
    { 0xEB, 0xEB, 0xEB }, /*  7: "Gly" */
    { 0x82, 0x82, 0xD2 }, /*  8: "His" */
    { 0x0F, 0x82, 0x0F }, /*  9: "Ile" */
    { 0x0F, 0x82, 0x0F }, /* 10: "Leu" */
    { 0x14, 0x5A, 0xFF }, /* 11: "Lys" */
    { 0xE6, 0xE6, 0x00 }, /* 12: "Met" */
    { 0x32, 0x32, 0xAA }, /* 13: "Phe" */
    { 0xDC, 0x96, 0x82 }, /* 14: "Pro" */
    { 0xFA, 0x96, 0x00 }, /* 15: "Ser" */
    { 0xFA, 0x96, 0x00 }, /* 16: "Thr" */
    { 0xB4, 0x5A, 0xB4 }, /* 17: "Trp" */
    { 0x32, 0x32, 0xAA }, /* 18: "Tyr" */
    { 0x0F, 0x82, 0x0F }, /* 19: "Val" */
    { 0xFF, 0x69, 0xB4 }, /* 20: "Asx" */
    { 0xFF, 0x69, 0xB4 }, /* 21: "Glx" */
    { 0xBE, 0xA0, 0x6E }  /* 22: Other / UNK */
  };

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
      // default is to color by element if no residue is specified
      std::vector<double> rgb = OpenBabel::etab.GetRGB( atom->atomicNumber() );
      if (!residue) {
        m_channels[0] = rgb[0];
        m_channels[1] = rgb[1];
        m_channels[2] = rgb[2];
        m_channels[3] = 1.0;
        return;
      }
      residueName = residue->GetName().c_str();
      if (residueName.compare("UNK", Qt::CaseInsensitive) == 0) {
        m_channels[0] = rgb[0];
        m_channels[1] = rgb[1];
        m_channels[2] = rgb[2];
        m_channels[3] = 1.0;
        return;
      }
    } else // not a residue or atom
      return; // not something we can color

    int offset;
    if (residueName.compare("Ala", Qt::CaseInsensitive) == 0) {
      offset = 0;
    } else if (residueName.compare("Arg", Qt::CaseInsensitive) == 0) {
      offset = 1;
    } else if (residueName.compare("Asn", Qt::CaseInsensitive) == 0) {
      offset = 2;
    } else if (residueName.compare("Asp", Qt::CaseInsensitive) == 0) {
      offset = 3;
    } else if (residueName.compare("Cys", Qt::CaseInsensitive) == 0) {
      offset = 4;
    } else if (residueName.compare("Gln", Qt::CaseInsensitive) == 0) {
      offset = 5;
    } else if (residueName.compare("Glu", Qt::CaseInsensitive) == 0) {
      offset = 6;
    } else if (residueName.compare("Gly", Qt::CaseInsensitive) == 0) {
      offset = 7;
    } else if (residueName.compare("His", Qt::CaseInsensitive) == 0) {
      offset = 8;
    } else if (residueName.compare("Ile", Qt::CaseInsensitive) == 0) {
      offset = 9;
    } else if (residueName.compare("Leu", Qt::CaseInsensitive) == 0) {
      offset = 10;
    } else if (residueName.compare("Lys", Qt::CaseInsensitive) == 0) {
      offset = 11;
    } else if (residueName.compare("Met", Qt::CaseInsensitive) == 0) {
      offset = 12;
    } else if (residueName.compare("Phe", Qt::CaseInsensitive) == 0) {
      offset = 13;
    } else if (residueName.compare("Pro", Qt::CaseInsensitive) == 0) {
      offset = 14;
    } else if (residueName.compare("Ser", Qt::CaseInsensitive) == 0) {
      offset = 15;
    } else if (residueName.compare("Thr", Qt::CaseInsensitive) == 0) {
      offset = 16;
    } else if (residueName.compare("Trp", Qt::CaseInsensitive) == 0) {
      offset = 17;
    } else if (residueName.compare("Tyr", Qt::CaseInsensitive) == 0) {
      offset = 18;
    } else if (residueName.compare("Val", Qt::CaseInsensitive) == 0) {
      offset = 19;
    } else if (residueName.compare("Asx", Qt::CaseInsensitive) == 0) {
      offset = 20;
    } else if (residueName.compare("Glx", Qt::CaseInsensitive) == 0) {
      offset = 21;
    } else {
      offset = 22;
    }

    m_channels[0] = jMolAmino[offset][0] / 255.0;
    m_channels[2] = jMolAmino[offset][1] / 255.0;
    m_channels[1] = jMolAmino[offset][2] / 255.0;
    m_channels[3] = 1.0;
  }

}

#include "residuecolor.moc"

Q_EXPORT_PLUGIN2(residuecolor, Avogadro::ResidueColorFactory)
