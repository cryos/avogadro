/**********************************************************************
  ElementComboBox - Element ComboBox Widget

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.  For more
  information, see <http://avogadro.sourceforge.net/>

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

#ifndef __ELEMENTCOMBOBOX_H
#define __ELEMENTCOMBOBOX_H


#include <avogadro/primitives.h>
#include <QComboBox>

namespace Avogadro {

  class ElementComboBox : public QComboBox
  {
    Q_OBJECT

    public:
      ElementComboBox(QWidget *parent=0);
      
      void showPopup();
      void hidePopup();
  };

}

#endif
