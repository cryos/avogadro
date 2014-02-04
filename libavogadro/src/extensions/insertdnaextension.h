/**********************************************************************
  InsertDNA - Insert DNA/RNA chains

  Copyright (C) 2012 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef INSERTDNAEXTENSION_H
#define INSERTDNAEXTENSION_H

#include <avogadro/extension.h>

namespace Avogadro {

  class InsertDNADialog;
  class InsertDNAExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("InsertDNA", tr("Insert DNA/RNA"),
                       tr("Insert DNA/RNA chains"))

  public:
    //! Constructor
    InsertDNAExtension(QObject *parent=0);
    //! Destructor
    ~InsertDNAExtension();

    //! Perform Action
    QList<QAction *> actions() const;
    QUndoCommand* performAction(QAction *action, GLWidget *widget);
    QString menuPath(QAction *action) const;
    void setMolecule(Molecule *molecule);

    void writeSettings(QSettings &settings) const;
    void readSettings(QSettings &settings);

  public Q_SLOTS:
    void performInsert();

    void updateText();
    void updateBPTurns(int type);
    void changeNucleicType(int type);

    void dialogDestroyed();

  private:
    QList<QAction *> m_actions;
    GLWidget* m_widget;
    Molecule *m_molecule;

    InsertDNADialog *m_dialog;

    void constructDialog();
  };

  class InsertDNAExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(InsertDNAExtension)
  };

} // end namespace Avogadro

#endif
