/**********************************************************************
  InsertPolymer - Build polymers and co-polymers, including custom units

  Copyright (C) 2012 Geoffrey R. Hutchison

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

#include "insertpolymerextension.h"
#include "insertcommand.h"

// Defines INSTALL_PREFIX among other things
#include "config.h" // krazy:exclude=includes

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>

#include <openbabel/mol.h>
#include <openbabel/builder.h>
#include <openbabel/obconversion.h>
#include <openbabel/forcefield.h>
#include <openbabel/rand.h>

#include <Eigen/Geometry>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QtWebKit/QWebFrame>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  class InsertPolymerDialog : public QDialog, public Ui::InsertPolymerDialog
  {
  public:
    InsertPolymerDialog(QWidget *parent=0) : QDialog(parent) {
      setWindowFlags(Qt::Dialog | Qt::Tool);
      setupUi(this);
    }
  };

  InsertPolymerExtension::InsertPolymerExtension(QObject *parent) :
    Extension(parent),
    m_molecule(0),
    m_dialog(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Polymer..."));
    m_actions.append(action);

    m_widget = qobject_cast<GLWidget *>(parent);
    // use system random -- on many systems, this is sufficient
    m_random = new OBRandom(true);
    m_random->TimeSeed();

    m_FF = OBForceField::FindForceField("UFF");
    // Since we only want a rough geometry, use distance cutoffs for VDW, Electrostatics
    m_FF->EnableCutOff(true);
    m_FF->SetVDWCutOff(10.0);
    m_FF->SetElectrostaticCutOff(20.0);
    m_FF->SetUpdateFrequency(10); // update non-bonded distances

#ifdef Q_WS_X11
    m_directory = QString( INSTALL_PREFIX ) + "/share/avogadro/polymers";
#else
    // Mac and Windows use relative path from application location
    m_directory = QCoreApplication::applicationDirPath() + "/../share/avogadro/polymers";
#endif
  }

  InsertPolymerExtension::~InsertPolymerExtension()
  {
    delete m_random;
  }

  QList<QAction *> InsertPolymerExtension::actions() const
  {
    return m_actions;
  }

  QString InsertPolymerExtension::menuPath(QAction *) const
  {
    return tr("&Build") + '>' + tr("&Insert");
  }

  void InsertPolymerExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* InsertPolymerExtension::performAction(QAction *, // only one action
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

  void InsertPolymerExtension::constructDialog()
  {
    if (!m_dialog) {
      m_dialog = new InsertPolymerDialog(qobject_cast<QWidget*>(parent()));

      // hidden fields to track the folder path
      m_dialog->monomerAType->setVisible(false);
      m_dialog->monomerBType->setVisible(false);
      m_dialog->monomerCType->setVisible(false);

      // Connect some signals and slots
      connect(m_dialog->monomerAButton, SIGNAL(clicked(bool)),
              this, SLOT(chooseMonomerA()));
      connect(m_dialog->monomerBButton, SIGNAL(clicked(bool)),
              this, SLOT(chooseMonomerB()));
      connect(m_dialog->monomerCButton, SIGNAL(clicked(bool)),
              this, SLOT(chooseMonomerC()));

      connect(m_dialog->aRepeatSpinBox, SIGNAL(valueChanged(int)),
              this, SLOT(validateMonomerRepeats(int)));
      connect(m_dialog->bRepeatSpinBox, SIGNAL(valueChanged(int)),
              this, SLOT(validateMonomerRepeats(int)));
      connect(m_dialog->cRepeatSpinBox, SIGNAL(valueChanged(int)),
              this, SLOT(validateMonomerRepeats(int)));
      connect(m_dialog->monomerRepeatStyle, SIGNAL(currentIndexChanged(int)),
              this, SLOT(validateMonomerRepeats(int)));

      connect(m_dialog->monomerAName, SIGNAL(editingFinished()),
              this, SLOT(updateMonomerGraphics()));
      connect(m_dialog->monomerBName, SIGNAL(editingFinished()),
              this, SLOT(updateMonomerGraphics()));
      connect(m_dialog->monomerCName, SIGNAL(editingFinished()),
              this, SLOT(updateMonomerGraphics()));

      connect(m_dialog->buildPolymerButton, SIGNAL(clicked(bool)),
              this, SLOT(build()));
    }
  }

  void InsertPolymerExtension::validateMonomerRepeats(int newValue)
  {
    if (!m_dialog)
      return; // nothing to do

    int repeatStyle = m_dialog->monomerRepeatStyle->currentIndex();
    // 0 == percent, 1 == repeat units
    if (repeatStyle != 0)
      return; // TODO: if repeats set to 0, hide/disable components

    // OK, we've got percents, so update/validate accordingly
    int whichMonomerChanged = 0;
    QString senderName(this->sender()->objectName());
    if (senderName.contains("cRepeat"))
      whichMonomerChanged = 3;
    else if (senderName.contains("bRepeat"))
      whichMonomerChanged = 2;
    else
      whichMonomerChanged = 1; // A

    // If we have one component
    if ((!m_dialog->bRepeatSpinBox->isEnabled() || m_dialog->bRepeatSpinBox->value() == 0)
        && newValue != 100) {
      m_dialog->aRepeatSpinBox->setValue(100);
      m_dialog->bRepeatSpinBox->setValue(0);
      m_dialog->cRepeatSpinBox->setValue(0);
      return;
    }

    // If we have 2 components
    if (!m_dialog->cRepeatSpinBox->isEnabled() || m_dialog->cRepeatSpinBox->value() == 0)
      {
        int remainingPercent = 100 - newValue;
        // set the other component to 100 - newValue
        // NOTE: we check the current value to stop any infinite loops
        if (whichMonomerChanged == 1 && m_dialog->bRepeatSpinBox->value() != remainingPercent) // A
          m_dialog->bRepeatSpinBox->setValue(remainingPercent);

        if (whichMonomerChanged == 2 && m_dialog->aRepeatSpinBox->value() != remainingPercent) // A
          m_dialog->aRepeatSpinBox->setValue(remainingPercent);
        m_dialog->cRepeatSpinBox->setValue(0);
      }
    // We have 3 components, so we can only really check if the total is 100%
    int totalPercent = 0;
    totalPercent += m_dialog->aRepeatSpinBox->value();
    totalPercent += m_dialog->bRepeatSpinBox->value();
    totalPercent += m_dialog->cRepeatSpinBox->value();
    if (totalPercent != 100) {
      m_dialog->aRepeatSpinBox->setStyleSheet("color: red");
      m_dialog->bRepeatSpinBox->setStyleSheet("color: red");
      m_dialog->cRepeatSpinBox->setStyleSheet("color: red");
    }
    else {
      m_dialog->aRepeatSpinBox->setStyleSheet("color: black");
      m_dialog->bRepeatSpinBox->setStyleSheet("color: black");
      m_dialog->cRepeatSpinBox->setStyleSheet("color: black");
    }
  }

  // helper function
  QString InsertPolymerExtension::getSmilesFromFile(QString type, QString name)
  {
    // Check to see if this is a valid file, read it
    QFile monomerFile(m_directory + '/' + type + '/' + name + ".smi");
    //    qDebug() << " get SMILES from: " << monomerFile.fileName();
    if (monomerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      // Each SMILES file should only be one line
      QString smiles = monomerFile.readLine();
      return smiles.trimmed();
    }
    // OK, now see if "name" is really a SMILES
    return name.trimmed();
  }

  QString clean(QString smiles)
  {
    QString cleanedSMI = smiles;
    if (cleanedSMI.endsWith('/'))
      cleanedSMI.remove(cleanedSMI.length());
    if (cleanedSMI.endsWith('\\'))
      cleanedSMI.remove(cleanedSMI.length());
    if (cleanedSMI.endsWith('='))
      cleanedSMI.remove(cleanedSMI.length());

    return cleanedSMI;
  }

  void InsertPolymerExtension::updateGraphics(QString smiles, QWebView *target)
  {
    OBMol monomer;
    OBConversion conv;
    if (!conv.SetInAndOutFormats("smi", "svg"))
      return; // can't do the depiction

    conv.AddOption("t",OBConversion::OUTOPTIONS);
    conv.AddOption("s",OBConversion::OUTOPTIONS);
    conv.AddOption("A",OBConversion::OUTOPTIONS);
    // Set the fixed size of the SVG
    int width = target->width();
    int height = target->height();
    QString pixels = QString("%1").arg(qMin(width, height));
    conv.AddOption("P",OBConversion::OUTOPTIONS, pixels.toAscii());

    QString cleaned = clean('*' + smiles + '*');
    if (conv.ReadString(&monomer, cleaned.toStdString())
        && monomer.NumAtoms() != 0) {
      QString svg = QString::fromStdString(conv.WriteString(&monomer, true));
      // Change the "Xx" from the "*" SMILES to an actual star
      // And bump up the font size
      svg.replace("font-size=\"16\" >Xx", "font-size=\"32\" >&#8727;");

      target->setContent(svg.toAscii(), "image/svg+xml");
      QWebFrame *webFrame = target->page()->mainFrame();
      webFrame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
      webFrame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    }
  }

  void InsertPolymerExtension::updateMonomerGraphics()
  {
    if (!m_widget || !m_molecule) {
      m_widget = GLWidget::current();
      m_molecule = m_widget->molecule();
    }

    if (m_dialog == 0 || m_molecule == 0)
      return;

    QString monomerASMI, monomerBSMI, monomerCSMI;
    // Get A repeat unit
    if (!m_dialog->monomerAName->text().isEmpty()) {
      monomerASMI = getSmilesFromFile(m_dialog->monomerAType->text(),
                                      m_dialog->monomerAName->text());
    }
    else
      return; // no monomer, nothing to do

    if (m_dialog->bRepeatSpinBox->value() != 0) {
      monomerBSMI = getSmilesFromFile(m_dialog->monomerBType->text(),
                                      m_dialog->monomerBName->text());
    }

    if (m_dialog->cRepeatSpinBox->value() != 0) {
      monomerCSMI = getSmilesFromFile(m_dialog->monomerCType->text(),
                                      m_dialog->monomerCName->text());
    }

    // OK, now the fun, let's generate some graphics!
    updateGraphics(monomerASMI, m_dialog->monomerAGraphics);
    if (!monomerBSMI.isEmpty())
      updateGraphics(monomerBSMI, m_dialog->monomerBGraphics);
    if (!monomerCSMI.isEmpty())
      updateGraphics(monomerCSMI, m_dialog->monomerCGraphics);
  }

void InsertPolymerExtension::build()
{
  if (!m_widget || !m_molecule) {
    m_widget = GLWidget::current();
    m_molecule = m_widget->molecule();
  }

  if (m_dialog == 0 || m_molecule == 0)
    return;

  QString monomerASMI, monomerBSMI, monomerCSMI;
  // Get A repeat unit
  if (!m_dialog->monomerAName->text().isEmpty()) {
    monomerASMI = getSmilesFromFile(m_dialog->monomerAType->text(),
                                    m_dialog->monomerAName->text());
  }
  else
    return; // no monomer, nothing to do

  m_dialog->buildPolymerButton->setEnabled(false); // Indicate you can't do anything until we finish
  m_dialog->buildPolymerButton->setText(tr("Working..."));
  QApplication::setOverrideCursor( Qt::WaitCursor );

  if (m_dialog->bRepeatSpinBox->value() != 0) {
    monomerBSMI = getSmilesFromFile(m_dialog->monomerBType->text(),
                                    m_dialog->monomerBName->text());
  }

  if (m_dialog->cRepeatSpinBox->value() != 0) {
    monomerCSMI = getSmilesFromFile(m_dialog->monomerCType->text(),
                                    m_dialog->monomerCName->text());
  }

  // build the polymer, repeat-by-repeat
  QString fullSMI, currentSMI;
  OBMol polymer;
  OBConversion conv;
  conv.SetInFormat("smi"); // parse as SMILES

  int aRepeats = m_dialog->aRepeatSpinBox->value();
  int bRepeats = m_dialog->bRepeatSpinBox->value();
  int cRepeats = m_dialog->cRepeatSpinBox->value();
  double aPercent = aRepeats / 100.0;
  double bPercent = bRepeats / 100.0;
  //    double cPercent = cRepeats / 100.0;
  int currentBlock = 0;
  int totalRepeats = m_dialog->totalLengthSpinBox->value();
  bool limitByMass = (m_dialog->totalLengthUnits->currentIndex() == 1);
  bool statisticalPolymer = (m_dialog->monomerRepeatStyle->currentIndex() == 0);
  int currentRepeatCount = 0; // for block counting
  bool switchTacticity = false;
  int tacticity = m_dialog->tacticityComboBox->currentIndex();
  QString nextMonomer;

  while (true) {
    if (!statisticalPolymer){
      // block copolymer
      if (currentBlock <= aRepeats)
        nextMonomer = monomerASMI;
      else if (currentBlock > aRepeats && currentBlock <= (aRepeats + bRepeats))
        nextMonomer = monomerBSMI;
      else
        nextMonomer = monomerCSMI;

      // Make sure to wrap around to
      currentBlock = (currentBlock + 1) % (aRepeats + bRepeats + cRepeats);
    } else {
      // random co-polymer
      // percents should be adjusted by user validation
      double random = m_random->NextFloat();
      if (random < aPercent)
        nextMonomer = monomerASMI;
      else if (random >= aPercent && random < (aPercent + bPercent))
        nextMonomer = monomerBSMI;
      else
        nextMonomer = monomerCSMI;
    }

    // Check if we need to switch stereo
    if (tacticity == 0) { // atactic
      double random = m_random->NextFloat();
      if (random < 0.5) {
        nextMonomer.replace(QLatin1String("@@"), QLatin1String("@"));
      }
    } else if (tacticity == 2) { // syndio
      if (switchTacticity)
        nextMonomer.replace(QLatin1String("@@"), QLatin1String("@")); // switch
      switchTacticity = !switchTacticity;
    }

    fullSMI += nextMonomer;
    qDebug() << " current polymer: " << fullSMI;
    currentSMI = clean(fullSMI); // clean this to check the progress

    polymer.Clear();
    if (!conv.ReadString(&polymer, currentSMI.toStdString())
        || polymer.NumAtoms() == 0)
      break; // no atoms, or failed validation

    // Check if we're past the limit
    if (limitByMass && polymer.GetMolWt() > totalRepeats)
      break; // we don't need to add hydrogens, since SMILES has implicit H
    else if (!limitByMass && currentRepeatCount++ > totalRepeats)
      break;
  }

  OBBuilder builder;
  Molecule fragment;

  // make sure we have a real fragment too
  if (!polymer.NumAtoms())
    return;

  builder.Build(polymer);
  polymer.AddHydrogens(); // once we have reasonable coordinates
  // Let's do a quick cleanup
  if (m_FF->Setup(polymer)) {
    m_FF->SteepestDescent(250, 1.0e-4);
    m_FF->UpdateCoordinates(polymer);
  }

  fragment.setOBMol(&polymer);
  fragment.center();

  if (fragment.numAtoms()) {
    emit performCommand(new InsertFragmentCommand(m_molecule, fragment,
                                                  m_widget, tr("Insert Polymer")));
  }
  m_dialog->buildPolymerButton->setEnabled(true); // Indicate you can't do anything until we finish
  m_dialog->buildPolymerButton->setText(tr("Insert Polymer"));
  QApplication::restoreOverrideCursor();
}

void InsertPolymerExtension::chooseMonomerA()
{
  QString fileName = QFileDialog::getOpenFileName(m_dialog, tr("Pick Monomer"),
                                                  m_directory);
  QFileInfo fileInfo(fileName);
  m_dialog->monomerAName->setText(fileInfo.baseName());
  m_dialog->monomerAType->setText(fileInfo.dir().dirName());

  m_dialog->monomerBButton->setEnabled(true);
  m_dialog->monomerBName->setEnabled(true);

  updateMonomerGraphics();
}

void InsertPolymerExtension::chooseMonomerB()
{
  QString fileName = QFileDialog::getOpenFileName(m_dialog, tr("Pick Monomer"),
                                                  m_directory);
  QFileInfo fileInfo(fileName);
  m_dialog->monomerBName->setText(fileInfo.baseName());
  m_dialog->monomerBType->setText(fileInfo.dir().dirName());

  m_dialog->bRepeatSpinBox->setValue(1);
  m_dialog->bRepeatSpinBox->setEnabled(true);
  m_dialog->monomerBGraphics->setEnabled(true);

  m_dialog->monomerCButton->setEnabled(true);
  m_dialog->monomerCName->setEnabled(true);

  updateMonomerGraphics();
}

void InsertPolymerExtension::chooseMonomerC()
{
  QString fileName = QFileDialog::getOpenFileName(m_dialog, tr("Pick Monomer"),
                                                  m_directory);
  QFileInfo fileInfo(fileName);
  m_dialog->monomerCName->setText(fileInfo.baseName());
  m_dialog->monomerCType->setText(fileInfo.dir().dirName());

  if(m_dialog->monomerRepeatStyle->currentIndex() != 0)
    // count, not percentage
    m_dialog->cRepeatSpinBox->setValue(1);
  m_dialog->cRepeatSpinBox->setEnabled(true);

  m_dialog->monomerCGraphics->setEnabled(true);

  updateMonomerGraphics();
}

void InsertPolymerExtension::dialogDestroyed()
{
  m_dialog = 0;
}

void InsertPolymerExtension::writeSettings(QSettings &settings) const
{
  Extension::writeSettings(settings);
  if (!m_dialog)
    return; // nothing to save

  settings.setValue("aRepeats", m_dialog->aRepeatSpinBox->value());
  settings.setValue("bRepeats", m_dialog->bRepeatSpinBox->value());
  settings.setValue("cRepeats", m_dialog->cRepeatSpinBox->value());
  settings.setValue("repeatStyle", m_dialog->monomerRepeatStyle->currentIndex());

  settings.setValue("totalLength", m_dialog->totalLengthSpinBox->value());
  settings.setValue("lengthUnits", m_dialog->totalLengthUnits->currentIndex());
  settings.setValue("tacticity", m_dialog->tacticityComboBox->currentIndex());
}

void InsertPolymerExtension::readSettings(QSettings &settings)
{
  Extension::readSettings(settings);

  if (!m_dialog) {
    constructDialog();
  }

  m_dialog->aRepeatSpinBox->setValue(settings.value("aRepeats", 1).toInt());
  m_dialog->bRepeatSpinBox->setValue(settings.value("bRepeats", 1).toInt());
  m_dialog->cRepeatSpinBox->setValue(settings.value("cRepeats", 1).toInt());
  m_dialog->monomerRepeatStyle->setCurrentIndex(settings.value("repeatStyle", 1).toInt());

  m_dialog->totalLengthSpinBox->setValue(settings.value("totalLength", 1).toInt());
  m_dialog->totalLengthUnits->setCurrentIndex(settings.value("lengthUnits", 0).toInt());
  m_dialog->tacticityComboBox->setCurrentIndex(settings.value("tacticity", 0).toInt());
}

}

Q_EXPORT_PLUGIN2(insertpolymerextension, Avogadro::InsertPolymerExtensionFactory)
