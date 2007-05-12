/**********************************************************************
  SMARTS - Select SMARTS plugin for Avogadro

  Copyright (C) 2006-2007 by Donald Ephraim Curtis
  Copyright (C) 2006-2007 by Geoffrey R. Hutchison

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

#ifndef __SMARTSEXTENSION_H
#define __SMARTSEXTENSION_H


#include <openbabel/mol.h>
#include <openbabel/parsmart.h>

#include <avogadro/extension.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>

namespace Avogadro {

 class SmartsExtension : public Extension
  {
    Q_OBJECT

    public:
      //! Constructor
      SmartsExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~SmartsExtension();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("SMARTS"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("SMARTS Plugin"); };
      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, Molecule *molecule, 
                                          GLWidget *widget, QTextEdit *messages=NULL);
      //@}

    private:
      QList<QAction *> m_actions;
  };

  class SmartsExtensionFactory : public QObject, public ExtensionFactory
  {
    Q_OBJECT;
    Q_INTERFACES(Avogadro::ExtensionFactory);

    public:
    Extension *createInstance(QObject *parent = 0) { return new SmartsExtension(parent); }
  };

  class SmartsCommand : public QUndoCommand
  {
    public:
    SmartsCommand(Molecule *molecule, GLWidget *widget, std::string pattern);

      virtual void undo();
      virtual void redo();
      virtual bool mergeWith ( const QUndoCommand * command );
      virtual int id() const;

    private:
      Molecule *m_molecule;
      GLWidget *m_widget;
      OpenBabel::OBSmartsPattern m_pattern;
      QList<Primitive *> m_selectedList;
  };

} // end namespace Avogadro

#endif
