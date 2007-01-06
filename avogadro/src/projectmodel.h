/**********************************************************************
  ProjectModel - Project Model

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

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

#ifndef __PROJECTMODEL_H
#define __PROJECTMODEL_H

#include <avogadro/primitives.h>

#include<QAbstractItemModel>

namespace Avogadro {

//dc:   class ProjectItem
//dc:   {
//dc:     public:
//dc:       ProjectItem(Primitive *primitive, QList<QVariant> data, ProjectItem *parent=0);
//dc:       ProjectItem(QList<QVariant> data, ProjectItem *parent=0);
//dc:       ~ProjectItem();
//dc: 
//dc:       void appendChild(ProjectItem *child);
//dc: 
//dc:       ProjectItem *child(int row);
//dc:       int childCount() const;
//dc:       int columnCount() const;
//dc:       QVariant data(int column) const;
//dc:       int row() const;
//dc:       ProjectItem *parent();
//dc: 
//dc:     private:
//dc:       QList<ProjectItem*> childItems;
//dc:       QList<QVariant> itemData;
//dc:       Primitive *userData;
//dc:       ProjectItem *parentItem;
//dc:   };

  //! Base class for our tools
  class ProjectModel : public QAbstractItemModel
  {
    Q_OBJECT

    public:

      ProjectModel(Molecule *molecule, QObject *parent=0);
      //! Deconstructor
      ~ProjectModel() {}

      QVariant data(const QModelIndex &index, int role) const;
      Qt::ItemFlags flags(const QModelIndex &index) const;
      QVariant headerData(int section, Qt::Orientation orientation,
          int role = Qt::DisplayRole) const;
      QModelIndex index(int row, int column,
          const QModelIndex &parent = QModelIndex()) const;
      QModelIndex parent(const QModelIndex &index) const;
      int rowCount(const QModelIndex &parent = QModelIndex()) const;
      int columnCount(const QModelIndex &parent = QModelIndex()) const;

    private:
      Primitive *getMoleculeRow(int row) const;

      Primitive *rootItem;
      Molecule *moleculeItem;

  };

} // end namespace Avogadro

#endif
