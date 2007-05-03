/**********************************************************************
  Ghemical - Ghemical Plugin for Avogadro

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

#ifndef __GHEMICALEXTENSION_H
#define __GHEMICALEXTENSION_H


#include <openbabel/mol.h>
#include <openbabel/forcefield.h>

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>

#ifndef BUFF_SIZE
#define BUFF_SIZE 256
#endif

namespace Avogadro {

 class GhemicalExtension : public QObject, public Extension
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::Extension)

    public:
      //! Constructor
      GhemicalExtension();
      //! Deconstructor
      virtual ~GhemicalExtension();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("Ghemical"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Ghemical Plugin"); };
      //! Perform Action
      virtual QUndoCommand* performAction(QAction *action, Molecule *molecule, QTextEdit *textEdit);
      //@}

    private:
      OpenBabel::OBForceField* m_forceField;
  };

 class GhemicalCommand : public QUndoCommand
  {
    public:
      GhemicalCommand(Molecule *molecule, OpenBabel::OBForceField *forcefield, QTextEdit *messages);

      virtual void redo();
      virtual void undo();
      virtual bool mergeWith ( const QUndoCommand * command );
      virtual int id() const;

    private:
      Molecule m_moleculeCopy;
      Molecule *m_molecule;
      QTextEdit *m_textEdit;
      int m_cycles;
      OpenBabel::OBForceField* m_forceField;
  };

} // end namespace Avogadro

#endif
