/**********************************************************************
  Copyright (C) 2007 Geoffrey R. Hutchison

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

#ifndef PERIODICTABLEDIALOG_H
#define PERIODICTABLEDIALOG_H

#include <QtGui/QDialog>

#include "ui_periodictabledialog.h"

class QAbstractButton;
namespace Avogadro {

class PeriodicTableDialog : public QDialog
{
    Q_OBJECT
    
    public:
        PeriodicTableDialog (QWidget *parent);
        ~PeriodicTableDialog();

        void setSelectedElement(int id);

    private Q_SLOTS:
        void buttonClicked(int id);

    private:
        Ui::PeriodicTableDialog ui;
        QButtonGroup *elementGroup;
        int initialElement;
        int currentElement;

    Q_SIGNALS:
        void elementChanged(int newElement);
};

} // end namespace Avogadro

#endif // PERIODICTABLEDIALOG_H
