/**********************************************************************
  GAMESSUKInputDialog - Dialog for generating GAMESS-UK input decks

  Copyright (C) 2010 Jens Thomas
  Copyright (C) 2008-2009 Marcus D. Hanwell
  Copyright (C) 2009 David C. Lonie

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

#ifndef GAMESSUKINPUTDIALOG_H
#define GAMESSUKINPUTDIALOG_H

#include "inputdialog.h"
#include "ui_gamessukinputdialog.h"

namespace Avogadro
{
  class Molecule;
  class GAMESSUKInputDialog : public InputDialog
  {
  Q_OBJECT

  public:
    explicit GAMESSUKInputDialog(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~GAMESSUKInputDialog();

    void setMolecule(Molecule *molecule);
    void readSettings(QSettings&);
    void writeSettings(QSettings&) const;

    enum calculationType{SP, OPT, TSS, FREQ};
    enum theoryType{RHF, DFT, MP2};
    enum basisType{STO3G, p321G, p631G, p631Gs, ccpVDZ, ccpVTZ};
    enum dftFunctionalType{SVWN, BLYP, B3LYP, B97, HCTH, FT97};
    enum coordType{CARTESIAN, ZMATRIX};

  protected:
    /**
     * Reimplemented to update the dialog when it is shown
     */
    void showEvent(QShowEvent *event);

  private:
    Ui::GAMESSUKInputDialog ui;
//    Molecule* m_molecule;

    // Internal data structure for the calculation
    //QString m_title;
    calculationType m_calculationType;
    theoryType m_theoryType;
    basisType m_basisType;
    dftFunctionalType m_dftFunctionalType;
    bool m_direct;
    coordType m_coordType;
    QString m_output;
    bool m_dirty;
    bool m_warned;

    // Generate an input deck as a string
    QString generateInputDeck();
    // Translate enums to strings
    QString getRunType(calculationType t);
    QString getScfType(theoryType t);
    QString getBasisType(basisType t);

    // Enable/disable form elements
    void deckDirty(bool);

  public Q_SLOTS:
    void updatePreviewText();

  private Q_SLOTS:
    //! Button Slots
    void resetClicked();
    void generateClicked();
    void enableFormClicked();
    void moreClicked();
    void previewEdited();

    void setTitle();
    void setCalculation(int);
    void setTheory(int);
    void setBasis(int);
    void setDftFunctional(int);
    void setMultiplicity(int);
    void setCharge(int);
    void setDirect(int);
    void setCoords(int);
  };
}

#endif
