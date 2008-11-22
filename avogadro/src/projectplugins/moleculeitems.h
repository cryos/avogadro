/**********************************************************************
  MoleculeItems - Project Tree Items for molecule (atoms, bonds, ...).

  Copyright (C) 2008 by Tim Vandermeersch

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

#ifndef MOLECULEITEMS_H
#define MOLECULEITEMS_H

#include <avogadro/global.h>
#include "../projectplugin.h"
#include "../projecttreemodel.h"

#include <QString>

namespace Avogadro {

  class Primitive;
  class AtomItems;
  class BondItems;

  class A_EXPORT MoleculeItems : public ProjectPlugin
  {
    Q_OBJECT

    public:
      MoleculeItems();
      ~MoleculeItems();

      QString name() const { return QObject::tr("Molecule"); }
      
      void setupModelData(ProjectTreeModel *, GLWidget *, ProjectTreeItem *parent);
      
      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);

    private:
      ProjectTreeItem *m_moleculeLabel;
      
      AtomItems *m_atomItems;
      BondItems *m_bondItems;
      
      ProjectTreeItem *m_residueLabel;
      ProjectTreeModel *m_model;
      GLWidget *m_widget;
  };

  /* 
  class MoleculeItemsFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)

    public:
      Plugin *createInstance(QObject * parent = 0) { return new MoleculeItems(); }
      Plugin::Type type() const { return Plugin::ProjectType; };
      QString name() const { return QObject::tr("Molecule"); };
      QString description() const { return QObject::tr("Project item for molecule."); };
  };
  */

 
} // end namespace Avogadro

#endif
