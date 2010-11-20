/**********************************************************************
  DynamicDisplayExtension

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

#ifndef DYNAMICDISPLAYDIALOG_H
#define DYNAMICDISPLAYDIALOG_H

#include <QtGui/QDialog>

#include "ui_dynamicdisplaydialog.h"

namespace Avogadro {
  class Molecule;
}

namespace DynamicDisplay {
  class DynamicDisplayExtension;

  class DynamicDisplayDialog : public QDialog
  {
    Q_OBJECT

  public:
    explicit DynamicDisplayDialog(QWidget *parent,
                                  DynamicDisplayExtension *ext);

    virtual ~DynamicDisplayDialog() {};

  public slots:
    // Reset the GUI if there is no molecule loaded
    void reset();
    // Count the number of hydrogens in the passed molecule and update
    // the GUI
    void update(Avogadro::Molecule *mol);

  private:
    // Pointer to the extension
    DynamicDisplayExtension *m_ext;
    Ui::DynamicDisplayDialog ui;
  };
}

#endif
