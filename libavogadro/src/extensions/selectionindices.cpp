/**********************************************************************
  Selection Indices Plugin for Avogadro

  Copyright (C) 2016 by Davide Olianas

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#include "selectionindices.h"
#include <avogadro/primitive.h>
#include <avogadro/primitivelist.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/mol.h>

#include <QAction>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  SelectionIndicesExtension::SelectionIndicesExtension(QObject *parent) : Extension(parent)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Export selection indices"));
    m_actions.append(action);

    action = new QAction( this );
    action->setSeparator(true);
    m_actions.append( action );
  }

  SelectionIndicesExtension::~SelectionIndicesExtension()
  {
  }

  QList<QAction *> SelectionIndicesExtension::actions() const
  {
    return m_actions;
  }

  QString SelectionIndicesExtension::menuPath(QAction *) const
  {
    return tr("E&xtensions");
  }

  void SelectionIndicesExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* SelectionIndicesExtension::performAction(QAction *action, GLWidget *widget)
  {



    // Fragment copied from InputDialog
    // Try to set default save path for dialog using the next sequence:
    //  1) directory of current file (if any);
    //  2) directory where previous deck was saved;
    //  3) $HOME
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if(m_savePath == "") {
      if (defaultPath.isEmpty())
        defaultPath = QDir::homePath();
    } else {
      defaultPath = m_savePath;
    }
    QString fileType = "text";
    QString ext = "txt";
    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + "." + ext;
    QString fileName = QFileDialog::getSaveFileName(widget, tr("Save indices of selection"),
        defaultFileName, fileType + " (*." + ext + ")");

    QFile file(fileName);
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream outStream(&file);


    PrimitiveList selectedPrimitives = widget->selectedPrimitives();

    foreach(Primitive *p, selectedPrimitives.subList(Primitive::AtomType)) {
      Atom *selected = static_cast<Atom*>(p);
      outStream << selected->index() + 1 << endl;
    }

    file.close(); // flush buffer!

    // Selections are per-view and as such are not saved or undo-able
    return NULL;
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(selectionindicesextension, Avogadro::SelectionIndicesExtensionFactory)
