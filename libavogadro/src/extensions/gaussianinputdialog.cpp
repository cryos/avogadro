/**********************************************************************
  GaussianInputDialog - Dialog for generating Gaussian input decks

  Copyright (C) 2008 Marcus D. Hanwell

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

#include "gaussianinputdialog.h"

#include <QString>
#include <QTextStream>
#include <QFileDialog>

using namespace OpenBabel;

namespace Avogadro
{

  GaussianInputDialog::GaussianInputDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_molecule(0), m_calculationType(OPT), m_theoryType(B3LYP),
    m_basisType(B631Gd), m_multiplicity(1), m_charge(0), m_procs(1)
  {
    ui.setupUi(this);
    connect(ui.calculationCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setCalculation(int)));
    connect(ui.theoryCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setTheory(int)));
    connect(ui.basisCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setBasis(int)));
    connect(ui.multiplicityCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setMultiplicity(int)));
    connect(ui.chargeCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setCharge(int)));
    connect(ui.procSpin, SIGNAL(valueChanged(int)),
        this, SLOT(setProcs(int)));
    connect(ui.generateButton, SIGNAL(clicked()),
        this, SLOT(generateClicked()));

    updatePreviewText();
  }

  GaussianInputDialog::~GaussianInputDialog()
  {
  }

  void GaussianInputDialog::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  void GaussianInputDialog::updatePreviewText()
  {
    // Generate the input deck and display it
    ui.previewText->setText(generateInputDeck());
  }

  void GaussianInputDialog::resetClicked() {}
  void GaussianInputDialog::generateClicked()
  {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Gaussian Input Deck"),
                                "", tr("Gaussian Input Deck (*.com)"));
    QFile file(fileName);
    // FIXME This really should pop up a warning if the file cannot be opened
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;

    QTextStream out(&file);
    out << generateInputDeck();
  }

  void GaussianInputDialog::setCalculation(int n)
  {
    switch (n)
    {
      case 0:
        m_calculationType = SP;
        break;
      case 1:
        m_calculationType = OPT;
        break;
      case 2:
        m_calculationType = FREQ;
        break;
      default:
        m_calculationType = SP;
    }
    updatePreviewText();
  }

  void GaussianInputDialog::setTheory(int n)
  {
    switch (n)
    {
      case 0:
        m_theoryType = AM1;
        break;
      case 1:
        m_theoryType = PM3;
        break;
      case 2:
        m_theoryType = RHF;
        break;
      case 3:
        m_theoryType = B3LYP;
        break;
      case 4:
        m_theoryType = MP2;
        break;
      case 5:
        m_theoryType = CCSD;
        break;
      default:
        m_theoryType = RHF;
    }
    updatePreviewText();
  }

  void GaussianInputDialog::setBasis(int n)
  {
    switch (n)
    {
      case 0:
        m_basisType = STO3G;
        break;
      case 1:
        m_basisType = B321G;
        break;
      case 2:
        m_basisType = B631Gd;
        break;
      case 3:
        m_basisType = B631Gdp;
        break;
      default:
        m_basisType = B631Gd;
    }
    updatePreviewText();
  }

  void GaussianInputDialog::setMultiplicity(int n)
  {
    switch (n)
    {
      case 0: // singlet
        m_multiplicity = 1;
        break;
      case 1: // doublet
        m_multiplicity = 2;
        break;
      case 2: // triplet
        m_multiplicity = 3;
        break;
      default:
        m_multiplicity = 1;
    }
    updatePreviewText();
  }
  void GaussianInputDialog::setCharge(int n)
  {
    switch (n)
    {
      case 0: // dication
        m_charge = -2;
        break;
      case 1: // cation
        m_charge = -1;
        break;
      case 2: // neutral
        m_charge = 0;
        break;
      case 3: // anion
        m_charge = 1;
        break;
      case 4: // dianion
        m_charge = 2;
        break;
      default:
        m_charge = 0;
    }
    updatePreviewText();
  }

  void GaussianInputDialog::setProcs(int n)
  {
    if (n > 0)
      m_procs = n;
    updatePreviewText();
  }

  QString GaussianInputDialog::generateInputDeck()
  {
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);

    // These directives are required before the job specification
    if (m_procs > 1)
      mol << "%NProcShared=" << m_procs << "\n";

    // Now specify the job type etc
    mol << "#N " << getTheoryType(m_theoryType) << "/"
        << getBasisType(m_basisType) << " "
        << getCalculationType(m_calculationType) << "\n\n";

    // Title line
    mol << " Title\n\n";

    // Now for the charge and multiplicity
    mol << m_charge << " " << m_multiplicity << "\n";

    // Now to output the actual molecular coordinates
    //Atom* atom;
    if (m_molecule)
    {
      QTextStream mol(&buffer);
      FOR_ATOMS_OF_MOL(atom, m_molecule)
      {
        mol << qSetFieldWidth(3) << left << QString(etab.GetSymbol(atom->GetAtomicNum()))
            << qSetFieldWidth(15) << qSetRealNumberPrecision(5) << forcepoint
            << fixed << right << atom->GetX() << atom->GetY() << atom->GetZ()
            << qSetFieldWidth(0) << "\n";
      }
      mol << "\n";
    }

    return buffer;
  }

  QString GaussianInputDialog::getCalculationType(calculationType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case SP:
        return "SP";
      case OPT:
        return "Opt";
      case FREQ:
        return "Freq";
      default:
        return "SP";
    }
  }

  QString GaussianInputDialog::getTheoryType(theoryType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case AM1:
        return "AM1";
      case PM3:
        return "PM3";
      case RHF:
        return "RHF";
      case B3LYP:
        return "B3LYP";
      case MP2:
        return "MP2";
      case CCSD:
        return "CCSD";
      default:
        return "RHF";
    }
  }

  QString GaussianInputDialog::getBasisType(basisType t)
  {
    // Translate the enum to text for the output generation
    switch (t)
    {
      case STO3G:
        return "STO-3G";
      case B321G:
        return "321-G";
      case B631Gd:
        return "6-31G(d)";
      case B631Gdp:
        return "6-31G(d,p)";
      default:
        return "6-31G(d)";
    }
  }

}

#include "gaussianinputdialog.moc"
