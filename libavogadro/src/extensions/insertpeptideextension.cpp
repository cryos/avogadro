/**********************************************************************
  InsertPeptide - Insert oligopeptide sequences

  Copyright (C) 2008-2009 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "insertpeptideextension.h"
#include "insertcommand.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>

#include <openbabel/mol.h>
#include <openbabel/residue.h>
#include <openbabel/atom.h>

#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  void AddResidue(string residue, bool lStereo, 
                  OBMol &mol, vector<OBInternalCoord*> &vic,
                  const char chain);

  InsertPeptideExtension::InsertPeptideExtension(QObject *parent) : 
    Extension(parent),
    m_molecule(0),
    phi(180.0), psi(180.0), omega(179.99),
    lStereo(true),
    structureType(0),
    m_dialog(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Peptide..."));
    m_actions.append(action);

    m_widget = qobject_cast<GLWidget *>(parent);
  }

  InsertPeptideExtension::~InsertPeptideExtension()
  {
    if (m_dialog) {
      delete m_dialog;
      m_dialog = 0;
    }
  }

  QList<QAction *> InsertPeptideExtension::actions() const
  {
    return m_actions;
  }

  QString InsertPeptideExtension::menuPath(QAction *) const
  {
    return tr("&Build") + '>' + tr("&Insert");
  }

  void InsertPeptideExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* InsertPeptideExtension::performAction(QAction *, 
                                                       GLWidget *widget)
  {
    if (m_molecule == NULL)
      return NULL; // nothing we can do

    m_widget = widget; // save for delayed response
    
    if (m_dialog == NULL) {
      constructDialog();
    }
    m_dialog->show();

    return NULL; // delayed action on user clicking the Insert button
  }

  void InsertPeptideExtension::performInsert()
  {
    if (!m_dialog)
      return; // nothing we can do

    OBMol obfragment;
    vector<OBInternalCoord*> vic;
    vic.push_back((OBInternalCoord*)NULL);
    OBInternalCoord* ic;
    int lastN, lastCa, lastCac, lastO; // backbone atoms
    lastN = lastCa = lastCac = lastO = 0;
    int newN, newCa, newCac, newO;
    int lastAtom = 0; // last atom read from previous residue
    
    double amideLength = 1.34;
    double bondAngle = 120.0;
    char chain = 'A';

    // Now the work begins
    // Get the sequence (in lower case)
    obfragment.BeginModify();
    QString sequence = m_dialog->sequenceText->toPlainText().toLower();
    foreach (const QString residue, sequence.split('-')) {
      AddResidue(residue.toStdString(), lStereo, obfragment, vic, chain);

      newN = lastAtom + 1;
      newCa = lastAtom + 2;
      newCac = lastAtom + 3;
      newO = lastAtom + 4;

      if (lastAtom != 0) {
        // set the peptide bond
        // first the nitrogen
        ic = vic[newN];
        ic->_a = obfragment.GetAtom(lastCac);
        ic->_dst = amideLength;
        ic->_b = obfragment.GetAtom(lastCa);
        ic->_ang = bondAngle;
        ic->_c = obfragment.GetAtom(lastN);
        ic->_tor = psi;
	
        // fix the O=C from previous residue
        ic = vic[lastO];
        ic->_tor = 180.0 + psi;

        // now the Calpha
        ic = vic[newCa];
        ic->_b = obfragment.GetAtom(lastCac);
        ic->_ang = bondAngle;
        ic->_c = obfragment.GetAtom(lastCa);
        ic->_tor = omega;

        // now the new C=O
        ic = vic[newCac];
        ic->_c = obfragment.GetAtom(lastCac);
        ic->_tor = phi;
        
        // add the peptide bond
        obfragment.AddBond(lastCac, newN, 1);
      }

      // add the known backbone bonds
      obfragment.AddBond(newN, newCa, 1);
      obfragment.AddBond(newCa, newCac, 1);
      obfragment.AddBond(newCac, newO, 2); // C=O

      lastN = newN;
      lastCa = newCa;
      lastCac = newCac;
      lastO = newO;
      lastAtom = obfragment.NumAtoms();
    }
    // Fix the final C=O if not straight-chain
    ic = vic[lastO];
    ic->_tor = 180.0 + psi;
    // "End group modification"
    // add the extra H to get NH2
    // need to add the new OH to get COOH

    obfragment.EndModify();
    InternalToCartesian(vic,obfragment);
    OBBitVec allAtoms;
    allAtoms.SetRangeOn(0, obfragment.NumAtoms());
    resdat.AssignBonds(obfragment, allAtoms);

    obfragment.SetPartialChargesPerceived();

    Molecule fragment;
    fragment.setOBMol(&obfragment);
    emit performCommand(new InsertFragmentCommand(m_molecule, fragment, m_widget, tr("Insert Peptide")));
  }

  void InsertPeptideExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
    settings.setValue("phiAngle", phi);
    settings.setValue("psiAngle", psi);
    settings.setValue("lStereo", lStereo);
    settings.setValue("structureType", structureType);
  }

  void InsertPeptideExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);

    phi = settings.value("phiAngle", 180.0).toDouble();
    psi = settings.value("psiAngle", 180.0).toDouble();
    lStereo = settings.value("lStereo", true).toBool();
    structureType = settings.value("structureType", 0).toInt();

    updateDialog();
  }

  void InsertPeptideExtension::constructDialog()
  {
    if (m_dialog == NULL) {
      m_dialog = new InsertPeptideDialog(m_widget);
      QButtonGroup* stereoGroup = new QButtonGroup(m_dialog);
      stereoGroup->addButton(m_dialog->dStereoButton, 0);
      stereoGroup->addButton(m_dialog->lStereoButton, 1);
      stereoGroup->setExclusive(true);

      connect(stereoGroup, SIGNAL(buttonClicked(int)), this, SLOT(setStereo(int)));

      connect(m_dialog->structureCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setStructureType(int)));
      connect(m_dialog->phiSpin, SIGNAL(valueChanged(double)), this, SLOT(setPhi(double)));
      connect(m_dialog->psiSpin, SIGNAL(valueChanged(double)), this, SLOT(setPsi(double)));
      connect(m_dialog->insertButton, SIGNAL(clicked()), this, SLOT(performInsert()));

      // Set the amino buttons to u[date the sequence
      foreach(QToolButton *child, m_dialog->findChildren<QToolButton*>()) {
        connect(child, SIGNAL(clicked()), this, SLOT(updateText()));
      }

    }
    m_dialog->sequenceText->setPlainText(QString());
    updateDialog();
  }

  void InsertPeptideExtension::updateDialog()
  {
    if (m_dialog == NULL)
      return; // the method will be called again when the dialog is created

    m_dialog->structureCombo->setCurrentIndex(structureType);
    m_dialog->phiSpin->setValue(phi);
    m_dialog->psiSpin->setValue(psi);
    if (lStereo)
      m_dialog->lStereoButton->setChecked(true);
    else
      m_dialog->dStereoButton->setChecked(true);
  }

  void InsertPeptideExtension::updateText()
  {
    QToolButton *button = qobject_cast<QToolButton*>(sender());
    if (button) {
      QString sequenceText = m_dialog->sequenceText->toPlainText();
      if (!sequenceText.isEmpty())
        sequenceText += '-'; // divider between amino acids
      
      sequenceText += button->text();

      m_dialog->sequenceText->setPlainText(sequenceText);
    }
  }

  void InsertPeptideExtension::setStereo(int stereoValue)
  {
    lStereo = stereoValue;
  }

  void InsertPeptideExtension::setPhi(double angle)
  {
    phi = angle;
  }

  void InsertPeptideExtension::setPsi(double angle)
  {
    psi = angle;
  }

  void InsertPeptideExtension::setStructureType(int type)
  {
    structureType = type;

    switch (type) {
    case 0: // straight chain
      setPhi(180.0);
      setPsi(180.0);
      m_dialog->phiSpin->setValue(phi);
      m_dialog->psiSpin->setValue(psi);
      break;
    case 1: // alpha helix
      setPhi(-60.0);
      setPsi(-40.0);
      m_dialog->phiSpin->setValue(phi);
      m_dialog->psiSpin->setValue(psi);
      break;
    case 2: // beta sheet
      setPhi(-135.0);
      setPsi(135.0);
      m_dialog->phiSpin->setValue(phi);
      m_dialog->psiSpin->setValue(psi);
      break;
    default: // arbitrary value
      break;
    }
  }

  string Uppercase(const string &residue)
  {
    string upperCase(residue);
    std::transform(upperCase.begin(), upperCase.end(), upperCase.begin(),
                   ::toupper);
    return upperCase;
  }
  
  void AddResidue(string residue, bool lStereo, 
                  OBMol &mol, vector<OBInternalCoord*> &vic,
                  const char chain)
  {
    QString filename;
    filename = QCoreApplication::applicationDirPath() + "/../share/avogadro/builder/amino/";

    if (residue != "Gly") {
      if (lStereo)
        filename += "L-";
      else // D stereo
        filename += "D-";
    }
    filename += residue.c_str();
    filename += ".zmat";
    
    ifstream ifs;
    ifs.open(filename.toAscii());
    
    if (!ifs) // file doesn't exist
      return;
    
    // Offset:
    //  When we add the internal coordinates, we have to increment
    //  based on the size of the molecule so far
    unsigned int offset = mol.NumAtoms();
    
    // setup the parent residue
    // TODO -- this doesn't seem to set all the fields properly
    // (maybe we need to set atom types?)
    int prevRes = mol.NumResidues() + 1;
    OBResidue *res = mol.NewResidue();
    res->SetNum(prevRes);
    res->SetChain(chain);
    // needs to be in uppercase
    res->SetName(Uppercase(residue));
    
    // Read in an amino z-matrix
    // similar to MOPAC zmat format
    char buffer[BUFF_SIZE];
    vector<string> vs;
    OBAtom *atom;
    
    while (ifs.getline(buffer, BUFF_SIZE)) {
      tokenize(vs, buffer);
      
      atom = mol.NewAtom();
      atom->SetAtomicNum(etab.GetAtomicNum(vs[0].c_str()));
      atom->SetPartialCharge(atof(vs[7].c_str()));
      res->AddAtom(atom);
      res->SetHetAtom(atom, false);
      res->SetSerialNum(atom, mol.NumAtoms());
      if (vs.size() == 9)
        res->SetAtomID(atom, vs[8]);
      
      OBInternalCoord *coord = new OBInternalCoord;
      coord->_dst = atof(vs[1].c_str());
      coord->_ang = atof(vs[2].c_str());
      coord->_tor = atof(vs[3].c_str());
      
      int index;
      // Set _a
      index = atoi(vs[4].c_str());
      if (index > 0 && index <= mol.NumAtoms())
        coord->_a = mol.GetAtom(index + offset);
      else
        coord->_a = NULL;
      // Set _b
      index = atoi(vs[5].c_str());
      if (index > 0 && index <= mol.NumAtoms())
        coord->_b = mol.GetAtom(index + offset);
      else
        coord->_b = NULL;
      // Set _c
      index = atoi(vs[6].c_str());    
      if (index > 0 && index <= mol.NumAtoms())
        coord->_c = mol.GetAtom(index + offset);
      else
        coord->_c = NULL;
      
      vic.push_back(coord);
    }
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(insertpeptideextension, Avogadro::InsertPeptideExtensionFactory)

