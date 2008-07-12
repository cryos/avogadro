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

#ifndef PROPEXTENSION_H
#define PROPEXTENSION_H


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

  class PropertiesView;

 class PropertiesExtension : public Extension
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
      virtual QString name() const { return QObject::tr("Properties"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Properties Plugin"); };

      /** @return a menu path for the extension's actions */
      virtual QString menuPath(QAction *action) const;

      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
      //@}

      void setMolecule(Molecule *molecule);

    private:
/*
      PropertiesView * m_view;
      PropertiesModel* m_atomModel;
      PropertiesModel* m_bondModel;
      PropertiesModel* m_angleModel;
      PropertiesModel* m_torsionModel;
      PropertiesModel* m_cartesianModel;
      PropertiesModel* m_conformerModel;
*/
      QList<QAction *> m_actions;
      Molecule *m_molecule;
  };

  class PropertiesView : public QTableView
  {
    Q_OBJECT

     public:
       enum Type {
         OtherType=0,
         AtomType,
         BondType,
         AngleType,
         TorsionType,
         CartesianType,
         ConformerType
       };

       explicit PropertiesView(Type type, QWidget *parent = 0);

       void selectionChanged(const QItemSelection &selected, const QItemSelection &previous);
       void setMolecule (Molecule *molecule);
       void setWidget (GLWidget *widget);
       void hideEvent(QHideEvent *event);

     private:
       int m_type;
       Molecule *m_molecule;
       GLWidget *m_widget;
  };
 
  class PropertiesExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)

    public:
      Plugin *createInstance(QObject *parent = 0) { return new PropertiesExtension(parent); }
      int type() const { return Plugin::ExtensionType; };
      QString name() const { return tr("Properties Extension"); };
      QString description() const { return tr("Extension with dialogs for displaying atom, bond, "
          "angle and tosion properties. It also includes a cartesian coordinate editor."); };
 
  };
 
} // end namespace Avogadro

#endif
