/**********************************************************************
  constraintsmodel.cpp - Model to hold constraints

  Copyright (C) 2007 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "structuresmodel.h"

#include <QSpinBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileInfo>
#include <QFileDialog>
#include <QDebug>

using namespace std;

namespace Avogadro
{


  int StructuresModel::rowCount(const QModelIndex &) const
  {
    return m_structures.size();
  }

  int StructuresModel::columnCount(const QModelIndex &) const
  {
    return 4;
  }

  QVariant StructuresModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= m_structures.size())
      return QVariant();

    if (role == Qt::DisplayRole) {
      switch (index.column()) {
        case 0:
          {
            QFileInfo fileInfo(m_structures[index.row()].fileName);
            return fileInfo.fileName();
          }
        case 1:
          if (m_structures[index.row()].type == Structure::Lipid)
            return "lipid";
          if (m_structures[index.row()].type == Structure::PolarSolvent)
            return "polar solvent";
          if (m_structures[index.row()].type == Structure::PolarSolute)
            return "polar solute";
          if (m_structures[index.row()].type == Structure::LipophilicSolute)
            return "lipophilic solute";
        case 2:
          return m_structures[index.row()].number;
        case 3:
          return QString::number(m_structures[index.row()].density, 'f', 2) + " g/ml";
      }
    } else if (role == Qt::EditRole) {
      switch (index.column()) {
        case 0:
          return m_structures[index.row()].fileName;
        case 1:
          return m_structures[index.row()].type;
        case 2:
          return m_structures[index.row()].number;
        case 3:
          return m_structures[index.row()].density;
      }
    } else if (role == ComboBoxRole) {
      if (index.column() == 1) {
        QStringList items;
        items << "lipid";
        items << "polar solvent";
        items << "polar solute";
        items << "lipophilic solute";
        return items;
      }
    }

    return QVariant();
  }

  QVariant StructuresModel::headerData(int column, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    if (orientation == Qt::Horizontal) {
      switch (column) {
        case 0:
          return QString("File");
        case 1:
          return QString("Type");
        case 2:
          return QString("Number");
        case 3:
          return QString("Density");
      }
    }

    return QString("%1").arg(column + 1);
  }

  Qt::ItemFlags StructuresModel::flags(const QModelIndex &index) const
  {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    switch (index.column()) {
      case 0:
      case 1:
      case 2:
      case 3:
        flags |= Qt::ItemIsEditable;
      default:
        break;
    }
    return flags;
  }

  bool StructuresModel::setData(const QModelIndex &index, const QVariant &value, int role)
  {
    if (!index.isValid())
      return false;

    if (index.row() >= m_structures.size())
      return false;

    if (role == Qt::EditRole) {
      switch (index.column()) {
        case 0:
          m_structures[index.row()].fileName = value.toString();
          emit dataChanged(index, index);
          return true;
        case 1:
          m_structures[index.row()].type = static_cast<Structure::Type>(value.toInt());
          emit dataChanged(index, index);
          return true;
        case 2:
          m_structures[index.row()].number = value.toInt();
          emit dataChanged(index, index);
          return true;
        case 3:
          m_structures[index.row()].density = value.toDouble();
          emit dataChanged(index, index);
          return true;
        default:
          break;
      }
    }

    return false;
  }

  void StructuresModel::setStructures(const QList<Structure> &structures)
  {
    m_structures = structures;
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
  }


  void StructuresModel::addStructure()
  {
    beginInsertRows(QModelIndex(), m_structures.size(), m_structures.size());
    m_structures.append(Structure());
    // initialize reasonable defaults
    m_structures.last().type = Structure::Lipid;
    m_structures.last().density = 1.0;
    endInsertRows();
  }

  void StructuresModel::addDefaultStructures()
  {
    beginInsertRows(QModelIndex(), m_structures.size(), m_structures.size());
    m_structures.append(Structure());
    m_structures.last().type = Structure::PolarSolvent;
    m_structures.last().density = 1.0;
    endInsertRows();
    beginInsertRows(QModelIndex(), m_structures.size(), m_structures.size());
    m_structures.append(Structure());
    m_structures.last().type = Structure::Lipid;
    m_structures.last().density = 0.85;
    endInsertRows();
  }

  /*
  void StructuresModel::addAtomConstraint(int index)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size());
    m_constraints.AddAtomConstraint(index);
    endInsertRows();
  }

  void StructuresModel::addAtomXConstraint(int index)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size());
    m_constraints.AddAtomXConstraint(index);
    endInsertRows();
  }

  void StructuresModel::addAtomYConstraint(int index)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size());
    m_constraints.AddAtomYConstraint(index);
    endInsertRows();
  }

  void StructuresModel::addAtomZConstraint(int index)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size());
    m_constraints.AddAtomZConstraint(index);
    endInsertRows();
  }

  void StructuresModel::addDistanceConstraint(int a, int b, double length)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size());
    m_constraints.AddDistanceConstraint(a, b, length);
    endInsertRows();
  }

  void StructuresModel::addAngleConstraint(int a, int b, int c, double angle)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size());
    m_constraints.AddAngleConstraint(a, b, c, angle);
    endInsertRows();
  }

  void StructuresModel::addTorsionConstraint(int a, int b, int c, int d, double torsion)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size());
    m_constraints.AddTorsionConstraint(a, b, c, d, torsion);
    endInsertRows();
  }
  */

  void StructuresModel::removeStructure(int index)
  {
    qDebug() << "StructuresModel::deleteConstraint(" << index << ")" << endl;
    if (m_structures.size() && (index >= 0)) {
      beginRemoveRows(QModelIndex(), index, index);
      m_structures.removeAt(index);
      endRemoveRows();
    }
  }

  //////////////////////////////////////////////////////////////////////////////

  SpinBoxDelegate::SpinBoxDelegate(QObject *parent) : QItemDelegate(parent)
  {
  }

  QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &/* option */,
     const QModelIndex &/* index */) const
  {
    QSpinBox *editor = new QSpinBox(parent);
    editor->setMinimum(0);
    editor->setMaximum(100000000);
    return editor;
  }

  void SpinBoxDelegate::setEditorData(QWidget *editor,
      const QModelIndex &index) const
  {
    int value = index.model()->data(index, Qt::EditRole).toInt();
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
  }

  void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
      const QModelIndex &index) const
  {
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();
    model->setData(index, value, Qt::EditRole);
  }

  void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
      const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
  {
    editor->setGeometry(option.rect);
  }

  ComboBoxDelegate::ComboBoxDelegate(QObject *parent) : QItemDelegate(parent)
  {
  }

  QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &/* option */,
     const QModelIndex &/* index */) const
  {
    QComboBox *editor = new QComboBox(parent);
    return editor;
  }

  void ComboBoxDelegate::setEditorData(QWidget *editor,
      const QModelIndex &index) const
  {
    QStringList options = index.model()->data(index, ComboBoxRole).toStringList();
    int value = index.model()->data(index, Qt::EditRole).toInt();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    comboBox->addItems(options);
    comboBox->setCurrentIndex(value);
  }

  void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
      const QModelIndex &index) const
  {
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = comboBox->currentIndex();
    model->setData(index, value, Qt::EditRole);
  }

  void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
      const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
  {
    editor->setGeometry(option.rect);
  }

  FileDelegate::FileDelegate(QObject *parent) : QItemDelegate(parent)
  {
  }

  QWidget *FileDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &/* option */,
     const QModelIndex &/* index */) const
  {
    QWidget *editor = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout(editor);
    QLineEdit *lineEdit = new QLineEdit;
    QPushButton *button = new QPushButton("...");
    connect(button, SIGNAL(clicked()), this, SLOT(browseClicked()));
    button->setMaximumSize(29, 29);
    layout->addWidget(lineEdit);
    layout->addWidget(button);
    layout->setContentsMargins(0, 0, 0, 0);
    return editor;
  }

  void FileDelegate::setEditorData(QWidget *editor,
      const QModelIndex &index) const
  {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    foreach (QLineEdit *lineEdit, editor->findChildren<QLineEdit*>()) {
      lineEdit->setText(value);
    }
  }

  void FileDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
      const QModelIndex &index) const
  {
    QString value;
    foreach (QLineEdit *lineEdit, editor->findChildren<QLineEdit*>())
      value = lineEdit->text();
    model->setData(index, value, Qt::EditRole);
  }

  void FileDelegate::updateEditorGeometry(QWidget *editor,
      const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
  {
    editor->setGeometry(option.rect);
  }

  void FileDelegate::browseClicked()
  {
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open Molecule"));

    QObject *obj = sender()->parent()->parent();
    foreach (QLineEdit *lineEdit, obj->findChildren<QLineEdit*>())
      lineEdit->setText(fileName);
  }

  DensityDelegate::DensityDelegate(QObject *parent) : QItemDelegate(parent)
  {
  }

  QWidget *DensityDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &/* option */,
     const QModelIndex &/* index */) const
  {
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setSuffix(" g/ml");
    return editor;
  }

  void DensityDelegate::setEditorData(QWidget *editor,
      const QModelIndex &index) const
  {
    double value = index.model()->data(index, Qt::EditRole).toDouble();
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->setValue(value);
  }

  void DensityDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
      const QModelIndex &index) const
  {
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    double value = spinBox->value();
    model->setData(index, value, Qt::EditRole);
  }

  void DensityDelegate::updateEditorGeometry(QWidget *editor,
      const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
  {
    editor->setGeometry(option.rect);
  }

} // end namespace Avogadro

