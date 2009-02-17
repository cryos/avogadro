/**********************************************************************
  UnitCell - Change parameters for crystallographic unit cells

  Copyright (C) 2007 by Geoffrey R. Hutchison

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

#ifndef UNITCELLEXTENSION_H
#define UNITCELLEXTENSION_H

#include <avogadro/extension.h>

#include "unitcellparamdialog.h"

namespace Avogadro {

  class UnitCellExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("UnitCell", tr("UnitCell"))

  public:
    //! Constructor
    UnitCellExtension(QObject *parent=0);
    //! Destructor
    ~UnitCellExtension();

    //! \name Description methods
    //@{
    //! Plugin Description (ie. Draws atoms and bonds)
    QString description() const { return QObject::tr("Unit Cell Parameters Plugin"); };
    //! Perform Action
    QList<QAction *> actions() const;
    QUndoCommand* performAction(QAction *action, GLWidget *widget);
    QString menuPath(QAction *action) const;
    void setMolecule(Molecule *molecule);
    //@}

  public Q_SLOTS:
    void unitCellDisplayChanged(int a, int b, int c);
    void unitCellParametersChanged(double a, double b, double c,
                                     double alpha, double beta, double gamma);

    void deleteUnitCell();
    void fillUnitCell();

  private:
    QList<QAction *> m_actions;
    UnitCellParamDialog *m_dialog;
    GLWidget *m_widget;
    Molecule *m_molecule;
  };

  class UnitCellExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)

      AVOGADRO_EXTENSION_FACTORY(UnitCellExtension,
          "Unit Cell Extension",
          tr("Unit Cell Extension"),
          tr("Extension for building unit cells."))
  };

} // end namespace Avogadro

#endif
