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

#ifndef __HYDROGENS_H
#define __HYDROGENS_H


#include <openbabel/mol.h>

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>

namespace Avogadro {

 class Hydrogens : public QObject, public Extension
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::Extension)

    public:
      //! Constructor
      Hydrogens();
      //! Deconstructor
      virtual ~Hydrogens();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("Hydrogens"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Hydrogens Plugin"); };
      //! Perform Action
      virtual QUndoCommand* performAction(QAction *action, Molecule *molecule, QTextEdit *messages=NULL);
      //@}
  };

  class HydrogensCommand : public QUndoCommand
  {
    public:
      enum Action {
        AddHydrogens = 0,
        RemoveHydrogens
      };

    public:
      HydrogensCommand(Molecule *molecule, enum Action action);

      virtual void undo();
      virtual void redo();
      virtual bool mergeWith ( const QUndoCommand * command );
      virtual int id() const;

    private:
      Molecule *m_molecule;
      Molecule m_moleculeCopy;
      enum Action m_action;
  };

} // end namespace Avogadro

#endif
