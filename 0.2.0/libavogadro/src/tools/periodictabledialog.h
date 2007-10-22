/**********************************************************************
  PeriodicTableDialog - widget for selecting elements (e.g., for drawing)

  Copyright (C) 2007 Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  This code is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation version 2.1 of the License.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
 ***********************************************************************/

#ifndef PERIODICTABLEDIALOG_H
#define PERIODICTABLEDIALOG_H

#include <QtGui/QDialog>

#include "ui_periodictabledialog.h"

namespace Avogadro {

  /**
   * @class PeriodicTableDialog <periodictabledialog.h>
   * @brief Class for picking individual elements, e.g. for drawing
   * @author Geoffrey R. Hutchison
   *
   * This class presents a general table of the elements (with atomic symbols)
   * described via the Qt Designer interface. Each element is a QToolButton
   * to be clicked by the user. To set the initial state, use
   * setSelectedElement() before showing the dialog.
   */
class PeriodicTableDialog : public QDialog
{
    Q_OBJECT
    
    public:
        PeriodicTableDialog (QWidget *parent);
        ~PeriodicTableDialog();

        //! Initial setup -- make sure one element is visually selected 
        void setSelectedElement(int id);

    private Q_SLOTS:
        void buttonClicked(int id);

    Q_SIGNALS:
        //! Sent any time an element is changed
        void elementChanged(int newElement);

    private:
        //! Qt Designer layout in periodictabledialog.ui 
        //! => ui_periodictabledialog.h
        Ui::PeriodicTableDialog ui;
        //! Logical grouping of element tool buttons
        QButtonGroup *elementGroup;
        //! Currently selected element number
        int currentElement;
};

} // end namespace Avogadro

#endif // PERIODICTABLEDIALOG_H
