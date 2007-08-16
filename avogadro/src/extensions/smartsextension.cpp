/**********************************************************************
  SMARTS - Select SMARTS plugin for Avogadro

  Copyright (C) 2006-2007 by Donald Ephraim Curtis
  Copyright (C) 2006-2007 by Geoffrey R. Hutchison

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

#include "smartsextension.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>

#include <openbabel/parsmart.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;

  namespace Avogadro {
    SmartsExtension::SmartsExtension(QObject *parent) : QObject(parent)
    {
      QAction *action = new QAction(this);
      action->setText("Select SMARTS");
      m_actions.append(action);
    }

    SmartsExtension::~SmartsExtension()
    {
    }

    QList<QAction *> SmartsExtension::actions() const
    {
      return m_actions;
    }

    QUndoCommand* SmartsExtension::performAction(QAction *,
                                                 Molecule *molecule,
                                                 GLWidget *widget,
                                                 QTextEdit *)
    {
      bool ok;
      QString pattern = QInputDialog::getText(qobject_cast<QWidget*>(parent()),
                                              tr("SMARTS Selection"),
                                              tr("SMARTS pattern to select"),
                                              QLineEdit::Normal,
                                              tr(""), &ok);
      if (ok && !pattern.isEmpty()) {
        OBSmartsPattern smarts;
        smarts.Init(pattern.toStdString());
        smarts.Match(*molecule);

        // if we have matches, select them
        if(smarts.NumMatches() != 0) {
          QList<Primitive *> matchedAtoms;

          vector< vector <int> > mapList = smarts.GetUMapList();
          vector< vector <int> >::iterator i; // a set of matching atoms
          vector<int>::iterator j; // atom ids in each match
          for (i = mapList.begin(); i != mapList.end(); ++i)
            for (j = i->begin(); j != i->end(); ++j) {
              matchedAtoms.append(static_cast<Atom*>(molecule->GetAtom(*j)));
            }

          widget->clearSelected();
          widget->setSelected(matchedAtoms, true);
          widget->update();
        } // end matches
      }

      return NULL;
    }

  } // end namespace Avogadro

#include "smartsextension.moc"
Q_EXPORT_PLUGIN2(smartsextension, Avogadro::SmartsExtensionFactory)
