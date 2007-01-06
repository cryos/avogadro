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

  class ProjectItem
  {
    public:
      ProjectItem(Primitive *primitive, QList<QVariant> data, ProjectItem *parent=0);
      ProjectItem(QList<QVariant> data, ProjectItem *parent=0);
      ~ProjectItem();

      void appendChild(ProjectItem *child);

      ProjectItem *child(int row);
      int childCount() const;
      int columnCount() const;
      QVariant data(int column) const;
      int row() const;
      ProjectItem *parent();

    private:
      QList<ProjectItem*> childItems;
      QList<QVariant> itemData;
      Primitive *userData;
      ProjectItem *parentItem;
  };

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
      ProjectItem *rootItem;

  };

} // end namespace Avogadro

#endif
