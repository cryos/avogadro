/**********************************************************************
  Polymer Tool - Make easy polymers, including custom monomer units

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

#include "polymertool.h"
#include "../extensions/insertcommand.h"

#include "ui_polymersettingswidget.h"

// Defines INSTALL_PREFIX among other things
#include "config.h" // krazy:exclude=includes

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/residue.h>
#include <avogadro/color.h>
#include <avogadro/glhit.h>
#include <avogadro/glwidget.h>
#include <avogadro/primitivelist.h>

#include <openbabel/mol.h>
#include <openbabel/builder.h>
#include <openbabel/obconversion.h>
#include <openbabel/forcefield.h>

#include <Eigen/Geometry>

#include <QDebug>
#include <QDirIterator>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  class PolymerSettingsWidget : public QWidget,
                                 public Ui::PolymerSettingsWidget
  {
  public:
    PolymerSettingsWidget(QWidget *parent=0) : QWidget(parent)
    {
      setupUi(this);
    }
  };

  PolymerTool::PolymerTool(QObject *parent) : Tool(parent),
                                              m_settingsWidget(0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/polymer/polymer.png")));
    action->setToolTip(tr("Polymer Tool"));
  }

  PolymerTool::~PolymerTool()
  {
    if(m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  int PolymerTool::usefulness() const
  {
    return 550000;
  }

  QUndoCommand* PolymerTool::mousePressEvent(GLWidget *widget, QMouseEvent *event)
  {
    m_movedSinceButtonPressed = false;
    m_doubleClick = false; // set true if we get a doubleClick event

    m_molecule = widget->molecule();
    if(!m_molecule) {
      return 0;
    }
    m_widget = widget;

    //! List of hits from initial click
    m_hits = widget->hits(event->pos().x()-2, event->pos().y()-2, 5, 5);

    // If there's a left button (and no modifier keys) continue adding to the list
    if(m_hits.size() && (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::NoModifier))
    {
      if(m_hits[0].type() != Primitive::AtomType)
        return 0;

      event->accept();

      Atom *atom = m_molecule->atom(m_hits[0].name());
      // First check if we've already selected this atom
      // Fixes PR#
      int indexOfAtom = m_selectedAtoms.indexOf(atom);
      if (indexOfAtom != -1) { // in the list
        m_numSelectedAtoms--; // update the count
        m_selectedAtoms.removeAt(indexOfAtom);
      }
      else { // new atom to add to list
        if(m_numSelectedAtoms < 4) {
          // Select another atom
          ++m_numSelectedAtoms;
          m_selectedAtoms.append(atom);
        }

      }
      widget->update();
    }
    // Right button or Left Button + modifier (e.g., Mac)
    else {
      // Clear all atoms
      event->accept();
      m_numSelectedAtoms = 0;
      m_selectedAtoms.clear();
      widget->update();
    }
    return 0;
  }

  QUndoCommand* PolymerTool::mouseReleaseEvent(GLWidget *widget,
                                                    QMouseEvent *event)
  {
    // Double clicks are handled in that event, not here
    if (m_doubleClick) {
      m_doubleClick = false;
    }

    return 0;
  }

  QUndoCommand* PolymerTool::mouseMoveEvent(GLWidget *widget, QMouseEvent *event)
  {
    return 0;
  }

  QUndoCommand* PolymerTool::mouseDoubleClickEvent(GLWidget *widget, QMouseEvent *event)
  {
    // @todo: set to build the polymer
    m_doubleClick = true; // events are handled here, not in release event
    build();
    return 0;
  }

  QUndoCommand* PolymerTool::wheelEvent(GLWidget*, QWheelEvent*)
  {
    return 0;
  }

  QStringList PolymerTool::monomerList()
  {
    if (m_monomerList.isEmpty()) {
      // build the list from disk

      QString directory, fileName;
#ifdef Q_WS_X11
      m_prefix = QString( INSTALL_PREFIX ) + "/share/avogadro/polymers/";
#else
      // Mac and Windows use relative path from application location
      m_prefix = QCoreApplication::applicationDirPath() + "/../share/avogadro/polymers/";
#endif

      QDirIterator it(m_prefix, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
      QFileInfo fileInfo;
      while (it.hasNext()) {
        fileInfo = it.fileInfo();
        if (fileInfo.isFile() && fileInfo.isReadable()) {
          // a real file!
          directory = fileInfo.path().split('/').last();
          fileName = fileInfo.baseName(); // strip off the ".smi" extension

          // add back the directory
          m_monomerList.append(directory + '/' + fileName);
        }
        it.next();
      }
    }

    return m_monomerList;
  }

  // helper function
  QString PolymerTool::getSmilesFromFile(QString fileName)
  {
    QString smiles;
    QFile monomerFile(m_prefix + '/' + fileName + ".smi");
    if (!monomerFile.open(QIODevice::ReadOnly | QIODevice::Text))
      return smiles;

    // Each SMILES file should only be one line
    smiles += monomerFile.readLine();
    return smiles.trimmed();
  }

  QUndoCommand* PolymerTool::build()
  {
    if (m_settingsWidget == 0 || m_molecule == 0)
      return 0;

    QString monomerAName(m_settingsWidget->monomerAComboBox->currentText());
    QString monomerASMI = getSmilesFromFile(monomerAName);

    QString monomerBName(m_settingsWidget->monomerBComboBox->currentText());
    QString monomerBSMI = getSmilesFromFile(monomerBName);

    qDebug() << monomerASMI << monomerBSMI;

    QString blockSMI, fullSMI;
    int aRepeats = m_settingsWidget->aRepeatSpinBox->value();
    int bRepeats = m_settingsWidget->bRepeatSpinBox->value();
    int totalRepeats = m_settingsWidget->totalRepeatSpinBox->value();

    // a * repeats
    blockSMI = monomerASMI.repeated(aRepeats) + monomerBSMI.repeated(bRepeats);
    fullSMI = blockSMI.repeated(totalRepeats);

    // check for trailing '/' or '\' stereo designators
    if (fullSMI.endsWith('/'))
      fullSMI.remove(fullSMI.length());
    if (fullSMI.endsWith('\\'))
      fullSMI.remove(fullSMI.length());

    qDebug() << fullSMI;

    OBBuilder builder;
    OBMol obfragment;
    OBConversion conv;
    Molecule fragment;

    std::string SmilesString(fullSMI.toAscii());
    if(conv.SetInFormat("smi") && conv.ReadString(&obfragment, SmilesString))
      {
        // make sure we have a real fragment too
        if (obfragment.NumAtoms()) {
          builder.Build(obfragment);
          qDebug() << " performing cleanup ";

          // Let's do a quick cleanup
          OBForceField* pFF =  OBForceField::FindForceField("MMFF94");
          if (pFF && pFF->Setup(obfragment)) {
            pFF->ConjugateGradients(250, 1.0e-4);
            pFF->UpdateCoordinates(obfragment);
          } // Note tricky assignment used as logic below
          else if ((pFF = OBForceField::FindForceField("UFF")) && pFF->Setup(obfragment)) {
            pFF->ConjugateGradients(250, 1.0e-4);
            pFF->UpdateCoordinates(obfragment);
          }

          fragment.setOBMol(&obfragment);
          fragment.addHydrogens(); // hydrogen addition is done by InsertCommand when bonding
          fragment.center();
        }
      }

    if (fragment.numAtoms()) {
      InsertFragmentCommand command(m_molecule, fragment, m_widget, tr("Insert Polymer"));
      command.redo();
    }

    return 0;
  }

  QWidget *PolymerTool::settingsWidget()
  {
    if (!m_settingsWidget) {
      m_settingsWidget = new PolymerSettingsWidget(qobject_cast<QWidget*>(parent()));

      m_settingsWidget->monomerAComboBox->addItems(monomerList());
      m_settingsWidget->monomerBComboBox->addItems(monomerList());

      // Connect some signals and slots
      connect(m_settingsWidget->monomerBComboBox, SIGNAL(currentIndexChanged(int)),
              this, SLOT(monomerBChanged(int)));

      connect(m_settingsWidget->buildPolymerButton, SIGNAL(clicked(bool)),
              this, SLOT(build()));
    }
    return m_settingsWidget;
  }

  void PolymerTool::monomerBChanged(int index)
  {
    if (index == 0) { // none
      // we should be saving the previous repeat number
      m_settingsWidget->bRepeatSpinBox->setValue(0);
      m_settingsWidget->bRepeatSpinBox->setEnabled(false);
    } else {
      // we should be saving and restoring values here
      m_settingsWidget->bRepeatSpinBox->setValue(1);
      m_settingsWidget->bRepeatSpinBox->setEnabled(true);
    }
  }

  bool PolymerTool::paint(GLWidget *)
  {
    return true;
  }

}

Q_EXPORT_PLUGIN2(polymertool, Avogadro::PolymerToolFactory)
