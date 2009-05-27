/**********************************************************************
  InsertPeptide - Insert oligopeptide sequences

  Copyright (C) 2008-2009 by Geoffrey R. Hutchison

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

#ifndef INSERTPEPTIDEEXTENSION_H
#define INSERTPEPTIDEEXTENSION_H

#include <avogadro/extension.h>

#include "ui_insertpeptidedialog.h"

namespace Avogadro {

  class InsertPeptideDialog : public QDialog, public Ui::InsertPeptideDialog
    {
    public:
    InsertPeptideDialog(QWidget *parent=0) : QDialog(parent) {
        setWindowFlags(Qt::Dialog | Qt::Tool);
        setupUi(this);
      }
    };

  class InsertPeptideExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("InsertPeptide", tr("Insert Peptide"),
                       tr("Insert oligopeptide sequences"))

  public:
    //! Constructor
    InsertPeptideExtension(QObject *parent=0);
    //! Destructor
    ~InsertPeptideExtension();

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

    void setStructureType(int);
    void setPhi(double);
    void setPsi(double);
    
    void setStereo(int);
    
    void dialogDestroyed();

  private:
    QList<QAction *> m_actions;
    GLWidget* m_widget;
    Molecule *m_molecule;

    double phi, psi, omega;
    bool lStereo;
    int structureType;

    InsertPeptideDialog *m_dialog;
    void constructDialog();
    void updateDialog();
  };

  class InsertPeptideExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(InsertPeptideExtension)
  };

} // end namespace Avogadro

#endif
