/**********************************************************************
  Psi4InputDialog - Dialog for generating Psi4 input decks

  Copyright (C) 2012 Matthew R. Kennedy

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

#ifndef PSI4INPUTDIALOG_H
#define PSI4INPUTDIALOG_H

#include <avogadro/glwidget.h>

#include "inputdialog.h"
#include "ui_psi4inputdialog.h"

namespace Avogadro
{
  class Psi4InputDialog : public InputDialog
  {
  Q_OBJECT

  public:
    explicit Psi4InputDialog(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~Psi4InputDialog();

    void setMolecule(Molecule *molecule);
    void readSettings(QSettings&);
    void writeSettings(QSettings&) const;

    enum calculationType{energy, optimize, frequencies};
    enum theoryType{SCF, SAPT0, SAPT2, B3LYPD, B97D, M052X, MP2, CCSD, CCSDT};
    enum basisType{STO3G, junDZ, ccpVDZ, augccpVDZ, ccpVTZ};

  protected:
    /**
     * Reimplemented to update the dialog when it is shown
     */
    void showEvent(QShowEvent *event);

  private:
    Ui::Psi4InputDialog ui;

    // Internal data structure for the calculation
    calculationType m_calculationType;
    theoryType m_theoryType;
    basisType m_basisType;
    QString m_output;
    coordType m_coordType;
    bool m_dirty;
    bool m_warned;

    // Generate an input deck as a string
    QString generateInputDeck();
    // Translate enums to strings
    QString getCalculationType(calculationType t);
    QString getTheoryType(theoryType t);
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
    void previewEdited();

    void setTitle();
    void setCalculation(int);
    void setTheory(int);
    void setBasis(int);
    void setMultiplicity(int);
    void setCharge(int);
  };
}

#endif
