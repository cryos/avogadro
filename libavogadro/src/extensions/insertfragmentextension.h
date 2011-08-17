/**********************************************************************
  InsertFragment - Insert molecular fragments or SMILES

  Copyright (C) 2009 by Geoffrey R. Hutchison

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

#ifndef INSERTFRAGMENTEXTENSION_H
#define INSERTFRAGMENTEXTENSION_H

#include <avogadro/extension.h>

#include "insertfragmentdialog.h"

namespace Avogadro {

  class Primitive;

  class InsertFragmentExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("InsertFragment", tr("Insert Fragment"),
                       tr("Insert molecular fragments for building larger molecules"))

  public:
    //! Constructor
    InsertFragmentExtension(QObject *parent=0);
    //! Destructor
    ~InsertFragmentExtension();

    //! Perform Action
    QList<QAction *> actions() const;
    QUndoCommand* performAction(QAction *action, GLWidget *widget);
    QString menuPath(QAction *action) const;
    void setMolecule(Molecule *molecule);

    void writeSettings(QSettings &settings) const;
    void readSettings(QSettings &settings);

    QList<int> findSelectedForInsert(QList<Primitive*> selectedAtomList) const;

  public Q_SLOTS:
    void insertCrystal();
    void insertFragment();

  private:

    QList<QAction *> m_actions;
    GLWidget* m_widget;
    InsertFragmentDialog *m_fragmentDialog;
    InsertFragmentDialog *m_crystalDialog;
    QString   m_smilesString;
    Molecule *m_molecule;
  };

  class InsertFragmentExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(InsertFragmentExtension)
  };

} // end namespace Avogadro

#endif
