/**********************************************************************
  SuperCell - Build up surfaces from crystallographic unit cels

  Copyright (C) 2007-2008 by Geoffrey R. Hutchison

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

#ifndef SUPERCELLBUILDER_H
#define SUPERCELLBUILDER_H

#include "supercelldialog.h"

#include <openbabel/mol.h>
#include <openbabel/generic.h>

#include <avogadro/extension.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>

namespace Avogadro {

  class SuperCellBuilder : public Extension
  {
    Q_OBJECT

      public:
    //! Constructor
    SuperCellBuilder(QObject *parent=0);
    //! Deconstructor
    virtual ~SuperCellBuilder();

    //! \name Description methods
    //@{
    //! Plugin Name (ie Draw)
    virtual QString name() const { return QObject::tr("SuperCell"); }
    //! Plugin Description (ie. Draws atoms and bonds)
    virtual QString description() const { return QObject::tr("Super Cell Builder"); };
    //! Perform Action
    virtual QList<QAction *> actions() const;
    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
    virtual QString menuPath(QAction *action) const;
    virtual void setMolecule(Molecule *molecule);
    //@}

  private:
    QList<QAction *> m_actions;
    SuperCellDialog *m_Dialog;
    Molecule *m_Molecule;
  };

  class SuperCellBuilderFactory : public QObject, public ExtensionFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::ExtensionFactory)

  public:
    Extension *createInstance(QObject *parent = 0) { return new SuperCellBuilder(parent); }
  };

} // end namespace Avogadro

#endif
