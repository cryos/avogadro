/**********************************************************************
  constraintsmodel.cpp - Model to hold constraints

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

#include "constraintsmodel.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/forcefield.h>

#include <QtGui>
#include <QProgressDialog>
#include <QWriteLocker>
#include <QMutex>
#include <QMutexLocker>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  int ConstraintsModel::rowCount(const QModelIndex &parent) const
  {
    return m_constraints.Size();
  }

  int ConstraintsModel::columnCount(const QModelIndex &parent) const
  {
    return 6;
  }

  QVariant ConstraintsModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= m_constraints.Size())
      return QVariant();

    if (role == Qt::DisplayRole)
      switch (index.column()) {
        case 0:
           if (m_constraints.GetConstraintType(index.row()) == OBFF_CONST_IGNORE)
	       return QString("Ignore Atom");
           else if (m_constraints.GetConstraintType(index.row()) == OBFF_CONST_ATOM)
	       return QString("Fix Atom");
           else if (m_constraints.GetConstraintType(index.row()) == OBFF_CONST_ATOM_X)
	       return QString("Fix Atom X");
           else if (m_constraints.GetConstraintType(index.row()) == OBFF_CONST_ATOM_Y)
	       return QString("Fix Atom Y");
           else if (m_constraints.GetConstraintType(index.row()) == OBFF_CONST_ATOM_Z)
	       return QString("Fix Atom Z");
           else if (m_constraints.GetConstraintType(index.row()) == OBFF_CONST_DISTANCE)
	       return QString("Bond length");
           else if (m_constraints.GetConstraintType(index.row()) == OBFF_CONST_ANGLE)
	       return QString("Angle");
           else if (m_constraints.GetConstraintType(index.row()) == OBFF_CONST_TORSION)
	       return QString("Torsion angle");
	   break;
        case 1:
           return m_constraints.GetConstraintValue(index.row());
	   break;
        case 2:
           return m_constraints.GetConstraintAtomA(index.row());
	   break;
        case 3:
           return m_constraints.GetConstraintAtomB(index.row());
	   break;
        case 4:
           return m_constraints.GetConstraintAtomC(index.row());
	   break;
        case 5:
           return m_constraints.GetConstraintAtomD(index.row());
	   break;
      }
    else
      return QVariant();
  }
  
  QVariant ConstraintsModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    if (orientation == Qt::Horizontal) {
      switch (section) {
        case 0:
	  return QString("Type");
	  break;
        case 1:
	  return QString("Value");
	  break;
        case 2:
	  return QString("Atom idx 1");
	  break;
        case 3:
	  return QString("Atom idx 2");
	  break;
        case 4:
	  return QString("Atom idx 3");
	  break;
        case 5:
	  return QString("Atom idx 4");
	  break;
      }
    } else
      return QString("Constraint %1").arg(section + 1);
  
  }
 
  void ConstraintsModel::addIgnore(int index)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size()); 
    m_constraints.AddIgnore(index);
    endInsertRows();
  }
  
  void ConstraintsModel::addAtomConstraint(int index)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size()); 
    m_constraints.AddAtomConstraint(index);
    endInsertRows();
  }
  
  void ConstraintsModel::addAtomXConstraint(int index)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size()); 
    m_constraints.AddAtomXConstraint(index);
    endInsertRows();
  }
  
  void ConstraintsModel::addAtomYConstraint(int index)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size()); 
    m_constraints.AddAtomYConstraint(index);
    endInsertRows();
  }
  
  void ConstraintsModel::addAtomZConstraint(int index)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size()); 
    m_constraints.AddAtomZConstraint(index);
    endInsertRows();
  }
  
  void ConstraintsModel::addBondConstraint(int a, int b, double length)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size()); 
    m_constraints.AddBondConstraint(a, b, length);
    endInsertRows();
  }
  
  void ConstraintsModel::addAngleConstraint(int a, int b, int c, double angle)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size()); 
    m_constraints.AddAngleConstraint(a, b, c, angle);
    endInsertRows();
  }
  
  void ConstraintsModel::addTorsionConstraint(int a, int b, int c, int d, double torsion)
  {
    beginInsertRows(QModelIndex(), m_constraints.Size(), m_constraints.Size()); 
    m_constraints.AddTorsionConstraint(a, b, c, d, torsion);
    endInsertRows();
  }
 
  void ConstraintsModel::clear()
  {
    qDebug() << "ConstraintsModel::clear()" << endl;
    if (m_constraints.Size()) {
      beginRemoveRows(QModelIndex(), 0, m_constraints.Size() - 1); 
      m_constraints.Clear();
      endRemoveRows();
    }
  }
  
  void ConstraintsModel::deleteConstraint(int index)
  { 
    qDebug() << "ConstraintsModel::deleteConstraint(" << index << ")" << endl;
    if (m_constraints.Size() && (index >= 0)) {
      beginRemoveRows(QModelIndex(), index, index); 
      m_constraints.DeleteConstraint(index);
      endRemoveRows();
    }
  }
  
  // remove all constraints in which the atom occurs
  void ConstraintsModel::primitiveRemoved(Primitive *primitive)
  {
    qDebug() << "ConstraintsModel::primitiveRemoved(...)" << endl;
    if (primitive->type() == Primitive::AtomType) {
      int index = static_cast<Atom*>(primitive)->GetIdx();
      for (int i = 0; i < m_constraints.Size(); ++i) {
        if ( (m_constraints.GetConstraintAtomA(i) == index) || 
	     (m_constraints.GetConstraintAtomB(i) == index) || 
	     (m_constraints.GetConstraintAtomC(i) == index) || 
	     (m_constraints.GetConstraintAtomD(i) == index) ) {

          beginRemoveRows(QModelIndex(), i, i);
	  m_constraints.DeleteConstraint(i);
          endRemoveRows();
	  i--; // this index will be replaced with a new, we want to check this aswell
	}
      }
    }
  }
} // end namespace Avogadro

#include "constraintsmodel.moc"
