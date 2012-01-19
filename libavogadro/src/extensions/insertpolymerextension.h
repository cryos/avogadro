/**********************************************************************
  InsertPolymer - Build polymers and co-polymers, including custom units

  Copyright (C) 2012 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef INSERTPOLYMEREXTENSION_H
#define INSERTPOLYMEREXTENSION_H

#include <avogadro/extension.h>

#include "ui_insertpolymerdialog.h"

// Forward declarations
namespace OpenBabel {
  class OBRandom;
  class OBForceField;
}

namespace Avogadro {

  class InsertPolymerDialog;
  class InsertPolymerExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("InsertPolymer", tr("Insert Polymer"),
                       tr("Insert polymers and co-polymers"))

  public:
    //! Constructor
    InsertPolymerExtension(QObject *parent=0);
    //! Destructor
    ~InsertPolymerExtension();

    //! Perform Action
    QList<QAction *> actions() const;
    QUndoCommand* performAction(QAction *action, GLWidget *widget);
    QString menuPath(QAction *action) const;
    void setMolecule(Molecule *molecule);

    void writeSettings(QSettings &settings) const;
    void readSettings(QSettings &settings);

  public Q_SLOTS:
    void chooseMonomerA();
    void chooseMonomerB();
    void chooseMonomerC();

    void validateMonomerRepeats(int value);

    void build();
    void dialogDestroyed();

    void updateMonomerGraphics();

  private:
    QList<QAction *> m_actions;
    GLWidget *m_widget;
    Molecule *m_molecule;

    QString m_directory;

    InsertPolymerDialog *m_dialog;
    void constructDialog();

    QString getSmilesFromFile(QString type, QString name);
    OpenBabel::OBRandom *m_random;
    OpenBabel::OBForceField *m_FF;

    void updateGraphics(QString monomer, QWebView *target);
  };

  class InsertPolymerExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(InsertPolymerExtension)
  };

} // end namespace Avogadro

#endif
