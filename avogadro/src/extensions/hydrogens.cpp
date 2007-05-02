/**********************************************************************
  Hydrogens - Hydrogens Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "hydrogens.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

Hydrogens::Hydrogens() : Extension()
{
  QAction *action = new QAction(this);
  action->setText("Add Hydrogens");
  m_actions.append(action);

  action = new QAction(this);
  action->setText("Remove Hydrogens");
  m_actions.append(action);
}

Hydrogens::~Hydrogens() 
{
}

void Hydrogens::performAction(QAction *action, Molecule *molecule, QTextEdit *messages)
{

  qDebug() << "Perform Action";
  int i = m_actions.indexOf(action);
  switch(i)
  {
    case 0:
      addHydrogens(molecule);
      break;
    case 1:
      removeHydrogens(molecule);
      break;
  }
}

void Hydrogens::addHydrogens(Molecule *molecule)
{
  qDebug() << "Add Hydrogens on " << molecule;
  //  molecule->BeginModify();
  molecule->AddHydrogens(false, true);
  molecule->update();
  //  molecule->EndModify();
}

void Hydrogens::removeHydrogens(Molecule *molecule)
{
  //  molecule->BeginModify();
  molecule->DeleteHydrogens();
  //  molecule->EndModify();
  molecule->update();
}

#include "hydrogens.moc"
Q_EXPORT_PLUGIN2(hydrogens, Hydrogens)
