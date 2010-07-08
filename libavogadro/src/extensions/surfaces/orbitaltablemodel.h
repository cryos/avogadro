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

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

namespace Avogadro {

  struct calcInfo;

  struct Orbital {
    double energy;
    int index;
    QString description; // (HOMO|LUMO)[(+|-)N]
    calcInfo *queueEntry;
    // Progress data:
    int min;
    int max;
    int current;
    int stage;
    int totalStages;
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
      C_Status,

      COUNT
    };

    //! Constructor
    explicit OrbitalTableModel(QWidget *parent = 0);
    //! Deconstructor
    virtual ~OrbitalTableModel();

    int rowCount(const QModelIndex&) const {return m_orbitals.size();};
    int columnCount(const QModelIndex&) const {return COUNT;}; // Energy, description, progress

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
