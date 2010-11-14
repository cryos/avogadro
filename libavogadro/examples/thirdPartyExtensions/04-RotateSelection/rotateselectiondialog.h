/**********************************************************************
  RotateSelectionExtension

  Copyright (C) 2010 David C. Lonie

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public icense for more details.
 ***********************************************************************/

#ifndef ROTATESELECTIONDIALOG_H
#define ROTATESELECTIONDIALOG_H

#include <Eigen/Core>

#include <QtGui/QDialog>

#include "ui_rotateselectiondialog.h"

namespace Avogadro {
  class Molecule;
}

namespace RotateSelection {

  class RotateSelectionDialog : public QDialog
  {
    // The Q_OBJECT macro must be include if a class is to have
    // signals/slots
    Q_OBJECT

  public:
    explicit RotateSelectionDialog(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    virtual ~RotateSelectionDialog() {};

    void getTransform(Eigen::Vector3d *axis,
                      Eigen::Vector3d *offset,
                      double *angle);

  signals:
    void requestUpdateAxisFromBond();
    void requestApply();

  public slots:
    void setAxis(const Eigen::Vector3d &axis,
                 const Eigen::Vector3d &offset);

  private:
    // This member provides access to all ui elements
    Ui::RotateSelectionDialog ui;
  };
}

#endif
