/**********************************************************************
  ResidueColor - Class for coloring based on residues (if available)

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

#include "residuecolor.h"

#include <QtPlugin>

#include <avogadro/residue.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>
#include <openbabel/atom.h>
#include <openbabel/residue.h>

#include <QDebug>

namespace Avogadro {

  // color tables
#define RESNUM  29

  // Colors from http://jmol.sourceforge.net/jscolors/
  // "Protein amino"
  const int jMolAmino[RESNUM][3] = {
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
    { 0xBE, 0xA0, 0x6E }, /* 22: Other / UNK */
    { 0xA0, 0xA0, 0xFF }, /* 23: A */
    { 0xFF, 0x70, 0x70 }, /* 24: G */
    { 0x80, 0xFF, 0xFF }, /* 25: I */
    { 0xFF, 0x8C, 0x4B }, /* 26: C */
    { 0xA0, 0xFF, 0xA0 }, /* 27: T */
    { 0xFF, 0x80, 0x80 }  /* 28: U */
  };

  // Colors from http://jmol.sourceforge.net/jscolors/
  // "Protein Shapely"
  const int jMolShapely[RESNUM][3] = {
    { 0x8C, 0xFF, 0x8C }, /*  0: "Ala" */
    { 0x00, 0x00, 0x7C }, /*  1: "Arg" */
    { 0xFF, 0x7C, 0x70 }, /*  2: "Asn" */
    { 0xA0, 0x00, 0x42 }, /*  3: "Asp" */
    { 0xFF, 0xFF, 0x70 }, /*  4: "Cys" */
    { 0xFF, 0x4C, 0x4C }, /*  5: "Gln" */
    { 0x66, 0x00, 0x00 }, /*  6: "Glu" */
    { 0xFF, 0xFF, 0xFF }, /*  7: "Gly" */
    { 0x70, 0x70, 0xFF }, /*  8: "His" */
    { 0x00, 0x4C, 0x00 }, /*  9: "Ile" */
    { 0x45, 0x5E, 0x45 }, /* 10: "Leu" */
    { 0x47, 0x47, 0xB8 }, /* 11: "Lys" */
    { 0xB8, 0xA0, 0x42 }, /* 12: "Met" */
    { 0x53, 0x4C, 0x52 }, /* 13: "Phe" */
    { 0x52, 0x52, 0x52 }, /* 14: "Pro" */
    { 0xFF, 0x70, 0x42 }, /* 15: "Ser" */
    { 0xB8, 0x4C, 0x00 }, /* 16: "Thr" */
    { 0x4F, 0x46, 0x00 }, /* 17: "Trp" */
    { 0x8C, 0x70, 0x4C }, /* 18: "Tyr" */
    { 0xFF, 0x8C, 0xFF }, /* 19: "Val" */
    { 0xFF, 0x00, 0xFF }, /* 20: "Asx" */
    { 0xFF, 0x00, 0xFF }, /* 21: "Glx" */
    { 0xFF, 0x00, 0xFF }, /* 22: Other / UNK */
    { 0xA0, 0xA0, 0xFF }, /* 23: A */
    { 0xFF, 0x70, 0x70 }, /* 24: G */
    { 0x80, 0xFF, 0xFF }, /* 25: I */
    { 0xFF, 0x8C, 0x4B }, /* 26: C */
    { 0xA0, 0xFF, 0xA0 }, /* 27: T */
    { 0xFF, 0x80, 0x80 }  /* 28: U */
  };

  const int hydrophobicity[RESNUM][3] = {
    { 0x99, 0x99, 0xFF }, /*  0: "Ala" */
    { 0xFF, 0x00, 0x00 }, /*  1: "Arg" */
    { 0xFF, 0x38, 0x38 }, /*  2: "Asn" */
    { 0xFF, 0x38, 0x38 }, /*  3: "Asp" */
    { 0x71, 0x71, 0xFF }, /*  4: "Cys" */
    { 0xFF, 0x38, 0x38 }, /*  5: "Gln" */
    { 0xFF, 0x38, 0x38 }, /*  6: "Glu" */
    { 0xFF, 0xE8, 0xE8 }, /*  7: "Gly" */
    { 0xFF, 0x49, 0x49 }, /*  8: "His" */
    { 0x00, 0x00, 0xFF }, /*  9: "Ile" */
    { 0x27, 0x27, 0xFF }, /* 10: "Leu" */
    { 0xFF, 0x22, 0x22 }, /* 11: "Lys" */
    { 0x93, 0x93, 0xFF }, /* 12: "Met" */
    { 0x60, 0x60, 0xFF }, /* 13: "Phe" */
    { 0xFF, 0xA4, 0xA4 }, /* 14: "Pro" */
    { 0xFF, 0xD1, 0xD1 }, /* 15: "Ser" */
    { 0xFF, 0xD7, 0xD7 }, /* 16: "Thr" */
    { 0xFF, 0xCC, 0xCC }, /* 17: "Trp" */
    { 0xFF, 0xB5, 0xB5 }, /* 18: "Tyr" */
    { 0x10, 0x10, 0xFF }, /* 19: "Val" */
    { 0xFF, 0x00, 0xFF }, /* 20: "Asx" */
    { 0xFF, 0x00, 0xFF }, /* 21: "Glx" */
    { 0xFF, 0x00, 0xFF }, /* 22: Other / UNK */
    { 0xA0, 0xA0, 0xFF }, /* 23: A */
    { 0xFF, 0x70, 0x70 }, /* 24: G */
    { 0x80, 0xFF, 0xFF }, /* 25: I */
    { 0xFF, 0x8C, 0x4B }, /* 26: C */
    { 0xA0, 0xFF, 0xA0 }, /* 27: T */
    { 0xFF, 0x80, 0x80 }  /* 28: U */
  };

  ResidueColor::ResidueColor() : m_settingsWidget(NULL), m_colorScheme(0)
  { }

  ResidueColor::~ResidueColor()
  {
    if (m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  void ResidueColor::setFromPrimitive(const Primitive *primitive)
  {
    if (!primitive)
      return;

    Primitive *p = const_cast<Primitive *>(primitive);
    Residue *residue = NULL;
    QString residueName; // this colors by residue name

    if (p->type() == Primitive::ResidueType) {
      residue = static_cast<Residue*>(p);
      if (!residue)
        return; // can't color this
      residueName = residue->name();
    } else if (p->type() == Primitive::AtomType) {
      Atom *atom = static_cast<Atom*>(p);
      if (atom)
        residue = atom->residue();

      // default is to color by element if no residue is specified
      std::vector<double> rgb = OpenBabel::etab.GetRGB( atom->atomicNumber() );

      if (!residue) {
        m_channels[0] = rgb[0];
        m_channels[1] = rgb[1];
        m_channels[2] = rgb[2];
        m_channels[3] = 1.0;
        return;
      }
      residueName = residue->name();
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
    } else if (residueName.compare("A", Qt::CaseInsensitive) == 0) {
      offset = 23;
    } else if (residueName.compare("G", Qt::CaseInsensitive) == 0) {
      offset = 24;
    } else if (residueName.compare("I", Qt::CaseInsensitive) == 0) {
      offset = 25;
    } else if (residueName.compare("C", Qt::CaseInsensitive) == 0) {
      offset = 26;
    } else if (residueName.compare("T", Qt::CaseInsensitive) == 0) {
      offset = 27;
    } else if (residueName.compare("U", Qt::CaseInsensitive) == 0) {
      offset = 28;
    } else {
      offset = 22;
    }

    if (m_colorScheme == 1) {
      m_channels[0] = jMolShapely[offset][0] / 255.0;
      m_channels[1] = jMolShapely[offset][1] / 255.0;
      m_channels[2] = jMolShapely[offset][2] / 255.0;
    } else if (m_colorScheme == 2) {
      m_channels[0] = hydrophobicity[offset][0] / 255.0;
      m_channels[1] = hydrophobicity[offset][1] / 255.0;
      m_channels[2] = hydrophobicity[offset][2] / 255.0;
    } else {
      m_channels[0] = jMolAmino[offset][0] / 255.0;
      m_channels[1] = jMolAmino[offset][1] / 255.0;
      m_channels[2] = jMolAmino[offset][2] / 255.0;
    }
    m_channels[3] = 1.0;
  }

  void ResidueColor::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }

  void ResidueColor::setColorScheme(int scheme)
  {
    m_colorScheme = scheme;
    emit changed();
  }

  QWidget *ResidueColor::settingsWidget()
  {
    if (!m_settingsWidget) {
      m_settingsWidget = new ResidueColorSettingsWidget();
      connect(m_settingsWidget->colorStyleComboBox, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setColorScheme(int)));
      connect(m_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));
    }

    return m_settingsWidget;
  }

}

Q_EXPORT_PLUGIN2(residuecolor, Avogadro::ResidueColorFactory)

