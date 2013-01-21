/**********************************************************************
  structuresmodel.h - Model to hold structures

  Copyright (C) 2010 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef CONSTRAINTSMODEL_H
#define CONSTRAINTSMODEL_H

#include <QObject>
#include <QList>
#include <QString>
#include <QAbstractTableModel>
#include <QItemDelegate>

namespace Avogadro {

  enum { ComboBoxRole = 100 };

  struct Structure
  {
    enum Type { Lipid, PolarSolvent, PolarSolute, LipophilicSolute };

    QString fileName;
    Type type;
    int number;
    double density;
  };
   
  class StructuresModel : public QAbstractTableModel
  {
    Q_OBJECT
     
    public:
      StructuresModel(QObject *parent = 0) : QAbstractTableModel(parent) {}

      int rowCount(const QModelIndex &parent = QModelIndex()) const;
      int columnCount(const QModelIndex &parent = QModelIndex()) const;
      QVariant data(const QModelIndex &index, int role) const;
      QVariant headerData(int section, Qt::Orientation orientation,
          int role = Qt::DisplayRole) const;
      Qt::ItemFlags flags(const QModelIndex &index) const;
      bool setData(const QModelIndex &index, const QVariant &value, 
          int role = Qt::EditRole);

      void addStructure();
      void addDefaultStructures();

      void removeStructure(int index);

      const QList<Structure>& structures() const { return m_structures; }
      void setStructures(const QList<Structure>& structures);

    private:
      QList<Structure> m_structures;

  };

  class SpinBoxDelegate : public QItemDelegate
  {
    Q_OBJECT

    public:
      SpinBoxDelegate(QObject *parent = 0);

      QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
         const QModelIndex &index) const;
      void setEditorData(QWidget *editor, const QModelIndex &index) const;
      void setModelData(QWidget *editor, QAbstractItemModel *model,
          const QModelIndex &index) const;
      void updateEditorGeometry(QWidget *editor,
          const QStyleOptionViewItem &option, const QModelIndex &index) const;
  };

  class ComboBoxDelegate : public QItemDelegate
  {
    Q_OBJECT

    public:
      ComboBoxDelegate(QObject *parent = 0);

      QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
         const QModelIndex &index) const;
      void setEditorData(QWidget *editor, const QModelIndex &index) const;
      void setModelData(QWidget *editor, QAbstractItemModel *model,
          const QModelIndex &index) const;
      void updateEditorGeometry(QWidget *editor,
          const QStyleOptionViewItem &option, const QModelIndex &index) const;
  };

  class FileDelegate : public QItemDelegate
  {
    Q_OBJECT

    public:
      FileDelegate(QObject *parent = 0);

      QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
         const QModelIndex &index) const;
      void setEditorData(QWidget *editor, const QModelIndex &index) const;
      void setModelData(QWidget *editor, QAbstractItemModel *model,
          const QModelIndex &index) const;
      void updateEditorGeometry(QWidget *editor,
          const QStyleOptionViewItem &option, const QModelIndex &index) const;
    public slots:
      void browseClicked();
  };

  class DensityDelegate : public QItemDelegate
  {
    Q_OBJECT

    public:
      DensityDelegate(QObject *parent = 0);

      QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
         const QModelIndex &index) const;
      void setEditorData(QWidget *editor, const QModelIndex &index) const;
      void setModelData(QWidget *editor, QAbstractItemModel *model,
          const QModelIndex &index) const;
      void updateEditorGeometry(QWidget *editor,
          const QStyleOptionViewItem &option, const QModelIndex &index) const;
  };


} // end namespace Avogadro

#endif
