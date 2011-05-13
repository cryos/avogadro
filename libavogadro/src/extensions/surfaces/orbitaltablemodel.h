/**********************************************************************
  OrbitalExtension - Molecular orbital explorer

  Copyright (C) 2010 by David C. Lonie

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

#ifndef ORBITALTABLEMODEL_H
#define ORBITALTABLEMODEL_H

#include "orbitalextension.h"

#include <QApplication>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

namespace Avogadro {

  struct calcInfo;

  struct Orbital {
    double energy;
    int index;
    QString description; // (HOMO|LUMO)[(+|-)N]
    QString symmetry; // e.g., A1g (with subscripts)
    calcInfo *queueEntry;
    // Progress data:
    int min;
    int max;
    int current;
    int stage;
    int totalStages;
  };

// Allow progress bars to be embedded in the table
class ProgressBarDelegate : public QStyledItemDelegate
{
  Q_OBJECT
    public:
  ProgressBarDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {};
  QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const {
    return QSize(60, 30);};

  void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &ind) const {
    QStyleOptionProgressBarV2 opt;
    // Call initFrom() which will set the style based on the parent
    // GRH: This is critical to get things right on Mac
    //   otherwise the status bars always look disabled
    opt.initFrom(qobject_cast<QWidget*>(this->parent()));

    opt.rect = o.rect;
    opt.minimum = 1; // percentage
    opt.maximum = 100;
    opt.textVisible = true;
    int percent = ind.model()->data(ind, Qt::DisplayRole).toInt();
    opt.progress = percent;
    opt.text = QString("%1%").arg(QString::number(percent));
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &opt, p);
  }
};

  // Used for sorting:
  class OrbitalSortingProxyModel : public QSortFilterProxyModel
  {
    Q_OBJECT

  public:
    OrbitalSortingProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent), m_HOMOFirst(false) {};

    bool isHOMOFirst() {return m_HOMOFirst;};
    void HOMOFirst(bool b) {m_HOMOFirst = b;};
  protected:
    // Compare orbital values
    bool lessThan ( const QModelIndex & left, const QModelIndex & right ) const {
      if (m_HOMOFirst)
        return left.row() < right.row();
      else
        return left.row() > right.row();
    }
  private:
    bool m_HOMOFirst;
  };

  class OrbitalTableModel : public QAbstractTableModel
  {
    Q_OBJECT

    public:

    enum Column {
      C_Description = 0,
      C_Energy,
      C_Symmetry,
      C_Status, // also occupation (0/1/2)

      COUNT
    };

    //! Constructor
    explicit OrbitalTableModel(QWidget *parent = 0);
    //! Deconstructor
    virtual ~OrbitalTableModel();

    int rowCount(const QModelIndex&) const {return m_orbitals.size();};
    int columnCount(const QModelIndex&) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QModelIndex HOMO() const;
    QModelIndex LUMO() const;

    bool setOrbital(const Orbital &Orbital);
    bool clearOrbitals();

    // Stages are used for multi-step processes, e.g. cube, posmesh, negmesh, etc
    void setOrbitalProgressRange(int orbital, int min, int max, int stage, int totalStages);
    void incrementStage(int orbital, int newmin, int newmax);
    void setOrbitalProgressValue(int orbital, int currentValue);
    void finishProgress(int orbital);
    void resetProgress(int orbital);
    void setProgressToZero(int orbital);

  private:
    QList<Orbital> m_orbitals;
  };
}

#endif
