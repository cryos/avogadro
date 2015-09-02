/**********************************************************************
  OrcaExtension

  Copyright (C) 2010 David C. Lonie
  Some portions Copyright (C) 2014 Dagmar lenk

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#ifndef ORCAEXTENSION_H
#define ORCAEXTENSION_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/extension.h>
#include <avogadro/molecule.h>

// This is a work around for a bug on older versions Avogadro, bug
// 3104853. Patch submitted.
using Avogadro::Plugin;

namespace Avogadro {
  // Forward declaration of header
  class OrcaInputDialog;
  class OrcaAnalyseDialog;
//  class OrcaData;



  class OrcaExtension : public Avogadro::Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION(// Static identifier:
                       "Orca Test Extension",
                       // Short description:
                       tr("Orca Test Example"),
                       // Long description:
                       tr("Provides a dialog box with the words \"H...\"."))
    Q_ENUMS (DFTFunctionalType)
    Q_ENUMS (DFTNoCosXType)
    Q_ENUMS (basisType)
    Q_ENUMS (gridType)
    Q_ENUMS (finalgridType)

  public:
    OrcaExtension(QObject *parent=NULL);
    ~OrcaExtension() {}

    // This tells Avogadro what actions to create
    virtual QList<QAction *> actions() const;
    // This returns a string that tells Avogadro where to put the menu entries
    virtual QString menuPath(QAction *action) const;
    // When an action is requested (e.g. a user selects a menu entry),
    // here is where it is handled:
    virtual QUndoCommand* performAction(QAction *action,
                                        Avogadro::GLWidget *widget);

    // This is new -- we will be using this to update the dialog when
    // a new molecule is loaded.
    virtual void setMolecule(Avogadro::Molecule *molecule);

    enum DFTFunctionalType {LDA, BP, BLYP, PW91, B3LYP, B3PW, PBEO, TPSS, TPSSH, M06L};
    enum DFTNoCosXType {NoBP, NoBLYP, NoPW91, NoTPSS};
    enum basisType {SVP, TZVP, TZVPP, QZVP};
    enum gridType {Default, None, Grid3, Grid4, Grid5, Grid6, Grid7, Grid8};
    enum finalgridType {fDefault, fNone, fGrid4, fGrid5, fGrid6, fGrid7, fGrid8, fGrid9};

  private:
    // List of actions implemented by the extension
    QList<QAction *> m_actions;
    // Dialog from orcainputdialog.*
    OrcaInputDialog *m_dialog;
//    OrcaData *m_orcaData;
    OrcaAnalyseDialog *m_analyseDialog;

    Molecule* m_molecule;
    GLWidget* m_widget;

    bool m_NOAnalyse;
  };

  // Plugin factory setup
  class OrcaExtensionFactory
    : public QObject,
      public Avogadro::PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(OrcaExtension)
  };

}

#endif

