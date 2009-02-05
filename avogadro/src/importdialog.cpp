/**********************************************************************
  ImportDialog - Import Dialog

  Copyright (C) 2007-2008 Donald Ephraim Curtis

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

#include "importdialog.h"

#include "mainwindow.h"

#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QDebug>

#include <openbabel/babelconfig.h>
#include <openbabel/obconversion.h>
#include <openbabel/format.h>

using namespace OpenBabel;

namespace Avogadro {

  bool formatCompare(OBFormat *first, OBFormat *second)
  {
    return first->Description() < second->Description();
  }

  ImportDialog::ImportDialog(MainWindow *mainWindow) :
    QDialog(mainWindow),
    m_mainWindow(mainWindow),
    m_filename(""),
    m_currentFormat(0)
  {
    ui.setupUi(this);

    // Set up list of codes and descriptions
    OBConversion conv;
    OBFormat *pFormat;
    Formatpos pos;
    const char* str=NULL;

    // This loops through by unique extension
    // The catch is that some formats have multiple extensions
    // So some formats, we'll see them a few times
    while(OBConversion::GetNextFormat(pos,str,pFormat))
      {
        if(!pFormat || (pFormat->Flags() & NOTREADABLE))
          continue; // obviously we only care about readable formats

        if (!m_formatList.contains(pFormat)) {
          m_formatList.append(pFormat);
        }
      }

    qSort(m_formatList.begin(), m_formatList.end(), formatCompare);

    foreach(OBFormat *pFormat, m_formatList) {
      QString description(pFormat->Description());
      // There can be multiple lines in the description -- we only want one
      int lineEnding = description.indexOf('\n');
      if (lineEnding != -1)
        description.truncate(lineEnding);
      // remove any remaining initial or ending whitespace
      description = description.trimmed();

      ui.formatComboBox->addItem(description);
    }

    // reset the checkboxes to defaults
    ui.checkBoxBonding->setChecked(true);
    ui.checkBoxBondOrders->setChecked(true);
    ui.checkBoxAngstroms->setChecked(true);

    connect(ui.formatComboBox, SIGNAL(currentIndexChanged(int)),
	    this, SLOT(changedFormat(int)));
    connect(ui.checkBoxBonding, SIGNAL(stateChanged(int)),
	    this, SLOT(changedOptions(int)));
    connect(ui.selectButton, SIGNAL(clicked(bool)),
	    this, SLOT(updateFilename(bool)));
    connect(ui.dialogButtonBox, SIGNAL(clicked(QAbstractButton *)),
	    this, SLOT(buttonClicked(QAbstractButton *)));
  }

  void ImportDialog::buttonClicked(QAbstractButton *button)
  {
    QDialogButtonBox::ButtonRole role = ui.dialogButtonBox->buttonRole(button);
    if(role == QDialogButtonBox::ApplyRole || role == QDialogButtonBox::AcceptRole)
      accepted();
    else if (role == QDialogButtonBox::RejectRole)
      rejected();
  }

  void ImportDialog::accepted()
  {
    if (!m_mainWindow)
      return; // nothing we can do

    OBFormat *pFormat = NULL;
    int currentFormat = ui.formatComboBox->currentIndex() - 1;
    if (currentFormat >= 0) {
      pFormat = m_formatList[currentFormat];
    }
    QString options;
    if (ui.checkBoxBonding->isEnabled() && !ui.checkBoxBonding->isChecked())
      options = "b\n";
    else if ((ui.checkBoxBonding->isEnabled() && ui.checkBoxBonding->isChecked())
	     && (ui.checkBoxBondOrders->isEnabled() && !ui.checkBoxBondOrders->isChecked()))
      options = "s\n";

    if (ui.checkBoxAngstroms->isEnabled() && ui.checkBoxAngstroms->isChecked())
      options += "a\n";

    m_mainWindow->loadFile(ui.fileName->text(), pFormat, options);
  }

  void ImportDialog::rejected()
  {
    //    qDebug() << "close";
  }

  void ImportDialog::changedFormat(int formatIndex)
  {
    m_currentFormat = formatIndex;

    // reset the checkboxes to defaults
    ui.checkBoxBonding->setEnabled(true);
    ui.checkBoxBonding->setChecked(true);
    ui.checkBoxBondOrders->setEnabled(true);
    ui.checkBoxBondOrders->setChecked(true);
    ui.checkBoxAngstroms->setEnabled(true);
    ui.checkBoxAngstroms->setChecked(true);

    // need to hide a bunch of stuff for auto-detection

    if (formatIndex >= 1) // not automatic detection
      {
	QString description = m_formatList[formatIndex - 1]->Description();

	// Don't translate these search strings -- they are literals from Open Babel
	ui.checkBoxBonding->setEnabled(description.contains("Disable bonding entirely"));

	ui.checkBoxBondOrders->setEnabled(description.contains("Output single bonds only"));

	ui.checkBoxAngstroms->setEnabled(description.contains("Input in Angstroms"));
      }
  }

  void ImportDialog::changedOptions(int state)
  {
    Q_UNUSED(state);
    // if the bonding checkbox is set, we can set or unset bond orders
    ui.checkBoxBondOrders->setEnabled(ui.checkBoxBonding->isChecked());
  }

  void ImportDialog::updateFilename(bool)
  {
    m_filename = QFileDialog::getOpenFileName( this, tr("Open File") );
    ui.fileName->setText(m_filename);
  }

} // end namespace Avogadro

#include "importdialog.moc"
