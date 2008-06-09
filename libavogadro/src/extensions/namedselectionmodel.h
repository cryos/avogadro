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
