/**********************************************************************
  namedselectionmodel.h - Model to hold named selections

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

#ifndef NAMEDSELECTIONMODEL_H
#define NAMEDSELECTIONMODEL_H


#include <openbabel/mol.h>

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QItemSelectionModel>

#ifndef BUFF_SIZE
#define BUFF_SIZE 256
#endif

namespace Avogadro {

  namespace SelectTreeItemType 
  {
    enum {
      HeaderType=1,
      AtomType,
      BondType,
      ResidueType,
      ChainType,
      SelectionType
    };
  }


  class SelectTreeItem
  {
    public:
      SelectTreeItem(const QList<QVariant> &data, int type, SelectTreeItem *parent = 0);
      ~SelectTreeItem();

      void appendChild(SelectTreeItem *child);

      SelectTreeItem *child(int row);
      int childCount() const;
      int columnCount() const;
      QVariant data(int column) const;
      int row() const;
      SelectTreeItem *parent();
      
      int type() const;

    private:
      QList<SelectTreeItem*>	m_childItems;
      QList<QVariant>		m_itemData;
      SelectTreeItem		*m_parentItem;
      int 			m_type;
  };

  class SelectTreeModel : public QAbstractItemModel
  {
    Q_OBJECT
    
    public:
      SelectTreeModel(GLWidget* widget, QObject *parent = 0);
      ~SelectTreeModel();

      QVariant data(const QModelIndex &index, int role) const;
      Qt::ItemFlags flags(const QModelIndex &index) const;
      QVariant headerData(int section, Qt::Orientation orientation,
      int role = Qt::DisplayRole) const;
      QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
      QModelIndex parent(const QModelIndex &index) const;
      int rowCount(const QModelIndex &parent = QModelIndex()) const;
      int columnCount(const QModelIndex &parent = QModelIndex()) const;

    private:
      void addChainData(Molecule *molecule, QList<SelectTreeItem*> &parents);
      void addSelectionData(GLWidget *widget, QList<SelectTreeItem*> &parents);
      void addResidueData(Molecule *molecule, char chain, QList<SelectTreeItem*> &parents);
      void setupModelData(GLWidget *widget, SelectTreeItem *parent);
      SelectTreeItem *rootItem;
  };

  class NamedSelectionModel : public QAbstractListModel
  {
    Q_OBJECT
     
     public:
       explicit NamedSelectionModel(GLWidget *widget, QObject *parent = 0) : 
	       QAbstractListModel(parent), m_widget(widget)
       {
       }

       int rowCount(const QModelIndex &parent = QModelIndex()) const;
       QVariant data(const QModelIndex &index, int role) const;
       Qt::ItemFlags flags(const QModelIndex &index) const;
       bool setData(const QModelIndex &index, const QVariant &value, 
           int role = Qt::EditRole);
       QVariant headerData(int section, Qt::Orientation orientation,
           int role = Qt::DisplayRole) const;
     
     private:
       int m_rowCount;
       GLWidget *m_widget;
 };
  
} // end namespace Avogadro

#endif
