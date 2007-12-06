/**********************************************************************
  propextension.h - Properties Plugin for Avogadro

  Copyright (C) 2007 by Tim Vandermeersch

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

#ifndef __PROPEXTENSION_H
#define __PROPEXTENSION_H


#include "propmodel.h"
//#include "propdialog.h"

#include <openbabel/mol.h>

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>
#include <QThread>
#include <QMutex>

#ifndef BUFF_SIZE
#define BUFF_SIZE 256
#endif

class QProgressDialog;
namespace Avogadro {

 class PropertiesExtension : public QObject, public Extension
  {
    Q_OBJECT

    public:
      //! Constructor
      PropertiesExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~PropertiesExtension();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("Propertiesd"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Properties Plugin"); };

      /** @return a menu path for the extension's actions */
      virtual QString menuPath(QAction *action) const;

      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, Molecule *molecule,
                                          GLWidget *widget, QTextEdit *textEdit);
      //@}

    private:
      AtomPropModel* m_atomModel;
      BondPropModel* m_bondModel;
      CartesianModel* m_cartesianModel;
      ConformerModel* m_conformerModel;
      QList<QAction *> m_actions;
  };

  class PropertiesExtensionFactory : public QObject, public ExtensionFactory
  {
    Q_OBJECT;
    Q_INTERFACES(Avogadro::ExtensionFactory);

    public:
    Extension *createInstance(QObject *parent = 0) { return new PropertiesExtension(parent); }
  };
 
 class ConformerView : public QTableView
  {
    Q_OBJECT

     public:
       ConformerView(QWidget *parent = 0) : QTableView(parent), m_molecule(NULL) {}

       void selectionChanged(const QItemSelection &selected, const QItemSelection &previous);
       void setMolecule (Molecule *molecule);

     private:
       Molecule *m_molecule;
 };
 
 class AtomTableView : public QTableView
  {
    Q_OBJECT

     public:
       AtomTableView(QWidget *parent = 0) : QTableView(parent), m_molecule(NULL) {}

       void selectionChanged(const QItemSelection &selected, const QItemSelection &previous);
       void setMolecule (Molecule *molecule);
       void setWidget (GLWidget *widget);
       void hideEvent(QHideEvent *event);

     private:
       Molecule *m_molecule;
       GLWidget *m_widget;
 };
 
 class BondTableView : public QTableView
  {
    Q_OBJECT

     public:
       BondTableView(QWidget *parent = 0) : QTableView(parent), m_molecule(NULL) {}

       void selectionChanged(const QItemSelection &selected, const QItemSelection &previous);
       void setMolecule (Molecule *molecule);
       void setWidget (GLWidget *widget);
       void hideEvent(QHideEvent *event);

     private:
       Molecule *m_molecule;
       GLWidget *m_widget;
 };
 
} // end namespace Avogadro

#endif
