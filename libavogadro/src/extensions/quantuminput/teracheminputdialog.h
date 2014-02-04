/**********************************************************************
  TeraChemInputDialog - Dialog for generating TeraChem input decks

  Copyright (C) 2012 Albert DeFusco
     copied from qmcheminputdialog.h

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#ifndef TERACHEMINPUTDIALOG_H
#define TERACHEMINPUTDIALOG_H

#include "inputdialog.h"
#include "ui_teracheminputdialog.h"

namespace Avogadro
{
  class Molecule;
  class TeraChemInputDialog : public InputDialog
  {
  Q_OBJECT

  public:
    explicit TeraChemInputDialog(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~TeraChemInputDialog();

    void setMolecule(Molecule *molecule);
    void readSettings(QSettings&);
    void writeSettings(QSettings&) const;

    enum calculationType{SP, GRAD, OPT};
    enum theoryType{HF, BLYP, B3LYP, B3LYP1, B3LYP5, PBE, REVPBE};
    enum basisType{STO3G, B321G, B631Gd, B631Gdp, B631plusGd, B6311Gd, ccpVDZ};
    enum coordType{PDB, XYZ};
    enum dispType{NO, YES, D2, D3};

  protected:
    /**
     * Reimplemented to update the dialog when it is shown
     */
    void showEvent(QShowEvent *event);

  private:
    Ui::TeraChemInputDialog ui;
    //Molecule* m_molecule;

    // Internal data structure for the calculation
    //QString m_title;
    calculationType m_calculationType;
    theoryType m_theoryType;
    basisType m_basisType;
    coordType m_coordType;
    dispType m_dispType;
    //int m_multiplicity;
    //int m_charge;
    QString m_output;
    bool m_unrestricted;

    bool m_dirty;
    bool m_warned;

    // Generate an input deck as a string
    QString generateInputDeck();
    // Translate enums to strings
    QString getCalculationType(calculationType t);
    QString getTheoryType(theoryType t);
    QString getBasisType(basisType t);
    QString getCoordType(coordType t);
    QString getDispType(dispType t);

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
    void setMultiplicity(int);
    void setCharge(int);
    void setUnrestricted(bool);
    void setCoordType(int);
    void setDispType(int);
  };
}

#endif
