/**********************************************************************
 QPeriodicTable - Periodic Table based on Avogadro

 Copyright (C) 2010 by Konstantin Tokarev

 This file is part of the Avogadro molecular editor project.
 For more information, see <http://avogadro.openmolecules.net/>

 Avogadro is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
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

#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMainWindow>

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QPointer>
#include <QtCore/QProcess>
#include <QtCore/QTranslator>

#include <openbabel/mol.h>

#include <qperiodictable/elementtranslator.h>
#include "qperiodictable.h"
#include "config.h"

using namespace Avogadro;

PeriodicTableWatcher::PeriodicTableWatcher(PeriodicTableView *periodicTable,
                        QPlainTextEdit *elementInfo)
{
  m_periodicTable = periodicTable;
  m_elementInfo = elementInfo;
  connect(m_periodicTable, SIGNAL(elementChanged(int)),
    this, SLOT(elementChanged(int)));
}

void PeriodicTableWatcher::elementChanged(int n)
{  
  if (n == 110)
    m_elementInfo->setPlainText("");  // prevent crash from OB
  else
    m_elementInfo->setPlainText(
                              tr("Official Name") + ": " + Avogadro::ElementTranslator::name(n)
                              + "\n" +tr("Atomic Number") +": " + QString::number(n)
                              + "\n" +tr("Atomic Weight") +": " + QString::number(OpenBabel::etab.GetMass(n)) + " a.u."
                              + "\n" +tr("Monoisotopic Weight") + ": " + QString::number(OpenBabel::isotab.GetExactMass(n)) + " a.u."
                              + "\n" +tr("Covalent Radius") + ": "  + QString::number(OpenBabel::etab.GetCovalentRad(n)) + " \xC5"
                              + "\n" +tr("VdW Radius") + ": "  + QString::number(OpenBabel::etab.GetVdwRad(n)) + " \xC5"
                              + "\n" +tr("Pauling Electronegativity") + ": "  + QString::number(OpenBabel::etab.GetElectroNeg(n))
                              + "\n" +tr("Allred-Rochow Electronegativity") + ": "  + QString::number(OpenBabel::etab.GetAllredRochowElectroNeg(n))
                              + "\n" +tr("Ionization Potential") + ": "  + QString::number(OpenBabel::etab.GetIonization(n)) + " eV"
                              + "\n" +tr("Electron Affinity") + ": "  + QString::number(OpenBabel::etab.GetElectronAffinity(n)) + " eV"
                              );
}


int main(int argc, char **argv)
{
  QApplication app(argc, argv);

    // Load the Periodic Table translations
    QPointer <QTranslator> ptTranslator = QPeriodicTable::createTranslator();
    if (ptTranslator)
      qApp->installTranslator(ptTranslator);

  // Construct Periodic Table
  PeriodicTableView* periodicTable = new PeriodicTableView;
  periodicTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QMainWindow *window = new QMainWindow();
  window->setWindowTitle("Periodic System of D.I.Mendeleyev");
  QWidget *widget = new QWidget;
  QHBoxLayout *layout = new QHBoxLayout(widget);
  widget->setLayout(layout);
  QPlainTextEdit *elementInfo = new QPlainTextEdit;
  elementInfo->setReadOnly(true);
  elementInfo->setPlainText("Click on element to get the information about it");
  elementInfo->setMaximumHeight(periodicTable->height());
  elementInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  layout->addWidget(periodicTable);
  layout->addWidget(elementInfo);
  window->setCentralWidget(widget);
  PeriodicTableWatcher *watcher = new PeriodicTableWatcher(periodicTable, elementInfo);
  window->show();
  app.exec();
  delete periodicTable;
  delete elementInfo;
  delete window;
  return 0;
}

//#ifdef CMAKE_INSTALL_PREFIX
  #include "qperiodictable.moc"
//#endif
